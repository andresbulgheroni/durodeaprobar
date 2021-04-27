
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
	INICIAR_PATOTA= 1, // MANDA DATA
	LISTAR_TRIPULANTES= 2,
	EXPULSAR_TRIPULANTE = 3, //MANDA DATA
	INICIAR_PLANIFICACION = 4,
	PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6, // MANDA DATA
	ERROR_CODIGO=7,
	//RESPUESTA CONSOLA
	LISTAR_TRIPULANTES_RTA= 8, // MANDA DATA
	OBTENER_BITACORA_RTA= 9, //MANDA DATA
	//TRIPULANTE CON RAM
	INICIAR_TRIPULANTE= 10, // MANDA DATA
	RAM_NECESITA_TAREAS= 11,
	RAM_NO_NECESITA_TAREAS= 12,
	INICIAR_TRIPULANTE_TAREAS= 13, // MANDA DATA
	SOLICITAR_SIGUIENTE_TAREA= 14, // MANDA DATA
	SOLICITAR_SIGUIENTE_TAREA_RTA= 15, // MANDA DATA
	//TRIPULANTE CON MONGO Y RAM
	INFORMAR_MOVIMIENTO= 16, // MANDA DATA
	//TRIPULANTE CON MONGO
	INICIO_TAREA= 17, // MANDA DATA
	FIN_TAREA= 18, // MANDA DATA
	ATENDER_SABOTAJE= 19, // MANDA DATA
	RESOLUCION_SABOTAJE= 20, // MANDA DATA
	//MONGO A DISCORD
	NOTIFICAR_SABOTAJE= 21 // MANDA DATA

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
	t_string* direccionTareas;
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
	t_string* status;

} tripulante_data_msg;

typedef struct {

	t_list* tripulantes;

} listar_tripulantes_rta;

typedef struct {

	t_string* bitacora;

} obtener_bitacora_rta;

typedef struct {

	uint32_t idTripulante;
	uint32_t idPatota;

} iniciar_tripulante_msg;

typedef struct {

	t_string* nombre;
	t_string* parametros;
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
	t_string* nombreTarea;

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

} notificar_sabotaje_msg;

//Funciones
int crear_conexion(char* ip, char* puerto);
int32_t enviar_paquete(void* mensaje, op_code codigo, uint32_t socketCliente);
uint32_t iniciar_servidor(char *ip, char *puerto);
uint32_t esperar_cliente(uint32_t socketServidor);
void liberar_conexion(uint32_t socketCliente);
t_paquete* recibir_paquete(uint32_t socket);

op_code string_to_op_code (char* string);
t_paquete* crear_paquete_a_serializar(op_code codigo, void* mensaje);
void* serializar_paquete(t_paquete* paquete);
void serializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
void serializar_string(void* stream, t_string* string, uint32_t* offset);

void* deserializar_paquete(t_paquete* paquete);
void deserializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset);
t_string* deserializar_string(void* stream, uint32_t* offset);


//SERIALIZADO MENSAJES

t_buffer* serializar_iniciar_patota_msg(iniciar_patota_msg* mensaje);
t_buffer* serializar_expulsar_tripulante_msg(expulsar_tripulante_msg* mensaje); //TODO
t_buffer* serializar_obtener_bitacora_msg(obtener_bitacora_msg* mensaje); //TODO
t_buffer* serializar_listar_tripulantes_rta(listar_tripulantes_rta* mensaje); //TODO
t_buffer* serializar_obtener_bitacora_rta(obtener_bitacora_rta* mensaje); //TODO
t_buffer* serializar_iniciar_tripulante_msg(iniciar_tripulante_msg* mensaje); //TODO
t_buffer* serializar_iniciar_tripulante_tareas_msg(iniciar_tripulante_tareas_msg* mensaje); //TODO
t_buffer* serializar_solicitar_siguiente_tarea_msg(solicitar_siguiente_tarea_msg* mensaje); //TODO
t_buffer* serializar_solicitar_siguiente_tarea_rta(solicitar_siguiente_tarea_rta* mensaje); //TODO
t_buffer* serializar_informar_movimiento_msg(informar_movimiento_msg* mensaje); //TODO
t_buffer* serializar_inicio_tarea_msg(inicio_tarea_msg* mensaje); //TODO
t_buffer* serializar_fin_tarea_msg(fin_tarea_msg* mensaje); //TODO
t_buffer* serializar_atender_sabotaje_msg(atender_sabotaje_msg* mensaje); //TODO
t_buffer* serializar_resolucion_sabotaje_msg(resolucion_sabotaje_msg* mensaje); //TODO
t_buffer* serializar_notificar_sabotaje_msg(notificar_sabotaje_msg* mensaje); //TODO


//DESERIALIZADO MENSAJES

iniciar_patota_msg* desserializar_iniciar_patota_msg(void* stream);
expulsar_tripulante_msg* desserializar_expulsar_tripulante_msg(void* stream); //TODO
obtener_bitacora_msg* desserializar_obtener_bitacora_msg(void* stream); //TODO
listar_tripulantes_rta* desserializar_listar_tripulantes_rta(void* stream); //TODO
obtener_bitacora_rta* desserializar_obtener_bitacora_rta(void* stream); //TODO
iniciar_tripulante_msg* desserializar_iniciar_tripulante_msg(void* stream); //TODO
iniciar_tripulante_tareas_msg* desserializar_iniciar_tripulante_tareas_msg(void* stream); //TODO
solicitar_siguiente_tarea_msg* desserializar_solicitar_siguiente_tarea_msg(void* stream); //TODO
solicitar_siguiente_tarea_rta* desserializar_solicitar_siguiente_tarea_rta(void* stream); //TODO
informar_movimiento_msg* desserializar_informar_movimiento_msg(void* stream); //TODO
inicio_tarea_msg* desserializar_inicio_tarea_msg(void* stream); //TODO
fin_tarea_msg* desserializar_fin_tarea_msg(void* stream); //TODO
atender_sabotaje_msg* desserializar_atender_abotaje_msg(void* stream); //TODO
resolucion_sabotaje_msg* desserializar_resolucion_sabotaje_msg(void* stream); //TODO
notificar_sabotaje_msg* desserializar_notificar_sabotaje_msg(void* stream); //TODO


//MISC

t_coordenadas* get_coordenadas(char* posicion);
t_string* get_t_string(char* string);
boolean leer_buffer(op_code codigo_operacion);

#endif
