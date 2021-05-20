/*
 ============================================================================
 Name        : mi_Ram_Hq.c
 Author      : duroDeAProbar
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "mi_Ram_Hq.h"

NIVEL* mapa;

int main(void) {

	init();

	int32_t socket_servidor = iniciar_servidor(IP, PUERTO);

	while(true){

		int32_t socket_cliente = esperar_cliente(socket_servidor);
		pthread_t hilo_mensaje;
		pthread_create(&hilo_mensaje,NULL,(void*)recibir_mensaje, (void*) (&socket_cliente));
		pthread_detach(hilo_mensaje);

	}

	terminar();

	return EXIT_SUCCESS;
}

void recibir_mensaje(int32_t* conexion){

	bool terminado = false;
	while (!terminado){

		t_paquete* paquete = recibir_paquete(*conexion);

		switch(paquete->codigo){
			case INICIAR_PATOTA_MSG:{

				iniciar_patota_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id patota: %d", mensaje->idPatota);
				log_debug(logger, "Tareas: %s", mensaje->tareas->string);

				free(mensaje->tareas);
				free(mensaje);

				break;

			}
			case INICIAR_TRIPULANTE:{

				iniciar_tripulante_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id patota: %d", mensaje->idPatota);
				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);
				log_debug(logger, "PosX: %d", mensaje->coordenadas->posX);
				log_debug(logger, "PosY: %d", mensaje->coordenadas->posY);

				free(mensaje->coordenadas);
				free(mensaje);

				break;

			}
			case INFORMAR_MOVIMIENTO_RAM:{

				informar_movimiento_ram_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);
				log_debug(logger, "PosX: %d", mensaje->coordenadasDestino->posX);
				log_debug(logger, "PosY: %d", mensaje->coordenadasDestino->posY);

				free(mensaje->coordenadasDestino);
				free(mensaje);

				break;

			}
			case CAMBIO_ESTADO:{

				cambio_estado_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Estado: %d", mensaje->estado);
				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);

				free(mensaje);

				break;

			}
			case SOLICITAR_SIGUIENTE_TAREA:{

				solicitar_siguiente_tarea_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);

				break;
			}
			case EXPULSAR_TRIPULANTE_MSG:{
				expulsar_tripulante_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id tripulante %d", mensaje->idTripulante);

				free(mensaje);

				break;

			}
			default: terminado = true; break;
		}
	}

	pthread_exit(NULL);
}

void init (){

	config = config_create("ram.config");
	logger = log_create("ram.log", "MI-RAM-HQ", true, LOG_LEVEL_DEBUG);

	TAMANIO_MEMORIA = config_get_int_value(config, "TAMANIO_MEMORIA");
	ESQUEMA_MEMORIA = get_esquema_memoria(config_get_string_value(config, "ESQUEMA_MEMORIA"));
	TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
	TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_REEMPLAZO");;
	IP = config_get_string_value(config, "IP");
	PUERTO = config_get_string_value(config, "PUERTO");

	memoria_principal = malloc(TAMANIO_MEMORIA);

	iniciarMapa();

}

void iniciarMapa(){

	int columnas, filas;

	//nivel_gui_inicializar();

	//nivel_gui_get_area_nivel(&columnas, &filas);

	//mapa = nivel_crear("Nave");

}

void terminar(){

	log_destroy(logger);
	config_destroy(config);
	free(memoria_principal);
	//nivel_gui_terminar();

}

int32_t get_esquema_memoria(char* esquema_config) {

	if(strcmp("PAGINACION", esquema_config) == 0){

		return PAGINACION_VIRTUAL;

	} else if(strcmp("SEGMENTACION", esquema_config) == 0){

		return SEGMENTACION_PURA;

	}

	return -1;
}
