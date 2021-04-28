#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include "discordiador.h"

//EJECUCUION DEL REPARTIDOR
t_list* sem_tripulantes_ejecutar;
sem_t sem_buscartripulanteMasCercano;
sem_t sem_tripulanteMoviendose;

sem_t semaforoInicioCicloCPU; //= malloc(sizeof(sem_t));
sem_t semaforoFinCicloCPU;

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

#endif /* TRIPULANTE_H_ */
