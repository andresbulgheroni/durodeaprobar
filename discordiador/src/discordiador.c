/*
 ============================================================================
 Name        : discordiador.c
 Author      : yoNoFuiFueUnVIrus
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "discordiador.h"
#include <stdio.h>
#include <stdlib.h>



t_config*config;



int main(void){

	inicializarConfig(config);

	puts("!!!hola soy el discordiador!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;

}


void inicializarConfig(t_config* config){


	config= config_create("discord.config");
	if( config==NULL){
		printf("no se pudo leer archivo config");
		exit(2);
	}
	IP_MI_RAM_HQ=config_get_string_value(config,"IP_MI_RAM_HQ");
	PUERTO_MI_RAM_HQ=config_get_string_value(config,"PUERTO_MI_RAM_HQ");
	IP_I_MONGO_STORE=config_get_string_value(config,"IP_I_MONGO_STORE");
	ALGORITMO=config_get_string_value(config,"ALGORITMO");
	PUERTO_I_MONGO_STORE=config_get_string_value(config,"PUERTO_I_MONGO_STORE");
	GRADO_MULTITAREA=config_get_int_value(config,"GRADO_MULTITAREA");
	QUANTUM=config_get_int_value(config,"QUANTUM");
	DURACION_SABOTAJE=config_get_int_value(config,"DURACION_SABOTAJE");
	RETARDO_CICLO_CPU=config_get_int_value(config,"RETARDO_CICLO_CPU");

	printf("el valor es: %s\n",IP_MI_RAM_HQ);
	printf("el valor es: %s\n",PUERTO_MI_RAM_HQ);
	printf("el valor es: %s\n",IP_I_MONGO_STORE);
	printf("el valor es: %s\n",ALGORITMO);
	printf("el valor es: %s\n",PUERTO_I_MONGO_STORE);
	printf("el valor es: %d\n",GRADO_MULTITAREA);
	printf("el valor es: %d\n",QUANTUM);
	printf("el valor es: %d\n",DURACION_SABOTAJE);
	printf("el valor es: %d\n",RETARDO_CICLO_CPU);


}
