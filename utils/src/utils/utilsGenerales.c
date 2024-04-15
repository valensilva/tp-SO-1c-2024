#include <utils/utilsGenerales.h>

void decir_hola(char* quien) {
    printf("Hola desde %s!!\n", quien);
}
t_config* iniciar_config(char* archivo_config)
{
	t_config* nuevo_config = config_create(archivo_config);
	if (nuevo_config == NULL){
		perror("Error en el config");
		exit(EXIT_FAILURE);
	}

	return nuevo_config;
}
t_log* iniciar_logger(char* nombreArhcivoLog, char* nombreLog, bool seMuestraEnConsola, t_log_level nivelDetalle)
{
	t_log* nuevo_logger;
	nuevo_logger = log_create( nombreArhcivoLog, nombreLog, seMuestraEnConsola, nivelDetalle);
    if (nuevo_logger == NULL) {
		perror("Error en el logger");
		exit(EXIT_FAILURE);
		}
	return nuevo_logger;
}
