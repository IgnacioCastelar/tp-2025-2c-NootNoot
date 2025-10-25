#ifndef SERIALIZACIONQUERY_H_
#define SERIALIZACIONQUERY_H_

#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include"conexiones_cliente.h"
#include"serializacion_envio.h"

typedef struct{
    char* path_query;
    uint32_t length_path_query;
    uint8_t prioridad;
}t_paths;

//Se usara el buffer del archivo utils

t_buffer *llenar_buffer_master(t_paths *path);

//void agregar_cadena(t_paquete *paquete, char* cadena);


#endif