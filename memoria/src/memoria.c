#include "memoria.h"

int main(int argc, char* argv[]) {
	
    //estructuras
	inicializarEstructurasMemoria();

    
	//inicializo servidor
	fd_memoria = iniciar_servidor(puerto_escucha_memoria, loggerMemoria, "memoria lista para recibir conexiones");
    /*
    fd_cpu = esperar_cliente(fd_memoria, loggerMemoria, "cpu conectada");
    fd_IO = esperar_cliente(fd_memoria, loggerMemoria, "I/O conectado");
    */
    fd_kernel = esperar_cliente(fd_memoria, loggerMemoria, "kernel conectado");
    /*
    //inicio espera con la cpu
	pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL,(void*) atender_cpu, NULL);
    pthread_detach(hilo_cpu);

	//inicio espera con la Interfaz I/O
	pthread_t hilo_IO;
    pthread_create(&hilo_IO, NULL,(void*) atender_IO , NULL);
    pthread_detach(hilo_IO);
    */
	//incio espera con kernel 
	pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);


    leer_archivo(pathArchivo);

	int i=0;
	while  (i < list_size(listaInstrucciones)){
        char*a= list_get(listaInstrucciones, i);
        printf("%s\n", a );
        i++;
    }

	config_destroy(configMemoria);
	log_destroy(loggerMemoria);

    return 0;
}

/*
void iterator(char* value) {
	log_info(logger,"%s", value);
}*/
void inicializarEstructurasMemoria(void){
	loggerMemoria = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
	configMemoria = iniciar_config("memoria.config");
	puerto_escucha_memoria = config_get_string_value(configMemoria, "PUERTO_ESCUCHA");

}

void atender_cpu(void) {
    
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_cpu);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_cpu, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_cpu);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del cliente:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case -1:
                log_error(loggerMemoria, "El cliente se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerMemoria, "Operación desconocida.");
                break;
        }
    }
}


	


void atender_IO(void) {
    
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_IO);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_IO, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_IO);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del cliente:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case -1:
                log_error(loggerMemoria, "El cliente se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerMemoria, "Operación desconocida.");
                break;
        }
    }
}


void iterator(char* value) {
	log_info(loggerMemoria, "%s", value);
}
void atender_kernel(void) {
    
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_kernel);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_kernel, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_kernel);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del cliente:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case PATHARCHIVO:
                recibir_path(fd_kernel, loggerMemoria, pathArchivo);
               // leer_archivo(pathArchivo);
            case -1:
                log_error(loggerMemoria, "El cliente se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerMemoria, "Operación desconocida.");
                break;
        }
    }
}



void leer_archivo(const char* file) {
    int contador = 0;
    char buffer[100]; // Define un buffer para almacenar una línea del archivo
    FILE *pseudocodiogo = fopen(file, "r");

    if (pseudocodiogo != NULL) {
        listaInstrucciones = list_create();

        while (fgets(buffer, 100, pseudocodiogo) != NULL) {
            // Elimina el carácter de nueva línea al final de la línea leída
            buffer[strcspn(buffer, "\n")] = '\0';
            
            // Asigna memoria para almacenar la instrucción
            char *instruccion = strdup(buffer);
            
            // Agrega la instrucción a la lista
            list_add_in_index(listaInstrucciones, contador, instruccion);
            contador++;
        }

        fclose(pseudocodiogo);
    } else {
        log_error(loggerMemoria, "No se pudo abrir el archivo");
        exit(1);
    }
}