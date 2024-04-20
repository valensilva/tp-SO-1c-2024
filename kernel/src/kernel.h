#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
//commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
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




void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasKernel(void);
void handshakeEntradaSalida(int, t_log*);
//void terminar_programa(t_log*, t_config*);


#endif /* CPU_H_ */