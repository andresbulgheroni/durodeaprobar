/*
 ============================================================================
 Name        : mi_Ram_Hq.c
 Author      : duroDeAProbar
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "mi_Ram_Hq.h"

int main(void) {

	t_config* config;
	t_log* logger;

	config = leer_config();
	logger = crear_log(config_get_string_value(config, "LOG"));

	int32_t conexion = crear_conexion(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO"));


	//Prueba Iniciar Patota
/*

	iniciar_patota_msg* mensaje = malloc(sizeof(iniciar_patota_msg));

	mensaje->cantidadTripulantes = 5;
	mensaje->direccionTareas = get_t_string("src/tareas");
	mensaje->listaPosiciones = list_create();

	list_add(mensaje->listaPosiciones, get_t_string("5|8"));
	list_add(mensaje->listaPosiciones, get_t_string("3|1"));
	list_add(mensaje->listaPosiciones, get_t_string("0|7"));
	list_add(mensaje->listaPosiciones, get_t_string("0|0"));
	list_add(mensaje->listaPosiciones, get_t_string("0|0"));

	enviar_paquete(mensaje, INICIAR_PATOTA, conexion);

	void free_list(t_string* string){
		free(string);
	}

	free(mensaje->direccionTareas);
	list_destroy_and_destroy_elements(mensaje->listaPosiciones, free_list);
	free(mensaje);
*/


/*
	//Prueba Expulsar Tripulante

	expulsar_tripulante_msg* mensaje = malloc(sizeof(expulsar_tripulante_msg));

	mensaje->idTripulante = 5;

	enviar_paquete(mensaje, EXPULSAR_TRIPULANTE_MSG, conexion);

	free(mensaje);

	sleep(5);

	expulsar_tripulante_msg* mensaje1 = malloc(sizeof(expulsar_tripulante_msg));

	mensaje1->idTripulante = 6;

	enviar_paquete(mensaje1, EXPULSAR_TRIPULANTE_MSG, conexion);

	free(mensaje1);

	sleep(5);

	enviar_paquete(NULL, COMPLETO_TAREAS, conexion);

	sleep(5);*/

	informar_movimiento_mongo_msg* mensaje1 = malloc(sizeof(informar_movimiento_mongo_msg));
	mensaje1->coordenadasOrigen = malloc(sizeof(t_coordenadas));
	mensaje1->coordenadasDestino = malloc(sizeof(t_coordenadas));

	mensaje1->idTripulante = 2;
	mensaje1->coordenadasOrigen->posX = 6;
	mensaje1->coordenadasOrigen->posY = 8;
	mensaje1->coordenadasDestino->posX = 5;
	mensaje1->coordenadasDestino->posY = 7;

	enviar_paquete(mensaje1, INFORMAR_MOVIMIENTO_MONGO, conexion);

	free(mensaje1->coordenadasDestino);
	free(mensaje1);

	t_paquete* paquete = recibir_paquete(conexion);
	solicitar_siguiente_tarea_rta* mensaje2 = deserializar_paquete(paquete);

	log_info(logger, "Tarea : %s", mensaje2->tarea->nombre_parametros);
	log_info(logger, "Duracion: %d", mensaje2->tarea->duracion);
	log_info(logger, "Pos: %d|%d", mensaje2->tarea->coordenadas->posX, mensaje2->tarea->coordenadas->posY);

	free(mensaje2->tarea->nombre_parametros);
	free(mensaje2->tarea->coordenadas);
	free(mensaje2->tarea);
	free(mensaje2);

	liberar_conexion(conexion);

	return 0;
}

t_config* leer_config(void)
{

	return config_create("ram.config");

}

t_log* crear_log(char* dir){
	return log_create(dir, "RAM", true, LOG_LEVEL_DEBUG);
}
