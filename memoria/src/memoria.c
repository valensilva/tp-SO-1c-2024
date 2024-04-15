#include <stdlib.h>
#include <stdio.h>
#include "memoria.h"

int main(int argc, char* argv[]) {
    return atender_cpu();
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}


int atender_cpu(void) {
	logger = log_create("memoria.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);

	int memoria_fd = iniciar_servidor();
	log_info(logger, "Memoria lista para recibir al cpu");
	int cpu_fd = esperar_cliente(memoria_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cpu_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cpu_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cpu_fd);
			log_info(logger, "Me llegaron los siguientes valores del cpu:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cpu se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida.");
			break;
		}
	}
	return EXIT_SUCCESS;
}