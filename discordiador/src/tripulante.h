#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include "discordiador.h"

uint32_t ID_TRIPULANTES;
t_list*tripulantes;
t_list*hilosTripulantes;

extern pthread_mutex_t mutex_id_tripulantes;
extern pthread_mutex_t mutex_tripulantes;


extern pthread_mutex_t mutex_listaNuevos;
extern pthread_mutex_t mutex_listaReady;
extern pthread_mutex_t mutex_listaBloqueados;
extern pthread_mutex_t mutex_listaEjecutando;
extern pthread_mutex_t mutex_listaFinalizados;


//EJECUCUION DEL REPARTIDOR
t_list* sem_tripulantes_ejecutar;
sem_t sem_buscartripulanteMasCercano;
sem_t sem_tripulanteMoviendose;


sem_t semaforoInicioCicloCPU; //= malloc(sizeof(sem_t));
sem_t semaforoFinCicloCPU;



typedef enum{

    NEW = 1,
    READY = 2,
    BLOCKED = 3,
    EXEC = 4,
    FINISHED = 5

}t_status_code;


typedef enum{

    FIFO = 1,
    RR = 2

}algoritmo_code;

const static struct {

	algoritmo_code codigo_algoritmo;
	const char* str;
}conversionAlgoritmo[] = {
		{FIFO, "FIFO"},
		{RR, "RR"},

};





void moverAlTripulanteHastaElSabotaje(t_tripulante*);
int llegoAlSabotaje(t_tripulante*);
int distanciaA(t_coordenadas*, t_coordenadas*);
int getIndexTripulanteEnLista(t_list* , t_tripulante* );
void liberarArray(char**);
int cantidadElementosArray(char**);
void sacarTripulanteDeLista(t_tripulante* , t_list* );
uint32_t generar_id_tripulante();

#endif /* TRIPULANTE_H_ */
