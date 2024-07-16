#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
//commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
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

//recursos ??
//instancias recursos ??
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
sem_t* semaforoEspacioEnReady;
sem_t* semaforoProcesoEnReady;



void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasKernel(void);
void iniciar_semaforos(void);
void handshakeEntradaSalida(int, t_log*);
void crearProceso(char* path, int socket_memoria);
void procesoAReady();
void procesoAExecute();
void enviarProcesoACpu(int conexion);
void planificarPorFIFO();
void algoritmoFIFO(t_queue* cola);
void recibirPCBCPUFIFO();
void terminar_proceso();
void esperarQuantum();
void recibirPCBCPURR();
void algoritmoRR(t_queue* cola);
void terminar_proceso();
int esRR();
int esFIFO();
int esVRR();
void planificarPorRR();
void atender_IO(void);
void iterator(char* value);
void enviarInterrupcion(int conexion);
void contadorQuantum(pcb* proceso);
void llamadaKernel();
void planificadorCortoPlazo();
void algoritmoVRR();
void recibirPCBCPUVRR();
//void terminar_programa(t_log*, t_config*);


#endif /* CPU_H_ */