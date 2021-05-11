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



	puts("hola viajero, soy el discordiador, en que puedo ayudarte?");
	leer_consola();

	puts("LLEGO AL FIN DEL PROGRAMA");

	return EXIT_SUCCESS;

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

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento

	sem_init(&sem_planificar, 0, GRADO_MULTITAREA);

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
void iniciarHiloSabotaje(){
	pthread_t hiloSabotaje;
	pthread_create(&hiloSabotaje, NULL, (void*) planificarSabotaje,NULL);
	pthread_detach(hiloSabotaje);
}
void planificarSabotaje(){
	while(true){
	uint32_t socketSabotaje = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
	t_paquete*mensaje=recibir_paquete(socketSabotaje);
	haySabotaje=1;
	pasarATodosLosRepartidoresAListaBloqueado();
	PasarAEjecutarAlRepartidorMasCercano();
	ponerATodosLosRepartidoresAReady();
	haySabotaje=0;
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
		pthread_mutex_lock(&mutex_tripulantes);
		sem_t* semaforoDelTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoDelTripulante, 0, 0);
		list_add(sem_tripulantes_ejecutar, (void*) semaforoDelTripulante);
		pthread_create(&pthread_id[numeroHiloTripulante], NULL, (void*) ejecutarTripulante, tripulante);
		pthread_detach(pthread_id[numeroHiloTripulante]);

		list_add(hilosTripulantes, &pthread_id[numeroHiloTripulante]);

		numeroHiloTripulante++;
		pthread_mutex_unlock(&mutex_tripulantes);
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

		case INICIAR_PATOTA: {		////	INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1 2|1

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

			liberar_conexion(socketExpulsar);

			break;
		}
		case INICIAR_PLANIFICACION: { //Con este comando se dará inicio a la planificación (es un semaforo sem init)

			//armar un hilo y utilizar flags. armar antes.


			if(estaPlanificando ==1){
				pthread_t hiloPlanificador;
				pthread_create(&hiloPlanificador, NULL, (void*) planificarSegun,NULL);
				pthread_detach(hiloPlanificador);

			}
			if(estaPlanificando ==0){
				estaPlanificando= 1;
				//sem post a algo
			}

			break;
		}
		case PAUSAR_PLANIFICACION: { //Este comando lo que busca es detener la planificación en cualquier momento(semaforo)

			estaPlanificando=0;
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

}


void agregarTripulanteAListaExecYAvisar(t_tripulante* tripulante){

	tripulante->estado=EXEC;

	//pthread_mutex_lock(&mutex_listaEjecutando);
	//list_add(listaExec,tripulante);				TODO
	//pthread_mutex_lock(&mutex_listaEjecutando);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado=tripulante->estado;

enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);

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
	mensajeMovimientoTarea->coordenadasDestino=tripulante->coordenadas;

	tripulante->misCiclosDeCPU++;
	enviar_paquete(mensajeMovimientoTarea,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);
	tripulante->misCiclosDeCPU++;

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

/*
void crearHilosTripulantes() {
	pthread_mutex_lock(&mutex_tripulantes);
	int cantidadTripulantes = list_size(tripulantes);
	pthread_t pthread_id[cantidadTripulantes];

	for (int i = 0; i < cantidadTripulantes; i++) {

		t_tripulante* tripulante = (t_tripulante*) list_get(tripulantes, i); //(t_tripulante*) list_get(Tripulantes, i);
		sem_t* semaforoDelTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoDelTripulante, 0, 0);
		list_add(sem_tripulantes_ejecutar, (void*) semaforoDelTripulante);
		pthread_create(&pthread_id[i], NULL, (void*) ejecutarTripulante, tripulante);
		pthread_detach(pthread_id[i]);

		list_add(hilosTripulantes, &pthread_id[i]);
	}
	pthread_mutex_unlock(&mutex_tripulantes);
}
*/
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





void planificarSegunFIFO(){
	puts("hola soy FIFO");

}
void planificarSegunRR(){
	puts("hola soy RR");
}


void ejecutarTripulante(t_tripulante* tripulante){

	int socketDelTripulanteConRam = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
	int socketDelTripulanteConImongo = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);
	tripulante->socketTripulanteImongo = socketDelTripulanteConImongo;
	tripulante->socketTripulanteRam = socketDelTripulanteConRam;


	//agregarTripulanteAListaReadyYAvisar(tripulante);

	/*if(tripulante->tareaAsignada == NULL){

					solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
					mensajeTarea->idTripulante=tripulante->idTripulante;

					enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);

					t_paquete*paqueteTarea = recibir_paquete(tripulante->socketTripulanteRam);
					obtener_bitacora_rta*mensajeTareaRecibida=deserializar_paquete(paqueteTarea);
	}*/



	while(tripulante->estado == FINISHED || tripulante->fueExpulsado == 1){

		sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante);
				sem_wait(semaforoDelTripulante);

		if(tripulante->tareaAsignada==NULL && estaPlanificando == 1 && haySabotaje == 0){

		}
	}
	}



