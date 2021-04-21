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

	puts("!!!soy Imongo!!"); /* prints !!!Hello World!!! */

	config = leer_config();

	return 0;


}

t_config* leer_config(void)
{

	t_config* config = config_create("mongo.config");
	puts("!!!funca!!");

	return config;

}
