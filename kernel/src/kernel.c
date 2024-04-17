#include "kernel.h"

int main(int argc, char* argv[]) {

    //Inicio Estrucutras
    inicializarEstructurasKernel();

    //COMIENZO PARTE SERVIDOR
    //Socket
    fd_kernel = iniciar_servidor(puertoEscuchaKernel, loggerKernel, "Kernel listo para recibir conexiones");
    
    //Espera de conexion E/S
    esperaEntradaSalida = esperar_cliente(fd_kernel, loggerKernel, "E/S conectado");

    //Handshake


    //PARTE CLIENTE EMPIEZA
    //creo conexiones
    conexionKernelCpuDispatch = crear_conexion(ipCpu, puertoCpuDispatch);
    conexionKernelCpuInterrupt = crear_conexion(ipCpu, puertoCpuInterrupt);
    //envio mensajes
    enviar_mensaje("hola_cpu_dispatch", conexionKernelCpuDispatch);
    enviar_mensaje("hola_cpu_Interrupt", conexionKernelCpuInterrupt);

    //libero conexiones
    liberar_conexion(conexionKernelCpuDispatch);
    liberar_conexion(conexionKernelCpuInterrupt);
    
    //PARTE CLIENTE TERMINA

    //termino programa
    terminar_programa(loggerKernel, configKernel);

    return 0;
}
void inicializarEstructurasKernel(void){
	loggerKernel = iniciar_logger("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
	configKernel = iniciar_config("kernel.config");
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


void terminar_programa(t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
	
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}
