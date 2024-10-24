#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE_WRITE 10  // Updated descriptor de escritura del pipe

// Manejador de señales
void manejador_senal(int senyal) {
    if (senyal == SIGQUIT) {
        printf("Generador: Recibida señal SIGQUIT. Comenzando a generar números.\n");
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
    signal(SIGQUIT, manejador_senal);  // Usamos SIGQUIT para iniciar la generación
    signal(SIGTERM, manejador_senal);

    // Abrir el descriptor de escritura de la pipe (previamente creada por el controlador)
    fd_write = PIPE_WRITE;  // Descriptor 11 asignado ahora

    // Esperar indefinidamente hasta recibir SIGQUIT
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
