#define _GNU_SOURCE  //define necesario para poder usar join no bloqueante de hilos
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<errno.h>
#include <signal.h>
#include <string.h>
#include "struct.h"
#include "claves.h"
#include "socket_message.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define MAX_THREADS 25

//Inicializador global de los fd para la bbdd y la queue del servidor
sqlite3* database_server = 0;


//Creación de hilos, arrays de hilos y contador de hilos ocupados
pthread_t thread_pool[MAX_THREADS];
int sc[MAX_THREADS];
int free_threads_array[MAX_THREADS];

//Inicializador de mutex para la copia local de parámetros, la gestión de la bbdd y variable condicion
int free_mutex_copy_params_cond = 0;
pthread_mutex_t mutex_copy_params;

pthread_cond_t cond_wait_cpy;
//contador para saber cuantos hilos estan trabajando
int workload = 0;

/**
 *@brief Esta función se usa para rellenar el array auxiliar que indica qué hilo está trabajando(1)
 *y cuál está libre(0)
 */
void pad_array()

{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        free_threads_array[i] = 0;
    }
}

/**
 *@brief Esta función se usa para enviar el mensaje de vuelta al cliente, recibe como parametros el socket y la estructura request
 * y con ella envia los datos. es invocada por cada hilo en la funcion process request.
 */
int answer_back(int socket, request* params)
{
    return send_message(socket, params);
}


/**
 *@brief Esta es la función que ejecutan los distintos hilos dentro de nuestra pool de hilos
 *Requiere de la dir de memoria de uns estructura de tipo parameters_to_pass que se compone de
 *una estructura petición y una variable id, que identifica al thread que está ejecutando, para
 *indicar en el array de threads que ese hilo no está disponible en el momento. Una vez realiza la copia
 *local de los datos se encargará de gestionar las distintas llamadas de claves.c para realizar las gestiones
 *en la base de datos correspondiente
 */
int process_request(parameters_to_pass *socket)
{
    pthread_mutex_lock(&mutex_copy_params);
    int socket_id = socket->identifier;
    free_mutex_copy_params_cond = 1;
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);

    request local_request = {0};
    int message = receive_message(sc[socket_id], &local_request);
    if (message < 0)
    {
        pthread_exit(0);
    }
    printf("Value 1 es %s\n", local_request.value_1);
    switch (local_request.type)
    {
    case 1: //INSERT
        local_request.answer = set_value(local_request.key, local_request.value_1, local_request.N_value_2,
                                         local_request.value_2, local_request.value_3);
        if (local_request.answer == -1)
        {
            printf("ERROR inserting, failure was detected\n");
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);
    case 2: // DELETE (destroy)
        local_request.answer = destroy();
        if (local_request.answer == -1)
        {
            printf("ERROR erasing tuples with destroy()\n");
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);
    case 3: // DELETE_KEY (delete_key)
        local_request.answer = delete_key(local_request.key);
        if (local_request.answer == -1)
        {
            printf("ERROR erasing key %d with delete_key()\n", local_request.key);
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);
    case 4: // MODIFY
        local_request.answer = modify_value(local_request.key, local_request.value_1, local_request.N_value_2,
                                            local_request.value_2, local_request.value_3);
        if (local_request.answer == -1)
        {
            printf("ERROR modifying key %d with modify_value()\n", local_request.key);
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);

    case 5: // GET_VALUE
        local_request.answer = get_value(local_request.key, local_request.value_1, &local_request.N_value_2,
                                         local_request.value_2, &local_request.value_3);
        if (local_request.answer == -1)
        {
            printf("ERROR obtaining key %d with get_value()\n", local_request.key);
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);
    case 6: //EXIST
        local_request.answer = exist(local_request.key);
        if (local_request.answer == -1)
        {
            printf("ERROR verifying key %d with exist()\n", local_request.key);
        }
        answer_back(sc[socket_id],&local_request);
        pthread_exit(0);
    default:
        pthread_exit(0);
    }
}


/**
 *@brief Función implementada al inicializarse el servidor que creará las tablas de SQL que se encargarán
 *de mantener nuestros datos ordenados. Hemos creado 2 tablas, una "data" que guardará tanto el id(Primary key)
 *como value1 y value3. Value2 como es un array de longitud variable, nos hemos creado una tabla "value2_all"
 *que hereda de data la PK a modo de Foreign Key con UPDATE y DELETE CASCADE(Si se borra la pk de data, se borrarán
 *todas las referencias a ella en "value2-all". La pk de esta tabla será para cada elemento del array, la conversión
 *a entero de la concatenación de la PK con el índice del elemento en el array. Por ejemplo, si para id 3 tengo que
 *insertar el vector {3.44, 2.15, 14.33} tendré 3 filas en esta nueva tabla
 *  PK   FK   VALUE
 *  30   3    3.44
 *  31   3    2.15
 *  32   3    14.33
 */
int create_table(sqlite3* db)
{
    char* message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database_server);
        return -4;
    }

    char* new_table =
        "CREATE TABLE IF NOT EXISTS data("
        " data_key INTEGER PRIMARY KEY,"
        " value1 TEXT,"
        " x INTEGER,"
        " y INTEGER"
        ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR CREATING MAIN TABLE %s\n", message_error);
        sqlite3_close(database_server);
        return -4;
    }
    message_error = NULL;
    new_table =
        "CREATE TABLE IF NOT EXISTS value2_all("
        " id TEXT PRIMARY KEY,"
        " data_key_fk INTEGER,"
        " value REAL,"
        "CONSTRAINT fk_origin FOREIGN KEY(data_key_fk) REFERENCES data(data_key)\n ON DELETE CASCADE\n"
        "ON UPDATE CASCADE);";

    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR CREATING SECONDARY TABLE 2\n");
        sqlite3_close(database_server);
        return -4;
    }
    return 0;
}


/**
 *@brief Función implementada para hacer un cierre seguro del servidor cuando se pulsa CRTL + C
 */
void safe_close(int ctrlc)
{
    printf("\n-----------------------------------------------\n");
    printf("\nEXIT SIGNAL RECEIVED. CLOSING ALL AND GOODBYE\n");
    printf("-----------------------------------------------\n");
    exit(0);
}


int main(int argc, char **argv)
{

    if(argc < 2)
    {
        perror("Server port not indicated\n");
        exit(-2);
    }
    int port_num = atoi(argv[1]);
    if(port_num <= 0 || port_num > 65535)
    {
        perror("Bad port\n");
        return -1;
    }
    //Inicializo la signal para tratar el crtl c y realizar un cierre seguro de la aplicacion
    signal(SIGINT, safe_close);

    //Creando e inicializando la base de datos
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    int create_database = sqlite3_open("/tmp/database.db", &database_server);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        exit(-4);
    }
    //Creo la tabla principal "data" y la subtabla "value2_all"
    if (create_table(database_server) < 0)
    {
        exit(-4);
    }
    sqlite3_close(database_server);

    //LLeno de 0s el array de threads ocupados
    pad_array();


    //Inicializo lo necesario para los socketss.
    struct sockaddr_in server_addr,  client_addr;
    socklen_t size;
    int sd, val = 0;
    int err;

    //inicializacion mutex para la copia local de parametros
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);



    if ((sd =  socket(AF_INET, SOCK_STREAM, 0))<0){
        printf ("Error creating socket");
        exit(-2);
    }
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int));

    bzero((char *)&server_addr, sizeof(server_addr));
    char *ip_str = getenv("IP_TUPLAS");
    if (!ip_str){
        fprintf(stderr, "ENV variable 'IP_TUPLAS' not defined\n");
        exit(-2);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port_num);
    if(inet_aton(ip_str, &server_addr.sin_addr) == 0){
        fprintf(stderr, "Invalid IP Adress\n");
        exit(-2);
    }

    err = bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1)
    {
        printf("Error on bind\n");
        return -1;
    }
    err = listen(sd, SOMAXCONN);
    if (err == -1) {
        printf("Error on listen\n");
        return -1;
    }

    //Me creo la estructura request para manejar las peticiones
    parameters_to_pass socket_id = {0};
    int sc_temp = 0;
    //Gestion de la concurrencia con las peticiones
    printf("SERVER ACTIVE. WAITING FOR REQUESTS............\n");
    while (1)
    {
        //Esto intenta realizar un join no bloqueante
        for (int i = 0; i < MAX_THREADS; i++)
        {
            if (free_threads_array[i] == 1)
            {
                //Si el hilo ha estado trabajando miro a ver si puedo hacerle join
                if (pthread_tryjoin_np(thread_pool[i], NULL) == 0)
                {
                    free_threads_array[i] = 0; //Al ruedo de nuevo, maquina
                    close(sc[i]);   //Cerramos el sc
                    workload--; // Avisar de que se puede currar
                    printf("Thread %d free and ready to reuse\n", i);
                }
            }
        }
        //Veo si hay mensajes
        sc_temp = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);
        if (sc_temp >= 0)
        {

            //Miro el primer hilo disponible y le mando currar.
            for (int i = 0; i < MAX_THREADS; i++)
            {
                if (free_threads_array[i] == 0)
                {
                    free_threads_array[i] = 1;
                    sc[i] = sc_temp;
                    workload++;
                    printf("Thread %d is now working\n", i);
                    socket_id.identifier = i;
                    if (pthread_create(&thread_pool[i],NULL, (void*)process_request, &socket_id) == 0)
                    {
                        pthread_mutex_lock(&mutex_copy_params);
                        while (free_mutex_copy_params_cond == 0)
                            pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                        free_mutex_copy_params_cond = 0;
                        pthread_mutex_unlock(&mutex_copy_params);
                    }
                    else
                    {
                        perror("Error creating thread\n");
                        exit(-1);
                    }
                    break;
                }

                //En caso de que no se pueda trabajar porque no hay hilos disponibles, en vez de hacer un wait, como
                //el mensaje ha sido leido ya, lo reenvio a la cola
                /*
                if (workload == MAX_THREADS)
                {
                    message = mq_send(server_queue, (char*)&new_request, sizeof(request), 0);
                    if (message < 0)
                    {
                        exit(-2);
                    }
                    printf("Back in queue again with id: %d\n", new_request.key);
                    usleep(500);
                }
                */
            }
        }
        else
        {
            //si se me lia el mensaje
            printf("Error receiving message %s\n", strerror(errno));
            exit(-2);
        }
    }
}
