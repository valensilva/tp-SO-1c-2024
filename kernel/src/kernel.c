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
    sem_post(semaforoServidorKernel);
    
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
            
            planificadorCortoPlazo();
            
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
    recursos = inicializarRecursos(configKernel);
    gradoMultiprogramacion = config_get_int_value(configKernel, "GRADO_MULTIPROGRAMACION");

    //creacion de colas de procesos
    colaNew = queue_create();
    colaReady = queue_create();
    colaExecute = queue_create();
    if(esVRR()==1){
        colaReadyPlus = queue_create();
    }
}

t_dictionary* inicializarRecursos(t_config* config){
    recursos = dictionary_create();
    char** nombres_recursos = config_get_array_value(config, "RECURSOS");
    char** instancias_recursos = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    int i = 0;
    while (nombres_recursos[i] != NULL) {
        t_recurso* recurso = malloc(sizeof(t_recurso));
        recurso->nombre = strdup(nombres_recursos[i]);
        recurso->instancias = atoi(instancias_recursos[i]);
        recurso->cola_bloqueados = list_create();

        dictionary_put(recursos, recurso->nombre, recurso);
        i++;
    }
      // Liberar arrays de configuración
    i = 0;
    while (nombres_recursos[i] != NULL) {
        free(nombres_recursos[i]);
        free(instancias_recursos[i]);
        i++;
    }
    free(nombres_recursos);
    free(instancias_recursos);

    return recursos;
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
    proceso->quantum_restante = quantum;
    proceso->estado = NEW;  
    for (int i=0; i<8; i++){
        proceso->registros[i] = 0;
    }  
    proceso->tiempoEnEjecucion = temporal_create();
    temporal_stop(proceso->tiempoEnEjecucion);
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
        procesoAReady(proceso);    
    } 
}

void procesoAReady(pcb* proceso){
    proceso->estado = READY;
    log_info(loggerKernel, "PID: <%d> - READY", proceso->pid);
    queue_push(colaReady, proceso);
    sem_post(semaforoProcesoEnReady);
}

void terminar_proceso(){
    pcb* proceso = recibir_pcb(conexionKernelCpuDispatch);
    //proceso->estado = PEXIT;
    enviar_a_exit(proceso);
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
    semaforoEspacioEnReady = sem_open("semaforoEspacioEnReady", O_CREAT, 0644, gradoMultiprogramacion);
    if(semaforoEspacioEnReady == SEM_FAILED){
		log_error(loggerKernel, "error en creacion de semaforo semaforoEspacioEnReady");
		exit(EXIT_FAILURE);
	}
    semaforoProcesoEnReady = sem_open("semaforoProcesoEnReady", O_CREAT, 0644, 0);
    if(semaforoProcesoEnReady == SEM_FAILED){
        log_error(loggerKernel, "error en creacion de semaforo semaforoProcesoEnReady");
		exit(EXIT_FAILURE);
    }
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

void manejar_wait(char* nombre_recurso, pcb* proceso){
    t_recurso* recurso = dictionary_get(recursos, nombre_recurso);
    if(recurso == NULL){
        enviar_a_exit(proceso);
        return;
    }
    recurso->instancias--;
    if (recurso->instancias < 0) {
        log_info(loggerKernel, "Proceso %d bloqueado por recurso %s", proceso->pid, nombre_recurso);
        proceso->estado = BLOCKED;
        list_add(recurso->cola_bloqueados, proceso);       
    }
    else{
        enviar_pcb(proceso, conexionKernelCpuDispatch);
        log_info(loggerKernel, "PID: <%d> continúa en EXEC después de WAIT", proceso->pid);
    } 
}
void manejar_signal(char* nombre_recurso, pcb* proceso){
    t_recurso* recurso = dictionary_get(recursos, nombre_recurso);

    if (recurso == NULL) {
        log_error(loggerKernel, "Recurso %s no encontrado", nombre_recurso);
        enviar_a_exit(proceso);
        return;
    }
    recurso->instancias++;
    if(!list_is_empty(recurso->cola_bloqueados)){
        pcb* proceso_desbloqueado = list_remove(recurso->cola_bloqueados,0);
        procesoAReady(proceso_desbloqueado);
    }
    enviar_pcb(proceso, conexionKernelCpuDispatch);
    log_info(loggerKernel, "PID: <%d> continúa en EXEC después de SIGNAL", proceso->pid);
}
void enviar_a_exit(pcb* proceso){
    t_paquete* paquete = crear_paquete(PCB_EXIT);
    enviar_paquete(paquete, conexionKernelMemoria);
    log_info(loggerKernel, "PID: <%d> - Finalizado", proceso->pid);
    free(proceso);
}

//PLANIFICADOR CORTO PLAZO
void crear_hilo_planificador_corto_plazo(){
    pthread_create(&hilo_planificador_corto_plazo, NULL, planificar_corto_plazo,NULL);
    pthread_detach(hilo_planificador_corto_plazo);
}
void planificar_corto_plazo(){
    log_info(loggerKernel, "empiezo planificador corto plazo");
    while(1){
        sem_wait(semaforoProcesoEnReady);
        if(esFIFO == 1) planificarPorFIFO();
        else if (esRR == 1) planificarPorRR();
        else if (esVRR == 1) planificarPorVRR();
        else{
            log_error(loggerKernel, "No existe la planificacion solicitada");
        }
    }
    
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
int esVRR(){
    if(strcmp(algoritmoPlanificacion, "VRR") == 0){
        return 1;
    }else return 0;
}
//FIFO
void planificarPorFIFO(){
    while(1){
        algoritmoFIFO(colaReady);
        recibirPCBCPUFIFO();
    }
}
void algoritmoFIFO(t_queue* cola){
    pcb* proceso = queue_pop(cola);
    sem_post(semaforoEspacioEnReady);
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
            terminar_proceso();
            break;
        case LLAMADAKERNEL:
            llamadaKernel();
        default:
            log_warning(loggerKernel, "operacion desconocida desde CPU Dispatch.");
            break;
    }
}
//ROUND ROBIN
void planificarPorRR(){
    while(1){
        algoritmoRR(colaReady);
        recibirPCBCPURR();
    }
}

void algoritmoRR(t_queue* cola){
    pcb* proceso = queue_pop(cola);
    sem_post(semaforoEspacioEnReady);
    proceso->estado = EXECUTE;
    reanudarProceso(proceso);
    enviar_pcb(proceso, conexionKernelCpuDispatch);
    log_info(loggerKernel, "PID: <%d> - EXECUTE", proceso->pid);
    free(proceso);    
}
//VIRTUAL ROUND ROBIN
void planificarPorVRR(){
    while(1){
        algoritmoVRR();
        recibirPCBCPUVRR();
    }
}
void algoritmoVRR(){
    pcb* proceso;
    if(!queue_is_empty(colaReadyPlus)){
        proceso = queue_pop(colaReadyPlus);        
        
    }
    else{
        proceso = queue_pop(colaReady);
        sem_post(semaforoEspacioEnReady);
    }
    proceso->estado = EXECUTE;
    reanudarProceso(proceso);
    enviar_pcb(proceso, conexionKernelCpuDispatch);
    log_info(loggerKernel, "PID: <%d> - EXECUTE", proceso->pid);
    free(proceso);
       
}
void recibirPCBCPURR(){
    op_code cod_op = recibir_operacion(conexionKernelCpuDispatch);
    switch (cod_op)
    {
    case PCB_EXIT:      
        terminar_proceso();
        break;  
    case FINQUANTUM:
        break;
    case LLAMADAKERNEL:
        llamadaKernel();
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
        terminar_proceso();
        break;
    case FINQUANTUM:
        //TODO
    case LLAMADAKERNEL:
        llamadaKernel();
    default:
        log_warning(loggerKernel, "operacion desconocida.");
        break;
    }
}

void enviarInterrupcion(int conexion){
    t_paquete* paquete = crear_paquete(INTERRUPCION);
    enviar_paquete(paquete, conexion);
}
void llamadaKernel(){
    //que haga la llamada de kernel
    //y se lo envie de vuelta a la cola de readyPlus si es VRR y tiene quantum todavia y si no a la de ready
    pcb* proceso = recibir_pcb(conexionKernelCpuDispatch);
    detenerProceso(proceso);
    //llamda a kernel
    int64_t quantumRestante = proceso->quantum_restante;
    if(esVRR()==1 && quantumRestante < quantum){
        queue_push(colaReadyPlus, proceso);
    } else queue_push(colaReady, proceso);
    
}
void detenerProceso(pcb* proceso){
    temporal_stop(proceso->tiempoEnEjecucion);
    int64_t tiempoTranscurrido = temporal_gettime(proceso->tiempoEnEjecucion);
    proceso->quantum_restante -= tiempoTranscurrido;
}
void reanudarProceso(pcb* proceso) {    
    temporal_resume(proceso->tiempoEnEjecucion);
    iniciarContadorQuantum(proceso); // Iniciar el contador de quantum con el tiempo restante
}
void* contadorQuantum(void* arg){
    pcb* proceso = (pcb*) arg;
    while (1) {
        if (verificarQuantum(proceso)) {
            enviarInterrupcion(conexionKernelCpuInterrupt);
            break;
        }
        usleep(1); // Espera un milisegundo antes de verificar nuevamente
    }
    return NULL;
}
void iniciarContadorQuantum(pcb* proceso) {
    pthread_t hilo_contador;
    pthread_create(&hilo_contador, NULL, contadorQuantum, (void*)proceso);
    pthread_detach(hilo_contador);
}
int verificarQuantum(pcb* proceso) {
    int64_t tiempoTranscurrido = temporal_gettime(proceso->tiempoEnEjecucion);
    return tiempoTranscurrido >= proceso->quantum_restante;
}

//FIN PLANIFICADOR CORTO PLAZO