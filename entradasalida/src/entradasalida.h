#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>

//Commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>


//Utils
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/utilsGenerales.h>

//Variables Globales
int fd_es;
int fd_esMem;
char* ipKernel;
char* puertoKernel;
char* puertoMemoria;
char* ipMemoria;
char* valor;
t_log* loggerEntradaSalida;
t_config*configEntradaSalida;

void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasEntradaSalida(void);

//void handshakeEntradaSalida(int, t_log*);
//void terminar_programa(t_log*, t_config*);


#endif /* ENTRADASALIDA_H_ */