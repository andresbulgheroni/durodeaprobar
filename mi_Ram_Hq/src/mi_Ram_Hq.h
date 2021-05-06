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

typedef enum{
	SEGMENTACION_PURA = 1,
	PAGINACION_VIRTUAL = 2
} esquema;

uint32_t TAMANIO_MEMORIA;
int32_t ESQUEMA_MEMORIA;
uint32_t TAMANIO_PAGINA;
uint32_t TAMANIO_SWAP;
char* PATH_SWAP;
char* ALGORITMO_REEMPLAZO;
char* IP;
char* PUERTO;

void* memoria_principal;

t_config* config;
t_log* logger;

void init ();
void terminar ();
int32_t get_esquema_memoria(char* esquema_config);
void recibir_mensaje(int32_t* conexion);

#endif /* MI_RAM_HQ_H_ */
