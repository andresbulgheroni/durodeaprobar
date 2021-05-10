#include "tripulante.h"

///////////////////////////////////////FUNCIONES GENERALES///////////////

int cantidadElementosArray(char** array)
{
	int i = 0;
	while(array[i])
	{
		i++;
	}
	return i;
}

void liberarArray(char** array)
{
	for(int i = 0; i < cantidadElementosArray(array); i++)
		free(array[i]);
	free(array);
}

void sacarTripulanteDeLista(t_tripulante* tripulante, t_list* lista){
	int a = list_size(lista);
	for(int i=0; i<a ; i++){
		t_tripulante* tripulanteDeLista = list_get(lista, i);
		if(tripulante->idTripulante == tripulanteDeLista->idTripulante){
			list_remove(lista, i);
			break;
		}
	}
}




int distanciaA(t_coordenadas* desde, t_coordenadas* hasta){

	if (desde == NULL || hasta == NULL) {
		return -1;
	}

	int distanciaX = abs(desde->posX - hasta->posX);
	int distanciaY = abs(desde->posY - hasta->posY);

	return distanciaX + distanciaY;

}

int llegoAlSabotaje(t_tripulante* tripulante){

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXsabotaje = tripulante->sabotaje->coordenadas->posX;
	uint32_t posicionYsabotaje = tripulante->sabotaje->coordenadas->posY;

	return (posicionXtripulante == posicionXsabotaje) && (posicionYtripulante == posicionYsabotaje);
}

void moverAlTripulanteHastaElSabotaje(t_tripulante*tripulante){


	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg* mensajeMovimientoSabotaje=malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoSabotaje->idTripulante = tripulante->idTripulante;

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXsabotaje = tripulante->sabotaje->coordenadas->posX;
	uint32_t posicionYsabotaje = tripulante->sabotaje->coordenadas->posY;

	if (posicionXtripulante != posicionXsabotaje) {

		int diferenciaEnX = posicionXsabotaje - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYsabotaje) {

		int diferenciaEnY = posicionYsabotaje - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}

	mensajeMovimientoSabotaje->coordenadasDestino=tripulante->coordenadas;

	tripulante->misCiclosDeCPU++;
	enviar_paquete(mensajeMovimientoSabotaje,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);


}

int llegoATarea(t_tripulante* tripulante){

	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	uint32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	return (posicionXtripulante == posicionXtarea) && (posicionYtripulante == posicionYtarea);
}

void moverAlTripulanteHastaLaTarea(t_tripulante*tripulante){


	sleep(RETARDO_CICLO_CPU);
	informar_movimiento_ram_msg*mensajeMovimientoTarea = malloc(sizeof(informar_movimiento_ram_msg));
	mensajeMovimientoTarea->idTripulante = tripulante->idTripulante;


	uint32_t posicionXtripulante = tripulante->coordenadas->posX;
	uint32_t posicionYtripulante = tripulante->coordenadas->posY;

	uint32_t posicionXtarea = tripulante->tareaAsignada->coordenadas->posX;
	uint32_t posicionYtarea = tripulante->tareaAsignada->coordenadas->posY;

	if (posicionXtripulante != posicionXtarea) {

		int diferenciaEnX = posicionXtarea - posicionXtripulante;
		if (diferenciaEnX > 0) {
			tripulante->coordenadas->posX = posicionXtripulante + 1;
		} else if (diferenciaEnX < 0) {
			tripulante->coordenadas->posX = posicionXtripulante - 1;
		}

	} else if (posicionYtripulante != posicionYtarea) {

		int diferenciaEnY = posicionYtarea - posicionYtripulante;
		if (diferenciaEnY > 0) {
			tripulante->coordenadas->posY = posicionYtripulante + 1;
		} else if (diferenciaEnY < 0) {
			tripulante->coordenadas->posY = posicionYtripulante - 1;
		}

	}
	mensajeMovimientoTarea->coordenadasDestino=tripulante->coordenadas;

	tripulante->misCiclosDeCPU++;
	enviar_paquete(mensajeMovimientoTarea,INFORMAR_MOVIMIENTO_RAM,tripulante->socketTripulanteRam);
	tripulante->misCiclosDeCPU++;

}


t_tripulante* tripulanteMasCercanoDelSabotaje(t_sabotaje* sabotaje){
	t_tripulante* tripulanteTemporal;

	int distanciaTemporal;


	pthread_mutex_lock(&mutex_listaNuevos);
	t_list* tripulantes_new = list_duplicate(listaNuevos);
	pthread_mutex_unlock(&mutex_listaNuevos);

	t_tripulante* tripulanteMasCercanoNew;
	int menorDistanciaNew = 1000;

	if(!list_is_empty(tripulantes_new)){
		tripulanteMasCercanoNew = list_get(tripulantes_new, 0);
		menorDistanciaNew = distanciaA(tripulanteMasCercanoNew->coordenadas, sabotaje->coordenadas);


		for(int i = 1; i < tripulantes_new->elements_count; i++){

			if(menorDistanciaNew == 0){
				break;
			}

			tripulanteTemporal = list_get(tripulantes_new, i);
			distanciaTemporal = distanciaA(tripulanteTemporal->coordenadas, sabotaje->coordenadas);

			if(distanciaTemporal < menorDistanciaNew){
				tripulanteMasCercanoNew = tripulanteTemporal;
				menorDistanciaNew = distanciaTemporal;
			}

		}
	}

	list_destroy(tripulantes_new);


		pthread_mutex_lock(&mutex_listaNuevos);
		sacarTripulanteDeLista(tripulanteMasCercanoNew, listaNuevos);
		pthread_mutex_unlock(&mutex_listaNuevos);

		return tripulanteMasCercanoNew;
}
