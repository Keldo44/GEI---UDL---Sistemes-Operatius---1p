#include <stdio.h>     // sprintf (solo para formatear cadenas)
#include <sys/types.h> // fork, pid_t, wait
#include <unistd.h>    // pipe, read, write, close, execl
#include <stdlib.h>    // exit, atoi, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>    // strlen
#include <sys/wait.h>  // wait
#include <signal.h>    // signal, SIGTERM, SIGQUIT
#include <errno.h>     // errno

/*
---------------------------------------------------------------------- 
--- C O N S T A N T S ----------------------------------------------- 
---------------------------------------------------------------------- 
*/
#define MIDA_MAX_CADENA 1024
#define INVERTIR_COLOR "\e[7m"
#define FI_COLOR "\e[0m"
#define MIDA_MAX_CADENA_COLORS 1024
#define FORMAT_TEXT_ERROR "\e[1;48;5;1;38;5;255m"

typedef struct {
    int pid;
    int nombre;
    int esPrimer;
} t_infoNombre;

/*
---------------------------------------------------------------------- 
--- F U N C I O N S   H E A D E R S ---------------------------------- 
---------------------------------------------------------------------- 
*/
void handle_sigquit(int sig);
void ImprimirInfoControlador(char *text);
void ImprimirError(char *text);

/*
---------------------------------------------------------------------- 
--- G L O B A L   V A R I A B L E S ---------------------------------- 
---------------------------------------------------------------------- 
*/

volatile sig_atomic_t signal_received = 0;
char capInfoControlador[MIDA_MAX_CADENA];
int pipeRespostes[2]; // pipes para comunicación

/*
---------------------------------------------------------------------- 
--- M A I N --------------------------------------------------------- 
---------------------------------------------------------------------- 
*/
int main(int argc, char *argv[])
{
    signal(SIGQUIT, handle_sigquit);

    unsigned short int numCalculadors, numFinal;
    unsigned char i;
    pid_t pid, pidsHijos[100]; 
    int estatWait;
    char cadena[MIDA_MAX_CADENA];

    if (argc != 3) {
        write(1, "Us: ", 4);
        write(1, argv[0], strlen(argv[0]));
        write(1, " <nombre processos calculadors> <nombre final>\n\n", 48);
        exit(2);
    }

    sprintf(capInfoControlador, "[%s-pid:%u]> ", argv[0], getpid());
    numCalculadors = atoi(argv[1]);
    numFinal = atoi(argv[2]);

    int PIPE_NOMBRES_WRITE[2];

    // Crear pipes
    if (pipe(PIPE_NOMBRES_WRITE) == -1 || pipe(pipeRespostes) == -1) {
        ImprimirError("Error creating pipes");
    }

    // Mensaje inicial
    ImprimirInfoControlador("* * * * * * * * * *  I N I C I  * * * * * * * * * *\n");
    write(1, "Processos calculadors: ", 23);
    sprintf(cadena, "%u\n", numCalculadors);
    write(1, cadena, strlen(cadena));

    // Crear proceso generador
    if ((pid = fork()) == -1) {
        ImprimirError("Error creando proceso generador");
    } else if (pid == 0) {
        // Proceso hijo: Generador
        close(PIPE_NOMBRES_WRITE[0]); // cerrar lectura en generador
        dup2(PIPE_NOMBRES_WRITE[1], 10);
        execl("./generador", "./generador", argv[2], NULL);
        ImprimirError("Error execl generador");
    } else if (pid != 0) {
        // Proceso padre: Esperar a que terminen los hijos
        wait(0);
        write(1, "Proceso gen terminó\n", strlen("Proceso gen terminó\n"));
    }

    // Crear procesos calculadores
    for (i = 0; i < numCalculadors; i++) {
        switch (pid = fork()) {
        case -1:
            ImprimirError("Error creando proceso calculador");
            break;
        case 0: /* Proceso hijo: Calculador */
            close(PIPE_NOMBRES_WRITE[1]); // Cerrar escritura
            close(pipeRespostes[0]); // Cerrar lectura
            dup2(PIPE_NOMBRES_WRITE[0], 11); // asignar descriptor 11 para lectura
            dup2(pipeRespostes[1], 20); // asignar descriptor 20 para escritura
            execl("./calculador", "./calculador", NULL);
            ImprimirError("Error execl calculador");
        default: /* Proceso padre */
            pidsHijos[i] = pid; // almacenar el PID del hijo
            break;
        }
    }

    // Proceso padre: Controlador
    close(PIPE_NOMBRES_WRITE[1]); // Cerrar escritura de nombres en controlador
    close(pipeRespostes[1]); // Cerrar escritura de respostes en controlador

    // Leer del pipe de respostes
    t_infoNombre infoNombre;
    int primersTotal = 0;
    while (read(pipeRespostes[0], &infoNombre, sizeof(t_infoNombre)) > 0) {
        sprintf(cadena, "Calculador (PID: %d) - Nombre: %d - %s\n",
                infoNombre.pid, infoNombre.nombre,
                infoNombre.esPrimer ? "Es primer" : "No es primer");
        write(1, cadena, strlen(cadena));
        if (infoNombre.esPrimer) {
            primersTotal++;
        }
    }

    // Enviar SIGTERM a los procesos hijos
    for (i = 0; i < numCalculadors; i++) {
        kill(pidsHijos[i], SIGTERM); // enviar SIGTERM al PID del hijo
        pid = wait(&estatWait); // Esperar a que terminen los hijos
    }

    // Mensaje final
    sprintf(cadena, "Total nombres primers: %d\n", primersTotal);
    write(1, cadena, strlen(cadena));
    ImprimirInfoControlador("* * * * * * * * * *  F I  * * * * * * * * * *\n");

    exit(EXIT_SUCCESS);
}

/*
---------------------------------------------------------------------- 
--- F U N C I O N S ------------------------------------------------- 
---------------------------------------------------------------------- 
*/
void handle_sigquit(int sig) {
    // Handle the SIGQUIT signal
    //signal_received = 1;
    //write(1, "Received SIGQUIT signal.\n", 25);
}
void ImprimirInfoControlador(char *text) {
    write(1, INVERTIR_COLOR, strlen(INVERTIR_COLOR));
    write(1, capInfoControlador, strlen(capInfoControlador));
    write(1, text, strlen(text));
    write(1, FI_COLOR, strlen(FI_COLOR));
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
