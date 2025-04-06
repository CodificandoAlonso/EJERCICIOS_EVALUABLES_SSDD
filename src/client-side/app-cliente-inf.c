//
// Created by hector on 3/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include "claves.h"

/*
typedef struct value_2 {
    int num_elem;
    double *array_doubles;
} value_2;
*/


int main() {
    char buffer[256];
    printf("WELCOME CLIENT");
    while (1) {
        printf("\n--------------------------------------------------------------------\n");
        printf("PLEASE, TYPE 'EXIT' TO LOG OUT OR "
            "'SET_VALUE', 'DESTROY', 'DELETE_KEY', 'MODIFY_VALUE' OR 'GET_VALUE'\n");
        printf("--------------------------------------------------------------------\n");
        fgets(buffer, sizeof(buffer),stdin);
        if (strcmp(buffer, "EXIT\n") == 0) {
            break;
        }
        if (strcmp(buffer, "SET_VALUE\n") == 0) {
            printf("DIME TU KEY\n");
            fgets(buffer, sizeof(buffer),stdin);
            int key = 0;
            if ((key = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            printf("DIME TU VALUE_1\n");
            fgets(buffer, sizeof(buffer),stdin);
            char value1[256];
            //nueva_peticion.value_1 = malloc(sizeof(buffer) * sizeof(char));
            memcpy(value1, &buffer, sizeof(buffer));
            printf("DIME LA LONGITUD DE TU VALUE_2\n");
            fgets(buffer, sizeof(buffer),stdin);
            int n2 = 0;
            if ((n2 = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            if (n2 > 32) {
                printf("Te pasaste de numero\n");
                return -1;
            }
            double v2[32];
            double insert_data = 0;
            for (int i = 0; i < n2; i++) {
                printf("DIME tu elemento %d del value2\n", i);
                fgets(buffer, sizeof(buffer),stdin);
                if ((sscanf(buffer, "%lf", &insert_data)) == 0) {
                    printf("Era un double. Mi programa se muere\n");
                    return -1;
                }
                v2[i] = insert_data;
            }

            struct Coord temp_coord = {0};
            printf("DIME tu value x\n");
            fgets(buffer, sizeof(buffer),stdin);
            int x, y = 0;
            if ((x = atoi(buffer)) == 0) {
                printf("Era un int. Mi programa se muere\n");
                return -1;
            }
            printf("DIME tu value y\n");
            fgets(buffer, sizeof(buffer),stdin);
            if ((y = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            temp_coord.x = x;
            temp_coord.y = y;
            if (set_value(key, value1, n2, v2, temp_coord) == 0) {
                printf("✅ Tupla insertada correctamente\n");
            } else {
                printf("❌ Error al insertar tupla\n");
                return -1;
            }
        } else if (strcmp(buffer, "DESTROY\n") == 0) {
            if (destroy() == 0) {
                printf("✅ Base de datos limpiada correctamente\n");
            } else {
                printf("❌ Error al limpiar la base de datos\n");
            }
        } else if (strcmp(buffer, "DELETE_KEY\n") == 0) {
            printf("Dime que key quieres eliminar:\n");
            fgets(buffer, sizeof(buffer), stdin);
            int key;
            if ((key = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            if (delete_key(key) == 0) {
                printf("✅ Tupla eliminada correctamente\n");
            } else {
                printf("❌ Error al eliminar la tupla\n");
                return -1;
            }
        } else if (strcmp(buffer, "MODIFY_VALUE\n") == 0) {
            printf("DIME TU KEY\n");
            fgets(buffer, sizeof(buffer),stdin);
            int key = 0;
            if ((key = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            printf("DIME TU VALUE_1\n");
            fgets(buffer, sizeof(buffer),stdin);
            char value1[256];
            memcpy(value1, &buffer, sizeof(buffer));
            printf("DIME LA LONGITUD DE TU VALUE_2\n");
            fgets(buffer, sizeof(buffer),stdin);
            int n2 = 0;
            if ((n2 = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            if (n2 > 32) {
                printf("Te pasaste de numero\n");
                return -1;
            }
            double v2[32];
            double insert_data = 0;
            for (int i = 0; i < n2; i++) {
                printf("DIME tu elemento %d del value2\n", i);
                fgets(buffer, sizeof(buffer),stdin);
                if ((sscanf(buffer, "%lf", &insert_data)) == 0) {
                    printf("Era un double. Mi programa se muere\n");
                    return -1;
                }
                v2[i] = insert_data;
            }

            struct Coord temp_coord = {0};
            printf("DIME tu value x\n");
            fgets(buffer, sizeof(buffer),stdin);
            int x, y = 0;
            if ((x = atoi(buffer)) == 0) {
                printf("Era un int. Mi programa se muere\n");
                return -1;
            }
            printf("DIME tu value y\n");
            fgets(buffer, sizeof(buffer),stdin);
            if ((y = atoi(buffer)) == 0) {
                printf("Era un entero tonto. Mi programa se muere\n");
                return -1;
            }
            temp_coord.x = x;
            temp_coord.y = y;
            if (modify_value(key, value1, n2, v2, temp_coord) == 0) {
                printf("✅ Tupla modificada correctamente\n");
            } else {
                printf("❌ Error al modificar tupla\n");
                return -1;
            }
        } else if (strcmp(buffer, "GET_VALUE\n") == 0) {
            printf("Dime de que key quieres obtener datos:\n");
            fgets(buffer, sizeof(buffer), stdin);
            int key;
            if ((key = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            char v1_obtenido[256];
            int N_value2;
            double v2_obtenido[32];
            struct Coord v3_obtenido;
            printf("\n🔹 PRUEBA 2: Obtener tupla\n");
            if (get_value(key, v1_obtenido, &N_value2, v2_obtenido, &v3_obtenido) == 0) {
                printf("✅ Tupla obtenida: v1= '%s' ", v1_obtenido);
                printf("v2= {");
                for (int i = 0; i < N_value2; i++) {
                    if (i == N_value2 - 1) {
                        printf("%lf}, ", v2_obtenido[i]);  //Para formatear bien la salida
                        break;
                    }
                    printf("%lf, ", v2_obtenido[i]);
                }
                printf("Coord= (%d, %d)\n", v3_obtenido.x, v3_obtenido.y);
            } else {
                printf("❌ Error al obtener la tupla\n");
            }
        } else if (strcmp(buffer, "EXIST\n") == 0) //EXIST == 6
        {
            printf("Dime que key quieres ver si existe:\n");
            fgets(buffer, sizeof(buffer), stdin);
            int key;
            if ((key = atoi(buffer)) == 0) {
                printf("Era un entero. Mi programa se muere\n");
                return -1;
            }
            if (exist(key) == 0) {
                printf("✅ Key %d existe\n", key);
            } else {
                printf("❌ Error viendo si key existe\n");
                return -1;
            }
        } else {
            printf("Bobi escribe bien plis\n");
        }
    }
    return 0;
}




