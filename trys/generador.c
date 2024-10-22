/* -----------------------------------------------------------------------
 PRA1. Processos, pipes i senyals: Primers
 Codi font: generador.c

 Nom complet autor1.
 Nom complet autor2.
 ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define PIPE_WRITE 10 // Descriptor d'escriptura del pipe

int M; // Límite superior de la secuencia de números

// Manejador para la señal SIGQUIT (Ctrl+4)
void sigquit_handler(int signum) {
    // Escribir los números de 2 a M en el pipe
    for (int i = 2; i <= M; i++) {
        if (write(PIPE_WRITE, &i, sizeof(int)) == -1) {
            perror("Error escribiendo en el pipe");
            close(PIPE_WRITE);
            exit(EXIT_FAILURE);
        }
    }

    // Cerrar el descriptor de escritura del pipe
    close(PIPE_WRITE);
    printf("Generador: Números de 2 a %d enviados por el pipe.\n", M);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <M>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    M = atoi(argv[1]); // Leer el valor de M desde los argumentos

    // Configurar el manejador de señal para SIGQUIT
    struct sigaction sa;
    sa.sa_handler = sigquit_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("Error al capturar SIGQUIT");
        exit(EXIT_FAILURE);
    }

    // Esperar indefinidamente señales
    printf("Generador: Esperando SIGQUIT para enviar números de 2 a %d...\n", M);
    while (1) {
        pause(); // Esperar señales
    }

    return 0;
}
