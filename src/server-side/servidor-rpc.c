/* src/server-side/servidor-rpc.c */

#include <stdio.h>
#include <stdlib.h>
#include "claves.h"
#include "treat_sql.h"
#include "claves_rpc.h"
#include <pwd.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "claves.h"



char *get_username() {
    struct passwd *pw = getpwuid(getuid());
    if (pw) return pw->pw_name;
    return "default";
}


static void init_database(void) {
    sqlite3 *db;
    int rc;
    char *errmsg = NULL;
    char db_name[256];
    char *user = get_username();
    snprintf(db_name, sizeof(db_name), "/tmp/database-%s.db", user);

    rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB %s: %s\n", db_name, sqlite3_errmsg(db));
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
bool_t
set_value_1_svc(entry *e, int *resultp, struct svc_req *rq)
{
    (void)rq;
    struct Coord c3;
    c3.x = e->value3.x;
    c3.y = e->value3.y;
    *resultp = set_value(
        e->key,
        e->value1,
        e->N_value2,
        e->V_value2.V_value2_val,
        c3
    );
    return TRUE;
}

/* GET_VALUE */
bool_t
get_value_1_svc(int *keyp, GetRes *resultp, struct svc_req *rq)
{
    (void)rq;
    char *buf1 = malloc(256 * sizeof(char));
    double *buf2 = malloc(sizeof(double) *32);
    static RpcCoord rc;
    int status;

    /* Llamamos a tu lógica y llenamos buf1, buf2 y rc */
    status = get_value(*keyp, buf1, &resultp->N_value2, buf2, (struct Coord *)&rc);

    /* Montamos la respuesta estática */
    resultp->status                    = status;
    resultp->value1                    = buf1;                 // apunta a buf1
    resultp->V_value2.V_value2_len     = resultp->N_value2;
    resultp->V_value2.V_value2_val     = buf2;                 // apunta a buf2
    resultp->value3.x                  = rc.x;
    resultp->value3.y                  = rc.y;
    return TRUE;
}


/* DELETE_KEY */
bool_t
delete_key_1_svc(int *k, int *resultp, struct svc_req *rq)
{
    (void)rq;
    *resultp = delete_key(*k);
    return TRUE;
}

/* MODIFY_VALUE */
bool_t
modify_value_1_svc(entry *e,int *resultp, struct svc_req *rq)
{
    (void)rq;
    *resultp = modify_value(
        e->key,
        e->value1,
        e->N_value2,
        e->V_value2.V_value2_val,
        *(struct Coord *)&e->value3
    );
    return TRUE;
}

/* EXIST */
bool_t
exist_1_svc(int *k, int *resultp, struct svc_req *rq)
{
    (void)rq;             /* evitamos “unused parameter” */

    /* llamamos a la función que comprueba la BBDD */
    *resultp = exist(*k);

    return TRUE;
}

/* INIT_SERVICE */
bool_t
init_service_1_svc(void *arg, int *resultp, struct svc_req *rq)
{
    (void)arg; (void)rq;
    *resultp = destroy();  /* reusa destroy() para limpiar */
    return TRUE;
}

/* DESTROY_SERVICE */
bool_t
destroy_service_1_svc(void *arg, int *resultp, struct svc_req *rq)
{
    (void)arg; (void)rq;
    *resultp = destroy();
    return TRUE;
}

int
main(void)
{
    init_database();
    register SVCXPRT *transp;

    pmap_unset(CLAVES_PROG, CLAVES_VERS);

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (!transp) {
        fprintf(stderr, "No puedo crear servicio TCP\n");
        exit(1);
    }

    if (!svc_register(transp, CLAVES_PROG, CLAVES_VERS,
                      claves_prog_1, IPPROTO_TCP)) {
        fprintf(stderr, "No puedo registrar CLAVES_PROG\n");
        exit(1);
    }

    svc_run();  /* bucle de servicio RPC */
    fprintf(stderr, "svc_run retornó\n");
    return 1;
}
