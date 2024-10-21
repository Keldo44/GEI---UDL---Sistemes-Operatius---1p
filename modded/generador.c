#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE_WRITE 10  // Descriptor de escritura del pipe

// Manejador de señales
void manejador_senal(int senyal) {
    if (senyal == SIGUSR1) {
        printf("Generador: Recibida señal SIGUSR1. Comenzando a generar números.\n");
    } else if (senyal == SIGTERM) {
        printf("Generador: Recibida señal SIGTERM. Terminando el proceso.\n");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <M>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int M = atoi(argv[1]);  // Número hasta el cual generar la secuencia de números
    int fd_write;

    // Establecer manejadores de señales
    signal(SIGUSR1, manejador_senal);  // Usamos SIGUSR1 en lugar de SIGQUIT
    signal(SIGTERM, manejador_senal);

    // Abrir el descriptor de escritura de la pipe (previamente creada por el controlador)
    fd_write = PIPE_WRITE;  // Descriptor 10 asignado en la práctica

    // Esperar indefinidamente hasta recibir SIGUSR1
    pause();

    // Escribir los números de 2 a M en el pipe
    for (int i = 2; i <= M; i++) {
        write(fd_write, &i, sizeof(int));  // Escribir cada número en el pipe
        printf("Generador: Número %d enviado.\n", i);
    }

    // Cerrar el descriptor de escritura
    close(fd_write);
    printf("Generador: Finalizada la generación de números. Esperando SIGTERM...\n");

    // Esperar indefinidamente la señal SIGTERM para finalizar
    while (1) {
        pause();
    }

    return 0;
}
