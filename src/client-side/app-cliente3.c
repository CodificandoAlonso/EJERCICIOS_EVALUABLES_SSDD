#include <stdio.h>
#include <string.h>
#include "claves.h"

int main() {
    int key = 7;
    char v1[256] = "valor inicial";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3;
    v3.x = 10;
    v3.y = 5;

    printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla\n");
    if (set_value(79, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }


    // Variables para get_value()
    char v1_obtenido[256];
    int N_value2;
    double v2_obtenido[32];
    struct Coord v3_obtenido;
    printf("\n🔹 PRUEBA 2: Obtener tupla\n");
    if (get_value(key, v1_obtenido, &N_value2, v2_obtenido, &v3_obtenido) == 0) {
        printf("✅ Tupla obtenida: v1= '%s' ",v1_obtenido);
        printf("v2= {");
        for(int i = 0; i< N_value2; i++){
            if (i == N_value2-1){
                printf("%lf}, ", v2_obtenido[i]);
                break;
            }
            printf("%lf, ", v2_obtenido[i]);
        }
        printf("Coord= (%d, %d)\n",v3_obtenido.x, v3_obtenido.y);
    } else {
        printf("❌ Error al obtener la tupla\n");
    }

    // Modificar la tupla
    char *nuevo_v1 = "valor modificado";
    double nuevo_v2[] = {9.9, 8.8, 7.7};
    struct Coord nuevo_v3 = {20, 15};

    printf("\n🔹 PRUEBA 3: Modificar tupla\n");
    if (modify_value(key, nuevo_v1, 3, nuevo_v2, nuevo_v3) == 0) {
        printf("✅ Tupla modificada correctamente\n");
    } else {
        printf("❌ Error al modificar la tupla\n");
    }

    printf("\n🔹 PRUEBA 4: Eliminar tupla con delete_key()\n");
    if (delete_key(key) == 0) {
        printf("✅ Tupla eliminada correctamente\n");
    } else {
        printf("❌ Error al eliminar la tupla\n");
    }

    printf("\n🔹 PRUEBA 5: Eliminar TODAS las tuplas con destroy()\n");
    if (destroy() == 0) {
        printf("✅ Base de datos limpiada correctamente\n");
    } else {
        printf("❌ Error al limpiar la base de datos\n");
    }


    return 0;
}