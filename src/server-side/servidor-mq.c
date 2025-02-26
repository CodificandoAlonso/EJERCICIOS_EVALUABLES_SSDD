#include <iso646.h>
#include<stdio.h>
#include<pthread.h>
#include "../claves.h"
#include<mqueue.h>
#include<stdlib.h>
#include "../claves.h"
#include<sqlite3.h>
#define MAX_THREADS 10








int free_mutex_cond = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_wait;

//LA COLA ACABA CON 9453

typedef struct value_2 {
    int num_elem;
    double *array_doubles;
}value_2;



typedef struct request {
    int type;
    int key;
    char *value_1;
    value_2 value_2;
    struct Coord value_3;
}request;

typedef struct parameters_to_pass{
  request *this_request;
  int identifier;


}parameters_to_pass;



void process_request(parameters_to_pass *parameters) {
    pthread_mutex_lock(&mutex);
    request local_request = *parameters->this_request;
    int local_id = parameters->identifier;
    free_mutex_cond = 1;
    pthread_mutex_unlock(&mutex);
}



void create_table(sqlite3 *db){
    char *message_error = NULL;
    char *new_table =
        "CREATE TABLE IF NOT EXISTS data("
        " data_key INTEGER PRIMARY KEY,"
        " value1 TEXT,"
        " x INTEGER,"
        " y INTEGER"
        ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK){
      fprintf(stderr, "ERROR CREANDO TABLA 1\n");
      exit(-4);
    }
    message_error = NULL;
    new_table =
        "CREATE TABLE IF NOT EXISTS value2_all("
        " id TEXT PRIMARY KEY,"
        " data_key INTEGER,"
        " value REAL,"
        "CONSTRAINT fk_origin FOREIGN KEY(data_key) REFERENCES data(pk) ON DELETE CASCADE ON UPDATE CASCADE"
        ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK){
        fprintf(stderr, "ERROR CREANDO TABLA 2\n");
        exit(-4);
    }
}


void insert_data_TEST(sqlite3 *db){
    char *error_message = NULL;
    char insert[256];

    //Insertar los primeros parametros en data
    sprintf(insert,
        "INSERT into data(data_key, value1,x,y) "
        " VALUES(%d, '%s', %d ,%d)",3, "SEXO",5, 5);
    printf("Esto vale insert: %s\n", insert);
    int test;
    if ((test =sqlite3_exec(db, insert, NULL, NULL, &error_message)) != SQLITE_OK){
        if (test != SQLITE_CONSTRAINT){
        fprintf(stderr, "ERROR insertando en TABLA\n");
        exit(-4);
        }
        else{
          printf("DUPLICADA LA FK BOBO\n");
        }
    }

    //insertar value2
    double var2[3] = {3.44, 4.3, 5.5};
    char primary_key[20];
    for(int i=0; i<3; i++){
        sprintf(primary_key, "%d%d", 3, i);
        sprintf(insert,
        "INSERT into value2_all(id,data_key,value) "
        " VALUES(%s, %d, %f)",primary_key, 3, var2[i]);
        printf("Esto vale insert: %s\n", insert);
        if ((test =sqlite3_exec(db, insert, NULL, NULL, &error_message)) != SQLITE_OK){
            if (test != SQLITE_CONSTRAINT){
                fprintf(stderr, "ERROR insertando en TABLA\n");
                exit(-4);
            }
            else{
                printf("DUPLICADA LA FK BOBO\n");
            }
        }
    }
}


int main(int argc, char *argv[]) {

    //Creando e inicializando la base de datos
    sqlite3 *database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK){
        fprintf(stderr,"Error opening the database\n");
        exit(-1);
    }
    printf("Exito abriendo la base de datos\n");

    //Creo la tabla principal "data"
    create_table(database);
    insert_data_TEST(database);




    //Creación de hilos, arrays de hilos, y mutex y avriables condicion
    pthread_t thread_array[MAX_THREADS];
    int free_threads_array[MAX_THREADS];
    for(int i =0; i< MAX_THREADS; i++) {
        free_threads_array[i] = 0;
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_wait, NULL);


    //Inicializo y abro la cola del servidor
    mqd_t server_queue;
    char nombre[20] = "/servidor_queue_9453";
    server_queue = mq_open(nombre, O_CREAT|O_WRONLY, 0700);
    if (server_queue == -1) {
        mq_close(server_queue);
        printf("Error abriendo la cola del servidor\n");
        return -1;
    }
    printf("Todo bien abriendo la cola del servidor con fd: %d\n", server_queue);


    //Gestion de la concurrencia con las peticiones
    while(1) {
        request new_request;
        if (mq_receive(server_queue,(char *) &new_request, sizeof(new_request),0 ) >= 0) {

        }
        else {
           printf("error al recibir, tonto\n");
            //DE MOMENTO SI ES ERROR EN COMUNICACION ES ERROR -2
            exit(-2);
        }
        pthread_mutex_lock(&mutex);
        while(free_mutex_cond == 0)
            pthread_cond_wait(&cond_wait, &mutex);
        free_mutex_cond = 0;
        pthread_mutex_unlock(&mutex);

    }
}