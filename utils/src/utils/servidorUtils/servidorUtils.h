#ifndef UTILSSERVER_H_
#define UTILSSERVER_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include <utils/utilsGenerales.h>

//#define PUERTO "4444"

/*typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;
*/
extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char* puerto, t_log* logger, char* mensaje);
int esperar_cliente(int socket_servidor, t_log* logger, char* mensaje);
t_list* recibir_paquete(int);
void recibir_mensaje(int socket_cliente,t_log* logger);
int recibir_operacion(int);
void handshakeServidor(int fd, t_log* logger);
pcb* recibir_pcb(int socket_cliente);
pcb* deserializar_pcb(void* buffer, int* desplazamiento);
void recibir_path(int socket_cliente,t_log* logger, char* path);

#endif /* UTILSSERVER_H_ */
