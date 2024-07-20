#ifndef UTILSGENERALES_H
#define UTILSGENERALES_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <commons/temporal.h>
#define TRUE 1

extern sem_t* semaforoServidorCPUDispatch;
extern sem_t* semaforoServidorCPUInterrupt;
extern sem_t* semaforoServidorMemoria;
extern sem_t* semaforoServidorIO;
extern sem_t* semaforoServidorKernel;
extern sem_t* mutexInterrupciones;

typedef struct {
    char* nombre;
    int instancias;
    t_list* cola_bloqueados;
} t_recurso;


typedef enum{
    NEW,
    READY,
    EXECUTE,
    BLOCKED,
    PEXIT
}EstadoProceso;
typedef struct
{
    int pid;
    int program_counter;
    int quantum;
    int quantum_restante;
    EstadoProceso estado;
    int registros[7];
    t_temporal* tiempoEnEjecucion;
} pcb;

extern int pidGeneral; //esto para que cada vez que creo un pcb nuevo voy actualizando el valor del 
//pidGeneral en +=1

/**
* @fn    decir_hola
* @brief Imprime un saludo al nombre que se pase por parámetro por consola.
*/
void decir_hola(char* quien);
t_config* iniciar_config(char* archivo_config);
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle);
void terminar_programa(t_log* logger, t_config* config);
char* estadoProcesoToString(EstadoProceso estado); 
#endif
