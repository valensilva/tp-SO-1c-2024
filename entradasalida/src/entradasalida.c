#include "entradasalida.h"

int main(void){
    //Inicializo Estructuras E/S
    inicializarEstructurasEntradaSalida();
    iniciar_semaforos();
    // Crear lista vacía
    lista = list_create();

    //Conexion E/S - Kernel
    fd_es = crear_conexion(ipKernel, puertoKernel);
    if (fd_es == -1) {
        log_error(loggerEntradaSalida, "Error al conectar con el Kernel.");
        terminar_programa(loggerEntradaSalida, configEntradaSalida);
        return EXIT_FAILURE;
    }

    //Handshake con kernel
    //handshakeCliente(fd_es, loggerEntradaSalida);

    //Cargo Interfaz en lista
    agregar_paquete_gen(lista, nombre, con, disp, loggerEntradaSalida);

    //Serializacion
    //serializar_lista(lista, fd_es, loggerEntradaSalida);

    //Conexion E/S - Memoria
    //fd_esMem = crear_conexion(ipMemoria, puertoMemoria);

    //Handshake
    //handshakeCliente(fd_esMem, loggerEntradaSalida);

     //STDIN Y STDOUT
     char comando[20];
    while (1) {
        printf("Ingrese el comando (IO_STDIN_READ / IO_STDOUT_WRITE / SALIR): ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = 0;  // Remover el salto de línea

        if (strcmp(comando, "IO_STDIN_READ") == 0) {
            io_stdin_read(ipMemoria, puertoMemoria, loggerEntradaSalida);
        } else if (strcmp(comando, "IO_STDOUT_WRITE") == 0) {
            io_stdout_write(ipMemoria, puertoMemoria, loggerEntradaSalida);
        } else if (strcmp(comando, "SALIR") == 0) {
            break;
        } else {
            printf("Comando no reconocido.\n");
        }
    }


  
    //liberar_conexion(fd_es);
    //liberar_conexion(fd_esMem);

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
    tiempoTrabajo = config_get_int_value(configEntradaSalida, "TIEMPO_UNIDAD_TRABAJO");
}


//Instruccion IO_GEN_SLEEP 
//Espera segun la cantidad de unidades de trabajo que se le pase de kernel
void io_gen_sleep(char* tipoInt, int ut, t_log* logger) {
    if (strcmp(tipoInt, "GENERICA") != 0)
    {
        log_error(loggerEntradaSalida, "No es del tipo generica");
        return;
    }
    
    // Calcular el tiempo de espera
    int total_time = ut * tiempoTrabajo;
    usleep(total_time * 1000); // usleep opera en microsegundos
    log_info(loggerEntradaSalida, "Operacion: IO_GEN_SLEEP completado");
}


// Serializacion
// Interfaz Genérica 
// Lleno la lista
void agregar_paquete_gen(t_list* lista, char* nombre, uint8_t con, uint8_t disp, t_log* logger) {
    //Hardcodeo prueba
    nombre = "NombreEjemplo";

    t_paquete_gen* paquete_gen = malloc(sizeof(t_paquete_gen));
    paquete_gen->nombre = strdup(nombre);
    paquete_gen->nombre_length = strlen(paquete_gen->nombre);
    paquete_gen->con = con;
    paquete_gen->disp = disp;
    strcpy(paquete_gen->tipoInterfaz, "GENERIC");
    strcpy(paquete_gen->opAdmitidas, "IO_GEN_SLEEP");
    list_add(lista, paquete_gen);
    log_info(logger, "Lista cargada con interfaz");
}

//Serializo
void serializar_lista(t_list* lista, int socket_cliente, t_log* logger){
    t_buffer* buffer = malloc(sizeof(t_buffer));

    // Obtengo el primer paquete de la lista - Generico
    t_paquete_gen* primer_paquete = (t_paquete_gen*)(lista->head->data);


    // Calculo el tamaño del buffer
    buffer->size = sizeof(uint32_t)                     // nombre_length
                    + sizeof(uint8_t) *  4              // con, disp, tipoInterfaz, opAdmitidas
                    + primer_paquete->nombre_length;    // longitud string

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &primer_paquete->con, sizeof(uint8_t));
    buffer->offset += sizeof(u_int8_t);
    memcpy(buffer->stream + buffer->offset, &primer_paquete->disp, sizeof(uint8_t));
    buffer->offset += sizeof(u_int8_t);
    memcpy(buffer->stream + buffer->offset, &primer_paquete->tipoInterfaz, sizeof(uint8_t));
    buffer->offset += sizeof(u_int8_t);
    memcpy(buffer->stream + buffer->offset, &primer_paquete->opAdmitidas, sizeof(uint8_t));
    buffer->offset += sizeof(u_int8_t);
    memcpy(buffer->stream + buffer->offset, &primer_paquete->nombre_length, sizeof(uint32_t));
    buffer->offset += sizeof(u_int32_t);
    memcpy(buffer->stream + buffer->offset, &primer_paquete->nombre, primer_paquete->nombre_length);

    buffer->stream = buffer->stream;

    free(primer_paquete->nombre);

    //Lleno el paquete con el buffer
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = GENERICO;
    paquete->buffer = buffer;

    //Armo el stream a enviar
    void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, &(paquete->buffer->stream), paquete->buffer->size);

    //Envio
    send(socket_cliente, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t),0);
    log_info(logger, "Interfaz enviada");

    //Liberar memoria
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

//STDIN Y STDOUT

void io_stdin_read(char* ip_memoria, char* puerto_memoria, t_log* logger) {
    char buffer[256];
    printf("Ingrese el texto: ");
    fgets(buffer, sizeof(buffer), stdin);

    int direccion_fisica;
    printf("Ingrese la dirección física: ");
    scanf("%d", &direccion_fisica);
    getchar();  // Limpiar el buffer de entrada

    // Conectar a la memoria
    int fd_memoria = crear_conexion(ip_memoria, puerto_memoria);
    if (fd_memoria == -1) {
        log_error(logger, "Error al conectar con la memoria.");
        return;
    }

    // Enviar el dato y la dirección física a la memoria
    // Supongamos que hay una función `enviar_datos_memoria` para esto
    enviar_datos_memoria(fd_memoria, direccion_fisica, buffer);
    log_info(logger, "Texto enviado a la memoria.");

    // Cerrar la conexión
    liberar_conexion(fd_memoria);
}

void io_stdout_write(char* ip_memoria, char* puerto_memoria, t_log* logger) {
    int direccion_fisica;
    printf("Ingrese la dirección física: ");
    scanf("%d", &direccion_fisica);
    getchar();  // Limpiar el buffer de entrada

    // Conectar a la memoria
    int fd_memoria = crear_conexion(ip_memoria, puerto_memoria);
    if (fd_memoria == -1) {
        log_error(logger, "Error al conectar con la memoria.");
        return;
    }

    // Solicitar el dato en la dirección física
    // Supongamos que hay una función `recibir_datos_memoria` para esto
    char buffer[256];
    recibir_datos_memoria(fd_memoria, direccion_fisica, buffer);
    log_info(logger, "Datos recibidos de la memoria.");

    // Mostrar el dato por pantalla
    printf("Datos en la dirección %d: %s\n", direccion_fisica, buffer);

    // Cerrar la conexión
    liberar_conexion(fd_memoria);
}





//DIALFS


/*void terminar_programa(t_log* logger, t_config* config)
{
	
	log_destroy(logger);
	config_destroy(config);
}
*/
void iniciar_semaforos(void){

	semaforoServidorMemoria = sem_open("semaforoServidorMemoria", O_CREAT, 0644, 0);
	if(semaforoServidorMemoria == SEM_FAILED){
		log_error(loggerEntradaSalida, "error en creacion de semaforo semaforoServidorMemoria");
		exit(EXIT_FAILURE);
	}
    semaforoServidorKernel = sem_open("semaforoServidorKernel", O_CREAT, 0644, 0);
	if(semaforoServidorKernel == SEM_FAILED){
		log_error(loggerEntradaSalida, "error en creacion de semaforo semaforoServidorKernel");
		exit(EXIT_FAILURE);
	}
}