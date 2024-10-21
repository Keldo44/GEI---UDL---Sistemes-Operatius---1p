#include <stdio.h>     // sprintf (solo para formatear cadenas)
#include <unistd.h>    // pipe, read, write, close
#include <stdlib.h>    // exit, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>    // strlen
#include <signal.h>    // signal, SIGTERM, pause
#include <errno.h>     // errno

/*
---------------------------------------------------------------------- 
--- C O N S T A N T S ------------------------------------------------ 
---------------------------------------------------------------------- 
*/
#define MIDA_MAX_CADENA 1024
#define INVERTIR_COLOR "\e[7m"
#define FI_COLOR "\e[0m"
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
void ImprimirError(char *text);
int esPrimer(int nombre);
void handle_SIGTERM(int signum);

/*
---------------------------------------------------------------------- 
--- G L O B A L   V A R I A B L E S ---------------------------------- 
---------------------------------------------------------------------- 
*/
int pipeNombres[2], pipeRespostes[2];
int primersCalculats = 0; // Número de primos calculados por este proceso

/*
---------------------------------------------------------------------- 
--- M A I N --------------------------------------------------------- 
---------------------------------------------------------------------- 
*/
int main() {
    t_infoNombre infoNombre;

    // Registrar el manejador de señales
    signal(SIGTERM, handle_SIGTERM);

    // Cerrar el extremo de escritura de ambos pipes
    close(pipeNombres[1]);
    close(pipeRespostes[0]);

    // Leer del pipe de nombres hasta EOF
    while (read(pipeNombres[0], &infoNombre.nombre, sizeof(int)) > 0) {
        infoNombre.pid = getpid();
        infoNombre.esPrimer = esPrimer(infoNombre.nombre);

        // Enviar el resultado a través del pipe de respuestas
        if (write(pipeRespostes[1], &infoNombre, sizeof(t_infoNombre)) == -1) {
            ImprimirError("Error al escribir en el pipe de respostes");
        }

        if (infoNombre.esPrimer) {
            primersCalculats++;
        }
    }

    // Cerrar descriptores de pipes después de recibir EOF
    close(pipeNombres[0]);
    close(pipeRespostes[1]);

    // Esperar la señal SIGTERM
    pause();

    return EXIT_SUCCESS;
}

/*
---------------------------------------------------------------------- 
--- F U N C I O N S ------------------------------------------------- 
---------------------------------------------------------------------- 
*/

// Función para determinar si un número es primo
int esPrimer(int nombre) {
    if (nombre < 2) return 0;
    for (int i = 2; i * i <= nombre; i++) {
        if (nombre % i == 0) return 0;
    }
    return 1;
}

// Manejador de la señal SIGTERM
void handle_SIGTERM(int signum) {
    // Retornar el número de primos calculados al terminar
    exit(primersCalculats);
}

// Función para imprimir mensajes de error y salir
void ImprimirError(char *text) {
    write(2, FORMAT_TEXT_ERROR, strlen(FORMAT_TEXT_ERROR));
    write(2, "[Error Calculador]: ", 20);
    write(2, text, strlen(text));

    // Obtener y escribir el mensaje de error basado en errno
    char *errorStr = strerror(errno);
    write(2, ": ", 2);
    write(2, errorStr, strlen(errorStr));
    write(2, FI_COLOR, strlen(FI_COLOR));
    write(2, "\n", 1);

    exit(EXIT_FAILURE);
}
