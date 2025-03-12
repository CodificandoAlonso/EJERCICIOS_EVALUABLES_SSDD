#include <stdio.h>
#include "claves.h"

int main() {
    int key = 5;
    char *v1 = "valor inicial";
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
    printf("\n🔹 PRUEBA 2: Insertar tupla\n");
    if (set_value(44, "sexo", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }
    printf("\n🔹 PRUEBA 3: Insertar tupla\n");
    if (set_value(33, "unai putero", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }



    return 0;
}