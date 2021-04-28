#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/log.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<pthread.h>
#include<semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

extern pthread_mutex_t mutex_id_tripulantes;
extern pthread_mutex_t mutex_tripulantes;

extern pthread_mutex_t mutex_listaNuevos;
extern pthread_mutex_t mutex_listaReady;
extern pthread_mutex_t mutex_listaBloqueados;
extern pthread_mutex_t mutex_listaEjecutando;
extern pthread_mutex_t mutex_listaFinalizados;

t_log* logger;

t_config*config;

char*IP_MI_RAM_HQ;
char*PUERTO_MI_RAM_HQ;
char*IP_I_MONGO_STORE;
char*PUERTO_I_MONGO_STORE;
int GRADO_MULTITAREA;
char*ALGORITMO;
int QUANTUM;
int DURACION_SABOTAJE;
int RETARDO_CICLO_CPU;

sem_t sem_planificar;

t_list* tareas;
t_list* posicionesTripulantes;

t_list* tripulantes;
t_list* hilosTripulantes;

t_list* listaNuevos;
t_list* listaReady;
t_list* listaBloqueados;
//t_list* listaEjecutando;
t_list* listaFinalizados;

typedef enum{

	INICIAR_PATOTA= 1,
	LISTAR_TRIPULANTES= 2,
	EXPULSAR_TRIPULANTE = 3,
	INICIAR_PLANIFICACION = 4,
	PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6,
	ERROR_CODIGO=7

}opCode;

typedef struct{

	int  cantPatota;
	int numeroPatota;
	t_list* tripulantes;
	char* tareas;		//TODO

} t_iniciarPatotaMsg;

typedef struct
{
	uint32_t posX;
	uint32_t posY;
} t_coordenadas;


typedef struct
{
	uint32_t id_sabotaje;		//TODO
	t_coordenadas* coordenadas;

} t_sabotaje;

typedef enum{

    NEW = 1,
    READY = 2,
    BLOCKED = 3,
    EXEC = 4,
    FINISHED = 5

}t_status_code;

typedef struct
{
	uint32_t nombreTarea;		//TODO
	t_coordenadas* coordenadas;
	uint32_t duracion;

} t_tarea;

typedef struct
{
	uint32_t idTripulante;
	uint32_t idPatota;
	t_coordenadas* coordenadas;
	t_status_code estado;
	t_sabotaje* sabotaje;		//TODO
	uint32_t misCiclosDeCPU;
	t_tarea* tareaAsignada;

} t_tripulante;


void iniciarLog();

void inicializarConfig(t_config*);
void inicializarSemaforoPlanificador();
void leer_consola();
void inicializarAtributosATripulante();
t_coordenadas* get_coordenadas(char*);
opCode string_a_op_code (char*);


#endif /* DISCORDIADOR_H_ */
