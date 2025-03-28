#ifndef STRUCT_H
#define STRUCT_H

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
}request;



/**
 * @brief
 *Estructura que se usará para pasar los parámetros a los hilos
 *
 *
 */
typedef struct parameters_to_pass_threads
{
    int identifier;
} parameters_to_pass;


typedef struct receive_sql {
    char value_1[256];
    int N_values;
    double value_2[32];
    struct Coord value3;
    int empty;;
}receive_sql;






#endif