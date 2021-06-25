#include "i_Mongo_Store.h"

int main(void) {

	//	printf("\ni-Mongo-Store iniciado! PID: %d\n",getpid());
	//	leerConfig();
	//	crear_log();
	//	inicializarFS();
	//	signal(SIGUSR1, sighandler);
	//
	//
	//	//////////////////////////////////////////////// Pruebas Tareas ////////////////////////////////////////////////
	//			estadoSuperBloque();
	//
	//			generarRecurso(190,'O');
	//			estadoSuperBloque();
	//			consumirRecurso(100, 'O');
	//
	//			estadoSuperBloque();
	//	//////////////////////////////////////////////// Pruebas Tareas ////////////////////////////////////////////////
	//
	//	//////////////////////////////////////////////// Pruebas Bitacora ////////////////////////////////////////////////
	//	estadoSuperBloque();
	//
	//	char* bitacora = string_new();
	//	string_append(&bitacora, "fiumba\n");
	//	string_append(&bitacora, "morgan freemaaaaan\n");
	//	string_append(&bitacora, "tucson\n");
	//	string_append(&bitacora, "Quieren bajarme y no saben como hacer, porque este pibito no va a correr\n");
	//	writeBitacora(1, bitacora);
	//	free(bitacora);
	//
	//	char* tareas = readBitacora(1);
	//	printf("\nBitacora:\n\n%s", tareas);
	//
	//	if(tareas != NULL){
	//		free(tareas);
	//	}
	//
	//	estadoSuperBloque();
	//	////////////////////////////////////////////// Pruebas Bitacora ////////////////////////////////////////////////

	////////////////////////////////////////////// Pruebas Sabotajes ////////////////////////////////////////////////
	//	fsckSuperBloque_Bloques();
		fsckSuperBloque_Bitmap();
	//	fsckFiles_BlockCount();
	//	fsckFiles_Blocks();
	//	fsckFiles_Size();
	////////////////////////////////////////////// Pruebas Sabotajes ////////////////////////////////////////////////



	//	int32_t socket_servidor = iniciar_servidor(IP, PUERTO);
	//
	//	while(true){
	//
	//		int32_t socket_cliente = esperar_cliente(socket_servidor);
	//		pthread_t hilo_mensaje;
	//		pthread_create(&hilo_mensaje,NULL,(void*)funcionPruebaDisc,(void*) (&socket_cliente));
	//		pthread_detach(hilo_mensaje);
	//
	//	}

	//	config_destroy(config);
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
		log_info(logger, "Sincronizando archivo blocks...");
		sleep(TIEMPO_SINCRONIZACION);
	}

}

//void funcionPruebaDisc(int32_t* socketCliente){
//
//	bool terminado = false;
//	while (!terminado){
//		t_paquete* paquete = recibir_paquete(*socketCliente);
//		switch(paquete->codigo){
//		case OBTENER_BITACORA_MSG:{
//			puts("funciono");
//			break;
//		}
//		default: terminado = true; break;
//		}
//	}
//	pthread_exit(NULL);
//
//}

void funcionPruebaTrip(int32_t* socketCliente){

	bool terminado = false;
	while (!terminado){

		t_paquete* paquete = recibir_paquete(*socketCliente);

		switch(paquete->codigo){

		case OBTENER_BITACORA_MSG:{

			obtener_bitacora_msg* bitacoraMsg = deserializar_paquete(paquete);
			char* bitacora = readBitacora(bitacoraMsg->idTripulante);
			enviar_paquete(bitacora,OBTENER_BITACORA_RTA,*socketCliente);
			if(bitacora != NULL){
				free(bitacora);
			}

			break;

		} case INFORMAR_MOVIMIENTO_MONGO:{

			informar_movimiento_mongo_msg* movimientoMsg = deserializar_paquete(paquete);
			char* cadena = string_new();
			string_append(&cadena, "Se mueve de ");
			string_append(&cadena, string_itoa(movimientoMsg->coordenadasOrigen->posX));
			string_append(&cadena, "|");
			string_append(&cadena, string_itoa(movimientoMsg->coordenadasOrigen->posY));
			string_append(&cadena, " a ");
			string_append(&cadena, string_itoa(movimientoMsg->coordenadasDestino->posX));
			string_append(&cadena, "|");
			string_append(&cadena, string_itoa(movimientoMsg->coordenadasDestino->posY));
			string_append(&cadena, "\n");
			writeBitacora(movimientoMsg->idTripulante,cadena);
			free(movimientoMsg);
			free(cadena);

			break;

		} case INICIO_TAREA:{

			inicio_tarea_msg* tareaMsg = deserializar_paquete(paquete);
			char* cadena = string_new();
			string_append(&cadena, "Comienza ejecución de tarea ");
			string_append(&cadena, tareaMsg->nombreTarea->string);
			string_append(&cadena, "\n");
			writeBitacora(tareaMsg->idTripulante,cadena);
			hacerTarea(tareaMsg);
			free(tareaMsg);
			free(cadena);

			break;

		} case FIN_TAREA:{

			fin_tarea_msg* tareaMsg = deserializar_paquete(paquete);
			char* cadena = string_new();
			string_append(&cadena, "Se finaliza la tarea ");
			string_append(&cadena, tareaMsg->nombreTarea->string);
			string_append(&cadena, "\n");
			writeBitacora(tareaMsg->idTripulante,cadena);
			free(tareaMsg);
			free(cadena);

			break;

		} case ATENDER_SABOTAJE:{

			atender_sabotaje_msg* sabotajeMsg = deserializar_paquete(paquete);
			char* cadena = string_new();
			string_append(&cadena, "Se corre en pánico hacia la ubicación del sabotaje");
			string_append(&cadena, "\n");
			writeBitacora(sabotajeMsg->idTripulante,cadena);
			free(sabotajeMsg);
			free(cadena);

			break;

		} case RESOLUCION_SABOTAJE:{

			resolucion_sabotaje_msg* sabotajeMsg = deserializar_paquete(paquete);
			char* cadena = string_new();
			string_append(&cadena, "Se resuelve el sabotaje");
			string_append(&cadena, "\n");
			writeBitacora(sabotajeMsg->idTripulante,cadena);
			free(sabotajeMsg);
			free(cadena);

			break;

		} default:

			terminado = true;
			log_error(logger, "Codigo de op invalido");

			break;

		}

		free(paquete);

	}

	pthread_exit(NULL);
}

void hacerTarea(inicio_tarea_msg* tarea) {

	switch(string_to_op_code_tareas(tarea->nombreTarea->string)){

	case GENERAR_OXIGENO:{

		generarRecurso(tarea->parametros, 'O');

		break;

	} case CONSUMIR_OXIGENO:{

		consumirRecurso(tarea->parametros, 'O');

		break;

	} case GENERAR_COMIDA:{

		generarRecurso(tarea->parametros, 'C');

		break;

	} case CONSUMIR_COMIDA:{

		consumirRecurso(tarea->parametros, 'C');

		break;

	} case GENERAR_BASURA:{

		generarRecurso(tarea->parametros, 'B');

		break;

	} case DESCARTAR_BASURA:{

		descartarBasura();

		break;

	} default:

		log_info(logger,"codigo de operacion incorrecto");

		break;

	}

}

void inicializarBlocks() {

	tamanioBlocks = BLOCKS*BLOCK_SIZE;

	char* rutaArchivoBlock = string_from_format("%s/Blocks.ims", PUNTO_MONTAJE);

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

	// DESCOMENTAR CUANDO PRUEBE LAS CONEXIONES

	//	pthread_t hilo_sincro_blocksmap;
	//	pthread_create(&hilo_sincro_blocksmap, NULL,(void*) timerSincronizacion_blocksMap, NULL);

	close(fd);

	log_debug(logger, "ARCHIVO %s LEIDO\n", rutaArchivoBlock);
	free(rutaArchivoBlock);
}

void inicializarSuperBloque(){

	char* ruta_SuperBloque = string_from_format("%s/SuperBloque.ims", PUNTO_MONTAJE);

	int fd = open(ruta_SuperBloque, O_RDWR, 0777);
	if (fd == -1) {
		log_error(logger, "No se pudo abrir el SuperBloque");
		perror("open file");
		free(ruta_SuperBloque);
		exit(1);
	}

	FILE* fp = fopen(ruta_SuperBloque,"r");
	fread(&BLOCK_SIZE,sizeof(uint32_t),1,fp);
	fread(&BLOCKS,sizeof(uint32_t),1,fp);
	fclose(fp);

	int offset = 2 * sizeof(uint32_t);
	int cantidadBloques = BLOCKS / 8;

	superBloqueMap = mmap(NULL, cantidadBloques + offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (superBloqueMap == MAP_FAILED) {
		perror("Error al inicializar SuperBloque");
		close(fd);
		free(ruta_SuperBloque);
		exit(1);
	}

	bitmap = bitarray_create_with_mode(superBloqueMap + offset, cantidadBloques, LSB_FIRST);

	close(fd);

	log_debug(logger, "ARCHIVO %s LEIDO\n", ruta_SuperBloque);
	free(ruta_SuperBloque);
}

void crearSuperBloque(){

	char* ruta_archivo = string_from_format("%s/SuperBloque.ims", PUNTO_MONTAJE);

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
	for (int i = 0; (unsigned)i < tope; i++) {
		bitarray_clean_bit(bitmap, i);
	}

	close(fd);

	log_debug(logger, "ARCHIVO %s CREADO\n", ruta_archivo);
	free(ruta_archivo);
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

int menorEntre(int a, int b){

	if(a>b) return b;
	else    return a;
}

void inicializarFS(){

	if(existeFS()){
		puts("File System existente detectado, recuperando...\n");

		inicializarSuperBloque();
	} else {
		puts("File System NO encontrado, generando...\n");

		BLOCKS = (uint32_t) config_get_int_value(config,"BLOCKS");
		BLOCK_SIZE = (uint32_t) config_get_int_value(config,"BLOCK_SIZE");

		crearDirectorio(PUNTO_MONTAJE);

		crearSuperBloque();

		char* rutaPuntoMontaje = string_from_format("%s/Files", PUNTO_MONTAJE);
		crearDirectorio(rutaPuntoMontaje);

		char* rutaBitacoras = string_from_format("%s/Bitacoras", rutaPuntoMontaje);
		crearDirectorio(rutaBitacoras);

		free(rutaBitacoras);
		free(rutaPuntoMontaje);
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

	char* rutaSuperBloque = string_from_format("%s/SuperBloque.ims", PUNTO_MONTAJE);
	char* rutaBlocks = string_from_format("%s/Blocks.ims", PUNTO_MONTAJE);

	if(existeArchivo(rutaSuperBloque) && existeArchivo(rutaBlocks)){
		free(rutaSuperBloque);
		free(rutaBlocks);
		return 1;
	}else{
		free(rutaSuperBloque);
		free(rutaBlocks);
		return 0;
	}
}

// Devuelve 0 si el Bitmap esta lleno
int primerBloqueLibre(){

	int posicion = 0;
	for(int i=0; (unsigned)i<bitarray_get_max_bit(bitmap); i++){
		if(!bitarray_test_bit(bitmap, i)){
			posicion = i+1;
			break;
		}
	}
	return posicion;
}

/*
string: cadena a grabar en Blocks
blockCount y blocks: parametros a devolver por referencia
 */
void stringToBlocks(char* string, char* blockCount, char* blocks){

	int bloquesGrabados[BLOCKS];
	int longitud = string_length(string);
	int contador = 0;

	while(longitud > 0){
		// length es BLOCK_SIZE, o si es menor solo los bytes a escribir
		char* bloqueAGrabar = string_substring(string, contador * BLOCK_SIZE, menorEntre(longitud, BLOCK_SIZE));
		bloquesGrabados[contador] = writeBlock(bloqueAGrabar, -1);
		contador++;
		longitud -= BLOCK_SIZE;
		free(bloqueAGrabar);
	}

	char* contador_String = string_itoa(contador);
	strcpy(blockCount, contador_String);
	strcpy(blocks, "[");
	for(int i=0; i<contador; i++){
		char* bloqueGrabado = string_itoa(bloquesGrabados[i]);
		strcat(blocks, bloqueGrabado);
		strcat(blocks, ",");
		free(bloqueGrabado);
	}
	blocks[strlen(blocks)-1] = ']';
	blocks[strlen(blocks)] = '\0';
	free(contador_String);
}

// Controlar previamente que no se pase del BLOCK_SIZE, si bloque es -1 busca el primero libre
int writeBlock(char* string, int bloque){

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

	return bloque;
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
	buf[sizeof digest * 2] = '\0';

	return buf;
}

int generarRecurso(int32_t cantidad, char recurso){

	if(cantidad < 1){
		log_error(logger, "Pasame una cantidad a generar positiva capo");
		return EXIT_FAILURE;
	}

	char* rutaMetadata;
	if(recurso == 'O')		rutaMetadata = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	else if(recurso == 'C') rutaMetadata = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	else if(recurso == 'B') rutaMetadata = string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);
	else{
		log_error(logger, "Pasame un recurso valido para generar capo");
		return EXIT_FAILURE;
	}

	if(access(rutaMetadata, F_OK) != -1){ //Existe archivo metadata, lo manejo como config

		t_config* metadata = config_create(rutaMetadata);

		int cantidadVieja = config_get_int_value(metadata, "SIZE");
		int cantidadNueva = cantidad + cantidadVieja;

		char* cantidadNueva_String = string_itoa(cantidadNueva);
		config_set_value(metadata, "SIZE", cantidadNueva_String);

		// Genero nueva cadena en memoria y le calculo MD5
		char* archivo = string_repeat(recurso, cantidadNueva);
		char* MD5 = calcularMD5(archivo);

		char* blocks = string_new();
		string_append(&blocks, config_get_string_value(metadata, "BLOCKS"));

		int blockCount = config_get_int_value(metadata, "BLOCK_COUNT");

		int caracteresEnUltimoBloque = cantidadVieja % BLOCK_SIZE; //0 si esta completo

		if(caracteresEnUltimoBloque > 0){// Termino de llenar último bloque antes de pedir otro
			char** blocksArray = string_get_string_as_array(blocks);
			int ultimoBloque = atoi(blocksArray[blockCount-1]);

			char* porcionAGrabar = string_substring(archivo, cantidadVieja, BLOCK_SIZE - caracteresEnUltimoBloque);

			memcpy(	blocksMap +	(ultimoBloque-1) * BLOCK_SIZE +		caracteresEnUltimoBloque,
					porcionAGrabar,
					BLOCK_SIZE - caracteresEnUltimoBloque * sizeof(char));

			cantidad -= (BLOCK_SIZE - caracteresEnUltimoBloque);

			for(int i=0;i<blockCount;i++){
				free(blocksArray[i]);
			}
			free(blocksArray);
			free(porcionAGrabar);

		}

		if(cantidad > 0){// Lleno nuevos bloques de ser necesario

			char blockCountNuevo[MAX_BUFFER] = "";
			char blocksNuevo[MAX_BUFFER] = "";

			char* archivoRestante = string_substring(archivo, ((caracteresEnUltimoBloque > 0) ? (BLOCK_SIZE - caracteresEnUltimoBloque) : 0) + cantidadVieja, cantidad);
			stringToBlocks(archivoRestante, blockCountNuevo, blocksNuevo);

			blocks[strlen(blocks)-1] = ',';
			memmove(blocksNuevo, blocksNuevo+1, strlen(blocksNuevo));
			string_append(&blocks, blocksNuevo);
			config_set_value(metadata, "BLOCKS", blocks);

			char* blockCountFinal = string_itoa(blockCount + atoi(blockCountNuevo));
			config_set_value(metadata, "BLOCK_COUNT", blockCountFinal);
			free(blockCountFinal);
			free(archivoRestante);
		}

		char* metadataNueva = string_new();
		string_append(&metadataNueva, "SIZE=");
		char* size = string_itoa(config_get_int_value(metadata,"SIZE"));
		string_append(&metadataNueva, size);
		string_append(&metadataNueva, "\nBLOCK_COUNT=");
		char* bcount = string_itoa(config_get_int_value(metadata,"BLOCK_COUNT"));
		string_append(&metadataNueva, bcount);
		string_append(&metadataNueva, "\nBLOCKS=");
		string_append(&metadataNueva, config_get_string_value(metadata,"BLOCKS"));
		string_append(&metadataNueva, "\nCARACTER_LLENADO=");
		string_append(&metadataNueva, config_get_string_value(metadata,"CARACTER_LLENADO"));
		string_append(&metadataNueva, "\nMD5_ARCHIVO=");
		string_append(&metadataNueva, MD5);

		FILE* fp = fopen(rutaMetadata, "w+");
		txt_write_in_file(fp, metadataNueva);
		txt_close_file(fp);

		free(archivo);
		free(blocks);
		free(cantidadNueva_String);
		free(MD5);
		free(size);
		free(bcount);
		free(metadataNueva);
		config_destroy(metadata);

	}else{// No existe archivo metadata, lo creo

		char blockCount[MAX_BUFFER] = "";
		char blocks[MAX_BUFFER] = "";

		char* archivo = string_repeat(recurso, cantidad);

		// Genero MD5 y grabo archivo a bloques
		char* MD5 = calcularMD5(archivo);
		stringToBlocks(archivo, blockCount, blocks);

		char* metadata = string_new();
		string_append(&metadata, "SIZE=");
		char* cantidad_String = string_itoa(cantidad);
		string_append(&metadata, cantidad_String);

		string_append(&metadata, "\nBLOCK_COUNT=");
		string_append(&metadata, blockCount);

		string_append(&metadata, "\nBLOCKS=");
		string_append(&metadata, blocks);

		string_append(&metadata, "\nCARACTER_LLENADO=");
		char* caracterLlenado = string_repeat(recurso, 1);
		string_append(&metadata, caracterLlenado);

		string_append(&metadata, "\nMD5_ARCHIVO=");
		string_append(&metadata, MD5);

		FILE* fp = txt_open_for_append(rutaMetadata);
		txt_write_in_file(fp, metadata);
		txt_close_file(fp);

		free(cantidad_String);
		free(caracterLlenado);
		free(metadata);
		free(archivo);
		free(MD5);
	}
	free(rutaMetadata);
	return EXIT_SUCCESS;
}

int descartarBasura(){
	char* rutaMetadata= string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);

	if(access(rutaMetadata, F_OK) != -1){

		t_config* metadata = config_create(rutaMetadata);
		char** bloques = config_get_array_value(metadata, "BLOCKS");
		int blockCount = config_get_int_value(metadata, "BLOCK_COUNT");

		// Libero bloques
		for(int i = 0; i < blockCount; i++){
			setBitmap(0, atoi(bloques[i]));
		}

		config_destroy(metadata);
		// Borro archivo Basura.ims
		remove(rutaMetadata);

	}else{
		log_error(logger,"No existe el archivo Basura.ims");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int consumirRecurso(int32_t cantidad, char recurso){

	char* rutaMetadata;
	if(recurso == 'O')		rutaMetadata = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	else if(recurso == 'C') rutaMetadata = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	else{
		log_error(logger, "Recurso a consumir invalido");
		return EXIT_FAILURE;
	}

	if(access(rutaMetadata, F_OK) != -1){// Existe la metadata del recurso

		if(cantidad < 1){
			free(rutaMetadata);
			return EXIT_SUCCESS;
		}

		t_config* metadata = config_create(rutaMetadata);
		int sizeViejo = config_get_int_value(metadata, "SIZE");
		int sizeNuevo = sizeViejo - cantidad;

		char** blocksViejo = config_get_array_value(metadata, "BLOCKS");
		int blockCountViejo = config_get_int_value(metadata, "BLOCK_COUNT");

		if(cantidad >= sizeViejo){// Quiero consumir todos los recursos o mas
			//Libero todos los bloques
			for(int i = 0; i < blockCountViejo; i++){
				setBitmap(0, atoi(blocksViejo[i]));
				free(blocksViejo[i]);
			}
			log_info(logger, "Se consumieron todos los recursos \"%c\", archivo vaciado", recurso);
			config_destroy(metadata);
			remove(rutaMetadata); //Borro archivo por la implementacion que hice del FS
			free(rutaMetadata);
			free(blocksViejo);
			return EXIT_SUCCESS;
		}

		int blockCountNuevo = sizeNuevo / BLOCK_SIZE + ((sizeNuevo % BLOCK_SIZE != 0)? 1 : 0);

		// Libero bloques
		while(blockCountViejo > blockCountNuevo){
			setBitmap(0, atoi(blocksViejo[blockCountViejo - 1]));
			//printf("\nLibere el bloque: %d", atoi(blocksViejo[blockCountViejo - 1]));
			free(blocksViejo[blockCountViejo - 1]);
			blockCountViejo--;
		}

		// Armo nueva cadena blocks
		char* blocksNuevo = string_new();
		string_append(&blocksNuevo, "[");
		for(int i = 0; i < blockCountNuevo; i++){
			string_append(&blocksNuevo, blocksViejo[i]);
			string_append(&blocksNuevo, ",");
			free(blocksViejo[i]);
		}
		blocksNuevo[strlen(blocksNuevo)-1] = ']';

		// Genero MD5 nuevo
		char* archivo = string_repeat(recurso, sizeNuevo);
		char* MD5 = calcularMD5(archivo);

		// Escribo nueva metadata
		char* metadataNueva = string_new();

		string_append(&metadataNueva, "SIZE=");
		char* sizeNuevo_String = string_itoa(sizeNuevo);
		string_append(&metadataNueva, sizeNuevo_String);

		string_append(&metadataNueva, "\nBLOCK_COUNT=");
		char* blockCountNuevo_String = string_itoa(blockCountNuevo);
		string_append(&metadataNueva, blockCountNuevo_String);

		string_append(&metadataNueva, "\nBLOCKS=");
		string_append(&metadataNueva, blocksNuevo);

		string_append(&metadataNueva, "\nCARACTER_LLENADO=");
		string_append(&metadataNueva, config_get_string_value(metadata,"CARACTER_LLENADO"));

		string_append(&metadataNueva, "\nMD5_ARCHIVO=");
		string_append(&metadataNueva, MD5);

		FILE* fp = fopen(rutaMetadata, "w+");
		txt_write_in_file(fp, metadataNueva);
		txt_close_file(fp);

		config_destroy(metadata);
		free(archivo);
		free(MD5);
		free(sizeNuevo_String);
		free(blockCountNuevo_String);
		free(blocksNuevo);
		free(blocksViejo);
		free(metadataNueva);

	}else{// No existe la metadata del recurso

		// Informar que no existe el archivo tambien por otro medio?

		log_info(logger, "No existe el archivo de metadata a consumir");
		return EXIT_SUCCESS;
	}
	free(rutaMetadata);
	return EXIT_SUCCESS;
}

// Pasar string con un token separador al final, como \n
int writeBitacora(int32_t tripulante, char* string){

	char* rutaMetadata;
	rutaMetadata = string_from_format("%s/Files/Bitacoras/Tripulante%d.ims", PUNTO_MONTAJE, tripulante);

	if(access(rutaMetadata, F_OK) != -1){ //Existe archivo metadata, lo manejo como config

		t_config* metadata = config_create(rutaMetadata);

		int cantidadVieja = config_get_int_value(metadata, "SIZE");
		int cantidad = string_length(string); // La cantidad de caracteres que voy a escribir
		int cantidadNueva = cantidad + cantidadVieja;

		char* cantidadNueva_String = string_itoa(cantidadNueva);

		char* blocks = string_new();
		string_append(&blocks, config_get_string_value(metadata, "BLOCKS"));

		int blockCount = 0;
		char** arrayBlocks = config_get_array_value(metadata, "BLOCKS");
		while(arrayBlocks[blockCount] != NULL) blockCount++;

		int caracteresEnUltimoBloque = cantidadVieja % BLOCK_SIZE; //0 si esta completo

		if(caracteresEnUltimoBloque > 0){// Termino de llenar último bloque antes de pedir otro
			char** blocksArray = string_get_string_as_array(blocks);
			int ultimoBloque = atoi(blocksArray[blockCount-1]);

			char* porcionAGrabar = string_substring(string, 0, BLOCK_SIZE - caracteresEnUltimoBloque);

			memcpy(	blocksMap +	(ultimoBloque-1) * BLOCK_SIZE +		caracteresEnUltimoBloque,
					porcionAGrabar,
					BLOCK_SIZE - caracteresEnUltimoBloque * sizeof(char));

			cantidad -= (BLOCK_SIZE - caracteresEnUltimoBloque);

			for(int i=0;i<blockCount;i++){
				free(blocksArray[i]);
			}
			free(blocksArray);
			free(porcionAGrabar);
		}

		if(cantidad > 0){// Lleno nuevos bloques de ser necesario

			char blockCountNuevo[MAX_BUFFER] = "";
			char blocksNuevo[MAX_BUFFER] = "";

			char* archivoRestante = string_substring(string,
					((caracteresEnUltimoBloque > 0) ? (BLOCK_SIZE - caracteresEnUltimoBloque) : 0),
					cantidad);
			stringToBlocks(archivoRestante, blockCountNuevo, blocksNuevo);

			blocks[strlen(blocks)-1] = ',';
			memmove(blocksNuevo, blocksNuevo+1, strlen(blocksNuevo));
			string_append(&blocks, blocksNuevo);
			config_set_value(metadata, "BLOCKS", blocks);

			free(archivoRestante);
		}

		char* metadataNueva = string_new();
		string_append(&metadataNueva, "SIZE=");
		string_append(&metadataNueva, cantidadNueva_String);
		string_append(&metadataNueva, "\nBLOCKS=");
		string_append(&metadataNueva, config_get_string_value(metadata,"BLOCKS"));

		FILE* fp = fopen(rutaMetadata, "w+");
		txt_write_in_file(fp, metadataNueva);
		txt_close_file(fp);

		for(int i=0;i<blockCount;i++){
			free(arrayBlocks[i]);
		}
		free(arrayBlocks);
		free(blocks);
		free(cantidadNueva_String);
		free(metadataNueva);
		config_destroy(metadata);

	}else{// No existe archivo metadata, lo creo

		char blockCount[MAX_BUFFER] = "";
		char blocks[MAX_BUFFER] = "";

		stringToBlocks(string, blockCount, blocks);
		char* size = string_itoa(string_length(string));

		char* metadata = string_new();
		string_append(&metadata, "SIZE=");
		string_append(&metadata, size);

		string_append(&metadata, "\nBLOCKS=");
		string_append(&metadata, blocks);

		FILE* fp = txt_open_for_append(rutaMetadata);
		txt_write_in_file(fp, metadata);
		txt_close_file(fp);

		free(size);
		free(metadata);
	}
	free(rutaMetadata);
	return EXIT_SUCCESS;
}

// Hacer free si es != NULL
char* readBitacora(int32_t tripulante){

	char* rutaMetadata;
	rutaMetadata = string_from_format("%s/Files/Bitacoras/Tripulante%d.ims", PUNTO_MONTAJE, tripulante);

	if(access(rutaMetadata, F_OK) != -1){ //Existe archivo metadata de la bitacora

		t_config* metadata = config_create(rutaMetadata);
		int size = config_get_int_value(metadata, "SIZE");
		int blockCount = size / BLOCK_SIZE + ((size % BLOCK_SIZE > 0) ? 1 : 0);

		char* blocks = string_new();
		string_append(&blocks, config_get_string_value(metadata, "BLOCKS"));
		char** blocksArray = string_get_string_as_array(blocks);

		char* bitacora = malloc(size * sizeof(char)+ 1);
		bitacora[size] = '\0';

		int contador = 0;

		while((unsigned)size >= BLOCK_SIZE){
			memcpy(bitacora + (BLOCK_SIZE * contador), blocksMap + (BLOCK_SIZE * (atoi(blocksArray[contador]) - 1)), BLOCK_SIZE);
			size -= BLOCK_SIZE;
			contador++;
		}

		if(size > 0){
			memcpy(bitacora + (BLOCK_SIZE * contador), blocksMap + (BLOCK_SIZE * (atoi(blocksArray[contador]) - 1)), size);
		}

		for(int i=0;i<blockCount;i++){
			free(blocksArray[i]);
		}
		free(blocksArray);
		free(rutaMetadata);
		free(blocks);
		config_destroy(metadata);
		return bitacora;

	}else{// No existe archivo metadata de la tarea
		log_error(logger, "404: Bitacora de tripulante not found");
		free(rutaMetadata);
		return NULL;
	}
}

void sighandler() {

	puts("Me sabotearon");

	// Avisarle al modulo de Andy

}

int fsckSuperBloque_Bloques(){

	char* rutaBlocks = string_from_format("%s/Blocks.ims", PUNTO_MONTAJE);
	char* rutaSuperBloque = string_from_format("%s/SuperBloque.ims", PUNTO_MONTAJE);

	FILE* blocks = fopen(rutaBlocks, "r");
	fseek(blocks, 0L, SEEK_END);
	uint32_t blocksReales = ftell(blocks) / BLOCK_SIZE;	// Tamaño del SuperBloque / tamaño de cada bloque = cantidad de bloques
	fclose(blocks);

	FILE* superbloque = fopen(rutaSuperBloque, "r");
	uint32_t blocksSaboteado;
	fseek(superbloque, sizeof(uint32_t),SEEK_SET);
	fread(&blocksSaboteado, sizeof(uint32_t), 1, superbloque);

	if(blocksReales != blocksSaboteado){
		memcpy(superBloqueMap + sizeof(uint32_t), &blocksReales, sizeof(uint32_t));
		log_info(logger, "Sabotaje en superbloque corregido, cambio BLOCKS = %d por BLOCKS = %d", blocksSaboteado, blocksReales);
	}else{
		log_info(logger, "La cantidad de bloques es correcta, no hubo sabotaje rey");
	}

	fclose(superbloque);
	free(rutaSuperBloque);
	free(rutaBlocks);
	return EXIT_SUCCESS;

}

int fsckSuperBloque_Bitmap(){

	char* rutaArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	char* rutaArchivoComida = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	char* rutaArchivoBasura = string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);
	char* rutaArchivosBitacora = string_from_format("%s/Files/Bitacoras/", PUNTO_MONTAJE);

	t_list* lista = listaArchivosDeBitacora();
	int count = 0;

	while(count< list_size(lista)){

		char* rutaArchivo = string_new();
		string_append(&rutaArchivo,rutaArchivosBitacora);
		string_append(&rutaArchivo,list_get(lista,count));
		string_append(&rutaArchivo, ".ims");
		haySabotajeBitmapEnElArchivo(rutaArchivoBasura);
		free(rutaArchivo);
		count++;

	}

	if(existeArchivo(rutaArchivoOxigeno)){

		haySabotajeBitmapEnElArchivo(rutaArchivoOxigeno);

	}

	if(existeArchivo(rutaArchivoComida)){

		haySabotajeBitmapEnElArchivo(rutaArchivoComida);

	}

	if(existeArchivo(rutaArchivoBasura)){

		haySabotajeBitmapEnElArchivo(rutaArchivoBasura);

	}

	free(lista);
	free(rutaArchivoOxigeno);
	free(rutaArchivoComida);
	free(rutaArchivoBasura);
	free(rutaArchivosBitacora);

	return EXIT_SUCCESS;

}



int fsckFiles_Size(){

	char* rutaArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	char* rutaArchivoComida = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	char* rutaArchivoBasura = string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);

	if(existeArchivo(rutaArchivoOxigeno)){

		t_config* metadata = config_create(rutaArchivoOxigeno);

		int blockCountBloques = 0;
		char** arrayBlocks = config_get_array_value(metadata, "BLOCKS");
		while(arrayBlocks[blockCountBloques] != NULL) blockCountBloques++;

		char* archivoMD5metadata = config_get_string_value(metadata, "MD5");
		int size = config_get_int_value(metadata, "SIZE");
		char* md5oxigeno = string_repeat('O',size);
		char* md5archivo = calcularMD5(md5oxigeno);

		int totalIteraciones = blockCountBloques * BLOCK_SIZE;
		int sizeCorregido = 0;

		if (strcmp(md5archivo,archivoMD5metadata)){

			log_info(logger, "No hay sabotaje del BLOCK_COUNT, todo piola rey");

		}else{

			char* md5archivoCalculado = calcularMD5(md5oxigeno);
			while(sizeCorregido <= totalIteraciones && !strcmp(md5archivoCalculado,archivoMD5metadata)){
				md5oxigeno = string_repeat('O',sizeCorregido);
				md5archivoCalculado = calcularMD5(md5oxigeno);
				sizeCorregido++;
			}

			log_info(logger, "Sabotaje en el SIZE, corrijo %s a %s", size, sizeCorregido);
			config_set_value(metadata,"SIZE", string_itoa(sizeCorregido));
			free(md5oxigeno);
			free(md5archivoCalculado);

		}

		char* metadataFiles = string_new();
		string_append(&metadataFiles,"SIZE=");
		string_append(&metadataFiles, config_get_string_value(metadata, "SIZE"));
		string_append(&metadataFiles,"\nBLOCK_COUNT=");
		string_append(&metadataFiles, config_get_string_value(metadata, "BLOCK_COUNT"));
		string_append(&metadataFiles,"\nBLOCKS=");
		string_append(&metadataFiles,config_get_string_value(metadata, "BLOCKS"));
		string_append(&metadataFiles,"\nCARACTER_LLENADO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "CARACTER_LLENADO"));
		string_append(&metadataFiles,"\nMD5_ARCHIVO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "MD5_ARCHIVO"));

		FILE* fp = fopen(rutaArchivoOxigeno, "w+");
		txt_write_in_file(fp, metadataFiles);
		txt_close_file(fp);


		for(int i=0;i<blockCountBloques;i++){
			free(arrayBlocks[i]);
		}
		free(arrayBlocks);
		free(metadataFiles);
		free(archivoMD5metadata);
		free(md5archivo);

		config_destroy(metadata);

	}

	if(existeArchivo(rutaArchivoComida)){

		t_config* metadata = config_create(rutaArchivoBasura);

		int blockCountBloques = 0;
		char** arrayBlocks = config_get_array_value(metadata, "BLOCKS");
		while(arrayBlocks[blockCountBloques] != NULL) blockCountBloques++;

		char* archivoMD5metadata = config_get_string_value(metadata, "MD5");
		int size = config_get_int_value(metadata, "SIZE");
		char* md5comida = string_repeat('C',size);

		int totalIteraciones = blockCountBloques * BLOCK_SIZE;
		int sizeCorregido = 0;

		if (strcmp(md5comida,archivoMD5metadata)){

			log_info(logger, "No hay sabotaje del BLOCK_COUNT, todo piola rey");

		}else{

			while(sizeCorregido <= totalIteraciones && !strcmp(md5comida,archivoMD5metadata)){
				md5comida = string_repeat('C',sizeCorregido);
				sizeCorregido++;
			}

			log_info(logger, "Sabotaje en el SIZE, corrijo %s a %s", size, sizeCorregido);
			config_set_value(metadata,"SIZE", string_itoa(sizeCorregido));
			free(md5comida);

		}

		char* metadataFiles = string_new();
		string_append(&metadataFiles,"SIZE=");
		string_append(&metadataFiles, config_get_string_value(metadata, "SIZE"));
		string_append(&metadataFiles,"\nBLOCK_COUNT=");
		string_append(&metadataFiles, config_get_string_value(metadata, "BLOCK_COUNT"));
		string_append(&metadataFiles,"\nBLOCKS=");
		string_append(&metadataFiles,config_get_string_value(metadata, "BLOCKS"));
		string_append(&metadataFiles,"\nCARACTER_LLENADO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "CARACTER_LLENADO"));
		string_append(&metadataFiles,"\nMD5_ARCHIVO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "MD5_ARCHIVO"));

		FILE* fp = fopen(rutaArchivoOxigeno, "w+");
		txt_write_in_file(fp, metadataFiles);
		txt_close_file(fp);


		for(int i=0;i<blockCountBloques;i++){
			free(arrayBlocks[i]);
		}
		free(arrayBlocks);
		free(metadataFiles);
		free(archivoMD5metadata);

		config_destroy(metadata);

	}

	if(existeArchivo(rutaArchivoBasura)){

		t_config* metadata = config_create(rutaArchivoBasura);

		int blockCountBloques = 0;
		char** arrayBlocks = config_get_array_value(metadata, "BLOCKS");
		while(arrayBlocks[blockCountBloques] != NULL) blockCountBloques++;

		char* archivoMD5metadata = config_get_string_value(metadata, "MD5");
		int size = config_get_int_value(metadata, "SIZE");
		char* md5basura = string_repeat('B',size);

		int totalIteraciones = blockCountBloques * BLOCK_SIZE;
		int sizeCorregido = 0;

		if (strcmp(md5basura,archivoMD5metadata)){

			log_info(logger, "No hay sabotaje del BLOCK_COUNT, todo piola rey");

		}else{

			while((sizeCorregido <= totalIteraciones) && !strcmp(md5basura,archivoMD5metadata)){
				md5basura = string_repeat('B',sizeCorregido);
				sizeCorregido++;
			}

			log_info(logger, "Sabotaje en el SIZE, corrijo %s a %s", size, sizeCorregido);
			config_set_value(metadata,"SIZE", string_itoa(sizeCorregido));
			free(md5basura);

		}

		char* metadataFiles = string_new();
		string_append(&metadataFiles,"SIZE=");
		string_append(&metadataFiles, config_get_string_value(metadata, "SIZE"));
		string_append(&metadataFiles,"\nBLOCK_COUNT=");
		string_append(&metadataFiles, config_get_string_value(metadata, "BLOCK_COUNT"));
		string_append(&metadataFiles,"\nBLOCKS=");
		string_append(&metadataFiles,config_get_string_value(metadata, "BLOCKS"));
		string_append(&metadataFiles,"\nCARACTER_LLENADO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "CARACTER_LLENADO"));
		string_append(&metadataFiles,"\nMD5_ARCHIVO=");
		string_append(&metadataFiles,config_get_string_value(metadata, "MD5_ARCHIVO"));

		FILE* fp = fopen(rutaArchivoOxigeno, "w+");
		txt_write_in_file(fp, metadataFiles);
		txt_close_file(fp);


		for(int i=0;i<blockCountBloques;i++){
			free(arrayBlocks[i]);
		}
		free(arrayBlocks);
		free(metadataFiles);
		free(archivoMD5metadata);

		config_destroy(metadata);

	}

	free(rutaArchivoOxigeno);
	free(rutaArchivoComida);
	free(rutaArchivoBasura);

	return EXIT_SUCCESS;

}

int fsckFiles_BlockCount(){

	char* rutaArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	char* rutaArchivoComida = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	char* rutaArchivoBasura = string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);

	if(existeArchivo(rutaArchivoOxigeno)){

		haySabotajeCountEnElArchivo(rutaArchivoOxigeno);

	}

	if(existeArchivo(rutaArchivoComida)){

		haySabotajeCountEnElArchivo(rutaArchivoComida);

	}

	if(existeArchivo(rutaArchivoBasura)){

		haySabotajeCountEnElArchivo(rutaArchivoBasura);

	}

	free(rutaArchivoOxigeno);
	free(rutaArchivoComida);
	free(rutaArchivoBasura);

	return EXIT_SUCCESS;

}


int fsckFiles_Blocks(){

	char* rutaArchivoOxigeno = string_from_format("%s/Files/Oxigeno.ims", PUNTO_MONTAJE);
	char* rutaArchivoComida = string_from_format("%s/Files/Comida.ims", PUNTO_MONTAJE);
	char* rutaArchivoBasura = string_from_format("%s/Files/Basura.ims", PUNTO_MONTAJE);

	if(existeArchivo(rutaArchivoOxigeno)){

		t_config* metadata = config_create(rutaArchivoOxigeno);
		char** blocks = config_get_array_value(metadata, "BLOCKS");
		uint32_t size = config_get_int_value(metadata, "SIZE");

		int j = 0;
		int i = 0;
		while(blocks[j]!=NULL){

			if(atoi(blocks[j]) > BLOCKS){
				log_info(logger, "hay sabotaje del BLOCKS");
				i=j;
			} else {
				log_info(logger, "no hay sabotaje del BLOCKS");
			}

			j++;
		}

		if(i != 0){
			consumirRecurso(size,'O');
			generarRecurso(size,'O');
		}

		int x = 0;
		while(blocks[x] != NULL){
			free(blocks[x]);
			x++;
		}
		free(blocks);
		config_destroy(metadata);

	}

	if(existeArchivo(rutaArchivoComida)){

		t_config* metadata = config_create(rutaArchivoComida);
		char** blocks = config_get_array_value(metadata, "BLOCKS");
		uint32_t size = config_get_int_value(metadata, "SIZE");

		int j = 0;
		int i = 0;
		while(blocks[j]!=NULL){

			if(atoi(blocks[j]) > BLOCKS){
				log_info(logger, "hay sabotaje del BLOCKS");
				i=j;
			} else {
				log_info(logger, "no hay sabotaje del BLOCKS");
			}

			j++;
		}

		if(i != 0){
			consumirRecurso(size,'C');
			generarRecurso(size,'C');
		}

		int x = 0;
		while(blocks[x] != NULL){
			free(blocks[x]);
			x++;
		}
		free(blocks);
		config_destroy(metadata);

	}

	if(existeArchivo(rutaArchivoBasura)){

		t_config* metadata = config_create(rutaArchivoBasura);
		char** blocks = config_get_array_value(metadata, "BLOCKS");
		uint32_t size = config_get_int_value(metadata, "SIZE");

		int j = 0;
		int i = 0;
		while(blocks[j]!=NULL){

			if(atoi(blocks[j]) > BLOCKS){
				log_info(logger, "hay sabotaje del BLOCKS");
				i=j;
			} else {
				log_info(logger, "no hay sabotaje del BLOCKS");
			}

			j++;
		}

		if(i != 0){
			descartarBasura();
			generarRecurso(size,'B');
		}

		int x = 0;
		while(blocks[x] != NULL){
			free(blocks[x]);
			x++;
		}
		free(blocks);
		config_destroy(metadata);

	}

	free(rutaArchivoOxigeno);
	free(rutaArchivoComida);
	free(rutaArchivoBasura);

	return EXIT_SUCCESS;

}

void haySabotajeBitmapEnElArchivo(char* rutaArchivo){

	t_config* metadata = config_create(rutaArchivo);
	char** blocks = config_get_array_value(metadata, "BLOCKS");

	int j = 0;
	while(blocks[j]!=NULL){

		if(bitarray_test_bit(bitmap, atoi(blocks[j])-1) == 1){
			log_info(logger, "no hay sabotaje en el Bitmap");
		}else{
			setBitmap(1, atoi(blocks[j]));
			log_info(logger, "Sabotaje en superbloque corregido, seteo BLOCK = %d en 1 en el Bitmap ", atoi(blocks[j])-1);
		}

		j++;

	}

	int i = 0;
	while(blocks[i] != NULL){
		free(blocks[i]);
		i++;
	}
	free(blocks);
	config_destroy(metadata);

}

void haySabotajeCountEnElArchivo(char* directorio){

	t_config* metadata = config_create(directorio);

	int blockCountBloques = 0;
	char** arrayBlocks = config_get_array_value(metadata, "BLOCKS");
	while(arrayBlocks[blockCountBloques] != NULL) blockCountBloques++;

	int blockCountMetadata = config_get_int_value(metadata, "BLOCK_COUNT");

	if (blockCountBloques == blockCountMetadata){
		log_info(logger, "No hay sabotaje del BLOCK_COUNT, todo piola rey");
	}else{
		log_info(logger, "Sabotaje en el BLOCK_COUNT, corrijo %d a %d", blockCountMetadata, blockCountBloques);
		char* blockCountCorregido = string_itoa(blockCountBloques);
		config_set_value(metadata,"BLOCK_COUNT", blockCountCorregido);
		free(blockCountCorregido);
	}

	char* metadataFiles = string_new();
	string_append(&metadataFiles,"SIZE=");
	string_append(&metadataFiles, config_get_string_value(metadata, "SIZE"));
	string_append(&metadataFiles,"\nBLOCK_COUNT=");
	string_append(&metadataFiles, config_get_string_value(metadata, "BLOCK_COUNT"));
	string_append(&metadataFiles,"\nBLOCKS=");
	string_append(&metadataFiles,config_get_string_value(metadata, "BLOCKS"));
	string_append(&metadataFiles,"\nCARACTER_LLENADO=");
	string_append(&metadataFiles,config_get_string_value(metadata, "CARACTER_LLENADO"));
	string_append(&metadataFiles,"\nMD5_ARCHIVO=");
	string_append(&metadataFiles,config_get_string_value(metadata, "MD5_ARCHIVO"));

	FILE* fp = fopen(directorio, "w+");
	txt_write_in_file(fp, metadataFiles);
	txt_close_file(fp);


	for(int i=0;i<blockCountBloques;i++){
		free(arrayBlocks[i]);
	}
	free(arrayBlocks);
	free(metadataFiles);

	config_destroy(metadata);

}

t_list* listaArchivosDeBitacora() {

	DIR *d;
	t_list* listaArchivos = list_create();
	struct dirent *dir;
	d = opendir("/home/utnso/polus/Files/Bitacoras");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if( strcmp( dir->d_name, "." ) != 0 &&
					strcmp( dir->d_name, ".." ) != 0 ){

				char* cadena = arrayToChar(dir->d_name);
				list_add(listaArchivos,cadena);

			}
		}
		closedir(d);
	}

	return listaArchivos;

}

char* arrayToChar(char cadena[]){

	char* a = string_new();
	for(int i=0; cadena[i]!='\0';i++){

		if(cadena[i]!='\0'){
			char caracter = cadena[i];
			char* aux = malloc(1);
			memcpy(aux,&caracter,1);
			string_append(&a,aux);
			free(aux);
		}
	}

	return a;

}
