#ifndef HEADERS_H
#define HEADERS_H

#include "claves.h"



/**
 * @brief
 *Estructura que se usará para la cola de mensajes
 *
 *
 */
typedef struct request {
    int type;
    int key;
    char value_1[256];
    int N_value_2;
    double value_2[32];
    struct Coord value_3;
    int answer;
    char client_queue[32];
}request;



/**
 * @brief
 *Estructura que se usará para pasar los parámetros a los hilos
 *
 *
 */
typedef struct parameters_to_pass_threads
{
    request* this_request;
    int identifier;
} parameters_to_pass;


typedef struct receive_sql {
    char value_1[256];
    int N_values;
    double value_2[32];
    struct Coord value3;
}receive_sql;






#endif