#include "cpu.h"

uint8_t  registros_8[4]  = {AX, BX, CX, DX};
uint32_t registros_32[7] = {EAX, EBX, ECX, EDX, SI, DI, PC};

uint32_t obtener_valor_registro(char *registro);
uint32_t obtener_valor_cod(char *str);
uint32_t strtouint32(char *str);
void recibir_instruccion(int numInstruccion, int socket_cliente, t_log* logger, char** instruccion);

int main(int argc, char* argv[]) {

	//inicializo estructuras cpu
	inicializarEstructurasCpu();
	iniciar_semaforos();

	//INICIA PARTE CLIENTE
	sem_wait(semaforoServidorMemoria);
	log_info(loggerCpu, "SEM: servidor de memoria listo");
    conexionCpuMemoria = crear_conexion(ipMemoria, puertoMemoria);
	sem_post(semaforoServidorMemoria);
    
	if (conexionCpuMemoria == -1) {
        log_error(loggerCpu, "Error al crear conexión con la memoria");
        return 1;
    }

    log_info(loggerCpu, "Conexión establecida con la memoria");
	handshakeCliente(conexionCpuMemoria, loggerCpu);

	//INICIA SERVIDOR CPU

	//incio servidores
	fd_cpu_dispatch = iniciar_servidor(puertoEscuchaDispatch, loggerCpu, "cpu dispatch lista para recibir conexiones");
	sem_post(semaforoServidorCPUDispatch);
	fd_cpu_interrupt = iniciar_servidor(puertoEscuchaInterrupt, loggerCpu, "cpu interrupt lista para recibir conexiones");
	sem_post(semaforoServidorCPUInterrupt);

	//atiendo kernel dispatch
	pthread_t thread_kernel_dispatch;
	pthread_create(&thread_kernel_dispatch, NULL, (void*) atender_kernel_dispatch, NULL);
	pthread_detach(thread_kernel_dispatch);
	
	//atiendo kernel interrupt
	pthread_t thread_kernel_interrupt;
	pthread_create(&thread_kernel_interrupt, NULL, (void*) atender_kernel_interrupt, NULL);
	pthread_join(thread_kernel_interrupt, NULL);

	//termina programa -- NO COMENTAR -- se tiene que liberar la memoria
	liberar_conexion(conexionCpuMemoria);
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
	hayInterrupciones = 0;
}
void atender_kernel_dispatch(void) {
	fd_kernel_dispatch = esperar_cliente(fd_cpu_dispatch, loggerCpu, "kernel dispatch conectado");
	pcb * pcb_recibido = NULL;
	int seguir = 1;//seguir es para que si se desconecta el cliente no se termine el programa y poder salir del while
	
	while (seguir!=0) {
		int cod_op = recibir_operacion(fd_kernel_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(fd_kernel_dispatch, loggerCpu);
			break;
		case PCB:
            pcb_recibido = recibir_pcb(fd_kernel_dispatch);
            if (!pcb_recibido) {
                seguir = 0;
                break;
            }
			log_info(loggerCpu, "--pcb recibido con exito");
            log_pcb(pcb_recibido);
            // pedir instrucciones y ejecutar ciclo de instrucción
            ciclo_de_instruccion(pcb_recibido);	
			log_info(loggerCpu, "--ciclo de instruccion finalizado con exito");
			log_pcb(pcb_recibido);
            free(pcb_recibido);
            break;
		case -1:
			seguir = 0;	
			log_error(loggerCpu, "el kernel dispatch se desconecto");
			break;//ver perdida de memoria
		default:
			log_warning(loggerCpu,"operacion desconocida desde kernel dispatch.");
			break;
		}
	}
}

void atender_kernel_interrupt(void) {
	fd_kernel_interrupt = esperar_cliente(fd_cpu_interrupt, loggerCpu, "kernel interrupt conectado");
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



void ciclo_de_instruccion(pcb* proceso_exec){

	char * instruccion;
	//fetch
	recibir_instruccion(proceso_exec->program_counter, conexionCpuMemoria,loggerCpu, &instruccion);
	char ** instruccion_separada = string_split(instruccion, " ");

	log_info(loggerCpu, "PID: %d - FETCH - Program Counter: %d", proceso_exec->pid, proceso_exec->program_counter);

	//decode
	uint32_t cod = obtener_valor_cod(instruccion_separada[0]);

	//decode - execute
	switch (cod)
	{
	case SET:

		//decode de parametros
		uint32_t reg1 = obtener_valor_registro(instruccion_separada[1]);
		uint32_t valor = strtouint32(instruccion_separada[2]);

		log_info(loggerCpu, "PID: %d - Ejecutando: %s - %s %s", proceso_exec->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);
		//execute
		if(reg1 < 4) {
			registros_8[reg1] = (uint8_t)valor;
		} else {
			registros_32[(reg1-4)] = valor;
		}
		
		registros_32[6] = ++(proceso_exec->program_counter);
		proceso_exec->registros[reg1] = valor;
		break;

	case SUM:
		reg1 = obtener_valor_registro(instruccion_separada[1]);
		uint32_t reg2 = obtener_valor_registro(instruccion_separada[2]);

		log_info(loggerCpu, "PID: %d - Ejecutando: %s - %s %s", proceso_exec->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);

		if(reg1 < 4) {
			if(reg2 < 4) {
				registros_8[reg1] = registros_8[reg1] + registros_8[reg2];
			} else {
				registros_8[reg1] = registros_8[reg1] + (uint8_t)registros_32[reg2-4];
			}
			//modifico el contexto
			proceso_exec->registros[reg1] = registros_8[reg1];
		} else {
			if(reg2<4) {
				registros_32[reg1-4] = registros_32[reg1-4] + registros_8[reg2];
			} else {
				registros_32[reg1-4] = registros_32[reg1-4] + registros_32[reg2-4];
			}
			//modifico el contexto
			proceso_exec->registros[reg1] = registros_32[reg1-4];
		}
		registros_32[6] = ++(proceso_exec->program_counter);
		
		break;

	case JNZ:
		reg1 = obtener_valor_registro(instruccion_separada[1]);
		valor = strtouint32(instruccion_separada[2]);

		log_info(loggerCpu, "PID: %d - Ejecutando: %s - %s %s", proceso_exec->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);

		if(reg1 < 4 && registros_8[reg1] != 0) {
			registros_32[6] = valor;
		} else if(registros_32[reg1] != 0){
			registros_32[6] = valor;
		}
		proceso_exec->registros[reg1] = valor;
		break;

	case SUB:

		reg1 = obtener_valor_registro(instruccion_separada[1]);
		reg2 = obtener_valor_registro(instruccion_separada[2]);

		log_info(loggerCpu, "PID: %d - Ejecutando: %s - %s %s", proceso_exec->pid, instruccion_separada[0], instruccion_separada[1], instruccion_separada[2]);

		if(reg1 < 4) {
			if(reg2<4) {
				registros_8[reg1] = registros_8[reg1] - registros_8[reg2];
			} else {
				registros_8[reg1] = registros_8[reg1] - (uint8_t)registros_32[reg2-4];			
			}
			proceso_exec->registros[reg1] = registros_8[reg1];
		} else {
			if(reg2<4) {
				registros_32[reg1-4] = registros_32[reg1-4] - registros_8[reg2];
			} else {
				registros_32[reg1-4] = registros_32[reg1-4] - registros_32[reg2-4];
			}
			proceso_exec->registros[reg1] = registros_8[reg1-4];
		}
		registros_32[6] = ++(proceso_exec->program_counter);

		break;
	case IO_GEN_SLEEP:
		
		log_info(loggerCpu, "PID: %d - Ejecutando: %s - %s %s", proceso_exec->pid, instruccion_separada[0], instruccion_separada[1]);
		
		break;	
	case EXIT:
		//Enviar PCB a KERNEL con codigo PCB_EXIT
		proceso_exec->estado = PEXIT;
		enviar_pcb_exit(proceso_exec, fd_kernel_dispatch);
		break;
	default:
		break;
	}

	//chek_Interrupt

	sem_wait(mutexInterrupciones);
	if(hayInterrupciones){
			log_info(loggerCpu, "--llego interrupción, desalojando proceso: %d", proceso_exec->pid);
			//enviar PCB a KERNEL con codigo ??
			//vaciarEspacioOcupadoTLB();
		hayInterrupciones--;
	}
	sem_post(mutexInterrupciones);

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
	else return 0;
}

uint32_t obtener_valor_cod(char *str) {
    if (strcmp(str, "SET") == 0) return SET;
    if (strcmp(str, "SUM") == 0) return SUM;
    if (strcmp(str, "SUB") == 0) return SUB;
    if (strcmp(str, "JNZ") == 0) return JNZ;
    if (strcmp(str, "IO_GEN_SLEEP") == 0) return IO_GEN_SLEEP;
	if (strcmp(str, "EXIT")==0) return EXIT;
	else return 0;
}

uint32_t strtouint32(char *str) {
    char *endptr;
    unsigned long value = strtoul(str, &endptr, 10);
    return (uint32_t)value;
}

void iniciar_semaforos(void){
	semaforoServidorCPUDispatch = sem_open("semaforoServidorCPUDispatch", O_CREAT, 0644, 0);
	if(semaforoServidorCPUDispatch == SEM_FAILED){
		log_error(loggerCpu, "error en creacion de semaforo semaforoServidorCPUDispatch");
		exit(EXIT_FAILURE);
	}
	semaforoServidorCPUInterrupt = sem_open("semaforoSeridorCPUInterrupt", O_CREAT, 0644, 0);
	if(semaforoServidorCPUDispatch == SEM_FAILED){
		log_error(loggerCpu, "error en creacion de semaforo semaforoServidorCPUInterrupt");
		exit(EXIT_FAILURE);
	}
	semaforoServidorMemoria = sem_open("semaforoServidorMemoria", O_CREAT, 0644, 0);
	if(semaforoServidorMemoria == SEM_FAILED){
		log_error(loggerCpu, "error en creacion de semaforo semaforoServidorMemoria");
		exit(EXIT_FAILURE);
	}
	mutexInterrupciones = sem_open("semaforoInterrupciones", O_CREAT, 0644, 1);
	if(mutexInterrupciones == SEM_FAILED){
		log_error(loggerCpu, "error en creacion de semaforo mutexInterrupciones");
		exit(EXIT_FAILURE);
	}
}

void solicitar_instruccion(int numInstruccion, int socket_cliente)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = INSTRUCCIONES;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &numInstruccion, sizeof(int));

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void* a_enviar = serializar_paquete(paquete, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}


void recibir_instruccion(int numInstruccion, int socket_cliente, t_log* logger, char** instruccion) {
    solicitar_instruccion(numInstruccion, socket_cliente);

    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    if (buffer == NULL) {
        log_error(logger, "Error al recibir la instrucción");
        return;
    }

    log_info(logger, "--instrucción recibida con exito");
    *instruccion = strdup(buffer); // Asigna la instrucción al puntero
    free(buffer);
}

void log_pcb(pcb* pcb_recibido){
	log_info(loggerCpu, "Valores del PCB del proceso: %d", pcb_recibido->pid);
    log_info(loggerCpu, "%-16s %-10d","Program Counter", pcb_recibido->program_counter);
    log_info(loggerCpu, "%-16s %-10d","Quantum", pcb_recibido->quantum);
    log_info(loggerCpu, "%-16s %-10s","Estado", estadoProcesoToString(pcb_recibido->estado));
    log_info(loggerCpu, "%-16s %s %d %s","Registro AX ","[",pcb_recibido->registros[0],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro BX ","[",pcb_recibido->registros[1],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro CX ","[",pcb_recibido->registros[2],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro DX ","[",pcb_recibido->registros[3],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro EAX","[",pcb_recibido->registros[4],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro EBX","[",pcb_recibido->registros[5],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro ECX","[",pcb_recibido->registros[6],"]");
    log_info(loggerCpu, "%-16s %s %d %s","Registro EDX","[",pcb_recibido->registros[7],"]");
}

void enviar_pcb_exit(pcb* pcb_a_enviar, int socket_cliente) {
    int cod_op = PCB_EXIT;
    send(socket_cliente, &cod_op, sizeof(int), 0); // Enviar el código de operación primero
    enviar_pcb(pcb_a_enviar, socket_cliente); // Luego enviar el PCB
}