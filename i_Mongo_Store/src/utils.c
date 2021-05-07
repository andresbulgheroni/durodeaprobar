#include "utils.h"

//MISC

t_coordenadas* get_coordenadas(char* posicion){

	char** posicionesSplit = string_split(posicion, "|");

	t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));

	coordenadas->posX = atoi(posicionesSplit[0]);
	coordenadas->posY = atoi(posicionesSplit[1]);

	return coordenadas;
}

op_code string_to_op_code (char* string){

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
		}else{
			return ERROR_CODIGO;
		}

}

t_string* get_t_string(char* string){

	t_string* newString = malloc(sizeof(t_string));

	newString->length = strlen(string) + 1;
	newString->string = string;

	return newString;
}

// SERIALIZADO BASE

void serializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset) {

	memcpy(stream + *offset, variable, size);
	*offset += size;

}

void serializar_string(void* stream, t_string* string, uint32_t* offset) {

	serializar_variable(stream, &(string->length),sizeof(string->length), offset);
	serializar_variable(stream, string->string, string->length, offset);

}

t_paquete* crear_paquete_a_serializar(op_code codigo, void* mensaje){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo = codigo;

	if(leer_buffer(codigo)){
		switch(codigo){

			case INICIAR_PATOTA:{
				paquete->buffer = serializar_iniciar_patota_msg(mensaje);
				break;
			}
			case EXPULSAR_TRIPULANTE:{

				paquete->buffer = serializar_expulsar_tripulante_msg(mensaje);

				break;

			}
			case OBTENER_BITACORA:{

				paquete->buffer = serializar_obtener_bitacora_msg(mensaje);

				break;

			}
			case LISTAR_TRIPULANTES_RTA:{

				paquete->buffer = serializar_listar_tripulantes_rta(mensaje);

				break;

			}
			case OBTENER_BITACORA_RTA:{

				paquete->buffer = serializar_obtener_bitacora_rta(mensaje);

				break;

			}
			case INICIAR_TRIPULANTE:{

				paquete->buffer = serializar_iniciar_tripulante_msg(mensaje);

				break;

			}
			case INICIAR_TRIPULANTE_TAREAS:{

				paquete->buffer = serializar_iniciar_tripulante_tareas_msg(mensaje);

				break;

			}
			case SOLICITAR_SIGUIENTE_TAREA:{

				paquete->buffer = serializar_solicitar_siguiente_tarea_msg(mensaje);

				break;

			}
			case SOLICITAR_SIGUIENTE_TAREA_RTA:{

				paquete->buffer = serializar_solicitar_siguiente_tarea_rta(mensaje);

				break;

			}
			case INFORMAR_MOVIMIENTO:{

				paquete->buffer = serializar_informar_movimiento_msg(mensaje);

				break;

			}
			case INICIO_TAREA:{

				paquete->buffer = serializar_inicio_tarea_msg(mensaje);

				break;

			}
			case FIN_TAREA:{

				paquete->buffer = serializar_fin_tarea_msg(mensaje);

				break;

			}
			case ATENDER_SABOTAJE:{

				paquete->buffer = serializar_atender_sabotaje_msg(mensaje);

				break;

			}
			case RESOLUCION_SABOTAJE:{

				paquete->buffer = serializar_resolucion_sabotaje_msg(mensaje);

				break;

			}
			case NOTIFICAR_SABOTAJE:{

				paquete->buffer = serializar_notificar_sabotaje_msg(mensaje);

				break;

			}
			default: break;

		}
	}else{

		paquete->buffer = NULL;

	}

	return paquete;
}

void* serializar_paquete(t_paquete* paquete){

	void* stream = malloc(sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size);

	uint32_t offset = 0;

	serializar_variable(stream, &(paquete->codigo), sizeof(op_code), &offset);

	if(paquete->buffer->size > 0){

		serializar_variable(stream, &(paquete->buffer->size), sizeof(paquete->buffer->size), &offset);
		serializar_variable(stream, paquete->buffer->stream, paquete->buffer->size, &offset);

	}

	return stream;
}

int32_t enviar_paquete(void* mensaje, op_code codigo, uint32_t socketCliente){

	t_paquete* paquete = crear_paquete_a_serializar(codigo, mensaje);
	void* stream  = serializar_paquete(paquete);
	fflush(stdout);
	int32_t status = send(socketCliente, stream, sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size, MSG_NOSIGNAL);

	free(stream);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return status;

}


// DESERIALIZADO BASE

boolean leer_buffer(op_code codigo_operacion){
	return  codigo_operacion != LISTAR_TRIPULANTES && codigo_operacion != INICIAR_PLANIFICACION &&
			codigo_operacion != PAUSAR_PLANIFICACION  && codigo_operacion != ERROR_CODIGO &&
			codigo_operacion != RAM_NECESITA_TAREAS  && codigo_operacion != RAM_NO_NECESITA_TAREAS;
}

void deserializar_variable(void* stream, void* variable, uint32_t size, uint32_t* offset){

	memcpy(variable, stream + *offset, size);
	*offset += size;

}

t_string* deserializar_string(void* stream, uint32_t* offset){

	t_string* stringRespuesta = malloc(sizeof(t_string));

	deserializar_variable(stream, &(stringRespuesta->length), sizeof(uint32_t), offset);
	char* string = malloc(stringRespuesta->length);
	deserializar_variable(stream, string, stringRespuesta->length, offset);

	stringRespuesta->string = string;

	return stringRespuesta;
}

t_paquete* recibir_paquete(uint32_t socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	recv(socket, &(paquete->codigo), sizeof(op_code), MSG_WAITALL);

	if(leer_buffer(paquete->codigo)){

		paquete->buffer = malloc(sizeof(t_buffer));

		recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

	}else{

		paquete->buffer = NULL;

	}

	return paquete;

}

void* deserializar_paquete(t_paquete* paquete){

	void* mensaje;

	switch(paquete->codigo){

		case INICIAR_PATOTA:{

			mensaje = desserializar_iniciar_patota_msg(paquete->buffer->stream);

			break;

		}
		case EXPULSAR_TRIPULANTE:{

			mensaje = desserializar_expulsar_tripulante_msg(paquete->buffer->stream);

			break;

		}
		case OBTENER_BITACORA:{

			mensaje = desserializar_obtener_bitacora_msg(paquete->buffer->stream);

			break;

		}
		case LISTAR_TRIPULANTES_RTA:{

			mensaje = desserializar_listar_tripulantes_rta(paquete->buffer->stream);

			break;

		}
		case OBTENER_BITACORA_RTA:{

			mensaje = desserializar_obtener_bitacora_rta(paquete->buffer->stream);

			break;

		}
		case INICIAR_TRIPULANTE:{

			mensaje = desserializar_iniciar_tripulante_msg(paquete->buffer->stream);

			break;

		}
		case INICIAR_TRIPULANTE_TAREAS:{

			mensaje = desserializar_iniciar_tripulante_tareas_msg(paquete->buffer->stream);

			break;

		}
		case SOLICITAR_SIGUIENTE_TAREA:{

			mensaje = desserializar_solicitar_siguiente_tarea_msg(paquete->buffer->stream);

			break;

		}
		case SOLICITAR_SIGUIENTE_TAREA_RTA:{

			mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete->buffer->stream);

			break;

		}
		case INFORMAR_MOVIMIENTO:{

			mensaje = desserializar_informar_movimiento_msg(paquete->buffer->stream);

			break;

		}
		case INICIO_TAREA:{

			mensaje = desserializar_inicio_tarea_msg(paquete->buffer->stream);

			break;

		}
		case FIN_TAREA:{

			mensaje = desserializar_fin_tarea_msg(paquete->buffer->stream);

			break;

		}
		case ATENDER_SABOTAJE:{

			mensaje = desserializar_atender_abotaje_msg(paquete->buffer->stream);

			break;

		}
		case RESOLUCION_SABOTAJE:{

			mensaje = desserializar_resolucion_sabotaje_msg(paquete->buffer->stream);

			break;

		}
		case NOTIFICAR_SABOTAJE:{

			mensaje = desserializar_notificar_sabotaje_msg(paquete->buffer->stream);

			break;

		}
		case GENERAR_OXIGENO: {
			mensaje = desserializar_notificar_sabotaje_msg(paquete->buffer->stream);

			break;
		}
		default: break;

	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	return mensaje;

}


// SERIALIZADO MENSAJES

t_buffer* serializar_iniciar_patota_msg(iniciar_patota_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t sizeStrings = 0;
	void get_list_items_size(t_string* string){

		sizeStrings += string->length;

	}

	list_iterate(mensaje->listaPosiciones, get_list_items_size);

	buffer->size = sizeof(mensaje->cantidadTripulantes) + sizeof(mensaje->direccionTareas->length)
			+ mensaje->direccionTareas->length + sizeof(uint32_t)
			+ mensaje->cantidadTripulantes * sizeof(uint32_t) + sizeStrings;

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->cantidadTripulantes), sizeof(uint32_t), &offset);
	serializar_string(buffer->stream, mensaje->direccionTareas, &offset);


	uint32_t cant_lista = list_size(mensaje->listaPosiciones);//para tener cuantos deserializar
	serializar_variable(buffer->stream, &cant_lista, sizeof(uint32_t), &offset);

	void serializar_lista(t_string* string){
		serializar_string(buffer->stream, string, &offset);
	}

	list_iterate(mensaje->listaPosiciones, serializar_lista);

	return buffer;

}

t_buffer* serializar_expulsar_tripulante_msg(expulsar_tripulante_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}

t_buffer* serializar_obtener_bitacora_msg(obtener_bitacora_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}

t_buffer* serializar_listar_tripulantes_rta(listar_tripulantes_rta* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t sizeTripulantes = 0;
	void get_list_items_size(tripulante_data_msg* elemento){

		sizeTripulantes += sizeof(elemento->idPatota);
		sizeTripulantes += sizeof(elemento->idTripulante);
		sizeTripulantes += sizeof(elemento->status);
		sizeTripulantes += sizeof(elemento->coordenadas->posX);
		sizeTripulantes += sizeof(elemento->coordenadas->posY);

	}

	list_iterate(mensaje->tripulantes, get_list_items_size);

	buffer->size = sizeTripulantes;
	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;
	uint32_t cantLista = list_size(mensaje->tripulantes);
	serializar_variable(buffer->stream, &cantLista, sizeof(uint32_t), &offset);

	void serializar_lista(tripulante_data_msg* elemento){

		serializar_variable(buffer->stream, &(elemento->idPatota), sizeof(elemento->idPatota), &offset);
		serializar_variable(buffer->stream, &(elemento->idTripulante), sizeof(elemento->idTripulante), &offset);
		serializar_variable(buffer->stream, &(elemento->status), sizeof(elemento->status), &offset);
		serializar_variable(buffer->stream, &(elemento->coordenadas->posX), sizeof(elemento->coordenadas->posX), &offset);
		serializar_variable(buffer->stream, &(elemento->coordenadas->posY), sizeof(elemento->coordenadas->posY), &offset);

	}

	list_iterate(mensaje->tripulantes, serializar_lista);

	return buffer;

}

t_buffer* serializar_obtener_bitacora_rta(obtener_bitacora_rta* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->bitacora->length) + mensaje->bitacora->length ;

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_string(buffer->stream, mensaje->bitacora, &offset);

	return buffer;

}

t_buffer* serializar_iniciar_tripulante_msg(iniciar_tripulante_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}


t_buffer* serializar_iniciar_tripulante_tareas_msg(iniciar_tripulante_tareas_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	uint32_t sizeTareas = 0;

	void get_list_items_size(tarea_data_msg* elemento){

		sizeTareas += sizeof(elemento->nombre->length);
		sizeTareas += elemento->nombre->length;
		sizeTareas += sizeof(elemento->parametros);
		sizeTareas += sizeof(elemento->coordenadas->posX);
		sizeTareas += sizeof(elemento->coordenadas->posY);
		sizeTareas += sizeof(elemento->duracion);

	}

	list_iterate(mensaje->tareas, get_list_items_size);

	buffer->size = sizeTareas;
	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;
	uint32_t cantLista = list_size(mensaje->tareas);
	serializar_variable(buffer->stream, &cantLista, sizeof(uint32_t), &offset);

	void serializar_lista(tarea_data_msg* elemento){

		serializar_string(buffer->stream, elemento->nombre, &offset);
		serializar_variable(buffer->stream, &(elemento->parametros), sizeof(elemento->parametros), &offset);
		serializar_variable(buffer->stream, &(elemento->coordenadas->posX), sizeof(elemento->coordenadas->posX), &offset);
		serializar_variable(buffer->stream, &(elemento->coordenadas->posY), sizeof(elemento->coordenadas->posY), &offset);
		serializar_variable(buffer->stream, &(elemento->duracion), sizeof(elemento->duracion), &offset);

	}

	list_iterate(mensaje->tareas, serializar_lista);

	return buffer;

}


t_buffer* serializar_solicitar_siguiente_tarea_msg(solicitar_siguiente_tarea_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}


t_buffer* serializar_solicitar_siguiente_tarea_rta(solicitar_siguiente_tarea_rta* mensaje){


	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->tarea->nombre->length) + mensaje->tarea->nombre->length
					+ sizeof(mensaje->tarea->parametros) + sizeof(mensaje->tarea->coordenadas->posX)
					+ sizeof(mensaje->tarea->coordenadas->posY) + sizeof(mensaje->tarea->duracion);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_string(buffer->stream,  mensaje->tarea->nombre, &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->parametros), sizeof(mensaje->tarea->parametros), &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->coordenadas->posX), sizeof(mensaje->tarea->coordenadas->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->coordenadas->posY), sizeof(mensaje->tarea->coordenadas->posY), &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->duracion), sizeof(mensaje->tarea->duracion), &offset);

	return buffer;

}


t_buffer* serializar_informar_movimiento_msg(informar_movimiento_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante) +
			       2 * sizeof(mensaje->coordenadasDestino->posX) + 2 * sizeof(mensaje->coordenadasDestino->posX);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasOrigen->posX), sizeof(mensaje->coordenadasOrigen->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasOrigen->posY), sizeof(mensaje->coordenadasOrigen->posY), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posX), sizeof(mensaje->coordenadasDestino->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posY), sizeof(mensaje->coordenadasDestino->posY), &offset);

	return buffer;

}

t_buffer* serializar_inicio_tarea_msg(inicio_tarea_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante) + sizeof(mensaje->nombreTarea->length) + mensaje->nombreTarea->length ;

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_string(buffer->stream, mensaje->nombreTarea, &offset);

	return buffer;

}


t_buffer* serializar_fin_tarea_msg(fin_tarea_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante) + sizeof(mensaje->nombreTarea->length) + mensaje->nombreTarea->length ;

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_string(buffer->stream, mensaje->nombreTarea, &offset);

	return buffer;

}


t_buffer* serializar_atender_sabotaje_msg(atender_sabotaje_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idSabotaje) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return buffer;

}


t_buffer* serializar_resolucion_sabotaje_msg(resolucion_sabotaje_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idSabotaje) + sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return buffer;

}

t_buffer* serializar_notificar_sabotaje_msg(notificar_sabotaje_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idSabotaje);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return buffer;

}

//DESERIALIZDO MENSAJES

iniciar_patota_msg* desserializar_iniciar_patota_msg(void* stream){

	iniciar_patota_msg* mensaje = malloc(sizeof(iniciar_patota_msg));

	uint32_t offset = 0;
	deserializar_variable(stream, &(mensaje->cantidadTripulantes), sizeof(uint32_t), &offset);
	mensaje->direccionTareas = deserializar_string(stream, &offset);
	uint32_t cantLista;
	deserializar_variable(stream, &cantLista, sizeof(uint32_t), &offset);

	mensaje->listaPosiciones =  list_create();
	for(uint32_t i = 0; i < cantLista; i++){
		t_string* elemento = deserializar_string(stream, &offset);

		list_add(mensaje->listaPosiciones, elemento);
	}

	return mensaje;

}

expulsar_tripulante_msg* desserializar_expulsar_tripulante_msg(void* stream){

	expulsar_tripulante_msg* mensaje = malloc(sizeof(expulsar_tripulante_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}

obtener_bitacora_msg* desserializar_obtener_bitacora_msg(void* stream){

	obtener_bitacora_msg* mensaje = malloc(sizeof(obtener_bitacora_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}


listar_tripulantes_rta* desserializar_listar_tripulantes_rta(void* stream){

	listar_tripulantes_rta* mensaje = malloc(sizeof(listar_tripulantes_rta));

	uint32_t offset = 0;
	uint32_t cantLista;
	deserializar_variable(stream, &cantLista, sizeof(uint32_t), &offset);

	mensaje->tripulantes =  list_create();
	for(uint32_t i = 0; i < cantLista; i++){

		tripulante_data_msg* elemento = malloc(sizeof(tripulante_data_msg));
		elemento->coordenadas = malloc(sizeof(t_coordenadas));

		deserializar_variable(stream, &(elemento->idPatota), sizeof(elemento->idPatota), &offset);
		deserializar_variable(stream, &(elemento->idTripulante), sizeof(elemento->idTripulante), &offset);
		deserializar_variable(stream, &(elemento->status), sizeof(elemento->status), &offset);
		deserializar_variable(stream, &(elemento->coordenadas->posX), sizeof(elemento->coordenadas->posX), &offset);
		deserializar_variable(stream, &(elemento->coordenadas->posY), sizeof(elemento->coordenadas->posY), &offset);

		list_add(mensaje->tripulantes, elemento);

	}

	return mensaje;

}

obtener_bitacora_rta* desserializar_obtener_bitacora_rta(void* stream){

	obtener_bitacora_rta* mensaje = malloc(sizeof(obtener_bitacora_rta));

	uint32_t offset = 0;
	mensaje->bitacora = deserializar_string(stream, &offset);

	return mensaje;

}


iniciar_tripulante_msg* desserializar_iniciar_tripulante_msg(void* stream){

	iniciar_tripulante_msg* mensaje = malloc(sizeof(iniciar_tripulante_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}


iniciar_tripulante_tareas_msg* desserializar_iniciar_tripulante_tareas_msg(void* stream){

	iniciar_tripulante_tareas_msg* mensaje = malloc(sizeof(iniciar_tripulante_tareas_msg));

	uint32_t offset = 0;
	uint32_t cantLista;
	deserializar_variable(stream, &cantLista, sizeof(uint32_t), &offset);

	mensaje->tareas =  list_create();
	for(uint32_t i = 0; i < cantLista; i++){

		tarea_data_msg* elemento = malloc(sizeof(tarea_data_msg));
		elemento->coordenadas = malloc(sizeof(t_coordenadas));

		elemento->nombre = deserializar_string(stream, &offset);
		deserializar_variable(stream, &(elemento->parametros), sizeof(elemento->parametros), &offset);
		deserializar_variable(stream, &(elemento->coordenadas->posX), sizeof(elemento->coordenadas->posX), &offset);
		deserializar_variable(stream, &(elemento->coordenadas->posY), sizeof(elemento->coordenadas->posY), &offset);
		deserializar_variable(stream, &(elemento->duracion), sizeof(elemento->duracion), &offset);

		list_add(mensaje->tareas, elemento);

	}

	return mensaje;

}


solicitar_siguiente_tarea_msg* desserializar_solicitar_siguiente_tarea_msg(void* stream){

	solicitar_siguiente_tarea_msg* mensaje = malloc(sizeof(solicitar_siguiente_tarea_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}


solicitar_siguiente_tarea_rta* desserializar_solicitar_siguiente_tarea_rta(void* stream){

	solicitar_siguiente_tarea_rta* mensaje = malloc(sizeof(solicitar_siguiente_tarea_rta));

	uint32_t offset = 0;

	tarea_data_msg* tarea = malloc(sizeof(tarea_data_msg));
	tarea->coordenadas = malloc(sizeof(t_coordenadas));
	mensaje->tarea = tarea;

	tarea->nombre = deserializar_string(stream, &offset);
	deserializar_variable(stream, &(tarea->parametros), sizeof(tarea->parametros), &offset);
	deserializar_variable(stream, &(tarea->coordenadas->posX), sizeof(tarea->coordenadas->posX), &offset);
	deserializar_variable(stream, &(tarea->coordenadas->posY), sizeof(tarea->coordenadas->posY), &offset);
	deserializar_variable(stream, &(tarea->duracion), sizeof(tarea->duracion), &offset);

	return mensaje;

}

informar_movimiento_msg* desserializar_informar_movimiento_msg(void* stream){

	informar_movimiento_msg* mensaje = malloc(sizeof(informar_movimiento_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasOrigen->posX), sizeof(mensaje->coordenadasOrigen->posX), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasOrigen->posY), sizeof(mensaje->coordenadasOrigen->posY), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasDestino->posX), sizeof(mensaje->coordenadasDestino->posX), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasDestino->posY), sizeof(mensaje->coordenadasDestino->posY), &offset);

	return mensaje;

}


inicio_tarea_msg* desserializar_inicio_tarea_msg(void* stream){

	inicio_tarea_msg* mensaje = malloc(sizeof(inicio_tarea_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	mensaje->nombreTarea = deserializar_string(stream, &offset);

	return mensaje;

}


fin_tarea_msg* desserializar_fin_tarea_msg(void* stream){

	fin_tarea_msg* mensaje = malloc(sizeof(fin_tarea_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	mensaje->nombreTarea = deserializar_string(stream, &offset);

	return mensaje;

}

atender_sabotaje_msg* desserializar_atender_abotaje_msg(void* stream){

	atender_sabotaje_msg* mensaje = malloc(sizeof(atender_sabotaje_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return mensaje;

}


resolucion_sabotaje_msg* desserializar_resolucion_sabotaje_msg(void* stream){

	resolucion_sabotaje_msg* mensaje = malloc(sizeof(resolucion_sabotaje_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return mensaje;

}


notificar_sabotaje_msg* desserializar_notificar_sabotaje_msg(void* stream){

	notificar_sabotaje_msg* mensaje = malloc(sizeof(notificar_sabotaje_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);

	return mensaje;

}


//CONEXION

uint32_t iniciar_servidor(char *ip, char *puerto){

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &servinfo);

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(socket_servidor);
			continue;
		}
		break;
	}

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;

}

uint32_t esperar_cliente(uint32_t socketServidor){

	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socketServidor, (struct sockaddr*)&dir_cliente, &tam_direccion);

	return socket_cliente;

}


int crear_conexion(char *ip, char* puerto){

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;

}

void liberar_conexion(uint32_t socketCliente){

	close(socketCliente);

}

