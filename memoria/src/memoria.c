#include <stdlib.h>
#include <stdio.h>
#include "memoria.h"

int main(int argc, char* argv[]) {
	//estructuras
	inicializarEstructurasMemoria();
	//inicializo servidor
	fd_memoria = iniciar_servidor(puerto_escucha_memoria, loggerMemoria, "memoria lista para recibir conexiones");
	//inicio espera con la cpu
	fd_cpu = esperar_cliente(fd_memoria, loggerMemoria, "cpu conectada");
	//atiendo cpu
	atender_cpu();

    return 0;
}

/*void iterator(char* value) {
	log_info(logger,"%s", value);
}*/
void inicializarEstructurasMemoria(void){
	loggerMemoria = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
	configMemoria = iniciar_config("memoria.config");
	puerto_escucha_memoria = config_get_string_value(configMemoria, "PUERTO_ESCUCHA");

}

void atender_cpu(void) {

	t_list* lista;

	while (1) {
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_cpu, loggerMemoria);
			break;
		/*case PAQUETE:
			lista = recibir_paquete(fd_cpu);
			log_info(loggerMemoria, "Me llegaron los siguientes valores del cpu:\n");
			list_iterate(lista, (void*) iterator);
			break;*/
		case -1:
			log_error(loggerMemoria, "el cpu se desconecto. Terminando servidor");			
			exit(EXIT_FAILURE);
		default:
			log_warning(loggerMemoria,"Operacion desconocida.");
			break;
		}
	}
	
}
