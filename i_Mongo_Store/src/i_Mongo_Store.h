/*
 * I_Mongo_Store.h
 *
 *  Created on: 21 abr. 2021
 *      Author: utnso
 */

#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<signal.h>
#include <commons/txt.h>
#include "utils.h"
#include<pthread.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>

#define MAX_BUFFER 512

char* PUNTO_MONTAJE;
char* PUERTO;
char* IP;
uint32_t TIEMPO_SINCRONIZACION;
char* POSICIONES_SABOTAJE;
uint32_t BLOCK_SIZE;
uint32_t BLOCKS;
char* superBloqueMap;
char* blocksMap;
t_log* logger;
t_config* config;

typedef enum{
	GENERAR_OXIGENO = 0,
	CONSUMIR_OXIGENO = 1,
	GENERAR_COMIDA = 2,
	CONSUMIR_COMIDA = 3,
	GENERAR_BASURA = 4,
	DESCARTAR_BASURA = 5,
	ERROR=6
}op_code_tareas;

void crear_log();
void leerConfig();
void crearDirectorio(char*);
void inicializarFS();
void crearSuperBloque();
void crearTodosLosBloquesEnFS();
void escribir_archivo(char*, char*);
int chequearSuperBloque();
bool existeArchivo(char* path);
op_code_tareas string_to_op_code_tareas (char*);
void buscarMensaje(inicio_tarea_msg*);
void funcionPruebaDisc(int32_t*);
void funcionPruebaTrip(int32_t*);
char* generar_oxigeno(int32_t);
char* consumir_oxigeno(int32_t);
char* generar_comida(int32_t);
char* consumir_comida(int32_t);
char* generar_basura(int32_t);
char* descartar_basura();
t_bitarray* crearBitmap(char*);


#endif /* I_MONGO_STORE_H_ */
