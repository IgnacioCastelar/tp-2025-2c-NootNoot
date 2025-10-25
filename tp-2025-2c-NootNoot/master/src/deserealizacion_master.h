#ifndef DESEREALIZACIONMASTER_H_
#define DESEREALIZACIONMASTER_H_

#include <commons/log.h>
#include <commons/config.h>

#include "conexiones_servidor.h"
#include "serializacion_envio.h"
#include "paquete.h"

typedef struct{
    char* path_query;
    uint32_t length_path_query;
    uint8_t prioridad;
}t_paths;


t_paths* recibir_paquete_query(int socket_query);
t_paths* paquete_deserializar(t_buffer* buffer);
void crear_paquete_y_enviar_worker(int socket_cliente, t_buffer *buffer);
t_buffer *llenar_buffer_para_worker(t_paths *path);
//int recibir_cod_op(int socket_query, int cod);
void *recibir_id_worker(int socket_worker);
int paquete_deserealizar_int(t_buffer buffer);


#endif