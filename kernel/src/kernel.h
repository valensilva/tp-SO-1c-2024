#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>
//commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<commons/collections/dictionary.h>
//utils
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/servidorUtils/servidorUtils.h>
#include <utils/utilsGenerales.h>


#define MAX_CONSOLA 100



//variables globales
t_log* loggerKernel;
t_config* configKernel;
char* puertoEscuchaKernel;
char* ipMemoria;
char* puertoMemoria;
char* ipCpu;
char* puertoCpuDispatch;
char* puertoCpuInterrupt;
char* algoritmoPlanificacion;
int quantum;
t_dictionary* recursos;
unsigned int gradoMultiprogramacion;
int conexionKernelCpuDispatch;
int conexionKernelCpuInterrupt; 
int fd_kernel;
int conexionEntradaSalida;
int conexionKernelMemoria;
int procesosEnReady;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExecute;
t_queue* colaReadyPlus;
pthread_t hiloContadorQuantum;
pthread_t hilo_planificador_corto_plazo;
sem_t* semaforoEspacioEnReady;
sem_t* semaforoProcesoEnReady;



void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasKernel(void);
void iniciar_semaforos(void);
void handshakeEntradaSalida(int, t_log*);
void crearProceso(char* path, int socket_memoria);
void procesoAReady(pcb* proceso);
void procesoAExecute();
void enviarProcesoACpu(int conexion);
void planificarPorFIFO();
void algoritmoFIFO(t_queue* cola);
void recibirPCBCPUFIFO();
void terminar_proceso();
void esperarQuantum();
void recibirPCBCPURR();
void algoritmoRR(t_queue* cola);
int esRR();
int esFIFO();
int esVRR();
void planificarPorRR();
void atender_IO(void);
void iterator(char* value);
void enviarInterrupcion(int conexion);
void* contadorQuantum(void* arg);
void llamadaKernel();
void planificadorCortoPlazo();
void algoritmoVRR();
void recibirPCBCPUVRR();
void enviar_a_exit(pcb* proceso);
void manejar_signal(char* nombre_recurso, pcb* proceso);
void manejar_wait(char* nombre_recurso, pcb* proceso);
t_dictionary* inicializarRecursos(t_config* config);
void planificarPorVRR();
void reanudarProceso(pcb* proceso);
void detenerProceso(pcb* proceso);
void iniciarContadorQuantum(pcb* proceso); 
int verificarQuantum(pcb* proceso);
void planificar_corto_plazo();
//void terminar_programa(t_log*, t_config*);


#endif /* CPU_H_ */