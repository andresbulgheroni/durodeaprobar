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
#include <math.h>

typedef enum{
	SEGMENTACION_PURA = 1,
	PAGINACION_VIRTUAL = 2
} esquema;

typedef enum{
	LRU= 1,
	CLOCK= 2
} algoritmo_reemplazo;

typedef struct{
	int32_t nro_frame;
	uint32_t nro_frame_mv;
	bool presente;
	bool uso;
	bool modificado;

} t_pagina_patota;

typedef struct{
	uint32_t pos;
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
int32_t CRITERIO_SELECCION;
char* PATH_SWAP;
int32_t ALGORITMO_REEMPLAZO;
char* IP;
char* PUERTO;

void* memoria_principal;

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
char get_status(t_status_code codigo);
void hilo_servidor();

//Paginacion
void configurar_paginacion();
void crear_patota_paginacion(iniciar_patota_msg* mensaje, bool* status);
void informar_movimiento_paginacion(informar_movimiento_ram_msg* mensaje, bool* status);
void cambiar_estado_paginacion(cambio_estado_msg* mensaje, bool* status);
char* siguiente_tarea_paginacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status);
void expulsar_tripulante_paginacion(expulsar_tripulante_msg* mensaje, bool* status);
t_list* get_frames_libres();
t_list* get_frames_a_ubicar(uint32_t cantidad);
void traer_pagina_a_memoria(t_pagina_patota* pagina);
bool memoria_llena();
bool swap_lleno();
int32_t get_frame();
t_list* swap_libres();
int32_t get_frame_memoria_virtual();
int32_t reemplazo_Clock();
int32_t reemplazo_LRU();
void guardar_en_memoria_principal(t_pagina_patota* pagina, void* datos);
void guardar_en_memoria_swap(t_pagina_patota* pagina, void* datos);
uint32_t generar_direccion_logica_paginacion(uint32_t pagina, uint32_t desplazamiento);
void obtener_direccion_logica_paginacion(uint32_t* pagina, uint32_t* desplazamiento, uint32_t direccion);
bool entra_en_swap(uint32_t cantidad);
bool entra_en_memoria(uint32_t size_pcb);
void leer_pagina_de_memoria(t_pagina_patota* pagina, void* to);
void liberar_memoria_principal_paginacion(t_pagina_patota* pagina);
void liberar_memoria_virtual(t_pagina_patota* pagina);
int32_t paginas_necesarias(uint32_t offset, uint32_t size);
void pasar_de_swap_a_principal(t_pagina_patota* pagina);
void modificar_en_memoria_principal(t_pagina_patota* pagina, void* datos);

/* Segmentacion */

typedef struct{
	uint32_t numero_segmento;
	uint32_t inicio;
	uint32_t tamanio;

}segmento;

//agrupa todas las tablas de segmentos
t_dictionary* tablas_seg_patota;

typedef enum{
	FF = 0,
	BF = 1
} criterio_seleccion;

//lista de segmentos libres
t_list* segmentos_libres();
void sacar_segmento_lista_libres(segmento* segmento_nuevo);
void ordenar_lista_segmentos_libres();
void liberar_segmento(segmento* seg);
int32_t get_espacio_libre(uint32_t size);
int32_t hay_espacio_libre(uint32_t size);

void inicializar_segmentacion();

void crear_patota_segmentacion(iniciar_patota_msg* mensaje, bool* status);
void informar_movimiento_segmentacion(informar_movimiento_ram_msg* mensaje, bool* status);
void cambiar_estado_segmentacion(cambio_estado_msg* mensaje, bool* status);

void compactar_memoria(); //sin implementar

#endif /* MI_RAM_HQ_H_ */
