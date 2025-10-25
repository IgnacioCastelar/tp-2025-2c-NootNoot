#ifndef PLANIFICADORQUERYS_H_
#define PLANIFICADORQUERYS_H_

#include<commons/log.h>
#include<commons/config.h>

#include"conexiones_servidor.h"
#include<deserealizacion_master.h>

extern t_config *config;

typedef enum{
    READY,
    EXEC,
    EXIT
}t_estado_query;

typedef struct{
    char* path_query;
    int id_query;
    t_estado_query estado_actual;
    //t_list* querys;
    int priorirdad;
}t_propQuery;

void planificador_corto_plazo(char *path_query);


#endif