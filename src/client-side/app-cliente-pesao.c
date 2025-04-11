#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>     // Para INT_MAX
#include "claves.h"

int main() {
    // Abre /dev/urandom para leer bytes "aleatorios" sin depender de time()
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo /dev/urandom");
        return 1;
    }

    unsigned int tmpKey1, tmpKey2;

    // Lee 4 bytes para cada clave
    if (read(fd, &tmpKey1, sizeof(tmpKey1)) == -1) {
        perror("Error leyendo tmpKey1");
        close(fd);
        return 1;
    }
    if (read(fd, &tmpKey2, sizeof(tmpKey2)) == -1) {
        perror("Error leyendo tmpKey2");
        close(fd);
        return 1;
    }

    close(fd);

    // Forzamos que estén en el rango 1..INT_MAX
    int key1 = (int) (tmpKey1 % INT_MAX) + 1;
    int key2 = (int) (tmpKey2 % INT_MAX) + 1;

    char v1[256] = "valor inicial";
    double v2[] = {2.3, 0.5, 23.45};
    struct Coord v3;
    v3.x = 10;
    v3.y = 5;

    printf("\n🔹 PRUEBA 1: Insertar tupla con key=%d\n", key1);
    if (set_value(key1, v1, 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }

    printf("\n🔹 PRUEBA 2: Insertar tupla con key=%d\n", key2);
    if (set_value(key2, "No me borras", 3, v2, v3) == 0) {
        printf("✅ Tupla insertada correctamente\n");
    } else {
        printf("❌ Error al insertar tupla\n");
    }
    return 0;
}
