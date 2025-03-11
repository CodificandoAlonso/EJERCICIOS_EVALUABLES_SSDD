#define _GNU_SOURCE  //define necesario para poder usar join no bloqueante de hilos
#include<stdio.h>
#include<pthread.h>
#include "../claves.h"
#include<mqueue.h>
#include<stdlib.h>
#include "../claves.h"
#include<sqlite3.h>
#include<errno.h>
#include <string.h>
#include<unistd.h>
#include "headers.h"


#define MAX_THREADS 10





//Creación de hilos, arrays de hilos y contador de hilos ocupados
pthread_t thread_pool[MAX_THREADS];
int free_threads_array[MAX_THREADS];
int workload = 0;
int free_mutex_copy_params_cond = 0;
pthread_mutex_t mutex_copy_params;
pthread_cond_t cond_wait_cpy;
pthread_mutex_t mutex_threads;
pthread_cond_t cond_wait_threads;



/**@brief Esta función se usa para rellenar el array auxiliar que indica qué hilo está trabajando(1)
 *y cuál está libre(0)
 */
void pad_array()

{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        free_threads_array[i] = 0;
    }
}


int answer_back(request *params) {
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 8; // Máximo 10 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;
    mqd_t client_queue;
    char name[32];
    sprintf(name, "client_queue_%s",params->client_queue);
    client_queue = mq_open(params->client_queue,O_CREAT | O_RDWR | O_NONBLOCK, 0660, &attr);
    if (client_queue < 0) {
        perror("Error abriendo la cola del cliente\n");
    }
    if(mq_send(client_queue, (char *)params, sizeof(request), 0) == -1) {
        perror("Error enviando\n");
    }
    return 0;
}




/**@brief Esta es la función que ejecutan los distintos hilos dentro de nuestra pool de hilos
 *Requiere de la dir de memoria de uns estructura de tipo parameters_to_pass que se compone de
 *una estructura petición y una variable id, que identifica al thread que está ejecutando, para
 *indicar en el array de threads que ese hilo no está disponible en el momento. Una vez realiza la copia
 *local de los datos se encargará de gestionar las distintas llamadas de claves.c para realizar las gestiones
 *en la base de datos correspondiente
 */
int process_request(parameters_to_pass* parameters)
{
    pthread_mutex_lock(&mutex_copy_params);
    request local_request = *parameters->this_request;
    int local_id = parameters->identifier;
    free_mutex_copy_params_cond = 1;
    free_threads_array[local_id] = 1;
    printf("Hilo %d ocupado\n", local_id);
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);

    switch (local_request.type){
        case 1: //INSERT
            int insert = set_value(local_request.key, local_request.value_1, local_request.N_value_2, local_request.value_2, local_request.value_3);
            if (insert == -1)
            {
                printf("ERROR INSERTANDO ME HA DEVUELTO FALIO\n");
                return -1;
            }
            return 0;
    case 2:  // DELETE (destroy)
        int delete = destroy();
        if (delete == -1) {
            printf("ERROR al eliminar TODAS las tuplas con destroy()\n");
            return -1;
        }
            printf("Todas las tuplas eliminadas correctamente con destroy()\n");
            return 0;

    case 3:  // DELETE_KEY (delete_key)
        int deletekey = delete_key(local_request.key);
        if (deletekey == -1) {
            printf("ERROR eliminando la clave %d con delete_key()\n", local_request.key);
            return -1;
        } else {
            printf("Clave %d eliminada correctamente con delete_key()\n", local_request.key);
        }
        return 0;
    case 4:  // MODIFY
        local_request.answer = modify_value(local_request.key, local_request.value_1, local_request.N_value_2, local_request.value_2, local_request.value_3);
        if (local_request.answer == -1) {
            printf("ERROR modificando la clave %d con modify_value()\n", local_request.key);
            return -1;
        } else {
            printf("Clave %d modificada correctamente con modify_value()\n", local_request.key);
        }
        return 0;

    case 5:  // GET_VALUE
        local_request.answer = get_value(local_request.key, local_request.value_1, &local_request.N_value_2, local_request.value_2, &local_request.value_3);
        if (local_request.answer == -1) {
            printf("ERROR obteniendo la clave %d con get_value()\n", local_request.key);
            return -1;
        } else {
            answer_back(&local_request);
        }
        return 0;


    }

    printf("Acabado el trabajo el hilo %d\n", local_id);
    pthread_exit(0);
}




/**@brief Función implementada al inicializarse el servidor que creará las tablas de SQL que se encargarán
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
void create_table(sqlite3* db)
{
    char* message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        exit(-3);
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
        fprintf(stderr, "ERROR CREANDO TABLA 1\n");
        exit(-4);
    }
    printf("EXITO CREANDO TABLA 1\n");
    message_error = NULL;
    new_table =
        "CREATE TABLE IF NOT EXISTS value2_all("
        " id TEXT PRIMARY KEY,"
        " data_key_fk INTEGER,"
        " value REAL,"
        "CONSTRAINT fk_origin FOREIGN KEY(data_key_fk) REFERENCES data(data_key)\n ON DELETE CASCADE\n"
        "ON UPDATE CASCADE);";

    printf("%s\n", new_table);
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR CREANDO TABLA 2\n");
        exit(-4);
    }
    printf("EXITO CREANDO TABLA 2\n");
}








int main(int argc, char* argv[])
{
    //Creando e inicializando la base de datos
    sqlite3* database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        exit(-1);
    }
    printf("Exito abriendo la base de datos\n");

    //Creo la tabla principal "data" y la subtabla "value2_all"
    create_table(database);
    sqlite3_close(database);

    //LLeno de 0s el array de threads ocupados
    pad_array();



    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Máximo 10 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;


    //inicializacion mutex para la copia local de parametros
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);
    //inicializacion mutex para la variacion del numero de threads usados
    pthread_mutex_init(&mutex_threads, NULL);
    pthread_cond_init(&cond_wait_threads, NULL);


    //Inicializo y abro la cola del servidor
    mqd_t server_queue;
    char *nombre = "/servidor_queue_9453";
    server_queue = mq_open(nombre, O_CREAT | O_RDWR | O_NONBLOCK, 0660, &attr);
    if (server_queue == -1)
    {
        mq_close(server_queue);
        fprintf(stderr, "Error abriendo la cola del servidor: %s %s\n", strerror(errno), nombre);
        printf("Error abriendo la cola del servidor\n");
        return -1;
    }
    printf("Todo bien abriendo la cola del servidor con fd: %d\n", server_queue);


    //Me creo la estructura request para manejar las peticiones
    request new_request;
    //Gestion de la concurrencia con las peticiones
    while (1)
    {
        /*
        printf("PARA SALIR PON 'exit' mamawebo\n");
        fgets(buffer,sizeof(buffer),stdin);
        if (strcmp(buffer, "exit"))
        {
            break;
        }
        */
        //Esto intenta realizar un join no bloqueante
        pthread_mutex_lock(&mutex_threads);
        for (int i = 0; i < MAX_THREADS; i++) {
            if (free_threads_array[i] == 1) { //Si el hilo ha estado trabajando miro a ver si puedo hacerle join
                if (pthread_tryjoin_np(thread_pool[i], NULL)== 0) {
                    free_threads_array[i] = 0; //Al ruedo de nuevo, maquina
                    workload--; // Avisar de que se puede currar
                    printf("Hilo %d liberado y listo para reutilizarse\n", i);
                }
            }
        }
        pthread_mutex_unlock(&mutex_threads);
        //Veo si hay mensajes
        ssize_t message = mq_receive(server_queue, (char*)&new_request, sizeof(request), 0);
        if (message >= 0)
        {
            printf("Se ha recibido un mensaje con id %d\n", new_request.key);


            //Miro el primer hilo disponible y le mando currar. JAAAAPIUU, a currar esclavo
            //Necesita seccion critica por el acceso al array
            pthread_mutex_lock(&mutex_threads);
            for(int i =0; i< MAX_THREADS; i++)
            {
                if(free_threads_array[i] == 0){
                    parameters_to_pass params;
                    params.identifier = i;
                    params.this_request = &new_request;
                    pthread_create(&thread_pool[i],NULL,(void *)process_request,&params);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex_threads);


            //Solo hago la espera si no hay ningun hilo currando. Si son todos unos putos vagos de mierda no entro
            pthread_mutex_lock(&mutex_threads);
            if (workload >0)
            {
                pthread_mutex_lock(&mutex_copy_params);
                while(free_mutex_copy_params_cond == 0)
                    pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                free_mutex_copy_params_cond = 0;
                pthread_mutex_unlock(&mutex_copy_params);
            }
            else
            {
                pthread_mutex_unlock(&mutex_threads);
            }
        }
        //ERROR RECEPCION MENSAJE
        else if(errno == EAGAIN) {
        usleep(1000);
        }
        else {
            //si se me lia el mensaje
            printf("Error al recibir mensaje: %s\n", strerror(errno));
            //DE MOMENTO SI ES ERROR EN COMUNICACION ES ERROR -2
            exit(-2);
        }
    }
    return 0;
}
