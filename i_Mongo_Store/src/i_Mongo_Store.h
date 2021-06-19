#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include<openssl/md5.h>
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

#define MAX_BUFFER 1024

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
int tamanioBlocks;
int tamanioSuperBloque;
t_bitarray* bitmap;

typedef enum{
	GENERAR_OXIGENO = 0,
	CONSUMIR_OXIGENO = 1,
	GENERAR_COMIDA = 2,
	CONSUMIR_COMIDA = 3,
	GENERAR_BASURA = 4,
	DESCARTAR_BASURA = 5,
	ERROR=6
}op_code_tareas;

typedef struct {
	uint32_t SIZE;
	uint32_t BLOCK_COUNT;
	t_list* BLOCKS;
	char* CARACTER_LLENADO;
	char* MD5_ARCHIVO;
}t_recurso;

//typedef struct {
//	uint32_t SIZE;
//	uint32_t BLOCK_COUNT;
//	t_list* BLOCKS;
//	char* CARACTER_LLENADO;
//	char* MD5_ARCHIVO;
//}t_comida;
//
//typedef struct {
//	uint32_t SIZE;
//	uint32_t BLOCK_COUNT;
//	t_list* BLOCKS;
//	char* CARACTER_LLENADO;
//	char* MD5_ARCHIVO;
//}t_basura;

void crear_log();
void leerConfig();
void crearDirectorio(char*);
void inicializarFS();
void inicializarSuperBloque();
void crearSuperBloque();
void inicializarBlocks();
bool existeArchivo(char* path);
int menorEntre(int, int);
op_code_tareas string_to_op_code_tareas (char*);
void hacerTarea(inicio_tarea_msg*);
void funcionPruebaDisc(int32_t*);
void funcionPruebaTrip(int32_t*);
t_bitarray* crearBitmap(char*);
void timerSincronizacion_blocksMap();
void setBitmap(int, int);
int existeFS();
int primerBloqueLibre();
void stringToBlocks(char*, char*, char*);
int writeBlock(char*, int);
char* calcularMD5(char*);
int generarRecurso(int32_t, char);
int descartarBasura();
int consumirRecurso(int32_t, char);
int writeBitacora(int32_t, char*);
char* readBitacora(int32_t);
void sighandler();
int fsckSuperBloque_Bloques();
int fsckSuperBloque_Bitmap();
int fsckFiles_Size();
int fsckFiles_BlockCount();
int fsckFiles_Blocks();

// FUNCIONES PARA TESTING:

void estadoSuperBloque(){
	puts("");
	printf("BLOCK SIZE: %d\n", BLOCK_SIZE);
	printf("BLOCKS: %d\n", BLOCKS);

	puts("------------------------------------");
	for(int i=0; (unsigned)i<bitarray_get_max_bit(bitmap); i++){
		if(bitarray_test_bit(bitmap, i))printf("\n%d.\tOCUPADO", i+1);
		else printf("\n%d.\tLIBRE", i+1);
	}
	puts("\n------------------------------------");
}

// FUNCIONES VIEJAS DICCIONARIO PROPIO:

//t_dictionary* armar_diccionario(char* stream){
//
//	t_dictionary* diccionario = dictionary_create();
//
//	char** lines = string_split(stream, "\n");
//
//	void add_cofiguration(char *line) {
//		if (!string_starts_with(line, "#")) {
//			char** keyAndValue = string_n_split(line, 2, "=");
//			dictionary_put(diccionario, keyAndValue[0], keyAndValue[1]);
//			free(keyAndValue[0]);
//			free(keyAndValue);
//		}
//	}
//	string_iterate_lines(lines, add_cofiguration);
//	string_iterate_lines(lines, (void*) free);
//
//	free(lines);
//
//	return diccionario;
//
//}
//
//char *config_get_string_value_propio(t_dictionary *self, char *key) {
//
//	return dictionary_get(self, key);
//
//}
//
//char** config_get_array_value_propio(t_dictionary *self, char* key) {
//
//	char* value_in_dictionary = config_get_string_value_propio(self, key);
//	return string_get_string_as_array(value_in_dictionary);
//
//}
//
//int config_get_int_value_propio(t_dictionary *self, char *key) {
//
//	char *value = config_get_string_value_propio(self, key);
//	return atoi(value);
//
//}
//
//void config_remove_key_propio(t_dictionary *self, char *key) {
//
//	if(dictionary_has_key(self, key)) {
//		dictionary_remove_and_destroy(self, key, free);
//	}
//
//}
//
//void config_set_value_propio(t_dictionary *self, char *key, char *value) {
//
//	config_remove_key_propio(self, key);
//	char* duplicate_value = string_duplicate(value);
//	dictionary_put(self, key, (void*)duplicate_value);
//
//}
//
//char* diccionarioFiles_to_char(t_dictionary* dic){
//
//	char* cadena = string_new();
//	char* keyAndValue;
//	char* valor;
//
//	valor = config_get_string_value_propio(dic, "SIZE");
//	keyAndValue = string_from_format("%s=%s\n","SIZE",valor);
//	string_append(&cadena, keyAndValue);
//	valor = config_get_string_value_propio(dic, "BLOCK_COUNT");
//	keyAndValue = string_from_format("%s=%s\n","BLOCK_COUNT",valor);
//	string_append(&cadena, keyAndValue);
//	valor = config_get_string_value_propio(dic, "BLOCKS");
//	keyAndValue = string_from_format("%s=%s\n","BLOCKS",valor);
//	string_append(&cadena, keyAndValue);
//	valor = config_get_string_value_propio(dic, "CARACTER_LLENADO");
//	keyAndValue = string_from_format("%s=%s\n","CARACTER_LLENADO",valor);
//	string_append(&cadena, keyAndValue);
//	valor = config_get_string_value_propio(dic, "MD5_ARCHIVO");
//	keyAndValue = string_from_format("%s=%s","MD5_ARCHIVO",valor);
//	string_append(&cadena, keyAndValue);
//
//	return cadena;
//
//}

#endif /* I_MONGO_STORE_H_ */
