#include <stdio.h> /* sprintf*/
#include <sys/types.h> // fork, pid_t, wait, ..
#include <unistd.h>
#include <stdlib.h> /* exit, EXIT_SUCCESS, ...*/
#include <string.h> /* strlen */
#include <sys/wait.h> /* wait */
#include <errno.h>    /* errno */

/*
---------------------------------------------------------------------- 
--- C O N S T A N T S------ ------------------------------------------ 
---------------------------------------------------------------------- 
*/
#define MIDA_MAX_CADENA 1024
#define INVERTIR_COLOR "\e[7m"
#define FI_COLOR "\e[0m"
#define MIDA_MAX_CADENA_COLORS 1024
#define FORMAT_TEXT_ERROR "\e[1;48;5;1;38;5;255m"

/*
---------------------------------------------------------------------- 
--- C A P Ç A L E R E S   D E   F U N C T I O N S -------------------- 
---------------------------------------------------------------------- 
*/
void ImprimirInfoControlador(char *text);
void ImprimirError(char *text);

/*
---------------------------------------------------------------------- 
--- V A R I A B L E S   G L O B A L S -------------------------------- 
---------------------------------------------------------------------- 
*/
char capInfoControlador[MIDA_MAX_CADENA];

int main(int argc, char *argv[]) {
    unsigned short int numCalculadors;
    unsigned char i;
    pid_t pid;
    int estatWait;
    char cadena[MIDA_MAX_CADENA];

    // Definim el path del calculador
    char *args[] = {"./calculador", "./calculador", NULL};

    // Pipes per a la comunicació
    int pipes[numCalculadors][2];  // Pipe per a cada calculador (fd[0] lectura, fd[1] escriptura)

    if (argc != 2) {
        sprintf(cadena, "Us: %s <nombre processos calculadors>\n\nPer exemple: %s 3\n\n", argv[0], argv[0]);
        write(1, cadena, strlen(cadena));
        exit(2);
    }

    sprintf(capInfoControlador, "[%s-pid:%u]> ", argv[0], getpid());
    numCalculadors = atoi(argv[1]);

    ImprimirInfoControlador("* * * * * * * * * *  I N I C I  * * * * * * * * * *\n");
    sprintf(cadena, "Processos calculadors: %u.\n\n", numCalculadors);
    ImprimirInfoControlador(cadena);

    // Creem els pipes i forquem processos
    for (i = 0; i < numCalculadors; i++) {
        if (pipe(pipes[i]) == -1) {
            ImprimirError("Error creant pipe");
        }

        switch (pid = fork()) {
        case -1:
            sprintf(cadena, "ERROR creacio fill %u.", i + 1);
            ImprimirError(cadena);

        case 0: /* Fill */
            // El fill llegeix des del pipe
            close(pipes[i][1]);  // Tanca l'extrem d'escriptura

            // Passar l'extrem de lectura com a argument al calculador
            char pipe_read_fd_str[10];
            sprintf(pipe_read_fd_str, "%d", pipes[i][0]);  // File descriptor de lectura

            execl(args[0], args[1], pipe_read_fd_str, NULL);  // Passar el FD al calculador

            sprintf(cadena, "Error execl fill %u.", i + 1);
            ImprimirError(cadena);

        default:
            // El pare escriu al pipe
            close(pipes[i][0]);  // Tanca l'extrem de lectura

            sprintf(cadena, "Activacio calculador %u (pid: %u)\n", i + 1, pid);
            ImprimirInfoControlador(cadena);

            int nombre = i + 2;  // Nombre per comprovar si és primer
            write(pipes[i][1], &nombre, sizeof(int));  // Enviar nombre al calculador

            close(pipes[i][1]);  // Tancar l'extrem d'escriptura un cop enviat
        }
    }

    // Esperem la finalització dels calculadors
    for (i = 0; i < numCalculadors; i++) {
        pid = wait(&estatWait);
        sprintf(cadena, "Rebuda finalitzacio calculador (pid-%u) codi:%d \n", pid, WEXITSTATUS(estatWait));
        ImprimirInfoControlador(cadena);
    }

    ImprimirInfoControlador("* * * * * * * * * *  F I  * * * * * * * * * *\n");

    exit(EXIT_SUCCESS);
}

void ImprimirInfoControlador(char *text) {
    char infoColor[strlen(capInfoControlador) + strlen(text) + MIDA_MAX_CADENA_COLORS * 2];
    sprintf(infoColor, "%s%s%s%s", INVERTIR_COLOR, capInfoControlador, text, FI_COLOR);
    if (write(1, infoColor, strlen(infoColor)) == -1)
        ImprimirError("ERROR write ImprimirInfoControlador");
}

void ImprimirError(char *text) {
    char infoColorError[strlen(capInfoControlador) + strlen(text) + MIDA_MAX_CADENA_COLORS * 2];
    sprintf(infoColorError, "%s%s%s: %s%s\n", FORMAT_TEXT_ERROR, capInfoControlador, text, strerror(errno), FI_COLOR);
    write(2, "\n", 1);
    write(2, infoColorError, strlen(infoColorError));
    write(2, "\n", 1);
    exit(EXIT_FAILURE);
}
