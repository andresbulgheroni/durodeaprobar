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

	/*

	Prueba Iniciar Patota

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

	liberar_conexion(conexion);

	return 0;
}

t_config* leer_config(void)
{

	return config_create("ram.config");

}

t_log* crear_log(char* dir){
	return log_create(dir, "RAM", TRUE, LOG_LEVEL_DEBUG);
}
