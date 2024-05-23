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

	pcb * pcb_recibido = NULL;
	int seguir = 1;//seguir es para que si se desconecta el cliente no se termine el programa y poder salir del while
	
	while (seguir!=0) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel_dispatch, loggerCpu);
			break;
		case PAQUETE:
    		if( (pcb_recibido = malloc(sizeof(pcb))) == NULL) {
				log_error(loggerCpu, "Error al asignar memoria");
				seguir = 0;
				break;
			}
			recibir_pcb(fd_kernel_dispatch,pcb_recibido);
			log_info(loggerCpu, "Me llegaron los siguientes valores del kernel dispatch:\n");
			log_info(loggerCpu, "pid: %d", pcb_recibido->pid);
			log_info(loggerCpu, "Program Counter: %d", pcb_recibido->program_counter);
            log_info(loggerCpu, "Quantum: %d", pcb_recibido->quantum);
			log_info(loggerCpu, "State: %d", pcb_recibido->estado);
			log_info(loggerCpu, "Registros: [ %d ][ %d ]", pcb_recibido->registros.registro1, pcb_recibido->registros.registro2);
			
			//pido instrucciones

			//ejecuto ciclo de instruccion
			ciclo_de_instruccion(pcb_recibido);

			free(pcb_recibido);
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

void recibir_pcb(int socket_cliente, pcb* pcb_recibido) {

    t_list* valores = recibir_paquete(socket_cliente);

    // Extraer los datos de la lista de valores
    // Se asume que los valores están en el mismo orden que en la función enviar_pcb
    int offset = 0;

    // Obtener el pid
    memcpy(&(pcb_recibido->pid), list_get(valores, offset), sizeof(int));
    offset++;

    // Obtener el program_counter
    memcpy(&(pcb_recibido->program_counter), list_get(valores, offset), sizeof(int));
    offset++;

    // Obtener el quantum
    memcpy(&(pcb_recibido->quantum), list_get(valores, offset), sizeof(int));
    offset++;

    // Obtener los registros
    memcpy(&(pcb_recibido->registros), list_get(valores, offset), sizeof(registros_CPU));
    offset++;

    // Obtener el estado
    memcpy(&(pcb_recibido->estado), list_get(valores, offset), sizeof(EstadoProceso));
    offset++;

    // Liberar la lista de valores
    list_destroy_and_destroy_elements(valores, free);

}
typedef enum {
	SET,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP
} cod_instruccion;

typedef enum {
	AX,
	BX,
	CX,
	DX,
} reg8_t;

uint8_t reg8[] = [AX, BX, CX, DX];
uint32_t reg32[] = [];

uint32_t reg[] = [AX, BX, CX, DX, EAX];

void ciclo_de_instruccion(pcb* proceso_exec/*, t_list* instrucciones*/){

	// "SET AX 3"
	// texto_separado = string_split(instruccion, " ");
	// inst[0] = "SET"-> cod_inst
	// isnt[1] = "AX" -> 
	// inst[2] = "3"  -> atoi()


	//fetch
	uint8_t AX;
	uint32_t valor = 2;

	uint32_t parametros[] = {AX, valor};
	//instruccion_t * proxima_instruccion = list_get(instrucciones, proceso_exec->program_counter);
	instruccion_t * instruccion = {SET, parametros};
	//decode
	
	// SET AX  2    SET uint8_t  uint32_t
	// SET EAX 2    SET uint32_t uint32_t
	// SUM AX EAX   SUM uint8_t  uint32_t

	//execute
	switch (instruccion->cod_instruccion)
	{
	case SET:
		log_info(loggerCpu, "INSTRUCCION SET ");

		reg = instruccion->parametros[1];
		log_info(loggerCpu, "valor de reg[] = %d", reg[valorRecibido]);
		++(proceso_exec->program_counter);
		break;
	case SUM:
		log_info(loggerCpu, "INSTRUCCION SUM");
		break;
	case JNZ:
		log_info(loggerCpu, "INSTRUCCION JNZ");
		break;
	case SUB:
		log_info(loggerCpu, "INSTRUCCION SUB");
		break;
	case IO_GEN_SLEEP:
		log_info(loggerCpu, "INSTRUCCION IO_GEN_SLEEP");
		break;	
	default:
		break;
	}

	//chek_Interrupt
}