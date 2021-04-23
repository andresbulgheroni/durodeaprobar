#include "utils.h"

void serializarVariable(void* stream, void* variable, uint32_t size, uint32_t* offset) {

	memcpy(stream + *offset, variable, size);
	*offset += size;

}

void serializarString(void* stream, tString* string, uint32_t* offset) {

	serializarVariable(stream, &(string->length),sizeof(string->length), offset);
	serializarVariable(stream, string->string, string->length, offset);

}

void deserializarVariable(void* stream, void* variable, uint32_t size, uint32_t* offset){

	memcpy(variable, stream + *offset, size);
	*offset += size;

}

tString* deserializarString(void* stream, uint32_t* offset){

	tString* stringRespuesta = malloc(sizeof(tString));

	deserializarVariable(stream, &(stringRespuesta->length), sizeof(uint32_t), offset);
	char* string = malloc(stringRespuesta->length);
	deserializarVariable(stream, string, stringRespuesta->length, offset);

	stringRespuesta->string = string;

	return stringRespuesta;
}

opCode stringToOpCode (char* string){

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

uint32_t iniciarServidor(char *ip, char *puerto){

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

uint32_t esperarCliente(uint32_t socketServidor){

	struct sockaddr_in dir_cliente;

	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socketServidor, (struct sockaddr*)&dir_cliente, &tam_direccion);

	return socket_cliente;

}


int crearConexion(char *ip, char* puerto){

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

void liberaConexion(uint32_t socketCliente){

	close(socketCliente);

}
