#include "entradasalida.h"

int main(void){
    //Inicializo Estructuras E/S
    inicializarEstructurasEntradaSalida();

    //Conexion E/S - Kernel
   // fd_es = crear_conexion(ipKernel, puertoKernel);

    //Handshake con kernel
   // handshakeCliente(fd_es, loggerEntradaSalida);

    

    //Conexion E/S - Memoria
    fd_esMem = crear_conexion(ipMemoria, puertoMemoria);

    //Handshake
    handshakeCliente(fd_esMem, loggerEntradaSalida);
  
    //liberar_conexion(fd_es);
    liberar_conexion(fd_esMem);

    //Terminar programa
    terminar_programa(loggerEntradaSalida, configEntradaSalida);
}


void inicializarEstructurasEntradaSalida(void){
	loggerEntradaSalida = iniciar_logger("entradasalida.log", "ENTRADASALIDA", 1, LOG_LEVEL_INFO);
	configEntradaSalida = iniciar_config("entradasalida.config");
    puertoKernel = config_get_string_value(configEntradaSalida, "PUERTO_KERNEL");
    ipKernel = config_get_string_value(configEntradaSalida, "IP_KERNEL");
    puertoMemoria = config_get_string_value(configEntradaSalida, "PUERTO_MEMORIA");
    ipMemoria = config_get_string_value(configEntradaSalida, "IP_MEMORIA");
}

/*void handshakeEntradaSalida(int fd_es, t_log* loggerEntradaSalida) {
    size_t bytes;
    int32_t handshake = 1;
    int32_t result;

    bytes = send(fd_es, &handshake, sizeof(int32_t), 0);
    if (bytes < 0) {
        log_error(loggerEntradaSalida, "Error al enviar el mensaje de handshake");
        return;
    }

    bytes = recv(fd_es, &result, sizeof(int32_t), MSG_WAITALL);
    if (bytes < 0) {
        log_error(loggerEntradaSalida, "Error al recibir la respuesta de handshake");
        return;
    }

    if (result == 0) {
        log_info(loggerEntradaSalida, "Handshake completado con éxito");
    } else {
        log_error(loggerEntradaSalida, "Error en el handshake, código de error: %d", result);
    }
}
*/

/*void terminar_programa(t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
}
*/