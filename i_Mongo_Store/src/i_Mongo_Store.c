#include "i_Mongo_Store.h"

int main(void) {

	leerConfig();
	crear_log();

	inicializarFS();


	estadoSuperBloque();
	int32_t dato = 22;
	generarOxigeno(dato);
	estadoSuperBloque();


//	while(1){}

	//	int32_t socket_servidor = iniciar_servidor(IP, PUERTO);
	//
	//	while(true){
	//
	//		int32_t socket_cliente = esperar_cliente(socket_servidor);
	//		pthread_t hilo_mensaje;
	//		pthread_create(&hilo_mensaje,NULL,(void*)funcionPruebaDisc, (void*) (&socket_cliente));
	//		pthread_detach(hilo_mensaje);
	//
	//	}
	//
	//	puts("fin programa");

	return EXIT_SUCCESS;

}

// Reemplazo el main por el que usó Andres para probar la conexion con su modulo, dejo la version vieja para referencia

//int main(void) {
//
//	leerConfig();
//	crear_log();
//
//	inicializarFS();
//
//	int32_t socket = iniciar_servidor(IP,PUERTO);
//
//	int32_t socketDiscordiador = esperar_cliente(socket);
//
//	pthread_t hiloDiscordiador;
//	pthread_create(&hiloDiscordiador, NULL,(void *) funcionPruebaDisc,(void *) socketDiscordiador);
//	pthread_detach(hiloDiscordiador);
//
//	while(1){
//		int32_t socketTripulante = esperar_cliente(socket);
//		pthread_t hiloTripulante;
//		pthread_create(&hiloTripulante, NULL,(void *) funcionPruebaTrip,(void *) socketTripulante);
//		pthread_detach(hiloTripulante);
//	}
//
//	return 0;
//
//}

void timerSincronizacion_blocksMap(){

	while(1){
		msync(blocksMap, tamanioBlocks, MS_SYNC);
		puts("Sincronizado blocks");
		sleep(TIEMPO_SINCRONIZACION);
	}

}

void funcionPruebaDisc(int32_t* socketCliente){

	bool terminado = false;
	while (!terminado){
		t_paquete* paquete = recibir_paquete(*socketCliente);
		switch(paquete->codigo){
		case OBTENER_BITACORA_MSG:{
			puts("funciono");
			break;
		}
		default: terminado = true; break;
		}
	}
	pthread_exit(NULL);

}

void funcionPruebaTrip(int32_t* socketCliente){

	t_paquete* paquete = recibir_paquete(*socketCliente);
	switch(paquete->codigo){

	case INFORMAR_MOVIMIENTO_MONGO:{

		break;

	}
	case INICIO_TAREA:{

		inicio_tarea_msg* tareaMsg = deserializar_paquete(paquete);
		buscarMensaje(tareaMsg);
		break;

	}
	case FIN_TAREA:{

		break;

	}

	case ATENDER_SABOTAJE:{

		break;

	}

	case RESOLUCION_SABOTAJE:{

		break;

	}

	}

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

char* diccionarioFiles_to_char(t_dictionary* dic){

	char* cadena = string_new();
	char* keyAndValue;
	char* valor;

	valor = config_get_string_value_propio(dic, "SIZE");
	keyAndValue = string_from_format("%s=%s\n","SIZE",valor);
	string_append(&cadena, keyAndValue);
	valor = config_get_string_value_propio(dic, "BLOCK_COUNT");
	keyAndValue = string_from_format("%s=%s\n","BLOCK_COUNT",valor);
	string_append(&cadena, keyAndValue);
	valor = config_get_string_value_propio(dic, "BLOCKS");
	keyAndValue = string_from_format("%s=%s\n","BLOCKS",valor);
	string_append(&cadena, keyAndValue);
	valor = config_get_string_value_propio(dic, "CARACTER_LLENADO");
	keyAndValue = string_from_format("%s=%s\n","CARACTER_LLENADO",valor);
	string_append(&cadena, keyAndValue);
	valor = config_get_string_value_propio(dic, "MD5_ARCHIVO");
	keyAndValue = string_from_format("%s=%s","MD5_ARCHIVO",valor);
	string_append(&cadena, keyAndValue);

	return cadena;

}

char* generar_oxigeno(int32_t parametros){

	char* oxigeno = "O";
	char* metadataFiles = string_new();
	string_append(&metadataFiles,"SIZE=");
	string_append(&metadataFiles,"\nBLOCK_COUNT=");
	string_append(&metadataFiles,"\nBLOCKS=");
	string_append(&metadataFiles,"\nCARACTER_LLENADO=");
	string_append(&metadataFiles,oxigeno);
	string_append(&metadataFiles,"\nMD5_ARCHIVO=");
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Oxigeno.ims");
	FILE* fp;

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		fp = fopen(rutaTarea, "r+b");
		//sacar todos los datos del fp como un char* y se lo pasas a la funcion de armar diccionario
		//saca todos valores, y opera como lo de abajo



	}else{

		fp = fopen(rutaTarea, "w+b");
		t_dictionary* dic = armar_diccionario(metadataFiles);
		config_set_value_propio(dic,"SIZE",string_itoa(parametros));
		uint32_t block_count = (parametros / BLOCK_SIZE) + 1;
		config_set_value_propio(dic,"BLOCK_COUNT",string_itoa(block_count));
		//hay que verificar en el bitmap los bloques libres
		//una vez obtenidos los guardamos como el block_count usando string_from_format en el diccionario
		//luego en los bloques que nos dijo el bitmap escribimos el caracter de llenado las veces que dice parametros
		//luego actualizamos el bitmap en 1 en los bloques que escribimos
		char* cadena = diccionarioFiles_to_char(dic);
		txt_write_in_file(fp, cadena);
		dictionary_destroy(dic);
		txt_close_file(fp);

	}
	return "Funciono";
}

char* consumir_oxigeno(int32_t parametros){

	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Oxigeno.ims");
	if(access(rutaTarea, F_OK) != -1){ //Si existe
		uint32_t cantCaracteres = (uint32_t) parametros;

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

char* generar_comida(int32_t parametros){

	char* comida = "C";
	char* metadataFiles = string_new();
	string_append(&metadataFiles,"SIZE=");
	string_append(&metadataFiles,"\nBLOCK_COUNT=");
	string_append(&metadataFiles,"\nBLOCKS=");
	string_append(&metadataFiles,"\nCARACTER_LLENADO=");
	string_append(&metadataFiles,comida);
	string_append(&metadataFiles,"\nMD5_ARCHIVO=");
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Comida.ims");

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		FILE* fp = fopen(rutaTarea, "w+b");
		t_dictionary* dic = armar_diccionario(metadataFiles);
		config_set_value_propio(dic,"SIZE",string_itoa(parametros));
		uint32_t block_count = (parametros / BLOCK_SIZE) + 1;
		config_set_value_propio(dic,"BLOCK_COUNT",string_itoa(block_count));
		//hay que verificar en el bitmap los bloques libres
		//una vez obtenidos los guardamos como el block_count usando string_from_format en el diccionario
		//luego en los bloques que nos dijo el bitmap escribimos el caracter de llenado las veces que dice parametros
		//luego actualizamos el bitmap en 1 en los bloques que escribimos
		char* cadena = diccionarioFiles_to_char(dic);
		txt_write_in_file(fp, cadena);
		dictionary_destroy(dic);
		txt_close_file(fp);

	}else{

		FILE* fp = txt_open_for_append(rutaTarea);
		txt_write_in_file(fp, metadataFiles);
		txt_close_file(fp);

	}
	return "Funciono";
}

char* consumir_comida(int32_t parametros){

	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Comida.ims");
	if(access(rutaTarea, F_OK) != -1){ //Si existe
		uint32_t caracteres = (uint32_t) parametros;
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

char* generar_basura(int32_t parametros){

	char* basura = "B";
	char* metadataFiles = string_new();
	string_append(&metadataFiles,"SIZE=");
	string_append(&metadataFiles,"\nBLOCK_COUNT=");
	string_append(&metadataFiles,"\nBLOCKS=");
	string_append(&metadataFiles,"\nCARACTER_LLENADO=");
	string_append(&metadataFiles,basura);
	string_append(&metadataFiles,"\nMD5_ARCHIVO=");
	char* rutaTarea= string_new();
	string_append(&rutaTarea, PUNTO_MONTAJE);
	string_append(&rutaTarea, "/Files");
	crearDirectorio(rutaTarea);
	string_append(&rutaTarea, "/Basura.ims");

	if(access(rutaTarea, F_OK) != -1){ //Si existe

		FILE* fp = fopen(rutaTarea, "w+b");
		t_dictionary* dic = armar_diccionario(metadataFiles);
		config_set_value_propio(dic,"SIZE",string_itoa(parametros));
		uint32_t block_count = (parametros / BLOCK_SIZE) + 1;
		config_set_value_propio(dic,"BLOCK_COUNT",string_itoa(block_count));
		//hay que verificar en el bitmap los bloques libres
		//una vez obtenidos los guardamos como el block_count usando string_from_format en el diccionario
		//luego en los bloques que nos dijo el bitmap escribimos el caracter de llenado las veces que dice parametros
		//luego actualizamos el bitmap en 1 en los bloques que escribimos
		char* cadena = diccionarioFiles_to_char(dic);
		txt_write_in_file(fp, cadena);
		dictionary_destroy(dic);
		txt_close_file(fp);

	}else{

		FILE* fp = txt_open_for_append(rutaTarea);
		txt_write_in_file(fp, metadataFiles);
		txt_close_file(fp);

	}
	return "Funciono";
}

char* descartar_basura(){

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

void buscarMensaje(inicio_tarea_msg* tarea) {

	switch(string_to_op_code_tareas(tarea->nombreTarea->string)){

	case GENERAR_OXIGENO:{

		char* generarOxigeno = generar_oxigeno(tarea->parametros);
		//		enviar_paquete(generarOxigeno, estado, socket_cliente);
		break;

	}
	case CONSUMIR_OXIGENO:{

		char* consumirOxigeno = consumir_oxigeno(tarea->parametros);
		//		enviar_paquete(consumirOxigeno, estado, socket_cliente);
		break;

	}
	case GENERAR_COMIDA:{

		char* generarComida = generar_comida(tarea->parametros);
		//		enviar_paquete(generarComida, estado, socket_cliente);
		break;

	}
	case CONSUMIR_COMIDA:{

		char* consumirComida = consumir_comida(tarea->parametros);
		//		enviar_paquete(consumirComida, estado, socket_cliente);
		break;

	}
	case GENERAR_BASURA:{

		char* generarBasura = generar_basura(tarea->parametros);
		//		enviar_paquete(generarBasura, estado, socket_cliente);
		break;

	}
	case DESCARTAR_BASURA:{

		char* descartarBasura = descartar_basura();
		//		enviar_paquete(descartarBasura, estado, socket_cliente);
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

void inicializarBlocks() {

	tamanioBlocks = BLOCKS*BLOCK_SIZE;
	char* rutaArchivoBlock = string_new();
	string_append(&rutaArchivoBlock, PUNTO_MONTAJE);
	string_append(&rutaArchivoBlock, "/Blocks.ims");

	int fd = open(rutaArchivoBlock, O_CREAT | O_RDWR, 0777);

	if (fd == -1) {
		log_error(logger, "No se pudo abrir el archivo Blocks");
		perror("open file");
		exit(1);
	}

	ftruncate(fd, tamanioBlocks);

	blocksMap = mmap(NULL, tamanioBlocks, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (blocksMap == MAP_FAILED) {
		perror("mmap");
		close(fd);
		exit(1);
	}

	// Thread con timer para sincronizar mmap a disco, iniciarlo despues del mmap!
	// Corre: msync(blocksMap, tamanioBlocks, MS_SYNC);
	pthread_t hilo_sincro_blocksmap;
	pthread_create(&hilo_sincro_blocksmap, NULL,(void*) timerSincronizacion_blocksMap, NULL);

	close(fd);

	log_debug(logger, "ARCHIVO %s LEIDO\n", rutaArchivoBlock);

}

void inicializarSuperBloque(){

	char* ruta_SuperBloque = string_new();
	string_append(&ruta_SuperBloque, PUNTO_MONTAJE);
	string_append(&ruta_SuperBloque, "/SuperBloque.ims");

	int fd = open(ruta_SuperBloque, O_RDWR, 0777);
	if (fd == -1) {
		log_error(logger, "No se pudo abrir el SuperBloque");
		perror("open file");
		exit(1);
	}

	int offset = 2 * sizeof(uint32_t);
	int cantidadBloques = BLOCKS / 8;

	superBloqueMap = mmap(NULL, cantidadBloques + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (superBloqueMap == MAP_FAILED) {
		perror("Error al inicializar SuperBloque");
		close(fd);
		exit(1);
	}

	bitmap = bitarray_create_with_mode(superBloqueMap + offset, cantidadBloques, LSB_FIRST);

	close(fd);

	log_debug(logger, "ARCHIVO %s LEIDO\n", ruta_SuperBloque);

}

void crearSuperBloque(){

	char* ruta_archivo = string_new();
	string_append(&ruta_archivo, PUNTO_MONTAJE);
	string_append(&ruta_archivo, "/SuperBloque.ims");

	int fd = open(ruta_archivo, O_CREAT | O_RDWR, 0777);
	if (fd == -1) {
		log_error(logger, "No se pudo abrir el bitmap");
		perror("open file");
		exit(1);
	}

	int offset = 2 * sizeof(uint32_t);
	int cantidadBloques = BLOCKS / 8;
	ftruncate(fd, cantidadBloques + offset);

	superBloqueMap = mmap(NULL, cantidadBloques + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (superBloqueMap == MAP_FAILED) {
		perror("superBloque error");
		close(fd);
		exit(1);
	}

	memcpy(superBloqueMap, &BLOCK_SIZE, sizeof(uint32_t));
	memcpy(superBloqueMap+sizeof(uint32_t), &BLOCKS, sizeof(uint32_t));
	msync(superBloqueMap, offset, MS_SYNC);

	bitmap = bitarray_create_with_mode(superBloqueMap + offset, cantidadBloques, LSB_FIRST);

	log_info(logger, "El tamano del bitmap creado es de %d bits.",
			bitarray_get_max_bit(bitmap));
	size_t tope = bitarray_get_max_bit(bitmap);
	for (int i = 0; i < tope; i++) {
		bitarray_clean_bit(bitmap, i);
	}

	close(fd);

	log_debug(logger, "ARCHIVO %s CREADO\n", ruta_archivo);

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

void inicializarFS(){

	if(existeFS()){
		puts("File System existente detectado, recuperando...");
		inicializarSuperBloque();
	} else {
		puts("File System NO encontrado, generando...");
		crearDirectorio(PUNTO_MONTAJE);
		crearSuperBloque();
		crearDirectorio(string_from_format("%s/Files", PUNTO_MONTAJE));
	}
	inicializarBlocks();

}

op_code_tareas string_to_op_code_tareas (char* string){

	op_code_tareas tarea = ERROR;
	if(strcmp(string, "GENERAR_OXIGENO") == 0){
		tarea = GENERAR_OXIGENO;
	}
	if(strcmp(string, "CONSUMIR_OXIGENO")  == 0){
		tarea = CONSUMIR_OXIGENO;
	}
	if(strcmp(string, "GENERAR_COMIDA")  == 0){
		tarea = GENERAR_COMIDA;
	}
	if(strcmp(string, "CONSUMIR_COMIDA")  == 0){
		tarea = CONSUMIR_COMIDA;
	}
	if(strcmp(string, "GENERAR_BASURA")  == 0){
		tarea = GENERAR_BASURA;
	}
	if(strcmp(string, "DESCARTAR_BASURA") == 0){
		tarea = DESCARTAR_BASURA;
	}
	return tarea;

}

void leerConfig() {

	config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/i_Mongo_Store/mongo.config");
	PUERTO = config_get_string_value(config, "PUERTO");
	IP = config_get_string_value(config, "IP");
	PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	TIEMPO_SINCRONIZACION = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
	POSICIONES_SABOTAJE = config_get_string_value(config, "POSICIONES_SABOTAJE"); // ver esto, es una lista en realidad
	BLOCKS= (uint32_t) config_get_int_value(config,"BLOCKS");
	BLOCK_SIZE= (uint32_t) config_get_int_value(config,"BLOCK_SIZE");
}

void crear_log(){

	logger = log_create("mongo.log", "MONGO", 1, LOG_LEVEL_INFO);
	puts("Log creado satisfactoriamente");

}

void setBitmap(int valor, int posicion){
	if(valor == 0){
		bitarray_clean_bit(bitmap, posicion - 1);
	}else{
	bitarray_set_bit(bitmap, posicion - 1);
	}
}

int existeFS(){
	if(existeArchivo(string_from_format("%s/SuperBloque.ims", PUNTO_MONTAJE)) &&
			existeArchivo(string_from_format("%s/Blocks.ims", PUNTO_MONTAJE))){
		return 1;
	}else return 0;
}

// Devuelve 0 si el Bitmap esta lleno
int primerBloqueLibre(){
	int posicion = 0;
	for(int i=0; i<bitarray_get_max_bit(bitmap); i++){
		if(!bitarray_test_bit(bitmap, i)){
			posicion = i+1;
			break;
		}
	}
	return posicion;
}

//Recibe un string a grabar en el archivo Blocks
void stringToBlocks(char caracter, int size, char* blockCount, char* blocks){
	string_append(&blocks, "[");

	int contador = 0;

	while(size > BLOCK_SIZE){
		string_append(&blocks, writeBlock(string_repeat(caracter, (int)BLOCK_SIZE), -1));
		string_append(&blocks, ",");
		size = size - BLOCK_SIZE;
		contador++;
	}

	string_append(&blocks,	writeBlock(string_repeat(caracter, size), -1));
	contador++;

	if(string_ends_with(blocks, ",")){
		blocks[strlen(blocks)-1] = ']';
	}else{
		string_append(&blocks, "]");
	}

	string_append(&blockCount, string_itoa(contador));

}

// Controlar previamente que no se pase del BLOCK_SIZE, si bloque es -1 busca el primero libre
char* writeBlock(char* string, int bloque){
	if(bloque == -1){
		bloque = primerBloqueLibre();
	}
	if (bloque == 0){
		log_error(logger, "File System lleno");
		perror("File System lleno");
		exit(1);
	}
	setBitmap(1, bloque);

	memcpy(blocksMap + ((bloque - 1) * BLOCK_SIZE),
			string,
			strlen(string) * sizeof(char));

	return string_itoa(bloque);
}

/*
Hacer free despues de usar! Ejemplo:
char* output = calcularMD5("aaaaa");
printf("Hash piola: %s", output);
free(output);
*/
char* calcularMD5(char *str) {
	unsigned char digest[16];
	char* buf = malloc(sizeof digest * 2 + 1);

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, str, strlen(str));
	MD5_Final(digest, &ctx);

	for (int i = 0, j = 0; i < 16; i++, j+=2)
		sprintf(buf+j, "%02x", digest[i]);
	buf[sizeof digest * 2] = 0;

	return buf;
}

// Version diego :D
void generarOxigeno(int32_t cantidad){
	char* rutaMetadata = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);

	if(access(rutaMetadata, F_OK) != -1){ //Existe archivo metadata, lo manejo como config

		t_config* metadata = config_create(rutaMetadata);

		printf("\nBlocks: %s", config_get_string_value(metadata, "BLOCKS"));
		config_destroy(metadata);

	}else{// No existe archivo metadata, lo creo

		char* metadata = string_new();

		char* blockCount = string_new();
		char* blocks = string_new();


		char* MD5 = calcularMD5(string_repeat('O', cantidad));


		stringToBlocks('O', (int)cantidad, blockCount, blocks);

		string_append(&metadata, "SIZE=");
		string_append(&metadata, string_itoa(cantidad));

		string_append(&metadata, "\nBLOCK_COUNT=");
		string_append(&metadata, blockCount);

		string_append(&metadata, "\nBLOCKS=");
		string_append(&metadata, blocks);

		string_append(&metadata, "\nCARACTER_LLENADO=");
		string_append(&metadata,"O");

		string_append(&metadata, "\nMD5_ARCHIVO=");
		string_append(&metadata, MD5);

		FILE* fp = txt_open_for_append(rutaMetadata);
		txt_write_in_file(fp, metadata);
		txt_close_file(fp);

		free(MD5);

	}
}
