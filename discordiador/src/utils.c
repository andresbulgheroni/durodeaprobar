#include "utils.h"

//MISC

t_coordenadas* get_coordenadas(char* posicion){

	char** posicionesSplit = string_split(posicion, "|");

	t_coordenadas* coordenadas = malloc(sizeof(t_coordenadas));

	coordenadas->posX = atoi(posicionesSplit[0]);
	coordenadas->posY = atoi(posicionesSplit[1]);

	return coordenadas;
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

	switch(codigo){

		case INICIAR_PATOTA_MSG:{
			paquete->buffer = serializar_iniciar_patota_msg(mensaje);
			break;
		}
		case EXPULSAR_TRIPULANTE_MSG:{

			paquete->buffer = serializar_expulsar_tripulante_msg(mensaje);

			break;

		}
		case OBTENER_BITACORA_MSG:{

			paquete->buffer = serializar_obtener_bitacora_msg(mensaje);

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
		case SOLICITAR_SIGUIENTE_TAREA:{

			paquete->buffer = serializar_solicitar_siguiente_tarea_msg(mensaje);

			break;

		}
		case SOLICITAR_SIGUIENTE_TAREA_RTA:{

			paquete->buffer = serializar_solicitar_siguiente_tarea_rta(mensaje);

			break;

		}
		case CAMBIO_ESTADO:{

			paquete->buffer = serializar_cambio_estado_msg(mensaje);

			break;

		}
		case INFORMAR_MOVIMIENTO_RAM:{

			paquete->buffer = serializar_informar_movimiento_ram_msg(mensaje);

			break;

		}
		case INFORMAR_MOVIMIENTO_MONGO:{

			paquete->buffer = serializar_informar_movimiento_mongo_msg(mensaje);

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

int32_t enviar_paquete(void* mensaje, op_code codigo, int32_t socketCliente){

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

t_paquete* recibir_paquete(int32_t socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	recv(socket, &(paquete->codigo), sizeof(op_code), MSG_WAITALL);

	paquete->buffer = malloc(sizeof(t_buffer));

	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

	return paquete;

}

void* deserializar_paquete(t_paquete* paquete){

	void* mensaje;

	switch(paquete->codigo){

		case INICIAR_PATOTA_MSG:{

			mensaje = desserializar_iniciar_patota_msg(paquete->buffer->stream);

			break;

		}
		case EXPULSAR_TRIPULANTE_MSG:{

			mensaje = desserializar_expulsar_tripulante_msg(paquete->buffer->stream);

			break;

		}
		case OBTENER_BITACORA_MSG:{

			mensaje = desserializar_obtener_bitacora_msg(paquete->buffer->stream);

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
		case SOLICITAR_SIGUIENTE_TAREA:{

			mensaje = desserializar_solicitar_siguiente_tarea_msg(paquete->buffer->stream);

			break;

		}
		case SOLICITAR_SIGUIENTE_TAREA_RTA:{

			mensaje = desserializar_solicitar_siguiente_tarea_rta(paquete->buffer->stream);

			break;

		}
		case CAMBIO_ESTADO:{

			mensaje = desserializar_cambio_estado_msg(paquete->buffer->stream);

			break;

		}
		case INFORMAR_MOVIMIENTO_RAM:{

			mensaje = desserializar_informar_movimiento_ram_msg(paquete->buffer->stream);

			break;

		}
		case INFORMAR_MOVIMIENTO_MONGO:{

			mensaje = desserializar_informar_movimiento_mongo_msg(paquete->buffer->stream);

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


	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->tareas->length)
			+ mensaje->tareas->length;

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(uint32_t), &offset);
	serializar_string(buffer->stream, mensaje->tareas, &offset);

	return buffer;

}

t_buffer* serializar_expulsar_tripulante_msg(expulsar_tripulante_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}

t_buffer* serializar_obtener_bitacora_msg(obtener_bitacora_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

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

	buffer->size = sizeof(mensaje->idPatota) + sizeof(mensaje->idTripulante) +
					sizeof(mensaje->coordenadas->posX) + sizeof(mensaje->coordenadas->posY);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadas->posX), sizeof(mensaje->coordenadas->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadas->posY), sizeof(mensaje->coordenadas->posY), &offset);

	return buffer;

}

t_buffer* serializar_solicitar_siguiente_tarea_msg(solicitar_siguiente_tarea_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return buffer;

}

t_buffer* serializar_solicitar_siguiente_tarea_rta(solicitar_siguiente_tarea_rta* mensaje){


	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->tarea->nombre_parametros->length) + mensaje->tarea->nombre_parametros->length
					+ sizeof(mensaje->tarea->coordenadas->posX)
					+ sizeof(mensaje->tarea->coordenadas->posY) + sizeof(mensaje->tarea->duracion);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_string(buffer->stream,  mensaje->tarea->nombre_parametros, &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->coordenadas->posX), sizeof(mensaje->tarea->coordenadas->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->coordenadas->posY), sizeof(mensaje->tarea->coordenadas->posY), &offset);
	serializar_variable(buffer->stream, &(mensaje->tarea->duracion), sizeof(mensaje->tarea->duracion), &offset);

	return buffer;

}

t_buffer* serializar_cambio_estado_msg(cambio_estado_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante) + sizeof(mensaje->estado);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->estado), sizeof(mensaje->estado), &offset);

	return buffer;

}


t_buffer* serializar_informar_movimiento_ram_msg(informar_movimiento_ram_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante) + sizeof(mensaje->coordenadasDestino->posX)
			+ sizeof(mensaje->coordenadasDestino->posX);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posX), sizeof(mensaje->coordenadasDestino->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posY), sizeof(mensaje->coordenadasDestino->posY), &offset);

	return buffer;

}

t_buffer* serializar_informar_movimiento_mongo_msg(informar_movimiento_mongo_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(mensaje->idTripulante) +2 * sizeof(mensaje->coordenadasDestino->posX)
			+ 2 * sizeof(mensaje->coordenadasDestino->posX);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasOrigen->posX), sizeof(mensaje->coordenadasOrigen->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasOrigen->posY), sizeof(mensaje->coordenadasOrigen->posY), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posX), sizeof(mensaje->coordenadasDestino->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadasDestino->posY), sizeof(mensaje->coordenadasDestino->posY), &offset);

	return buffer;

}

t_buffer* serializar_inicio_tarea_msg(inicio_tarea_msg* mensaje){

	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = 	sizeof(mensaje->idTripulante) + sizeof(mensaje->nombreTarea->length) + mensaje->nombreTarea->length
				 	+ sizeof(mensaje->parametros);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	serializar_string(buffer->stream, mensaje->nombreTarea, &offset);
	serializar_variable(buffer->stream, mensaje->parametros, sizeof(mensaje->parametros), &offset);

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

	buffer->size = sizeof(mensaje->idSabotaje) + sizeof(mensaje->coordenadas->posX) + sizeof(mensaje->coordenadas->posY);

	buffer->stream = malloc(buffer->size);

	uint32_t offset = 0;

	serializar_variable(buffer->stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadas->posX), sizeof(mensaje->coordenadas->posX), &offset);
	serializar_variable(buffer->stream, &(mensaje->coordenadas->posY), sizeof(mensaje->coordenadas->posY), &offset);

	return buffer;

}

//DESERIALIZDO MENSAJES

iniciar_patota_msg* desserializar_iniciar_patota_msg(void* stream){

	iniciar_patota_msg* mensaje = malloc(sizeof(iniciar_patota_msg));

	uint32_t offset = 0;
	deserializar_variable(stream, &(mensaje->idPatota), sizeof(uint32_t), &offset);
	mensaje->tareas = deserializar_string(stream, &offset);

	return mensaje;

}

expulsar_tripulante_msg* desserializar_expulsar_tripulante_msg(void* stream){

	expulsar_tripulante_msg* mensaje = malloc(sizeof(expulsar_tripulante_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}

obtener_bitacora_msg* desserializar_obtener_bitacora_msg(void* stream){

	obtener_bitacora_msg* mensaje = malloc(sizeof(obtener_bitacora_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

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
	mensaje->coordenadas = malloc(sizeof(t_coordenadas));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idPatota), sizeof(mensaje->idPatota), &offset);
	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->coordenadas->posX), sizeof(mensaje->coordenadas->posX), &offset);
	deserializar_variable(stream, &(mensaje->coordenadas->posY), sizeof(mensaje->coordenadas->posY), &offset);

	return mensaje;

}

solicitar_siguiente_tarea_msg* desserializar_solicitar_siguiente_tarea_msg(void* stream){

	solicitar_siguiente_tarea_msg* mensaje = malloc(sizeof(solicitar_siguiente_tarea_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);

	return mensaje;

}


solicitar_siguiente_tarea_rta* desserializar_solicitar_siguiente_tarea_rta(void* stream){

	solicitar_siguiente_tarea_rta* mensaje = malloc(sizeof(solicitar_siguiente_tarea_rta));

	uint32_t offset = 0;

	tarea_data_msg* tarea = malloc(sizeof(tarea_data_msg));
	tarea->coordenadas = malloc(sizeof(t_coordenadas));
	mensaje->tarea = tarea;

	tarea->nombre_parametros = deserializar_string(stream, &offset);
	deserializar_variable(stream, &(tarea->coordenadas->posX), sizeof(tarea->coordenadas->posX), &offset);
	deserializar_variable(stream, &(tarea->coordenadas->posY), sizeof(tarea->coordenadas->posY), &offset);
	deserializar_variable(stream, &(tarea->duracion), sizeof(tarea->duracion), &offset);

	return mensaje;

}

cambio_estado_msg* desserializar_cambio_estado_msg(void* stream){

	cambio_estado_msg* mensaje = malloc(sizeof(cambio_estado_msg));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->estado), sizeof(mensaje->estado), &offset);

	return mensaje;

}


informar_movimiento_ram_msg* desserializar_informar_movimiento_ram_msg(void* stream){

	informar_movimiento_ram_msg* mensaje = malloc(sizeof(informar_movimiento_ram_msg));
	mensaje->coordenadasDestino = malloc(sizeof(t_coordenadas));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idTripulante), sizeof(mensaje->idTripulante), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasDestino->posX), sizeof(mensaje->coordenadasDestino->posX), &offset);
	deserializar_variable(stream, &(mensaje->coordenadasDestino->posY), sizeof(mensaje->coordenadasDestino->posY), &offset);

	return mensaje;

}

informar_movimiento_mongo_msg* desserializar_informar_movimiento_mongo_msg(void* stream){

	informar_movimiento_mongo_msg* mensaje = malloc(sizeof(informar_movimiento_mongo_msg));
	mensaje->coordenadasOrigen = malloc(sizeof(t_coordenadas));
	mensaje->coordenadasDestino = malloc(sizeof(t_coordenadas));

	uint32_t offset = 0;

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
	deserializar_variable(stream, &(mensaje->parametros), sizeof(mensaje->parametros), &offset);

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
	mensaje->coordenadas = malloc(sizeof(t_coordenadas));

	uint32_t offset = 0;

	deserializar_variable(stream, &(mensaje->idSabotaje), sizeof(mensaje->idSabotaje), &offset);
	deserializar_variable(stream, &(mensaje->coordenadas->posX), sizeof(mensaje->coordenadas->posX), &offset);
	deserializar_variable(stream, &(mensaje->coordenadas->posY), sizeof(mensaje->coordenadas->posY), &offset);

	return mensaje;

}


//CONEXION

int32_t iniciar_servidor(char *ip, char *puerto){

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

int32_t esperar_cliente(int32_t socketServidor){

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