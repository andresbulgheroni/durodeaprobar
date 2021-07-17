
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
#include<unistd.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>

typedef enum{

	//CONSOLA
	INICIAR_PATOTA_MSG= 1, // MANDA DATA
	EXPULSAR_TRIPULANTE_MSG = 3, //MANDA DATA
	OBTENER_BITACORA_MSG=6, // MANDA DATA
	ERROR_CODIGO=7,
	//RESPUESTA CONSOLA
	OBTENER_BITACORA_RTA= 9, //MANDA DATA
	//TRIPULANTE CON RAM
	SOLICITAR_SIGUIENTE_TAREA= 14, // MANDA DATA
	SOLICITAR_SIGUIENTE_TAREA_RTA= 15, // MANDA DATA
	COMPLETO_TAREAS=25,
	CAMBIO_ESTADO= 12,
	//TRIPULANTE CON MONGO Y RAM
	INFORMAR_MOVIMIENTO_RAM= 16, // MANDA DATA
	INFORMAR_MOVIMIENTO_MONGO= 11, // MANDA DATA
	//TRIPULANTE CON MONGO
	INICIO_TAREA= 17, // MANDA DATA
	FIN_TAREA= 18, // MANDA DATA
	ATENDER_SABOTAJE= 19, // MANDA DATA
	RESOLUCION_SABOTAJE= 20, // MANDA DATA
	//MONGO A DISCORD
	NOTIFICAR_SABOTAJE= 21, // MANDA DATA
	DESCONECTADO = -1, //CUANDO recv devuelve -1,
	OK_MSG =  22,
	FAIL_MSG = 23

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

	uint32_t idTripulante;
	t_coordenadas* coordenadas;

} tripulante_data_msg;

typedef struct {

	uint32_t idPatota;
	uint32_t cant_tripulantes;
	t_list* tripulantes;
	t_string* tareas; // Pasar tal cual esta en el archivo

} iniciar_patota_msg;

typedef struct {

	uint32_t idPatota;
	uint32_t idTripulante;

} expulsar_tripulante_msg;

typedef struct {

	uint32_t idTripulante;

} obtener_bitacora_msg;

typedef struct {

	t_string* bitacora;

} obtener_bitacora_rta;

typedef struct {

	char* nombre_parametros;
	t_coordenadas* coordenadas;
	uint32_t duracion;

} tarea_data_msg;

typedef struct {

	uint32_t idPatota;
	uint32_t idTripulante;

} solicitar_siguiente_tarea_msg;

typedef struct {

	tarea_data_msg* tarea;

} solicitar_siguiente_tarea_rta;

typedef struct{

	uint32_t idPatota;
	uint32_t idTripulante;
	uint32_t estado;

} cambio_estado_msg;

typedef struct {

	uint32_t idPatota;
	uint32_t idTripulante;
	t_coordenadas* coordenadasDestino;

} informar_movimiento_ram_msg;


typedef struct {

	uint32_t idTripulante;
	t_coordenadas* coordenadasOrigen;
	t_coordenadas* coordenadasDestino;

} informar_movimiento_mongo_msg;

typedef struct{

	uint32_t idTripulante;
	t_string* nombreTarea;
	int32_t parametros;

} inicio_tarea_msg;

typedef struct{

	uint32_t idTripulante;
	t_string* nombreTarea;

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
	t_coordenadas* coordenadas;

} notificar_sabotaje_msg;

//Funciones
int crear_conexion(char* ip, char* puerto);
int32_t enviar_paquete(void* mensaje, op_code codigo, int32_t socketCliente);
int32_t iniciar_servidor(char *ip, char *puerto);
int32_t esperar_cliente(int32_t socketServidor);
void liberar_conexion(uint32_t socketCliente);
t_paquete* recibir_paquete(int32_t socket);

op_code string_to_op_code (char* string);
t_paquete* crear_paquete_a_serializar(op_code codigo, void* mensaje);
void* serializar_paquete(t_paquete* paquete, int32_t* size);
void serializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
void serializar_string(void* stream, t_string* string, uint32_t* offset);

void* deserializar_paquete(t_paquete* paquete);
void deserializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
t_string* deserializar_string(void* stream, uint32_t* offset);


//SERIALIZADO MENSAJES

t_buffer* serializar_iniciar_patota_msg(iniciar_patota_msg* mensaje);
t_buffer* serializar_expulsar_tripulante_msg(expulsar_tripulante_msg* mensaje);
t_buffer* serializar_obtener_bitacora_msg(obtener_bitacora_msg* mensaje);
t_buffer* serializar_obtener_bitacora_rta(obtener_bitacora_rta* mensaje);
t_buffer* serializar_solicitar_siguiente_tarea_msg(solicitar_siguiente_tarea_msg* mensaje);
t_buffer* serializar_solicitar_siguiente_tarea_rta(t_string* mensaje);
t_buffer* serializar_cambio_estado_msg(cambio_estado_msg* mensaje);
t_buffer* serializar_informar_movimiento_ram_msg(informar_movimiento_ram_msg* mensaje);
t_buffer* serializar_informar_movimiento_mongo_msg(informar_movimiento_mongo_msg* mensaje);
t_buffer* serializar_inicio_tarea_msg(inicio_tarea_msg* mensaje);
t_buffer* serializar_fin_tarea_msg(fin_tarea_msg* mensaje);
t_buffer* serializar_atender_sabotaje_msg(atender_sabotaje_msg* mensaje);
t_buffer* serializar_resolucion_sabotaje_msg(resolucion_sabotaje_msg* mensaje);
t_buffer* serializar_notificar_sabotaje_msg(notificar_sabotaje_msg* mensaje);
t_buffer* serializar_fail_msg(t_string* mensaje);


//DESERIALIZADO MENSAJES

iniciar_patota_msg* desserializar_iniciar_patota_msg(void* stream);
expulsar_tripulante_msg* desserializar_expulsar_tripulante_msg(void* stream);
obtener_bitacora_msg* desserializar_obtener_bitacora_msg(void* stream);
obtener_bitacora_rta* desserializar_obtener_bitacora_rta(void* stream);
solicitar_siguiente_tarea_msg* desserializar_solicitar_siguiente_tarea_msg(void* stream);
solicitar_siguiente_tarea_rta* desserializar_solicitar_siguiente_tarea_rta(void* stream);
cambio_estado_msg* desserializar_cambio_estado_msg(void* stream);
informar_movimiento_ram_msg* desserializar_informar_movimiento_ram_msg(void* stream);
informar_movimiento_mongo_msg* desserializar_informar_movimiento_mongo_msg(void* stream);
inicio_tarea_msg* desserializar_inicio_tarea_msg(void* stream);
fin_tarea_msg* desserializar_fin_tarea_msg(void* stream);
atender_sabotaje_msg* desserializar_atender_abotaje_msg(void* stream);
resolucion_sabotaje_msg* desserializar_resolucion_sabotaje_msg(void* stream);
notificar_sabotaje_msg* desserializar_notificar_sabotaje_msg(void* stream);
t_string* desserializar_fail_msg(void* stream);

//MISC

t_coordenadas* get_coordenadas(char* posicion);
t_string* get_t_string(char* string);
bool leer_buffer(int32_t codigo);

#endif
