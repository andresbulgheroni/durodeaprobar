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

pthread_mutex_t  m_MEM_PRINCIPAL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_MEM_VIRTUAL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLAS_PAGINAS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLA_LIBRES_P = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLA_LIBRES_V = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_LISTA_REEMPLAZO = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_LOGGER = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t  m_SEGMENTOS_LIBRES = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_SEG_EN_MEMORIA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLAS_SEGMENTOS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_MAPA = PTHREAD_MUTEX_INITIALIZER;

NIVEL* mapa;

void sig_handler(int n){

	switch(n){
		case SIGUSR1:{


			char* time_stamp_text = get_timestamp();
			char* time_stamp_file = temporal_get_string_time("%d_%m_%y_%H:%M:%S");
			char* path = string_from_format("/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/dump/Dump_%s.dmp", time_stamp_file);
			char* inicio_texto = string_from_format("Dump: %s\n", time_stamp_text);

			FILE* dump = fopen(path, "w+");

			fputs(inicio_texto, dump);

			free(path);
			free(inicio_texto);
			free(time_stamp_text);
			free(time_stamp_file);

			switch(ESQUEMA_MEMORIA){
				case PAGINACION_VIRTUAL: dump_paginacion(dump); break;
				case SEGMENTACION_PURA: dump_segmentacion(dump); break;
			}

			fclose(dump);
			break;
		}

		case SIGUSR2:{
			if(ESQUEMA_MEMORIA == SEGMENTACION_PURA)
				compactar_memoria();
			break;
		}
		case SIGINT:{
			switch(ESQUEMA_MEMORIA){
				case PAGINACION_VIRTUAL: terminar_paginacion(); break;
				case SEGMENTACION_PURA: terminar_segmentacion(); break;
			}
			exit(2);
			break;
		}
		default: break;
	}

}

void mostrar_mapa_opt(int option){

	switch(option){
		case 1: mapa_mostrar = true; break;
		case 2: mapa_mostrar = false; break;
		default: printf("Opcion ingresada no valida"); exit(2); break;
	}

}

int main(void) {

	printf("\n--------------------------------------\n");
	printf("mi-RAM-HQ iniciado!   PID: %d\n",getpid());
	printf("Cerrar modulo: ctrl + c");
	printf("\n--------------------------------------\n");

	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGINT, sig_handler);

	mapa_mostrar = true;

	char* option = readline("\nSeleccione la configuracion que desea utilizar:\n1-GENERAL\n2-PRUEBA DISCORDIADOR CPU\n3-PRUEBA DISCORDIADOR E/S\n4-PRUEBA RAM SEGMENTACION FF\n5-PRUEBA RAM SEGMENTACION BF\n6-PRUEBA RAM PAGINACION LRU\n7-PRUEBA RAM PAGINACION CLOCK\n8-PRUEBA FILE SYSTEM\n>");
	char* mapa_option = readline("\nDesea visualizar el mapa?\n1-SI\n2-NO\n>");

	mostrar_mapa_opt(atoi(mapa_option));

	init(option);

	free(mapa_option);
	free(option);

	pthread_t hilo_server;
	pthread_create(&hilo_server,NULL,(void*)hilo_servidor, NULL);
	pthread_join(hilo_server, NULL);

	terminar();

	return EXIT_SUCCESS;

}

char* get_timestamp(){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char* timestamp = string_from_format("%d/%02d/%02d %02d:%02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return timestamp;

}

void dump_paginacion(FILE* dump){
	//TODO PREGUNTAR SI HACE FALTAN LOS SEMAFOROS, COMO SE TRATA DE UNA INTERRUPCION

	t_list* tabla_dump = list_create();

	void leer_libres(t_frame* frame){

		t_dump_pag* pag = malloc(sizeof(t_dump_pag));

		pag->marco = frame->pos;
		pag->estado = "LIBRE";
		pag->pagina = "-";
		pag->proceso = "-";

		list_add(tabla_dump, pag);

	}
	list_iterate(frames_libres_principal, leer_libres);

	void leer_paginas(char* key, t_tabla_paginas* tabla){

		t_list* paginas = tabla->tabla_paginas;

		for(int32_t i = 0; i < list_size(paginas); i++){

			t_pagina_patota* pagina = list_get(paginas, i);

			if(pagina->presente){

				t_dump_pag* pag = malloc(sizeof(t_dump_pag));

				pag->marco = pagina->nro_frame;
				pag->estado = "OCUPADO";
				pag->proceso = malloc(strlen(key) + 1);
				strcpy(pag->proceso, key);
				pag->pagina = string_itoa(pagina->nro_pagina);

				list_add(tabla_dump, pag);
			}

		}

	}
	dictionary_iterator(tabla_paginas_patota, leer_paginas);

	bool ordenar_tabla(t_dump_pag* marco1, t_dump_pag* marco2){

		return marco1->marco < marco2->marco;

	}
	list_sort(tabla_dump, ordenar_tabla);

	void guardar_tabla(t_dump_pag* pag){

		char* fila = string_from_format("MARCO:%d\tESTADO:%s\tPROCESO:%s\tPAGINA:%s\n", pag->marco, pag->estado, pag->proceso, pag->pagina);
		fputs(fila, dump);
		free(fila);
	}

	list_iterate(tabla_dump, guardar_tabla);

	void vaciar_lista(t_dump_pag* pag){	}
	list_destroy_and_destroy_elements(tabla_dump, vaciar_lista);
}


void hilo_servidor(){

	int32_t socket_servidor = iniciar_servidor(IP, PUERTO);

	while(true){

		int32_t* socket_cliente = malloc(sizeof(int32_t));
		*socket_cliente = esperar_cliente(socket_servidor);
		pthread_t hilo_mensaje;
		pthread_create(&hilo_mensaje,NULL,(void*)recibir_mensaje, (void*) (socket_cliente));
		pthread_detach(hilo_mensaje);

	}

}

void recibir_mensaje(int32_t* conexion){

	bool terminado = false;
	while (!terminado){

		t_paquete* paquete = recibir_paquete(*conexion);

		switch(paquete->codigo){
			case INICIAR_PATOTA_MSG:{

				iniciar_patota_msg* mensaje = deserializar_paquete(paquete);
				pthread_mutex_lock(&m_LOGGER);
					log_info(logger, "SOLICITUD INICIO PATOTA %d", mensaje->idPatota);
				pthread_mutex_unlock(&m_LOGGER);
				bool status = true;

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: crear_patota_segmentacion(mensaje, &status);break;
					case PAGINACION_VIRTUAL: crear_patota_paginacion(mensaje, &status);break;
				}

				if(status){
					pthread_mutex_lock(&m_LOGGER);
						log_info(logger, "SE CREO LA PATOTA CON EXITO %d", mensaje->idPatota);
					pthread_mutex_unlock(&m_LOGGER);
					enviar_paquete(NULL, OK_MSG, *conexion);
				}else{
					char* mensaje_err = string_new();
					mensaje_err = "Fallo la insercion de la patota";

					t_string* rta_error = get_t_string(mensaje_err);

					enviar_paquete(rta_error, FAIL_MSG, *conexion);

					free(rta_error);

					pthread_mutex_lock(&m_LOGGER);
						log_info(logger, "FALLO LA CREACION DE LA PATOTA %d", mensaje->idPatota);
					pthread_mutex_unlock(&m_LOGGER);

				}

				void vaciar_trips(tripulante_data_msg* trip){
					free(trip);
				}
				list_destroy_and_destroy_elements(mensaje->tripulantes, vaciar_trips);

				if(strcmp(mensaje->tareas->string, "") != 0)
					free(mensaje->tareas->string);
				free(mensaje->tareas);
				free(mensaje);

				break;

			}
			case INFORMAR_MOVIMIENTO_RAM:{

				informar_movimiento_ram_msg* mensaje = deserializar_paquete(paquete);

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: informar_movimiento_segmentacion(mensaje, NULL);break;
					case PAGINACION_VIRTUAL: informar_movimiento_paginacion(mensaje, NULL);break;
				}

				free(mensaje->coordenadasDestino);
				free(mensaje);


				break;

			}
			case CAMBIO_ESTADO:{

				cambio_estado_msg* mensaje = deserializar_paquete(paquete);
				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: cambiar_estado_segmentacion(mensaje, NULL);break;
					case PAGINACION_VIRTUAL: cambiar_estado_paginacion(mensaje, NULL);break;
				}

				free(mensaje);

				break;

			}
			case SOLICITAR_SIGUIENTE_TAREA:{

				solicitar_siguiente_tarea_msg* mensaje = deserializar_paquete(paquete);

				bool completo_tareas = false;
				char* tarea;
				pthread_mutex_lock(&m_LOGGER);
					log_info(logger, "SOLICITO TAREA TRIPULANTE %d DE LA PATOTA %d", mensaje->idTripulante, mensaje->idPatota);;
				pthread_mutex_unlock(&m_LOGGER);

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: tarea = siguiente_tarea_segmentacion(mensaje, &completo_tareas, NULL);break;
					case PAGINACION_VIRTUAL: tarea = siguiente_tarea_paginacion(mensaje, &completo_tareas, NULL);break;
				}

				if(completo_tareas){
					//printf("COMPLETO TAREAS TRIP %d PAT %d\n", mensaje->idTripulante, mensaje->idPatota);
					enviar_paquete(NULL, COMPLETO_TAREAS, *conexion);
				}else{
					pthread_mutex_lock(&m_LOGGER);
						log_info(logger, "EL TRIPULANTE %d DE LA PATOTA %d TIENE LA TAREA %s", mensaje->idTripulante, mensaje->idPatota, tarea);;
					pthread_mutex_unlock(&m_LOGGER);
					t_string* tarea_msg = get_t_string(tarea);
					enviar_paquete(tarea_msg, SOLICITAR_SIGUIENTE_TAREA_RTA, *conexion);
					free(tarea_msg);
					free(tarea);
				}

				free(mensaje);


				break;
			}
			case EXPULSAR_TRIPULANTE_MSG:{

				expulsar_tripulante_msg* mensaje = deserializar_paquete(paquete);
				pthread_mutex_lock(&m_LOGGER);
					log_info(logger, "SE SOLICITO EXPULSAR AL TRIPULANTE %d DE LA PATOTA %d\n", mensaje->idTripulante, mensaje->idPatota);
				pthread_mutex_unlock(&m_LOGGER);

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: expulsar_tripulante_segmentacion(mensaje, NULL);break;
					case PAGINACION_VIRTUAL: expulsar_tripulante_paginacion(mensaje, NULL);break;
				}

				free(mensaje);

				break;

			}

			DESCONECTADO:
			default: terminado = true; break;
		}
	}
	close(*conexion);
	free(conexion);
	pthread_exit(NULL);
}

void init (char* config_elect){

	char* cfg_path = get_config(atoi(config_elect));

	if(strcmp("err", cfg_path) == 0){
		printf("LA CONFIGURACION SOLICITADA NO ES CORRECTA");
		exit(2);
	}

	config = config_create(cfg_path);

	logger = log_create("/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/ram.log", "MI-RAM-HQ", false, LOG_LEVEL_INFO);

	TAMANIO_MEMORIA = config_get_int_value(config, "TAMANIO_MEMORIA");
	char* esquema = config_get_string_value(config, "ESQUEMA_MEMORIA");
	ESQUEMA_MEMORIA = get_esquema_memoria(esquema);
	TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
	TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	char* algoritmo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	ALGORITMO_REEMPLAZO = get_algoritmo(algoritmo);;
	IP = config_get_string_value(config, "IP");
	char* criterio = config_get_string_value(config, "CRITERIO_SELECCION");
	CRITERIO_SELECCION = get_criterio(criterio);
	PUERTO = config_get_string_value(config, "PUERTO");

	log_info(logger, "\n\n\nINICIA EJECUCION DE RAM");

	log_info(logger, "ESQUEMA DE MEMORIA: %s", esquema);
	log_info(logger, "TAMANIO MEMORIA PRINCIPAL: %d", TAMANIO_MEMORIA);

	memoria_principal = malloc(TAMANIO_MEMORIA);

	switch(ESQUEMA_MEMORIA){
		case SEGMENTACION_PURA:{

			log_info(logger, "CRITERIO DE SELECCION: %s", criterio);

			inicializar_segmentacion();

			break;
		}
		case PAGINACION_VIRTUAL:{
			log_info(logger, "TAMANIO SWAP: %d", TAMANIO_SWAP);
			log_info(logger, "TAMANIO PAGINA: %d", TAMANIO_PAGINA);
			log_info(logger, "ALGORITMO DE REEMPLAZO: %s", algoritmo);

			configurar_paginacion();

			break;
		}
	}

	if(mapa_mostrar)
		iniciarMapa();

}

void configurar_paginacion(){


	// TODO Probar archivo con path a la carpeta /home/utnso/tp-2021-1c-DuroDeAprobar/mi Ram Hq/
	if(TAMANIO_MEMORIA%TAMANIO_PAGINA != 0 || TAMANIO_SWAP%TAMANIO_PAGINA != 0){
		log_error(logger, "Mal configurados tamanios de memoria y pagina");
		exit(2);
	}
	fileDes = open(PATH_SWAP, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | 0777);
//	int32_t fileDes = open(PATH_SWAP, O_RDWR | O_APPEND | O_CREAT, 0777);

	if(fileDes == -1){
		log_error(logger, "No se pudo abrir el archivo");
		close(fileDes);
		exit(2);
	}

	if(fileDes >= 0){

		lseek(fileDes, TAMANIO_SWAP - 1, SEEK_SET);

		write(fileDes, "", 1);

		memoria_virtual =  mmap(0, TAMANIO_SWAP, PROT_WRITE | PROT_READ, MAP_SHARED, fileDes, 0);

		if(memoria_virtual == MAP_FAILED){
			log_error(logger, "Fallo en el mapeo de archivo SWAP");
			close(fileDes);
			exit(2);
		}

		frames_swap = list_create();
		frames_libres_principal = list_create();
		lista_para_reemplazo = list_create();

		for(uint32_t i = 0; i < (TAMANIO_SWAP / TAMANIO_PAGINA);i++){
			t_frame* frame_swap = malloc(sizeof(t_frame));
			frame_swap->pos = i;
			list_add(frames_swap, frame_swap);
		}

		for(uint32_t i = 0; i < (TAMANIO_MEMORIA / TAMANIO_PAGINA); i++){
			t_frame* frame_p = malloc(sizeof(t_frame));
			frame_p->pos = i;
			list_add(frames_libres_principal, frame_p);
		}

		if(ALGORITMO_REEMPLAZO == CLOCK){
			buffer_clock_pos = 0;
			for(uint32_t i = 0; i < (TAMANIO_MEMORIA / TAMANIO_PAGINA); i++){
				t_buffer_clock* frame = malloc(sizeof(t_buffer_clock));
				frame->pagina = NULL;
				list_add(lista_para_reemplazo, frame);
			}
		}

		tabla_paginas_patota = dictionary_create();

	}

}

void iniciarMapa(){

	int columnas, filas;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&columnas, &filas);

	mapa = nivel_crear("Nave");

	nivel_gui_dibujar(mapa);

}

void* render_mapa(){

	while(true){

		pthread_mutex_lock(&m_MAPA);
		nivel_gui_dibujar(mapa);
		pthread_mutex_unlock(&m_MAPA);

	}

	return NULL;
}


void terminar_paginacion(){
	void destroy(void* a){
		free(a);
	}

	if(frames_swap != NULL)
		list_destroy_and_destroy_elements(frames_swap, destroy);
	if(frames_libres_principal != NULL)
		list_destroy_and_destroy_elements(frames_libres_principal, destroy);
	if(lista_para_reemplazo != NULL)
		list_destroy_and_destroy_elements(lista_para_reemplazo, destroy);
	void destroy_dict(t_tabla_paginas* elemento){
		if(lista_para_reemplazo != NULL)
			list_destroy_and_destroy_elements(elemento->tabla_direcciones, destroy);
		if(lista_para_reemplazo != NULL)
			list_destroy_and_destroy_elements(elemento->tabla_paginas, destroy);
		pthread_mutex_destroy(&(elemento->m_TABLA));
		free(elemento);
	}

	if(tabla_paginas_patota != NULL)
		dictionary_destroy_and_destroy_elements(tabla_paginas_patota, destroy_dict);

	terminar();
}

void terminar(){

	log_destroy(logger);
	config_destroy(config);
	if(memoria_principal != NULL)
		free(memoria_principal);

	nivel_gui_terminar();

	pthread_mutex_destroy(&m_LISTA_REEMPLAZO);
	pthread_mutex_destroy(&m_LOGGER);
	pthread_mutex_destroy(&m_MEM_PRINCIPAL);
	pthread_mutex_destroy(&m_MEM_VIRTUAL);
	pthread_mutex_destroy(&m_SEGMENTOS_LIBRES);
	pthread_mutex_destroy(&m_SEG_EN_MEMORIA);
	pthread_mutex_destroy(&m_TABLAS_PAGINAS);
	pthread_mutex_destroy(&m_TABLAS_SEGMENTOS);
	pthread_mutex_destroy(&m_TABLA_LIBRES_P);
	pthread_mutex_destroy(&m_TABLA_LIBRES_V);
	pthread_mutex_destroy(&m_MAPA);

}


uint32_t generar_direccion_logica_paginacion(uint32_t pagina, uint32_t desplazamiento){

	uint32_t direccion = 0;

	uint32_t bits_derecha = floor(log((double)TAMANIO_PAGINA) / log(2)) + 1;

	direccion = (pagina << bits_derecha) | desplazamiento;

	return direccion;

}

void obtener_direccion_logica_paginacion(uint32_t* pagina, uint32_t* desplazamiento, uint32_t direccion){

	uint32_t bits_derecha = floor(log((double)TAMANIO_PAGINA) / log(2)) + 1;
	uint32_t bits_izquierda = 32  - bits_derecha;

	*pagina = (direccion >> bits_derecha);
	*desplazamiento = (direccion << bits_izquierda) >> bits_izquierda;

}

void crear_patota_paginacion(iniciar_patota_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);
	char* id_patota_string = string_itoa(mensaje->idPatota);
	if(!dictionary_has_key(tabla_paginas_patota, id_patota_string)){

		//Saco cantidad de frames/paginas que va a ocupar
		uint32_t size = (sizeof(t_pcb) + (mensaje->cant_tripulantes * sizeof(t_tcb))
				+  mensaje->tareas->length);

		uint32_t cantidad_frames = 1 + (size / TAMANIO_PAGINA);
		uint32_t size_pcb = cantidad_frames * TAMANIO_PAGINA;

		// MODIFICAR SWAPEO DE PAGINAS. INTERCAMBIAN ENTRE REELPLAZO Y REEMPLAZADO
		*status = entra_pcb(cantidad_frames);

		if(*status){

			t_list* paginas = list_create();
			uint32_t direccion_tareas = sizeof(t_pcb);

			//Cargo datos de pcb y tcb para copiarlos a memoria
			t_pcb* pcb = malloc(sizeof(t_pcb));
			t_list* tcbs = list_create();

			pcb->pid = mensaje->idPatota;

			void cargar_tcb(tripulante_data_msg* tripulante){

				t_tcb* tcb = malloc(sizeof(t_tcb));

				tcb->tid = tripulante->idTripulante;
				tcb->estado = 'R';
				tcb->posX = tripulante->coordenadas->posX;
				tcb->posY = tripulante->coordenadas->posY;
				tcb->proxima_instruccion = 0;
				tcb->direccion_patota = 0;

				list_add(tcbs, tcb);
				direccion_tareas += sizeof(t_tcb);

			}

			list_iterate(mensaje->tripulantes, cargar_tcb);

			pcb->direccion_tareas = generar_direccion_logica_paginacion(direccion_tareas / TAMANIO_PAGINA, direccion_tareas % TAMANIO_PAGINA);

			// Pongo los datos en un void* por conveniencia para el paso mem principal
			void* datos = malloc(size_pcb);
			int32_t offset = 0;

			memcpy(datos + offset, pcb, sizeof(t_pcb));
			offset += sizeof(t_pcb);

			void cargarTcbDatos(t_tcb* tcb){

				memcpy(datos + offset, tcb, sizeof(t_tcb));
				offset += sizeof(t_tcb);

			}
			list_iterate(tcbs, cargarTcbDatos);

			memcpy(datos + offset, mensaje->tareas->string, mensaje->tareas->length);

			free(pcb);

			void liberar_tcbs(t_tcb* tcb){
				free(tcb);
			}

			list_destroy_and_destroy_elements(tcbs, liberar_tcbs);

			// Creo las paginas y paso los datos a swap
			offset = 0;
			for(int32_t i = 0; i < cantidad_frames - 1; i++){

				t_pagina_patota* pagina = malloc(sizeof(t_pagina_patota));
				pagina->nro_pagina = i;
				pagina->presente = false;
				pagina->uso = false;
				pagina->nuevo = true;
				pagina->ocupado = TAMANIO_PAGINA;



				guardar_en_memoria_principal(pagina, datos + i * TAMANIO_PAGINA, id_patota_string);

				list_add(paginas, pagina);

			}

			t_pagina_patota* pagina = malloc(sizeof(t_pagina_patota));
			pagina->nro_pagina = (cantidad_frames - 1);
			pagina->presente = false;
			pagina->uso = false;
			pagina->nuevo = true;
			pagina->ocupado = TAMANIO_PAGINA - (size_pcb - size);;

			guardar_en_memoria_principal(pagina, datos + (cantidad_frames - 1) * TAMANIO_PAGINA, id_patota_string);

			list_add(paginas, pagina);

			t_list* tabla_direcciones = list_create();
			offset = sizeof(t_pcb);

			void cargar_direcciones(tripulante_data_msg* tripulante){

				t_direcciones_trips* direccion = malloc(sizeof(t_direcciones_trips));

				direccion->tid = tripulante->idTripulante;
				direccion->direccion_log = generar_direccion_logica_paginacion(offset/TAMANIO_PAGINA, offset%TAMANIO_PAGINA);
				offset += sizeof(t_tcb);

				list_add(tabla_direcciones, direccion);
			}

			list_iterate(mensaje->tripulantes, cargar_direcciones);

			free(datos);

			t_tabla_paginas* tabla = malloc(sizeof(t_tabla_paginas));
			tabla->tabla_paginas = paginas;
			pthread_mutex_init(&(tabla->m_TABLA), NULL);
			tabla->tabla_direcciones = tabla_direcciones;

			//Guardo tabla de paginas
			dictionary_put(tabla_paginas_patota, string_itoa(mensaje->idPatota), tabla);
			if(mapa_mostrar){

				pthread_mutex_lock(&m_MAPA);

				void cargar_en_mapa(tripulante_data_msg* tripulante){

					personaje_crear(mapa, get_tripulante_codigo(tripulante->idTripulante), tripulante->coordenadas->posX, tripulante->coordenadas->posY);
				}


				list_iterate(mensaje->tripulantes, cargar_en_mapa);

				nivel_gui_dibujar(mapa);

				pthread_mutex_unlock(&m_MAPA);

			}


		}

	}
	free(id_patota_string);
	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

}

char get_tripulante_codigo(int32_t id){

	return id + 64;

}

int32_t paginas_necesarias(uint32_t offset, uint32_t size){

	int32_t pagina_inicial = offset/TAMANIO_PAGINA;

	uint32_t nuevo_offset = offset + size;

	int32_t pagina_final = nuevo_offset/TAMANIO_PAGINA - (nuevo_offset % TAMANIO_PAGINA == 0 ? 1 : 0);

	return pagina_final - pagina_inicial + 1;

}

void informar_movimiento_paginacion(informar_movimiento_ram_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);
	char* id_patota_string = string_itoa(mensaje->idPatota);
	if(dictionary_has_key(tabla_paginas_patota, id_patota_string)){

		int32_t* posx_anterior = malloc(sizeof(int32_t));
		int32_t* posy_anterior = malloc(sizeof(int32_t));

		t_tabla_paginas* tabla =  dictionary_get(tabla_paginas_patota, id_patota_string);

		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
		pthread_mutex_lock(&(tabla->m_TABLA));

		bool find_tid(t_direcciones_trips* trip){
			return trip->tid == mensaje->idTripulante;
		}

		t_direcciones_trips* tripulante = list_find(tabla->tabla_direcciones, find_tid);

		if(tripulante != NULL){

			uint32_t pagina = 0;
			uint32_t desplazamiento = 0;

			obtener_direccion_logica_paginacion(&pagina, &desplazamiento, tripulante->direccion_log);

			uint32_t offset_pos = desplazamiento + sizeof(uint32_t) + sizeof(char);

			uint32_t paginas_mover = (offset_pos / TAMANIO_PAGINA);
			uint32_t desplazamiento_pos = (offset_pos % TAMANIO_PAGINA);

			uint32_t cargado = 0;
			uint32_t cargar = sizeof(uint32_t);
			uint32_t paginas_ne = paginas_necesarias(desplazamiento_pos, cargar);

			for(uint32_t i = 0; i < paginas_ne; i++){

				bool es_la_pagina (t_pagina_patota* patota){

					return patota->nro_pagina == i + pagina + paginas_mover;

				}

				t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

				pthread_mutex_lock(&m_MEM_PRINCIPAL);

				void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

				uint32_t disponible = TAMANIO_PAGINA - desplazamiento_pos;

				if(disponible < cargar){

					memcpy(posx_anterior + cargado, frame + desplazamiento_pos, disponible);
					memcpy(frame + desplazamiento_pos, &(mensaje->coordenadasDestino->posX) + cargado, disponible);

					cargado += disponible;
					cargar -= disponible;

					desplazamiento_pos = 0;

				}else{

					memcpy(posx_anterior + cargado, frame + desplazamiento_pos, cargar);
					memcpy(frame + desplazamiento_pos, &(mensaje->coordenadasDestino->posX) + cargado, cargar);

				}

				pthread_mutex_unlock(&m_MEM_PRINCIPAL);

			}

			offset_pos += sizeof(uint32_t);

			paginas_mover = (offset_pos / TAMANIO_PAGINA);
			desplazamiento_pos = (offset_pos % TAMANIO_PAGINA);

			cargado = 0;
			cargar = sizeof(uint32_t);
			paginas_ne = paginas_necesarias(desplazamiento_pos, cargar);

			for(uint32_t i = 0; i < paginas_ne; i++){

				bool es_la_pagina (t_pagina_patota* patota){

					return patota->nro_pagina == i + pagina + paginas_mover;

				}

				t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

				pthread_mutex_lock(&m_MEM_PRINCIPAL);

				void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

				uint32_t disponible = TAMANIO_PAGINA - desplazamiento_pos;

				if(disponible < cargar){

					memcpy(posy_anterior + cargado, frame + desplazamiento_pos, disponible);
					memcpy(frame + desplazamiento_pos, &(mensaje->coordenadasDestino->posY) + cargado, disponible);

					cargado += disponible;
					cargar -= disponible;

					desplazamiento_pos = 0;

				}else{

					memcpy(posy_anterior + cargado, frame + desplazamiento_pos, cargar);
					memcpy(frame + desplazamiento_pos, &(mensaje->coordenadasDestino->posY) + cargado, cargar);

				}

				pthread_mutex_unlock(&m_MEM_PRINCIPAL);

			}

			pthread_mutex_unlock(&(tabla->m_TABLA));

			pthread_mutex_lock(&m_LOGGER);
				log_info(logger, "EL TRIPULANTE %d DE LA PATOTA %d SE MOVIO A %d|%d DESDE %d|%d\n", mensaje->idTripulante,
							mensaje->idPatota, mensaje->coordenadasDestino->posX, mensaje->coordenadasDestino->posY,
							*posx_anterior, *posy_anterior);
			pthread_mutex_unlock(&m_LOGGER);

			if(mapa_mostrar){

				pthread_mutex_lock(&m_MAPA);

				if (*posx_anterior != mensaje->coordenadasDestino->posX) {
					int32_t diferenciaEnX =   *posx_anterior - mensaje->coordenadasDestino->posX;
					if (diferenciaEnX > 0) {
						item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),-1,0);
					} else if (diferenciaEnX < 0) {
						item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),1,0);
					}

				} else if (*posy_anterior != mensaje->coordenadasDestino->posY) {

					int32_t diferenciaEnY =  *posy_anterior - mensaje->coordenadasDestino->posY;
					if (diferenciaEnY > 0) {
						item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),0,-1);
					} else if (diferenciaEnY < 0) {
						item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),0,1);
					}
				}

				nivel_gui_dibujar(mapa);

				pthread_mutex_unlock(&m_MAPA);

			}
		}else{
			pthread_mutex_unlock(&(tabla->m_TABLA));
			pthread_mutex_lock(&m_LOGGER);
				char* log = string_from_format("EL TRIPULANTE %d NO EXISTE\n", mensaje->idTripulante);
				log_info(logger, log);
				free(log);
			pthread_mutex_unlock(&m_LOGGER);

		}

		free(posx_anterior);
		free(posy_anterior);


	}else{
		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
	}

	free(id_patota_string);
}


void cambiar_estado_paginacion(cambio_estado_msg* mensaje, bool* status){
	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	char* id_patota_string = string_itoa(mensaje->idPatota);
	if(dictionary_has_key(tabla_paginas_patota, id_patota_string)){

		char* estado_anterior = malloc(sizeof(char));

		t_tabla_paginas* tabla =  dictionary_get(tabla_paginas_patota, id_patota_string);

		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
		pthread_mutex_lock(&(tabla->m_TABLA));

		bool find_tid(t_direcciones_trips* trip){
			return trip->tid == mensaje->idTripulante;
		}

		t_direcciones_trips* tripulante = list_find(tabla->tabla_direcciones, find_tid);

		if(tripulante != NULL){

			uint32_t pagina = 0;
			uint32_t desplazamiento = 0;

			obtener_direccion_logica_paginacion(&pagina, &desplazamiento, tripulante->direccion_log);

			uint32_t offset_pos = desplazamiento + sizeof(uint32_t);

			uint32_t paginas_mover = (offset_pos / TAMANIO_PAGINA);
			uint32_t desplazamiento_pos = (offset_pos % TAMANIO_PAGINA);

			uint32_t cargar = sizeof(char);
			uint32_t paginas_ne = paginas_necesarias(desplazamiento_pos, cargar);

			bool es_la_pagina (t_pagina_patota* patota){

				return patota->nro_pagina == pagina + paginas_mover;

			}

			t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

			pthread_mutex_lock(&m_MEM_PRINCIPAL);

			char* estado_nuevo = malloc(sizeof(char));

			*estado_nuevo = get_status(mensaje->estado);

			void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

			memcpy(estado_anterior, frame + desplazamiento_pos, cargar);
			memcpy(frame + desplazamiento_pos, estado_nuevo, cargar);

			pthread_mutex_unlock(&m_MEM_PRINCIPAL);

			pthread_mutex_unlock(&(tabla->m_TABLA));

			pthread_mutex_lock(&m_LOGGER);
				log_info(logger, "EL TRIPULANTE %d DE LA PATOTA %d PASO DEL ESTADO %c a %c\n", mensaje->idTripulante,
							mensaje->idPatota, estado_nuevo[0], estado_anterior[0]);
			pthread_mutex_unlock(&m_LOGGER);
			free(estado_nuevo);

		}else{

			pthread_mutex_unlock(&(tabla->m_TABLA));
			pthread_mutex_lock(&m_LOGGER);
				char* log = string_from_format("EL TRIPULANTE %d NO EXISTE\n", mensaje->idTripulante);
				log_info(logger, log);
				free(log);
			pthread_mutex_unlock(&m_LOGGER);

		}

		free(estado_anterior);

	}else{
		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
	}

	free(id_patota_string);

}

char* siguiente_tarea_paginacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status){

	char* tarea = string_new();

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	char* id_patota_string = string_itoa(mensaje->idPatota);
	if(dictionary_has_key(tabla_paginas_patota, id_patota_string)){

		t_tabla_paginas* tabla =  dictionary_get(tabla_paginas_patota, id_patota_string);

		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
		pthread_mutex_lock(&(tabla->m_TABLA));

		uint32_t direccion_tareas = 0;

		uint32_t offset_direccion_tareas = sizeof(uint32_t);

		uint32_t pagina_direccion_tareas = offset_direccion_tareas / TAMANIO_PAGINA;
		uint32_t despl_direccion_tareas = offset_direccion_tareas % TAMANIO_PAGINA;

		uint32_t cargado = 0;
		uint32_t cargar = sizeof(uint32_t);
		uint32_t paginas_ne = paginas_necesarias(offset_direccion_tareas, cargar);

		for(uint32_t i = 0; i < paginas_ne; i++){

			bool es_la_pagina (t_pagina_patota* patota){

				return patota->nro_pagina == i + pagina_direccion_tareas;

			}

			t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

			pthread_mutex_lock(&m_MEM_PRINCIPAL);

			void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

			uint32_t disponible = TAMANIO_PAGINA - despl_direccion_tareas;

			if(disponible < cargar){

				memcpy(&direccion_tareas + cargado, frame + despl_direccion_tareas + cargado, disponible);

				cargado += disponible;
				cargar -= disponible;

				despl_direccion_tareas = 0;

			}else{

				memcpy(&direccion_tareas + cargado, frame + despl_direccion_tareas + cargado, cargar);

			}

			pthread_mutex_unlock(&m_MEM_PRINCIPAL);

		}

		bool find_tid(t_direcciones_trips* trip){
			return trip->tid == mensaje->idTripulante;
		}

		t_direcciones_trips* tripulante = list_find(tabla->tabla_direcciones, find_tid);

		if(tripulante != NULL){


			uint32_t pagina = 0;
			uint32_t desplazamiento = 0;

			obtener_direccion_logica_paginacion(&pagina, &desplazamiento, tripulante->direccion_log);

			uint32_t offset_pos = desplazamiento + (sizeof(uint32_t) * 3) + sizeof(char);

			uint32_t paginas_mover = (offset_pos / TAMANIO_PAGINA);
			uint32_t desplazamiento_pos = (offset_pos % TAMANIO_PAGINA);

			cargado = 0;
			cargar = sizeof(uint32_t);
			paginas_ne = paginas_necesarias(desplazamiento_pos, cargar);

			uint32_t tarea_actual = 0;

			for(uint32_t i = 0; i < paginas_ne; i++){

				bool es_la_pagina (t_pagina_patota* patota){

					return patota->nro_pagina == i + pagina + paginas_mover;

				}

				t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

				pthread_mutex_lock(&m_MEM_PRINCIPAL);

				void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

				uint32_t disponible = TAMANIO_PAGINA - desplazamiento_pos;

				if(disponible < cargar){

					memcpy(&tarea_actual + cargado, frame + desplazamiento_pos + cargado, disponible);

					cargado += disponible;
					cargar -= disponible;

					desplazamiento_pos = 0;

				}else{

					memcpy(&tarea_actual + cargado, frame + desplazamiento_pos + cargado, cargar);

				}

				pthread_mutex_unlock(&m_MEM_PRINCIPAL);

			}

			uint32_t pagina_tareas;
			uint32_t desp_tareas;
			obtener_direccion_logica_paginacion(&pagina_tareas, &desp_tareas, direccion_tareas);
			char* letra = malloc(sizeof(char));

			bool leer_siguiente_pagina = true;
			void* frame;
			void* frame_tareas = malloc(TAMANIO_PAGINA);
			uint32_t i = 0;
			do{

				bool es_la_pagina (t_pagina_patota* patota){

					return patota->nro_pagina == pagina_tareas;

				}

				t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

				if(leer_siguiente_pagina){



					if(pagina_enc == NULL){

						if(strcmp(tarea, "") != 0)
							free(tarea);

						tarea = "";
						break;

					}

					pthread_mutex_lock(&m_MEM_PRINCIPAL);

					frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

					memcpy(frame_tareas, frame, TAMANIO_PAGINA);

					pthread_mutex_unlock(&m_MEM_PRINCIPAL);

				}

				t_pagina_patota* ultima_pagina = list_get(tabla->tabla_paginas, list_size(tabla->tabla_paginas) - 1);

				if(pagina_enc->nro_pagina == ultima_pagina->nro_pagina){

					if(desp_tareas == (pagina_enc->ocupado - 1) && i < tarea_actual){

						if(strcmp(tarea, "") != 0)
							free(tarea);

						tarea = "";
						break;

					}

				}

				memcpy(letra, frame_tareas + desp_tareas, 1);

				desp_tareas++;

				if(desp_tareas == TAMANIO_PAGINA){

					leer_siguiente_pagina = true;
					pagina_tareas++;
					desp_tareas = 0;

				}else{

					leer_siguiente_pagina = false;

				}

				if(letra[0] == '\n' || letra[0] == '\0'){

					if(i < tarea_actual){
						if(strcmp(tarea, "") != 0)
							free(tarea);

						tarea = "";
					}

					i++;

				}else{

					char* string_tarea_nuevo = string_from_format("%s%c", tarea, letra[0]);

					if(strcmp(tarea, "") != 0)
						free(tarea);

					tarea = string_tarea_nuevo;
				}


			}while(i <= tarea_actual);
			free(letra);

			free(frame_tareas);

			if(strcmp(tarea, "") == 0){

				*termino = true;

				pthread_mutex_lock(&m_LOGGER);
					log_info(logger, "EL TRIPULANTE %d DE LA PATOTA %d FINALIZO SUS TAREAS", mensaje->idTripulante, mensaje->idPatota);
				pthread_mutex_unlock(&m_LOGGER);


			} else {

				tarea_actual++;

				desplazamiento_pos = (offset_pos % TAMANIO_PAGINA);

				cargado = 0;
				cargar = sizeof(uint32_t);

				for(uint32_t i = 0; i < paginas_ne; i++){

					bool es_la_pagina (t_pagina_patota* patota){

						return patota->nro_pagina == i + pagina + paginas_mover;

					}

					t_pagina_patota* pagina_enc = list_find(tabla->tabla_paginas, es_la_pagina);

					pthread_mutex_lock(&m_MEM_PRINCIPAL);

					void* frame = leer_de_memoria_principal(pagina_enc, id_patota_string);

					uint32_t disponible = TAMANIO_PAGINA - desplazamiento_pos;

					if(disponible < cargar){

						memcpy(frame + desplazamiento_pos, &tarea_actual + cargado, disponible);

						cargado += disponible;
						cargar -= disponible;

						desplazamiento_pos = 0;

					}else{

						memcpy(frame + desplazamiento_pos, &tarea_actual  + cargado, cargar);

					}

					pthread_mutex_unlock(&m_MEM_PRINCIPAL);

				}

			}

			pthread_mutex_unlock(&(tabla->m_TABLA));

		}else{

			pthread_mutex_unlock(&(tabla->m_TABLA));

			pthread_mutex_lock(&m_LOGGER);
				char* log = string_from_format("EL TRIPULANTE %d NO EXISTE\n", mensaje->idTripulante);
				log_info(logger, log);
				free(log);
			pthread_mutex_unlock(&m_LOGGER);

		}

	}else{
		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
	}

	free(id_patota_string);

	return tarea;

}

//TODO CORREGIR Y AGREGAR LIBERAR PATOTA CUANDO NO HAY MAS TRIPULANTES
void expulsar_tripulante_paginacion(expulsar_tripulante_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	char* id_patota_string = string_itoa(mensaje->idPatota);
	if(dictionary_has_key(tabla_paginas_patota, id_patota_string)){

		t_tabla_paginas* tabla =  dictionary_get(tabla_paginas_patota, id_patota_string);

		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
		pthread_mutex_lock(&(tabla->m_TABLA));

		bool find_tid(t_direcciones_trips* trip){
			return trip->tid == mensaje->idTripulante;
		}

		void expulsar_trip(t_direcciones_trips* tripulante){

			uint32_t pagina = 0;
			uint32_t desplazamiento = 0;

			obtener_direccion_logica_paginacion(&pagina, &desplazamiento, tripulante->direccion_log);

			uint32_t offset = desplazamiento + sizeof(t_tcb);
			uint32_t ultima_pagina = pagina + offset / TAMANIO_PAGINA;
			uint32_t desplazamiento_ult = offset % TAMANIO_PAGINA;

			uint32_t cant_borrar = 0;

			void iterar_tabla (t_pagina_patota* patota){

				if(patota->nro_pagina == pagina){

					patota->ocupado -= TAMANIO_PAGINA - desplazamiento;

				} else if (patota->nro_pagina == ultima_pagina){

					patota->ocupado -= desplazamiento_ult;

				} else if (patota->nro_pagina > pagina && patota->nro_pagina < ultima_pagina){

					patota->ocupado = 0;

				}

				if(patota->ocupado == 0){

					cant_borrar++;

				}

			}

			list_iterate(tabla->tabla_paginas, iterar_tabla);

			for(uint32_t i = 0; i < cant_borrar; i ++){

				void borrar_pagina(t_pagina_patota* patota){

					if(patota->presente){
						liberar_memoria_principal_paginacion(patota);
					}else{
						liberar_memoria_virtual(patota);
					}
					free(patota);

				}

				bool borrar(t_pagina_patota* patota){

					return patota->ocupado == 0;

				}

				list_remove_and_destroy_by_condition(tabla->tabla_paginas, borrar, borrar_pagina);

			}


		}

		list_remove_and_destroy_by_condition(tabla->tabla_direcciones, find_tid, expulsar_trip);

		pthread_mutex_lock(&m_LOGGER);
			log_info(logger, "SE EXPULSO AL TRIPULANTE %d DE LA PATOTA %d\n", mensaje->idTripulante, mensaje->idPatota);
		pthread_mutex_unlock(&m_LOGGER);

		if(list_size(tabla->tabla_direcciones) == 0){

			pthread_mutex_lock(&m_TABLAS_PAGINAS);

			void liberar_tabla(t_tabla_paginas* tabla){

				borrar_patota(tabla);

			}

			dictionary_remove_and_destroy(tabla_paginas_patota, string_itoa(mensaje->idPatota), liberar_tabla);

			pthread_mutex_unlock(&m_TABLAS_PAGINAS);

			pthread_mutex_lock(&m_LOGGER);
				log_info(logger, "SE ELIMINO LA PATOTA %d AL HABER SIDO EXPULSADOS TODOS SUS TRIPULANTES\n", mensaje->idPatota);
			pthread_mutex_unlock(&m_LOGGER);


		} else {

			pthread_mutex_unlock(&(tabla->m_TABLA));

		}
		if(mapa_mostrar){

			pthread_mutex_lock(&m_MAPA);
				item_borrar(mapa, get_tripulante_codigo(mensaje->idTripulante));
				nivel_gui_dibujar(mapa);
			pthread_mutex_unlock(&m_MAPA);

		}


	}else{
		pthread_mutex_unlock(&m_TABLAS_PAGINAS);
	}

	free(id_patota_string);
}


void guardar_en_memoria_swap(t_pagina_patota* pagina, void* datos){

	memcpy(memoria_virtual + pagina->nro_frame * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);

}

void* traer_de_swap(t_pagina_patota* pagina){

	void* datos = malloc(TAMANIO_PAGINA);

	int32_t offset = pagina->nro_frame * TAMANIO_PAGINA;

	memcpy(datos, memoria_virtual +  offset, TAMANIO_PAGINA);

	return datos;

}

void* traer_de_principal(t_pagina_patota* pagina){

	void* datos = malloc(TAMANIO_PAGINA);

	int32_t offset = pagina->nro_frame * TAMANIO_PAGINA;

	memcpy(datos, memoria_principal +  offset, TAMANIO_PAGINA);

	return datos;

}



bool entra_pcb(uint32_t cantidad){
	pthread_mutex_lock(&m_TABLA_LIBRES_P);
	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	bool valor = (list_size(frames_swap) + list_size(frames_libres_principal)) >= cantidad;

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);
	pthread_mutex_unlock(&m_TABLA_LIBRES_P);

	return valor;
}

bool entra_en_memoria(uint32_t size_pcb){
	return TAMANIO_MEMORIA >= size_pcb;
}

bool memoria_llena(){
	pthread_mutex_lock(&m_TABLA_LIBRES_P);

	bool valor = list_size(frames_libres_principal) == 0;

	pthread_mutex_unlock(&m_TABLA_LIBRES_P);

	return valor;
}

bool swap_lleno(){
	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	bool valor = list_size(frames_swap) == 0;

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

	return valor;
}

void* leer_de_memoria_principal(t_pagina_patota* pagina, char* id_patota){

	void* frame;

	pthread_mutex_unlock(&m_MEM_VIRTUAL);
	pthread_mutex_lock(&m_LISTA_REEMPLAZO);

	if(pagina->presente){

		frame = memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA);

		switch(ALGORITMO_REEMPLAZO){
			case LRU:{
				bool encontrado(t_pagina_patota* patota){
					return patota == pagina;
				}

				t_pagina_patota* aux = list_remove_by_condition(lista_para_reemplazo, encontrado);

				list_add(lista_para_reemplazo, aux);
				break;
			}case CLOCK:{break;	}
		}

		pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
		pthread_mutex_unlock(&m_MEM_VIRTUAL);

		pthread_mutex_lock(&m_LOGGER);
		char* mensaje = string_from_format("Patota %s accedio a frame %d sin PF", id_patota, pagina->nro_frame);
		log_info(logger, mensaje);
		free(mensaje);
		pthread_mutex_unlock(&m_LOGGER);

	}else{

		int32_t nro_frame = -1;

		if(!memoria_llena()){

			t_frame* first_fit = list_remove(frames_libres_principal, 0);
			nro_frame = first_fit->pos;
			if(ALGORITMO_REEMPLAZO == CLOCK){
				buffer_clock_pos = nro_frame + 1;
				if(buffer_clock_pos == (TAMANIO_MEMORIA / 32)){
					buffer_clock_pos = 0;
				}
			}
			free(first_fit);

			void* datos_mv =  traer_de_swap(pagina);


			liberar_memoria_virtual(pagina);

			pagina->nro_frame = nro_frame;

			memcpy(memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, datos_mv, TAMANIO_PAGINA);

			free(datos_mv);

			frame = memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA);

			switch(ALGORITMO_REEMPLAZO){
				case LRU:{
					list_add(lista_para_reemplazo, pagina);
					break;
				}case CLOCK:{
					t_buffer_clock* frame_p = list_get(lista_para_reemplazo, pagina->nro_frame);
					frame_p->pagina = pagina;
					pagina->uso = true;
					break;
				}
			}
			pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
			pthread_mutex_unlock(&m_MEM_VIRTUAL);
			pthread_mutex_lock(&m_LOGGER);

			char* mensaje = string_from_format("Pagina %d de la patota %s ingreso en frame %d que estaba libre", pagina->nro_pagina, id_patota, pagina->nro_frame);
			log_info(logger, mensaje);
			free(mensaje);
			mensaje = string_from_format("Patota %s accedio a frame %d con PF sin reemplazo", id_patota, pagina->nro_frame);
			log_info(logger, mensaje);
			free(mensaje);
			pthread_mutex_unlock(&m_LOGGER);

		}else{

			t_pagina_patota* reemplazo = NULL;

			switch(ALGORITMO_REEMPLAZO){

				case LRU: reemplazo = reemplazo_LRU(); break;
				case CLOCK: reemplazo = reemplazo_Clock(); break;

			}


			void* datos_prin = traer_de_principal(reemplazo);
			void* datos_swap = traer_de_swap(pagina);

			nro_frame  = reemplazo->nro_frame;

			reemplazo->nro_frame = pagina->nro_frame;

			if(reemplazo->nuevo)
				reemplazo->nuevo = false;

			pagina->nro_frame = nro_frame;

			guardar_en_memoria_swap(reemplazo, datos_prin);


			memcpy(memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, datos_swap, TAMANIO_PAGINA);

			frame = memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA);

			switch(ALGORITMO_REEMPLAZO){
				case LRU:{
					list_add(lista_para_reemplazo, pagina);
					break;
				}case CLOCK:{
					t_buffer_clock* frame_p = list_get(lista_para_reemplazo, pagina->nro_frame);
					frame_p->pagina = pagina;
					pagina->uso = true;
					break;
				}
			}

			pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
			pthread_mutex_unlock(&m_MEM_VIRTUAL);

			free(datos_prin);
			free(datos_swap);

			pthread_mutex_lock(&m_LOGGER);
			if(nro_frame >= 0){
				char* mensaje = string_from_format("Pagina %d de la patota %s reeemplazo en frame %d", pagina->nro_pagina, id_patota,nro_frame);
				log_info(logger, mensaje);
				free(mensaje);
				mensaje = string_from_format("Patota %s accedio a frame %d con PF con reemplazo", id_patota, nro_frame);
				log_info(logger, mensaje);
				free(mensaje);
			}else{
				char* mensaje = "Fallo reemplazo en memoria principal";
				log_error(logger, mensaje);
				free(mensaje);
			}
			pthread_mutex_unlock(&m_LOGGER);

		}

		pagina->presente = true;

	}

	pagina->uso = true;

	return frame;
}

void guardar_en_memoria_principal(t_pagina_patota* pagina, void* from, char* id_patota){

	pthread_mutex_lock(&m_MEM_PRINCIPAL);
	pthread_mutex_lock(&m_LISTA_REEMPLAZO);
	pthread_mutex_lock(&m_MEM_VIRTUAL);

	if(pagina->presente){

		memcpy( memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA), from, TAMANIO_PAGINA);

		switch(ALGORITMO_REEMPLAZO){
			case LRU:{
				bool encontrado(t_pagina_patota* patota){
					return patota == pagina;
				}

				t_pagina_patota* aux = list_remove_by_condition(lista_para_reemplazo, encontrado);

				list_add(lista_para_reemplazo, aux);
				break;
			}case CLOCK:{break;	}
		}

		pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
		pthread_mutex_unlock(&m_MEM_VIRTUAL);
		pthread_mutex_unlock(&m_MEM_PRINCIPAL);

		pthread_mutex_lock(&m_LOGGER);
		char* mensaje = string_from_format("Patota %s accedio a frame %d sin PF", id_patota, pagina->nro_frame);
		log_info(logger, mensaje);
		free(mensaje);
		pthread_mutex_unlock(&m_LOGGER);

	}else{

		int32_t nro_frame = -1;

		if(!memoria_llena()){

			t_frame* first_fit = list_remove(frames_libres_principal, 0);
			nro_frame = first_fit->pos;
			if(ALGORITMO_REEMPLAZO == CLOCK){
				buffer_clock_pos = nro_frame + 1;
				if(buffer_clock_pos == (TAMANIO_MEMORIA / 32)){
					buffer_clock_pos = 0;
				}
			}
			free(first_fit);

			pagina->nro_frame = nro_frame;

			memcpy( memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA), from, TAMANIO_PAGINA);

			switch(ALGORITMO_REEMPLAZO){
				case LRU:{
					list_add(lista_para_reemplazo, pagina);
					break;
				}case CLOCK:{
					t_buffer_clock* frame_p = list_get(lista_para_reemplazo, pagina->nro_frame);
					frame_p->pagina = pagina;
					pagina->uso = true;
					break;
				}
			}

			pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
			pthread_mutex_unlock(&m_MEM_VIRTUAL);
			pthread_mutex_unlock(&m_MEM_PRINCIPAL);

			pthread_mutex_lock(&m_LOGGER);
			char* mensaje = string_from_format("Pagina %d de la patota %s ingreso en frame %d que estaba libre", pagina->nro_pagina, id_patota, pagina->nro_frame);
			log_info(logger, mensaje);
			free(mensaje);
			mensaje = string_from_format("Patota %s accedio a frame %d con PF sin reemplazo", id_patota, pagina->nro_frame);
			log_info(logger, mensaje);
			free(mensaje);
			pthread_mutex_unlock(&m_LOGGER);

		}else{

			t_pagina_patota* reemplazo = NULL;

			switch(ALGORITMO_REEMPLAZO){

				case LRU: reemplazo = reemplazo_LRU(); break;
				case CLOCK: reemplazo = reemplazo_Clock(); break;

			}
			void* datos = traer_de_principal(reemplazo);

			nro_frame  = reemplazo->nro_frame;

			if(pagina->nuevo){

				int32_t frame_virtual = get_frame_memoria_virtual();

				reemplazo->nro_frame = frame_virtual;

				if(reemplazo->nuevo)
					reemplazo->nuevo = false;

			}else{

				reemplazo->nro_frame = pagina->nro_frame;

				if(reemplazo->nuevo)
					reemplazo->nuevo = false;
			}

			pagina->nro_frame = nro_frame;

			guardar_en_memoria_swap(reemplazo, datos);

			memcpy( memoria_principal + (pagina->nro_frame * TAMANIO_PAGINA), from, TAMANIO_PAGINA);

			switch(ALGORITMO_REEMPLAZO){
				case LRU:{
					list_add(lista_para_reemplazo, pagina);
					break;
				}case CLOCK:{
					t_buffer_clock* frame_p = list_get(lista_para_reemplazo, pagina->nro_frame);
					frame_p->pagina = pagina;
					pagina->uso = true;
					break;
				}
			}

			pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
			pthread_mutex_unlock(&m_MEM_VIRTUAL);
			pthread_mutex_unlock(&m_MEM_PRINCIPAL);


			free(datos);

			pthread_mutex_lock(&m_LOGGER);
			if(nro_frame >= 0){
				char* mensaje = string_from_format("Pagina %d de la patota %s reeemplazo en frame %d", pagina->nro_pagina, id_patota,nro_frame);
				log_info(logger, mensaje);
				free(mensaje);
				mensaje = string_from_format("Patota %s accedio a frame %d con PF con reemplazo", id_patota, pagina->nro_frame);
				log_info(logger, mensaje);
				free(mensaje);
			}else{
				log_error(logger, "Fallo reemplazo en memoria principal");
			}
			pthread_mutex_unlock(&m_LOGGER);

		}

		pagina->presente = true;

	}

	pagina->uso = true;

}

int32_t get_frame_memoria_virtual(){

	int32_t pos = -1;

	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	if(list_size(frames_swap) > 0){

		void destroyer(t_frame* frame){

			pos = frame->pos;

			free(frame);
		}

		list_remove_and_destroy_element(frames_swap, 0, destroyer);

	}

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

	return pos;

}

t_pagina_patota* reemplazo_LRU(){

	t_pagina_patota* pagina_removida = list_remove(lista_para_reemplazo, 0);
	pagina_removida->presente =false;
	pagina_removida->uso= false;

	return pagina_removida;
}

t_pagina_patota*  reemplazo_Clock(){
	t_pagina_patota* pagina_removida = NULL;
	uint32_t buffer_recorrido = buffer_clock_pos;
	uint32_t pos_max = (TAMANIO_MEMORIA / TAMANIO_PAGINA) - 1;
	while(pagina_removida == NULL){

		do{
			t_buffer_clock* buff = list_get(lista_para_reemplazo, buffer_recorrido);

			if(!buff->pagina->uso){
				pagina_removida = buff->pagina;
				buff->pagina = NULL;
			}else{
				buff->pagina->uso = false;
			}
			buffer_recorrido++;
			if(buffer_recorrido > pos_max){
				buffer_recorrido = 0;
			}
		}while(pagina_removida == NULL);

	}

	buffer_clock_pos = buffer_recorrido;

	pagina_removida->presente =false;

	return pagina_removida;
}

int32_t get_esquema_memoria(char* esquema_config) {

	if(strcmp("PAGINACION", esquema_config) == 0){

		return PAGINACION_VIRTUAL;

	} else if(strcmp("SEGMENTACION", esquema_config) == 0){

		return SEGMENTACION_PURA;

	}

	return -1;

}

int32_t get_algoritmo(char* algoritmo_config) {

	if(strcmp("LRU", algoritmo_config) == 0){

		return LRU;

	} else if(strcmp("CLOCK", algoritmo_config) == 0){

		return CLOCK;

	}

	return -1;
}

char get_status(t_status_code codigo){

	switch(codigo){
		case NEW: return 'N';
		case READY: return 'R';
		case EXEC: return 'E';
		case BLOCKED: return 'B';
		default: return 'N';
	}

}

void liberar_memoria_principal_paginacion(t_pagina_patota* pagina){

	pthread_mutex_lock(&m_TABLA_LIBRES_P);
	pthread_mutex_lock(&m_LISTA_REEMPLAZO);

	switch(ALGORITMO_REEMPLAZO){
		case LRU:{
			bool encontrado(t_pagina_patota* patota){
				return patota == pagina;
			}

			list_remove_by_condition(lista_para_reemplazo, encontrado);

			break;
		}
		case CLOCK:{
			t_buffer_clock* buff = list_get(lista_para_reemplazo, pagina->nro_frame);

			buff->pagina = NULL;

			break;
		}
	}

	pthread_mutex_unlock(&m_LISTA_REEMPLAZO);

	t_frame* frame = malloc(sizeof(t_frame));
	frame->pos = pagina->nro_frame;
	list_add(frames_libres_principal, frame);

	bool cmp_frames_libres (t_frame* frame1, t_frame* frame2){
		return frame1->pos < frame2->pos;
	}

	list_sort(frames_libres_principal, cmp_frames_libres);

	pthread_mutex_unlock(&m_TABLA_LIBRES_P);

	pthread_mutex_lock(&m_LOGGER);
		char* mensaje = string_from_format("Liberado frame nro. %d. MEMORIA PRINCIPAL ", frame->pos);
		log_info(logger, mensaje);
		free(mensaje);
	pthread_mutex_unlock(&m_LOGGER);

}

void liberar_memoria_virtual(t_pagina_patota* pagina){

	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	t_frame* frame = malloc(sizeof(t_frame));
	frame->pos = pagina->nro_frame;
	list_add(frames_swap, frame);

	bool cmp_frames_libres (t_frame* frame1, t_frame* frame2){
		return frame1->pos < frame2->pos;
	}

	list_sort(frames_swap, cmp_frames_libres);

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

	pthread_mutex_lock(&m_LOGGER);
		char* mensaje = string_from_format("Liberado frame nro. %d. MEMORIA VIRTUAL ", frame->pos);
		log_info(logger, mensaje);
		free(mensaje);
	pthread_mutex_unlock(&m_LOGGER);

}

void borrar_patota(t_tabla_paginas* tabla){

	pthread_mutex_unlock(&(tabla->m_TABLA));

	pthread_mutex_lock(&(tabla->m_TABLA));

	void liberar_paginas(t_pagina_patota* pagina){

		if(pagina->presente){
			liberar_memoria_principal_paginacion(pagina);
		}else{
			liberar_memoria_virtual(pagina);
		}
		free(pagina);
	}

	list_destroy_and_destroy_elements(tabla->tabla_paginas, liberar_paginas);

	void liberar_direcciones(t_direcciones_trips* direccion){
		free(direccion);
	}

	list_destroy_and_destroy_elements(tabla->tabla_direcciones, liberar_direcciones);

	pthread_mutex_destroy(&(tabla->m_TABLA));

}


char* get_config(int opt){

	char* config_path = string_new();

	switch(opt){
		case 1: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_general.config";break;
		case 2: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_seg1.config";break;
		case 3: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_seg2.config";break;
		case 4: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_seg3_1.config";break;
		case 5: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_seg3_2.config";break;
		case 6: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_pag1_1.config";break;
		case 7: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_pag1_2.config";break;
		case 8: config_path = "/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/configs/ram_seg4.config";break;

		default: config_path = "err"; break;
	}

	return config_path;

}

// SEGMENTACION

// devuelve si el criterio es BF o FF
int32_t get_criterio(char* algoritmo_config) {

	if(strcmp("FF", algoritmo_config) == 0){

		return FF;

	} else if(strcmp("BF", algoritmo_config) == 0){

		return BF;
	}
	return -1;
}

// crea la lista de segmentos libres
void inicializar_segmentacion(){
	segmento* memoria_vacia = malloc(sizeof(segmento));
	segmentos_libres = list_create();
	segmentos_en_memoria = list_create();
	memoria_vacia->inicio = 0;
	memoria_vacia->tamanio = TAMANIO_MEMORIA;
	list_add(segmentos_libres, memoria_vacia);
	tablas_seg_patota = dictionary_create();
}

void terminar_segmentacion(){
	void destroy(void* a){}

	if(segmentos_libres != NULL)
		list_destroy_and_destroy_elements(segmentos_libres, destroy);
	if(segmentos_en_memoria != NULL)
		list_destroy_and_destroy_elements(segmentos_en_memoria, destroy);

	void destroy_dict(tabla_segmentos* elemento){

		list_destroy_and_destroy_elements(elemento->segmentos, destroy);
		pthread_mutex_destroy(&(elemento->m_TABLA));

	}

	if(tablas_seg_patota != NULL)
		dictionary_destroy_and_destroy_elements(tablas_seg_patota, destroy_dict);

	terminar();
}

void dump_segmentacion(FILE* dump){

	//char* titulos = string_from_format("PROCESO\t\tSEGMENTO \t\tINICIO\t\TAMANIO\n");
	char* titulos = string_from_format("\t\tESTADO DE MEMORIA SEGMENTACION\n");
	fputs(titulos, dump);
	free(titulos);

	t_list* tabla_dump = list_create();

	void leer_tablas_segmentos(char* key, tabla_segmentos* tabla){

		t_list* segmentos = tabla->segmentos;

		void copiar_segmentos(segmento* seg){

			segmento_dump* seg_dump = malloc(sizeof(segmento_dump));

			seg_dump->pid = malloc(strlen(key) + 1); //TODO preguntar a santi porq esto
			strcpy(seg_dump->pid, key);
			seg_dump->inicio = seg->inicio;
			seg_dump->numero_segmento = seg->numero_segmento;
			seg_dump->tamanio = seg->tamanio;

			list_add(tabla_dump, seg_dump);
		}
		list_iterate(segmentos, copiar_segmentos);
	}
	dictionary_iterator(tablas_seg_patota, leer_tablas_segmentos);

	bool ordenar_tabla(segmento_dump* seg1, segmento_dump* seg2){
		return seg1->inicio < seg2->inicio;
	}
	list_sort(tabla_dump, ordenar_tabla);

	void guardar_tabla(segmento_dump* seg){

		char* fila = string_from_format("proceso: %s\t\t nro de segmento: %d\t\t inicio: %d\t\t tamanio: %d\n",seg->pid, seg->numero_segmento, seg->inicio, seg->tamanio);
		fputs(fila, dump);
		free(fila);
	}

	list_iterate(tabla_dump, guardar_tabla);

	void vaciar_lista(segmento_dump* seg){ }
	list_destroy_and_destroy_elements(tabla_dump, vaciar_lista);

}

uint32_t obtener_limite(segmento* seg){
	return seg->inicio + seg->tamanio - 1;
}

//le paso el tamanio de un segmento, devuelve 1 si hay espacio y 0 si no hay espacio
bool entra_en_un_seg_libre(uint32_t size){

	bool entra_en_el_segmento(segmento* seg){
		return seg->tamanio > size;
	}

	return list_any_satisfy(segmentos_libres, entra_en_el_segmento);
}

//devuelve un offset de donde arrancar a guardar un segmento en memoria
int32_t get_espacio_libre(uint32_t size){

	t_list* lista_auxiliar = list_create();

	bool entra_en_el_segmento(segmento* seg){
		return seg->tamanio >= size;
	}

	uint32_t criterio_seleccion = CRITERIO_SELECCION;

	list_add_all(lista_auxiliar, list_filter(segmentos_libres, entra_en_el_segmento));

	segmento* primer_seg_libre = list_get(lista_auxiliar, 0);

	if(list_size(lista_auxiliar) == 1){

		free(lista_auxiliar);
		return primer_seg_libre->inicio;

	} else {
		if(criterio_seleccion == FF){

			free(lista_auxiliar);
			return primer_seg_libre->inicio;

		} else {

			uint32_t offset_bf;
			uint32_t dif_tamanio_anterior = TAMANIO_MEMORIA - 1; //por ahora lo pongo asi porque no se como setear el primer anterior si no

			void encontrar_best_fit(segmento* seg){
				uint32_t dif_de_tamanio = seg->tamanio - size;
				if(dif_de_tamanio < dif_tamanio_anterior){
					offset_bf = seg->inicio;
				}
				dif_tamanio_anterior = dif_de_tamanio;
			}
			list_iterate(lista_auxiliar, encontrar_best_fit);

			free(lista_auxiliar);

			return offset_bf;
		}
	}
}

segmento* buscar_segmento_tripulante(uint32_t id_tripulante, uint32_t id_patota){

	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(id_patota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	t_list* tabla_patota = tabla_seg->segmentos;
	segmento* seg_tripulante;
	uint32_t contador = 2;
	uint32_t tid;

	uint32_t encontrado = 0;
	uint32_t offset;
	void* buffer = malloc(sizeof(t_tcb));

	while(!encontrado){
		seg_tripulante = list_get(tabla_patota, contador);
		offset = seg_tripulante->inicio;
		pthread_mutex_lock(&m_MEM_PRINCIPAL);
		memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
		pthread_mutex_unlock(&m_MEM_PRINCIPAL);
		memcpy(&tid, buffer, sizeof(tid));
		if(tid == id_tripulante){
				encontrado = 1;
		}
		contador++;
	}

	free(buffer);
	free(id_patota_str);

	return seg_tripulante;
}

uint32_t buscar_offset_tripulante(uint32_t id_tripulante, uint32_t id_patota){

	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(id_patota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	free(id_patota_str);

	t_list* tabla_patota = tabla_seg->segmentos;
	uint32_t contador = 2;
	uint32_t tid;

	uint32_t encontrado = 0;
	uint32_t offset;
	void* buffer = malloc(sizeof(t_tcb));

	while(!encontrado){
		segmento* seg_tripulante = list_get(tabla_patota, contador);
		offset = seg_tripulante->inicio;
		pthread_mutex_lock(&m_MEM_PRINCIPAL); //asi esta bien este?
		memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
		pthread_mutex_unlock(&m_MEM_PRINCIPAL);
		memcpy(&tid, buffer, sizeof(tid));
		if(tid == id_tripulante){
			encontrado = 1;
		}
		contador++;
	}

	//libero memoria
	free(buffer);

	return offset;
}

void eliminar_patota(t_list* tabla){

	void liberar_segmentos(segmento* seg){ free(seg); }
	list_destroy_and_destroy_elements(tabla, liberar_segmentos);
}

bool entra_en_memoria_seg(uint32_t tamanio_necesario){

	uint32_t espacio_libre_memoria = 0;

	void sumar_tamanios(segmento* seg){
		espacio_libre_memoria += seg->tamanio;
	}

	pthread_mutex_lock(&m_SEGMENTOS_LIBRES);

	list_iterate(segmentos_libres, sumar_tamanios);

	pthread_mutex_unlock(&m_SEGMENTOS_LIBRES);

	return espacio_libre_memoria >= tamanio_necesario;
}

//MENSAJES

// guarda la patota en memoria, crea la tabla de segmentos y la guarda en el dictionary
void crear_patota_segmentacion(iniciar_patota_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(mensaje->idPatota);
	if(!dictionary_has_key(tablas_seg_patota, id_patota_str)){

		//creo la estructura de la tabla de segmentos
		tabla_segmentos* tabla_seg = malloc(sizeof(tabla_segmentos));
		tabla_seg->segmentos = list_create();
		tabla_seg->m_TABLA = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		uint32_t size_tareas = mensaje->tareas->length;

		uint32_t tam_tripulantes = 0;

		for(uint32_t i = 0; i < mensaje->cant_tripulantes; i++){
			tam_tripulantes += sizeof(t_tcb);
		}

		uint32_t tamanio_necesario = sizeof(t_pcb) + size_tareas + tam_tripulantes;
		uint32_t offset;

		if(entra_en_memoria_seg(tamanio_necesario)){

			pthread_mutex_lock(&m_SEGMENTOS_LIBRES);
			pthread_mutex_lock(&m_SEG_EN_MEMORIA);
			pthread_mutex_lock(&m_MEM_PRINCIPAL);

			//Creo los segmentos para la tabla de segmentos

			segmento* seg_pcb = malloc(sizeof(segmento));
			seg_pcb->numero_segmento = 0;
			seg_pcb->tamanio = sizeof(t_pcb);
			list_add(tabla_seg->segmentos, seg_pcb);

			segmento* seg_tareas = malloc(sizeof(segmento));
			seg_tareas->numero_segmento = 1;
			seg_tareas->tamanio = size_tareas;
			list_add(tabla_seg->segmentos, seg_tareas);

			for(uint32_t j = 0; j < mensaje->cant_tripulantes; j++){
				segmento* seg_tcb = malloc(sizeof(segmento));
				seg_tcb->numero_segmento = j+2;
				seg_tcb->tamanio = sizeof(t_tcb);
				list_add(tabla_seg->segmentos, seg_tcb);
			}

			//Cargo datos para copiarlos a memoria

			//pcb
			t_pcb* pcb = malloc(sizeof(t_pcb));
			pcb->pid = mensaje->idPatota;
			pcb->direccion_tareas = 1;

			//tcb
			t_list* tcbs = list_create();
			void cargar_tcb(tripulante_data_msg* tripulante){

				t_tcb* tcb = malloc(sizeof(t_tcb));
				tcb->tid = tripulante->idTripulante;
				tcb->estado = 'R';
				tcb->posX = tripulante->coordenadas->posX;
				tcb->posY = tripulante->coordenadas->posY;
				tcb->proxima_instruccion = 0;
				tcb->direccion_patota = 0;

				list_add(tcbs, tcb);

			}

			list_iterate(mensaje->tripulantes, cargar_tcb);

			//Los copio a memoria

			//pcb
			if(entra_en_un_seg_libre(sizeof(t_pcb))){
				offset = get_espacio_libre(sizeof(t_pcb));
				seg_pcb->inicio = offset;
				memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
				agregar_seg_listas(seg_pcb);
			} else {
				compactar_memoria();
				offset = get_espacio_libre(sizeof(t_pcb));
				seg_pcb->inicio = offset;
				memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
				agregar_seg_listas(seg_pcb);
			}

			//tareas
			if(entra_en_un_seg_libre(size_tareas)){
				offset = get_espacio_libre(size_tareas);
				seg_tareas->inicio = offset;
				memcpy(memoria_principal + offset, mensaje->tareas->string, size_tareas);
				agregar_seg_listas(seg_tareas);
			} else {
				compactar_memoria();
				offset = get_espacio_libre(size_tareas);
				seg_tareas->inicio = offset;
				memcpy(memoria_principal + offset, mensaje->tareas->string, size_tareas);
				agregar_seg_listas(seg_tareas);
			}

			uint32_t orden_seg_tcb = 2;

			//guarda los tcb
			void cargarTcbDatos(t_tcb* tcb){
				segmento* seg_tcb = list_get(tabla_seg->segmentos, orden_seg_tcb);
				if(entra_en_un_seg_libre(sizeof(t_tcb))){
					offset = get_espacio_libre(sizeof(t_tcb));
					seg_tcb->inicio = offset;
					memcpy(memoria_principal + offset, tcb, sizeof(t_tcb));
					agregar_seg_listas(seg_tcb);
					orden_seg_tcb ++;
				} else {
					compactar_memoria();
					offset = get_espacio_libre(sizeof(t_tcb));
					seg_tcb->inicio = offset;
					memcpy(memoria_principal + offset, tcb, sizeof(t_tcb));
					agregar_seg_listas(seg_tcb);
					orden_seg_tcb ++;
				}
			}

			list_iterate(tcbs, cargarTcbDatos);

			void liberar_tcbs(t_tcb* tcb){ free(tcb); }
			list_destroy_and_destroy_elements(tcbs, liberar_tcbs);

			dictionary_put(tablas_seg_patota, id_patota_str, tabla_seg);

			//Libero las estructuras para guardar en memoria
			free(pcb);
			free(id_patota_str);

			pthread_mutex_unlock(&m_SEGMENTOS_LIBRES);
			pthread_mutex_unlock(&m_SEG_EN_MEMORIA);
			pthread_mutex_unlock(&m_MEM_PRINCIPAL);
			pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

			if(mapa_mostrar){

				pthread_mutex_lock(&m_MAPA);

				void cargar_en_mapa(tripulante_data_msg* tripulante){
					personaje_crear(mapa, get_tripulante_codigo(tripulante->idTripulante), tripulante->coordenadas->posX, tripulante->coordenadas->posY);
				}
				list_iterate(mensaje->tripulantes, cargar_en_mapa);

				nivel_gui_dibujar(mapa);

				pthread_mutex_unlock(&m_MAPA);

			}


			pthread_mutex_lock(&m_LOGGER);
			log_info(logger, "Se creo la patota %d de tamanio %d con %d tripulantes", mensaje->idPatota, tamanio_necesario, mensaje->cant_tripulantes);
			pthread_mutex_unlock(&m_LOGGER);

		} else {
			free(tabla_seg); //con esto libero lo de adentro tmb? (segmentos)
			pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);
			*status = false;
		}
	}
}

// modifica las coordenadas de un tripulante en memoria
void informar_movimiento_segmentacion(informar_movimiento_ram_msg* mensaje, bool* status){

	//no me hacia falta esto, lo hice por el lock de la tabla para buscar offset
	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(mensaje->idPatota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	void* buffer = malloc(sizeof(t_tcb));

	pthread_mutex_lock(&(tabla_seg->m_TABLA));
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);
	pthread_mutex_unlock(&(tabla_seg->m_TABLA));

	pthread_mutex_lock(&m_MEM_PRINCIPAL);

	//traigo de memoria el tcb con el offset encontrado
	memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));

	//reemplazo los valores y guardo los valores anteriores para el log
	uint32_t x_anterior;
	uint32_t y_anterior;
	uint32_t offset_buffer = sizeof(uint32_t) + sizeof(char); //id y estado

	memcpy(&x_anterior, buffer + offset_buffer, sizeof(uint32_t));
	memcpy(buffer + offset_buffer, &(mensaje->coordenadasDestino->posX), sizeof(uint32_t));

	offset_buffer += sizeof(uint32_t); //posX

	memcpy(&y_anterior, buffer + offset_buffer, sizeof(uint32_t));
	memcpy(buffer + offset_buffer, &(mensaje->coordenadasDestino->posY), sizeof(uint32_t));


	//lo copio en memoria modificado
	memcpy(memoria_principal + offset, buffer, sizeof(t_tcb));

	pthread_mutex_lock(&m_LOGGER);
	log_info(logger, "Se modifico la posicion del tripulante %d de la patota %d a --> %d en X y %d en Y", mensaje->idTripulante, mensaje->idPatota, mensaje->coordenadasDestino->posX, mensaje->coordenadasDestino->posY);
	log_info(logger, "La posicion anterior era %d en X y %d en Y", x_anterior, y_anterior);
	pthread_mutex_unlock(&m_LOGGER);

	pthread_mutex_unlock(&m_MEM_PRINCIPAL);

	//gestiono el mapa
	uint32_t x_nuevo = mensaje->coordenadasDestino->posX;
	uint32_t y_nuevo = mensaje->coordenadasDestino->posY;

	if(mapa_mostrar){

		pthread_mutex_lock(&m_MAPA);

		if (x_anterior != x_nuevo) {
			int32_t diferenciaEnX =   x_anterior - x_nuevo;
			if (diferenciaEnX > 0) {
				item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),-1,0);
			} else if (diferenciaEnX < 0) {
				item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),1,0);
			}

		} else if (y_anterior != y_nuevo) {

			int32_t diferenciaEnY =  y_anterior - y_nuevo;
			if (diferenciaEnY > 0) {
				item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),0,-1);
			} else if (diferenciaEnY < 0) {
				item_desplazar(mapa,get_tripulante_codigo(mensaje->idTripulante),0,1);
			}
		}

		nivel_gui_dibujar(mapa);

		pthread_mutex_unlock(&m_MAPA);

	}


	//libero memoria
	free(id_patota_str);
	free(buffer);
}

// modifica el estado de un tripulante en memoria
void cambiar_estado_segmentacion(cambio_estado_msg* mensaje, bool* status){

	//no me hacia falta esto, lo hice por el lock de la tabla para buscar offset
	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(mensaje->idPatota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	void* buffer = malloc(sizeof(t_tcb));

	pthread_mutex_lock(&(tabla_seg->m_TABLA));
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);
	pthread_mutex_unlock(&(tabla_seg->m_TABLA));

	char estado = get_status(mensaje->estado);
	char estado_anterior;

	pthread_mutex_lock(&m_MEM_PRINCIPAL);
	//traigo de memoria el tcb con el offset encontrado
	memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));

	uint32_t offset_buffer = sizeof(uint32_t); //id

	//copio el estado anterior para el log
	memcpy(&estado_anterior, buffer + offset_buffer, sizeof(char));

	//reemplazo el estado
	memcpy(buffer + offset_buffer, &estado, sizeof(char));

	//lo copio en memoria modificado
	memcpy(memoria_principal + offset, buffer, sizeof(t_tcb));

	pthread_mutex_unlock(&m_MEM_PRINCIPAL);

	pthread_mutex_lock(&m_LOGGER);
	log_info(logger, "Se modifico el estado del tripulante %d de la patota %d de %c a %c", mensaje->idTripulante, mensaje->idPatota, estado_anterior, estado);
	pthread_mutex_unlock(&m_LOGGER);

	//libero memoria
	free(id_patota_str);
	free(buffer);
}

// devuelve la siguiente tarea de un tripulante
char* siguiente_tarea_segmentacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status){

	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(mensaje->idPatota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	pthread_mutex_lock(&(tabla_seg->m_TABLA));
	t_list* tabla_patota = tabla_seg->segmentos;
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);
	uint32_t proxima_instruccion;

	void* buffer_tcb = malloc(sizeof(t_tcb));

	pthread_mutex_lock(&m_MEM_PRINCIPAL);
	memcpy(buffer_tcb, memoria_principal + offset, sizeof(t_tcb));

	//traigo el numero de la siguiente instruccion de memoria
	uint32_t offset_tcb = 3 * sizeof(uint32_t) + sizeof(char); //id, estado, posx, posy
	memcpy(&proxima_instruccion, buffer_tcb + offset_tcb, sizeof(uint32_t));

	segmento* seg_tareas = list_get(tabla_patota, 1);

    char* tareas = malloc(seg_tareas->tamanio);
    memcpy(tareas, memoria_principal + seg_tareas->inicio, seg_tareas->tamanio);

    char* tarea = string_new();

	char** array_tareas = string_split(tareas, "\n");

    if(array_tareas[proxima_instruccion] == NULL){
    	*termino = true;
    } else {
    	string_append(&tarea, array_tareas[proxima_instruccion]);
    }

    //free cada elemento, free array
    uint32_t contador = 0;
    while(array_tareas[contador] != NULL){
        free(array_tareas[contador]);
        contador++;
    }
    free(array_tareas);
    free(tareas);

	//le sumo uno en memoria a la proxima instruccion
	proxima_instruccion += 1;
	//copio el nuevo valor en el buffer

	memcpy(buffer_tcb + offset_tcb, &proxima_instruccion, sizeof(uint32_t));
	//lo paso a la memoria
	memcpy(memoria_principal + offset, buffer_tcb, sizeof(t_tcb));

	pthread_mutex_unlock(&m_MEM_PRINCIPAL);
	pthread_mutex_unlock(&(tabla_seg->m_TABLA));

	free(buffer_tcb);
	free(id_patota_str);

	if(!(*termino)){
		pthread_mutex_lock(&m_LOGGER);
		log_info(logger, "Al tripulante %d de la patota %d le tocaba la tarea numero %d: %s", mensaje->idTripulante, mensaje->idPatota, proxima_instruccion-1, tarea);
		pthread_mutex_unlock(&m_LOGGER);
	}

	return tarea;
}

//saca el segmento de la tabla de segmentos y agrega el segmento libre a la lista de libres
void expulsar_tripulante_segmentacion(expulsar_tripulante_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_SEGMENTOS);
	char* id_patota_str = string_itoa(mensaje->idPatota);
	tabla_segmentos* tabla_seg = dictionary_get(tablas_seg_patota, id_patota_str);
	pthread_mutex_unlock(&m_TABLAS_SEGMENTOS);

	pthread_mutex_lock(&(tabla_seg->m_TABLA));
	t_list* tabla_patota = tabla_seg->segmentos;

	//busco el segmento
	segmento* seg_tripulante = buscar_segmento_tripulante(mensaje->idTripulante, mensaje->idPatota);

	uint32_t index = 0;
	uint32_t contador = 0;

	void index_del_segmento(segmento* seg){
		if(seg->inicio == seg_tripulante->inicio){
			index = contador;
		}
		contador++;
	}

	list_iterate(tabla_patota, index_del_segmento);

	pthread_mutex_lock(&m_SEGMENTOS_LIBRES);
	pthread_mutex_lock(&m_SEG_EN_MEMORIA);

	// saco el segmento de las tablas globales
	liberar_segmento(seg_tripulante);

	// saco el segmento de la tabla de segmentos de la patota
	void liberar_seg(segmento* seg){
		free(seg);
	}
	list_remove_and_destroy_element(tabla_patota, index, liberar_seg);

	pthread_mutex_unlock(&m_SEGMENTOS_LIBRES);
	pthread_mutex_unlock(&m_SEG_EN_MEMORIA);

	if(mapa_mostrar){

		pthread_mutex_lock(&m_MAPA);

		item_borrar(mapa, get_tripulante_codigo(mensaje->idTripulante));

		nivel_gui_dibujar(mapa);

		pthread_mutex_unlock(&m_MAPA);

	}

	pthread_mutex_lock(&m_LOGGER);
	log_info(logger, "Se expulso al tripulante %d de la patota %d", mensaje->idTripulante, mensaje->idPatota);
	pthread_mutex_unlock(&m_LOGGER);

	if(list_size(tabla_patota) == 2){ //osea, es el ultimo tripulante

		pthread_mutex_lock(&m_SEGMENTOS_LIBRES);
		pthread_mutex_lock(&m_SEG_EN_MEMORIA);

		list_iterate(tabla_patota, liberar_segmento); // aca libera el segmento en las tablas
		eliminar_patota(tabla_patota);

		pthread_mutex_unlock(&m_SEGMENTOS_LIBRES);
		pthread_mutex_unlock(&m_SEG_EN_MEMORIA);

		pthread_mutex_lock(&m_LOGGER);
		log_info(logger, "El tripulante %d era el ultimo, se elimino toda la patota %d", mensaje->idTripulante, mensaje->idPatota);
		pthread_mutex_unlock(&m_LOGGER);

		dictionary_remove(tablas_seg_patota, id_patota_str);
		pthread_mutex_destroy(&(tabla_seg->m_TABLA));
		free(tabla_seg);

	} else {
	pthread_mutex_unlock(&(tabla_seg->m_TABLA));
	}

	free(id_patota_str);
}

//saca un segmento de la lista libres, si sobraba segmento guarda el sobrante, falta revision y free
void agregar_seg_listas(segmento* segmento_nuevo){

	uint32_t inicio_seg_nuevo = segmento_nuevo->inicio;
	uint32_t limite_seg_nuevo = obtener_limite(segmento_nuevo);

	bool se_encuentra_contenido(segmento* seg){
		uint32_t limite_segmento = obtener_limite(seg);
		return seg->inicio == inicio_seg_nuevo;
	}

	segmento* seg_a_modificar = list_get(list_filter(segmentos_libres, se_encuentra_contenido), 0);

	bool es_el_segmento(segmento* seg){
		return seg->inicio == seg_a_modificar->inicio && seg->tamanio == seg_a_modificar->tamanio;
	}

	list_remove_by_condition(segmentos_libres, es_el_segmento);

	uint32_t limite_seg_libre = obtener_limite(seg_a_modificar);

	if(limite_seg_libre != limite_seg_nuevo){

		seg_a_modificar->inicio = limite_seg_nuevo+1; //el segmento libre nuevo arranca donde termina el otro
		seg_a_modificar->tamanio = limite_seg_libre - limite_seg_nuevo;
		list_add(segmentos_libres, seg_a_modificar);

		ordenar_lista_segmentos_libres();

	}

	list_add(segmentos_en_memoria, segmento_nuevo);
} //no lleva semaforos, porque donde la llamo ya estan los semaforos

void ordenar_lista_segmentos_libres(){

	bool ordenar_segmentos(segmento* primer_segmento, segmento* segundo_segmento){
		return primer_segmento->inicio < segundo_segmento->inicio;
	}

	list_sort(segmentos_libres, ordenar_segmentos);

} //no lleva semaforos, porque donde la llamo ya estan los semaforos

//agrega el segmento a la lista de segmentos libres y lo saca de segmentos en memoria
void liberar_segmento(segmento* seg_viejo){

	bool esta_el_segmento(segmento *seg_list){
		return seg_viejo->inicio == seg_list->inicio;
	}

	segmento* seg = malloc(sizeof(segmento));
	seg->inicio = seg_viejo->inicio;
	seg->tamanio = seg_viejo->tamanio;

	if(list_any_satisfy(segmentos_en_memoria, esta_el_segmento)){

		// saco el segmento de la lista de segmentos en memoria
		uint32_t index = 0;
		uint32_t contador = 0;

		void index_segmento(segmento* segmento){
			if(seg->inicio == segmento->inicio){
				index = contador;
			}
			contador++;
		}

		list_iterate(segmentos_en_memoria, index_segmento);

		list_remove(segmentos_en_memoria, index);

		index = 0;
		contador = 0;

		///////

		//agregO a la lista de segmentos libres
		list_add(segmentos_libres, seg);
		ordenar_lista_segmentos_libres();

		//busca el index del segmento que acaba de agregar
		list_iterate(segmentos_libres, index_segmento);

		if(index != list_size(segmentos_libres)-1){
			segmento* seg_siguiente = list_get(segmentos_libres, index+1);

			//me fijo si lo puedo compactar con el hueco siguiente
			uint32_t limite_seg = obtener_limite(seg);
			if(seg_siguiente->inicio == limite_seg + 1){
				list_remove(segmentos_libres, index + 1);
				list_remove(segmentos_libres, index);
				seg->tamanio = seg->tamanio + seg_siguiente->tamanio;
				free(seg_siguiente);
				list_add(segmentos_libres, seg);
				ordenar_lista_segmentos_libres();
			}
		}

		if(index != 0){
			segmento* seg_anterior = list_get(segmentos_libres, index - 1);

			//me fijo si lo puedo compactar con el hueco siguiente
			uint32_t limite_seg_anterior = obtener_limite(seg_anterior);
			if(seg->inicio == limite_seg_anterior + 1){
				list_remove(segmentos_libres, index);
				list_remove(segmentos_libres, index - 1);
				seg->inicio = seg_anterior->inicio;
				seg->tamanio = seg->tamanio + seg_anterior->tamanio;
				free(seg_anterior);
				list_add(segmentos_libres, seg);
				ordenar_lista_segmentos_libres();
			}
		}

	}
}//no lleva semaforos, porque donde la llamo ya estan los semaforos

//junta todos los segmentos en la parte superior de la memoria y crea un segmento libre con el restante
void compactar_memoria(){

	bool ordenar_segmentos(segmento* seg1, segmento* seg2){
		return seg1->inicio < seg2->inicio;
	}

	void* buffer = malloc(TAMANIO_MEMORIA);
	uint32_t offset = 0;

	list_sort(segmentos_en_memoria, ordenar_segmentos);

	segmento* seg_anterior = list_get(segmentos_en_memoria, 0);
	uint32_t limite_seg_anterior;

	//copio en el buffer el primer segmento al principio (en 0)
	memcpy(buffer, memoria_principal + seg_anterior->inicio, seg_anterior->tamanio);

	if(seg_anterior->inicio != 0){
		seg_anterior->inicio = 0;
	}

	limite_seg_anterior = obtener_limite(seg_anterior);
	offset = limite_seg_anterior + 1;

	void modificar_inicio(segmento *seg){
		if(seg->inicio != 0){

			//transcribo el segmento al buffer
			memcpy(buffer + offset, memoria_principal + seg->inicio, seg->tamanio);

			if(offset != seg->inicio){
				seg->inicio = offset;
			}

			seg_anterior = seg;
			limite_seg_anterior = obtener_limite(seg);
			offset = limite_seg_anterior + 1;
		}
	}

	list_iterate(segmentos_en_memoria, modificar_inicio);

	uint32_t cant_segmentos = list_size(segmentos_en_memoria);
	segmento* ult_seg = list_get(segmentos_en_memoria, cant_segmentos - 1);
	uint32_t tamanio_ocupado_en_memoria = obtener_limite(ult_seg) + 1;

	segmento* segmento_libre = malloc(sizeof(segmento));
	segmento_libre->inicio = tamanio_ocupado_en_memoria;
	segmento_libre->tamanio = TAMANIO_MEMORIA - tamanio_ocupado_en_memoria;

	list_clean(segmentos_libres);
	list_add(segmentos_libres, segmento_libre);

	memcpy(memoria_principal, buffer, TAMANIO_MEMORIA);

	pthread_mutex_lock(&m_LOGGER);
	log_info(logger, "Se compacta memoria");
	pthread_mutex_unlock(&m_LOGGER);

	free(buffer);
} //no lleva semaforos porq tiene en el llamado a la funcion

void print_huecos_libres(){

	printf("HUECOS LIBRES");
	for(int32_t i = 0; i<list_size(segmentos_libres); i++){
		segmento* seg = list_get(segmentos_libres, i);
		printf("Inicio: %d, tamanio: %d, limite: %d\n", seg->inicio, seg->tamanio, obtener_limite(seg));
	}

	printf("SEGMENTOS EN MEMORIA");
	for(int32_t i = 0; i<list_size(segmentos_en_memoria); i++){
		segmento* seg = list_get(segmentos_en_memoria, i);
		printf("Inicio: %d, tamanio: %d, limite: %d\n", seg->inicio, seg->tamanio, obtener_limite(seg));
	}

}

