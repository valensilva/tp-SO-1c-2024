#include "memoria.h"

int main(int argc, char* argv[]) {
	
    //estructuras
	inicializarEstructurasMemoria();
    iniciar_semaforos();
    
	//inicializo servidor
	fd_memoria = iniciar_servidor(puerto_escucha_memoria, loggerMemoria, "memoria lista para recibir conexiones");
    sem_post(semaforoServidorMemoria);
    
    //inicio espera con la cpu
	pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL,(void*) atender_cpu, NULL);
    pthread_detach(hilo_cpu);

	//inicio espera con la Interfaz I/O
	pthread_t hilo_IO;
    pthread_create(&hilo_IO, NULL,(void*) atender_IO , NULL);
    pthread_detach(hilo_IO);
    

	//incio espera con kernel 
	pthread_t hilo_kernel;
    pthread_create(&hilo_kernel, NULL, (void*) atender_kernel, NULL);
    pthread_join(hilo_kernel, NULL);

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
    fd_cpu = esperar_cliente(fd_memoria, loggerMemoria, "cpu conectada");
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_cpu);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_cpu, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_cpu);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del CPU:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case INSTRUCCIONES:
                log_info(loggerMemoria, "Solicitud de instrucciones de CPU:\n");
                enviarInstruccion();
            case -1:
                log_error(loggerMemoria, "El CPU se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerMemoria, "Operación desconocida.");
                break;
        }
    }
}


	


void atender_IO(void) {
    fd_IO = esperar_cliente(fd_memoria, loggerMemoria, "I/O conectado");
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_IO);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_IO, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_IO);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del I/O:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case -1:
                log_error(loggerMemoria, "El I/O se desconectó. Terminando hilo de conexión");
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
    fd_kernel = esperar_cliente(fd_memoria, loggerMemoria, "kernel conectado");
    t_list* lista;

    while (TRUE) {
        int cod_op = recibir_operacion(fd_kernel);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(fd_kernel, loggerMemoria);
                break;
            case PAQUETE:
                lista = recibir_paquete(fd_kernel);
                log_info(loggerMemoria, "Me llegaron los siguientes valores del kernel:\n");
                list_iterate(lista, (void*)iterator);
                break;
            case PATHARCHIVO:
                recibir_path(fd_kernel, loggerMemoria, &pathArchivo);
                leer_archivo(pathArchivo);
                break;
            case -1:
                log_error(loggerMemoria, "El kernel se desconectó. Terminando hilo de conexión");
                pthread_exit(NULL); // Terminar el hilo si la conexión se pierde
            default:
                log_warning(loggerMemoria, "Operación desconocida.");
                break;
        }
    }
}



void leer_archivo(const char* file) {
    int contador = 0;
    int confirmacion;
    char buffer[100]; // Define un buffer para almacenar una línea del archivo
    FILE *pseudocodigo = fopen(file, "rt");

    if (pseudocodigo != NULL) {
        listaInstrucciones = list_create();

        while (fgets(buffer, 100, pseudocodigo) != NULL) {
            // Elimina el carácter de nueva línea al final de la línea leída
            buffer[strcspn(buffer, "\n")] = '\0';
            
            // Asigna memoria para almacenar la instrucción
            char *instruccion = strdup(buffer);
            
            // Agrega la instrucción a la lista
            list_add_in_index(listaInstrucciones, contador, instruccion);
            contador++;
        }
        confirmacion = 1;
        size_t bytes = send(fd_cpu, &confirmacion, sizeof(int), 0);
        fclose(pseudocodigo);
    } else {
        confirmacion = 0;
        log_error(loggerMemoria, "No se pudo abrir el archivo: %s", strerror(errno));
        exit(1);
    }
}

void enviarInstruccion(){
    int numeroDeInstruccion;
    size_t bytes;
    bytes = recv(fd_cpu, &numeroDeInstruccion, sizeof(int), MSG_WAITALL);
    if(bytes<0){
        log_error(loggerMemoria, "error al recibir número de instruccion");
        return;
    }
    char* instruccion = list_get(listaInstrucciones, numeroDeInstruccion);
    bytes = send(fd_cpu, instruccion, strlen(instruccion) + 1, 0);
    if(bytes<0) log_error(loggerMemoria, "error al enviar la instruccion");
}
void iniciar_semaforos(void){

	semaforoServidorMemoria = sem_open("semaforoServidorMemoria", O_CREAT, 0644, 0);
	if(semaforoServidorMemoria == SEM_FAILED){
		log_error(loggerMemoria, "error en creacion de semaforo semaforoServidorMemoria");
		exit(EXIT_FAILURE);
	}
}
