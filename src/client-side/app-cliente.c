
/*
#include<stdio.h>
#include<string.h>
#include "../claves.h"

int main (int argc, char **argv)
{
    int key = 5;
    char *v1 = "ejemplo de valor 1";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3;
    v3.x = 10;
    v3.y = 5;
    int err = set_value(key, v1, 3, v2, v3);
    if (err == -1) {
        printf("Error al insertar la tupla\n");
    }
    return 0;

}
*/
#include <stdio.h>
#include <string.h>
#include "../claves.h"

int main() {
    int key = 6;
    char *v1 = "valor inicial";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3;
    v3.x = 10;
    v3.y = 5;

    // Variables para get_value()
    char v1_obtenido[256];
    int N_value2;
    double v2_obtenido[32];
    struct Coord v3_obtenido;
    printf("\n🔹 PRUEBA 2: Obtener tupla\n");
    if (get_value(key, v1_obtenido, &N_value2, v2_obtenido, &v3_obtenido) == 0) {
        printf("✅ Tupla obtenida: v1='%s', N_value2=%d, Coord=(%d, %d)\n",
                v1_obtenido, N_value2, v3_obtenido.x, v3_obtenido.y);
    } else {
        printf("❌ Error al obtener la tupla\n");
    }



    return 0;
}

