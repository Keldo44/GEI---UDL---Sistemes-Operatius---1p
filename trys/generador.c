/* -----------------------------------------------------------------------
 PRA1. Processos, pipes i senyals: Primers
 Codi font: generador.c
 
 Iván Ortiga Aguilar   
 ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

int pipe_nombres_fd = 10;  // Descriptor de escritura del pipe
int max_nombres = 0; // M nombres
int recived_quit_sigs = 0; // Recived quit signals
char buffer[100];

// Manejador de señal para SIGQUIT (Ctrl + 4)
void sigquit_handler(int sig) {
    recived_quit_sigs = 1;  // Marcar que se recibió SIGQUIT

    int i, M = max_nombres;

    // Leer el número M desde el pipe (descriptor 11)
    int pipe_lectura_fd = 11;
    

    // Escribir mensaje indicando que se recibió SIGQUIT
    snprintf(buffer, sizeof(buffer), "Generador: Señal SIGQUIT recibida. Enviando números al pipe...\n");
    write(1, buffer, strlen(buffer));  // Escribir en stdout

    // Escribir los números de 2 a M en el pipe (descriptor 10)
    // print M
    snprintf(buffer, sizeof(buffer), "Generador: Enviando números de 2 a %d...\n", M);
    write(1, buffer, strlen(buffer));  // Escribir en stdout
    for (i = 2; i <= M; i++) {
        // Escribir cada número en el pipe
        snprintf(buffer, sizeof(buffer), "Generador: Número %d enviado al pipe.\n", i);  
        
        // Escribir el número en el stdout
        write(1, buffer, strlen(buffer));
        write(pipe_nombres_fd, &i, sizeof(int));
    }

    // Cerrar el descriptor de escritura del pipe
    close(pipe_nombres_fd);

    // Escribir mensaje indicando que se terminaron de enviar los números
    snprintf(buffer, sizeof(buffer), "Generador: Todos los números de 2 a %d han sido enviados al pipe.\n", M);
    write(1, buffer, strlen(buffer));  // Escribir en stdout
}



int main(int argc, char *argv[]) {
    max_nombres = atoi(argv[1]);

    // Configurar manejadores de señales
    while (recived_quit_sigs == 0){
        // Escribir mensaje indicando que el proceso está esperando SIGQUIT
        snprintf(buffer, sizeof(buffer), "Generador: Esperando señal SIGQUIT...\n");
        write(1, buffer, strlen(buffer));  // Escribir en stdout
        signal(SIGQUIT, sigquit_handler);  // SIGQUIT para iniciar el envío de números
        pause();
    }
    exit(0);
    return 0;
}
