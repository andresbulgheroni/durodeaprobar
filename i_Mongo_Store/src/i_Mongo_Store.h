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
void escribir_archivo(char*, char*);
int chequearSuperBloque();
bool existeArchivo(char* path);
int menorEntre(int, int);
op_code_tareas string_to_op_code_tareas (char*);
void buscarMensaje(inicio_tarea_msg*);
void funcionPruebaDisc(int32_t*);
void funcionPruebaTrip(int32_t*);
t_bitarray* crearBitmap(char*);
void timerSincronizacion_superBloqueMap();
void timerSincronizacion_blocksMap();
char* diccionarioFiles_to_char(t_dictionary*);
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

// FUNCIONES PARA TESTING:

void estadoSuperBloque(){
	puts("");
	printf("BLOCK SIZE: %d\n",superBloqueMap[0]);
	printf("BLOCKS: %d\n",superBloqueMap[4]);

	puts("------------------------------------");
	for(int i=0; (unsigned)i<bitarray_get_max_bit(bitmap); i++){
		if(bitarray_test_bit(bitmap, i))printf("\n%d.\tOCUPADO", i+1);
		else printf("\n%d.\tLIBRE", i+1);
	}
	puts("\n------------------------------------");
}

#endif /* I_MONGO_STORE_H_ */
