#include "memoria.h"

int main(int argc, char* argv[]) {
	//estructuras
	inicializarEstructurasMemoria();
	//inicializo servidor
	fd_memoria = iniciar_servidor(puerto_escucha_memoria, loggerMemoria, "memoria lista para recibir conexiones");
	//inicio espera con la cpu
	//fd_cpu = esperar_cliente(fd_memoria, loggerMemoria, "cpu conectada");

	//atiendo cpu
	//atender_cpu();
	//inicio espera con la Interfaz I/O
	fd_IO = esperar_cliente(fd_memoria, loggerMemoria, "Interfaz conectada");

	//atiendo Interfaz I/O
	atender_IO();
	//incio espera con kernel
	//fd_kernel = esperar_cliente(fd_memoria, loggerMemoria, "Kernel conectado");
	//atiendo kernel 
	//atender_kernel();

	config_destroy(configMemoria);
	log_destroy(loggerMemoria);

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

	while (TRUE) {
		int cod_op = recibir_operacion(fd_cpu);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_cpu, loggerMemoria);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_cpu);
			log_info(loggerMemoria, "Me llegaron los siguientes valores del cpu:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(loggerMemoria, "el cpu se desconecto. Terminando servidor");			
			exit(EXIT_FAILURE);
		default:
			log_warning(loggerMemoria,"Operacion desconocida.");
			break;
		}
	}
	
}

void atender_IO(void) {

	t_list* lista;

	while (TRUE) {
		int cod_op = recibir_operacion(fd_IO);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_IO, loggerMemoria);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_IO);
			log_info(loggerMemoria, "Me llegaron los siguientes valores de la interfaz:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(loggerMemoria, "La interfaz se desconecto. Terminando servidor");			
			exit(EXIT_FAILURE);
		default:
			log_warning(loggerMemoria,"Operacion desconocida.");
			break;
		}
	}
}

void iterator(char* value) {
	log_info(loggerMemoria, "%s", value);
}
void atender_kernel(void) {
	
	t_list* lista;

	while (TRUE) {
		int cod_op = recibir_operacion(fd_kernel);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel, loggerMemoria);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_kernel);
			log_info(loggerMemoria, "Me llegaron los siguientes valores del kernel:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(loggerMemoria, "el kernel se desconecto. Terminando servidor");			
			exit(EXIT_FAILURE);
		default:
			log_warning(loggerMemoria,"Operacion desconocida.");
			break;
		}
	}


}