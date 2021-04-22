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


typedef enum{
    INICIAR_PATOTA= 1,
    LISTAR_TRIPULANTES= 2,
    EXPULSAR_TRIPULANTE = 3,
    INICIAR_PLANIFICACION = 4,
    PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6
}opCode;



void inicializarConfig(t_config*);
void inicializarSemaforoPlanificador();


#endif /* DISCORDIADOR_H_ */
