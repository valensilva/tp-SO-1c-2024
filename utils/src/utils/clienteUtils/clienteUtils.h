#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<utils/utilsGenerales.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
	HANDSHAKE,
	RESPUESTA_HANDSHAKE,
	PATHARCHIVO,
	ENVIARPROCESO,
	PCB_EXIT
}op_code;

typedef struct
{
	int size;
	int offset;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;



int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(op_code cope_op);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void handshakeCliente(int fd, t_log* logger);
void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente);
void enviar_path(char* path, int socket_cliente);
void crear_buffer(t_paquete* paquete);

#endif /* UTILS_H_ */
