#include "kernel.h"

int main(int argc, char* argv[]) {

    //inicializo estructuras
    inicializarEstructurasKernel();
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
    algoritmoPlanificacion = config_get_string_value(configKernel, "ALGORITMO_PLANIFICACION");
    quantum = config_get_int_value(configKernel, "QUANTUM");
    //recursos ¿como implementar listas?
    //instancias recursos ¿como implementar listas?
    gradoMultiprogramacion = config_get_int_value(configKernel, "GRADO_MULTIPROGRAMACION");
}
void terminar_programa(t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
	
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}
