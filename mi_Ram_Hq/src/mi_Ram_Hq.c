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


NIVEL* mapa;

int main(void) {

	init();

	pthread_t hilo_server;
	pthread_create(&hilo_server,NULL,(void*)hilo_servidor, NULL);
	pthread_join(hilo_server, NULL);

	terminar();

	return EXIT_SUCCESS;
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

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: break;
					case PAGINACION_VIRTUAL: crear_patota_paginacion(mensaje, NULL);break;
				}

				free(mensaje->tareas);
				free(mensaje);

				break;

			}
			case INFORMAR_MOVIMIENTO_RAM:{

				informar_movimiento_ram_msg* mensaje = deserializar_paquete(paquete);

				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: break;
					case PAGINACION_VIRTUAL: informar_movimiento_paginacion(mensaje, NULL);break;
				}

				free(mensaje->coordenadasDestino);
				free(mensaje);

				break;

			}
			case CAMBIO_ESTADO:{

				cambio_estado_msg* mensaje = deserializar_paquete(paquete);


				switch(ESQUEMA_MEMORIA){
					case SEGMENTACION_PURA: break;
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
					case SEGMENTACION_PURA: break;
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
					case SEGMENTACION_PURA: break;
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

	config = config_create("Debug/ram.config");
	logger = log_create("Debug/ram.log", "MI-RAM-HQ", true, LOG_LEVEL_DEBUG);

	TAMANIO_MEMORIA = config_get_int_value(config, "TAMANIO_MEMORIA");
	ESQUEMA_MEMORIA = get_esquema_memoria(config_get_string_value(config, "ESQUEMA_MEMORIA"));
	TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
	TAMANIO_SWAP = config_get_int_value(config, "TAMANIO_SWAP");
	PATH_SWAP = config_get_string_value(config, "PATH_SWAP");
	ALGORITMO_REEMPLAZO = get_algoritmo(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));;
	IP = config_get_string_value(config, "IP");
	CRITERIO_SELECCION = get_criterio(config_get_string_value(config, "CRITERIO_SELECCION"));
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

void crear_patota_paginacion(iniciar_patota_msg* mensaje, bool* status){

	if(!dictionary_has_key(tabla_paginas_patota, string_itoa(mensaje->idPatota))){

		//Saco cantidad de frames/paginas que va a ocupar
		uint32_t cantidad_frames = ceil((sizeof(t_pcb) + mensaje->cant_tripulantes * (sizeof(t_tcb))
				+  mensaje->tareas->length) / TAMANIO_PAGINA);
		uint32_t size_pcb = cantidad_frames * TAMANIO_PAGINA;

		*status = entra_en_swap(cantidad_frames);
		*status = entra_en_memoria(size_pcb);// PREGUNTAR SI ESTA BIEN

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

void* leer_datos_patota_paginacion(t_list* paginas){

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

void informar_movimiento_paginacion(informar_movimiento_ram_msg* mensaje, bool* status){

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	void* datos = leer_datos_patota_paginacion(paginas);

	uint32_t offset = sizeof(t_pcb);
	bool encontrado = false;
	while(!encontrado){

		uint32_t tid;
		memcpy(&tid, datos + offset, sizeof(uint32_t));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			offset += sizeof(uint32_t) + sizeof(char);
			memcpy(datos + offset, mensaje->coordenadasDestino->posX, sizeof(uint32_t));
			uint32_t pagina = floor(offset/TAMANIO_PAGINA);
			modificar_en_memoria_principal(list_get(paginas, pagina), datos + pagina * TAMANIO_PAGINA);

			offset += sizeof(uint32_t);
			memcpy(datos + offset, mensaje->coordenadasDestino->posX, sizeof(uint32_t));
			uint32_t pagina = floor(offset/TAMANIO_PAGINA);
			modificar_en_memoria_principal(list_get(paginas, pagina), datos + pagina * TAMANIO_PAGINA);

		}else{

			offset += sizeof(t_tcb);

		}

	}

	free(datos);

}


void cambiar_estado_paginacion(cambio_estado_msg* mensaje, bool* status){

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	void* datos = leer_datos_patota_paginacion(paginas);

	uint32_t offset = sizeof(t_pcb);
	bool encontrado = false;
	while(!encontrado){

		uint32_t tid;
		memcpy(&tid, datos + offset, sizeof(uint32_t));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			offset += sizeof(uint32_t);
			char status = get_status(mensaje->estado);
			memcpy(datos + offset, &status, sizeof(char));
			uint32_t pagina = floor(offset/TAMANIO_PAGINA);
			modificar_en_memoria_principal(list_get(paginas, pagina), datos + pagina * TAMANIO_PAGINA);

		}else{

			offset += sizeof(t_tcb);

		}

	}
	free(datos);
}

char* siguiente_tarea_paginacion(solicitar_siguiente_tarea_msg* mensaje, bool* termino, bool* status){

	char* tarea = "";

	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	void* datos = leer_datos_patota_paginacion(paginas);

	uint32_t direccion_tareas;

	memcpy(&direccion_tareas, datos + sizeof(uint32_t), sizeof(uint32_t));

	uint32_t offset = sizeof(t_pcb);
	bool encontrado = false;
	uint32_t tarea_actual;
	while(!encontrado){

		uint32_t tid;
		memcpy(&tid, datos + offset, sizeof(uint32_t));

		if(tid == mensaje->idTripulante){

			encontrado = true;

			offset += sizeof(uint32_t) * 3 + sizeof(char);
			memcpy(tarea_actual, datos + offset, sizeof(uint32_t));

		}else{

			offset += sizeof(t_tcb);

		}

	}

	uint32_t i = 0;
	do{

		char* letra = malloc(1);
		memcpy(letra, datos + direccion_tareas, 1);
		if(strcmp(letra, "\n") == 0){
			if(i < tarea_actual){
				tarea = "";
			}
			i++;

		} else if (strcmp(letra, "\0") == 0){
			termino = true;
			break;
		}else{
			string_append(tarea, letra);
			direccion_tareas++;
		}


	}while(i <= tarea_actual);

	free(datos);

	return tarea;

}

void expulsar_tripulante_paginacion(expulsar_tripulante_msg* mensaje, bool* status){
	//falta verificar si no encuentra el id
	t_list* paginas = dictionary_get(tabla_paginas_patota, string_itoa(mensaje->idPatota));

	void* datos = leer_datos_patota_paginacion(paginas);

	uint32_t size_viejo = list_size(paginas) * TAMANIO_PAGINA;
	uint32_t cant_frames = ceil((size_viejo - sizeof(t_tcb)) / TAMANIO_PAGINA);
	uint32_t size_pcb = cant_frames * TAMANIO_PAGINA;

	void* datos_nuevo = malloc(size_pcb);

	uint32_t pagina_modificar = 0;

	memcpy(datos_nuevo, datos, sizeof(t_tcb));

	bool encontrado = false;
	uint32_t offset_viejo = sizeof(t_tcb);
	uint32_t offset_nuevo = sizeof(t_tcb);

	while(!encontrado){

		uint32_t tid;
		memcpy(&tid, datos + offset_viejo, sizeof(uint32_t));

		if(tid == mensaje->idTripulante){

			encontrado = true;
			pagina_modificar = floor(offset_viejo/TAMANIO_PAGINA);
			offset_viejo += sizeof(t_tcb);

		}else{

			memcpy(datos_nuevo + offset_nuevo, datos + offset_viejo, sizeof(t_tcb));
			offset_nuevo += sizeof(t_tcb);
			offset_viejo += sizeof(t_tcb);

		}

	}

	memcpy(datos_nuevo + offset_nuevo, datos + offset_viejo, size_viejo - offset_viejo);

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

	for(int32_t i = pagina_modificar; i < cant_frames; i++){

		t_pagina_patota* pagina = list_get(paginas, i);

		modificar_en_memoria_principal(pagina, datos_nuevo + (i * TAMANIO_PAGINA));

	}

	free(datos);

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


char get_status(t_status_code codigo){

	switch(codigo){
		case NEW: return 'N';
		case READY: return 'R';
		case EXEC: return 'E';
		case BLOCKED: return 'B';
		default: return 'N';
	}

}

int32_t get_criterio(char* algoritmo_config) {

	if(strcmp("FF", algoritmo_config) == 0){

		return FF;

	} else if(strcmp("BF", algoritmo_config) == 0){

		return BF;

	}

	return -1;
}

void liberar_memoria_principal_paginacion(t_pagina_patota* pagina){

	if(pagina->presente){

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

		t_frame_libre* frame = malloc(sizeof(t_frame_libre));
		frame->pos = pagina->nro_frame;
		list_add(frames_libres_principal, frame);

		bool cmp_frames_libres (t_frame_libre* frame1, t_frame_libre* frame2){
			return frame1->pos < frame2->pos;
		}

		list_sort(frames_libres_principal, cmp_frames_libres);

	}
}

void liberar_memoria_virtual(t_pagina_patota* pagina){

	t_frame* frame = malloc(sizeof(t_frame));
	frame->pos = pagina->nro_frame_mv;
	list_add(frames_swap, frame);

	bool cmp_frames_libres (t_frame* frame1, t_frame* frame2){
		return frame1->pos < frame2->pos;
	}

	list_sort(frames_swap, cmp_frames_libres);

}

//por ahora lo dejo aca, dsp habria que organizar un poco el codigo
void cargar_tcb(tripulante_data_msg* tripulante, t_tcb* tcb){

		tcb->tid = tripulante->idTripulante;
		tcb->estado = 'R';
		tcb->posX = tripulante->coordenadas->posX;
		tcb->posY = tripulante->coordenadas->posY;
		tcb->proxima_instruccion = 0;
		tcb->direccion_patota = 0;

}

/*   SEGMENTACION   */

//no esta referenciada de ningun lado
void inicializar_segmentacion(){
	segmento memoria_vacia;
	memoria_vacia.inicio = 0;
	memoria_vacia.tamanio = TAMANIO_MEMORIA;
	list_add(segmentos_libres, memoria_vacia);
}

void crear_tabla_segmentos_patota(iniciar_patota_msg* mensaje){ //status

	t_list *tabla_segmentos_patota;
	tabla_segmentos_patota = crear_estructura_tabla_seg(mensaje);

	//Guardo los datos para pasarlos a memoria

	//pcb
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->pid = mensaje->idPatota;		//por ahora solo eso, hasta que ubique las tareas?

	//tareas
	char* tareas = malloc(); //malloc que?

	//tcbs
	t_tcb* tcb[mensaje->cant_tripulantes];
	for(int i = 0; i<mensaje->cant_tripulantes; i++){
		tcb[i] = malloc(sizeof(t_tcb));
		//cargar_tcb(tcb[i]); //esto esta claramente mal, como itero en los msj de trip, de donde los saco?
		tcb[i]->tid = i; //nqv pero para poner algo
	}

	uint32_t offset;

	//guarda el pcb
	if(hay_espacio_libre(sizeof(t_pcb))){
		offset = get_espacio_libre(sizeof(t_pcb));
		memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
	} else {
		compactar_memoria();
		if(hay_espacio_libre(sizeof(t_pcb))){
			offset = get_espacio_libre(sizeof(t_pcb));
			memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
		} else {
			//aca hay error, se cancela toodo
		}
	}

	//guarda las tareas
	if(hay_espacio_libre(sizeof(tareas))){
			offset = get_espacio_libre(sizeof(tareas));
			memcpy(memoria_principal + offset, tareas, sizeof(tareas));
		} else {
			compactar_memoria();
			if(hay_espacio_libre(sizeof(tareas))){
				offset = get_espacio_libre(sizeof(tareas));
				memcpy(memoria_principal + offset, pcb, sizeof(tareas));
			} else {
				//aca hay error, se cancela toodo
			}
		}

	//guarda los tcb
	for(int j = 0; j<mensaje->cant_tripulantes; j++){
		//se fija si hay lugar
		if(hay_espacio_libre(sizeof(t_pcb))){
				offset = get_espacio_libre(sizeof(t_pcb));
				memcpy(memoria_principal + offset, tcb[j], sizeof(t_tcb));
		} else {
			compactar_memoria();
			if(hay_espacio_libre(sizeof(t_pcb))){
				offset = get_espacio_libre(sizeof(t_pcb));
				memcpy(memoria_principal + offset, pcb, sizeof(t_pcb));
			} else {
					//aca hay error, se cancela toodo
			}
		}
	}
	//FALTA PONER EL INICIO EN CADA SEGMENTO EN LA TABLA DE SEGMENTOS

	//si llego hasta aca sin romper nada guardo las tablas

	//aca tengo mis dudas con lo que estoy guardando
	dictionary_put(tablas_seg_patota, string_itoa(mensaje->idPatota), NULL); //le clave un null xq no se q va ahi, no entendi

	//agrego los segmentos a la lista de segmentos ocupados
	list_add_all(segmentos_ocupados, tabla_segmentos_patota);

	//falta modificar la lista segmentos_libres
}

//SIN TERMINAR
void sacar_segmento_lista_libres(segmento *segmento_nuevo){

	uint32_t dir_fisica_segmento_nuevo = obtener_direccion_fisica(segmento_nuevo);

	bool se_encuentra_direccion_fisica(segmento seg, uint32_t direccion_fisica){
		uint32_t dir_fisica_segmento = obtener_direccion_fisica(seg);
		return seg.inicio >= direccion_fisica && dir_fisica_segmento <= direccion_fisica;
	}
	//esto seguro esta mal
	segmento a_modificar = list_filter(segmentos_libres, se_encuentra_direccion_fisica);
}

uint32_t obtener_direccion_fisica(segmento *seg){
	uint32_t inicio = seg->inicio;
	uint32_t tamanio = seg->tamanio;
	return inicio + tamanio;
}

//SIN TERMINAR
int32_t hay_espacio_libre(uint32_t size){
	t_list *lista_auxiliar = filter(segmentos_libres, entra_en_el_segmento(size)); //es *lista_auxiliar?
	return !list_is_empty(lista_auxiliar); //es valido eso sintacticamente?
}

bool entra_en_el_segmento(uint32_t tamanio, segmento seg){
		return seg.tamanio > tamanio;
	}

//SIN TERMINAR
int32_t get_espacio_libre(uint32_t size){
	if(list_is_empty(segmentos_ocupados)){
		return 0;
	}
	else{
		t_list *lista_auxiliar = filter(segmentos_libres, entra_en_el_segmento(size)); //esto no anda (filter)
		if(list_size(lista_auxiliar) == 1){
			segmento seg = list_get(lista_auxiliar, 0);
			return seg.inicio;
		} else {
			return 0; //aca hay que implementar FF y BF
		}

	}
}

/* devuelve una tabla de paginas, solo falta completar el inicio
 * de todos los segmentos y tambien el tamanio de tareas*/

t_list* crear_estructura_tabla_seg(iniciar_patota_msg* mensaje){

	t_list *tabla_segmentos_patota;

	tabla_segmentos_patota = list_create();

	segmento pcb;
	pcb.numero_segmento = 0; 			// el pcb siempre es el segmento 0
	pcb.tamanio = sizeof(t_pcb);

	segmento tareas;
	tareas.numero_segmento = 1;			// las tareas son siempre el segmento 1

	list_add(tabla_segmentos_patota, pcb);
	list_add(tabla_segmentos_patota, tareas);

	segmento tcb[mensaje->cant_tripulantes];
	for(int i = 0; i<mensaje->cant_tripulantes; i++){
		tcb[i].numero_segmento = i + 2;	// los tcb son los segmentos 2 en adelante
		tcb[i].tamanio = sizeof(t_tcb);
		list_add(tabla_segmentos_patota, tcb[i]);
	}

	return tabla_segmentos_patota;
}


//IMPORTANTE
//if(!dictionary_has_key(tablas_seg_patota, string_itoa(mensaje->idPatota))) falta esta validacion



