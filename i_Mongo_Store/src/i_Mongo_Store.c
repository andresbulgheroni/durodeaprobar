/*
 ============================================================================
 Name        : i_Mongo_Store.c
 Author      : duroDeAProbar
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "i_Mongo_Store.h"

int main(void) {

	t_config* config;
	t_log* logger;

	config = leer_config();
	logger = crear_log(config_get_string_value(config, "LOG"));

	uint32_t socket = iniciar_servidor(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO"));

	uint32_t socketCliente = esperar_cliente(socket);

	t_paquete* paquete = recibir_paquete(socketCliente);

/*

	//Prueba Iniciar Patota

	iniciar_patota_msg* mensaje = deserializar_paquete(paquete);

	log_debug(logger, "%d", mensaje->cantidadTripulantes);
	log_debug(logger, mensaje->direccionTareas->string);

	void mostrar_strings(t_string* string){

		log_debug(logger, string->string);

	}

	list_iterate(mensaje->listaPosiciones, mostrar_strings);

		void free_list(t_string* string){
		free(string);
	}

	free(mensaje->direccionTareas);
	list_destroy_and_destroy_elements(mensaje->listaPosiciones, free_list);
	free(mensaje);

*/

	informar_movimiento_mongo_msg* mensaje = deserializar_paquete(paquete);

	log_info(logger, "TID: %d", mensaje->idTripulante);
	log_info(logger, "Pos: %d|%d", mensaje->coordenadasOrigen->posX, mensaje->coordenadasOrigen->posY);
	log_info(logger, "Pos: %d|%d", mensaje->coordenadasDestino->posX, mensaje->coordenadasDestino->posY);

	free(mensaje->coordenadasDestino);
	free(mensaje);

	t_string* tarea = get_t_string("PRUEBA 2;2;3;5");
	enviar_paquete(tarea, SOLICITAR_SIGUIENTE_TAREA_RTA, socketCliente);

	free(tarea);

	while(1);

	return 0;
}

t_config* leer_config(void)
{

	return config_create("mongo.config");

}

t_log* crear_log(char* dir){

	return log_create(dir, "MONGO", true, LOG_LEVEL_DEBUG);

}
