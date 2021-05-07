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

#define MAX_BUFFER 512

char* PUNTO_MONTAJE;
char* PUERTO;
char* IP;
uint32_t TIEMPO_SINCRONIZACION;
char* POSICIONES_SABOTAJE;
uint32_t BLOCK_SIZE;
uint32_t BLOCKS;
t_log* logger;
t_config* config;

void crear_log();
void leerConfig();
void crearDirectorio(char*);
void inicializarFS();
void crearMetadata();
void crearTodosLosBloquesEnFS();
void escribir_archivo(char*, char*);
void crearSuperBloque();
int chequearMetadata();
bool existeArchivo(char* path);
void buscarMensaje(t_paquete*, uint32_t);
char* generar_oxigeno(tarea_data_msg*);
char* consumir_oxigeno(tarea_data_msg*);
char* generar_comida(tarea_data_msg*);
char* consumir_comida(tarea_data_msg*);
char* generar_basura(tarea_data_msg*);
char* descartar_basura(tarea_data_msg*);


#endif /* I_MONGO_STORE_H_ */
