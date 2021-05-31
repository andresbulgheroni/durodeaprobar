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

				crear_patota(mensaje);

				free(mensaje->tareas);
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

	config = config_create("Debug/ram.config");
	logger = log_create("Debug/ram.log", "MI-RAM-HQ", true, LOG_LEVEL_DEBUG);

	TAMANIO_MEMORIA = config_get_int_value(config, "TAMANIO_MEMORIA");
	ESQUEMA_MEMORIA = get_esquema_memoria(config_get_string_value(config, "ESQUEMA_MEMORIA"));
	TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
	TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	ALGORITMO_REEMPLAZO = get_algoritmo(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));;
	IP = config_get_string_value(config, "IP");
	PUERTO = config_get_string_value(config, "PUERTO");

	memoria_principal = malloc(TAMANIO_MEMORIA);

	switch(ESQUEMA_MEMORIA){
		case SEGMENTACION_PURA:break;
		case PAGINACION_VIRTUAL:{

			configurar_paginacion();

			break;
		}
	}
	//iniciarMapa();

}

void configurar_paginacion(){

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
			frame->pagina = NULL;
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

void crear_patota(iniciar_patota_msg* mensaje){


	if(!dictionary_has_key(tabla_paginas_patota, string_itoa(mensaje->idPatota))){

		int32_t cantidad_frames = ceil((sizeof(mensaje->idPatota) + mensaje->cant_tripulantes * (sizeof(uint32_t) * 3)
				+ sizeof(mensaje->tareas->length) + mensaje->tareas->length) / TAMANIO_PAGINA);

		t_list* paginas = list_create();

		void* datos = malloc(TAMANIO_PAGINA * cantidad_frames);

		int32_t offset = 0;

		//ver como mapear a memoria el mensaje

		offset = 0;
		for(int32_t i = 0; i < cantidad_frames; i++){

			t_pagina_patota* pagina = malloc(sizeof(t_pagina_patota));
			int32_t frame = get_frame();
			pagina->nro_frame = frame;
			pagina->presente = true;
			pagina->uso = true;
			pagina->modificado = false;

			guardar_en_memoria_swap(pagina, datos + i * TAMANIO_PAGINA);
			guardar_en_memoria_principal(pagina,  datos + i * TAMANIO_PAGINA);

			list_add(pagina);

		}

		dictionary_put(tabla_paginas_patota, string_itoa(mensaje->idPatota), paginas);

	}

}

void guardar_en_memoria_principal(t_pagina_patota* pagina, void* datos){

	memcpy( memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);

}
void guardar_en_memoria_swap(t_pagina_patota* pagina, void* datos){

	int32_t pos = pos_swap(pagina);

	t_frame* frame_swap = list_get(frames_swap, pos);
	memcpy(memoria_virtual + pos * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);
	frame_swap->pagina = pagina;
	pagina->nro_frame = pos;

}
/*

void iniciar_tripulante(iniciar_tripulante_msg* tripulante){

	if(dictionary_has_key(tabla_paginas_patota, string_itoa(tripulante->idPatota))){

		t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(tripulante->idPatota));
		uint32_t cantidad_paginas_inicial = list_size(paginas);
		int32_t size_inicial = cantidad_paginas_inicial * TAMANIO_PAGINA;
		void* datos_cargados = malloc(size_inicial);
		uint32_t indice = 0;
		void obtener_paginas_de_memoria(t_pagina_patota* pagina){

			if(!pagina->presente)
				traer_pagina_a_memoria(pagina);

			memcpy(datos_cargados + indice++ * TAMANIO_PAGINA ,memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, TAMANIO_PAGINA);
			pagina->uso = true;

		}

		list_iterate(paginas, obtener_paginas_de_memoria);


		//REVISAR ESTA PARTE SEGUN LO QUE DIGAN LOS AYUDANTES
		t_tcb* tripulante_nuevo = malloc(sizeof(t_tcb));

		tripulante_nuevo->tid = tripulante->idTripulante;
		tripulante_nuevo->estado = 'N';
		tripulante_nuevo->posX = tripulante->coordenadas->posX;
		tripulante_nuevo->posY = tripulante->coordenadas->posY;
		tripulante_nuevo->proxima_instruccion = 0;
		tripulante_nuevo->direccion_patota = 0;

		uint32_t direccion_tareas;
		memcpy(&direccion_tareas, datos_cargados + sizeof(uint32_t), sizeof(uint32_t));

		uint32_t cantidad_frames = ceil((size_inicial + sizeof(t_tcb)) / TAMANIO_PAGINA);

		uint32_t size_nuevo = cantidad_frames * TAMANIO_PAGINA;

		void* datos_nuevos = malloc(size_nuevo);

		if(cantidad_frames > cantidad_paginas_inicial){

			t_list* nuevos_frames = get_frames_a_ubicar(cantidad_frames - cantidad_paginas_inicial);

			void agregar_paginas_nuevas(uint32_t* frame){

				t_pagina_patota* pagina_nueva = malloc(sizeof(t_pagina_patota));

				pagina_nueva->nro_frame = *frame;
				pagina_nueva->uso = true;
				pagina_nueva->presente = true;
				pagina_nueva->modificado = false;

				list_add(paginas, pagina_nueva);

			}

			list_iterate(nuevos_frames, agregar_paginas_nuevas);

		}



	}

}
*/

void traer_pagina_a_memoria(t_pagina_patota* pagina){



}

bool memoria_llena(){
	return list_size(frames_libres_principal) == 0;
}

bool swap_lleno(){
	return list_size(swap_libres()) == 0;
}

t_list* swap_libres(){
	bool libre(t_frame* frame){
		return frame->pagina == NULL;
	}

	return list_filter(frames_swap, libre);
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
			case LRU: nro_frame = reemplazo_LRU(); break;
			case CLOCK: nro_frame = reemplazo_Clock(); break;
		}
		if(nro_frame >= 0){
			log_debug(logger, "Reemplazo en frame %d", nro_frame);
		}else{
			log_error(logger, "Fallo reemplazo en memoria principal");
		}

	}

	return nro_frame;
}

int32_t pos_swap(t_pagina_patota* pagina){
	int32_t pos = -1;
	t_frame* frame_swap;

	bool en_swap(t_frame* frame){
		return frame->pagina == pagina;
	}

	t_list* contiene = list_filter(frames_swap, en_swap);

	if(list_size(contiene) == 0){
		t_list* libres = swap_libres();

		if(list_size(libres) > 0){
			frame_swap = (t_frame*)(list_get(libres, 0));
			pos = frame_swap->pos;
		}

	}else{
		frame_swap = (t_frame*)(list_get(contiene, 0));
		pos = frame_swap->pos;
	}

	return pos;

}

int32_t reemplazo_LRU(){
	int32_t frame = -1;

	t_pagina_patota* pagina_removida = list_remove(lista_para_reemplazo, 0);
	frame = pagina_removida->nro_frame;
	if(pagina_removida->modificado){
		uint32_t estado;
		pagina_removida->presente =false;
		guardar_en_memoria_swap(pagina_removida, traer_de_memoria_principal(pagina_removida), &estado);
	}

	return frame;
}

int32_t reemplazo_Clock(){
	int32_t frame = -1;
	t_pagina_patota* pagina_removida = NULL;
	uint32_t paso = 0;
	uint32_t buffer_recorrido = buffer_clock_pos;
	uint32_t pos_max = (TAMANIO_MEMORIA / TAMANIO_PAGINA) - 1;
	while(pagina_removida == NULL){
		switch (paso) {
			case 0:{
				do{
					t_buffer_clock* buff = list_get(lista_para_reemplazo, buffer_recorrido);

					if(!(buff->pagina->uso) && !(buff->pagina->modificado)){
						pagina_removida = buff->pagina;
						buff->pagina = NULL;
					}
					buffer_recorrido++;
					if(buffer_recorrido > pos_max){
						buffer_recorrido = 0;
					}
				}while(pagina_removida == NULL && buffer_recorrido != buffer_clock_pos);
				paso = 1;
				break;
			}
			case 1:{
				do{
					t_buffer_clock* buff = list_get(lista_para_reemplazo, buffer_recorrido);
					if(!(buff->pagina->uso) && (buff->pagina->modificado)){
						pagina_removida = buff->pagina;
						buff->pagina = NULL;
					}
					buff->pagina->uso = false;
					buffer_recorrido++;
					if(buffer_recorrido > pos_max){
						buffer_recorrido = 0;
					}
				}while(pagina_removida == NULL && buffer_recorrido != buffer_clock_pos);
				paso = 0;
				break;
			}
		}
	}

	buffer_clock_pos = buffer_recorrido;
	frame = pagina_removida->nro_frame;

	if(pagina_removida->modificado){
		uint32_t estado;
		pagina_removida->presente =false;
		guardar_en_memoria_swap(pagina_removida, traer_de_memoria_principal(pagina_removida), &estado);
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

