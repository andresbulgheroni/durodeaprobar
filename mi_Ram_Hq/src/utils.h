
#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef enum{

	TRUE = 1,
	FALSE = 0

} boolean;

typedef enum{

	//CONSOLA
	INICIAR_PATOTA= 1,
	LISTAR_TRIPULANTES= 2,
	EXPULSAR_TRIPULANTE = 3,
	INICIAR_PLANIFICACION = 4,
	PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6,
	ERROR_CODIGO=7,
	//RESPUESTA CONSOLA
	LISTAR_TRIPULANTES_RTA= 8,
	OBTENER_BITACORA_RTA= 9,
	//TRIPULANTE CON RAM
	INICIAR_TRIPULANTE= 10,
	RAM_NECESITA_TAREAS= 11,
	RAM_NO_NECESITA_TAREAS= 12,
	INICIAR_TRIPULANTE_TAREAS= 13,
	SOLICITAR_SIGUIENTE_TAREA= 14,
	SOLICITAR_SIGUIENTE_TAREA_RTA= 15,
	//TRIPULANTE CON MONGO Y RAM
	INFORMAR_MOVIMIENTO= 16,
	//TRIPULANTE CON MONGO
	INICIO_TAREA= 17,
	FIN_TAREA= 18,
	ATENDER_SABOTAJE= 19,
	RESOLUCION_SABOTAJE= 20,
	//MONGO A DISCORD
	NOTIFICAR_SABOTAJE= 21

} op_code;

typedef enum{

    NEW = 1,
    READY = 2,
    BLOCKED = 3,
    EXEC = 4,
    FINISHED = 5

} t_status_code;

typedef struct
{

	uint32_t size;
	void* stream;

} t_buffer;

typedef struct{

	op_code codigo;
	t_buffer* buffer;

} t_paquete;

typedef struct{

	uint32_t length;
	char* string;

} t_string;

typedef struct
{
	uint32_t posX;
	uint32_t posY;
} t_coordenadas;

// Mensajes

typedef struct {

	uint32_t cantidadTripulantes;
	char* direccionTareas;
	t_list* listaPosiciones;

} iniciar_patota_msg;

typedef struct {

	uint32_t idTripulante;

} expulsar_tripulante_msg;

typedef struct {

	uint32_t idTripulante;

} obtener_bitacora_msg;

typedef struct {

	uint32_t idTripulante;
	uint32_t idPatota;
	char* status;

} tripulante_data_msg;

typedef struct {

	t_list* tripulantes;

} listar_tripulantes_rta;

typedef struct {

	char* bitacora;

} obtener_bitacora_rta;

typedef struct {

	uint32_t idTripulante;
	uint32_t idPatota;

} iniciar_tripulante_msg;

typedef struct {

	char* nombre;
	char* parametros;
	t_coordenadas* coordenadas;
	uint32_t duracion;

} tarea_data_msg;

typedef struct {

	t_list* tareas;

} iniciar_tripulante_tareas_msg;

typedef struct {

	uint32_t idTripulante;

} solicitar_siguiente_tarea_msg;

typedef struct {

	tarea_data_msg* tarea;

} solicitar_siguiente_tarea_rta;

typedef struct {

	uint32_t idTripulante;
	t_coordenadas* coordenadasOrigen;
	t_coordenadas* coordenadasDestino;

} informar_movimiento_msg;

typedef struct{

	uint32_t idTripulante;
	char* nombreTarea;

} inicio_tarea_msg;

typedef struct{

	uint32_t idTripulante;
	char* nombreTarea;

} fin_tarea_msg;

typedef struct{

	uint32_t idTripulante;
	uint32_t idSabotaje;

} atender_sabotaje_msg;

typedef struct{

	uint32_t idTripulante;
	uint32_t idSabotaje;

} resolucion_sabotaje_msg;

typedef struct{

	uint32_t idSabotaje;

} notificar_sabotaje_msg;

//Funciones
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(void* mensaje, op_code codigo, uint32_t socketCliente);
uint32_t iniciar_servidor(char *ip, char *puerto);
uint32_t esperar_cliente(uint32_t socketServidor);
void liberar_conexion(uint32_t socketCliente);

op_code string_to_op_code (char* string);
void* serializar_paquete(t_paquete* paquete);
void serializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
void serializar_string(void* stream, t_string* string, uint32_t* offset);
void deserializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
t_string* deserializar_string(void* stream, uint32_t* offset);


//SERIALIZADO MENSAJES




t_coordenadas* get_coordenadas(char* posicion);

#endif
