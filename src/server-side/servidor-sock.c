#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "struct.h"
#include "claves.h"
#include "socket_message.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <netdb.h>
#define MAX_THREADS 25

//Inicializador global de los fd para la bbdd y la queue del servidor
sqlite3* database_server = 0;

//Creación de hilos, arrays de hilos y contador de hilos ocupadosc
pthread_t thread_pool[MAX_THREADS];
int sc[MAX_THREADS];
int free_threads_array[MAX_THREADS];

//Inicializador de mutex para la copia local de parámetros, la gestión de la bbdd y variable condicion
//E inicializador de semáforos para gestionar bien la concurrencia
int free_mutex_copy_params_cond = 0;
sem_t available_threads;
pthread_mutex_t mutex_workload;
pthread_mutex_t mutex_copy_params;
pthread_cond_t cond_wait_cpy;


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
 *@brief Función que se encarga de "anunciar" que un hilo ha terminado su tarea y pueda volver al ruedo
 */
void end_thread(int thread_id) {
    pthread_mutex_lock(&mutex_workload);
    free_threads_array[thread_id] = 0;
    close(sc[thread_id]);
    sem_post(&available_threads);
    pthread_mutex_unlock(&mutex_workload);
    pthread_exit(NULL);
}


void * process_request(parameters_to_pass *socket) {
    pthread_mutex_lock(&mutex_copy_params);
    int socket_id = socket->identifier;
    free_mutex_copy_params_cond = 1;
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);

    request local_request = {0};
    int message = receive_message(sc[socket_id], &local_request);
    if (message < 0)
    {
        end_thread(socket_id);
        pthread_exit(0);
    }

    switch (local_request.type) {
        case 1: // INSERT
            local_request.answer = set_value(local_request.key, local_request.value_1, local_request.N_value_2, local_request.value_2, local_request.value_3);
            break;
        case 2: // DELETE
            local_request.answer = destroy();
            break;
        case 3: // DELETE_KEY
            local_request.answer = delete_key(local_request.key);
            break;
        case 4: // MODIFY
            local_request.answer = modify_value(local_request.key, local_request.value_1, local_request.N_value_2, local_request.value_2, local_request.value_3);
            break;
        case 5: // GET_VALUE
            local_request.answer = get_value(local_request.key, local_request.value_1, &local_request.N_value_2, local_request.value_2, &local_request.value_3);
            break;
        case 6: // EXIST
            local_request.answer = exist(local_request.key);
            break;
        default:
            local_request.answer = -1;
            break;
    }

    send_message(sc[socket_id], &local_request);
    end_thread(socket_id);
    return NULL;
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


int main(int argc, char **argv) {
    if (argc < 2) {
        perror("Server port not indicated\n");
        exit(-2);
    }
    int port_num = atoi(argv[1]);
    if (port_num <= 0 || port_num > 65535) {
        perror("Bad port\n");
        return -1;
    }

    signal(SIGINT, safe_close);

    //Creando e inicializando la base de datos
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    char *user = getlogin(); //PARA LA BASE DE DATOS
    char db_name[256];
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);
    int create_database = sqlite3_open(db_name, &database_server);
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

    //Inicializacion mutex, semaforo y estructuras
    sem_init(&available_threads, 0, MAX_THREADS);
    pthread_mutex_init(&mutex_workload, NULL);
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);
    pad_array();

    //Inicializo lo necesario para los sockets.
    struct sockaddr_in server_addr, client_addr;
    socklen_t size;
    int sd, val = 0;
    int err;

    //Crear socket 0principal sd
    if ((sd =  socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creating socket\n");
        exit(-2);
    }
    val = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int));

    // bzero al server_addr
    bzero((char *)&server_addr, sizeof(server_addr));

    //Obtenemos ip de env
    char *ip_str = getenv("IP_TUPLAS");
    if (!ip_str){
        fprintf(stderr, "ENV variable 'IP_TUPLAS' not defined\n");
        exit(-2);
    }

    //getaddrinfo PARA RESOLVER TANTO IP NUMÉRICA COMO TEXTO ("localhost", etc.)
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int gai_ret = getaddrinfo(ip_str, NULL, &hints, &res);
    if (gai_ret != 0) {
        fprintf(stderr, "Invalid IP/hostname: %s\n", ip_str);
        exit(-2);
    }

    //Copiamos la dirección resultante en server_addr
    struct sockaddr_in *addr4 = (struct sockaddr_in *)res->ai_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = addr4->sin_addr;
    server_addr.sin_port = htons((uint16_t)port_num);
    freeaddrinfo(res);

    //bind
    err = bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("Error on bind\n");
        return -1;
    }
    //luisten
    err = listen(sd, SOMAXCONN);
    if (err == -1) {
        printf("Error on listen\n");
        return -1;
    }

    //inicializacion de la estructura para pasar parametros al hilo.
    //El único parametro será el id del pool del hilo y del pool de sockets sc, que es el mismo
    parameters_to_pass params = {0};
    while (1) {

        sem_wait(&available_threads); // Esperar hasta que haya hilos libres
        //Se realiza el accept en in sc temporal
        int sc_temp = accept(sd, (struct sockaddr *)&client_addr, &size);
        if (sc_temp < 0) {
            sem_post(&available_threads);
            continue;
        }
        //Miro el primer hilo disponible y le mando currar.
        for (int i = 0; i < MAX_THREADS; i++) {
            if (free_threads_array[i] == 0) {
                pthread_mutex_lock(&mutex_workload);
                free_threads_array[i] = 1;
                //Se copia el fd del sc_temp al indice correspondiente
                sc[i] = sc_temp;
                params.identifier = i;
                pthread_mutex_unlock(&mutex_workload);
                if (pthread_create(&thread_pool[i], NULL, (void*)process_request, &params) == 0) {
                    pthread_mutex_lock(&mutex_copy_params);
                    while (free_mutex_copy_params_cond == 0)
                        pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                    free_mutex_copy_params_cond = 0;
                    pthread_mutex_unlock(&mutex_copy_params);
                }
                break;
            }
        }
    }
}