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

void iterator(char* value);
void atender_cpu(void) ;
void inicializarEstructurasMemoria(void);


#endif /* MEMORIA_H_ */
