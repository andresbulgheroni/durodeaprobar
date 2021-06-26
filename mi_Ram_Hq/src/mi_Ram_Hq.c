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

//TODO CASOS DE PRUEBA

///AVERIGUAR SI HACE FALTA UN SEMAFORO POR TABLA O SI ESTA BIEN ASI
pthread_mutex_t  m_MEM_PRINCIPAL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_MEM_VIRTUAL = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLAS_PAGINAS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLA_LIBRES_P = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_TABLA_LIBRES_V = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  m_LISTA_REEMPLAZO = PTHREAD_MUTEX_INITIALIZER;


NIVEL* mapa;
/*
void prueba_msg (uint32_t codigo, void* mensaje_v){

	switch(codigo){
		case INICIAR_PATOTA_MSG:{

			iniciar_patota_msg* mensaje = mensaje_v;
			bool status = true;

			switch(ESQUEMA_MEMORIA){
				//case SEGMENTACION_PURA: crear_patota_segmentacion(mensaje, &status);break;
				case PAGINACION_VIRTUAL: crear_patota_paginacion(mensaje, &status);break;
			}


			if(status){
				log_info(logger, "CREADO CORRECTAMENTE PATOTA %d", mensaje->idPatota);
			}else{
				log_error(logger, "Fallo la insercion de la patota");
			}

			free(mensaje->tareas);
			free(mensaje);

			break;

		}
		case INFORMAR_MOVIMIENTO_RAM:{

			informar_movimiento_ram_msg* mensaje = mensaje_v;

			switch(ESQUEMA_MEMORIA){
				//case SEGMENTACION_PURA: informar_movimiento_segmentacion(mensaje, NULL);break;
				case PAGINACION_VIRTUAL: informar_movimiento_paginacion(mensaje, NULL);break;
			}

			free(mensaje->coordenadasDestino);
			free(mensaje);

			break;

		}
		case CAMBIO_ESTADO:{

			cambio_estado_msg* mensaje = mensaje_v;

			switch(ESQUEMA_MEMORIA){
				//case SEGMENTACION_PURA: cambiar_estado_segmentacion(mensaje, NULL);break;
				case PAGINACION_VIRTUAL: cambiar_estado_paginacion(mensaje, NULL);break;
			}

			free(mensaje);

			break;

		}
		case SOLICITAR_SIGUIENTE_TAREA:{

			solicitar_siguiente_tarea_msg* mensaje = mensaje_v;

			bool completo_tareas = false;
			char* tarea;
			switch(ESQUEMA_MEMORIA){
				//case SEGMENTACION_PURA: tarea = siguiente_tarea_segmentacion(mensaje, &completo_tareas, NULL);break;
				case PAGINACION_VIRTUAL: tarea = siguiente_tarea_paginacion(mensaje, &completo_tareas, NULL);break;
			}

			if(completo_tareas){

				log_error(logger, "COMPLETO TAREAS");

			}else{
				log_info(logger, tarea);
			}

			free(mensaje);

			break;
		}
		case EXPULSAR_TRIPULANTE_MSG:{

			expulsar_tripulante_msg* mensaje = mensaje_v;

			switch(ESQUEMA_MEMORIA){
				//case SEGMENTACION_PURA: expulsar_tripulante_segmentacion(mensaje, NULL);break;
				case PAGINACION_VIRTUAL: expulsar_tripulante_paginacion(mensaje, NULL);break;
			}

			free(mensaje);

			break;

		}
	}
}*/

void sig_handler(int n){

	switch(n){
		case SIGUSR1:{


			char* time_stamp_text = get_timestamp();
			char* path = string_from_format("/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/dump/Dump_%s.dmp", temporal_get_string_time());
			char* inicio_texto = string_from_format("Dump: %s\n", time_stamp_text);

			FILE* dump = fopen(path, "w+");

			fputs(inicio_texto, dump);

			switch(ESQUEMA_MEMORIA){
				case PAGINACION_VIRTUAL: dump_paginacion(dump); break;
				case SEGMENTACION_PURA: break;
			}

			fclose(dump);
			break;
		}
		default: break;
	}

}

int main(void) {

	//TODO PREPARAR PARA USAR DISTINTOS CONFIG PARA LAS  PRUEBAS FINALES

	init();

	log_info(logger,"MIRAMHQ PID: %d ", getpid());

	signal(SIGUSR1, sig_handler);

	pthread_t hilo_server;
	pthread_create(&hilo_server,NULL,(void*)hilo_servidor, NULL);
	pthread_join(hilo_server, NULL);

	terminar();

	return EXIT_SUCCESS;
}

char* get_timestamp(){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	return string_from_format("%d/%02d/%02d %02d:%02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

}

void dump_paginacion(FILE* dump){
	//TODO PREGUNTAR SI HACE FALTAN LOS SEMAFOROS, COMO SE TRATA DE UNA INTERRUPCION

	char* titulos = string_from_format("MARCO\t\tESTADO \t\tPROCESO\t\tPAGINA\n");
	fputs(titulos, dump);

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

	void leer_paginas(char* key, t_list* tabla){

		for(uint32_t i = 0; i < list_size(tabla); i++){

			t_pagina_patota* pagina = list_get(tabla, i);

			if(pagina->presente){

				t_dump_pag* pag = malloc(sizeof(t_dump_pag));

				pag->marco = pagina->nro_frame;
				pag->estado = "OCUPADO";
				strcpy(pag->proceso, key);
				pag->pagina = string_itoa(i);

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

		char* fila = string_from_format("%10d\t\t%15s\t\t%20s\t\t%20s\n", pag->marco, pag->estado, pag->proceso, pag->pagina);
		fputs(fila, dump);

	}

	list_iterate(tabla_dump, guardar_tabla);

	void vaciar_lista(void* pag){
		free(pag);
	}
	list_destroy_and_destroy_elements(tabla_dump, vaciar_lista);
}


void hilo_servidor(){

	int32_t socket_servidor = iniciar_servidor(IP, PUERTO);

	while(true){

		int32_t socket_cliente = esperar_cliente(socket_servidor);
		pthread_t hilo_mensaje;
		pthread_create(&hilo_mensaje,NULL,(void*)recibir_mensaje, (void*) (&socket_cliente));
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
				bool status = true;

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: crear_patota_segmentacion(mensaje, &status);break;
					case PAGINACION_VIRTUAL: crear_patota_paginacion(mensaje, &status);break;
				}

				if(status){
					enviar_paquete(NULL, OK, *conexion);
				}else{
					t_string* rta_error = get_t_string("Fallo la insercion de la patota");

					enviar_paquete(rta_error, FAIL, *conexion);

					free(rta_error);
				}

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
				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: tarea = siguiente_tarea_segmentacion(mensaje, &completo_tareas, NULL);break;
					case PAGINACION_VIRTUAL: tarea = siguiente_tarea_paginacion(mensaje, &completo_tareas, NULL);break;
				}

				if(completo_tareas){

					enviar_paquete(NULL, COMPLETO_TAREAS, *conexion);

				}else{
					t_string* tarea_msg = get_t_string(tarea);
					enviar_paquete(tarea_msg, SOLICITAR_SIGUIENTE_TAREA_RTA, *conexion);
					free(tarea_msg);
				}

				free(mensaje);

				break;
			}
			case EXPULSAR_TRIPULANTE_MSG:{

				expulsar_tripulante_msg* mensaje = deserializar_paquete(paquete);

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: expulsar_tripulante_segmentacion(mensaje, NULL);break;
					case PAGINACION_VIRTUAL: expulsar_tripulante_paginacion(mensaje, NULL);break;
				}

				free(mensaje);

				break;

			}
			default: terminado = true; break;
		}
	}

	pthread_exit(NULL);
}

void init (){

	config = config_create("/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/ram.config");
	logger = log_create("/home/utnso/tp-2021-1c-DuroDeAprobar/mi_Ram_Hq/ram.log", "MI-RAM-HQ", true, LOG_LEVEL_DEBUG);

	TAMANIO_MEMORIA = config_get_int_value(config, "TAMANIO_MEMORIA");
	ESQUEMA_MEMORIA = get_esquema_memoria(config_get_string_value(config, "ESQUEMA_MEMORIA"));
	TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
	TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	ALGORITMO_REEMPLAZO = get_algoritmo(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));;
	IP = config_get_string_value(config, "IP");
	CRITERIO_SELECCION = get_criterio(config_get_string_value(config, "CRITERIO_SELECCION")); // TODO CELES TE FALTA DECLAR LA FUNCION EN EL .H CREO
	PUERTO = config_get_string_value(config, "PUERTO");

	memoria_principal = malloc(TAMANIO_MEMORIA);

	switch(ESQUEMA_MEMORIA){
		case SEGMENTACION_PURA:{

		//	inicializar_segmentacion();

			break;
		}
		case PAGINACION_VIRTUAL:{

			configurar_paginacion();

			break;
		}
	}
	//iniciarMapa();

}

void configurar_paginacion(){


	// TODO Probar archivo con path a la carpeta /home/utnso/tp-2021-1c-DuroDeAprobar/mi Ram Hq/
	if(TAMANIO_MEMORIA%TAMANIO_PAGINA != 0 || TAMANIO_SWAP%TAMANIO_PAGINA != 0){
		log_error(logger, "Mal configurados tamanios de memoria y pagina");
	}

	int32_t fileDes = open(PATH_SWAP, O_RDWR | O_APPEND | O_CREAT, 0777);

	if(fileDes == -1){
		log_error(logger, "No se pudo abrir el archivo");
		close(fileDes);
	}

	if(fileDes >= 0){

		lseek(fileDes, TAMANIO_SWAP - 1, SEEK_SET);

		write(fileDes, "", 1);

		memoria_virtual =  mmap(0, TAMANIO_SWAP, PROT_WRITE | PROT_READ, MAP_SHARED, fileDes, 0);

		if(memoria_virtual == MAP_FAILED){
			log_error(logger, "Fallo en el mapeo de archivo SWAP");
			close(fileDes);
		}

		frames_swap = list_create();
		frames_libres_principal = list_create();
		lista_para_reemplazo = list_create();

		for(uint32_t i = 0; i < (TAMANIO_SWAP / TAMANIO_PAGINA);i++){
			t_frame* frame = malloc(sizeof(t_frame));
			frame->pos = i;
			list_add(frames_swap, frame);
		}

		for(uint32_t i = 0; i < (TAMANIO_MEMORIA / TAMANIO_PAGINA); i++){
			t_frame_libre* nro_frame = malloc(sizeof(t_frame_libre));
			nro_frame->pos = i;
			list_add(frames_libres_principal, nro_frame);
		}

		if(ALGORITMO_REEMPLAZO == CLOCK){
			buffer_clock_pos = 0;
			for(uint32_t i = 0; i < (TAMANIO_MEMORIA / TAMANIO_PAGINA); i++){
				t_buffer_clock* frame = malloc(sizeof(t_frame));
				frame->pagina = NULL;
				list_add(lista_para_reemplazo, frame);
			}
		}

		tabla_paginas_patota = dictionary_create();

	}

}

void iniciarMapa(){

	//int columnas, filas;

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

void leer_pagina_de_memoria(t_pagina_patota* pagina, void* to){

	if(!pagina->presente)
		pasar_de_swap_a_principal(pagina);

	memcpy(to, memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, TAMANIO_PAGINA);

}


void crear_patota_paginacion(iniciar_patota_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	if(!dictionary_has_key(tabla_paginas_patota, string_itoa(mensaje->idPatota))){

		//Saco cantidad de frames/paginas que va a ocupar
		uint32_t cantidad_frames = ceil((sizeof(t_pcb) + mensaje->cant_tripulantes * (sizeof(t_tcb))
				+  mensaje->tareas->length) / TAMANIO_PAGINA);
		uint32_t size_pcb = cantidad_frames * TAMANIO_PAGINA;

		*status = entra_en_swap(cantidad_frames);

		if(status){

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

			pcb->direccion_tareas = direccion_tareas;//generar_direccion_logica_paginacion(direccion_tareas / TAMANIO_PAGINA, direccion_tareas % TAMANIO_PAGINA);

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
			for(int32_t i = 0; i < cantidad_frames; i++){

				t_pagina_patota* pagina = malloc(sizeof(t_pagina_patota));
				pagina->nro_frame = -1;
				pagina->nro_frame_mv = get_frame_memoria_virtual();
				pagina->presente = false;
				pagina->uso = false;
				pagina->modificado = false;

				guardar_en_memoria_swap(pagina, datos + i * TAMANIO_PAGINA);

				list_add(paginas, pagina);

			}

			free(datos);

			//Guardo tabla de paginas
			dictionary_put(tabla_paginas_patota, string_itoa(mensaje->idPatota), paginas);

		}

	}

	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

}

int32_t paginas_necesarias(uint32_t offset, uint32_t size){

	int32_t pagina_inicial = floor(offset/TAMANIO_PAGINA);

	uint32_t nuevo_offset = offset + size;

	int32_t pagina_final = floor(nuevo_offset/TAMANIO_PAGINA) - (nuevo_offset % TAMANIO_PAGINA == 0 ? 1 : 0);

	return pagina_final - pagina_inicial + 1;

}

void informar_movimiento_paginacion(informar_movimiento_ram_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));
	t_list* paginas_leidas = list_create();
	void* datos = malloc(list_size(paginas) * TAMANIO_PAGINA);
	uint32_t offset = sizeof(t_pcb);
	uint32_t pagina = floor(offset/TAMANIO_PAGINA);
	bool encontrado = false;
	while(!encontrado){
		uint32_t tid;
		int32_t paginas_ne = paginas_necesarias(offset, sizeof(tid));

		for(uint32_t i = 0; i < paginas_ne; i++){

			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina + i);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina + i);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
				list_add(paginas_leidas, patota);

			}

		}

		memcpy(&tid, datos + offset, sizeof(tid));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			offset += sizeof(uint32_t) + sizeof(char);
			pagina = floor(offset/TAMANIO_PAGINA);

			int32_t paginas_ne = paginas_necesarias(offset, sizeof(uint32_t));

			for(uint32_t i = 0; i < paginas_ne; i++){

				bool ya_leida(t_pagina_patota* patota){

					return patota == list_get(paginas, pagina + i);

				}

				if(!list_any_satisfy(paginas_leidas, ya_leida)){

					t_pagina_patota* patota = list_get(paginas, pagina + i);
					leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
					list_add(paginas_leidas, patota);

				}

			}

			memcpy(datos + offset, mensaje->coordenadasDestino->posX, sizeof(uint32_t));
			for(uint32_t i = 0; i < paginas_ne; i++){

				modificar_en_memoria_principal(list_get(paginas, pagina + i), datos + (pagina + i) * TAMANIO_PAGINA);

			}

			offset += sizeof(uint32_t);
			pagina = floor(offset/TAMANIO_PAGINA);

			paginas_ne = paginas_necesarias(offset, sizeof(uint32_t));

			for(uint32_t i = 0; i < paginas_ne; i++){

				bool ya_leida(t_pagina_patota* patota){

					return patota == list_get(paginas, pagina + i);

				}

				if(!list_any_satisfy(paginas_leidas, ya_leida)){

					t_pagina_patota* patota = list_get(paginas, pagina + i);
					leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
					list_add(paginas_leidas, patota);

				}

			}

			memcpy(datos + offset, &(mensaje->coordenadasDestino->posX), sizeof(uint32_t));
			for(uint32_t i = 0; i < paginas_ne; i++){

				modificar_en_memoria_principal(list_get(paginas, pagina + i), datos + (pagina + i) * TAMANIO_PAGINA);

			}

		}else{

			offset += sizeof(t_tcb);
			pagina = floor(offset/TAMANIO_PAGINA);

		}

	}

	void liberar_nros_usadas(int32_t* frame){
		free(frame);
	}
	list_destroy_and_destroy_elements(paginas_leidas, liberar_nros_usadas);
	free(datos);

	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

}

void cambiar_estado_paginacion(cambio_estado_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	t_list* paginas_leidas = list_create();
	void* datos = malloc(list_size(paginas) * TAMANIO_PAGINA);
	uint32_t offset = sizeof(t_pcb);
	uint32_t pagina = floor(offset/TAMANIO_PAGINA);
	bool encontrado = false;
	while(!encontrado){
		uint32_t tid;
		int32_t paginas_ne = paginas_necesarias(offset, sizeof(tid));

		for(uint32_t i = 0; i < paginas_ne; i++){


			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina + i);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina + i);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
				list_add(paginas_leidas, patota);

			}

		}

		memcpy(&tid, datos + offset, sizeof(tid));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			offset += sizeof(uint32_t);
			pagina = floor(offset/TAMANIO_PAGINA);

			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * pagina);
				list_add(paginas_leidas, patota);

			}

			char estado = get_status(mensaje->estado);
			memcpy(datos + offset, &estado, sizeof(char));
			modificar_en_memoria_principal(list_get(paginas, pagina), datos + pagina * TAMANIO_PAGINA);

		}else{

			offset += sizeof(t_tcb);
			pagina = floor(offset/TAMANIO_PAGINA);

		}

	}

	void liberar_nros_usadas(int32_t* frame){
		free(frame);
	}
	list_destroy_and_destroy_elements(paginas_leidas, liberar_nros_usadas);
	free(datos);

	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

}

char* siguiente_tarea_paginacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	char* tarea = "";

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	t_list* paginas_leidas = list_create();
	void* datos =  malloc(list_size(paginas) * TAMANIO_PAGINA);
	uint32_t offset = sizeof(uint32_t);
	int32_t paginas_ne = paginas_necesarias(offset, sizeof(uint32_t));
	uint32_t pagina = floor(offset/TAMANIO_PAGINA);

	for(uint32_t i = 0; i < paginas_ne; i++){


		t_pagina_patota* patota = list_get(paginas, pagina + i);
		leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
		int32_t* pagina_leida = malloc(sizeof(int32_t));
		*pagina_leida = pagina + i;
		list_add(paginas_leidas, pagina_leida);

	}

	uint32_t direccion_tareas;

	memcpy(&direccion_tareas, datos + offset, sizeof(uint32_t));
	uint32_t direccion_estado;
	bool encontrado = false;
	uint32_t tarea_actual;
	while(!encontrado){

		uint32_t tid;
		int32_t paginas_ne = paginas_necesarias(offset, sizeof(tid));

		for(uint32_t i = 0; i < paginas_ne; i++){


			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina + i);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina + i);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
				list_add(paginas_leidas, patota);

			}

		}

		memcpy(&tid, datos + offset, sizeof(tid));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			direccion_estado = offset + sizeof(uint32_t);
			offset += sizeof(uint32_t) * 3 + sizeof(char);
			pagina = floor(offset/TAMANIO_PAGINA);
			int32_t paginas_ne = paginas_necesarias(offset, sizeof(tarea_actual));

			for(uint32_t i = 0; i < paginas_ne; i++){


				bool ya_leida(t_pagina_patota* patota){

					return patota == list_get(paginas, pagina + i);

				}

				if(!list_any_satisfy(paginas_leidas, ya_leida)){

					t_pagina_patota* patota = list_get(paginas, pagina + i);
					leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
					list_add(paginas_leidas, patota);

				}

			}

			memcpy(&tarea_actual, datos + offset, sizeof(tarea_actual));

			uint32_t sig_tarea = tarea_actual + 1;

			memcpy(datos + offset, &sig_tarea, sizeof(sig_tarea));

			for(uint32_t i = 0; i < paginas_ne; i++){

				modificar_en_memoria_principal(list_get(paginas, pagina + i), datos + (pagina + i) * TAMANIO_PAGINA);

			}

		}else{

			offset += sizeof(t_tcb);
			pagina = floor(offset/TAMANIO_PAGINA);

		}

	}

	uint32_t i = 0;
	do{

		pagina = floor(direccion_tareas/TAMANIO_PAGINA);

		bool ya_leida(t_pagina_patota* patota){

			return patota == list_get(paginas, pagina);

		}

		if(!list_any_satisfy(paginas_leidas, ya_leida)){

			t_pagina_patota* patota = list_get(paginas, pagina);
			leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * pagina);
			list_add(paginas_leidas, patota);

		}

		char letra;
		memcpy(&letra, datos + direccion_tareas, 1);
		if(letra == '\n'){
			if(i < tarea_actual){
				tarea = "";
			}
			i++;

		} else if (letra == '\0'){
			*termino = true;

			pagina = floor(direccion_estado/TAMANIO_PAGINA);

			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * pagina);
				list_add(paginas_leidas, patota);

			}

			char estado_e = 'E';

			memcpy(datos + direccion_estado, &estado_e, sizeof(char));

			modificar_en_memoria_principal(list_get(paginas, pagina), datos + pagina * TAMANIO_PAGINA);

			break;
		}else{
			string_append(&tarea, &letra);
			direccion_tareas++;
		}


	}while(i <= tarea_actual);

	void liberar_nros_usadas(int32_t* frame){
		free(frame);
	}
	list_destroy_and_destroy_elements(paginas_leidas, liberar_nros_usadas);
	free(datos);

	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

	return tarea;

}

void expulsar_tripulante_paginacion(expulsar_tripulante_msg* mensaje, bool* status){

	pthread_mutex_lock(&m_TABLAS_PAGINAS);

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	uint32_t size_viejo = list_size(paginas) * TAMANIO_PAGINA;
	void* datos = malloc(size_viejo);
	uint32_t cant_frames = ceil((size_viejo - sizeof(t_tcb)) / TAMANIO_PAGINA);
	uint32_t size_pcb = cant_frames * TAMANIO_PAGINA;
	void* datos_nuevo = malloc(size_pcb);
	t_list* paginas_a_borrar = list_create();
	t_list* paginas_leidas = list_create();
	bool encontrado = false;
	uint32_t offset_viejo = sizeof(t_pcb);
	uint32_t offset_nuevo = sizeof(t_pcb);
	uint32_t pagina = floor(offset_viejo/TAMANIO_PAGINA);


	while(!encontrado){

		uint32_t tid;
		uint32_t paginas_ne = paginas_necesarias(offset_viejo, sizeof(tid));

		for(uint32_t i = 0; i < paginas_ne; i++){

			bool ya_leida(t_pagina_patota* patota){

				return patota == list_get(paginas, pagina + i);

			}

			if(!list_any_satisfy(paginas_leidas, ya_leida)){

				t_pagina_patota* patota = list_get(paginas, pagina + i);
				leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
				list_add(paginas_leidas, patota);

			}

		}

		memcpy(&tid, datos + offset_viejo, sizeof(uint32_t));

		if(tid == mensaje->idTripulante){

			encontrado = true;
			offset_viejo += sizeof(t_tcb);
			pagina = floor(offset_viejo/TAMANIO_PAGINA);
			paginas_ne = paginas_necesarias(offset_viejo, size_viejo - offset_viejo);

			for(uint32_t i = 0; i < paginas_ne; i++){


				bool ya_leida(t_pagina_patota* patota){

					return patota == list_get(paginas, pagina + i);

				}

				if(!list_any_satisfy(paginas_leidas, ya_leida)){

					t_pagina_patota* patota = list_get(paginas, pagina + i);
					leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
					list_add(paginas_leidas, patota);

				}


			}

			memcpy(datos_nuevo + offset_nuevo, datos + offset_viejo, size_viejo - offset_viejo);


		}else{
			offset_nuevo += sizeof(t_tcb);
			offset_viejo += sizeof(t_tcb);
		}

	}


	if(list_size(paginas) > cant_frames){

		for(int32_t i = cant_frames; i < list_size(paginas); i++){

			t_pagina_patota* pagina = list_get(paginas ,i);
			liberar_memoria_principal_paginacion(pagina);
			liberar_memoria_virtual(pagina);
			void liberar_pagina(t_pagina_patota* pagina){
				free(pagina);
			}
			list_remove_and_destroy_element(paginas, i, liberar_pagina);

		}

	}

	for(int32_t i = pagina; i < cant_frames; i++){

		t_pagina_patota* pagina = list_get(paginas, i);

		modificar_en_memoria_principal(pagina, datos_nuevo + (i * TAMANIO_PAGINA));

	}

	uint32_t direccion_tareas;
	offset_viejo = sizeof(uint32_t);
	pagina  = floor(offset_viejo/TAMANIO_PAGINA);
	uint32_t paginas_ne = paginas_necesarias(offset_viejo, sizeof(direccion_tareas));

	for(uint32_t i = 0; i < paginas_ne; i++){


		bool ya_leida(t_pagina_patota* patota){

			return patota == list_get(paginas, pagina + i);

		}

		if(!list_any_satisfy(paginas_leidas, ya_leida)){

			t_pagina_patota* patota = list_get(paginas, pagina + i);
			leer_pagina_de_memoria(patota, datos + TAMANIO_PAGINA * (pagina + i));
			list_add(paginas_leidas, patota);

		}


	}

	memcpy(&direccion_tareas, datos + offset_viejo, sizeof(direccion_tareas));

	direccion_tareas -= sizeof(t_tcb);

	for(uint32_t i = 0; i < paginas_ne; i++){
		modificar_en_memoria_principal(list_get(paginas, pagina + i), datos + (pagina + i) * TAMANIO_PAGINA);
	}

	void liberar_nros_usadas(int32_t* frame){
		free(frame);
	}
	list_destroy_and_destroy_elements(paginas_leidas, liberar_nros_usadas);
	free(datos);
	free(datos_nuevo);

	pthread_mutex_unlock(&m_TABLAS_PAGINAS);

}

void guardar_en_memoria_principal(t_pagina_patota* pagina, void* datos){

	pthread_mutex_lock(&m_MEM_PRINCIPAL);

	if(!pagina->presente)
		pasar_de_swap_a_principal(pagina);

	memcpy( memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);

	pthread_mutex_unlock(&m_MEM_PRINCIPAL);

}

void guardar_en_memoria_swap(t_pagina_patota* pagina, void* datos){

	pthread_mutex_lock(&m_MEM_VIRTUAL);

	memcpy(memoria_virtual + pagina->nro_frame_mv * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);

	pthread_mutex_unlock(&m_MEM_VIRTUAL);
}

void* traer_de_swap(t_pagina_patota* pagina){
	pthread_mutex_lock(&m_MEM_VIRTUAL);

	void* datos = malloc(sizeof(TAMANIO_PAGINA));

	memcpy(datos, memoria_virtual + pagina->nro_frame * TAMANIO_PAGINA , sizeof(TAMANIO_PAGINA));

	return datos;

	pthread_mutex_unlock(&m_MEM_VIRTUAL);

}

void modificar_en_memoria_principal(t_pagina_patota* pagina, void* datos){

	guardar_en_memoria_principal(pagina, datos);
	pagina->modificado = true;

}

void pasar_de_swap_a_principal(t_pagina_patota* pagina){
	void* datos = traer_de_swap(pagina);

	pagina->nro_frame = get_frame();
	guardar_en_memoria_principal(pagina, datos);
	pagina->modificado = false;
	pagina->presente = true;
	pagina->uso = true;
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

	free(datos);

}

bool entra_en_swap(uint32_t cantidad){
	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	bool valor = list_size(frames_swap) >= cantidad;

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

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

int32_t get_frame(){
	int32_t nro_frame = -1;

	if(!memoria_llena()){
		t_frame* first_fit = list_remove(frames_libres_principal, 0);
		nro_frame = first_fit->pos;
		if(ALGORITMO_REEMPLAZO == LRU){
			buffer_clock_pos = nro_frame + 1;
			if(buffer_clock_pos == (TAMANIO_MEMORIA / 32)){
				buffer_clock_pos = 0;
			}
		}
		free(first_fit);
	}else{

		switch(ALGORITMO_REEMPLAZO){
			pthread_mutex_lock(&m_LISTA_REEMPLAZO);

			case LRU: nro_frame = reemplazo_LRU(); break;
			case CLOCK: nro_frame = reemplazo_Clock(); break;

			pthread_mutex_unlock(&m_LISTA_REEMPLAZO);
		}
		if(nro_frame >= 0){
			log_debug(logger, "Reemplazo en frame %d", nro_frame);
		}else{
			log_error(logger, "Fallo reemplazo en memoria principal");
		}

	}

	return nro_frame;
}

int32_t get_frame_memoria_virtual(){

	int32_t pos = -1;

	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	if(list_size(frames_swap) > 0){


		t_frame* frame = (list_remove(frames_swap, 0));

		pos = frame->pos;

		free(frame);

	}

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

	return pos;

}

int32_t reemplazo_LRU(){
	int32_t frame = -1;

	t_pagina_patota* pagina_removida = list_remove(lista_para_reemplazo, 0);
	frame = pagina_removida->nro_frame;
	pagina_removida->presente =false;
	pagina_removida->uso= false;
	if(pagina_removida->modificado){
		pthread_mutex_lock(&m_MEM_PRINCIPAL);

		guardar_en_memoria_swap(pagina_removida, memoria_principal + pagina_removida->nro_frame * TAMANIO_PAGINA);

		pthread_mutex_unlock(&m_MEM_PRINCIPAL);
	}

	return frame;
}

int32_t reemplazo_Clock(){
	int32_t frame = -1;
	t_pagina_patota* pagina_removida = NULL;
	uint32_t buffer_recorrido = buffer_clock_pos;
	uint32_t pos_max = (TAMANIO_MEMORIA / TAMANIO_PAGINA) - 1;
	while(pagina_removida == NULL){

		do{
			t_buffer_clock* buff = list_get(lista_para_reemplazo, buffer_recorrido);

			if(!buff->pagina->uso){
				pagina_removida = buff->pagina;
				buff->pagina = NULL;
			}
			buffer_recorrido++;
			if(buffer_recorrido > pos_max){
				buffer_recorrido = 0;
			}
		}while(pagina_removida == NULL);

	}

	buffer_clock_pos = buffer_recorrido;
	frame = pagina_removida->nro_frame;

	pagina_removida->presente =false;
	if(pagina_removida->modificado){
		pthread_mutex_lock(&m_MEM_PRINCIPAL);

		guardar_en_memoria_swap(pagina_removida, memoria_principal + pagina_removida->nro_frame * TAMANIO_PAGINA);

		pthread_mutex_unlock(&m_MEM_PRINCIPAL);
	}
	return frame;
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

	if(pagina->presente){

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

		t_frame_libre* frame = malloc(sizeof(t_frame_libre));
		frame->pos = pagina->nro_frame;
		list_add(frames_libres_principal, frame);

		bool cmp_frames_libres (t_frame_libre* frame1, t_frame_libre* frame2){
			return frame1->pos < frame2->pos;
		}

		list_sort(frames_libres_principal, cmp_frames_libres);

		pthread_mutex_unlock(&m_TABLA_LIBRES_P);

	}

}

void liberar_memoria_virtual(t_pagina_patota* pagina){

	pthread_mutex_lock(&m_TABLA_LIBRES_V);

	t_frame* frame = malloc(sizeof(t_frame));
	frame->pos = pagina->nro_frame_mv;
	list_add(frames_swap, frame);

	bool cmp_frames_libres (t_frame* frame1, t_frame* frame2){
		return frame1->pos < frame2->pos;
	}

	list_sort(frames_swap, cmp_frames_libres);

	pthread_mutex_unlock(&m_TABLA_LIBRES_V);

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
	list_create(segmentos_libres);
	list_create(segmentos_en_memoria);
	memoria_vacia->inicio = 0;
	memoria_vacia->tamanio = TAMANIO_MEMORIA;
	list_add(segmentos_libres, memoria_vacia);
	tablas_seg_patota = dictionary_create();
}

// guarda la patota en memoria, crea la tabla de segmentos y la guarda en el dictionary
void crear_patota_segmentacion(iniciar_patota_msg* mensaje, bool* status){

	if(!dictionary_has_key(tablas_seg_patota, string_itoa(mensaje->idPatota))){

		t_list* tabla_patota = list_create();

		uint32_t size_tareas = mensaje->tareas->length;

		//Creo los segmentos para la tabla de segmentos

		segmento* seg_pcb = malloc(sizeof(segmento));
		seg_pcb->numero_segmento = 0;
		seg_pcb->tamanio = sizeof(t_pcb);

		segmento* seg_tareas = malloc(sizeof(segmento));
		seg_tareas->numero_segmento = 1;
		seg_tareas->tamanio = size_tareas;

		segmento* seg_tcb[mensaje->cant_tripulantes];
		for(int j = 0; j < mensaje->cant_tripulantes; j++){
			seg_tcb[j] = malloc(sizeof(segmento));
			seg_tcb[j]->numero_segmento = j+2;
			seg_tcb[j]->tamanio = sizeof(t_pcb);
		}

		//Cargo datos para copiarlos a memoria

		//pcb
		t_pcb* pcb = malloc(sizeof(t_pcb));
		pcb->pid = mensaje->idPatota;

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

		uint32_t offset;

		uint32_t error_guardado = 0;

		//guarda el pcb
		if(hay_espacio_libre(sizeof(t_pcb))){
			offset = get_espacio_libre(sizeof(t_pcb));
			seg_pcb->inicio = offset;
			memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
		} else {
			compactar_memoria();
			if(hay_espacio_libre(sizeof(t_pcb))){
				offset = get_espacio_libre(sizeof(t_pcb));
				seg_pcb->inicio = offset;
				memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
			} else {
				error_guardado = 1;
			}
		}

		//guarda las tareas
		if(hay_espacio_libre(size_tareas)){
			offset = get_espacio_libre(size_tareas);
			seg_tareas->inicio = offset;
			memcpy(memoria_principal + offset, mensaje->tareas->string, size_tareas);
		} else {
			compactar_memoria();
			if(hay_espacio_libre(size_tareas)){
				offset = get_espacio_libre(size_tareas);
				seg_tareas->inicio = offset;
				memcpy(memoria_principal + offset, mensaje->tareas->string, size_tareas);
			} else {
				error_guardado = 1;
			}
		}

		//guarda los tcb
		void cargarTcbDatos(t_tcb* tcb){
			if(hay_espacio_libre(sizeof(t_tcb))){
				offset = get_espacio_libre(sizeof(t_tcb));
				seg_tcb->inicio = offset;
				memcpy(memoria_principal + offset, tcb, sizeof(t_tcb));
			} else {
				compactar_memoria();
				if(hay_espacio_libre(sizeof(t_tcb))){
					offset = get_espacio_libre(sizeof(t_tcb));
					seg_tcb->inicio = offset;
					memcpy(memoria_principal + offset, tcb, sizeof(t_tcb));
				} else {
					error_guardado = 1;
				}
			}
		}

		list_iterate(tcbs, cargarTcbDatos);

		//Libero las estructuras para guardar en memoria
		free(pcb);

		void liberar_tcbs(t_tcb* tcb){
			free(tcb);
		}
		list_destroy_and_destroy_elements(tcbs, liberar_tcbs);

		//los segmentos no deberia liberarlos no? porque estan siendo referenciados por las listas

		if(!error_guardado){

			//Guardo tabla de paginas
			dictionary_put(tablas_seg_patota, string_itoa(mensaje->idPatota), tabla_patota);

			list_iterate(tabla_patota, sacar_segmento_lista_libres);

			list_add_all(segmentos_en_memoria, tabla_patota);

		} else {

			status = false;
		}
	}
}

// modifica las coordenadas de un tripulante en memoria
void informar_movimiento_segmentacion(informar_movimiento_ram_msg* mensaje, bool* status){

	void* buffer = malloc(sizeof(t_tcb));
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);

	//traigo de memoria el tcb con el offset encontrado
	memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
	//reemplazo los valores
	uint32_t offset_buffer = sizeof(uint32_t) + sizeof(char); //id y estado
	memcpy(buffer + offset_buffer, mensaje->coordenadasDestino->posX, sizeof(uint32_t));
	offset_buffer += sizeof(uint32_t); //posX
	memcpy(buffer + offset_buffer, mensaje->coordenadasDestino->posY, sizeof(uint32_t));

	//lo copio en memoria modificado
	memcpy(memoria_principal + offset, buffer, sizeof(t_tcb));

	//libero memoria
	free(buffer);
}

// modifica el estado de un tripulante en memoria
void cambiar_estado_segmentacion(cambio_estado_msg* mensaje, bool* status){

	void* buffer = malloc(sizeof(t_tcb));
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);

	char estado = get_status(mensaje->estado);

	//traigo de memoria el tcb con el offset encontrado
	memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
	//reemplazo los valores
	uint32_t offset_buffer = sizeof(uint32_t); //id
	memcpy(buffer + offset_buffer, &estado, sizeof(char));

	//lo copio en memoria modificado
	memcpy(memoria_principal + offset, buffer, sizeof(t_tcb));

	//libero memoria
	free(buffer);
}

// SIN TERMINAR
char* siguiente_tarea_segmentacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status){

	char* tarea = "";
	t_list* tabla_patota = dictionary_get(tablas_seg_patota, string_itoa(mensaje->idPatota));
	uint32_t offset = buscar_offset_tripulante(mensaje->idTripulante, mensaje->idPatota);
	uint32_t proxima_instruccion;

	void* buffer_tcb = malloc(sizeof(t_tcb));
	memcpy(buffer_tcb, memoria_principal + offset, sizeof(t_tcb));
	//traigo de alguna manera el numero de la siguiente instruccion

	segmento* seg_tareas = list_get(tabla_patota, 1);
	uint32_t direccion_tareas = seg_tareas->inicio;

	void* buffer_tareas = malloc(sizeof(seg_tareas->tamanio)); //el tamanio esta bien?
	memcpy(buffer_tareas, memoria_principal + seg_tareas->inicio, sizeof(seg_tareas->tamanio));

	//copiar la instruccion en tarea, si prox_instruccion es 0 la primera, y asi sucesivamente

	uint32_t offset_tcb = 0; //no

	//le sumo uno en memoria a la proxima instruccion
	proxima_instruccion += 1;
	memcpy(buffer_tcb + offset_tcb, proxima_instruccion, sizeof(uint32_t));
	memcpy(memoria_principal + offset, buffer_tcb, sizeof(t_tcb));

	return tarea;
}

//saca el segmento de la tabla de segmentos y agrega el segmento libre a la lista de libres
void expulsar_tripulante_segmentacion(expulsar_tripulante_msg* mensaje, bool* status){

	t_list* tabla_patota = dictionary_get(tablas_seg_patota, string_itoa(mensaje->idPatota));
	segmento* seg_tripulante = malloc(sizeof(segmento));
	seg_tripulante = buscar_segmento_tripulante(mensaje->idTripulante, mensaje->idPatota);
	uint32_t index = seg_tripulante->numero_segmento;

	void liberar_seg(segmento* seg){
		free(seg);
	}
	// saco el segmento de la tabla de segmentos de la patota
	list_remove_and_destroy_element(tabla_patota, index, liberar_seg);

	// agrego el segmento liberado a la lista de segmentos libres
	liberar_segmento(seg_tripulante);

}

segmento* buscar_segmento_tripulante(uint32_t id_tripulante, uint32_t id_patota){
	t_list* tabla_patota = dictionary_get(tablas_seg_patota, string_itoa(id_patota));
	segmento* seg_tripulante;

	uint32_t cant_tripulantes_patota = list_size(tabla_patota) - 2;
	uint32_t contador = 2;
	uint32_t tid;

	uint32_t encontrado = 0;
	uint32_t offset;
	void* buffer = malloc(sizeof(t_tcb));

	while(!encontrado){
		seg_tripulante = list_get(tabla_patota, contador);
		offset = seg_tripulante->inicio;
		memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
		memcpy(&tid, buffer, sizeof(tid));
		if(tid == id_tripulante){
				encontrado = 1;
		}
		contador++;
	}

	free(buffer);

	return seg_tripulante;
}

uint32_t buscar_offset_tripulante(uint32_t id_tripulante, uint32_t id_patota){

	t_list* tabla_patota = dictionary_get(tablas_seg_patota, string_itoa(id_patota));

	uint32_t cant_tripulantes_patota = list_size(tabla_patota) - 2;
	uint32_t contador = 2;
	uint32_t tid;

	uint32_t encontrado = 0;
	uint32_t offset;
	void* buffer = malloc(sizeof(t_tcb));

	while(!encontrado){
		segmento* seg_tripulante = list_get(tabla_patota, contador);
		offset = seg_tripulante->inicio;
		memcpy(buffer, memoria_principal + offset, sizeof(t_tcb));
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

//saca un segmento de la lista libres, si sobraba segmento guarda el sobrante, falta revision y free
void sacar_segmento_lista_libres(segmento* segmento_nuevo){

	uint32_t inicio_seg_nuevo = segmento_nuevo->inicio;
	uint32_t limite_seg_nuevo = obtener_limite(segmento_nuevo);

	bool se_encuentra_contenido(segmento* seg){
		uint32_t limite_segmento = obtener_limite(seg);
		return seg->inicio == inicio_seg_nuevo && limite_segmento >= limite_seg_nuevo;
	}

	segmento* seg_a_modificar;
	seg_a_modificar = list_get(list_filter(segmentos_libres, se_encuentra_contenido), 0);
	//ESTO ESTA BIEN PARA OBTENER EL SEGMENTO?

	bool es_el_segmento(segmento* seg){
		return seg->inicio == seg_a_modificar->inicio && seg->tamanio == seg_a_modificar->tamanio;
	}

	list_remove_by_condition(segmentos_libres, es_el_segmento);

	uint32_t limite_seg_libre = obtener_limite(seg_a_modificar);

	if(limite_seg_libre != limite_seg_nuevo){

		seg_a_modificar->inicio = limite_seg_nuevo; //el segmento libre nuevo arranca donde termina el otro
		list_add(segmentos_libres, seg_a_modificar);

		ordenar_lista_segmentos_libres();
	}
}

void ordenar_lista_segmentos_libres(){

	bool ordenar_segmentos(segmento* primer_segmento, segmento* segundo_segmento){
		return primer_segmento->inicio > segundo_segmento->inicio;
	}

	list_sort(segmentos_libres, ordenar_segmentos);
}

uint32_t obtener_limite(segmento* seg){
	return seg->inicio + seg->tamanio;
}

//le paso el tamanio de un segmento, devuelve 1 si hay espacio y 0 si no hay espacio
int32_t hay_espacio_libre(uint32_t size){

	t_list* lista_auxiliar = list_create();

	bool entra_en_el_segmento(segmento* seg){
		return seg->tamanio > size;
	}

	list_add_all(lista_auxiliar, filter(segmentos_libres, entra_en_el_segmento));

	return !list_is_empty(lista_auxiliar);
}

//devuelve un offset de donde arrancar a guardar un segmento en memoria
int32_t get_espacio_libre(uint32_t size){

	t_list* lista_auxiliar = list_create();

	bool entra_en_el_segmento(segmento* seg){
		return seg->tamanio > size;
	}

	list_add_all(lista_auxiliar, filter(segmentos_libres, entra_en_el_segmento));

	segmento* primer_seg_libre = malloc(sizeof(segmento));
	primer_seg_libre = list_get(lista_auxiliar, 0);

	if(list_size(lista_auxiliar) == 1){

		return primer_seg_libre->inicio;

	} else {
		if(CRITERIO_SELECCION == FF){

			return primer_seg_libre->inicio;

		} else {

			uint32_t offset_bf;
			uint32_t dif_tamanio_anterior = TAMANIO_MEMORIA - 1; //por ahora lo pongo asi porque no se como setear el primer anterior si no

			void encontrar_best_fit(segmento* seg){
				uint32_t dif_de_tamanio = seg->tamanio - size;
				if(dif_de_tamanio < dif_tamanio_anterior){
					offset_bf = seg->inicio;
				}
			}
			list_iterate(lista_auxiliar, encontrar_best_fit);

			return offset_bf;
		}
	}
}

//agrega el segmento a la lista de segmentos libres y lo saca de segmentos en memoria
void liberar_segmento(segmento* seg){
	//deberia chequear si esta? antes de agregar

	//agrega a la lista de segmentos libres
	list_add(segmentos_libres, seg);
	ordenar_lista_segmentos_libres();

	//lo saca de la lista de segmentos en memoria
	bool es_el_segmento(segmento* seg_a_comparar){
		return seg->inicio == seg_a_comparar->inicio && seg->tamanio == seg_a_comparar->tamanio;
	}
	remove_by_condition(segmentos_en_memoria, es_el_segmento);
}

//junta todos los segmentos en la parte superior de la memoria y crea un segmento libre con el restante
void compactar(){

	bool ordenar_segmentos(segmento* seg1, segmento* seg2){
		return seg1->inicio < seg2->inicio;
	}
	list_sort(segmentos_en_memoria, ordenar_segmentos());

	void* buffer = malloc(sizeof(TAMANIO_MEMORIA));
	uint32_t offset = 0;

	segmento* seg_anterior = malloc(sizeof(segmento));
	seg_anterior = list_get(segmentos_en_memoria, 0);
	uint32_t limite_seg_anterior;

	//copio en el buffer el primer segmento al principio (en 0)
	memcpy(buffer, memoria_principal + seg_anterior->inicio, sizeof(seg_anterior->tamanio));

	if(seg_anterior->inicio != 0){
		seg_anterior->inicio = 0;
		limite_seg_anterior = obtener_limite(seg_anterior);
	}

	offset = limite_seg_anterior;

	void modificar_inicio(segmento *seg){
		if(seg->inicio != 0){

			//transcribo el segmento al buffer
			memcpy(buffer + offset, memoria_principal + seg->inicio, sizeof(seg->tamanio));

			if(limite_seg_anterior != seg->inicio){
				seg->inicio = limite_seg_anterior;
			}

			seg_anterior = seg;
			limite_seg_anterior = obtener_limite(seg);
			offset = limite_seg_anterior;
		}
	}

	list_iterate(segmentos_en_memoria, modificar_inicio());

	uint32_t cant_segmentos = list_size(segmentos_en_memoria);
	segmento* ult_seg = malloc(sizeof(segmento));
	ult_seg = list_get(segmentos_en_memoria, cant_segmentos - 1);

	uint32_t tamanio_ocupado_en_memoria = obtener_limite(ult_seg);

	segmento* segmento_libre = malloc(sizeof(segmento));
	segmento_libre->inicio = tamanio_ocupado_en_memoria;
	segmento_libre->tamanio = TAMANIO_MEMORIA - tamanio_ocupado_en_memoria;

	list_clean(segmentos_libres);
	list_add(segmentos_libres, segmento_libre);

	memcpy(memoria_principal, buffer, sizeof(TAMANIO_MEMORIA));
	free(buffer);
}


