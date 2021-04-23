
#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>

typedef enum{

	TRUE = 1,
	FALSE = 0

} boolean;

typedef enum{

	INICIAR_PATOTA= 1,
	LISTAR_TRIPULANTES= 2,
	EXPULSAR_TRIPULANTE = 3,
	INICIAR_PLANIFICACION = 4,
	PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6,
	ERROR_CODIGO=7

} opCode;

typedef enum{

    NEW = 1,
    READY = 2,
    BLOCKED = 3,
    EXEC = 4,
    FINISHED = 5

} tStatusCode;

typedef struct
{

	uint32_t size;
	void* stream;

} tBuffer;

typedef struct{

	opCode codigo;
	tBuffer* buffer;

} tPaquete;

typedef struct{

	uint32_t length;
	char* string;

} tString;

int crearConexion(char* ip, char* puerto);
void enviarMensaje(void* mensaje, opCode codigo, uint32_t socketCliente);
uint32_t iniciarServidor(char *ip, char *puerto);
uint32_t esperarCliente(uint32_t socketServidor);
void liberaConexion(uint32_t socketCliente);

opCode stringToOpCode (char* string);
void* serializarPaquete(tPaquete* paquete);
void serializarVariable(void* stream, void* variable, uint32_t size, uint32_t* offset);
void serializarString(void* stream, tString* string, uint32_t* offset);
void deserializarVariable(void* stream, void* variable, uint32_t size, uint32_t* offset);
tString* deserializarString(void* stream, uint32_t* offset);

#endif
