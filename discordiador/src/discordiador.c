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


pthread_mutex_t mutex_tripulantes = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sockets = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaNuevos= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaReady = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBloqueadosPorSabotaje = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaEjecutando = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaFinalizados = PTHREAD_MUTEX_INITIALIZER;

uint32_t id_tripulante_para_enviar=1;
uint32_t id_tripulante = 1;
uint32_t id_patota = 1;


int main(void){

	printf("\n--------------------------------------\n");
	printf("Discordiador iniciado!   PID: %d\n",getpid());
	printf("--------------------------------------\n");
	char* option = readline("\nSeleccione la configuracion que desea utilizar:\n1.\tGeneral\n2.\tDiscordiador_FIFO\n3.\tDiscordiador_RR\n4.\tMemoria_Paginacion\n5.\tMemoria_Segmentacion\n6.\tFile System\n7.\tSabotaje\n");

	leerConfig(option);
	free(option);
	iniciarLog();
	inicializarListasGlobales();
	int32_t*socketSabotaje = malloc(sizeof(int32_t));
	*socketSabotaje = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);

	iniciarHiloSabotaje(socketSabotaje);

	crearHiloPlanificador();
	inicioHiloPlanificarBloqueo();
	inicializarSemaforoPlanificador();

	puts("hola viajero, soy el discordiador, ¿En que puedo ayudarte?");
	leer_consola();

	puts("LLEGO AL FIN DEL PROGRAMA");

	finalizar();
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

	//estaPlanificando=1;
	haySabotaje=0;
	numeroHiloTripulante=0;

	listaNuevos = list_create();
	listaReady = list_create();
	listaBloqueados= list_create();
	listaBloqueadosPorSabotaje= list_create();

	listaEjecutando = list_create();
	listaFinalizados = list_create();

	tripulantes = list_create();

	sem_init(&sem_hiloTripulante,0,1);
}

void iniciarLog(){

	logger = log_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/discordiador.log", "discordiador", 0, LOG_LEVEL_INFO);


}

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento

	sem_init(&sem_planificarMultitarea, 0, GRADO_MULTITAREA);

	sem_init(&sem_pausarPlanificacion,0,0);

	sem_init(&sem_sabotaje,0,0);

	sem_init(&semaforoInicioCicloBloqueado,0,0);



}

void leerConfig(char* option) {

	int opt = atoi(option);

	switch(opt){
		case 1: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/general.config");break;
		case 2: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/discordiador_FIFO.config");break;
		case 3: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/discordiador_RR.config");break;
		case 4: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/memoria_paginacion.config");break;
		case 5: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/memoria_segmentacion.config");break;
		case 6: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/fs.config");break;
		case 7: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/sabotaje.config");break;
		default: config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/discordiador/configs/general.config"); break;
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

void iniciarHiloSabotaje(int32_t*socketSabotaje){
	pthread_t hiloSabotaje;
	pthread_create(&hiloSabotaje, NULL, (void*) planificarSabotaje,(void*) socketSabotaje);
	pthread_detach(hiloSabotaje);
}
void planificarSabotaje(int32_t*socketSabotaje){

	while(true){

		t_paquete*paqueteSabotaje=recibir_paquete(*socketSabotaje);
		if(paqueteSabotaje->codigo != DESCONECTADO){
		notificar_sabotaje_msg*mensajeDeSabotaje=deserializar_paquete(paqueteSabotaje);

		log_info(logger,"recibi el paquete de un sabotaje");

		t_sabotaje*sabotajeActivo=malloc(sizeof(t_sabotaje));
		sabotajeActivo->coordenadas=malloc(sizeof(t_coordenadas));

		sabotajeActivo->coordenadas->posX = mensajeDeSabotaje->coordenadas->posX;
		sabotajeActivo->coordenadas->posY = mensajeDeSabotaje->coordenadas->posY;
		sabotajeActivo->id_sabotaje = mensajeDeSabotaje->idSabotaje;

		log_info(logger,"llego el sabotaje con ID: %d",(int32_t) sabotajeActivo->id_sabotaje);

		haySabotaje=1;

		if(!list_is_empty(listaEjecutando)||!list_is_empty(listaReady)){
		pasarATodosLosTripulantesAListaBloqueado();


		t_tripulante* tripulanteMasCercano;
		tripulanteMasCercano = tripulanteMasCercanoDelSabotaje(sabotajeActivo);

		pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
		sacarTripulanteDeLista(tripulanteMasCercano, listaBloqueadosPorSabotaje);
		pthread_mutex_unlock(&mutex_listaBloqueadosPorSabotaje);

		pasarAEjecutarAlTripulanteMasCercano(sabotajeActivo,tripulanteMasCercano);

		pasarTripulantesAListaReady();

		pthread_mutex_lock(&mutex_listaEjecutando);
		sacarTripulanteDeLista(tripulanteMasCercano, listaEjecutando);
		pthread_mutex_unlock(&mutex_listaEjecutando);

		agregarTripulanteAListaReadyYAvisar(tripulanteMasCercano);		//aca va al final de la lista de ready
		}else{
			log_info(logger,"GANO EL IMPOSTOR pero gracias a cristo rey se desactivo el sabotaje...seguimos");
		}
		if(!list_is_empty(listaBloqueados)){
			void iterador(t_tripulante*tripulante){
				sem_post(tripulante->semaforoDelSabotaje);		//esto es para el tripulante que este bloqueado y justo le llega un sabotaje
			}
			list_iterate(listaBloqueados,(void*) iterador);
		}

		haySabotaje=0;
		sem_post(&sem_sabotaje);
		free(sabotajeActivo->coordenadas);
		free(sabotajeActivo);
	}
	}
}


bool ordenarTripulantesDeMenorIdAMayor(void* elemento1,void*elemento2){

	t_tripulante* tripulante1 =(t_tripulante*) elemento1;
	t_tripulante*tripulante2 =(t_tripulante*) elemento2;

	return  tripulante1->idTripulante > tripulante2->idTripulante;
}

void pasarATodosLosTripulantesAListaBloqueado(){

	if(!list_is_empty(listaEjecutando)){
	if(list_size(listaEjecutando) > 1){
		list_sort(listaEjecutando, (void*) ordenarTripulantesDeMenorIdAMayor);
	}
	int32_t sizeListaEjecutando = list_size(listaEjecutando);
	for(int32_t i=0; i < sizeListaEjecutando;i++){

		pthread_mutex_lock(&mutex_listaEjecutando);
		t_tripulante* tripulanteSacadoDeEjecutar= (t_tripulante*) list_remove(listaEjecutando,0);
		pthread_mutex_unlock(&mutex_listaEjecutando);

		log_info(logger,"se fue a lista Bloqueados por Sabotaje el tripulante con ID:%d\n",tripulanteSacadoDeEjecutar->idTripulante);
		agregarTripulanteAListaBloqueadosPorSabotajeYAvisar(tripulanteSacadoDeEjecutar);
	}
	}

	if(!list_is_empty(listaReady)){
	if(list_size(listaReady) > 1){
		list_sort(listaReady,ordenarTripulantesDeMenorIdAMayor);
	}


	int32_t sizeLista = list_size(listaReady);
	for(int32_t i=0; i < sizeLista ;i++){

		pthread_mutex_lock(&mutex_listaReady);
		t_tripulante* tripulanteSacadoDeReady= (t_tripulante*) list_remove(listaReady,0);
		pthread_mutex_unlock(&mutex_listaReady);

		log_info(logger,"se fue a lista Bloqueados por Sabotaje el tripulante con ID:%d\n",tripulanteSacadoDeReady->idTripulante);

		agregarTripulanteAListaBloqueadosPorSabotajeYAvisar(tripulanteSacadoDeReady);
	}
	}

	//faltaria la parte de bloqueados que iria dentro del tripulante

}

void pasarAEjecutarAlTripulanteMasCercano(t_sabotaje*sabotaje,t_tripulante* tripulanteMasCercano){
	log_info(logger," el tripulante con ID:%d se tiene que moverdesde %d|%d hasta la posicion del sabotaje en %d|%d",tripulanteMasCercano->idTripulante,tripulanteMasCercano->coordenadas->posX,tripulanteMasCercano->coordenadas->posY,sabotaje->coordenadas->posX,sabotaje->coordenadas->posY);
	agregarTripulanteAListaExecYAvisar(tripulanteMasCercano);

	while(!llegoAlSabotaje(tripulanteMasCercano,sabotaje)){

		if(estaPlanificando==0){
			sem_wait(tripulanteMasCercano->semaforoCiclo);
		}

		moverAlTripulanteHastaElSabotaje(tripulanteMasCercano,sabotaje);
		sleep(RETARDO_CICLO_CPU);
	}

	atender_sabotaje_msg*mensajeSabotaje= malloc(sizeof(atender_sabotaje_msg));
	mensajeSabotaje->idSabotaje = sabotaje->id_sabotaje;
	mensajeSabotaje->idTripulante = tripulanteMasCercano->idTripulante;
	enviar_paquete(mensajeSabotaje, ATENDER_SABOTAJE, tripulanteMasCercano->socketTripulanteImongo);
	free(mensajeSabotaje);
	log_info(logger,"se envio el mensaje atenderSabotaje del tripulante con ID:%d",tripulanteMasCercano->idTripulante);

	for(int32_t i=1; DURACION_SABOTAJE >= i; i++){

		if(estaPlanificando==0){
			sem_wait(tripulanteMasCercano->semaforoCiclo);
		}

		sleep(RETARDO_CICLO_CPU);
		log_info(logger,"El tripulante con ID %d hizo %d de SABOTAJE de un total de %d",tripulanteMasCercano->idTripulante,i,DURACION_SABOTAJE);

	}

	resolucion_sabotaje_msg*mensajeSabotajeResolucion= malloc(sizeof(resolucion_sabotaje_msg));
	mensajeSabotajeResolucion->idSabotaje = sabotaje->id_sabotaje;
	mensajeSabotajeResolucion->idTripulante = tripulanteMasCercano->idTripulante;
	enviar_paquete(mensajeSabotajeResolucion, RESOLUCION_SABOTAJE, tripulanteMasCercano->socketTripulanteImongo);
	free(mensajeSabotajeResolucion);

}

void pasarTripulantesAListaReady(){
	int32_t sizeListaBloqueadoSabotaje = list_size(listaBloqueadosPorSabotaje);
	for(uint32_t i=0; sizeListaBloqueadoSabotaje  > i;i++){

		pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
		t_tripulante* tripulanteSacadoDeBloqueadoPorSabotaje= (t_tripulante*) list_remove(listaBloqueadosPorSabotaje,0);
		pthread_mutex_unlock(&mutex_listaBloqueadosPorSabotaje);

		agregarTripulanteAListaReadyYAvisar(tripulanteSacadoDeBloqueadoPorSabotaje);
	}
}



uint32_t getIndexTripulanteEnLista(t_list* lista, t_tripulante* tripulante) {
	if (lista->head == NULL)
		return -1;

	t_link_element *element = lista->head;
	t_tripulante* otroTripulante = (t_tripulante*) (lista->head->data);

	uint32_t index = 0;
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

	uint32_t cantidadTripulantes = list_size(posicionesTripulantes);
	pthread_t pthread_id[cantidadTripulantes+numeroHiloTripulante];


	for(uint32_t i=0; i<(list_size(posicionesTripulantes)) ; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));
		tripulante->tareaAsignada= malloc(sizeof(t_tarea));						//TODO
		tripulante->tareaAsignada->coordenadas = malloc(sizeof(t_coordenadas));
		char* posicion = list_get(posicionesTripulantes,i);
		tripulante->coordenadas = get_coordenadas(posicion);
		tripulante->idTripulante = id_tripulante;
		tripulante->fueExpulsado = 0;
		id_tripulante++;
		tripulante->idPatota = id_patota;
		tripulante->misCiclosDeCPU = 0;
		tripulante->estado = NEW;
		tripulante->quantumDisponible = QUANTUM;


		sem_t* semaforoParaBloquearCicloTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoParaBloquearCicloTripulante, 0, 0);
		tripulante->semaforoBloqueadoTripulante=semaforoParaBloquearCicloTripulante;		//primero sem_destroy y luego el free


		sem_t* semaforoParaCortarCicloTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoParaCortarCicloTripulante, 0, 0);
		tripulante->semaforoCiclo=semaforoParaCortarCicloTripulante;

		list_add(tripulantes,tripulante);

		sem_t* semaforoTripulante = malloc(sizeof(sem_t));
		sem_init(semaforoTripulante, 0, 0);
		tripulante->semaforoDelTripulante=semaforoTripulante;

		sem_t* semaforoSabotaje = malloc(sizeof(sem_t));
		sem_init(semaforoSabotaje, 0, 0);
		tripulante->semaforoDelSabotaje=semaforoSabotaje;
		//LE CREO UN HILO.

		pthread_mutex_lock(&mutex_listaNuevos);
		list_add(listaNuevos,tripulante);
		pthread_mutex_unlock(&mutex_listaNuevos);


		sem_wait(&sem_hiloTripulante);


		pthread_create(&pthread_id[numeroHiloTripulante], NULL, (void*) ejecutarTripulante, tripulante);
		pthread_detach(pthread_id[numeroHiloTripulante]);


		numeroHiloTripulante++;


		usleep(400);
	}

	id_patota++;
}



void leer_consola(){ // proximamente recibe como parm uint32_t* socket_server

	int32_t*socketDiscordiador = malloc(sizeof(int32_t));
	*socketDiscordiador = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);

	int32_t*socketBitacora = malloc(sizeof(int32_t));
	*socketBitacora = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);

	char* leido=readline(">");

	while(strcmp(leido, "") != 0){

		log_info(logger, leido);

		char** mensaje = string_split(leido, " ");
		op_code_consola codigo_mensaje = string_to_op_code_consola(mensaje[0]);

		switch(codigo_mensaje){

		case INICIAR_PATOTA: {	//INICIAR_PATOTA 3 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1 2|1
			//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota2.txt 1|1 2|1
			//INICIAR_PATOTA 1 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota2.txt 1|1
			//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1 2|1
			//INICIAR_PATOTA 1 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt



			t_list* posicionesTripulantes = list_create();

			t_list*posicionesTripulantesParaRam = list_create();	//esto es una lista

			char* rutaTarea = string_new();
			string_append(&rutaTarea,mensaje[2]);
			FILE* fileTarea = fopen(rutaTarea,"r");



			if(fileTarea != NULL){

				struct stat stat_file;
				stat(rutaTarea, &stat_file);
				char* buffer = calloc(1, stat_file.st_size + 1);
				fread(buffer, stat_file.st_size, 1, fileTarea);

				uint32_t cantidadDeTripulantes= atoi(mensaje[1]);

				log_info(logger,"tareas que van a ser asignadas son:%s",buffer);


				iniciar_patota_msg* mensajePatota=malloc(sizeof(iniciar_patota_msg));
				mensajePatota->idPatota=id_patota;
				mensajePatota->tareas=get_t_string(buffer);
				mensajePatota->cant_tripulantes = cantidadDeTripulantes;



				uint32_t contadorLista = 0;

				while(mensaje[3+contadorLista]!= NULL){
					list_add(posicionesTripulantes,mensaje[3+contadorLista]);
					contadorLista++;
				}

				contadorLista=0;
				while(mensaje[contadorLista]!= NULL){
					contadorLista++;
				}

				for(uint32_t j=0; j <(3 + cantidadDeTripulantes - contadorLista); j++){

					list_add(posicionesTripulantes,"0|0");

				}

				for(uint32_t i = 0; i< list_size(posicionesTripulantes);i++){
					char* posiciones = list_get(posicionesTripulantes, i);
					log_info(logger,posiciones);
				}


				for(uint32_t i=0; i< list_size(posicionesTripulantes) ; i++){			//TODO


					tripulante_data_msg*tripulanteConCoordenadas=malloc(sizeof(tripulante_data_msg));

					tripulanteConCoordenadas->coordenadas=  get_coordenadas(list_get(posicionesTripulantes, i));

					tripulanteConCoordenadas->idTripulante = id_tripulante_para_enviar;

					list_add(posicionesTripulantesParaRam,tripulanteConCoordenadas);

					id_tripulante_para_enviar++;

				}
				mensajePatota->tripulantes=posicionesTripulantesParaRam;

				enviar_paquete(mensajePatota,INICIAR_PATOTA_MSG,*socketDiscordiador);


				t_paquete*paquete =recibir_paquete(*socketDiscordiador);
				if(paquete->codigo == OK_MSG){
					log_info(logger,"mensaje a RAM exitoso, creando patotas con tripulantes");
				inicializarAtributosATripulante(posicionesTripulantes);
				}else if(paquete->codigo == FAIL_MSG){

					t_string* mensaje = deserializar_paquete(paquete);

					log_info(logger,"%s",mensaje->string);

					id_tripulante_para_enviar= id_tripulante_para_enviar - list_size(posicionesTripulantes);
					log_info(logger,"fallo la inicializacion de la patota");
				}

				fclose(fileTarea);
				void funcion(void*elemento){
									tripulante_data_msg*tripulanteConCoordenadas =(tripulante_data_msg*) elemento;
									free(tripulanteConCoordenadas->coordenadas);
									free(tripulanteConCoordenadas);
								}
				list_destroy_and_destroy_elements(mensajePatota->tripulantes,funcion);

				list_destroy(posicionesTripulantes);


				//free(mensajePatota->tareas);

				free(mensajePatota);





			}else{
				log_info(logger, "No existe la tarea");
			}

			break;

		}
		case LISTAR_TRIPULANTES: {

			time_t tiempo = time(0);
			struct tm *tlocal = localtime(&tiempo);
			char output[128];
			strftime(output,128,"%d/%m/%y %H:%M:%S\n",tlocal);
			printf("Estado de la Nave: %s\n",output);
			log_info(logger,"Estado de la Nave: %s",output);
			uint32_t i = 0;

			for(i=0; i<list_size(tripulantes);i++){
				t_tripulante* tripulante = list_get(tripulantes, i);
				printf("Tripulante: %s Patota: %s Status: %s\n",
						string_itoa(tripulante->idTripulante),
						string_itoa(tripulante->idPatota),
						convertirEnumAString(tripulante->estado));
				log_info(logger,"\nTripulante: %s Patota: %s Status: %s",
						string_itoa(tripulante->idTripulante),
						string_itoa(tripulante->idPatota),
						convertirEnumAString(tripulante->estado));
			}
			break;
		}

		case EXPULSAR_TRIPULANTE: {


			//ENVIO MENSAJE
			uint32_t id=atoi(mensaje[1]);

			expulsar_tripulante_msg* mensajeExpulsar=malloc(sizeof(expulsar_tripulante_msg));


			bool tieneMismoNombre(void*elemento){
				t_tripulante*tripulante= (t_tripulante*) elemento;
				return tripulante->idTripulante==id;
			}

			t_tripulante*tripulanteExpulsado = list_find(tripulantes,tieneMismoNombre);

			if(tripulanteExpulsado !=NULL){

			mensajeExpulsar->idTripulante= tripulanteExpulsado->idTripulante;
			mensajeExpulsar->idPatota= tripulanteExpulsado->idPatota;
			enviar_paquete(mensajeExpulsar,EXPULSAR_TRIPULANTE_MSG,*socketDiscordiador);

			//LO SACO DE TODAS LAS LISTAS Y LO SACO DEL WHILE DEL HILO
			tripulanteExpulsado->fueExpulsado=1;		//ESTO ES UN FLAG.

			if(list_any_satisfy(listaNuevos,tieneMismoNombre)){
							sacarTripulanteDeLista(tripulanteExpulsado,listaNuevos);
			}
			if(list_any_satisfy(listaReady,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaReady);
			}
			if(list_any_satisfy(listaBloqueados,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaBloqueados);
			}
			if(list_any_satisfy(listaBloqueadosPorSabotaje,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaBloqueadosPorSabotaje);
			}
			if(list_any_satisfy(listaEjecutando,tieneMismoNombre)){
				sacarTripulanteDeLista(tripulanteExpulsado,listaEjecutando);
			}

			tripulanteExpulsado->estado =FINISHED;
			pthread_mutex_lock(&mutex_listaFinalizados);
			list_add(listaFinalizados,tripulanteExpulsado);
			pthread_mutex_unlock(&mutex_listaFinalizados);

			log_info(logger,"fui expulsado mi id era:%d",tripulanteExpulsado->idTripulante);

			free(mensajeExpulsar);

			}

			break;
		}
		case INICIAR_PLANIFICACION: { //Con este comando se dará inicio a la planificación (es un semaforo sem init)

			if(list_is_empty(listaReady) && list_is_empty(listaEjecutando) &&list_is_empty(listaBloqueados) && list_is_empty(listaBloqueadosPorSabotaje) && list_is_empty(listaNuevos)){
				puts("asegurese de que el discordiador tenga tripulantes para planificar . Pruebe de nuevo");			//TODO
			}else {

				estaPlanificando=1;

				if(!list_is_empty(listaNuevos)){

					void iterador(t_tripulante*tripulante){
					sem_post(tripulante->semaforoCiclo);
					}
					list_iterate(listaNuevos,(void*) iterador);
				}

				if(!list_is_empty(listaEjecutando)){

					void iterador(t_tripulante*tripulante){
						sem_post(tripulante->semaforoCiclo);
					}
					list_iterate(listaEjecutando,(void*) iterador);

				}

				if(!list_is_empty(listaBloqueados)){

					void iterador(t_tripulante*tripulante){
						sem_post(tripulante->semaforoCiclo);
					}
					list_iterate(listaBloqueados,(void*) iterador);

				}

				sem_post(&sem_pausarPlanificacion);

			}

			break;
		}
		case PAUSAR_PLANIFICACION: { //Este comando lo que busca es detener la planificación en cualquier momento(semaforo)

			//INICIAR_PATOTA 1 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1

			estaPlanificando=0;


			break;
		}
		case OBTENER_BITACORA: { //Este comando obtendrá la bitácora del tripulante pasado por parámetro a través de una consulta a i-Mongo-Store.

			//enviarMensaje();

			uint32_t id=atoi(mensaje[1]);

			obtener_bitacora_msg*mensajeBitacora=malloc(sizeof(obtener_bitacora_msg));
			mensajeBitacora->idTripulante=id;

			log_info(logger,"solicito la bitacora el ID: %d:",id);
			enviar_paquete(mensajeBitacora,OBTENER_BITACORA_MSG,*socketBitacora);


			free(mensajeBitacora);

			//recibirMensaje()
			t_paquete*paqueteBitacora = recibir_paquete(*socketBitacora);

			if(paqueteBitacora->codigo != DESCONECTADO){


			obtener_bitacora_rta*mensajeBitacoraRta = deserializar_paquete(paqueteBitacora);




			log_info(logger,"el contenido de la bitacora del ID: %d es:\n%s",id,mensajeBitacoraRta->bitacora->string);


			}


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
	close(*socketDiscordiador);
	free(socketDiscordiador);
	close(*socketBitacora);
	free(socketBitacora);
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

uint32_t cantidadElementosArray(char** array)
{
	uint32_t i = 0;
	while(array[i])
	{
		i++;
	}
	return i;
}

void liberarArray(char** array)
{
	for(uint32_t i = 0; i < cantidadElementosArray(array); i++)
		free(array[i]);
	free(array);
}

void sacarTripulanteDeLista(t_tripulante* tripulante, t_list* lista){

	uint32_t a = list_size(lista);
	for(uint32_t i=0; i<a ; i++){
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
	mensaje->idPatota=tripulante->idPatota;
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado= (uint32_t) tripulante->estado;

	enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);


	log_info(logger,"cambio De Estado A Ready el tripulante con ID:%d\n",tripulante->idTripulante);
	free(mensaje);

}


void agregarTripulanteAListaExecYAvisar(t_tripulante* tripulante){

	tripulante->estado=EXEC;

	pthread_mutex_lock(&mutex_listaEjecutando);
	list_add(listaEjecutando,tripulante);
	pthread_mutex_unlock(&mutex_listaEjecutando);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idPatota=tripulante->idPatota;
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado= (uint32_t) tripulante->estado;

	enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);

	log_info(logger,"cambio De Estado A Exec el tripulante con ID:%d\n",tripulante->idTripulante);
	free(mensaje);

}


void agregarTripulanteAListaBloqueadosYAvisar(t_tripulante* tripulante){

	tripulante->estado=BLOCKED;

	pthread_mutex_lock(&mutex_listaBloqueados);
	list_add(listaBloqueados,tripulante);
	pthread_mutex_unlock(&mutex_listaBloqueados);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idPatota=tripulante->idPatota;
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado= (uint32_t) tripulante->estado;

	enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);
	log_info(logger,"cambio De Estado A Bloqueado el tripulante con ID:%d\n",tripulante->idTripulante);

	free(mensaje);

}
void agregarTripulanteAListaBloqueadosPorSabotajeYAvisar(t_tripulante* tripulante){

	tripulante->estado=BLOCKED;

	pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
	list_add(listaBloqueadosPorSabotaje,tripulante);
	pthread_mutex_unlock(&mutex_listaBloqueadosPorSabotaje);

	cambio_estado_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idPatota=tripulante->idPatota;
	mensaje->idTripulante=tripulante->idTripulante;
	mensaje->estado= (uint32_t) tripulante->estado;
	enviar_paquete(mensaje,CAMBIO_ESTADO,tripulante->socketTripulanteRam);
	log_info(logger,"cambio De Estado A BloqueadoPorSabotaje el tripulante con ID:%d\n",tripulante->idTripulante);
	free(mensaje);

}

void agregarTripulanteAListaFinishedYAvisar(t_tripulante* tripulante){

	tripulante->estado=FINISHED;

	pthread_mutex_lock(&mutex_listaFinalizados);
	list_add(listaFinalizados,tripulante);
	pthread_mutex_unlock(&mutex_listaFinalizados);

	expulsar_tripulante_msg*mensaje=malloc(sizeof(cambio_estado_msg));
	mensaje->idPatota=tripulante->idPatota;
	mensaje->idTripulante=tripulante->idTripulante;


	enviar_paquete(mensaje,EXPULSAR_TRIPULANTE,tripulante->socketTripulanteRam);
	log_info(logger,"cambio De Estado A Finished el tripulante con ID :%d\n",tripulante->idTripulante);

	free(mensaje);

}


uint32_t distanciaA(t_coordenadas* desde, t_coordenadas* hasta){

	if (desde == NULL || hasta == NULL) {
		return -1;
	}

	uint32_t distanciaX = abs(desde->posX - hasta->posX);
	uint32_t distanciaY = abs(desde->posY - hasta->posY);

	return distanciaX + distanciaY;

}

uint32_t llegoAlSabotaje(t_tripulante* tripulante,t_sabotaje*sabotaje){

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXsabotaje = sabotaje->coordenadas->posX;
	uint32_t posicionYsabotaje = sabotaje->coordenadas->posY;

	return (posicionXtripulante == posicionXsabotaje) && (posicionYtripulante == posicionYsabotaje);
}

void moverAlTripulanteHastaElSabotaje(t_tripulante*tripulante,t_sabotaje*sabotaje){


	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg* mensajeMovimientoSabotaje=malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoSabotaje->idPatota = tripulante->idPatota;
	mensajeMovimientoSabotaje->idTripulante = tripulante->idTripulante;

	mensajeMovimientoSabotaje->coordenadasDestino = malloc(sizeof(t_coordenadas));


		informar_movimiento_mongo_msg* mensajeMovimientoSabotajeMongo=malloc(sizeof(informar_movimiento_mongo_msg));
			mensajeMovimientoSabotajeMongo->coordenadasOrigen = malloc(sizeof(t_coordenadas));
				mensajeMovimientoSabotajeMongo->coordenadasDestino = malloc(sizeof(t_coordenadas));

		mensajeMovimientoSabotajeMongo->idTripulante = tripulante->idTripulante;
			mensajeMovimientoSabotajeMongo->coordenadasOrigen->posX = tripulante->coordenadas->posX;
			mensajeMovimientoSabotajeMongo->coordenadasOrigen->posY = tripulante->coordenadas->posY;

	int32_t posicionXtripulante = tripulante->coordenadas->posX;
	int32_t posicionYtripulante = tripulante->coordenadas->posY;

	int32_t posicionXsabotaje = sabotaje->coordenadas->posX;
	int32_t posicionYsabotaje = sabotaje->coordenadas->posY;

	if (posicionXtripulante != posicionXsabotaje) {

		int32_t diferenciaEnX = posicionXsabotaje - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYsabotaje) {

		int32_t diferenciaEnY = posicionYsabotaje - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}
	log_movimiento_tripulante(tripulante->idTripulante,tripulante->coordenadas->posX,tripulante->coordenadas->posY);

	mensajeMovimientoSabotaje->coordenadasDestino->posX = tripulante->coordenadas->posX;
	mensajeMovimientoSabotaje->coordenadasDestino->posY = tripulante->coordenadas->posY;

	mensajeMovimientoSabotajeMongo->coordenadasDestino->posX =tripulante->coordenadas->posX ;
	mensajeMovimientoSabotajeMongo->coordenadasDestino->posY =tripulante->coordenadas->posY ;

	enviar_paquete(mensajeMovimientoSabotaje,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);

	enviar_paquete(mensajeMovimientoSabotajeMongo,INFORMAR_MOVIMIENTO_MONGO,tripulante->socketTripulanteImongo);

	free(mensajeMovimientoSabotaje->coordenadasDestino);
	free(mensajeMovimientoSabotaje);


}

int32_t llegoATarea(t_tripulante* tripulante){

	int32_t posicionXtripulante = tripulante->coordenadas->posX;
	int32_t posicionYtripulante = tripulante->coordenadas->posY;

	int32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	int32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	return (posicionXtripulante == posicionXtarea) && (posicionYtripulante == posicionYtarea);
}

void moverAlTripulanteHastaLaTarea(t_tripulante*tripulante){
	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg*mensajeMovimientoTarea = malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoTarea->idPatota = tripulante->idPatota;
	mensajeMovimientoTarea->idTripulante = tripulante->idTripulante;

	mensajeMovimientoTarea->coordenadasDestino = malloc(sizeof(t_coordenadas));

		informar_movimiento_mongo_msg* mensajeMovimientoMongo = malloc(sizeof(informar_movimiento_mongo_msg));
			mensajeMovimientoMongo->coordenadasOrigen = malloc(sizeof(t_coordenadas));
			mensajeMovimientoMongo->coordenadasDestino = malloc(sizeof(t_coordenadas));					//TODO

			mensajeMovimientoMongo->idTripulante = tripulante->idTripulante;
			mensajeMovimientoMongo->coordenadasOrigen->posX = tripulante->coordenadas->posX;
			mensajeMovimientoMongo->coordenadasOrigen->posY = tripulante->coordenadas->posY;


	int32_t posicionXtripulante = tripulante->coordenadas->posX;
	int32_t posicionYtripulante = tripulante->coordenadas->posY;

	int32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	int32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	if (posicionXtripulante != posicionXtarea) {

		int32_t diferenciaEnX = posicionXtarea - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYtarea) {

		int32_t diferenciaEnY = posicionYtarea - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}
	log_movimiento_tripulante(tripulante->idTripulante,tripulante->coordenadas->posX,tripulante->coordenadas->posY);



	mensajeMovimientoTarea->coordenadasDestino->posX = tripulante->coordenadas->posX;
	mensajeMovimientoTarea->coordenadasDestino->posY = tripulante->coordenadas->posY;

	mensajeMovimientoMongo->coordenadasDestino->posX = tripulante->coordenadas->posX;
	mensajeMovimientoMongo->coordenadasDestino->posY = tripulante->coordenadas->posY;




	enviar_paquete(mensajeMovimientoTarea,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);

	enviar_paquete(mensajeMovimientoMongo,INFORMAR_MOVIMIENTO_MONGO,tripulante->socketTripulanteImongo);

	free(mensajeMovimientoTarea->coordenadasDestino);
	free(mensajeMovimientoTarea);

		free(mensajeMovimientoMongo->coordenadasDestino);
		free(mensajeMovimientoMongo->coordenadasOrigen);
		free(mensajeMovimientoMongo);


}


t_tripulante* tripulanteMasCercanoDelSabotaje(t_sabotaje* sabotaje){
	t_tripulante* tripulanteMasCercanoSabotaje;
	t_tripulante* tripulanteTemporal;


	uint32_t distanciaTemporal;
	uint32_t menorDistanciaSabotaje = 1000;


	pthread_mutex_lock(&mutex_listaNuevos);
	t_list* tripulantesBloqueadosSabotaje = list_duplicate(listaBloqueadosPorSabotaje);
	pthread_mutex_unlock(&mutex_listaNuevos);

	if(!list_is_empty(tripulantesBloqueadosSabotaje)){
		tripulanteMasCercanoSabotaje = list_get(tripulantesBloqueadosSabotaje, 0);
		menorDistanciaSabotaje = distanciaA(tripulanteMasCercanoSabotaje->coordenadas, sabotaje->coordenadas);



		for(uint32_t i = 1; i < tripulantesBloqueadosSabotaje->elements_count; i++){

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

	pthread_mutex_lock(&mutex_listaBloqueadosPorSabotaje);
	sacarTripulanteDeLista(tripulanteMasCercanoSabotaje, listaBloqueadosPorSabotaje);
	pthread_mutex_unlock(&mutex_listaBloqueadosPorSabotaje);


	list_destroy(tripulantesBloqueadosSabotaje);

	return tripulanteMasCercanoSabotaje;
}


algoritmo_code stringACodigoAlgoritmo(const char* string) {
	for (uint32_t i = 0;
			i < sizeof(conversionAlgoritmo) / sizeof(conversionAlgoritmo[0]);
			i++) {
		if (!strcmp(string, conversionAlgoritmo[i].str))
			return conversionAlgoritmo[i].codigo_algoritmo;
	}
	return ERROR_CODIGO_ALGORITMO;
}




void ejecutarTripulante(t_tripulante* tripulante){
	//INICIAR_PATOTA 2 /home/utnso/tp-2021-1c-DuroDeAprobar/Tareas/tareasPatota1.txt 1|1

	log_info(logger,"hola soy el tripulante con ID:%d\n",tripulante->idTripulante);

	uint32_t socketDelTripulanteConImongo = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);		//TODO
	tripulante->socketTripulanteImongo = socketDelTripulanteConImongo;

	uint32_t socketDelTripulanteConRam = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
	tripulante->socketTripulanteRam = socketDelTripulanteConRam;


	//mandarTarea()
			solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
			mensajeTarea->idPatota=tripulante->idPatota;
			mensajeTarea->idTripulante=tripulante->idTripulante;
			enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
			log_info(logger,"se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

			free(mensajeTarea);
		//recibirMensaje()

				t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

				switch(paqueteTareaRta->codigo){

				case SOLICITAR_SIGUIENTE_TAREA_RTA:{

				solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

				log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
				log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
				log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


				char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

				if(nombreTarea[1] ==NULL){

					log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

					tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
					strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
					tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
					tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
					tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
					tripulante->tareaAsignada->parametros=0;
					tripulante->tareaAsignada->finalizoTarea=false;

				}	else   {

					log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

					tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);
					strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
					tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
					tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
					tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
					tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
					tripulante->tareaAsignada->finalizoTarea=false;


				}

				int i = 0;
				while(nombreTarea[i] != NULL){
					free(nombreTarea[i]);
					i++;
				}
				free(nombreTarea);

				log_info(logger,"le asigno la primer tarea al tripulante %d\n",tripulante->idTripulante);

				break;

					} case COMPLETO_TAREAS:{
						log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
						agregarTripulanteAListaFinishedYAvisar(tripulante);
						sem_post(&sem_planificarMultitarea);

						break;
				 }
				}

	sem_post(&sem_hiloTripulante);

	if(estaPlanificando==0){
		sem_wait(tripulante->semaforoCiclo);
	}

	pthread_mutex_lock(&mutex_listaNuevos);
	sacarTripulanteDeLista(tripulante, listaNuevos);
	pthread_mutex_unlock(&mutex_listaNuevos);

	tripulante->estado=READY;
	pthread_mutex_lock(&mutex_listaReady);
	list_add(listaReady,tripulante);
	pthread_mutex_unlock(&mutex_listaReady);
	log_info(logger,"cambio De Estado A Ready el tripulante con ID:%d\n",tripulante->idTripulante);






	while(tripulante->estado != FINISHED && tripulante->fueExpulsado != 1){		//TODO



		log_info(logger,"el tripulante se queda esperando a ser asignado %d",tripulante->idTripulante);

		sem_wait(tripulante->semaforoDelTripulante);

		log_info(logger,"fue seleccionado para ejecutar el tripulante %d",tripulante->idTripulante);

		if(stringACodigoAlgoritmo(ALGORITMO)==FIFO){


			if(!llegoATarea(tripulante) && tripulante->fueExpulsado != 1 && haySabotaje !=1){
				uint32_t distancia;
				distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);
				log_info(logger,"se esta moviendo a la tarea la tarea el tripulante %d",tripulante->idTripulante);

				while (distancia != 0 && distancia != -1 && tripulante->fueExpulsado != 1 && haySabotaje != 1) {

					if(estaPlanificando==0){
						sem_wait(tripulante->semaforoCiclo);
					}

					moverAlTripulanteHastaLaTarea(tripulante);


					distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);


				}

			}




			if(llegoATarea(tripulante) && haySabotaje !=1 && tripulante->estado != FINISHED){
				log_info(logger,"va a ejecutar la tarea el tripulante con ID %d",tripulante->idTripulante);


				ejecucionDeTareaTripulanteFIFO(tripulante);

				log_info(logger,"finalizo la ejecucion de la tarea el tripulante con ID: %d",tripulante->idTripulante);

			}

			if(tripulante->tareaAsignada->finalizoTarea==true && tripulante->fueExpulsado !=1 && haySabotaje!=1 && tripulante->estado != FINISHED ){

				//mandarTarea()
						solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
						mensajeTarea->idPatota=tripulante->idPatota;
						mensajeTarea->idTripulante=tripulante->idTripulante;
						enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
						log_info("se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);


					//recibirMensaje()				TODO

							t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

							switch(paqueteTareaRta->codigo){

							case SOLICITAR_SIGUIENTE_TAREA_RTA:{

							solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

							log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
							log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
							log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


							char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

							if(nombreTarea[1] ==NULL){

								log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

								tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
								strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
								tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
								tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
								tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
								tripulante->tareaAsignada->parametros=0;
								tripulante->tareaAsignada->finalizoTarea=false;

							}	else   {

								log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

								tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);	//ACA ROMPE
								strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
								tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
								tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
								tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
								tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
								tripulante->tareaAsignada->finalizoTarea=false;

							}

							int i = 0;
							while(nombreTarea[i] != NULL){
								free(nombreTarea[i]);
								i++;
							}
							free(nombreTarea);


							break;

								} case COMPLETO_TAREAS:{
									log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
									agregarTripulanteAListaFinishedYAvisar(tripulante);
									sem_post(&sem_planificarMultitarea);

									break;
							 }
							}
				}

				if(haySabotaje==1){
					log_info(logger, "Llego un sabotaje, toca hacer huelga soy el tripulante con ID: %d",tripulante->idTripulante);
					sem_post(&sem_planificarMultitarea);
				}

				if(tripulante->fueExpulsado == 1){
						log_info(logger,"Fui expulsado yo era el tripulante con ID: %d", tripulante->idTripulante);
						sem_post(&sem_planificarMultitarea);
				}






		}
		else if(stringACodigoAlgoritmo(ALGORITMO)==RR){

			ejecucionRR(tripulante);
			if(tripulante->fueExpulsado == 1){
					log_info(logger,"Fui expulsado yo era el tripulante con ID: %d", tripulante->idTripulante);
					sem_post(&sem_planificarMultitarea);
			}

		}



	}



	sem_destroy(tripulante->semaforoBloqueadoTripulante);
	sem_destroy(tripulante->semaforoDelSabotaje);
	sem_destroy(tripulante->semaforoDelTripulante);
	sem_destroy(tripulante->semaforoCiclo);
	close(tripulante->socketTripulanteImongo);
	close(tripulante->socketTripulanteRam);
	free(tripulante->coordenadas);
	free(tripulante->tareaAsignada->coordenadas);
	free(tripulante->tareaAsignada);


}




void ejecucionDeTareaTripulanteFIFO(t_tripulante*tripulante){

	if(string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)==TAREA_CPU){
		log_info(logger, "el tripulante %d esta realizando una tarea de CPU ",tripulante->idTripulante);
		inicio_tarea_msg* mandarTareaCpu=malloc(sizeof(inicio_tarea_msg));
		mandarTareaCpu->idTripulante = tripulante->idTripulante;
		mandarTareaCpu->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);
		mandarTareaCpu->parametros =0;

		enviar_paquete(mandarTareaCpu, INICIO_TAREA,tripulante->socketTripulanteImongo);
		free(mandarTareaCpu);

		while(tripulante->misCiclosDeCPU<tripulante->tareaAsignada->duracion && tripulante->fueExpulsado != 1 && haySabotaje != 1){

			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}
			sleep(RETARDO_CICLO_CPU);
			tripulante->misCiclosDeCPU++;

			log_info(logger,"El tripulante con ID %d hizo %d de CPU de un total de %d",tripulante->idTripulante,tripulante->misCiclosDeCPU,tripulante->tareaAsignada->duracion);



		}


		if(tripulante->misCiclosDeCPU==tripulante->tareaAsignada->duracion && tripulante->fueExpulsado != 1 && haySabotaje != 1){

			fin_tarea_msg* mandarTareaCPU=malloc(sizeof(fin_tarea_msg));
					mandarTareaCPU->idTripulante = tripulante->idTripulante;
					mandarTareaCPU->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);		//mando el fin de la tarea
					enviar_paquete(mandarTareaCPU, FIN_TAREA,tripulante->socketTripulanteImongo);
					free(mandarTareaCPU);

			tripulante->tareaAsignada->finalizoTarea=true;

			free(tripulante->tareaAsignada->nombreTarea);


			tripulante->misCiclosDeCPU=0;
			sem_post(tripulante->semaforoDelTripulante);

		}



	}
	else if( (string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)!=TAREA_CPU) ){		//aca no agrego si hay sabotaje dejo que entre a bloqueados y salga

		if(tripulante->fueExpulsado !=1	&& haySabotaje!=1){																				//&& tripulante->fueExpulsado != 1 && haySabotaje != 1
			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}

			log_info(logger,"se realiza un ciclo de CPU para enviar tarea del tripulante con ID: %d",tripulante->idTripulante);
			sleep(RETARDO_CICLO_CPU);


										inicio_tarea_msg* mandarTareaIO=malloc(sizeof(inicio_tarea_msg));
										mandarTareaIO->idTripulante = tripulante->idTripulante;
										mandarTareaIO->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);
										mandarTareaIO->parametros = tripulante->tareaAsignada->parametros;
										enviar_paquete(mandarTareaIO, INICIO_TAREA,tripulante->socketTripulanteImongo);

										free(mandarTareaIO);
			log_info(logger,"el mensaje a imongo ha sido enviado exitosamente del tripulante con ID: %d",tripulante->idTripulante);


			if(tripulante->fueExpulsado !=1){

				list_remove(listaEjecutando, getIndexTripulanteEnLista(listaEjecutando,tripulante));
				agregarTripulanteAListaBloqueadosYAvisar(tripulante);
				log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "ES UNA TAREA DE IO", "BLOCKED"); //PONER EN LISTA DE BLOQUEADOS


				sem_post(&sem_planificarMultitarea);		//ES UNA IO PUEDO EJECUTAR OTRO TRIPULANTE

				sem_post(&semaforoInicioCicloBloqueado);		//tiraSemaforoPostALHiloBLoqueado

				sem_wait(tripulante->semaforoBloqueadoTripulante);		//el hilo me da el post para que pueda seguir, es por tripulante para que sea en orden.



				log_info(logger,"llego al final de la tarea el tripulante con ID %d\n",tripulante->idTripulante);

				fin_tarea_msg*mandarFinTareaIO =malloc(sizeof(fin_tarea_msg));
				mandarFinTareaIO->idTripulante = tripulante->idTripulante;
				mandarFinTareaIO->nombreTarea = get_t_string(tripulante->tareaAsignada->nombreTarea);
				enviar_paquete(mandarFinTareaIO,FIN_TAREA,tripulante->socketTripulanteImongo);

				free(mandarFinTareaIO);

				//tripulante->tareaAsignada->finalizoTarea=true;		No es necesario ya que si es expulsado no entra en el otro y sino directamente pide otra tarea
				free(tripulante->tareaAsignada->nombreTarea);

				if(tripulante->fueExpulsado !=1){

					//mandarTarea()
							solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
							mensajeTarea->idPatota=tripulante->idPatota;
							mensajeTarea->idTripulante=tripulante->idTripulante;
							enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
							log_info(logger,"se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

							free(mensajeTarea);
						//recibirMensaje()				TODO

								t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

								switch(paqueteTareaRta->codigo){

								case SOLICITAR_SIGUIENTE_TAREA_RTA:{

								solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

								log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
								log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
								log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


								char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

								if(nombreTarea[1] ==NULL){

									log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

									tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
									strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
									tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
									tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
									tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
									tripulante->tareaAsignada->parametros=0;
									tripulante->tareaAsignada->finalizoTarea=false;

								}	else   {

									log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

									tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);
									strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
									tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
									tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
									tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
									tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
									tripulante->tareaAsignada->finalizoTarea=false;


								}

								agregarTripulanteAListaReadyYAvisar(tripulante);


								break;

									} case COMPLETO_TAREAS:{
										log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
										agregarTripulanteAListaFinishedYAvisar(tripulante);

										break;
								 }
								}
				}
			}
		}
	}
}



void planificarBloqueo(){
	while(true){

		sem_wait(&semaforoInicioCicloBloqueado);



		pthread_mutex_lock(&mutex_listaBloqueados);
		t_tripulante*tripulante=list_get(listaBloqueados,0);
		pthread_mutex_unlock(&mutex_listaBloqueados);

		log_info(logger,"asigno al tripulante con ID: %d a planificar bloqueo\n",tripulante->idTripulante);

		uint32_t r=1;

		while(tripulante->tareaAsignada->duracion>=r && tripulante->fueExpulsado != 1){		// DURACION DE LA TAREA BLOQUEADA	si fue expulsado que salga y si hay sabotaje que entre en el wait

			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}
						if(haySabotaje==1){
							sem_wait(tripulante->semaforoDelSabotaje);
						}

			sleep(RETARDO_CICLO_CPU);

			log_info(logger,"El tripulante con ID %d hizo %d de IO de un total de %d",tripulante->idTripulante,r,tripulante->tareaAsignada->duracion);

			r++;
		}
		pthread_mutex_lock(&mutex_listaBloqueados);
		list_remove(listaBloqueados, getIndexTripulanteEnLista(listaBloqueados,tripulante));		//	aca lo saco de la lista de bloqueo
		pthread_mutex_unlock(&mutex_listaBloqueados);


		sem_post(tripulante->semaforoBloqueadoTripulante);




	}
}


void inicioHiloPlanificarBloqueo(){
	pthread_t hiloPlaificadorBloqueo;
	pthread_create(&hiloPlaificadorBloqueo, NULL, (void*) planificarBloqueo,NULL);
	pthread_detach(hiloPlaificadorBloqueo);
}


void planificarSegun(){			//TODO
	while(true){

		sem_wait(&sem_pausarPlanificacion);

		while(estaPlanificando==1){	  //FALTARIA SABOTAJE FLAG

			if(haySabotaje==1){
				sem_wait(&sem_sabotaje);
			}

			sem_wait(&sem_planificarMultitarea);

			if(!list_is_empty(listaReady)){

			pthread_mutex_lock(&mutex_listaReady);
			t_tripulante* tripulante = (t_tripulante*) list_remove(listaReady, 0);
			pthread_mutex_unlock(&mutex_listaReady);


				log_info(logger,"PLANIFICADOR:voy a asignar al tripulante %d\n",tripulante->idTripulante);

				agregarTripulanteAListaExecYAvisar(tripulante);
				log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "fue seleccionado para ejecutar", "EXEC");


				sem_post(tripulante->semaforoDelTripulante);


			}else{

				sleep(RETARDO_CICLO_CPU);
				sem_post(&sem_planificarMultitarea);
			}

		}
	}
}

void ejecucionRR(t_tripulante*tripulante){

	if(!llegoATarea(tripulante) && tripulante->fueExpulsado!=1 && haySabotaje!=1){
		uint32_t distancia;
		distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);
		log_info(logger,"se esta moviendo a la tarea el tripulante %d",tripulante->idTripulante);

		while (distancia != 0 && distancia != -1 && (tripulante->quantumDisponible)>0 && tripulante->fueExpulsado!=1) {

			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}

			moverAlTripulanteHastaLaTarea(tripulante);
			tripulante->quantumDisponible--;


			distancia = distanciaA(tripulante->coordenadas, tripulante->tareaAsignada != NULL ? tripulante->tareaAsignada->coordenadas : 0);

		}
	}


	if(llegoATarea(tripulante) && (tripulante->quantumDisponible > 0 && tripulante->fueExpulsado!=1 && haySabotaje!=1) ){


		log_info(logger,"va a ejecutar la tarea el tripulante %d",tripulante->idTripulante);

		ejecucionDeTareaTripulanteRR(tripulante);

	}


	if((tripulante->quantumDisponible)==0 && tripulante->fueExpulsado!=1 && haySabotaje!=1){
		tripulante->quantumDisponible = QUANTUM;
		log_info(logger,"no termino la tarea y se quedo sin quantum el tripulante con ID: %d\n",tripulante->idTripulante);
		pthread_mutex_lock(&mutex_listaEjecutando);
		list_remove(listaEjecutando, getIndexTripulanteEnLista(listaEjecutando,tripulante));
		pthread_mutex_unlock(&mutex_listaEjecutando);

		agregarTripulanteAListaReadyYAvisar(tripulante);

		sem_post(&sem_planificarMultitarea);

	}

	if(haySabotaje==1){
		log_info(logger, "Llego un sabotaje, toca hacer huelga soy el tripulante con ID: %d",tripulante->idTripulante);
		sem_post(&sem_planificarMultitarea);
	}


}


void ejecucionDeTareaTripulanteRR(t_tripulante*tripulante){



	if(string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)==TAREA_CPU){
		log_info(logger, "el tripulante %d esta realizando una tarea de CPU ",tripulante->idTripulante);

		inicio_tarea_msg* mandarTareaCpu=malloc(sizeof(inicio_tarea_msg));
				mandarTareaCpu->idTripulante = tripulante->idTripulante;
				mandarTareaCpu->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);
				mandarTareaCpu->parametros =0;
				enviar_paquete(mandarTareaCpu, INICIO_TAREA,tripulante->socketTripulanteImongo);
				free(mandarTareaCpu);


		while(tripulante->misCiclosDeCPU<tripulante->tareaAsignada->duracion && (tripulante->quantumDisponible)>0 && tripulante->fueExpulsado!=1 && haySabotaje !=1){

			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}
			sleep(RETARDO_CICLO_CPU);

			tripulante->misCiclosDeCPU++;
			tripulante->quantumDisponible--;
			log_info(logger,"El tripulante con ID %d hizo %d de CPU de un total de %d",tripulante->idTripulante,tripulante->misCiclosDeCPU,tripulante->tareaAsignada->duracion);


		}


		if(tripulante->misCiclosDeCPU==tripulante->tareaAsignada->duracion && tripulante->quantumDisponible > 0 && tripulante->fueExpulsado!=1){	//SI LLEGO Y EL QUANTUM NO ES 0 SIGUE EJECUTANDO OTRA TAREA
			tripulante->tareaAsignada->finalizoTarea=true;

			//le mando el FIN tarea a MONGO  	TODO
			fin_tarea_msg* mandarTareaCPU=malloc(sizeof(fin_tarea_msg));
			mandarTareaCPU->idTripulante = tripulante->idTripulante;
			mandarTareaCPU->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);		//mando el fin de la tarea
			enviar_paquete(mandarTareaCPU, FIN_TAREA,tripulante->socketTripulanteImongo);
			free(mandarTareaCPU);

			free(tripulante->tareaAsignada->nombreTarea);
			tripulante->misCiclosDeCPU=0;

			log_info(logger,"se termino la tarea y de quantum le queda %d del tripulante con ID: %d\n",tripulante->quantumDisponible,tripulante->idTripulante);


			//mandarTarea()
					solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
					mensajeTarea->idPatota=tripulante->idPatota;
					mensajeTarea->idTripulante=tripulante->idTripulante;
					enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
					log_info(logger,"se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

					free(mensajeTarea);

				//recibirMensaje()				TODO

						t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

						switch(paqueteTareaRta->codigo){

						case SOLICITAR_SIGUIENTE_TAREA_RTA:{

						solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

						log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
						log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
						log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


						char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

						if(nombreTarea[1] ==NULL){

							log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

							tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
							strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
							tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
							tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
							tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
							tripulante->tareaAsignada->parametros=0;
							tripulante->tareaAsignada->finalizoTarea=false;

						}	else   {

							log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

							tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);
							strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
							tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
							tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
							tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
							tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
							tripulante->tareaAsignada->finalizoTarea=false;

						}

						int i = 0;
						while(nombreTarea[i] != NULL){
							free(nombreTarea[i]);
							i++;
						}
						free(nombreTarea);

						log_info(logger,"se le asigno otra tarea al tripulante%d",tripulante->idTripulante);

						sem_post(tripulante->semaforoDelTripulante);		//es el post al semaforo del tripulante para que no salga

						break;

							} case COMPLETO_TAREAS:{
								log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
								agregarTripulanteAListaFinishedYAvisar(tripulante);
								sem_post(&sem_planificarMultitarea);

								break;
						 }
						}




		}

		if(tripulante->misCiclosDeCPU == tripulante->tareaAsignada->duracion && tripulante->quantumDisponible == 0 && tripulante->fueExpulsado!=1 && haySabotaje!=1){	//SI EL QUANTUM ES 0 OSEA HIZO SU ULTIMA RAFAGA Y TAMBIEN TERMINO LA TAREA

			tripulante->tareaAsignada->finalizoTarea=true;

						//le mando el FIN tarea a MONGO
			fin_tarea_msg* mandarTareaCPU=malloc(sizeof(fin_tarea_msg));
			mandarTareaCPU->idTripulante = tripulante->idTripulante;
			mandarTareaCPU->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);		//mando el fin de la tarea
			enviar_paquete(mandarTareaCPU, FIN_TAREA,tripulante->socketTripulanteImongo);
			free(mandarTareaCPU);

			free(tripulante->tareaAsignada->nombreTarea);

			tripulante->misCiclosDeCPU=0;		//LE SETEO EL VALOR AL CICLO
			tripulante->quantumDisponible=QUANTUM; //le vuelvo a setear el quantum

			log_info(logger,"termino la tarea y se quedo sin quantum el tripulante con ID: %d\n",tripulante->idTripulante);

			pthread_mutex_lock(&mutex_listaEjecutando);
			list_remove(listaEjecutando, getIndexTripulanteEnLista(listaEjecutando,tripulante)); //lo saco de exec y lo mando a ready
			pthread_mutex_unlock(&mutex_listaEjecutando);


				//mandarTarea()
						solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
						mensajeTarea->idPatota=tripulante->idPatota;
						mensajeTarea->idTripulante=tripulante->idTripulante;
						enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
						log_info(logger,"se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

						free(mensajeTarea);

					//recibirMensaje()				TODO

							t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

							switch(paqueteTareaRta->codigo){

							case SOLICITAR_SIGUIENTE_TAREA_RTA:{

							solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

							log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
							log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
							log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


							char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

							if(nombreTarea[1] ==NULL){

								log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

								tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
								strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
								tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
								tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
								tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
								tripulante->tareaAsignada->parametros=0;
								tripulante->tareaAsignada->finalizoTarea=false;

							}	else   {

								log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

								tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);
								strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
								tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
								tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
								tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
								tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
								tripulante->tareaAsignada->finalizoTarea=false;

							}

							agregarTripulanteAListaReadyYAvisar(tripulante);

							int i = 0;
							while(nombreTarea[i] != NULL){
								free(nombreTarea[i]);
								i++;
							}
							free(nombreTarea);

							break;

								} case COMPLETO_TAREAS:{
									log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
									agregarTripulanteAListaFinishedYAvisar(tripulante);


									break;
							 }
							}

				sem_post(&sem_planificarMultitarea);



		}

		if((tripulante->quantumDisponible)==0 && tripulante->misCiclosDeCPU != tripulante->tareaAsignada->duracion && tripulante->fueExpulsado!=1 && haySabotaje!=1){ //en este caso no termino la tarea

			tripulante->quantumDisponible = QUANTUM;	//solo seteo el quantum sin setear los ciclos
			log_info(logger,"no termino la tarea y se quedo sin quantum el tripulante con ID: %d\n",tripulante->idTripulante);
			pthread_mutex_lock(&mutex_listaEjecutando);
			list_remove(listaEjecutando, getIndexTripulanteEnLista(listaEjecutando,tripulante));
			pthread_mutex_unlock(&mutex_listaEjecutando);

			agregarTripulanteAListaReadyYAvisar(tripulante);
			sem_post(&sem_planificarMultitarea);

		}



	}
	else if(string_to_op_code_tareas(tripulante->tareaAsignada->nombreTarea)!=TAREA_CPU){

		if((tripulante->quantumDisponible)>0 && tripulante->fueExpulsado!=1 && haySabotaje!=1){		//SI EL QUANTUM ES 0 NO PUEDE MANDAR EL MENSAJE.

			if(estaPlanificando==0){
				sem_wait(tripulante->semaforoCiclo);
			}

			log_info(logger,"se realiza un ciclo de CPU para enviar tarea del tripulante %d",tripulante->idTripulante);
			sleep(RETARDO_CICLO_CPU);


			inicio_tarea_msg* mandarTareaIO=malloc(sizeof(inicio_tarea_msg));
			mandarTareaIO->idTripulante = tripulante->idTripulante;
			mandarTareaIO->nombreTarea =get_t_string(tripulante->tareaAsignada->nombreTarea);
			mandarTareaIO->parametros = tripulante->tareaAsignada->parametros;

			enviar_paquete(mandarTareaIO, INICIO_TAREA,tripulante->socketTripulanteImongo);

			free(mandarTareaIO);

			log_info(logger,"el mensaje a imongo ha sido enviado exitosament edel tripulante %d",tripulante->idTripulante);

			tripulante->quantumDisponible--;

			if(tripulante->fueExpulsado != 1){
			list_remove(listaEjecutando, getIndexTripulanteEnLista(listaEjecutando,tripulante));
			agregarTripulanteAListaBloqueadosYAvisar(tripulante);
			log_tripulante_cambio_de_cola_planificacion(tripulante->idTripulante, "ES UNA TAREA DE IO", "BLOCKED"); //PONER EN LISTA DE BLOQUEADOS


			sem_post(&sem_planificarMultitarea);		//ES UNA IO PUEDO EJECUTAR OTRO TRIPULANTE

			sem_post(&semaforoInicioCicloBloqueado);		//tiraSemaforoPostALHiloBLoqueado

			sem_wait(tripulante->semaforoBloqueadoTripulante);		//el hilo me da el post para que pueda seguir, es por tripulante para que sea en orden.



			fin_tarea_msg*mandarFinTareaIO =malloc(sizeof(fin_tarea_msg));
			mandarFinTareaIO->idTripulante = tripulante->idTripulante;
			mandarFinTareaIO->nombreTarea = get_t_string(tripulante->tareaAsignada->nombreTarea);
			enviar_paquete(mandarFinTareaIO,FIN_TAREA,tripulante->socketTripulanteImongo);

			free(mandarFinTareaIO);

			tripulante->tareaAsignada->finalizoTarea=true;
			free(tripulante->tareaAsignada->nombreTarea);

			tripulante->quantumDisponible = QUANTUM;

			if(tripulante->fueExpulsado !=1){
			//mandarTarea()
					solicitar_siguiente_tarea_msg* mensajeTarea=malloc(sizeof(solicitar_siguiente_tarea_msg));
					mensajeTarea->idPatota=tripulante->idPatota;
					mensajeTarea->idTripulante=tripulante->idTripulante;
					enviar_paquete(mensajeTarea,SOLICITAR_SIGUIENTE_TAREA,tripulante->socketTripulanteRam);
					log_info(logger,"se solicito una tarea del tripulante:%d\n",tripulante->idTripulante);

					free(mensajeTarea);

				//recibirMensaje()				TODO

						t_paquete*paqueteTareaRta = recibir_paquete(tripulante->socketTripulanteRam);

						switch(paqueteTareaRta->codigo){

						case SOLICITAR_SIGUIENTE_TAREA_RTA:{

						solicitar_siguiente_tarea_rta*mensajeTareaRta=deserializar_paquete(paqueteTareaRta);

						log_info(logger,mensajeTareaRta->tarea->nombre_parametros);
						log_info(logger, "Duracion: %d", mensajeTareaRta->tarea->duracion);
						log_info(logger, "Pos: %d|%d\n", mensajeTareaRta->tarea->coordenadas->posX, mensajeTareaRta->tarea->coordenadas->posY);


						char** nombreTarea = string_split(mensajeTareaRta->tarea->nombre_parametros, " ");		//es un char*

						if(nombreTarea[1] ==NULL){

							log_info(logger,"solicito una tarea de CPU el tripulante con ID:%d",tripulante->idTripulante);

							tripulante->tareaAsignada->nombreTarea=malloc(strlen(nombreTarea[0] ) + 1);
							strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
							tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
							tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
							tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
							tripulante->tareaAsignada->parametros=0;
							tripulante->tareaAsignada->finalizoTarea=false;
						}	else   {

							log_info(logger,"solicito una tarea de IO el tripulante con ID:%d",tripulante->idTripulante);

							tripulante->tareaAsignada->nombreTarea = malloc(strlen(nombreTarea[0] ) + 1);
							strcpy(tripulante->tareaAsignada->nombreTarea,nombreTarea[0]);
							tripulante->tareaAsignada->coordenadas->posX=mensajeTareaRta->tarea->coordenadas->posX;
							tripulante->tareaAsignada->coordenadas->posY=mensajeTareaRta->tarea->coordenadas->posY;
							tripulante->tareaAsignada->duracion=mensajeTareaRta->tarea->duracion;
							tripulante->tareaAsignada->parametros=atoi(nombreTarea[1]);
							tripulante->tareaAsignada->finalizoTarea=false;

						}

						int i = 0;
						while(nombreTarea[i] != NULL){
							free(nombreTarea[i]);
							i++;
						}
						free(nombreTarea);

						agregarTripulanteAListaReadyYAvisar(tripulante);

						break;

							} case COMPLETO_TAREAS:{
								log_info(logger,"completo todas las tareas el tripulante con ID:%d",tripulante->idTripulante);
								agregarTripulanteAListaFinishedYAvisar(tripulante);

								break;
						 }
						}

			}
			}
		}

		if((tripulante->quantumDisponible)==0){
			tripulante->quantumDisponible = QUANTUM;
		}
	}

}



void finalizar(){
	void destruirTripulantes(void* elemento){
			t_tripulante* tripulante = (t_tripulante*) elemento;

			free(tripulante->semaforoCiclo);
			free(tripulante->semaforoBloqueadoTripulante);
			free(tripulante->semaforoDelTripulante);
			free(tripulante->semaforoDelSabotaje);
			free(tripulante);

	}
	list_destroy_and_destroy_elements(tripulantes, destruirTripulantes);

	list_destroy(listaNuevos);
	list_destroy(listaReady);
	list_destroy(listaBloqueados);
	list_destroy(listaBloqueadosPorSabotaje);
	list_destroy(listaEjecutando);
	list_destroy(listaFinalizados);

//	listaNuevos = list_create();
//	listaReady = list_create();
//	listaBloqueados= list_create();
//	listaBloqueadosPorSabotaje= list_create();
//	listaEjecutando = list_create();
//	listaFinalizados = list_create();
}




