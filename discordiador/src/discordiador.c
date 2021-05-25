/*
 ============================================================================
 Name        : discordiador.c
 Author      : DuroDeAProbar
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "discordiador.h"

int a=1;


pthread_mutex_t mutex_tripulantes = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sockets = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaNuevos= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaReady = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBloqueadosPorSabotaje = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaEjecutando = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaFinalizados = PTHREAD_MUTEX_INITIALIZER;

uint32_t id_tripulante = 1;
uint32_t id_patota = 1;


int main(void){
	inicializarConfig(config);

	iniciarLog();
	inicializarListasGlobales();
	//iniciarHiloSabotaje();
	crearHiloPlanificador();

	inicializarSemaforoPlanificador();

	puts("hola viajero, soy el discordiador, ¿En que puedo ayudarte?");
	leer_consola();

	puts("LLEGO AL FIN DEL PROGRAMA");

	//config_destroy(config);
	log_destroy(logger);

	return EXIT_SUCCESS;

}

void crearHiloPlanificador(){
	pthread_t hiloPlanificador;
	pthread_create(&hiloPlanificador, NULL, (void*) planificarSegun,NULL);
	pthread_detach(hiloPlanificador);
}

void inicializarListasGlobales(){

	estaPlanificando=1;
	haySabotaje=0;
	numeroHiloTripulante=0;

	listaNuevos = list_create();
	listaReady = list_create();
	listaBloqueados= list_create();
	listaBloqueadosPorSabotaje= list_create();

	listaFinalizados = list_create();

	tripulantes = list_create();

	hilosTripulantes = list_create();
	sem_tripulantes_ejecutar = list_create();

	sem_init(&sem_hiloTripulante,0,1);
}

void inicializarConfig(t_config* config){

	config= config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/discord.config"); //tp-2021-1c-DuroDeAprobar/discordiador
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

	printf("el valor IP RAM: %s\n",IP_MI_RAM_HQ);
	printf("el valor PUERO RAM: %s\n",PUERTO_MI_RAM_HQ);
	printf("el valor IP IMONGO: %s\n",IP_I_MONGO_STORE);
	printf("el valor ALGORITMO: %s\n",ALGORITMO);
	printf("el valor PUERTO IMONGO: %s\n",PUERTO_I_MONGO_STORE);
	printf("el valor MULTITAREA: %d\n",GRADO_MULTITAREA);
	printf("el valor QUANTUM: %d\n",QUANTUM);
	printf("el valor SABOTAJE: %d\n",DURACION_SABOTAJE);
	printf("el valor RETARDO: %d\n",RETARDO_CICLO_CPU);

}

void iniciarLog(){

	logger = log_create("discordiador.log", "discordiador", 0, LOG_LEVEL_INFO);
	puts("Se creo el log del discordiador");

}

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento		//TODO

	sem_init(&sem_planificarMultitarea, 0, GRADO_MULTITAREA);

	sem_init(&sem_pausarPlanificacion,0,0);

	sem_init(&sem_sabotaje,0,1);

	puts("se inicializaron los semaforos correctamente");

}

op_code_consola string_to_op_code_consola (char* string){
		if(strcmp(string, "INICIAR_PATOTA") == 0){
			return INICIAR_PATOTA;
		}
		if(strcmp(string, "LISTAR_TRIPULANTES")  == 0){
			return LISTAR_TRIPULANTES;
		}
		if(strcmp(string, "EXPULSAR_TRIPULANTE")  == 0){
			return EXPULSAR_TRIPULANTE;
		}
		if(strcmp(string, "INICIAR_PLANIFICACION")  == 0){
			return INICIAR_PLANIFICACION;
		}
		if(strcmp(string, "PAUSAR_PLANIFICACION")  == 0){
			return PAUSAR_PLANIFICACION;
		}
		if(strcmp(string, "OBTENER_BITACORA") == 0){
			return OBTENER_BITACORA;
		}else{
			return ERROR_CODIGO;
		}
}
op_code_tareas string_to_op_code_tareas (char* string){
		if(strcmp(string, "GENERAR_OXIGENO") == 0){
			return GENERAR_OXIGENO;
		}
		if(strcmp(string, "CONSUMIR_OXIGENO")  == 0){
			return CONSUMIR_OXIGENO;
		}
		if(strcmp(string, "GENERAR_COMIDA")  == 0){
			return GENERAR_COMIDA;
		}
		if(strcmp(string, "CONSUMIR_COMIDA")  == 0){
			return CONSUMIR_COMIDA;
		}
		if(strcmp(string, "GENERAR_BASURA")  == 0){
			return GENERAR_BASURA;
		}
		if(strcmp(string, "DESCARTAR_BASURA") == 0){
			return DESCARTAR_BASURA;
		}else{
			return TAREA_CPU;
		}
}
/*
void iniciarHiloSabotaje(){		//TODO
	pthread_t hiloSabotaje;
	pthread_create(&hiloSabotaje, NULL, (void*) planificarSabotaje,NULL);
	pthread_detach(hiloSabotaje);
}
void planificarSabotaje(){
	while(true){
	uint32_t socketSabotaje = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
	t_paquete*mensaje=recibir_paquete(socketSabotaje);
	sem_wait(&sem_sabotaje);
	haySabotaje=1;
	pasarATodosLosTripulantesAListaBloqueado();
	PasarAEjecutarAlTripulanteMasCercano();
	ponerATodosLosTripulantesAReady();
	haySabotaje=0;
	sem_post(&sem_sabotaje)
	liberar_conexion(socketSabotaje);
	}
}
*/

int getIndexTripulanteEnLista(t_list* lista, t_tripulante* tripulante) {
	if (lista->head == NULL)
		return -1;

	t_link_element *element = lista->head;
	t_tripulante* otroTripulante = (t_tripulante*) (lista->head->data);

	int index = 0;
	while(element != NULL) {
		if (otroTripulante->idTripulante == tripulante->idTripulante)
			return index;

		element = element->next;
		otroTripulante = element == NULL ? NULL : element->data;
		index++;
	}

	return -1;
}

char* convertirEnumAString (t_status_code code){

	if(strcmp(string_itoa(code), "1") == 0){
		return "NEW";
	}
	if(strcmp(string_itoa(code), "2") == 0){
		return "READY";
	}
	if(strcmp(string_itoa(code), "3") == 0){
		return "BLOCKED";
	}
	if(strcmp(string_itoa(code), "4") == 0){
		return "EXEC";
	}
	if(strcmp(string_itoa(code), "5") == 0){
		return "FINISHED";
	}
	return "ERROR";

}


void inicializarAtributosATripulante(t_list* posicionesTripulantes){

		int cantidadTripulantes = list_size(posicionesTripulantes);
		pthread_t pthread_id[cantidadTripulantes+numeroHiloTripulante];


	for(int i=0; i<(list_size(posicionesTripulantes)) ; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));
		char* posicion = list_get(posicionesTripulantes,i);
		tripulante->coordenadas = get_coordenadas(posicion);
		tripulante->idTripulante = id_tripulante;
		id_tripulante++;
		tripulante->idPatota = id_patota;
		tripulante->misCiclosDeCPU = 0;
		tripulante->estado = NEW;
		tripulante->quantumDisponible = QUANTUM;

		list_add(tripulantes,tripulante);

		//LE CREO UN HILO.

		sem_wait(&sem_hiloTripulante);
		sem_t* semaforoDelTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoDelTripulante, 1, 0);
		list_add(sem_tripulantes_ejecutar, (void*) semaforoDelTripulante);
		pthread_create(&pthread_id[numeroHiloTripulante], NULL, (void*) ejecutarTripulante, tripulante);
		pthread_detach(pthread_id[numeroHiloTripulante]);

		list_add(hilosTripulantes, &pthread_id[numeroHiloTripulante]);

		numeroHiloTripulante++;


		usleep(250);
	}

	id_patota++;
}


void leer_consola(){ // proximamente recibe como parm uint32_t* socket_server


	char* leido=readline(">");

	while(strcmp(leido, "") != 0){

		log_info(logger, leido);

		char** mensaje = string_split(leido, " ");
		op_code_consola codigo_mensaje = string_to_op_code_consola(mensaje[0]);

		switch(codigo_mensaje){

		case INICIAR_PATOTA: {	//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1 2|1

			uint32_t socketPatota = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);

			t_list* posicionesTripulantes = list_create();
			t_patota* patota = malloc(sizeof(t_patota));
			patota->cantPatota = atoi(mensaje[1]);
			char* rutaTarea = string_new();
			string_append(&rutaTarea,mensaje[2]);
			FILE* fileTarea = fopen(rutaTarea,"r");



			if(fileTarea != NULL){

				struct stat stat_file;
				stat(rutaTarea, &stat_file);
				char* buffer = calloc(1, stat_file.st_size + 1);
				fread(buffer, stat_file.st_size, 1, fileTarea);

				printf("tareas que van a ser asignadas son:%s",buffer);

				iniciar_patota_msg* mensajePatota=malloc(sizeof(iniciar_patota_msg));
				mensajePatota->idPatota=id_patota;
				mensajePatota->tareas=get_t_string(buffer);
				enviar_paquete(mensajePatota,INICIAR_PATOTA_MSG,socketPatota);


				int contadorLista = 0;

				while(mensaje[3+contadorLista]!= NULL){
					list_add(posicionesTripulantes,mensaje[3+contadorLista]);
					contadorLista++;
				}

				contadorLista=0;
				while(mensaje[contadorLista]!= NULL){
					contadorLista++;
				}

				for(int j=0; j <(3 + patota->cantPatota - contadorLista); j++){

					list_add(posicionesTripulantes,"0|0");

				}

				for(int i = 0; i< list_size(posicionesTripulantes);i++){
					char* posiciones = list_get(posicionesTripulantes, i);
					log_info(logger,posiciones);
				}

				inicializarAtributosATripulante(posicionesTripulantes);


				fclose(fileTarea);
				list_destroy(posicionesTripulantes);

				free(mensajePatota->tareas);
				free(mensajePatota);

				liberar_conexion(socketPatota);

			}else{
				log_info(logger, "No existe la tarea");
			}

			break;

		}
		case LISTAR_TRIPULANTES: {

			time_t tiempo = time(0);
			struct tm *tlocal = localtime(&tiempo);
			char output[128];
			strftime(output,128,"%d/%m/%y %H:%M:%S",tlocal);
			printf("\nEstado de la Nave: %s",output);
			int i = 0;

			for(i=0; i<list_size(tripulantes);i++){
				t_tripulante* tripulante = list_get(tripulantes, i);
				printf("\nTripulante: %s Patota: %s Status: %s",
						string_itoa(tripulante->idTripulante),
						string_itoa(tripulante->idPatota),
						convertirEnumAString(tripulante->estado));
			}
			break;
		}

		case EXPULSAR_TRIPULANTE: {


			//ENVIO MENSAJE
			uint32_t socketExpulsar = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
			int id=atoi(mensaje[1]);

			expulsar_tripulante_msg* mensajeExpulsar=malloc(sizeof(expulsar_tripulante_msg));
			mensajeExpulsar->idTripulante=id;
			enviar_paquete(mensajeExpulsar,EXPULSAR_TRIPULANTE_MSG,socketExpulsar);

			bool tieneMismoNombre(void*elemento){
				t_tripulante*tripulante= (t_tripulante*) elemento;
				return tripulante->idTripulante==id;
			}

			t_tripulante*tripulanteExpulsado = list_find(tripulantes,tieneMismoNombre);

			//LO SACO DE TODAS LAS LISTAS Y LO SACO DEL WHILE DEL HILO
			tripulanteExpulsado->fueExpulsado=1;		//ESTO ES UN FLAG.

			if(list_any_satisfy(listaReady,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaReady);
			}
			if(list_any_satisfy(listaBloqueados,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaBloqueados);
			}
			if(list_any_satisfy(listaBloqueadosPorSabotaje,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaBloqueadosPorSabotaje);
			}

				list_remove_by_condition(tripulantes,tieneMismoNombre);
			printf("fui expulsado mi id era:%d",tripulanteExpulsado->idTripulante);

			free(mensajeExpulsar);
			liberar_conexion(socketExpulsar);

			break;
		}
		case INICIAR_PLANIFICACION: { //Con este comando se dará inicio a la planificación (es un semaforo sem init)


			sem_post(&sem_pausarPlanificacion);

//			if(estaPlanificando ==1){
//				pthread_t hiloPlanificador;
//				pthread_create(&hiloPlanificador, NULL, (void*) planificarSegun,NULL);
//				pthread_detach(hiloPlanificador);
//
//			}
//			if(estaPlanificando ==0){
//				estaPlanificando= 1;
//				//sem post a algo
//			}

			break;
		}
		case PAUSAR_PLANIFICACION: { //Este comando lo que busca es detener la planificación en cualquier momento(semaforo)

			sem_wait(&sem_pausarPlanificacion);
		//	estaPlanificando=0;

			//ACA QUITARIA A LOS TRIPULANTES DE LAS RESPECTIVAS LISTAS EN ORDEN. LIST FILTER Y LIST SORT

			break;
		}
		case OBTENER_BITACORA: { //Este comando obtendrá la bitácora del tripulante pasado por parámetro a través de una consulta a i-Mongo-Store.

			//enviarMensaje();
			int socketBitacora = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
			int id=atoi(mensaje[1]);

			obtener_bitacora_msg*mensajeBitacora=malloc(sizeof(obtener_bitacora_msg));
			mensajeBitacora->idTripulante=id;

			enviar_paquete(mensajeBitacora,OBTENER_BITACORA_MSG,socketBitacora);


			//recibirMensaje()
			t_paquete*paqueteBitacora = recibir_paquete(socketBitacora);
			obtener_bitacora_rta*mensajeBitacoraRta=deserializar_paquete(paqueteBitacora);


			printf("el contenido de la bitacora es:%s",mensajeBitacoraRta->bitacora->string);

			liberar_conexion(socketBitacora);


			break;
		}
		case ERROR_CODIGO_CONSOLA: {

			puts(" error en el codigo consola");

			break;
		}
		}


		free(leido);
		leido = readline("\n>");
	}

	free(leido);

}

///////////////////////////////////////FUNCIONES GENERALES///////////////

void log_movimiento_tripulante(uint32_t id, uint32_t coordX, uint32_t coordY)
{
	char* log_msg = "El tripulante con ID %d se movió a la posición [%d,%d]";
	log_info(logger,log_msg, id, coordX, coordY);
}
void log_tripulante_cambio_de_cola_planificacion(uint32_t id, char* razon, char* cola)
{
	char* log_msg = "El tripulante con ID %d cambió a la cola %s porque %s";
	log_info(logger,log_msg, id, cola, razon);
}

int cantidadElementosArray(char** array)
{
	int i = 0;
	while(array[i])
	{
		i++;
	}
	return i;
}

void liberarArray(char** array)
{
	for(int i = 0; i < cantidadElementosArray(array); i++)
		free(array[i]);
	free(array);
}

void sacarTripulanteDeLista(t_tripulante* tripulante, t_list* lista){

	int a = list_size(lista);
	for(int i=0; i<a ; i++){
		t_tripulante* tripulanteDeLista = list_get(lista, i);
		if(tripulante->idTripulante == tripulanteDeLista->idTripulante){
			list_remove(lista, i);
			break;
		}
	}

}


void agregarTripulanteAListaReadyYAvisar(t_tripulante* tripulante){

	tripulante->estado=READY;

	pthread_mutex_lock(&mutex_listaReady);
	list_add(listaReady,tripulante);
	pthread_mutex_unlock(&mutex_listaReady);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado=tripulante->estado;

enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);

printf("cambioDeEstadoAReady el tripulante %d\n",tripulante->idTripulante);
log_info(logger,"cambio De Estado A Ready:%d\n",tripulante->idTripulante);
free(mensaje);

}


void agregarTripulanteAListaExecYAvisar(t_tripulante* tripulante){

	tripulante->estado=EXEC;

	//pthread_mutex_lock(&mutex_listaEjecutando);
	//list_add(listaExec,tripulante);
	//pthread_mutex_lock(&mutex_listaEjecutando);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado=tripulante->estado;

enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);

log_info(logger,"cambio De Estado A Exec:%d\n",tripulante->idTripulante);
free(mensaje);

}


void agregarTripulanteAListaBloqueadosYAvisar(t_tripulante* tripulante){

	tripulante->estado=BLOCKED;

	pthread_mutex_lock(&mutex_listaBloqueados);
	list_add(listaBloqueados,tripulante);
	pthread_mutex_unlock(&mutex_listaBloqueados);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado=tripulante->estado;

enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);
log_info(logger,"cambio De Estado A Bloqueado:%d\n",tripulante->idTripulante);

free(mensaje);

}
void agregarTripulanteAListaBloqueadosPorSabotajeYAvisar(t_tripulante* tripulante){

	tripulante->estado=BLOCKED;

	pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
	list_add(listaBloqueadosPorSabotaje,tripulante);
	pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado=tripulante->estado;
enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);
log_info(logger,"cambio De Estado A BloqueadoPorSabotaje:%d\n",tripulante->idTripulante);
free(mensaje);

}


int distanciaA(t_coordenadas* desde, t_coordenadas* hasta){

	if (desde == NULL || hasta == NULL) {
		return -1;
	}

	int distanciaX = abs(desde->posX - hasta->posX);
	int distanciaY = abs(desde->posY - hasta->posY);

	return distanciaX + distanciaY;

}

int llegoAlSabotaje(t_tripulante* tripulante){

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXsabotaje = tripulante->sabotaje->coordenadas->posX;
	uint32_t posicionYsabotaje = tripulante->sabotaje->coordenadas->posY;

	return (posicionXtripulante == posicionXsabotaje) && (posicionYtripulante == posicionYsabotaje);
}

void moverAlTripulanteHastaElSabotaje(t_tripulante*tripulante){


	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg* mensajeMovimientoSabotaje=malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoSabotaje->idTripulante = tripulante->idTripulante;

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXsabotaje = tripulante->sabotaje->coordenadas->posX;
	uint32_t posicionYsabotaje = tripulante->sabotaje->coordenadas->posY;

	if (posicionXtripulante != posicionXsabotaje) {

		int diferenciaEnX = posicionXsabotaje - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYsabotaje) {

		int diferenciaEnY = posicionYsabotaje - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}

	mensajeMovimientoSabotaje->coordenadasDestino=tripulante->coordenadas;

	tripulante->misCiclosDeCPU++;
	enviar_paquete(mensajeMovimientoSabotaje,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);


}

int llegoATarea(t_tripulante* tripulante){

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	uint32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	return (posicionXtripulante == posicionXtarea) && (posicionYtripulante == posicionYtarea);
}

void moverAlTripulanteHastaLaTarea(t_tripulante*tripulante){
	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg*mensajeMovimientoTarea = malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoTarea->idTripulante = tripulante->idTripulante;


	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	uint32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	if (posicionXtripulante != posicionXtarea) {

		int diferenciaEnX = posicionXtarea - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYtarea) {

		int diferenciaEnY = posicionYtarea - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}
	log_movimiento_tripulante(tripulante->idTripulante,tripulante->coordenadas->posX,tripulante->coordenadas->posY);

	mensajeMovimientoTarea->coordenadasDestino=tripulante->coordenadas;

	enviar_paquete(mensajeMovimientoTarea,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);
	tripulante->misCiclosDeCPU++;

	//sem_wait(&semaforoFinCicloCPU);
}


t_tripulante* tripulanteMasCercanoDelSabotaje(t_sabotaje* sabotaje){
	t_tripulante* tripulanteMasCercanoSabotaje;
	t_tripulante* tripulanteTemporal;

	int distanciaTemporal;
	int menorDistanciaSabotaje = 1000;


	pthread_mutex_lock(&mutex_listaNuevos);
	t_list* tripulantesBloqueadosSabotaje = list_duplicate(listaBloqueadosPorSabotaje);
	pthread_mutex_unlock(&mutex_listaNuevos);

	if(!list_is_empty(tripulantesBloqueadosSabotaje)){
		tripulanteMasCercanoSabotaje = list_get(tripulantesBloqueadosSabotaje, 0);
		menorDistanciaSabotaje = distanciaA(tripulanteMasCercanoSabotaje->coordenadas, sabotaje->coordenadas);


		for(int i = 1; i < tripulantesBloqueadosSabotaje->elements_count; i++){

			if(menorDistanciaSabotaje == 0){
				break;
			}

			tripulanteTemporal = list_get(tripulantesBloqueadosSabotaje, i);
			distanciaTemporal = distanciaA(tripulanteTemporal->coordenadas, sabotaje->coordenadas);

			if(distanciaTemporal < menorDistanciaSabotaje){
				tripulanteMasCercanoSabotaje = tripulanteTemporal;
				menorDistanciaSabotaje = distanciaTemporal;
			}

		}
	}

	list_destroy(tripulantesBloqueadosSabotaje);


		pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
		sacarTripulanteDeLista(tripulanteMasCercanoSabotaje, listaBloqueadosPorSabotaje);
		pthread_mutex_unlock(&mutex_listaBloqueadosPorSabotaje);

		return tripulanteMasCercanoSabotaje;
}


algoritmo_code stringACodigoAlgoritmo(const char* string) {
	for (int i = 0;
			i < sizeof(conversionAlgoritmo) / sizeof(conversionAlgoritmo[0]);
			i++) {
		if (!strcmp(string, conversionAlgoritmo[i].str))
			return conversionAlgoritmo[i].codigo_algoritmo;
	}
	return ERROR_CODIGO_ALGORITMO;
}

void planificarSegun() {

	switch (stringACodigoAlgoritmo(ALGORITMO)) {

		case FIFO:
			planificarSegunFIFO();

			break;

		case RR:
			planificarSegunRR();
			break;



		case ERROR_CODIGO_ALGORITMO:
			break;

		default:
			break;

	}

}



void planificarSegunFIFO(){			//TODO
		while(true){
		sem_wait(&sem_sabotaje);
		sem_wait(&sem_pausarPlanificacion);
		sem_wait(&sem_planificarMultitarea);

		puts("hola soy FIFO que esta pasando?");
		t_tripulante* tripulante = (t_tripulante*) list_remove(listaReady, 0);

		if(tripulante!=NULL){
		printf("FIFO:voy a asignar al tripulante %d\n",tripulante->idTripulante);

		agregarTripulanteAListaExecYAvisar(tripulante);
		log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "fue seleccionado para ejecutar", "EXEC");

		printf("FIFO:voy a planificar a el tripulante %d\n",tripulante->idTripulante);
		sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante-1);
		sem_post(semaforoDelTripulante);


		}else{
			printf("FIFO:no hay nadie a quien ejecutar");
			sleep(RETARDO_CICLO_CPU);
		}

		sem_post(&sem_pausarPlanificacion);
		sem_post(&sem_sabotaje);

		puts("FIFO:el tripulante ya puede planificar");
		}

}

/*void planificarSegunFIFO(){
		while(true){
		sem_wait(&sem_sabotaje);
		sem_wait(&sem_pausarPlanificacion);
		sem_wait(&sem_planificarMultitarea);

		t_tripulante* tripulante = (t_tripulante*) list_remove(listaReady, 0);
		tripulante->estado = EXEC;	//agregarTripulanteAListaExecYAvisar(tripulante);

		log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "fue seleccionado para ejecutar", "EXEC");

		//sem_init(&sem_tripulanteMoviendose, 0, 0);
		sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante);
		sem_post(semaforoDelTripulante);

		sem_wait(&sem_tripulanteMoviendose);

		distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);

		while (distancia != 0 && distancia != -1) {
			sem_post(semaforoDelTripulante);
			sem_wait(&sem_tripulanteMoviendose);
			distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : NULL);
		}
		sem_destroy(&sem_tripulanteMoviendose);

		sem_post(&sem_pausarPlanificacion);
		sem_post(&sem_sabotaje);
		}

}*/






void planificarSegunRR(){
	while(true){
	puts("hola soy RR");

	sem_wait(&sem_sabotaje);
	sem_wait(&sem_pausarPlanificacion);
	sem_wait(&sem_planificarMultitarea);


	//int distancia;

		t_tripulante* tripulante = (t_tripulante*) list_remove(listaReady, 0);

		tripulante->estado = EXEC;

	/*	sem_init(&sem_tripulanteMoviendose, 0, 0);
				sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante);
				sem_post(semaforoDelTripulante);

				sem_wait(&sem_tripulanteMoviendose);
				distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);

				tripulante->quantumDisponible -=1;

				while (((distancia != 0) && (tripulante->quantumDisponible)>0) && (distancia != -1)) {
					sem_post(semaforoDelTripulante);
					sem_wait(&sem_tripulanteMoviendose);
					distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);
					tripulante->quantumDisponible -= 1;
				}

				sem_destroy(&sem_tripulanteMoviendose);

				if(((tripulante->quantumDisponible)==0) && ((tripulante->tareaAsignada != NULL ? !llegoATarea(tripulante) : 0))){
					list_add(listaReady, tripulante);
					tripulante->estado = READY;
				//	log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "se le terminó el quantum", "READY");

					tripulante->quantumDisponible = QUANTUM;
*/

					sem_post(&sem_pausarPlanificacion);
					sem_post(&sem_sabotaje);

}
}

void enviarMensajeDeInicioDeTripulante(t_tripulante*tripulante){
	iniciar_tripulante_msg* mensajeIniciarTripulante=malloc(sizeof(iniciar_tripulante_msg));
	mensajeIniciarTripulante->idPatota=tripulante->idPatota;
	mensajeIniciarTripulante->idTripulante=tripulante->idTripulante;
	mensajeIniciarTripulante->coordenadas=tripulante->coordenadas;

	enviar_paquete(mensajeIniciarTripulante,INICIAR_TRIPULANTE,tripulante->socketTripulanteRam);
	printf("se inicio al tripulante:%d\n",tripulante->idTripulante);

	free(mensajeIniciarTripulante);
}

void ejecutarTripulante(t_tripulante* tripulante){
	//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1



		printf("hola soy:%d\n",tripulante->idTripulante);

		//	//int socketDelTripulanteConImongo = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
		//	//tripulante->socketTripulanteImongo = socketDelTripulanteConImongo;

		int socketDelTripulanteConRam = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
		tripulante->socketTripulanteRam = socketDelTripulanteConRam;

		enviarMensajeDeInicioDeTripulante(tripulante);

		usleep(150);
		agregarTripulanteAListaReadyYAvisar(tripulante);


		solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
		mensajeTarea->idTripulante=tripulante->idTripulante;
		enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
		printf("se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

		/*
		 	//recibirMensaje()
			t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);
			solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);
		 */


		free(mensajeTarea);

		t_tarea*tareaPrueba=malloc(sizeof(t_tarea));
		t_coordenadas*coordenadas=malloc(sizeof(t_coordenadas));
		coordenadas->posX=2;
		coordenadas->posY=2;
			tareaPrueba->nombreTarea="cualquier_tarea";
			tareaPrueba->coordenadas=coordenadas;
			tareaPrueba->duracion=5;
			tripulante->tareaAsignada=tareaPrueba;

		sem_post(&sem_hiloTripulante);


		while(tripulante->estado != FINISHED || tripulante->fueExpulsado == 1){		//TODO



			sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante-1);

			log_info(logger,"el tripulante se queda esperando a ser asignado %d",tripulante->idTripulante);

			sem_wait(semaforoDelTripulante);

			log_info(logger,"fue seleccionado para ejecutar el tripulante %d",tripulante->idTripulante);

			if(stringACodigoAlgoritmo(ALGORITMO)==FIFO){


				if(!llegoATarea(tripulante)){
					int distancia;
					distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);
					log_info(logger,"se esta moviendo a la tarea la tarea el tripulante %d",tripulante->idTripulante);

					while (distancia != 0 && distancia != -1) {
						moverAlTripulanteHastaLaTarea(tripulante);
						distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : NULL);
					}

				}


				if(llegoATarea(tripulante)){
					log_info(logger,"llego a la tarea el tripulante %d",tripulante->idTripulante);

					usleep(100);

					ejecucionDeTareaTripulanteFIFO(tripulante);

					log_info(logger,"finalizo la ejecucion de la tarea el tripulante %d",tripulante->idTripulante);

				}

				if(tripulante->tareaAsignada==NULL){
					log_info(logger,"acaba de solicitar otra tarea el tripulante %d",tripulante->idTripulante);
					/*solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
					mensajeTarea->idTripulante=tripulante->idTripulante;
					enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
					printf("se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

					t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);
					solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);
					tripulante->tareaAsignada=mensajeTareaRta;
					free(mensajeTarea);
					 */
					t_tarea*tareaPrueba=malloc(sizeof(t_tarea));
					t_coordenadas*coordenadas=malloc(sizeof(t_coordenadas));
					coordenadas->posX=3;
					coordenadas->posY=4;
					tareaPrueba->nombreTarea="GENERAR_OXIGENO_AHRE";
					tareaPrueba->coordenadas=coordenadas;
					tareaPrueba->duracion=2;
					tripulante->tareaAsignada=tareaPrueba;

					log_info(logger,"se le asigno otra tarea al tripulante%d",tripulante->idTripulante);
					//	agregarTripulanteAListaReadyYAvisar(tripulante);

					//	sem_post(&sem_planificarMultitarea);		DEBERIA IR ACA DENTRO DEL FIN DE TAREA

				}



			}
			else if(stringACodigoAlgoritmo(ALGORITMO)==RR){
				puts("hola soy RR");
			}
			puts("llegue al final");
			tripulante->estado=FINISHED;


		}

}




void ejecucionDeTareaTripulanteFIFO(t_tripulante*tripulante){

	if(string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)==TAREA_CPU){
		log_info(logger, "el tripulante %d esta realizando una tarea de CPU ",tripulante->idTripulante);
		for(int i=1;i<=tripulante->tareaAsignada->duracion;i++){
			sleep(RETARDO_CICLO_CPU);

			log_info(logger,"El tripulante con ID %d hizo %d de CPU de un total de %d",tripulante->idTripulante,i,tripulante->tareaAsignada->duracion);
		}


	}
	else if(string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)!=TAREA_CPU){


		agregarTripulanteAListaBloqueadosYAvisar(tripulante);
		log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "ES UNA TAREA DE ENTRADA Y SALIDA", "BLOCKED"); //PONER EN LISTA DE BLOQUEADOS


		sem_post(&sem_planificarMultitarea);		//ES UNA IO PUEDO EJECUTAR OTRO TRIPULANTE


		int r=1;	// DURACION DE LA TAREA BLOQUEADA
		while(tripulante->tareaAsignada->duracion>=r){
			sleep(RETARDO_CICLO_CPU);
			r++;


			log_info(logger,"El tripulante con ID %d hizo %d de IO de un total de %d",tripulante->idTripulante,r,tripulante->tareaAsignada->duracion);



		}
		list_remove(listaBloqueados, getIndexTripulanteEnLista(listaBloqueados,tripulante));		//	aca lo saco de la lista de bloqueo
		agregarTripulanteAListaReadyYAvisar(tripulante);
	}




	//ACA MANDARIA EL MENSAJE DE LA TAREA A IMONGO



	tripulante->tareaAsignada=NULL;

}




void iniciarHiloRETARDO_CICLO_CPU(){
	pthread_t hiloRetardoCiclo;
	pthread_create(&hiloRetardoCiclo, NULL, (void*) sincronizarRETARDO_CICLO_CPU,NULL);
	pthread_detach(hiloRetardoCiclo);
}

void sincronizarRETARDO_CICLO_CPU(){
	while(true){

		sem_wait(&semaforoInicioCicloCPU);
		sleep(RETARDO_CICLO_CPU);
		sem_post(&semaforoFinCicloCPU);

	}
}
















//void ejecutarTripulante(t_tripulante* tripulante){
//
//	//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1 2|2
//	pthread_mutex_lock(&mutex_listaNuevos);
//
//	pthread_mutex_lock(&mutex_sockets);
//	printf("hola soy:%d\n",tripulante->idTripulante);
//	pthread_mutex_unlock(&mutex_sockets);
//
//	//int socketDelTripulanteConImongo = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
//	//tripulante->socketTripulanteImongo = socketDelTripulanteConImongo;
//	int socketDelTripulanteConRam = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
//	tripulante->socketTripulanteRam = socketDelTripulanteConRam;
//
//
//	cambio_estado_msg*mensajeEstado=malloc(sizeof(cambio_estado_msg));
//	mensajeEstado->idTripulante=tripulante->idTripulante;
//	mensajeEstado->estado=tripulante->estado;
//
//	pthread_mutex_lock(&mutex_sockets);
//	enviar_paquete(mensajeEstado,CAMBIO_ESTADO,tripulante->socketTripulanteRam);
//	pthread_mutex_unlock(&mutex_sockets);
//
//	//agregarTripulanteAListaReadyYAvisar(tripulante);
//
//	//solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
//	//mensajeTarea->idTripulante=tripulante->idTripulante;
//	//pthread_mutex_lock(&mutex_sockets);
//	//enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
//
//	//printf("se mando una tarea del tripulante:%d\n",tripulante->idTripulante);
//	//pthread_mutex_unlock(&mutex_sockets);
//
//	free(mensajeEstado);
//	//free(mensajeTarea);
//
//	pthread_mutex_unlock(&mutex_listaNuevos);
//
//	/*			//		t_paquete*paqueteTarea = recibir_paquete(tripulante->socketTripulanteRam);
////			//		obtener_bitacora_rta*mensajeTareaRecibida=deserializar_paquete(paqueteTarea);
//
//
//
//	while(tripulante->estado == FINISHED || tripulante->fueExpulsado == 1){
//
//		sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante);
//				sem_wait(semaforoDelTripulante);
//
//		if(tripulante->tareaAsignada==NULL && estaPlanificando == 1 && haySabotaje == 0){
//
//		}
//	}
//	 */
//}



