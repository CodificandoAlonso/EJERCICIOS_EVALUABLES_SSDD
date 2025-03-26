#define _GNU_SOURCE  //define necesario para poder usar join no bloqueante de hilos
#include<stdio.h>
#include<pthread.h>
#include<mqueue.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<errno.h>
#include <signal.h>
#include <string.h>
#include "struct.h"
#include "claves.h"


#define MAX_THREADS 50

//Inicializador global de los fd para la bbdd y la queue del servidor
sqlite3 *database_server = 0;
mqd_t server_queue = 0;


//Creación de hilos, arrays de hilos y contador de hilos ocupados
pthread_t thread_pool[MAX_THREADS];
int free_threads_array[MAX_THREADS];

//Inicializador de mutex para la copia local de parámetros, la gestión de la bbdd y variable condicion
int free_mutex_copy_params_cond = 0;
pthread_mutex_t mutex_copy_params;
//pthread_mutex_t ddbb_mutex;
pthread_cond_t cond_wait_cpy;

//contador para saber cuantos hilos estan trabajando
int workload = 0;

/**
 *@brief Esta función se usa para rellenar el array auxiliar que indica qué hilo está trabajando(1)
 *y cuál está libre(0)
 */
void pad_array() {
    for (int i = 0; i < MAX_THREADS; i++) {
        free_threads_array[i] = 0;
    }
}

/**
 *@brief Esta función se usa para enviar el mensaje de vuelta al cliente, recibe como parametros la estructura request
 * y con ella envia los datos. es invocada por cada hilo en la funcion process request.
 */
int answer_back(request *params) {
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 8; // Máximo 10 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;
    mqd_t client_queue;
    char name[32];
    //Name nunca pued ser mayor que 32, de ahi el .18s
    snprintf(name, sizeof(name), "client_queue_%.18s", params->client_queue);
    client_queue = mq_open(params->client_queue,O_CREAT | O_WRONLY, 0644, &attr);
    if (client_queue < 0) {
        printf("Error opening clients queue\n");
        return -2;
    }
    if (mq_send(client_queue, (char *) params, sizeof(request), 0) == -1) {
        printf("Error sending\n");
        return -2;
    }
    return 0;
}


/**
 *@brief Esta es la función que ejecutan los distintos hilos dentro de nuestra pool de hilos
 *Requiere de la dir de memoria de uns estructura de tipo parameters_to_pass que se compone de
 *una estructura petición y una variable id, que identifica al thread que está ejecutando, para
 *indicar en el array de threads que ese hilo no está disponible en el momento. Una vez realiza la copia
 *local de los datos se encargará de gestionar las distintas llamadas de claves.c para realizar las gestiones
 *en la base de datos correspondiente
 */
int process_request(request *request_received) {
    pthread_mutex_lock(&mutex_copy_params);
    request local_request = *request_received;
    free_mutex_copy_params_cond = 1;
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);
    switch (local_request.type) {
        case 1: //INSERT
            //pthread_mutex_lock(&ddbb_mutex);
            local_request.answer = set_value(local_request.key, local_request.value_1, local_request.N_value_2,
                                             local_request.value_2, local_request.value_3);
            //pthread_mutex_unlock(&ddbb_mutex);
            if (local_request.answer == -1) {
                printf("ERROR inserting, failure was detected\n");
            }
            answer_back(&local_request);

            pthread_exit(0);
        case 2: // DELETE (destroy)
            //pthread_mutex_lock(&ddbb_mutex);
            local_request.answer = destroy();
            //pthread_mutex_unlock(&ddbb_mutex);
            if (local_request.answer == -1) {
                printf("ERROR erasing tuples with destroy()\n");
            }
            answer_back(&local_request);
            pthread_exit(0);
        case 3: // DELETE_KEY (delete_key)
            //pthread_mutex_lock(&ddbb_mutex);
            local_request.answer = delete_key(local_request.key);
            //pthread_mutex_unlock(&ddbb_mutex);
            if (local_request.answer == -1) {
                printf("ERROR erasing key %d with delete_key()\n", local_request.key);
            }
            answer_back(&local_request);
            pthread_exit(0);
        case 4: // MODIFY
            //pthread_mutex_lock(&ddbb_mutex);
            local_request.answer = modify_value(local_request.key, local_request.value_1, local_request.N_value_2,
                                                local_request.value_2, local_request.value_3);
            //pthread_mutex_unlock(&ddbb_mutex);
            if (local_request.answer == -1) {
                printf("ERROR modifying key %d with modify_value()\n", local_request.key);
            }
            answer_back(&local_request);
            pthread_exit(0);

        case 5: // GET_VALUE
            //pthread_mutex_lock(&ddbb_mutex);
            local_request.answer = get_value(local_request.key, local_request.value_1, &local_request.N_value_2,
                                             local_request.value_2, &local_request.value_3);
            //pthread_mutex_unlock(&ddbb_mutex);
            if (local_request.answer == -1) {
                printf("ERROR obtaining key %d with get_value()\n", local_request.key);
            }
            answer_back(&local_request);
            pthread_exit(0);
        case 6: //EXIST
            local_request.answer = exist(local_request.key);
            if (local_request.answer == -1) {
                printf("ERROR verifying key %d because it doesnt exist\n", local_request.key);
            }
            answer_back(&local_request);
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
int create_table(sqlite3 *db) {
    char *message_error = NULL;
    //Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database_server);
        return -4;
    }
    char *new_table =
            "CREATE TABLE IF NOT EXISTS data("
            " data_key INTEGER PRIMARY KEY,"
            " value1 TEXT,"
            " x INTEGER,"
            " y INTEGER"
            ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK) {
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

    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "ERROR CREATING SECONDARY TABLE 2\n");
        sqlite3_close(database_server);
        return -4;
    }
    return 0;
}


/**
 *@brief Función implementada para hacer un cierre seguro del servidor cuando se pulsa CRTL + C
 */
void safe_close(int ctrlc) {
    printf("-----------------------------------------------\n");
    printf("\nEXIT SIGNAL RECEIVED. CLOSING ALL AND GOODBYE\n");
    printf("-----------------------------------------------\n");
    mq_close(server_queue);
    mq_unlink("/servidor_queue_9453");
    exit(0);
}


int main() {
    //Inicializo la signal para tratar el crtl c y realizar un cierre seguro de la aplicacion
    signal(SIGINT, safe_close);

    //Creando e inicializando la base de datos
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    int create_database = sqlite3_open("/tmp/database.db", &database_server);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        exit(-4);
    }
    //Creo la tabla principal "data" y la subtabla "value2_all"
    if (create_table(database_server) < 0) {
        exit(-4);
    }
    sqlite3_close(database_server);

    //LLeno de 0s el array de threads ocupados
    pad_array();


    //Inicializo la estructura de la cola de mensajes.
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Máximo 10 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;


    //inicializacion mutex para la copia local de parametros
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);
    //pthread_mutex_init(&ddbb_mutex, NULL);


    //Inicializo y abro la cola del servidor
    mqd_t server_queue;
    char *nombre = "/servidor_queue_9453";
    server_queue = mq_open(nombre, O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
    if (server_queue == -1) {
        mq_close(server_queue);
        fprintf(stderr, "Error opening server queue. Error code: %s\n", strerror(errno));
        exit(-1);
    }


    //Me creo la estructura request para manejar las peticiones
    request new_request;
    //Gestion de la concurrencia con las peticiones
    printf("SERVER ACTIVE. WAITING FOR REQUESTS...........\n");
    while (1) {
        //Esto intenta realizar un join no bloqueante
        for (int i = 0; i < MAX_THREADS; i++) {
            if (free_threads_array[i] == 1) {
                //Si el hilo ha estado trabajando miro a ver si puedo hacerle join
                if (pthread_tryjoin_np(thread_pool[i], NULL) == 0) {
                    free_threads_array[i] = 0; //Al ruedo de nuevo, maquina
                    workload--; // Avisar de que se puede currar
                    printf("Thread %d free and ready to reuse\n", i);
                }
            }
        }
        //Veo si hay mensajes
        ssize_t message = mq_receive(server_queue, (char *) &new_request, sizeof(request), 0);
        if (message >= 0) {
            printf("Message received with id %d\n", new_request.key);

            //Miro el primer hilo disponible y le mando currar.
            for (int i = 0; i < MAX_THREADS; i++) {
                if (free_threads_array[i] == 0) {
                    free_threads_array[i] = 1;
                    workload++;
                    printf("Thread %d is now working\n", i);

                    if (pthread_create(&thread_pool[i],NULL, (void *) process_request, &new_request) == 0) {
                        pthread_mutex_lock(&mutex_copy_params);
                        while (free_mutex_copy_params_cond == 0)
                            pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                        free_mutex_copy_params_cond = 0;
                        pthread_mutex_unlock(&mutex_copy_params);
                    }
                    break;
                }
                //En caso de que no se pueda trabajar porque no hay hilos disponibles, en vez de hacer un wait, como
                //el mensaje ha sido leido ya, lo reenvio a la cola
                if (workload == MAX_THREADS) {
                    message = mq_send(server_queue, (char *) &new_request, sizeof(request), 0);
                    if (message < 0) {
                        exit(-2);
                    }
                    printf("Back in queue again with id: %d\n", new_request.key);
                }
            }
        }
        //en caso de no recibir, como es un mq_receive no bloqueante, que no lo cuente como error.
        else if (errno == EAGAIN) {
        } else {
            //si se me lia el mensaje
            printf("Error receiving message %s\n", strerror(errno));
            exit(-2);
        }
    }
    return 0;
}
