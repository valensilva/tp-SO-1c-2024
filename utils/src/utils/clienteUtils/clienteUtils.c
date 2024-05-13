#include "clienteUtils.h"


void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
                        	    server_info->ai_socktype,
                        	    server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);
	

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

void handshakeCliente(int fd, t_log* logger) {
    size_t bytes;
    int32_t handshake = 1;
    int32_t result;

    bytes = send(fd, &handshake, sizeof(int32_t), 0);
    if (bytes < 0) {
        log_error(logger, "Error al enviar el mensaje de handshake");
        return;
    }

    bytes = recv(fd, &result, sizeof(int32_t), MSG_WAITALL);
    if (bytes < 0) {
        log_error(logger, "Error al recibir la respuesta de handshake");
        return;
    }

    if (result == 0) {
        log_info(logger, "Handshake completado con éxito");
    } else {
        log_error(logger, "Error en el handshake, código de error: %d", result);
    }
}
void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente){
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete, &(pcb_a_enviar->pid), sizeof(int));

    // Serializar el program_counter
    agregar_a_paquete(paquete, &(pcb_a_enviar->program_counter), sizeof(int));

    // Serializar el quantum
    agregar_a_paquete(paquete, &(pcb_a_enviar->quantum), sizeof(int));

    // Serializar los registros
    agregar_a_paquete(paquete, &(pcb_a_enviar->registros), sizeof(registros_CPU));

    // Serializar el estado
    agregar_a_paquete(paquete, &(pcb_a_enviar->estado), sizeof(EstadoProceso));

    // Enviar el paquete serializado a través del socket
    enviar_paquete(paquete, socket_cliente);

    // Liberar recursos
    eliminar_paquete(paquete);
}