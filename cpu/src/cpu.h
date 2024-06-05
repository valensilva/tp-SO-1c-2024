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

#define LOG_CPU_FILE_NAME "cpu.log"
#define LOG_CPU_NAME "CPU"
#define CONFIG_FILE_NAME "cpu.config"


#define AX 0
#define BX 1
#define CX 3
#define DX 0
#define EAX 0 
#define EBX 0 
#define ECX 0 
#define EDX 0
#define SI 0
#define DI 0
#define PC 0


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
void iniciar_semaforos(void);
void terminar_programa(t_log*, t_config*);
void atender_kernel_dispatch(void);
void atender_kernel_interrupt(void);
void iterator(char* value);

typedef enum {
	SET,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP
} cod_instruccion;

typedef enum {
	REG_AX,
	REG_BX,
	REG_CX,
	REG_DX,
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_SI,
	REG_DI,
	REG_PC,
} reg_t;


#endif /* CPU_H_ */