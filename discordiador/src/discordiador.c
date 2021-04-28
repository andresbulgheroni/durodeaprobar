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


pthread_mutex_t mutex_id_tripulantes = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_tripulantes = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_listaNuevos= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaReady = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaEjecutando = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaFinalizados = PTHREAD_MUTEX_INITIALIZER;

uint32_t ID_TRIPULANTES = 0;
uint32_t ID_PATOTA = 0;


int main(void){
	inicializarConfig(config);
	puts("!!!hola soy el discordiador!!!");

	iniciarLog();

	leer_consola();

	puts("LLEGO AL FIN DEL PROGRAMA");

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

void iniciarLog(){

	logger = log_create("discordiador.log", "discordiador", 1, LOG_LEVEL_INFO);
	puts("log creado");

}

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento

	sem_init(&sem_planificar, 0, GRADO_MULTITAREA);

}

uint32_t generar_id(uint32_t id) {
	pthread_mutex_lock(&mutex_id_tripulantes);
	uint32_t id_generado = id++;
	pthread_mutex_unlock(&mutex_id_tripulantes);

	return id_generado;
}

opCode string_a_op_code (char* string){

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
	}
	return ERROR_CODIGO;
}

t_coordenadas* get_coordenadas(char* posicion){

	char** posicionesSplit = string_split(posicion, "|");

	t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));

	coordenadas->posX = atoi(posicionesSplit[0]);
	coordenadas->posY = atoi(posicionesSplit[1]);

	return coordenadas;
}

void agregarAtributosATripulante(){

	tripulantes = list_create();

	for(int i=0; i<(list_size(posicionesTripulantes)) ; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));
		char* posicion = list_get(posicionesTripulantes,i);
		tripulante->coordenadas = get_coordenadas(posicion);
		tripulante->idTripulante = generar_id(ID_TRIPULANTES);
		tripulante->idPatota = ID_PATOTA;

		list_add(tripulantes,tripulante);

	}
	ID_PATOTA++;

}


void leer_consola(){ // proximamente recibe como parm uint32_t* socket_server

	posicionesTripulantes= list_create();
	char* leido=readline(">");

	while(strcmp(leido, "") != 0){

		log_info(logger, leido);

		char** mensaje = string_split(leido, " ");
		opCode codigo_mensaje = string_a_op_code(mensaje[0]);
		int cantidadMensaje=atoi(mensaje[1]);

		printf("el valor es: %d\n",cantidadMensaje);

		switch(codigo_mensaje){

		case INICIAR_PATOTA: {

			t_iniciarPatotaMsg* patota = malloc(sizeof(t_iniciarPatotaMsg*));
			patota->cantPatota = atoi(mensaje[1]);
			char* rutaTarea = string_new();
			string_append(&rutaTarea,mensaje[2]);
			FILE* fileTarea = fopen(rutaTarea,"r");

			if(fileTarea != NULL){

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

				agregarAtributosATripulante();

				fclose(fileTarea);

			}else{
				log_info(logger, "No existe la tarea");
			}

			break;

		}

		case LISTAR_TRIPULANTES: {
			//	fput(tripulantes);
			//date
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			//conseguirTripulante;
			//list_add(listaFinalizados,tripulante);
			//enviarMensaje();
			break;
		}
		case INICIAR_PLANIFICACION: { //Con este comando se dará inicio a la planificación (es un semaforo sem init)
			break;
		}
		case PAUSAR_PLANIFICACION: { //Este comando lo que busca es detener la planificación en cualquier momento(semaforo)
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
