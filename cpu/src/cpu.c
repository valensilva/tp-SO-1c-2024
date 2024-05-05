#include "cpu.h"

int main(int argc, char* argv[]) {

	//inicializo estructuras cpu
	inicializarEstructurasCpu();

	//INICIA SERVIDOR CPU

	//incio servidores
	fd_cpu_dispatch = iniciar_servidor(puertoEscuchaDispatch, loggerCpu, "cpu dispatch lista para recibir conexiones");
	fd_cpu_interrupt = iniciar_servidor(puertoEscuchaInterrupt, loggerCpu, "cpu interrupt lista para recibir conexiones");

	//imicio esperas con kernel
	fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, loggerCpu, "kernel dispatch conectado");
	fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, loggerCpu, "kernel interrupt conectado");

	//atiendo kernel dispatch
	pthread_t thread_kernel_dispatch;
	pthread_create(&thread_kernel_dispatch, NULL, (void*) atender_kernel_dispatch, NULL);
	pthread_detach(thread_kernel_dispatch);
	//atiendo kernel interrupt
	pthread_t thread_kernel_interrupt;
	pthread_create(&thread_kernel_interrupt, NULL, (void*) atender_kernel_interrupt, NULL);
	pthread_join(thread_kernel_interrupt, NULL);

/*
	//INICIA PARTE CLIENTE

    conexionCpuMemoria = crear_conexion(ipMemoria, puertoMemoria);
	handshakeCliente(conexionCpuMemoria, loggerCpu);
	// envio a la memoria el mensaje hola_memoria
	enviar_mensaje("hola_memoria", conexionCpuMemoria);
	liberar_conexion(conexionCpuMemoria);
	
	//TERMINA PARTE CLIENTE
*/
	//termina programa -- NO COMENTAR -- se tiene que liberar la memoria
	terminar_programa(loggerCpu, configCpu);

	return 0;
}
void inicializarEstructurasCpu(void){
	loggerCpu = iniciar_logger(LOG_CPU_FILE_NAME, LOG_CPU_NAME, TRUE, LOG_LEVEL_INFO);
	configCpu = iniciar_config(CONFIG_FILE_NAME);	
	ipMemoria = config_get_string_value(configCpu, "IP_MEMORIA");
	puertoMemoria = config_get_string_value(configCpu, "PUERTO_MEMORIA");
	puertoEscuchaDispatch = config_get_string_value(configCpu, "PUERTO_ESCUCHA_DISPATCH");
	puertoEscuchaInterrupt = config_get_string_value(configCpu, "PUERTO_ESCUCHA_INTERRUPT");
	cantidadEntradasTLB = config_get_int_value(configCpu, "CANTIDAD_ENTRADAS_TLB");
	algoritmoTLB = config_get_string_value(configCpu, "ALGORITMO_TLB");
}
void atender_kernel_dispatch(void) {

	t_list* lista;
	int seguir = 1;//seguir es para que si se desconecta el cliente no se termine el programa y poder salir del while
	while (seguir!=0) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel_dispatch, loggerCpu);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_kernel_dispatch);
			log_info(loggerCpu, "Me llegaron los siguientes valores del kernel dispatch:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			seguir = 0;	
			log_error(loggerCpu, "el kernel dispatch se desconecto");
			break;//ver perdida de memoria
		default:
			log_warning(loggerCpu,"Operacion desconocida.");
			break;
		}
	}
}

void atender_kernel_interrupt(void) {

	t_list* lista;
	int seguir = 1;
	while (seguir!=0) {	
		int cod_op = recibir_operacion(fd_kernel_interrupt);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel_interrupt, loggerCpu);
			break;
		case PAQUETE:
			lista = recibir_paquete(fd_kernel_interrupt);
			log_info(loggerCpu, "Me llegaron los siguientes valores del kernel interrupt:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			seguir = 0;	
			log_error(loggerCpu, "el kernel interrupt se desconecto");	
			break;
		default:
			log_warning(loggerCpu,"Operacion desconocida.");
			break;
		}
	}
	
}

void iterator(char* value) {
	log_info(loggerCpu, "%s", value);
}