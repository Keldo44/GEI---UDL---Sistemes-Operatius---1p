/* -----------------------------------------------------------------------
 Controlador de prueba para verificar el generador.
 Codi font: test_controlador.c

 Nom complet autor1.
 Nom complet autor2.
 ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PIPE_READ 11 // Descriptor de lectura del pipe
#define PIPE_WRITE 10 // Descriptor de escritura del pipe

int main(int argc, char *argv[]) {
    int pipe_numeros[2]; // pipe_numeros[0] es para lectura, pipe_numeros[1] para escritura
    pid_t pid;
    int M;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <M>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    M = atoi(argv[1]); // Obtener el valor de M

    // Crear el pipe
    if (pipe(pipe_numeros) == -1) {
        perror("Error creando el pipe");
        exit(EXIT_FAILURE);
    }

    // Crear el proceso hijo (el generador)
    pid = fork();
    if (pid == -1) {
        perror("Error en fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Código del proceso hijo (generador)

        // Cerrar el extremo de lectura del pipe en el generador
        close(pipe_numeros[0]);

        // Duplicar el descriptor de escritura al descriptor 10
        dup2(pipe_numeros[1], PIPE_WRITE);

        // Ejecutar el generador
        execl("./generador", "generador", argv[1], NULL);

        // Si execl falla
        perror("Error en execl");
        exit(EXIT_FAILURE);
    } else {
        // Código del proceso padre (controlador de prueba)

        // Cerrar el extremo de escritura del pipe en el controlador
        close(pipe_numeros[1]);

        // Esperar un poco antes de enviar la señal (para asegurarse de que el generador está listo)
        sleep(1);

        // Enviar la señal SIGQUIT al proceso hijo (generador)
        kill(pid, SIGQUIT);

        // Leer los números del pipe
        int numero;
        printf("Controlador de prueba: Leyendo números del pipe...\n");
        while (read(pipe_numeros[0], &numero, sizeof(int)) > 0) {
            printf("Número recibido: %d\n", numero);
        }

        // Esperar que el proceso hijo termine
        wait(NULL);

        // Cerrar el descriptor de lectura del pipe
        close(pipe_numeros[0]);

        printf("Controlador de prueba: Finalizado.\n");
    }

    return 0;
}
