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



int main(void){
	inicializarConfig(config);
	puts("!!!hola soy el discordiador!!!");

	iniciarLog();

	consolita();

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
	log = log_create("discordiador.log", "discordiador", 1, LOG_LEVEL_INFO);
	puts("log creado");
}

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento

	sem_init(&sem_planificar, 0, GRADO_MULTITAREA);

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





void consolita(){

	//posicionesTripulantes= list_create();
	char* leido=malloc(sizeof(char*));
	leido=readline(">");
	while(strcmp(leido, "") != 0){

		char** mensaje = string_n_split(leido, 10, " ");
		log_debug(log, leido);

		char* codigo = string_from_format("%s %s",mensaje[0], mensaje[1]);

		opCode codigo_mensaje = string_a_op_code(codigo);


		switch(codigo_mensaje){

		case INICIAR_PATOTA: {
			printf("hola soy INICIAR_PATOTA");

			break;
		}


		case LISTAR_TRIPULANTES: {
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			break;
		}
		case INICIAR_PLANIFICACION: {		//Con este comando se dará inicio a la planificación (es un semaforo sem init)
			break;
		}
		case PAUSAR_PLANIFICACION: {		//Este comando lo que busca es detener la planificación en cualquier momento(semaforo)
			break;
		}
		case OBTENER_BITACORA: {
			break;
		}
		default:{

			break;
		}
		}


	}}

void leer_consola(uint32_t* socket_server){

	posicionesTripulantes= list_create();
	char* leido;
	leido=readline(">");
	while(strcmp(leido, "") != 0){

		char** mensaje = string_n_split(leido, 10, " ");
		log_debug(log, leido);

		char* codigo = string_from_format("%s %s",mensaje[0], mensaje[1]);

		opCode codigo_mensaje = string_a_op_code(codigo);

		switch(codigo_mensaje){

		case INICIAR_PATOTA: {  //LEO INICIAR_PATOTA, LEO SU CANTIDAD,LEO LA CANTIDAD DE TAREAS Y LAS METO DENTRO DE UNA LISTA DE TAREAS,
			//ALGORITMO PARA CONSEGUIR UNA LISTA DE POSICIONES
			//SACO TRIPULANTES DE LAS POSICIONES Y CREO STRUCT DE TRIPULANTES Y LOS AGREGO A LAS LISTAS.
			//ENVIO MENSAJES DE TODOS LOS DATOS A MI RAM HQ.



			t_iniciarPatotaMsg*patota;
			int totalPatota=atoi(mensaje[1]);
			patota->cantPatota=(totalPatota);		//le estoy pasando un string
			char*tarea[1000];
			FILE*fileTarea;

			if((fileTarea=fopen(mensaje[2],"r")) == NULL){
				printf("Error no se pudo abrir el file");
				//separar tareas por enter y separar atributos por ;

			}else{
				while(fileTarea != NULL){
					//	fscanf(fileTarea,"%[^\n]",tarea);							//TO DO
					list_add(tareas,tarea);		//si sobreescribe, piola!
				}
			}
			fclose(fileTarea);

			if(sizeof(mensaje) != (3+totalPatota) ){

			}
			for(int i=0; i<(3+totalPatota-sizeof(mensaje)); i++){		//TO DO
				list_add(posicionesTripulantes,"0|0");

			}

			int contadorLista=0;
			int sizeListaMensajeHarcodeada=3+contadorLista;
			while(sizeof(mensaje)!=sizeListaMensajeHarcodeada){
				list_add(posicionesTripulantes,mensaje[sizeListaMensajeHarcodeada]);
				contadorLista++;
			}
			contadorLista=0;

			//MOMENTO DE CREAR TRIPULANTES

			tripulantes = list_create();	//creamos la lista de tripulantes



			for(int i=0; i<(list_size(posicionesTripulantes)) ; i++){		//TODO


				t_tripulante*tripulante=malloc(sizeof(t_tripulante));;
				char* posicion = list_get(posicionesTripulantes,i);
				char** posicionesSplit = string_split(posicion, "|");

				tripulante->coordenadas->posX= atoi(posicionesSplit[0]);		//TODO



				tripulante->coordenadas->posY=atoi(posicionesSplit[1]);

//				pthread_mutex_lock(&mutex_tripulantes);
//				list_add(tripulantes, tripulante);
//				pthread_mutex_unlock(&mutex_tripulantes);
//
//				pthread_mutex_lock(&mutex_listaNuevos);
//				list_add(listaNuevos, tripulante);
//				pthread_mutex_unlock(&mutex_listaNuevos);


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
		case INICIAR_PLANIFICACION: {		//Con este comando se dará inicio a la planificación (es un semaforo sem init)

			break;
		}
		case PAUSAR_PLANIFICACION: {		//Este comando lo que busca es detener la planificación en cualquier momento(semaforo)

			break;
		}
		case OBTENER_BITACORA: {			//Este comando obtendrá la bitácora del tripulante pasado por parámetro a través de una consulta a i-Mongo-Store.

			//enviarMensaje();
			//recibirMensaje();

			break;
		}
		default:{

			break;
		}
		}

		leido = readline("\n>");
	}

	free(leido);
}




