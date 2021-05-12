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

	leerConfig();
	crear_log();
	//
	//	uint32_t socket = iniciar_servidor(IP,PUERTO);
	//
	//	uint32_t socketCliente = esperar_cliente(socket);
	//
	//	t_paquete* paquete = recibir_paquete(socketCliente);
	//
	//	buscarMensaje(paquete, socketCliente);
	//
	//	inicializarFS();

	tarea_data_msg* tarea = malloc(sizeof(tarea_data_msg));
	tarea->parametros = 5;
	//char* data = generar_oxigeno(tarea);
	//data = generar_comida(tarea);
	//data = generar_basura(tarea);
	//data = descartar_basura(tarea);
	char* data = consumir_oxigeno(tarea);
	//data = consumir_comida(tarea);
	puts(data);

	return 0;

}

t_dictionary* armar_diccionario(char* stream){
	t_dictionary* diccionario = dictionary_create();

	char** lines = string_split(stream, "\n");

	void add_cofiguration(char *line) {
		if (!string_starts_with(line, "#")) {
			char** keyAndValue = string_n_split(line, 2, "=");
			dictionary_put(diccionario, keyAndValue[0], keyAndValue[1]);
			free(keyAndValue[0]);
			free(keyAndValue);
		}
	}
	string_iterate_lines(lines, add_cofiguration);
	string_iterate_lines(lines, (void*) free);

	free(lines);

	return diccionario;
}

char *config_get_string_value_propio(t_dictionary *self, char *key) {
	return dictionary_get(self, key);
}

char** config_get_array_value_propio(t_dictionary *self, char* key) {
	char* value_in_dictionary = config_get_string_value_propio(self, key);
	return string_get_string_as_array(value_in_dictionary);
}

int config_get_int_value_propio(t_dictionary *self, char *key) {
	char *value = config_get_string_value_propio(self, key);
	return atoi(value);
}

void config_remove_key_propio(t_dictionary *self, char *key) {
	if(dictionary_has_key(self, key)) {
		dictionary_remove_and_destroy(self, key, free);
	}
}

void config_set_value_propio(t_dictionary *self, char *key, char *value) {
	config_remove_key_propio(self, key);
	char* duplicate_value = string_duplicate(value);
	dictionary_put(self, key, (void*)duplicate_value);
}

int contar_caracteres(char* path){

	FILE* fp = fopen(path, "r");

	int count = 0;
	char c;
	for (c = getc(fp); c != EOF; c = getc(fp))

		// Increment count for this character
		count = count + 1;

	// Close the file
	fclose(fp);

	return count;

}


char* generar_oxigeno(tarea_data_msg* tarea){

	char oxigeno = 'O' ;
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Oxigeno.ims");

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		FILE* fp =  txt_open_for_append(rutaTarea);

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&oxigeno, sizeof(char),1, fp);
		}
		fclose(fp);

	}else{
		FILE* fp = fopen(rutaTarea,"w+b");

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&oxigeno, sizeof(char),1, fp);
		}
		fclose(fp);
	}
	return "Funciono";

}

char* consumir_oxigeno(tarea_data_msg* tarea){

	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Oxigeno.ims");
	if(access(rutaTarea, F_OK) != -1){ //Si existe
		uint32_t cantCaracteres = tarea->parametros;

		char oxigeno = 'O' ;
		int diferencia_caracteres = contar_caracteres(rutaTarea) - cantCaracteres;

		remove(rutaTarea);
		FILE* fp = fopen(rutaTarea,"w+b");

		if(diferencia_caracteres >= 0){

			for(int i=0; i < diferencia_caracteres; i++){
				fwrite(&oxigeno, sizeof(char),1, fp);
			}
			fclose(fp);
		}else{

			fclose(fp);

			log_info(logger,"Se quieren borrar mas caracteres de los que hay");

		}

	}else{

		log_info(logger,"no existe el archivo");

	}
	return "BORRO";

}

char* generar_comida(tarea_data_msg* tarea){

	char comida = 'C' ;
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Comida.ims");

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		FILE* fp =  txt_open_for_append(rutaTarea);

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&comida, sizeof(char),1, fp);
		}
		fclose(fp);

	}else{
		FILE* fp = fopen(rutaTarea,"w+b");

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&comida, sizeof(char),1, fp);
		}
		fclose(fp);
	}
	return "Funciono";

}

char* consumir_comida(tarea_data_msg* tarea){

	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Comida.ims");
	if(access(rutaTarea, F_OK) != -1){ //Si existe
		uint32_t caracteres = tarea->parametros;
		char comida = 'C' ;

		int diferencia_caracteres = contar_caracteres(rutaTarea) - caracteres;

		remove(rutaTarea);
		FILE* fp = fopen(rutaTarea,"w+b");

		if(diferencia_caracteres >= 0){

			for(int i=0; i < diferencia_caracteres; i++){
				fwrite(&comida, sizeof(char),1, fp);
			}
			fclose(fp);
		}else{

			fclose(fp);
			log_info(logger,"Se quieren borrar mas caracteres de los que hay");
		}

	}else{

		log_info(logger,"No existe el archivo");

	}
	return "BORRO";

}

char* generar_basura(tarea_data_msg* tarea){

	char basura = 'B' ;
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Basura.ims");

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		FILE* fp =  txt_open_for_append(rutaTarea);

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&basura, sizeof(char),1, fp);
		}
		fclose(fp);

	}else{
		FILE* fp = fopen(rutaTarea,"w+b");

		for(int i=0; i < tarea->parametros; i++){
			fwrite(&basura, sizeof(char),1, fp);
		}
		fclose(fp);
	}
	return "Funciono";

}

char* descartar_basura(tarea_data_msg* tarea){

	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Basura.ims");
	if(access(rutaTarea, F_OK) != -1){ //Si existe
		remove(rutaTarea);
	}else{
		log_info(logger,"no existe el archivo");
	}
	return "BORRO";

}

void buscarMensaje(t_paquete* paquete, uint32_t socket_cliente) {

	switch(paquete->codigo){

	case GENERAR_OXIGENO:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = GENERAR_OXIGENO;
		char* generarOxigeno = generar_oxigeno(mensaje->tarea);
		enviar_paquete(generarOxigeno, estado, socket_cliente);
		free(mensaje);
		break;
	}
	case CONSUMIR_OXIGENO:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = CONSUMIR_OXIGENO;
		char* consumirOxigeno = consumir_oxigeno(mensaje->tarea);
		enviar_paquete(consumirOxigeno, estado, socket_cliente);
		free(mensaje);
		break;
	}
	case GENERAR_COMIDA:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = GENERAR_COMIDA;
		char* generarComida = generar_comida(mensaje->tarea);
		enviar_paquete(generarComida, estado, socket_cliente);
		free(mensaje);
		break;
	}
	case CONSUMIR_COMIDA:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = CONSUMIR_COMIDA;
		char* consumirComida = consumir_comida(mensaje->tarea);
		enviar_paquete(consumirComida, estado, socket_cliente);
		free(mensaje);
		break;
	}
	case GENERAR_BASURA:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = GENERAR_BASURA;
		char* generarBasura = generar_basura(mensaje->tarea);
		enviar_paquete(generarBasura, estado, socket_cliente);
		free(mensaje);
		break;
	}
	case DESCARTAR_BASURA:{
		solicitar_siguiente_tarea_rta* mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete);
		op_code estado = DESCARTAR_BASURA;
		char* descartarBasura = descartar_basura(mensaje->tarea);
		enviar_paquete(descartarBasura, estado, socket_cliente);
		free(mensaje);
		break;
	}
	default:
		log_info(logger,"codigo de operacion incorrecto");
		break;
	}

}

void escribir_archivo(char* rutaArchivo, char* stringAEscribir) {

	FILE *fp = txt_open_for_append(rutaArchivo);

	if (fp == NULL) {
		log_error(logger, "Error al crear archivo %s\n", rutaArchivo);
		exit(1);
	}
	txt_write_in_file(fp, stringAEscribir);
	txt_close_file(fp);

	log_debug(logger, "ARCHIVO %s ACTUALIZADO\n", rutaArchivo);
	return;

}

void crearTodosLosBloquesEnFS() {

	char* rutaArchivoBlock = string_new();
	string_append(&rutaArchivoBlock, PUNTO_MONTAJE);
	string_append(&rutaArchivoBlock, "Blocks.ims");
	escribir_archivo(rutaArchivoBlock, "");
	log_info(logger, "Blocks creados", rutaArchivoBlock);

}

void crearMetadata(){

	char* buffer = string_new();

	char* ruta_carpeta = string_new();
	string_append(&ruta_carpeta, PUNTO_MONTAJE);
	string_append(&ruta_carpeta, "/Metadata/");
	puts(ruta_carpeta);
	crearDirectorio(ruta_carpeta);

	char* ruta_archivo = string_new();
	string_append(&ruta_archivo, ruta_carpeta);
	string_append(&ruta_archivo, "Files");

	FILE *fp;
	fp = fopen(ruta_archivo, "wb+");

	puts("Ingresar BLOCK_SIZE:");
	fgets(buffer, MAX_BUFFER, stdin);
	strtok(buffer, "\n");
	fputs("BLOCK_SIZE=",fp);
	fputs(buffer,fp);

	puts("Ingresar BLOCKS:");
	fgets(buffer, MAX_BUFFER, stdin);
	strtok(buffer, "\n");
	fputs("\nBLOCKS=",fp);
	fputs(buffer,fp);

	puts("Metadata creado");

}

void crearDirectorio(char *path) {

	char *sep = strrchr(path, '/');
	if(sep != NULL) {
		*sep = 0;
		crearDirectorio(path);
		*sep = '/';
	}
	mkdir(path, 0777);

}

bool existeArchivo(char* path) {

	if(access(path, F_OK) != -1) {
		return true;
	} else {
		return false;
	}

}

int chequearMetadata(){

	char* ruta_archivo = string_from_format("%s/Files", PUNTO_MONTAJE);

	if(existeArchivo(ruta_archivo)){

		t_config* metadata = config_create(string_from_format("%s/SuperBloque.ims", ruta_archivo));

		BLOCK_SIZE = atoi(config_get_string_value(metadata, "BLOCK_SIZE"));

		BLOCKS = atoi(config_get_string_value(metadata, "BLOCKS"));

		//config_destroy(metadata);

		log_debug(logger, "cargado metadata");

		return 1;
	}

	return 0;

}

void crearSuperBloque(){

}

void inicializarFS(){

	puts("Buscando metadata...");

	if(chequearMetadata()){

		puts("Metadata existente encontrado");


	} else {

		puts("Metadata no encontrado, crear uno");
		crearMetadata();
		inicializarFS(); // Hace falta ??
		crearSuperBloque();
		crearTodosLosBloquesEnFS();

		// Creo carpeta recetas
		char* ruta_carpeta = string_new();
		string_append(&ruta_carpeta, PUNTO_MONTAJE);
		string_append(&ruta_carpeta, "/Files/Recetas/");
		crearDirectorio(ruta_carpeta);
		//	free(ruta_carpeta);

	}

}

void leerConfig() {

	config = config_create("mongo.config");
	PUERTO = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP = config_get_string_value(config, "IP");
	PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	TIEMPO_SINCRONIZACION = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
	POSICIONES_SABOTAJE = config_get_string_value(config, "POSICIONES_SABOTAJE"); // ver esto, es una lista en realidad

}

void crear_log(){

	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
	puts("Log creado satisfactoriamente");

}
