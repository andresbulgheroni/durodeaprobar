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

	log_debug(logger, "%d", socketCliente);

	return 0;
}

t_config* leer_config(void)
{

	return config_create("mongo.config");

}

t_log* crear_log(char* dir){

	return log_create(dir, "MONGO", TRUE, LOG_LEVEL_DEBUG);

}
