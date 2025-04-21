/* src/server-side/claves.c */

#include "claves.h"
#include "struct.h"
#include "treat_sql.h"

#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Mutex global para serializar accesos a la base de datos */
pthread_mutex_t ddbb_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Crea las tablas si no existen.
 */
static int ensure_schema(sqlite3 *db) {
    const char *schema =
        "CREATE TABLE IF NOT EXISTS data ("
        "  data_key INTEGER PRIMARY KEY,"
        "  value1   TEXT    NOT NULL,"
        "  x        INTEGER NOT NULL,"
        "  y        INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS value2_all ("
        "  id           TEXT PRIMARY KEY,"
        "  data_key_fk  INTEGER NOT NULL,"
        "  value        REAL,"
        "  FOREIGN KEY(data_key_fk) REFERENCES data(data_key) ON DELETE CASCADE"
        ");";
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, schema, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creando esquema: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}

/**
 * @brief Inicializa (o limpia) la base de datos completa.
 */
int destroy(void)
{
    sqlite3 *db;
    char path[256];

    snprintf(path, sizeof(path), "/tmp/database-%s.db", getlogin());
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        fprintf(stderr, "destroy: error abriendo DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    pthread_mutex_lock(&ddbb_mutex);
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM value2_all;",   NULL, NULL, NULL);
    sqlite3_exec(db, "DELETE FROM data;",         NULL, NULL, NULL);
    pthread_mutex_unlock(&ddbb_mutex);

    sqlite3_close(db);
    return 0;
}

/**
 * @brief Inserta o reemplaza un elemento completo <key, value1, V2[], coord>.
 */
int set_value(int key, char *value1, int N_value2,
              double *V_value2, struct Coord value3)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char path[256];
    int rc;

    snprintf(path, sizeof(path), "/tmp/database-%s.db", getlogin());
    if ((rc = sqlite3_open(path, &db)) != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (ensure_schema(db) < 0) {
        sqlite3_close(db);
        return -1;
    }

    sqlite3_exec(db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

    /* Debug */
    printf("[DEBUG][set_value] key=%d, value1='%s', x=%d, y=%d\n",
           key, value1, value3.x, value3.y);

    /* 1) Tabla data */
    const char *sql1 =
        "INSERT OR REPLACE INTO data(data_key, value1, x, y) "
        "VALUES(?1, ?2, ?3, ?4);";
    rc = sqlite3_prepare_v2(db, sql1, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto err;
    sqlite3_bind_int   (stmt, 1, key);
    sqlite3_bind_text  (stmt, 2, value1, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int   (stmt, 3, value3.x);
    sqlite3_bind_int   (stmt, 4, value3.y);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) goto err;

    /* 2) Tabla value2_all */
    if (N_value2 > 32) N_value2 = 32;
    const char *sql2 =
        "INSERT OR REPLACE INTO value2_all(id, data_key_fk, value) "
        "VALUES(?1, ?2, ?3);";
    for (int i = 0; i < N_value2; i++) {
        char id[32];
        snprintf(id, sizeof(id), "%d_%d", key, i);
        rc = sqlite3_prepare_v2(db, sql2, -1, &stmt, NULL);
        if (rc != SQLITE_OK) goto err;
        sqlite3_bind_text  (stmt, 1, id, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int   (stmt, 2, key);
        sqlite3_bind_double(stmt, 3, V_value2[i]);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) goto err;
    }

    sqlite3_close(db);
    return 0;

err:
    fprintf(stderr, "Error en set_value: %s\n", sqlite3_errmsg(db));
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return -1;
}

/**
 * @brief Recupera todos los campos de la tupla con clave `key`.
 */
int get_value(int key, char *value1, int *N_value2,
              double *V_value2, struct Coord *value3)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char path[256];
    int rc;

    snprintf(path, sizeof(path), "/tmp/database-%s.db", getlogin());
    if ((rc = sqlite3_open(path, &db)) != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (ensure_schema(db) < 0) {
        sqlite3_close(db);
        return -1;
    }

    /* 1) Leer data */
    const char *sql1 =
        "SELECT value1, x, y FROM data WHERE data_key = ?1;";
    rc = sqlite3_prepare_v2(db, sql1, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto err;
    sqlite3_bind_int(stmt, 1, key);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }
    /* Copiar value1 y coord */
    const unsigned char *text = sqlite3_column_text(stmt, 0);
    strncpy(value1, (const char*)text, 255);
    value1[255] = '\0';
    value3->x = sqlite3_column_int(stmt, 1);
    value3->y = sqlite3_column_int(stmt, 2);

    /* Debug */
    printf("[DEBUG][get_value] key=%d -> value1='%s', x=%d, y=%d\n",
           key, value1, value3->x, value3->y);

    sqlite3_finalize(stmt);

    /* 2) Leer vector */
    *N_value2 = 0;
    const char *sql2 =
        "SELECT value FROM value2_all WHERE data_key_fk = ?1 ORDER BY id;";
    rc = sqlite3_prepare_v2(db, sql2, -1, &stmt, NULL);
    if (rc != SQLITE_OK) goto err;
    sqlite3_bind_int(stmt, 1, key);
    while (sqlite3_step(stmt) == SQLITE_ROW && *N_value2 < 32) {
        V_value2[*N_value2] = sqlite3_column_double(stmt, 0);
        (*N_value2)++;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;

err:
    fprintf(stderr, "Error en get_value: %s\n", sqlite3_errmsg(db));
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return -1;
}

/**
 * @brief Modifica un elemento: borra y vuelve a insertar.
 */
int modify_value(int key, char *value1, int N_value2,
                 double *V_value2, struct Coord value3)
{
    if (delete_key(key) < 0) return -1;
    return set_value(key, value1, N_value2, V_value2, value3);
}

/**
 * @brief Borra un elemento completo.
 */
int delete_key(int key)
{
    sqlite3 *db;
    char path[256];
    char *errmsg = NULL;
    int rc;

    snprintf(path, sizeof(path), "/tmp/database-%s.db", getlogin());
    if ((rc = sqlite3_open(path, &db)) != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    if (ensure_schema(db) < 0) {
        sqlite3_close(db);
        return -1;
    }
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;", NULL, NULL, NULL);

    char sql[64];
    snprintf(sql, sizeof(sql),
             "DELETE FROM data WHERE data_key = %d;", key);
    rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error borrando clave %d: %s\n", key, errmsg);
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return -1;
    }
    sqlite3_close(db);
    return 0;
}

/**
 * @brief Comprueba existencia usando SELECT EXISTS.
 */
int exist(int key)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char path[256];
    int rc, found = 0;

    snprintf(path, sizeof(path), "/tmp/database-%s.db", getlogin());
    if ((rc = sqlite3_open(path, &db)) != SQLITE_OK) {
        fprintf(stderr, "Error abriendo DB: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    if (ensure_schema(db) < 0) {
        sqlite3_close(db);
        return -1;
    }

    const char *sql =
        "SELECT EXISTS(SELECT 1 FROM data WHERE data_key = ?1);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, key);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        found = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return found;
}
