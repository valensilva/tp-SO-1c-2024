#include "kernel.h"

int main(int argc, char* argv[]) {

    //Inicio Estrucutras
    inicializarEstructurasKernel();
 /*   //COMIENZO PARTE SERVIDOR
    //Socket
    fd_kernel = iniciar_servidor(puertoEscuchaKernel, loggerKernel, "Kernel listo para recibir conexiones");
    
    //Espera de conexion E/S
    conexionEntradaSalida = esperar_cliente(fd_kernel, loggerKernel, "E/S conectado");


*/
    
    //  TERMINA PARTE SERVIDOR 

    //PARTE CLIENTE EMPIEZA

    //creo conexiones
    conexionKernelCpuDispatch = crear_conexion(ipCpu, puertoCpuDispatch);
    conexionKernelCpuInterrupt = crear_conexion(ipCpu, puertoCpuInterrupt);
    //conexionKernelMemoria = crear_conexion(ipMemoria, puertoMemoria);


    //hago handshakes
    //handshakeCliente(conexionKernelMemoria, loggerKernel);
    handshakeCliente(conexionKernelCpuDispatch, loggerKernel);
    handshakeCliente(conexionKernelCpuInterrupt, loggerKernel);

    //envio mensajes
    enviar_mensaje("hola_cpu_dispatch", conexionKernelCpuDispatch);
    enviar_mensaje("hola_cpu_interrupt", conexionKernelCpuInterrupt);
    
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

