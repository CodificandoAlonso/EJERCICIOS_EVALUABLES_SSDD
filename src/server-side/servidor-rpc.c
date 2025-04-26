/* src/server-side/servidor-rpc.c */

#include <stdio.h>
#include <stdlib.h>
#include "claves.h"
#include "treat_sql.h"
#include "claves_rpc.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "claves.h"
#include "claves_rpc.h"

static void init_database(void) {
    sqlite3 *db;
    int rc;
    char *errmsg = NULL;
    const char *path = "/tmp/database.db";   /* coincide con el path que usa tu claves.c */

    rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB %s: %s\n", path, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    /* activar foreign keys (si alguna tabla tiene ON DELETE CASCADE) */
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    /* crea las tablas si no existen */
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS data ("
          "data_key INTEGER PRIMARY KEY,"
          "value1   TEXT NOT NULL,"
          "x        INTEGER NOT NULL,"
          "y        INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS value2_all ("
          "id           TEXT PRIMARY KEY,"
          "data_key_fk  INTEGER NOT NULL,"
          "value        REAL,"
          "FOREIGN KEY(data_key_fk) REFERENCES data(data_key) ON DELETE CASCADE"
        ");",
        NULL, NULL, &errmsg);
    if (errmsg) {
        fprintf(stderr, "Error creando esquema: %s\n", errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_close(db);
}


void claves_prog_1(struct svc_req *rqstp, register SVCXPRT *transp);

/* SET_VALUE */
int *
set_value_1_svc(entry *e, struct svc_req *rq)
{
    (void)rq;
    static int result;
    struct Coord c3;
    c3.x = e->value3.x;
    c3.y = e->value3.y;
    result = set_value(
        e->key,
        e->value1,
        e->N_value2,
        e->V_value2.V_value2_val,
        c3
    );
    return &result;
}

/* GET_VALUE */
GetRes *
get_value_1_svc(int *keyp, struct svc_req *rq)
{
    (void)rq;
    static GetRes  result;
    static char    buf1[256];
    static double  buf2[32];
    static RpcCoord rc;
    int status;

    /* Llamamos a tu lógica y llenamos buf1, buf2 y rc */
    status = get_value(*keyp, buf1, &result.N_value2, buf2, (struct Coord *)&rc);

    /* Montamos la respuesta estática */
    result.status                    = status;
    result.value1                    = buf1;                 // apunta a buf1
    result.V_value2.V_value2_len     = result.N_value2;
    result.V_value2.V_value2_val     = buf2;                 // apunta a buf2
    result.value3.x                  = rc.x;
    result.value3.y                  = rc.y;

    return &result;
}


/* DELETE_KEY */
int *
delete_key_1_svc(int *k, struct svc_req *rq)
{
    (void)rq;
    static int result;
    result = delete_key(*k);
    return &result;
}

/* MODIFY_VALUE */
int *
modify_value_1_svc(entry *e, struct svc_req *rq)
{
    (void)rq;
    static int res;
    /* e->value3 es RpcCoord, same layout que struct Coord */
    res = modify_value(
        e->key,
        e->value1,
        e->N_value2,
        e->V_value2.V_value2_val,
        *(struct Coord *)&e->value3
    );
    return &res;
}

/* EXIST */
int *
exist_1_svc(int *k, struct svc_req *rq)
{
    (void)rq;             /* evitamos “unused parameter” */
    static int result;    /* vive en data, no en stack */

    /* llamamos a la función que comprueba la BBDD */
    result = exist(*k);

    return &result;
}

/* INIT_SERVICE */
int *
init_service_1_svc(void *arg, struct svc_req *rq)
{
    (void)arg; (void)rq;
    static int result;
    result = destroy();  /* reusa destroy() para limpiar */
    return &result;
}

/* DESTROY_SERVICE */
int *
destroy_service_1_svc(void *arg, struct svc_req *rq)
{
    (void)arg; (void)rq;
    static int result;
    result = destroy();
    return &result;
}

int
main(void)
{
    init_database();
    register SVCXPRT *transp;

    pmap_unset(CLAVES_PROG, CLAVES_VERS);

    transp = svcudp_create(RPC_ANYSOCK);
    if (!transp) {
        fprintf(stderr, "No puedo crear servicio UDP\n");
        exit(1);
    }

    if (!svc_register(transp, CLAVES_PROG, CLAVES_VERS,
                      claves_prog_1, IPPROTO_UDP)) {
        fprintf(stderr, "No puedo registrar CLAVES_PROG\n");
        exit(1);
    }

    svc_run();  /* bucle de servicio RPC */
    fprintf(stderr, "svc_run retornó\n");
    return 1;
}
