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
pthread_mutex_t mutex_listaEjecutando = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaFinalizados = PTHREAD_MUTEX_INITIALIZER;

uint32_t id_tripulante = 1;
uint32_t id_patota = 1;


int main(void){
	inicializarConfig(config);

	iniciarLog();
	inicializarListasGlobales();
	//uint32_t socket = crear_conexion(IP_I_MONGO_STORE,PUERTO_I_MONGO_STORE);


	puts("hola viajero, soy el discordiador, en que puedo ayudarte?");
	leer_consola();

	puts("LLEGO AL FIN DEL PROGRAMA");

	return EXIT_SUCCESS;




}
void inicializarListasGlobales(){

	estaPlanificando=1;
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
		op_code codigo_mensaje = string_to_op_code(mensaje[0]);

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
			uint32_t socket = crear_conexion(IP_MI_RAM_HQ,PUERTO_MI_RAM_HQ);
			int id=atoi(mensaje[1]);

			expulsar_tripulante_msg* mensaje=malloc(sizeof(expulsar_tripulante_msg));
			mensaje->idPatota=0;
			mensaje->idTripulante=id;
			enviar_paquete(mensaje,EXPULSAR_TRIPULANTE,socket);

			bool tieneMismoNombre(void*elemento){
				t_tripulante*tripulante= (t_tripulante*) elemento;
				return tripulante->idTripulante==id;
			}

			list_remove_by_condition(tripulantes,tieneMismoNombre);

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
			//recibirMensaje();

			break;
		}
		case ERROR_CODIGO: {

			break;
		}
		}


		free(leido);
		leido = readline("\n>");
	}

	free(leido);

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
	while(1){
		pthread_mutex_lock(&mutex_tripulantes);
		if(a==3&&tripulante->quantumDisponible==QUANTUM){
			printf("hola soy que se yo un tripulante: %d\n",tripulante->idTripulante);
						a=0;
		}
		if(a==2 &&tripulante->quantumDisponible==QUANTUM){
			printf("hola soy el segundo tripulante: %d\n",tripulante->idTripulante);
			a=3;
			tripulante->quantumDisponible=5;
		}
		if(a==1){
			puts("hola");
			printf("hola soy el primer tripulante: %d\n",tripulante->idTripulante);
			a=2;
			tripulante->quantumDisponible=5;
		}
		pthread_mutex_unlock(&mutex_tripulantes);
						}
		//sem_t* semaforoDelTripulante = (sem_t*) list_get(sem_tripulantes_ejecutar, tripulante->idTripulante);
		//		sem_wait(semaforoDelTripulante);

		//if(tripulante->tareaAsignada==NULL){}

	}

