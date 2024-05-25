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
int gradoMultiprogramacion;
int conexionKernelCpuDispatch;
int conexionKernelCpuInterrupt; 
int fd_kernel;
int conexionEntradaSalida;
int conexionKernelMemoria;
int procesosEnReady;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExecute;
pthread_t hiloContadorQuantum;



void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasKernel(void);
void handshakeEntradaSalida(int, t_log*);
void crearProceso(char* path, int socket_memoria);
void procesoAReady();
void procesoAExecute();
void enviarProcesoACpu(int conexion);
void planificarPorFIFO();
void algoritmoFIFO(t_queue* cola);
void recibirPCBCPUFIFO();
void terminar_proceso(op_code code_op);
void esperarQuantum();
void recibirPCBCPURR();
void algoritmoRR(t_queue* cola);
void terminar_proceso(op_code code_op);
int esRR();
int esFIFO();
void planificarPorRR();
//void terminar_programa(t_log*, t_config*);


#endif /* CPU_H_ */