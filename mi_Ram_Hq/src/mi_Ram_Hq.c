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

	uint32_t socket = crearConexion(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO"));

	liberaConexion(socket);

	return 0;
}

t_config* leer_config(void)
{

	return config_create("ram.config");

}

t_log* crear_log(char* dir){
	return log_create(dir, "RAM", TRUE, LOG_LEVEL_DEBUG);
}
