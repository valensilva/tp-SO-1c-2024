#include "cpu.h"

uint8_t  registros_8[4]  = {AX, BX, CX, DX};
uint32_t registros_32[7] = {EAX, EBX, ECX, EDX, SI, DI, PC};

uint32_t obtener_valor_registro(char *registro);
uint32_t obtener_valor_cod(char *str);
uint32_t strtouint32(char *str);

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



void ciclo_de_instruccion(pcb* proceso_exec/*, t_list* instrucciones*/){

	//fetch
	uint32_t * parametros;
	
	//instruccion_t * proxima_instruccion = list_get(instrucciones, proceso_exec->program_counter);
	char * instruccion = "SET AX 2"; 
	char ** instruccion_separada = string_split(instruccion, " ");

	//decode
	uint32_t cod = obtener_valor_cod(instruccion_separada[0]);

	//decode - execute
	switch (cod)
	{
	case SET:

		//decode de parametros
		uint32_t reg1 = obtener_valor_registro(instruccion_separada[1]);
		uint32_t valor = strtouint32(instruccion_separada[2]);

		//execute
		log_info(loggerCpu, "INSTRUCCION SET ");
		if(reg1 < 4) {
			registros_8[reg1] = (uint8_t)valor;
			log_info(loggerCpu, "Valor de registros_8[%u] = %u", reg1, registros_8[reg1]);
		} else {
			registros_32[(reg1-4)] = valor;
			log_info(loggerCpu, "Valor de registros_32[%u] = %u", reg1-4, registros_32[reg1-4]);
		}
		
		registros_32[6] = ++(proceso_exec->program_counter);
		break;

	case SUM:
		log_info(loggerCpu, "INSTRUCCION SUM");

		reg1 = obtener_valor_registro(instruccion_separada[1]);
		uint32_t reg2 = obtener_valor_registro(instruccion_separada[2]);

		if(reg1 < 4) {
			if(reg2 < 4) {
				registros_8[reg1] = registros_8[reg1] + registros_8[reg2];
				log_info(loggerCpu, "Valor de registros_8[%u] + = %u", reg1, registros_8[reg1]);
			} else {
				registros_8[reg1] = registros_8[reg1] + (uint8_t)registros_32[reg2-4];
				log_info(loggerCpu, "Valor de registros_8[%u] + = %u", reg1, registros_8[reg1]);
			}
		} else {
			if(reg2<4) {
				registros_32[reg1-4] = registros_32[reg1-4] + registros_8[reg2];
				log_info(loggerCpu, "Valor de registros_32[%u] + = %u", reg1-4, registros_32[reg1-4]);
			} else {
				registros_32[reg1-4] = registros_32[reg1-4] + registros_32[reg2-4];
				log_info(loggerCpu, "Valor de registros_32[%u] + = %u", reg1-4, registros_32[reg1-4]);
			}
		}
		registros_32[6] = ++(proceso_exec->program_counter);
		break;

	case JNZ:
		log_info(loggerCpu, "INSTRUCCION JNZ");
		reg1 = obtener_valor_registro(instruccion_separada[1]);
		valor = strtouint32(instruccion_separada[2]);

		if(reg1 < 4 && registros_8[reg1] != 0) {
			registros_32[6] = valor;
			log_info(loggerCpu, "Valor de PC = %u", registros_32[6]);
		} else if(registros_32[reg1] != 0){
			registros_32[6] = valor;
			log_info(loggerCpu, "Valor de PC = %u", registros_32[6]);
		}
		break;

	case SUB:
		log_info(loggerCpu, "INSTRUCCION SUB");

		reg1 = obtener_valor_registro(instruccion_separada[1]);
		reg2 = obtener_valor_registro(instruccion_separada[2]);

		if(reg1 < 4) {
			if(reg2<4) {
				registros_8[reg1] = registros_8[reg1] - registros_8[reg2];
				log_info(loggerCpu, "Valor de registros_8[%u] + = %u", reg1, registros_8[reg1]);
			} else {
				registros_8[reg1] = registros_8[reg1] - (uint8_t)registros_32[reg2-4];
				log_info(loggerCpu, "Valor de registros_8[%u] + = %u", reg1, registros_8[reg1]);
			}
		} else {
			if(reg2<4) {
				registros_32[reg1-4] = registros_32[reg1-4] - registros_8[reg2];
				log_info(loggerCpu, "Valor de registros_32[%u] + = %u", reg1-4, registros_32[reg1-4]);
			} else {
				registros_32[reg1-4] = registros_32[reg1-4] - registros_32[reg2-4];
				log_info(loggerCpu, "Valor de registros_32[%u] + = %u", reg1-4, registros_32[reg1-4]);
			}
		}
		registros_32[6] = ++(proceso_exec->program_counter);

		break;
	case IO_GEN_SLEEP:
		log_info(loggerCpu, "INSTRUCCION IO_GEN_SLEEP");
		break;	
	default:
		break;
	}

	//chek_Interrupt

}

uint32_t obtener_valor_registro(char * registro) {
	if (strcmp(registro, "AX") == 0) return REG_AX;
    if (strcmp(registro, "BX") == 0) return REG_BX;
    if (strcmp(registro, "CX") == 0) return REG_CX;
    if (strcmp(registro, "DX") == 0) return REG_DX;
    if (strcmp(registro, "EAX") == 0) return REG_EAX;
    if (strcmp(registro, "EBX") == 0) return REG_EBX;
    if (strcmp(registro, "ECX") == 0) return REG_ECX;
    if (strcmp(registro, "EDX") == 0) return REG_EDX;
    if (strcmp(registro, "SI") == 0) return REG_SI;
    if (strcmp(registro, "DI") == 0) return REG_DI;
    if (strcmp(registro, "PC") == 0) return REG_PC;
}

uint32_t obtener_valor_cod(char *str) {
    if (strcmp(str, "SET") == 0) return SET;
    if (strcmp(str, "SUM") == 0) return SUM;
    if (strcmp(str, "SUB") == 0) return SUB;
    if (strcmp(str, "JNZ") == 0) return JNZ;
    if (strcmp(str, "IO_GEN_SLEEP") == 0) return IO_GEN_SLEEP;
}

uint32_t strtouint32(char *str) {
    char *endptr;
    unsigned long value = strtoul(str, &endptr, 10);
    return (uint32_t)value;
}