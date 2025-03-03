//
// Created by hector on 3/03/25.
//
#include <mqueue.h>
#include <stdio.h>

int main() {
    const char *queue_name = "/servidor_queue_9453";

    if (mq_unlink(queue_name) == 0) {
        printf("Cola eliminada correctamente: %s\n", queue_name);
    } else {
        perror("Error al eliminar la cola");
    }

    return 0;
}
