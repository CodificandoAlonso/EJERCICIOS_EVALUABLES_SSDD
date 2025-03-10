#include "../claves.h"
#include<stdio.h>
#include<sqlite3.h>
#include <stdlib.h>
#include <string.h>


/**
 * @brief Esta llamada permite inicializar el servicio de elementos clave-valor1-valor2-valor3.
 * Mediante este servicio se destruyen todas las tuplas que estuvieran almacenadas previamente.
 *
 * @return int La función devuelve 0 en caso de éxito y -1 en caso de error.
 * @retval 0 en caso de exito.
 * @retval -1 en caso de error.
 */
int destroy() {
    sqlite3 *database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }

    char *delete_data_table = "DELETE from data";
    char *delete_value2_table = "DELETE from value2_all";

    if (sqlite3_exec(database, delete_data_table, NULL, NULL, NULL) != SQLITE_OK ||
        sqlite3_exec(database, delete_value2_table, NULL, NULL, NULL) != SQLITE_OK) {
        fprintf(stderr, "ERROR borrando las tablas\n");
        sqlite3_close(database);
        return -1;
        }

    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio inserta el elemento <key, value1, value2, value3>.
 * El vector correspondiente al valor 2 vendrá dado por la dimensión del vector (N_Value2) y
 * el vector en si (V_value2).
 * El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * Se considera error, intentar insertar una clave que ya existe previamente o
 * que el valor N_value2 esté fuera de rango. En este caso se devolverá -1 y no se insertará.
 * También se considerará error cualquier error en las comunicaciones.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32].
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 si se insertó con éxito.
 * @retval -1 en caso de error.
 */
int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    sqlite3 *database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        exit(-1);
    }
    char *error_message = NULL;
    value1[strcspn(value1, "\r\n")] = 0;
    char insert[256];
    char local_value1[256];
    memcpy(local_value1, value1, sizeof(value1));
    printf("Esto vale value1 %s\n", local_value1);

    //Insertar los primeros parametros en data
    sprintf(insert,
            "INSERT into data(data_key, value1,x,y) "
            " VALUES(%d, '%s', %d ,%d);", key, value1, value3.x, value3.y);
    printf("Esto vale insert: %s\n", insert);
    int test;
    if ((test = sqlite3_exec(database, insert, NULL, NULL, &error_message)) != SQLITE_OK) {
        if (test != SQLITE_CONSTRAINT) {
            fprintf(stderr, "ERROR insertando en TABLA\n");
            exit(-4);
        }
        fprintf(stderr,"DUPLICADA LA PK BOBO\n");
        return -1;
    }
    if (N_value2 >32) {
        fprintf(stderr, "Too many arguments in value2\n");
        return -1;
    }
    char primary_key[20];

    for (int i = 0; i < N_value2; i++) {
        sprintf(primary_key, "%d%d", key, i);
        sprintf(insert,
                "INSERT into value2_all(id, data_key_fk, value) "
                " VALUES(%s, %d, %f);", primary_key, key, V_value2[i]);
        printf("Esto vale insert: %s\n", insert);
        if ((test = sqlite3_exec(database, insert, NULL, NULL, &error_message)) != SQLITE_OK) {
            if (test != SQLITE_CONSTRAINT) {
                fprintf(stderr, "ERROR insertando en TABLA\n");
                exit(-4);
            }
            fprintf(stderr,"DUPLICADA LA PK BOBO\n");
            return -1;
        }
    }

    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite obtener los valores asociados a la clave key.
 * La cadena de caracteres asociada se devuelve en value1.
 * En N_Value2 se devuelve la dimensión del vector asociado al valor 2 y en V_value2 las componentes del vector.
 * Tanto value1 como V_value2 tienen que tener espacio reservado para poder almacenar el máximo número
 * de elementos posibles (256 en el caso de la cadena de caracteres y 32 en el caso del vector de doubles).
 * La función devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo,
 * si no existe un elemento con dicha clave o si se produce un error de comunicaciones.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32] por referencia.
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord por referencia.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 en caso de éxito.
 * @retval -1 en caso de error.
 */
int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    sqlite3 *database;
    sqlite3_stmt *stmt;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char query[128];
    strcpy(query, "SELECT value1, x, y FROM data WHERE data_key = ?");
    if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(database);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, key);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *text = (const char*)sqlite3_column_text(stmt, 0);
        if (text){
            printf("🔍 Valor leído de BD: %s\n", text ? text : "NULL");

            strncpy(value1, text, 255);
            value1[255] = '\0';  // Evitar problemas de memoria
        } else {
            value1[0] = '\0';  // Si el valor es NULL, dejarlo vacío
        }

        value3->x = sqlite3_column_int(stmt, 1);
        value3->y = sqlite3_column_int(stmt, 2);
    } else {
        sqlite3_finalize(stmt);
        sqlite3_close(database);
        return -1;
    }
    sqlite3_finalize(stmt);

    query[0] = '\0';
    snprintf(query, sizeof(query), "SELECT value FROM value2_all WHERE data_key_fk = ? ORDER BY id");
    if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(database);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, key);

    *N_value2 = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *N_value2 < 32) {
        V_value2[*N_value2] = sqlite3_column_double(stmt, 0);
        (*N_value2)++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite modificar los valores asociados a la clave key.
 * La función devuelve 0 en caso de éxito y -1 en caso de error, por ejemplo,
 * si no existe un elemento con dicha clave o si se produce un error en las comunicaciones.
 * También se devolverá -1 si el valor N_value2 está fuera de rango.
 *
 *
 * @param key clave.
 * @param value1   valor1 [256].
 * @param N_value2 dimensión del vector V_value2 [1-32].
 * @param V_value2 vector de doubles [32].
 * @param value3   estructura Coord.
 * @return int El servicio devuelve 0 si se insertó con éxito y -1 en caso de error.
 * @retval 0 si se modificó con éxito.
 * @retval -1 en caso de error.
 */


int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (N_value2 > 32) return -1;

    sqlite3 *database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }
    char query[128];
    strcpy (query, "UPDATE data SET value1 = ?, x = ?, y = ? WHERE data_key = ?");
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(database);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, value1, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, value3.x);
    sqlite3_bind_int(stmt, 3, value3.y);
    sqlite3_bind_int(stmt, 4, key);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(database);
        return -1;
    }
    sqlite3_finalize(stmt);

    query[0] = '\0';
    snprintf(query, sizeof(query), "DELETE FROM value2_all WHERE data_key_fk = ?");
    if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(database);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, key);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    query[0] = '\0';
    snprintf(query, sizeof(query), "INSERT INTO value2_all (id, data_key_fk, value) VALUES (?, ?, ?)");
    for (int i = 0; i < N_value2; i++) {
        if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) != SQLITE_OK) {
            sqlite3_close(database);
            return -1;
        }
        char primary_key[20];
        sprintf(primary_key, "%d%d", key, i);
        sqlite3_bind_text(stmt, 1, primary_key, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, key);
        sqlite3_bind_double(stmt, 3, V_value2[i]);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    sqlite3_close(database);
    return 0;
}


/**
 * @brief Este servicio permite borrar el elemento cuya clave es key.
 * La función devuelve 0 en caso de éxito y -1 en caso de error.
 * En caso de que la clave no exista también se devuelve -1.
 *
 * @param key clave.
 * @return int La función devuelve 0 en caso de éxito y -1 en caso de error.
 * @retval 0 en caso de éxito.
 * @retval -1 en caso de error.
 */
int delete_key(int key) {
    sqlite3 *database;
    sqlite3_stmt *stmt;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        return -1;
    }

    char* message_error = NULL;
    // Habilitar las foreign keys para mejor manejo de la base de datos
    if (sqlite3_exec(database, "PRAGMA foreign_keys = ON;", NULL, NULL, &message_error) != SQLITE_OK) {
        fprintf(stderr, "Error with the fk definition %s", message_error);
        sqlite3_close(database);
        return -3;
    }

    // Nueva consulta preparada
    char delete_query[] = "DELETE FROM data WHERE data_key = ?";
    sqlite3_busy_timeout(database, 5000);  // Espera hasta 5 segundos antes de fallar

    if (sqlite3_prepare_v2(database, delete_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparando la consulta DELETE\n");
        sqlite3_close(database);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, key);  // Enlazar el valor de la clave a eliminar

    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        fprintf(stderr, "ERROR eliminando la clave %d: %s\n", key, sqlite3_errmsg(database));
        sqlite3_finalize(stmt);
        sqlite3_close(database);
        return -1;
    }

    printf("Clave %d eliminada correctamente\n", key);

    sqlite3_finalize(stmt);
    sqlite3_close(database);
    return 0;
}

/**
 * @brief Este servicio permite determinar si existe un elemento con clave key.
 * La función devuelve 1 en caso de que exista y 0 en caso de que no exista.
 * En caso de error se devuelve -1. Un error puede ocurrir en este caso por un problema en las comunicaciones.
 *
 * @param key clave.
 * @return int La función devuelve 1 en caso de que exista y 0 en caso de que no exista. En caso de error se devuelve -1.
 * @retval 1 en caso de que exista.
 * @retval 0 en caso de que no exista.
 * @retval -1 en caso de error.
 */
int exist(int key) {
    sqlite3 *database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK) {
        fprintf(stderr, "Error opening the database\n");
        exit(-1);
    }
    return 0;
}
