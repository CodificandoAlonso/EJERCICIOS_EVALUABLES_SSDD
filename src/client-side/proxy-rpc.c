/* src/client-side/proxy-rpc.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves_rpc.h"
#include "claves.h"

static CLIENT *clnt;
int set_value_rpc(int key, const char *value1, int N_value2,
                  double *V_value2, int x, int y)
{
    entry e = {0};
    e.key = key;
    e.value1 = strdup(value1);
    e.N_value2 = N_value2;
    e.V_value2.V_value2_len = N_value2;
    e.V_value2.V_value2_val = V_value2;
    e.value3.x = x;
    e.value3.y = y;

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

int get_value_rpc(int key)
{
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

int exist_rpc(int key)
{
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

int delete_rpc(int key)
{
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

int modify_rpc(int key,
               const char *value1,
               int N_value2,
               double *V_value2,
               int x,
               int y)
{
    entry e = {0};
    e.key                  = key;
    e.value1               = strdup(value1);
    e.N_value2             = N_value2;
    e.V_value2.V_value2_len = N_value2;
    e.V_value2.V_value2_val = V_value2;
    e.value3.x             = x;
    e.value3.y             = y;

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


int main(void)
{
    const char *srv = getenv("IP_TUPLAS");
    if (!srv) {
        fprintf(stderr, "Error: definir IP_TUPLAS\n");
        return 1;
    }
    clnt = clnt_create(srv, CLAVES_PROG, CLAVES_VERS, "udp");
    if (!clnt) {
        clnt_pcreateerror(srv);
        return 1;
    }

    char line[512];
    while (1) {
        fputs("rpc> ", stdout);
        if (!fgets(line, sizeof line, stdin)) break;
        char *cmd = strtok(line, " \t\n");
        if (!cmd) continue;
        if (strcasecmp(cmd, "quit")==0 || strcasecmp(cmd, "exit")==0) break;

        if (strcasecmp(cmd, "set")==0) {
            char *ks = strtok(NULL, " \t\n");
            char *v1 = strtok(NULL, " \t\n");
            char *n2s = strtok(NULL, " \t\n");
            if (!ks || !v1 || !n2s) {
                printf("Uso: set <clave> <value1> <N2> <v2_1>...<v2_N2> <x> <y>\n");
                continue;
            }

            int key = atoi(ks);
            /* 1) Compruebo si ya existe */
            int existe = exist_rpc(key);
            if (existe > 0) {
                printf("[SET] Error: la clave %d ya existe, no se puede crear una nueva tupla con esa clave primaria.\n", key);
                continue;
            }
            if (existe < 0) {
                /* hubo un error al comprobar */
                continue;
            }

            int N2 = atoi(n2s);
            double V2[32] = {0};
            for (int i = 0; i < N2; i++) {
                char *vs = strtok(NULL, " \t\n");
                if (!vs) break;
                V2[i] = atof(vs);
            }
            char *xs = strtok(NULL, " \t\n");
            char *ys = strtok(NULL, " \t\n");
            if (!xs || !ys) {
                printf("Uso: set <clave> <value1> <N2> <v2_1>...<v2_N2> <x> <y>\n");
                continue;
            }
            int x = atoi(xs), y = atoi(ys);
            set_value_rpc(key, v1, N2, V2, x, y);
        }
        else if (strcasecmp(cmd, "get")==0) {
            char *ks = strtok(NULL, " \t\n");
            if (!ks) { printf("Uso: get <clave>\n"); continue; }
            get_value_rpc(atoi(ks));
        }
        else if (strcasecmp(cmd, "exist")==0) {
            char *ks = strtok(NULL, " \t\n");
            if (!ks) { printf("Uso: exist <clave>\n"); continue; }
            exist_rpc(atoi(ks));
        }
        else if (strcasecmp(cmd, "delete")==0) {
            char *ks = strtok(NULL, " \t\n");
            if (!ks) { printf("Uso: delete <clave>\n"); continue; }
            delete_rpc(atoi(ks));
        }
        else if (strcasecmp(cmd, "modify") == 0) {
            /* Mismo parseo que set */
            char *ks = strtok(NULL, " \t\n");
            char *v1 = strtok(NULL, " \t\n");
            char *n2s = strtok(NULL, " \t\n");
            if (!ks || !v1 || !n2s) {
                printf("Uso: modify <clave> <value1> <N2> <v2_1>...<v2_N2> <x> <y>\n");
                continue;
            }
            int key = atoi(ks), N2 = atoi(n2s);
            double V2[32] = {0};
            for (int i = 0; i < N2; i++) {
                char *vs = strtok(NULL, " \t\n");
                if (!vs) break;
                V2[i] = atof(vs);
            }
            char *xs = strtok(NULL, " \t\n");
            char *ys = strtok(NULL, " \t\n");
            if (!xs || !ys) {
                printf("Uso: modify <clave> <value1> <N2> <v2_1>...<v2_N2> <x> <y>\n");
                continue;
            }
            int x = atoi(xs), y = atoi(ys);
            modify_rpc(key, v1, N2, V2, x, y);
        }
        else {
            printf("Comando desconocido: %s\n", cmd);
        }
    }

    clnt_destroy(clnt);
    return 0;
}