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
	RESPUESTA_HANDSHAKE
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

typedef struct
{
	uint32_t cod_instruccion;
	uint32_t * parametros;
	//tipo de dato de parametros??
} instruccion_t;

//typedef struct {
//	uint8_t AX, BX, CX, DX;
//	uint32_t PC, EAX, EBX, ECX, EDX, SI, DI;
//}registros_t;

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void handshakeCliente(int fd, t_log* logger);
void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente);

#endif /* UTILS_H_ */
