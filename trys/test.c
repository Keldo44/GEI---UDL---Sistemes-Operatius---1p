/* -----------------------------------------------------------------------
 Controlador de prueba para verificar el generador.
 Codi font: test_controlador.c

 Nom complet autor1.
 Nom complet autor2.
 ---------------------------------------------------------------------- */

#include <stdio.h>     // sprintf (solo para formatear cadenas)
#include <sys/types.h> // fork, pid_t, wait
#include <unistd.h>    // pipe, read, write, close, execl
#include <stdlib.h>    // exit, atoi, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>    // strlen
#include <sys/wait.h>  // wait
#include <signal.h>    // signal, SIGTERM, SIGQUIT
#include <errno.h>     // errno

#include <fcntl.h>

#define MIDA_MAX_CADENA 1024
#define INVERTIR_COLOR "\e[7m"
#define FI_COLOR "\e[0m"
#define MIDA_MAX_CADENA_COLORS 1024
#define FORMAT_TEXT_ERROR "\e[1;48;5;1;38;5;255m"


typedef struct {
    int pid;
    int nombre;
    int es_primer;
} t_infoNombre;

void handle_sigquit(int sig);
void ImprimirInfoControlador(char *text);
void ImprimirError(char *text);

volatile sig_atomic_t signal_received = 0;
char capInfoControlador[MIDA_MAX_CADENA];

int gen_pid;

int main(int argc, char *argv[]) {
    int pipe_numeros[2]; // pipe_numeros[0] es para lectura, pipe_numeros[1] para escritura (10 WRITE, 11 READ)
    #define NUM_WRITE 10
    #define NUM_READ 11

    int pipe_respostes[2]; // pipe_respostes[0] es para lectura, pipe_respostes[1] es para escritura (20 WRITE, 21 READ)
    #define RES_WRITE 20
    #define RES_READ 21

    pid_t pid;

    // Nos aseguramos de que el usuario introduzca los argumentos correctamente
    if (argc != 3) {write(1, "Us: ", 4);
        write(1, argv[0], strlen(argv[0]));
        write(1, " <nombre processos calculadors> <nombre final>\n\n", 48);
        exit(2);
    } 

    // Los parseamos
    int numCalculadors = atoi(argv[1]);
    int numNombres = atoi(argv[2]);

    int pidsHijos[numCalculadors];

    //Señales que ignoramos
    signal(SIGQUIT, SIG_IGN); //Ignoramos el sig quit
    signal(SIGTERM, SIG_IGN); //Ignoramos el sig term
    signal(SIGINT, SIG_IGN); //Ignoramos el sig ter
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
        dup2(pipe_numeros[1], NUM_WRITE);

        // Ejecutar el generador
        execl("./generador", "generador", argv[1], NULL);

        // Si execl falla
        perror("Error en execl");
        exit(EXIT_FAILURE);

    } else {
        
        if (pipe(pipe_respostes) == -1) {
            ImprimirError("Error creating pipes");
        }

        // Crear procesos calculadores
        for (int i = 0; i < numCalculadors; i++) {
            // Generar un hijo por cada numCalculadors
            switch (pid = fork()) {
            case -1:
                ImprimirError("Error creando proceso calculador");
                break;
            case 0: /* Proceso hijo: Calculador */
                close(pipe_numeros[1]); // Cerrar escritura de numeros
                close(pipe_respostes[0]); // Cerrar lectura de respuestas
                dup2(pipe_numeros[0], NUM_READ); 
                dup2(pipe_respostes[1], RES_WRITE); 

                

                execl("./calculador", "./calculador", NULL);
                ImprimirError("Error execl calculador");
            default: /* Proceso padre */
                pidsHijos[i] = pid; // almacenar el PID del hijo
                break;
            }
        }
        //Espera a que acabe un hijo para proseguir (solo puede ser el generador)
        wait(NULL);

        // Enviar señal a cada hijo para que comiencen
        for (int i = 0; i < numCalculadors; i++) {
            kill(pidsHijos[i], SIGUSR1);
        }
        
        // Código del proceso padre (controlador de prueba)
        // Ignore SIGQUIT 
        gen_pid = pid;
        
        
        
        // Cerrar el extremo de escritura del pipe en el controlador
        close(pipe_numeros[1]);
        close(pipe_respostes[1]); // Cerrar escritura de respuestas en controlador


        // Leer las respuestas del pipe
        int fd_pipe_respuestas = 20; // Descriptor de escritura del pipe de respuestas
        t_infoNombre infoNombre;      // Estructura para recibir la información de resultados
        dup2(pipe_respostes[0], 20);
        // Crea el pipe (asume que ya se ha creado previamente)
        // int fd_pipe_nombres[2]; // Un ejemplo si necesitas crear un pipe aquí
        // pipe(fd_pipe_nombres); // Crear el pipe de nombres

        // Aquí iría la lógica para crear el proceso hijo, que ejecutaría el proceso calculador

        // Proceso padre: leer respuestas del pipe
        ssize_t bytes_leidos;

        // Bucle para leer estructuras del pipe de respuestas hasta recibir EOF
        while ((bytes_leidos = read(20, &infoNombre, sizeof(t_infoNombre))) > 0) {
            // Procesar la información recibida
            printf("Proceso PID: %d, Número: %d, Es primo: %s\n",
                infoNombre.pid,
                infoNombre.nombre,
                infoNombre.es_primer ? "Sí" : "No");
        }

        // Cerrar el descriptor de lectura del pipe de respuestas
        close(fd_pipe_respuestas);        


        printf("Controlador de prueba: Finalizado.\n");
    }

    return 0;
}

void handle_sigquit(int sig) {
    // ignore SIGquit
    //kill(gen_pid, SIGQUIT);
    //write(1, "Received SIGQUIT signal.\n", 25);
}

void ImprimirError(char *text) {
    write(2, FORMAT_TEXT_ERROR, strlen(FORMAT_TEXT_ERROR));
    write(2, capInfoControlador, strlen(capInfoControlador));
    write(2, text, strlen(text));

    // Obtener y escribir el mensaje de error basado en errno
    char *errorStr = strerror(errno);
    write(2, ": ", 2);
    write(2, errorStr, strlen(errorStr));
    write(2, FI_COLOR, strlen(FI_COLOR));
    write(2, "\n", 1);
    
    exit(EXIT_FAILURE);
}

void ImprimirInfoControlador(char *text) {
    write(1, INVERTIR_COLOR, strlen(INVERTIR_COLOR));
    write(1, capInfoControlador, strlen(capInfoControlador));
    write(1, text, strlen(text));
    write(1, FI_COLOR, strlen(FI_COLOR));
}