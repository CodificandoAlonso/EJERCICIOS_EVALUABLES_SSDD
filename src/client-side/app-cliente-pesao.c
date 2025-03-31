//
// Created by hectorpc on 1/04/25.
//
#include <stdio.h>
#include <string.h>
#include "claves.h"

int main() {
    int key = 5;
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
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla\n");
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }
    printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla\n");
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }
    printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla\n");
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }
    printf("\n🔹 PRUEBA 1: Insertar tupla\n");
    if (set_value(key, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 1.5: Insertar tupla\n");
    if (set_value(77, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

}