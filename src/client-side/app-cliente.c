/* src/client-side/app-cliente-inf.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"       /* prototipos de wrappers */
#include "struct.h"       /* struct Coord */

/*
 * Requiere:
 *   export IP_TUPLAS=servidor
 * Y haber enlazado contra libclaves.so que implementa ensure_client.
 */

int main(void) {
    /* Comprobamos que el proxy podrá leer IP_TUPLAS */
    if (!getenv("IP_TUPLAS")) {
        fprintf(stderr, "Error: definir IP_TUPLAS\n");
        return 1;
    }

    int key = 5;
    char v1[256] = "valor inicial";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3 = { .x = 10, .y = 5 };

    printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla (key=77)\n");
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    /* PRUEBA 2: get_value */
    char v1_obt[256];
    int N2;
    double v2_obt[32];
    struct Coord v3_obt;
    printf("\n🔹 PRUEBA 2: Obtener tupla (key=5)\n");
    if (get_value(key, v1_obt, &N2, v2_obt, &v3_obt) == 0) {
        printf("✅ Tupla obtenida: v1='%s' N2=%d V2={", v1_obt, N2);
        for (int i = 0; i < N2; i++) {
            printf("%g%s", v2_obt[i], i+1 < N2 ? ", " : "");
        }
        printf("} Coord=(%d,%d)\n", v3_obt.x, v3_obt.y);
    } else {
        printf("❌ Error al obtener la tupla\n");
    }

    /* PRUEBA 3: modify_value */
    char *nv1 = "valor modificado";
    double nv2[] = {9.9, 8.8, 7.7};
    struct Coord nv3 = { .x = 20, .y = 15 };
    printf("\n🔹 PRUEBA 3: Modificar tupla (key=5)\n");
    if (modify_value(key, nv1, 3, nv2, nv3) == 0) {
        printf("✅ Tupla modificada correctamente\n");
    } else {
        printf("❌ Error al modificar la tupla\n");
    }

    /* PRUEBA 4: delete_key */
    printf("\n🔹 PRUEBA 4: Eliminar tupla (key=5)\n");
    if (delete_key(key) == 0) {
        printf("✅ Tupla eliminada correctamente\n");
    } else {
        printf("❌ Error al eliminar la tupla\n");
    }

    /* PRUEBA 5: exist */
    printf("\n🔹 PRUEBA 5: Verificar existencia (key=77)\n");
    int ex = exist(77);
    if (ex == 1) {
        printf("✅ Tupla con key=77 EXISTE\n");
    } else if (ex == 0) {
        printf("❌ Tupla con key=77 NO EXISTE\n");
    } else {
        printf("❌ Error comprobando existencia (código %d)\n", ex);
    }

    /* PRUEBA 6: destroy */
    printf("\n🔹 PRUEBA 6: Limpiar TODAS las tuplas\n");
    if (destroy() == 0) {
        printf("✅ Base de datos limpiada correctamente\n");
    } else {
        printf("❌ Error al limpiar la base de datos\n");
    }

    return 0;
}
