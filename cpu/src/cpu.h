#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
//commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
//utils
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/utilsGenerales.h>


//variables globales
t_log* loggerCpu;
t_config* configCpu;
int conexionCpuMemoria;
char* ipMemoria;
char* puertoMemoria;


void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasCpu(void);
void terminar_programa(int, t_log*, t_config*);


#endif /* CPU_H_ */