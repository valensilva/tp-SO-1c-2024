#ifndef MEMORIA_H_
#define MEMORIA_H_

#define TRUE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//commons
#include <commons/log.h>
#include <commons/config.h>

//utils
#include <utils/servidorUtils/servidorUtils.h>
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/utilsGenerales.h>
//variables globales
t_log* loggerMemoria;
t_config* configMemoria;
char* puerto_escucha_memoria;
int fd_memoria;
int fd_cpu;
int fd_IO;
int fd_kernel;
t_list* listaInstrucciones;
void iterator(char* value);
void atender_cpu(void) ;
void atender_IO(void) ;
void atender_kernel(void);
void inicializarEstructurasMemoria(void);
void iterator(char* value);
void leer_archivo( const char*);
char* pathArchivo;

#endif /* MEMORIA_H_ */
