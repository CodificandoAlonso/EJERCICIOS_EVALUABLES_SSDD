#include <iso646.h>
#include<stdio.h>
#include<pthread.h>
#include "../claves.h"
#include<mqueue.h>
#include<stdlib.h>
#include "../claves.h"
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



void process_request(request *this_request) {
    pthread_mutex_lock(&mutex);
    request local_request = *this_request;
    free_mutex_cond = 1;
    pthread_mutex_unlock(&mutex);
}



int main(int argc, char *argv[]) {
    char nombre[20] = "/servidor_queue_9453";

    pthread_t thread_array[MAX_THREADS];
    int free_threads_array[MAX_THREADS];
    for(int i =0; i< MAX_THREADS; i++) {
        free_threads_array[i] = 0;
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_wait, NULL);

    mqd_t server_queue;
    server_queue = mq_open(nombre, O_CREAT|O_WRONLY, 0700);
    if (server_queue == -1) {
        mq_close(server_queue);
        printf("Error\n");
        return -1;
    }
    printf("Todo bien %d\n", server_queue);

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