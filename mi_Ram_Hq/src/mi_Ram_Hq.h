/*
 * mi_Ram_Hq.h
 *
 *  Created on: 21 abr. 2021
 *      Author: utnso
 */

#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include "utils.h"
#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <netdb.h>

typedef enum{
	SEGMENTACION_PURA = 1,
	PAGINACION_VIRTUAL = 2
} esquema;

typedef enum{
	LRU= 1,
	CLOCK= 2
} algoritmo_reemplazo;

typedef struct{
	uint32_t nro_frame;
	bool presente;
	bool uso;
	bool modificado;

} t_pagina_patota;

typedef struct{
	uint32_t pos;
	t_pagina_patota* pagina;
} t_frame;

typedef struct{
	uint32_t pos;
} t_frame_libre;

typedef struct{
	t_pagina_patota* pagina;
} t_buffer_clock;

uint32_t TAMANIO_MEMORIA;
int32_t ESQUEMA_MEMORIA;
uint32_t TAMANIO_PAGINA;
uint32_t TAMANIO_SWAP;
char* PATH_SWAP;
uint32_t ALGORITMO_REEMPLAZO;
char* IP;
char* PUERTO;

void* memoria_principal;
//Segmentacion

//Paginacion


void* memoria_virtual;
t_dictionary* tabla_paginas_patota;
t_list* frames_libres_principal;
t_list* frames_swap;
t_list* lista_para_reemplazo;
uint32_t buffer_clock_pos;

t_config* config;
t_log* logger;

typedef struct {

	uint32_t pid;
	uint32_t direccion_tareas;

} t_pcb;

typedef struct {

	uint32_t tid;
	char estado;
	uint32_t posX;
	uint32_t posY;
	uint32_t proxima_instruccion;
	uint32_t direccion_patota;

} t_tcb;

void init ();
void iniciarMapa();
void terminar ();
int32_t get_esquema_memoria(char* esquema_config);
int32_t get_algoritmo(char* algoritmo_config);
void recibir_mensaje(int32_t* conexion);

#endif /* MI_RAM_HQ_H_ */
