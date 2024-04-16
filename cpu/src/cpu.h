#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>
//commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

//utils
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/servidorUtils/servidorUtils.h>
#include <utils/utilsGenerales.h>


//variables globales
t_log* loggerCpu;
t_config* configCpu;
int conexionCpuMemoria;
char* ipMemoria;
char* puertoMemoria;
char* puertoEscuchaDispatch;
char* puertoEscuchaInterrupt;
int cantidadEntradasTLB;
char* algoritmoTLB;
int fd_cpu_dispatch;
int fd_cpu_interrupt;
int fd_kernel_dispatch;
int fd_kernel_interrupt;


void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasCpu(void);
void terminar_programa(t_log*, t_config*);
void atender_kernel_dispatch(void);
void atender_kernel_interrupt(void);


#endif /* CPU_H_ */