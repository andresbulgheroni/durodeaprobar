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



int main(void){
	inicializarConfig(config);
	puts("!!!hola soy el discordiador!!!"); /* prints !!!Hello World!!! */
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

void inicializarSemaforoPlanificador(){			//Maneja el multiprocesamiento

	sem_init(&sem_planificar, 0, GRADO_MULTITAREA);

}

void leer_consola(uint32_t* socket_server)
{
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
				int totalPatota=mensaje[1];
				patota->cantPatota=totalPatota;
				char*tarea[1000];
				FILE*fileTarea;

				if((fileTarea=fopen(mensaje[2],"r")) == NULL){
					printf("Error no se pudo abrir el file");
					//separar tareas por enter y separar atributos por ;

				}else{
					while(fileTarea != NULL){
						fscanf(fileTarea,"%[^\n]",tarea);
						list_add(tareas,tarea);		//si sobreescribe, piola!
					}
				}
				fclose(fileTarea);

				if(sizeof(mensaje) != (3+totalPatota) ){
					for(int i=0; i<(3+totalPatota-sizeof(mensaje)); i++){
						list_add(posicionesTripulantes,"0|0");

					}

					int contadorLista=0;
					int sizeListaMensaje=3+contadorLista;
				while(sizeof(mensaje)!=sizeListaMensaje){
					list_add(posicionesTripulantes,mensaje[sizeListaMensaje]);
					contadorLista++;
				}
				contadorLista=0;

				crearTripulantes();


				tripulantes = list_create();	//creamos la lista de tripulantes

				int i = 0, j = 0;

				while (posicionesTripulantes[i] != NULL) {		//1|2,2|2;
						t_tripulante tripulante;
						char* posicion = list_get(posicionesTripulantes,i);
						tripulante->coordenadas->posX=atoi(posicion[i][0]);
						tripulante->coordenadas->posY=atoi(posicion[i][2]);


				}

				/*
				    void ponerRepartidoresEnLista() {



	repartidores = list_create(); //Creamos la lista de repartidores

	char** coordenadasRepartidores = REPARTIDORES;

	char** frecuenciaDeDescanso = FRECUENCIA_DE_DESCANSO;

	char** tiempoDeDescanso= TIEMPO_DE_DESCANSO;

	int i = 0, j = 0;


	while (coordenadasRepartidores[i] != NULL) {
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));

		t_coordenadas* coords = malloc(sizeof(t_coordenadas));

		coords->posX = atoi(&coordenadasRepartidores[i][0]);
		coords->posY = atoi(&coordenadasRepartidores[i][2]);

		uint32_t frecuenciaDescanso=atoi(frecuenciaDeDescanso[j]);		//VA &??
		uint32_t tiempoDescanso=atoi(tiempoDeDescanso[j]);				//VA &??

		uint32_t idRepartidor = generar_id_repartidor();

		repartidor->id_repartidor = idRepartidor;
			repartidor->coordenadas = coords;
			repartidor->estado = NEW;
			repartidor->misCiclosDeCPU = 0;


			repartidor->frecuenciaDeDescanso=frecuenciaDescanso;
			repartidor->tiempoDeDescanso=tiempoDescanso;

			repartidor->tiempoDisponibleFrecuenciaDeDescanso=repartidor->frecuenciaDeDescanso;


			repartidor->estimacionInicial = estimacionInicial;
			repartidor->rafagaAnteriorReal = 0;
			repartidor->tiempoDeEsperaEnReady=0; //HRRN
			repartidor->duracionProximaRafagaCPU=0;

		pthread_mutex_lock(&mutex_repartidores);
		list_add(repartidores, repartidor);
		pthread_mutex_unlock(&mutex_repartidores);

		pthread_mutex_lock(&mutex_listaNuevos);
		list_add(listaNuevos, repartidor);
		pthread_mutex_unlock(&mutex_listaNuevos);
		j++;
		i++;

	}

	pthread_mutex_lock(&mutex_repartidores);
	int cantRepartidores = list_size(repartidores);
	sem_init(&sem_buscarRepartidorMasCercano, 0, cantRepartidores);
	pthread_mutex_unlock(&mutex_repartidores);



	liberarArray(coordenadasRepartidores);
	liberarArray(frecuenciaDeDescanso);
	liberarArray(tiempoDeDescanso);
}
					void sacarRepartidorDeLista(t_repartidor* repartidor, t_list* lista){
					int a = list_size(lista);
					for(int i=0; i<a ; i++){
						t_repartidor* RepartidorDeLista = list_get(lista, i);
						if(repartidor->id_repartidor == RepartidorDeLista->id_repartidor){
							list_remove(lista, i);
							break;
						}
					}
				}
				*/












//				int variableTripulante=0;
//
//				if(mensaje[3]==NULL){
//					int tripulantesPosicionDefault=totalPatota-variableTripulante;
//					crearTripulantesEnPosicionCero();  //TODO
//				}else{
//
//				while((variableTripulante+totalPatota)<=0){
//
//				}
//
//				}


				//aca creo al tripulante










				break;
			case LISTAR_TRIPULANTES: {
				//	fput(tripulantes);
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
