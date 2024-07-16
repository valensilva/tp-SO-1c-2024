#include <kernel.h>

pthread_t hilo_planificador_corto_plazo;

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
        terminar_proceso(PCB_EXIT);
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
        terminar_proceso(PCB_EXIT);
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