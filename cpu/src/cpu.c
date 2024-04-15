#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"

int main(int argc, char* argv[]) {

    int conexion;
	char* ip_cpu;
	char* puerto_cpu;
	char* valor;

	t_log* logger;
	t_config* config;

    logger = iniciar_logger();
	config = iniciar_config();

    ip_cpu 		= config_get_string_value(config, "IP_CPU");
	puerto_cpu 	= config_get_string_value(config, "PUERTO_CPU");
	valor 		= config_get_string_value(config, "CLAVE");

	// Loggeamos el valor de config
	log_info(logger, "\nIP = %s\nPUERTO = %s\n", ip_cpu, puerto_cpu);

    conexion = crear_conexion(ip_cpu, puerto_cpu);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor, conexion);

	terminar_programa(conexion, logger, config);
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;
	nuevo_logger = log_create( "CPU.log", "CPU", 1, LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config;
	nuevo_config = config_create( "./cpu.config");
	return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}
