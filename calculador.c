/*
----------------------------------------------------------------------
--- L L I B R E R I E S ------------------------------
----------------------------------------------------------------------
*/
#include <stdio.h> /* sprintf*/

// pid_t
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h> /* exit, EXIT_SUCCESS, ...*/
#include <string.h> /* strlen */
#include <unistd.h> /* STDOUT_FILENO */
#include <errno.h>  /* errno */

/*
----------------------------------------------------------------------
--- C O N S T A N T S------ ------------------------------------------
----------------------------------------------------------------------
*/
#define MIDA_MAX_CADENA 1024

#define FI_COLOR "\e[0m"
#define MIDA_MAX_CADENA_COLORS 1024
#define FORMAT_TEXT_ERROR "\e[1;48;5;1;38;5;255m"

/*
----------------------------------------------------------------------
--- C A P Ç A L E R E S   D E   F U N C T I O N S --------------------
----------------------------------------------------------------------
*/
void ComprovarPrimer(int nombre);
void ImprimirInfoControlador(char *text);
void ImprimirError(char *text);

/*
----------------------------------------------------------------------
--- D E F I N I C I Ó   D E   T I P U S ------------------------------
----------------------------------------------------------------------
*/
typedef enum
{
    FALS = 0,
    CERT
} t_logic;

/*
----------------------------------------------------------------------
--- V A R I A B L E S   G L O B A L S --------------------------------
----------------------------------------------------------------------
*/
unsigned char numControlador;
pid_t pidPropi;
char cadena[MIDA_MAX_CADENA];
char dadesControlador[MIDA_MAX_CADENA];

char taulaColors[8][MIDA_MAX_CADENA_COLORS] = {
    "\e[01;31m", // Vermell
    "\e[01;32m", // Verd
    "\e[01;33m", // Groc
    "\e[01;34m", // Blau
    "\e[01;35m", // Magenta
    "\e[01;36m", // Cian
    "\e[00;33m", // Taronja
    "\e[1;90m"   // Gris fosc
};

/*
----------------------------------------------------------------------
--- P R O G R A M A   P R I N C I P A L ------------------------------
----------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        sprintf(cadena, "Us: %s <número calculador> <fd_pipe>\n\nPer exemple: %s 1 3\n\n", argv[0], argv[0]);
        write(STDOUT_FILENO, cadena, strlen(cadena));
        exit(1);
    }

    pidPropi = getpid();
    numControlador = atoi(argv[1]);
    int fdPipe = atoi(argv[2]);  // File descriptor per llegir del pipe

    sprintf(dadesControlador, "[Calculador %u-pid:%u]> ", numControlador, pidPropi);

    sprintf(cadena, "Calculador %u activat!\n", numControlador);
    ImprimirInfoControlador(cadena);

    int nombre;
    while (read(fdPipe, &nombre, sizeof(int)) > 0)  // Llegir nombres del pipe
    {
        ComprovarPrimer(nombre);
    }

    close(fdPipe);  // Tancar el file descriptor del pipe quan acabem
    exit(0);
}


void ComprovarPrimer(int nombre){
    int i = 2;
    t_logic esPrimer = CERT;

    if (nombre > 2)
    {
        do
        {
            if (nombre % i == 0)
                esPrimer = FALS;

            i++;

        } while (i < nombre && esPrimer);
    }

    if (esPrimer)
    {
        sprintf(cadena, "%i és primer.\n", nombre);
        ImprimirInfoControlador(cadena);
        // Aquí enviem el resultat al controlador a través del pipe de respostes
    }
    else
    {
        sprintf(cadena, "%i NO és primer.\n", nombre);
        ImprimirInfoControlador(cadena);
    }
}


void ImprimirInfoControlador(char *text)
{
    unsigned char i;
    char info[numControlador * 3 + strlen(dadesControlador) + strlen(text) + 1];
    char infoColor[numControlador * 3 + strlen(dadesControlador) + strlen(text) + 1 + MIDA_MAX_CADENA_COLORS * 2];

    for (i = 0; i < numControlador * 3; i++)
        info[i] = ' ';

    for (i = 0; i < strlen(dadesControlador); i++)
        info[i + numControlador * 3] = dadesControlador[i];

    for (i = 0; i < strlen(text); i++)
        info[i + numControlador * 3 + strlen(dadesControlador)] = text[i];

    info[numControlador * 3 + strlen(dadesControlador) + strlen(text)] = '\0';

    sprintf(infoColor, "%s%s%s", taulaColors[(numControlador - 1) % 8], info, FI_COLOR);

    if (write(STDOUT_FILENO, infoColor, strlen(infoColor)) == -1)
        ImprimirError("ERROR write ImprimirInfoControlador");
}

void ImprimirError(char *text)
{
    unsigned char i;
    char info[numControlador * 3 + strlen(dadesControlador) + strlen(text) + 1];
    char infoColorError[MIDA_MAX_CADENA];

    for (i = 0; i < numControlador * 3; i++)
        info[i] = ' ';

    for (i = 0; i < strlen(dadesControlador); i++)
        info[i + numControlador * 3] = dadesControlador[i];

    for (i = 0; i < strlen(text); i++)
        info[i + numControlador * 3 + strlen(dadesControlador)] = text[i];

    info[numControlador * 3 + strlen(dadesControlador) + strlen(text)] = '\0';

    sprintf(infoColorError, "%s%s: %s%s\n", FORMAT_TEXT_ERROR, info, strerror(errno), FI_COLOR);
    write(STDERR_FILENO, "\n", 1);
    write(STDERR_FILENO, infoColorError, strlen(infoColorError));
    write(STDERR_FILENO, "\n", 1);

    exit(2);
}