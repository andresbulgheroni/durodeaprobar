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



char*IP_MI_RAM_HQ;
char*PUERTO_MI_RAM_HQ;
char*IP_I_MONGO_STORE;
char*PUERTO_I_MONGO_STORE;
int GRADO_MULTITAREA;
char*ALGORITMO;
int QUANTUM;
int DURACION_SABOTAJE;
int RETARDO_CICLO_CPU;

void inicializarConfig(t_config*);


#endif /* DISCORDIADOR_H_ */
