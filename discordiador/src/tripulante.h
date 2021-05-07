#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include "discordiador.h"

//EJECUCUION DEL REPARTIDOR

sem_t sem_buscartripulanteMasCercano;
sem_t sem_tripulanteMoviendose;

sem_t semaforoInicioCicloCPU; //= malloc(sizeof(sem_t));
sem_t semaforoFinCicloCPU;



void moverAlTripulanteHastaElSabotaje(t_tripulante*);
int llegoAlSabotaje(t_tripulante*);
int distanciaA(t_coordenadas*, t_coordenadas*);
int getIndexTripulanteEnLista(t_list* , t_tripulante* );
void liberarArray(char**);
int cantidadElementosArray(char**);
void sacarTripulanteDeLista(t_tripulante* , t_list* );
void moverAlTripulanteHastaLaTarea(t_tripulante*);
int llegoATarea(t_tripulante*);
t_tripulante* tripulanteMasCercanoDelSabotaje(t_sabotaje*);
#endif /* TRIPULANTE_H_ */
