#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_
#define GENERICO 1

#include<stdio.h>
#include<stdlib.h>
#include<readline/readline.h>

//Commons
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

//Utils
#include <utils/clienteUtils/clienteUtils.h>
#include <utils/utilsGenerales.h>

//Estructuras
//Interfaz Generica
typedef struct {
    char* nombre;
    uint32_t nombre_length;
    uint8_t con;            // Conexión (0 para no conectado, 1 para conectado)
    uint8_t disp;           // Disponibilidad (0 para no disponible, 1 para disponible)
    char tipoInterfaz[8];   // "GENERICA"
    char opAdmitidas[11];   // "IO_GEN_SLEEP"
} t_paquete_gen;

//Interfaz STDIN
typedef struct {
    char* nombre;
    uint32_t nombre_lenght;
    uint8_t con;            // Conexión (0 para no conectado, 1 para conectado)
    uint8_t disp;           // Disponibilidad (0 para no disponible, 1 para disponible)
    char tipoInterfaz[8];   // "STDIN"
    char opAdmitidas[11];   // "IO_STDIN_READ"
} t_paquete_stdin;

//Interfaz STDOUT
typedef struct {
    char* nombre;
    uint32_t nombre_lenght;
    uint8_t con;            // Conexión (0 para no conectado, 1 para conectado)
    uint8_t disp;           // Disponibilidad (0 para no disponible, 1 para disponible)
    char tipoInterfaz[8];   // "STDOUT"
    char opAdmitidas[11];   // "IO_STDOUT_WRITE"
} t_paquete_stdout;

//Interfaz DIALFS 

/*//Buffer
typedef struct {
    uint32_t size; // Tamaño payload
    uint32_t offset; // Desplazamiento dentro del playload
    void* stream; // Payload
} t_buffer;

//Lista
typedef struct {
    void** items;  
    size_t size;   
} t_list;

//Protocolo
typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;*/



//Variables Globales
int fd_es;
int fd_esMem;
int tiempoTrabajo;
uint8_t con;
uint8_t disp;
char* ipKernel;
char* puertoKernel;
char* puertoMemoria;
char* ipMemoria;
char* valor;
char* nombre;
t_list* lista;
t_log* loggerEntradaSalida;
t_config*configEntradaSalida;

void agregar_paquete_gen(t_list*, char*, uint8_t, uint8_t, t_log*);
void serializar_lista(t_list*, int socket_cliente, t_log*);
void leer_consola(t_log*);
void paquete(int);
void inicializarEstructurasEntradaSalida(void);
void io_gen_sleep(char*, int, t_log*);

//void terminar_programa(t_log*, t_config*);

#endif /* ENTRADASALIDA_H_ */