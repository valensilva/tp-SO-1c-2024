#include"servidorUtils.h"

//t_log* logger;

int iniciar_servidor(char* puerto,t_log* logger, char* mensaje)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	int socket_servidor;

	struct addrinfo hints, *server_info, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &server_info);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(server_info->ai_family,
                      server_info->ai_socktype,
                      server_info->ai_protocol);


	// Asociamos el socket a un puerto
	bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(server_info);
	log_info(logger, mensaje);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger, char* mensaje)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, mensaje);
	handshakeServidor(socket_cliente, logger);
	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente,t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el path %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
void handshakeServidor(int fd, t_log* logger){
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

   recv(fd, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
      send(fd, &resultOk, sizeof(int32_t), 0);
        log_info(logger, "Handshake completado con éxito");
    } else {
      send(fd, &resultError, sizeof(int32_t), 0);
        log_error(logger, "Error al recibir el handshake");
    }
}
pcb* recibir_pcb(int socket_cliente) {
    int size;
    int desplazamiento = 0;
    void *buffer;

    buffer = recibir_buffer(&size, socket_cliente);
    pcb *proceso = deserializar_pcb(buffer, &desplazamiento);
    free(buffer);
    return proceso;
}
pcb* deserializar_pcb(void* buffer, int* desplazamiento){
	pcb* proceso = malloc(sizeof(pcb));
	memcpy(&(proceso->pid), buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);
	memcpy(&(proceso->program_counter), buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);
	memcpy(&(proceso->estado), buffer + *desplazamiento, sizeof(EstadoProceso));
	desplazamiento += sizeof(EstadoProceso); 
	for(int i=0; i<2; i++){
		memcpy(&(proceso->registros[i]), buffer + *desplazamiento, sizeof(int));
		*desplazamiento += sizeof(int);
	}
	return proceso;
}
void recibir_path(int socket_cliente, t_log* logger, char** path) {
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    log_info(logger, "Me llego el path: %s", buffer);
    *path = strdup(buffer); // Asigna la nueva ruta al puntero path
    free(buffer);
}