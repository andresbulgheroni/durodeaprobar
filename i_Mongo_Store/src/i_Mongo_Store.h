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
#include <semaphore.h>

#define MAX_BUFFER 1024

uint32_t ID_SABOTAJE;
char* PUNTO_MONTAJE;
char* PUERTO;
char* IP;
uint32_t TIEMPO_SINCRONIZACION;
char** POSICIONES_SABOTAJE;
uint32_t BLOCK_SIZE;
uint32_t BLOCKS;
char* superBloqueMap;
char* blocksMap;
char* blocksMapOriginal;
t_log* logger;
t_config* config;
int tamanioBlocks;
int tamanioSuperBloque;
int32_t*SOCKET_SABOTAJE;
t_bitarray* bitmap;
pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_oxigeno;
pthread_mutex_t mutex_comida;
pthread_mutex_t mutex_basura;
bool moduloAbierto = true;

typedef enum{
	GENERAR_OXIGENO = 0,
	CONSUMIR_OXIGENO = 1,
	GENERAR_COMIDA = 2,
	CONSUMIR_COMIDA = 3,
	GENERAR_BASURA = 4,
	DESCARTAR_BASURA = 5,
	ERROR=6
}op_code_tareas;

void inicializarSemaforos();
void crearLog();
void leerConfig(char*);
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
void recibirMensajeTripulante(int32_t*);
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
void actualizarBitmap(char*);
void haySabotajeCountEnElArchivo(char*, char);
void liberarRecursos();
void iniciarProtocoloFSCK();
void cerrarModulo();
void corregirSabotajeBlocks(char* , char);
void haySabotajeSizeEn(char*, char );


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

#endif /* I_MONGO_STORE_H_ */
