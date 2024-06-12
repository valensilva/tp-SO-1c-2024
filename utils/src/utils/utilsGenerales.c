#include <utils/utilsGenerales.h>
sem_t* semaforoServidorCPUDispatch;
sem_t* semaforoServidorCPUInterrupt;
sem_t* semaforoServidorMemoria;
sem_t* semaforoServidorIO;
sem_t* semaforoServidorKernel;
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

void terminar_programa(t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
}
int pidGeneral = 1;
char* estadoProcesoToString(EstadoProceso estado){
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case EXECUTE: return "EXECUTE";
        case BLOCKED: return "BLOCKED";
        case EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}