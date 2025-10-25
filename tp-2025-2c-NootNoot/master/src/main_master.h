#ifndef MAINMASTER_H_
#define MAINMASTER_H_

#include<commons/log.h>
#include<commons/config.h>
#include"conexiones_servidor.h"
#include<deserealizacion_master.h>
#include<pthread.h>
#include<planificador_querys.h>



typedef struct{
    int id_cpu;
    bool libre;
    bool cerrada;
}t_cpu_conectada;

void *atender_conexiones_cliente(void* args_conexion);
void *atender_querys(void* socket_server);
void *atender_worker(void *socket_server);
void iniciar_conexiones(int socket_server);

//void iterator(char* value);

#endif