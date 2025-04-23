/* src/client-side/proxy-rpc.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves_rpc.h"
#include "claves.h"

/* ————————————————————————————————————————————— */
/* 1) Cliente RPC compartido                        */
static CLIENT *clnt = NULL;

/* 2) Inicializa clnt la primera vez que se necesite */
static void ensure_client(void) {
    if (clnt) return;  /* ya está hecho */

    const char *srv = getenv("IP_TUPLAS");
    if (!srv) {
        fprintf(stderr, "Error: definir IP_TUPLAS\n");
        exit(1);
    }

    clnt = clnt_create(srv, CLAVES_PROG, CLAVES_VERS, "udp");
    if (!clnt) {
        clnt_pcreateerror(srv);
        exit(1);
    }
}

/* 3) Destructor automático al cerrar el proceso */
__attribute__((destructor))
static void cleanup_rpc(void) {
    if (clnt) {
        clnt_destroy(clnt);
        clnt = NULL;
    }
}

int set_value(int key, char *value1, int N_value2,
                  double *V_value2, struct Coord value3)
{
    ensure_client();
    entry e = {0};
    e.key = key;
    e.value1 = strdup(value1);
    e.N_value2 = N_value2;
    e.V_value2.V_value2_len = N_value2;
    e.V_value2.V_value2_val = V_value2;
    e.value3.x = value3.x;
    e.value3.y = value3.y;

    int *r = set_value_1(&e, clnt);
    free(e.value1);
    if (!r) {
        clnt_perror(clnt, "Error RPC al insertar clave");
        return -1;
    }
    if (*r == 0) {
        printf("[SET] La clave %d ha sido insertada correctamente.\n", key);
    } else {
        printf("[SET] Error al insertar la clave %d (código %d).\n", key, *r);
    }
    return *r;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3)
{
    ensure_client();
    GetRes *r = get_value_1(&key, clnt);
    if (!r) {
        clnt_perror(clnt, "Error RPC al obtener clave");
        return -1;
    }
    if (r->status < 0) {
        printf("[GET] Error: no existe la clave %d o fallo en la consulta (código %d).\n",
               key, r->status);
        return r->status;
    }
    printf("[GET] Clave %d recuperada:\n", key);
    printf("      value1 = \"%s\"\n", r->value1);
    printf("      N2     = %d\n", r->N_value2);
    printf("      V2     = [");
    for (int i = 0; i < r->N_value2; i++) {
        printf("%s%g", i ? ", " : "", r->V_value2.V_value2_val[i]);
    }
    printf("]\n");
    printf("      coord  = (%d, %d)\n", r->value3.x, r->value3.y);
    return 0;
}

int exist(int key)
{
    ensure_client();
    int *r = exist_1(&key, clnt);
    if (!r) {
        clnt_perror(clnt, "Error RPC comprobando existencia");
        return -1;
    }
    if (*r == 1) {
        printf("[EXIST] La clave %d SÍ existe en la base de datos.\n", key);
    } else {
        printf("[EXIST] La clave %d NO existe en la base de datos.\n", key);
    }
    return *r;
}

int delete_key(int key)
{
    ensure_client();
    int *r = delete_key_1(&key, clnt);
    if (!r) {
        clnt_perror(clnt, "Error RPC al borrar clave");
        return -1;
    }
    if (*r == 0) {
        printf("[DELETE] La clave %d se ha borrado correctamente.\n", key);
    } else {
        printf("[DELETE] No se pudo borrar la clave %d (código %d).\n", key, *r);
    }
    return *r;
}

int modify_value(int key,
               char *value1,
               int N_value2,
               double *V_value2,
               struct Coord value3)
{
    ensure_client();
    entry e = {0};
    e.key                  = key;
    e.value1               = strdup(value1);
    e.N_value2             = N_value2;
    e.V_value2.V_value2_len = N_value2;
    e.V_value2.V_value2_val = V_value2;
    e.value3.x             = value3.x;
    e.value3.y             = value3.y;

    int *r = modify_value_1(&e, clnt);
    free(e.value1);
    if (!r) {
        clnt_perror(clnt, "Error RPC al modificar clave");
        return -1;
    }
    if (*r == 0) {
        printf("[MODIFY] La clave %d se ha modificado correctamente.\n", key);
    } else {
        printf("[MODIFY] Error al modificar la clave %d (código %d).\n", key, *r);
    }
    return *r;
}
/* DESTROY_SERVICE wrapper con la firma exacta de claves.h */
int destroy(void)
{
    ensure_client();
    /* RPC no toma argumentos para destroy */
    void *arg = NULL;
    int *r = destroy_service_1(arg, clnt);
    if (!r) {
        clnt_perror(clnt, "RPC destroy");
        return -1;
    }
    return *r;
}
