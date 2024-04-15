#include "cpu.h"

int main(int argc, char* argv[]) {

    //inicializo estructuras cpu
	inicializarEstructurasCpu();
	
    conexionCpuMemoria = crear_conexion(ipMemoria, puertoMemoria);


	// envio a la memoria el mensaje holaaa
	enviar_mensaje("holaaa", conexionCpuMemoria);

	terminar_programa(conexionCpuMemoria, loggerCpu, configCpu);
}
void inicializarEstructurasCpu(void){
	loggerCpu = iniciar_logger("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
	configCpu = iniciar_config("cpu.config");	
	ipMemoria = config_get_string_value(configCpu, "IP_MEMORIA");
	puertoMemoria = config_get_string_value(configCpu, "PUERTO_MEMORIA");
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}
