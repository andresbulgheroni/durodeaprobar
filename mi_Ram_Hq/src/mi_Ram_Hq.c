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

				//FALTA ID PATOTA EN EL MENSAJE
				informar_movimiento_ram_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);
				log_debug(logger, "PosX: %d", mensaje->coordenadasDestino->posX);
				log_debug(logger, "PosY: %d", mensaje->coordenadasDestino->posY);

				free(mensaje->coordenadasDestino);
				free(mensaje);

				break;

			}
			case CAMBIO_ESTADO:{

				//FALTA ID PATOTA EN EL MENSAJE
				cambio_estado_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Estado: %d", mensaje->estado);
				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);

				free(mensaje);

				break;

			}
			case SOLICITAR_SIGUIENTE_TAREA:{

				//FALTA ID PATOTA EN EL MENSAJE
				solicitar_siguiente_tarea_msg* mensaje = deserializar_paquete(paquete);

				log_debug(logger, "Id tripulante: %d", mensaje->idTripulante);

				break;
			}
			case EXPULSAR_TRIPULANTE_MSG:{

				//FALTA ID PATOTA EN EL MENSAJE
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

uint32_t generar_direccion_logica_paginacion(uint32_t pagina, uint32_t desplazamiento){

	uint32_t direccion = 0;

	uint32_t bits_derecha = floor(log(TAMANIO_PAGINA) / log(2)) + 1;

	direccion = (pagina << bits_derecha) | desplazamiento;

	return direccion;

}
void obtener_direccion_logica_paginacion(uint32_t* pagina, uint32_t* desplazamiento, uint32_t direccion){

	uint32_t bits_derecha = floor(log(TAMANIO_PAGINA) / log(2)) + 1;
	uint32_t bits_izquierda = 32  - bits_derecha;

	*pagina = (direccion >> bits_derecha);
	*desplazamiento = (direccion << bits_izquierda) >> bits_izquierda;

}

void crear_patota(iniciar_patota_msg* mensaje, bool* status){

	if(!dictionary_has_key(tabla_paginas_patota, string_itoa(mensaje->idPatota))){

		//Saco cantidad de frames/paginas que va a ocupar
		uint32_t cantidad_frames = ceil((sizeof(t_pcb) + mensaje->cant_tripulantes * (sizeof(t_tcb))
				+  mensaje->tareas->length) / TAMANIO_PAGINA);
		uint32_t size_pcb = cantidad_frames * TAMANIO_PAGINA;

		status = entra_en_swap(cantidad_frames);
		status = entra_en_memoria(size_pcb);

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
				tcb->direccion_patota = 0;
				tcb->estado = 'R';
				tcb->proxima_instruccion = 0;
				tcb->posX = tripulante->coordenadas->posX;
				tcb->posY = tripulante->coordenadas->posY;

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

			// Creo las paginas y paso los datos a memoria principal y swap
			offset = 0;
			for(int32_t i = 0; i < cantidad_frames; i++){

				t_pagina_patota* pagina = malloc(sizeof(t_pagina_patota));
				pagina->nro_frame = get_frame();
				pagina->nro_frame_mv = get_frame_memoria_virtual();
				pagina->presente = true;
				pagina->uso = true;
				pagina->modificado = false;

				guardar_en_memoria_swap(pagina, datos + i * TAMANIO_PAGINA);
				guardar_en_memoria_principal(pagina,  datos + i * TAMANIO_PAGINA);

				list_add(pagina);

			}

			//Guardo tabla de paginas
			dictionary_put(tabla_paginas_patota, string_itoa(mensaje->idPatota), paginas);

		}

	}

}

void* leer_datos_patota(uint32_t id_patota){

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(id_patota));

	void* datos = malloc(TAMANIO_PAGINA * list_size(paginas));

	uint32_t offset = 0;
	void leer_pagina_de_memoria(t_pagina_patota* pagina){

		if(!pagina->presente)
			pasar_de_swap_a_principal(pagina);

		memcpy(datos + offset, memoria_principal + pagina->nro_frame  * TAMANIO_PAGINA, sizeof(TAMANIO_PAGINA));
		offset += sizeof(TAMANIO_PAGINA);

	}

	list_iterate(paginas, leer_pagina_de_memoria);

	return datos;

}

void guardar_en_memoria_principal(t_pagina_patota* pagina, void* datos){

	memcpy( memoria_principal + pagina->nro_frame * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);

}
void guardar_en_memoria_swap(t_pagina_patota* pagina, void* datos){

	memcpy(memoria_virtual + pagina->nro_frame_mv * TAMANIO_PAGINA, datos, TAMANIO_PAGINA);
}


void* traer_de_swap(t_pagina_patota* pagina){
	void* datos = malloc(sizeof(TAMANIO_PAGINA));

	memcpy(datos, memoria_virtual + pagina->nro_frame * TAMANIO_PAGINA , sizeof(TAMANIO_PAGINA));

	return datos;
}

void pasar_de_swap_a_principal(t_pagina_patota* pagina){
	void* datos = traer_de_swap(pagina);

	pagina->nro_frame = get_frame();
	guardar_en_memoria_principal(pagina, datos);
	pagina->modificado = false;
	pagina->presente = true;
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
		return list_size(frames_swap) >= cantidad;
}

bool entra_en_memoria(uint32_t size_pcb){
		return TAMANIO_MEMORIA >= size_pcb;
}

bool memoria_llena(){
	return list_size(frames_libres_principal) == 0;
}

bool swap_lleno(){
	return list_size(frames_swap) == 0;
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

int32_t get_frame_memoria_virtual(){
	int32_t pos = -1;

	if(list_size(frames_swap) > 0){

		pos = *(list_remove(frames_swap, 0));

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

