/* -----------------------------------------------------------------------
 PRA1. Processos, pipes i senyals: Primers
 Codi font: calculador.c

 Iván Ortiga Aguilar 
 ---------------------------------------------------------------------- */

#include <stdio.h>      // Necesario para printf en caso de debugging, pero no se debe usar.
#include <stdlib.h>     // Para la función exit.
#include <unistd.h>     // Para las llamadas a sistema como read, write y close.
#include <signal.h>     // Para la gestión de señales.
#include <fcntl.h>      // Para las flags del pipe (O_RDONLY, O_WRONLY).
#include <stdbool.h>    // Para el tipo de dato bool.
#include <sys/types.h>  // Para el tipo pid_t.
#include <string.h>

// Estructura para enviar la información de los números comprobados
typedef struct {
    int pid;        // PID del proceso calculador
    int nombre;     // Número que se ha verificado
    bool es_primer; // Resultado de la verificación (true si es primo, false en caso contrario)
} t_infoNombre;

int contador_primos = 0; // Número de primos que ha calculado el proceso calculador

// Función que determina si un número es primo
bool es_primo(int num) {
    if (num <= 1) {
        return false;
    }
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0){
            return false;            
        }
    }
    return true;
}



// Manejador de la señal SIGTERM: se usará para finalizar el proceso calculador
void manejador_SIGTERM(int sig) {
    // Finaliza el proceso devolviendo el número de primos encontrados como código de estado
    exit(contador_primos);
}

// Manejador de la señal SIGUSR1: se usará para imprimir el número de primos calculados
void manejador_SIGUSR1(int sig) {
    // Controlador para la señal SIGTERM
    signal(SIGTERM, manejador_SIGTERM);
    signal(SIGUSR1, SIG_IGN); // Desactiva el manejador para la señal SIGUSR1

    int fd_pipe_nombres = 11;  // Descriptor de lectura del pipe de nombres
    int fd_pipe_respuestas = 20; // Descriptor de escritura del pipe de respuestas

    t_infoNombre infoNombre;  // Estructura para enviar la información de resultados
    infoNombre.pid = getpid(); // Asigna el PID del proceso calculador
    
    
    int nombre;
    ssize_t bytes_leidos;

    // Bucle para leer números del pipe de nombres hasta recibir EOF
    while ((bytes_leidos = read(fd_pipe_nombres, &nombre, sizeof(int))) > 0) {
        // Comprueba si el número leído es primo
        bool resultado = es_primo(nombre);

        // Actualiza la estructura con el resultado del número
        infoNombre.nombre = nombre;
        infoNombre.es_primer = resultado;

        // Si el número es primo, incrementa el contador de primos
        if (resultado) {
            contador_primos++;
        }

        //Descomentar para generar delay y que no se procese todo en el mismo calculador
        //sleep(1);
        
        // Escribe la estructura con los resultados en el pipe de respuestas
        write(fd_pipe_respuestas, &infoNombre, sizeof(t_infoNombre));
    }
    
    // Cierra los descriptores de los pipes cuando termina la lectura
    close(fd_pipe_nombres);
    close(fd_pipe_respuestas);

    exit(0);


    // Espera indefinidamente hasta recibir la señal SIGTERM
    while (1) {
        pause();  // Pausa el proceso hasta recibir una señal
    }
}

    


int main() {
    // Ignorar la señal SIGTERM
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGUSR1, manejador_SIGUSR1);
   
    // Espera indefinidamente hasta recibir la señal SIGUSR1
    while (1) {
        pause();  // Pausa el proceso hasta recibir una señal
    }

    return 0;  // No debería llegar aquí, el proceso finaliza en el manejador SIGTERM
}
