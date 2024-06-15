#include "kernel.h"

int main(int argc, char* argv[]) {
    char texto_por_consola[MAX_CONSOLA];
    char* cod_op_kernel;
    char* path;
    char **texto_separado;
    pidGeneral = 1;
    //Inicio Estrucutras
    inicializarEstructurasKernel();
    iniciar_semaforos();
    //COMIENZO PARTE SERVIDOR
    //Socket
    fd_kernel = iniciar_servidor(puertoEscuchaKernel, loggerKernel, "Kernel listo para recibir conexiones");
    sem_post(&semaforoServidorKernel);
    
    //PARTE CLIENTE EMPIEZA

    //creo conexiones
    sem_wait(semaforoServidorCPUDispatch);
    conexionKernelCpuDispatch = crear_conexion(ipCpu, puertoCpuDispatch);
    handshakeCliente(conexionKernelCpuDispatch, loggerKernel);   
    
    sem_wait(semaforoServidorCPUInterrupt);
    conexionKernelCpuInterrupt = crear_conexion(ipCpu, puertoCpuInterrupt);
    handshakeCliente(conexionKernelCpuInterrupt, loggerKernel);

    sem_wait(semaforoServidorMemoria);
    log_info(loggerKernel, "SEM: servidor de memoria listo");
    conexionKernelMemoria = crear_conexion(ipMemoria, puertoMemoria);
    handshakeCliente(conexionKernelMemoria, loggerKernel);   
   /* 
    //Espera de conexion E/S
    pthread_t hilo_conexion_IO;
    pthread_create(&hilo_conexion_IO, NULL, (void*) atender_IO, NULL);
    pthread_join(hilo_conexion_IO, NULL);  
   */
    //INICIO CONSOLA
    while(1){ 
        printf("Ingrese codigo de operacion\n");
        if (fgets(texto_por_consola, sizeof(texto_por_consola), stdin) == NULL) {
            fprintf(stderr, "error leyendo de consola\n");          
        }
        texto_por_consola[strlen(texto_por_consola)-1] = '\0';
        texto_separado = string_split(texto_por_consola, " ");
        
        cod_op_kernel = texto_separado[0];
        path = texto_separado[1];
        //puts(path);
        if (strcmp(cod_op_kernel, "INICIAR_PROCESO") == 0){
            crearProceso(path, conexionKernelMemoria);
            
            if(esFIFO() == 1) planificarPorFIFO();
            else if(esRR() == 1) planificarPorRR();
            
        }
        else {
            printf("operacion desconocida");
        }
        
    }  
    
    //PARTE CLIENTE TERMINA 

    //termino programa
    liberar_conexion(conexionKernelMemoria);
    terminar_programa(loggerKernel, configKernel);

    return 0;
}

void inicializarEstructurasKernel(void){
	loggerKernel = iniciar_logger("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
	configKernel = iniciar_config("kernel.config");
    ipMemoria = config_get_string_value(configKernel, "IP_MEMORIA");
    puertoMemoria = config_get_string_value(configKernel, "PUERTO_MEMORIA");
    ipCpu = config_get_string_value(configKernel, "IP_CPU");
    puertoCpuDispatch = config_get_string_value(configKernel, "PUERTO_CPU_DISPATCH");
    puertoCpuInterrupt = config_get_string_value(configKernel, "PUERTO_CPU_INTERRUPT");	
    puertoEscuchaKernel = config_get_string_value(configKernel, "PUERTO_ESCUCHA");
    algoritmoPlanificacion = config_get_string_value(configKernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_int_value(configKernel, "QUANTUM");

    //recursos ¿como implementar listas?
    //instancias recursos ¿como implementar listas?
    gradoMultiprogramacion = config_get_int_value(configKernel, "GRADO_MULTIPROGRAMACION");

    //creacion de colas de procesos
    colaNew = queue_create();
    colaReady = queue_create();
    colaExecute = queue_create();
    if(algoritmoPlanificacion == "VRR"){
        colaReadyPlus = queue_create();
    }
}

void handshakeKernel(int fd_kernel,t_log* loggerKernel){
    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

   recv(fd_kernel, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == 1) {
      send(fd_kernel, &resultOk, sizeof(int32_t), 0);
        log_info(loggerKernel, "Handshake completado con éxito");
    } else {
      send(fd_kernel, &resultError, sizeof(int32_t), 0);
        log_error(loggerKernel, "Error al recibir el handshake");
    }
}

void crearProceso(char* path, int socket_memoria){
    int confirmacion;
    pcb* proceso = malloc(sizeof(pcb));
    if (proceso == NULL) {
        log_error(loggerKernel, "Error al asignar memoria para el proceso");
        return;
    }
    proceso->pid = pidGeneral;
    proceso->program_counter = 0;
    proceso->quantum = quantum;
    proceso->estado = NEW;  
    for (int i=0; i<8; i++){
        proceso->registros[i] = 0;
    }
    queue_push(colaNew, proceso);
    pidGeneral += 1;
    log_info(loggerKernel, "PID: <%d> - NEW", proceso->pid);
    enviar_path(path, socket_memoria);
    
    size_t bytes = recv(socket_memoria, &confirmacion, sizeof(int), 0);
    if(bytes<0){
        log_error(loggerKernel, "error al recibir confirmacion");
        return;
    }
    log_info(loggerKernel,"Path recibido por memoria con exito");
    if(confirmacion == 1){
        sem_wait(semaforoEspacioEnReady);
        procesoAReady();    
    } 
}

void procesoAReady(){
    pcb* proceso = queue_pop(colaNew);
    proceso->estado = READY;
    log_info(loggerKernel, "PID: <%d> - READY", proceso->pid);
    queue_push(colaReady, proceso);
    
}
int esFIFO(){
    if (strcmp(algoritmoPlanificacion, "FIFO") == 0){
        return 1;
    }
    else return 0;
}
int esRR(){
    if(strcmp(algoritmoPlanificacion, "RR") == 0){
        return 1;
    }else return 0;
}
void planificarPorFIFO(){
    while(1){
        algoritmoFIFO(colaReady);
        recibirPCBCPUFIFO();
    }
}
void algoritmoFIFO(t_queue* cola){
    pcb* proceso = queue_pop(cola);
    sem_post(&semaforoEspacioEnReady);
    proceso->estado = EXECUTE;
    enviar_pcb(proceso, conexionKernelCpuDispatch);
    log_info(loggerKernel, "PID: <%d> - EXECUTE", proceso->pid);
    free(proceso);
}

void recibirPCBCPUFIFO(){
    op_code code_op = recibir_operacion(conexionKernelCpuDispatch);
    switch(code_op){
        case PCB_EXIT:
            log_info(loggerKernel,"--proceso recibido para finalizacion");
            terminar_proceso(PCB_EXIT);
            break;
        default:
            log_warning(loggerKernel, "operacion desconocida desde CPU Dispatch.");
            break;
    }
}
void planificarPorRR(){
    while(1){
        algoritmoRR(colaReady);
        recibirPCBCPURR();
    }
}
void terminar_proceso(op_code code_op){
    pcb* proceso = recibir_pcb(conexionKernelCpuDispatch);
    //proceso->estado = PEXIT;
    t_paquete* paquete = crear_paquete(PCB_EXIT);
    enviar_paquete(paquete, conexionKernelMemoria);
    log_info(loggerKernel, "PID: <%d> - Finalizado", proceso->pid);
    free(proceso);
}
void algoritmoRR(t_queue* cola){
    pcb* proceso = queue_pop(cola);
    sem_post(&semaforoEspacioEnReady);
    proceso->estado = EXECUTE;
    enviar_pcb(proceso, conexionKernelCpuDispatch);
    t_temporal contadorQuamtum = temporal_create();
    free(proceso);    
}
void algoritmoVRR(){
    if(!queue_is_empty(colaReadyPlus)){
        pcb* proceso = queue_pop(colaReadyPlus);        
        proceso->estado = EXECUTE;
        temporal_resume(proceso->tiempoEnEjecucion); //habria que pausarlo cuando se recibe
        enviar_pcb(proceso, conexionKernelCpuDispatch);
    }
    else{
        pcb* proceso = queue_pop(colaReady);
        sem_post(&semaforoEspacioEnReady);
        proceso->estado = EXECUTE;
        proceso->tiempoEnEjecucion = temporal_create();
        enviar_pcb(proceso, conexionKernelCpuDispatch);
        t_temporal contadorQuamtum = temporal_create();
    }
    
    free(proceso);    
}
void recibirPCBCPURR(){
    op_code cod_op = recibir_operacion(conexionKernelCpuDispatch);
    switch (cod_op)
    {
    case PCB_EXIT:      
        terminar_proceso(PCB_EXIT);
        break;    
    default:
        log_warning(loggerKernel, "operacion desconocida.");
        break;
    }
}
void recibirPCBCPUVRR(){
    op_code cod_op = recibir_operacion(conexionKernelCpuDispatch);
    switch (cod_op)
    {
    case PCB_EXIT:      
        terminar_proceso(PCB_EXIT);
        break;
    case FINQUANTUM:
        //TODO
    case LLAMADAKERNEL:
            
    default:
        log_warning(loggerKernel, "operacion desconocida.");
        break;
    }
}
int verificarQuantum(pcb* proceso, t_temporal tiempo){
    int64_t tiempoTranscurrido = temporal_gettime(tiempo);
    if(tiempoTranscurrido >= quantum){
        
        return 1;
    }
    else return 0;  
}
void iniciar_semaforos(void){
	semaforoServidorCPUDispatch = sem_open("semaforoServidorCPUDispatch", O_CREAT, 0644, 0);
	if(semaforoServidorCPUDispatch == SEM_FAILED){
		log_error(loggerKernel, "error en creacion de semaforo semaforoServidorCPUDispatch");
		exit(EXIT_FAILURE);
	}
	semaforoServidorCPUInterrupt = sem_open("semaforoSeridorCPUInterrupt", O_CREAT, 0644, 0);
	if(semaforoServidorCPUDispatch == SEM_FAILED){
		log_error(loggerKernel, "error en creacion de semaforo semaforoServidorCPUInterrupt");
		exit(EXIT_FAILURE);
	}
	semaforoServidorMemoria = sem_open("semaforoServidorMemoria", O_CREAT, 0644, 0);
	if(semaforoServidorMemoria == SEM_FAILED){
		log_error(loggerKernel, "error en creacion de semaforo semaforoServidorMemoria");
		exit(EXIT_FAILURE);
	}
    semaforoServidorKernel = sem_open("semaforoServidorKernel", O_CREAT, 0644, 0);
	if(semaforoServidorKernel == SEM_FAILED){
		log_error(loggerKernel, "error en creacion de semaforo semaforoServidorKernel");
		exit(EXIT_FAILURE);
	}
    sem_init(semaforoEspacioEnReady, 0, gradoMultiprogramacion);
}
void atender_IO(void){
    
    t_list* lista;
    conexionEntradaSalida = esperar_cliente(fd_kernel, loggerKernel, "E/S conectado"); 
    while (TRUE) {
        int cod_op = recibir_operacion(conexionEntradaSalida);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(conexionEntradaSalida, loggerKernel);
                break;
            case PAQUETE:
                lista = recibir_paquete(conexionEntradaSalida);
                log_info(loggerKernel, "Me llegaron los siguientes valores del cliente:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case -1:
                log_error(loggerKernel, "El cliente se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerKernel, "Operación desconocida.");
                break;
        }
    }
}
void iterator(char* value) {
	log_info(loggerKernel, "%s", value);
}

void enviarInterrupcion(int conexion){
    t_paquete paquete = crear_paquete(INTERRUPCION);
    enviar_paquete(paquete, conexion);
}
void contadorQuantum(){
    int confirmacion = 1;
    while(confirmacion == 1){
       confirmacion = verificarQuantum();
    }
    enviarInterrupcion(conexionKernelCpuInterrupt);
}