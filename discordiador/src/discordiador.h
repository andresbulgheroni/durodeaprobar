#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/log.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<pthread.h>
#include<semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>

extern pthread_mutex_t mutex_tripulantes;
extern pthread_mutex_t mutex_sockets;

extern pthread_mutex_t mutex_listaNuevos;
extern pthread_mutex_t mutex_listaReady;
extern pthread_mutex_t mutex_listaBloqueados;
extern pthread_mutex_t mutex_listaBloqueadosPorSabotaje;
extern pthread_mutex_t mutex_listaEjecutando;
extern pthread_mutex_t mutex_listaFinalizados;

typedef enum{

	//CONSOLA
	INICIAR_PATOTA= 1, // MANDA DATA
	LISTAR_TRIPULANTES= 2,
	EXPULSAR_TRIPULANTE = 3, //MANDA DATA
	INICIAR_PLANIFICACION = 4,
	PAUSAR_PLANIFICACION = 5,
	OBTENER_BITACORA=6, // MANDA DATA
	ERROR_CODIGO_CONSOLA=7

} op_code_consola;

t_log* logger;

t_config*config;

uint32_t estaPlanificando;
uint32_t haySabotaje;

char*IP_MI_RAM_HQ;
char*PUERTO_MI_RAM_HQ;
char*IP_I_MONGO_STORE;
char*PUERTO_I_MONGO_STORE;
uint32_t GRADO_MULTITAREA;
char*ALGORITMO;
uint32_t QUANTUM;
uint32_t DURACION_SABOTAJE;
uint32_t RETARDO_CICLO_CPU;

sem_t sem_planificarMultitarea;
sem_t sem_pausarPlanificacion;
sem_t sem_sabotaje;

sem_t semaforoInicioCicloBloqueado;






sem_t sem_hiloTripulante;
t_list* tripulantes;
uint32_t numeroHiloTripulante;


t_list* listaNuevos;
t_list* listaReady;
t_list* listaBloqueados;
t_list*listaBloqueadosPorSabotaje;
t_list* listaFinalizados;
t_list* sem_tripulantes_ejecutar;
t_list*listaEjecutando;


sem_t sem_buscartripulanteMasCercano;
//sem_t sem_tripulanteMoviendose;

sem_t semaforoInicioCicloCPU; //= malloc(sizeof(sem_t));
sem_t semaforoFinCicloCPU;

typedef enum{

    FIFO = 1,
    RR = 2,
	ERROR_CODIGO_ALGORITMO = 3

}algoritmo_code;

typedef enum{
	GENERAR_OXIGENO = 22,
	CONSUMIR_OXIGENO = 23,
	GENERAR_COMIDA = 24,
	CONSUMIR_COMIDA = 25,
	GENERAR_BASURA = 26,
	DESCARTAR_BASURA = 27,
	TAREA_CPU=28
}op_code_tareas;

const static struct {

	algoritmo_code codigo_algoritmo;
	const char* str;
}conversionAlgoritmo[] = {
		{FIFO, "FIFO"},
		{RR, "RR"},

};

typedef struct{
	uint32_t cantPatota;
	uint32_t idPatota;
	t_list* tripulantes;
	char* tareas;		//TODO

} t_patota;

typedef struct
{
	uint32_t id_sabotaje;		//TODO
	t_coordenadas* coordenadas;

} t_sabotaje;

typedef struct
{
	char* nombreTarea;		//TODO
	t_coordenadas* coordenadas;
	uint32_t duracion;
	uint32_t parametros;
	bool finalizoTarea;

} t_tarea;

typedef struct
{
	uint32_t idTripulante;
	uint32_t idPatota;
	t_coordenadas* coordenadas;
	t_status_code estado;
	uint32_t misCiclosDeCPU;
	t_tarea* tareaAsignada;
	uint32_t quantumDisponible;
	uint32_t socketTripulanteRam;
	uint32_t socketTripulanteImongo;

	uint32_t fueExpulsado; //1 o 0

	sem_t*semaforoCiclo;
	sem_t*semaforoBloqueadoTripulante;
	sem_t*semaforoDelTripulante;
	sem_t*semaforoDelSabotaje;

} t_tripulante;


//Inicializacion
void iniciarLog();
void inicializarListasGlobales();
void inicializarSemaforoPlanificador();
void crearHiloPlanificador();
void leerConfig(char*);

//Consola
void leer_consola();
//void pausarPlanificacion();
op_code_consola string_to_op_code_consola (char*);
op_code_tareas string_to_op_code_tareas (char*);




//TRIPULANTES
void liberarArray(char**);
uint32_t cantidadElementosArray(char**);

void sacarTripulanteDeLista(t_tripulante* , t_list* );

char* convertirEnumAString (t_status_code);
void inicializarAtributosATripulante(t_list*);
void crearHilosTripulantes();
void ejecutarTripulante(t_tripulante*);
void enviarMensajeDeInicioDeTripulante(t_tripulante*);
void agregarTripulanteAListaReadyYAvisar(t_tripulante*);
void agregarTripulanteAListaExecYAvisar(t_tripulante*);
void agregarTripulanteAListaBloqueadosYAvisar(t_tripulante*);
void agregarTripulanteAListaBloqueadosPorSabotajeYAvisar(t_tripulante*);
void agregarTripulanteAListaFinishedYAvisar(t_tripulante*);
uint32_t getIndexTripulanteEnLista(t_list* , t_tripulante* );
void log_movimiento_tripulante(uint32_t, uint32_t, uint32_t);
void log_tripulante_cambio_de_cola_planificacion(uint32_t, char*, char*);
void planificarBloqueo();
void inicioHiloPlanificarBloqueo();


void moverAlTripulanteHastaLaTarea(t_tripulante*);
int32_t llegoATarea(t_tripulante*);
uint32_t distanciaA(t_coordenadas*, t_coordenadas*);

//SABOTAJE
t_tripulante* tripulanteMasCercanoDelSabotaje(t_sabotaje*);
void moverAlTripulanteHastaElSabotaje(t_tripulante*,t_sabotaje*);
uint32_t llegoAlSabotaje(t_tripulante*,t_sabotaje*);

//PLANIFICACION
algoritmo_code stringACodigoAlgoritmo(const char* );
void planificarSegun();
void planificarSegunFIFO();

void ejecucionDeTareaTripulanteFIFO(t_tripulante*tripulante);

void ejecucionRR(t_tripulante*);
void ejecucionDeTareaTripulanteRR(t_tripulante*);



//SABOTAJE

bool ordenarTripulantesDeMenorIdAMayor(void* ,void*);
void pasarATodosLosTripulantesAListaBloqueado();
void pasarAEjecutarAlTripulanteMasCercano(t_sabotaje*,t_tripulante*);
void pasarTripulantesAListaReady();
void iniciarHiloSabotaje();
void planificarSabotaje();


//COSAS QUE NOSE SI VAN
void iniciarHiloRETARDO_CICLO_CPU();
void sincronizarRETARDO_CICLO_CPU();


void finalizar();
#endif /* DISCORDIADOR_H_ */
