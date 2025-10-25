#ifndef PAQUETE_H
#define PAQUETE_H

#include<stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<sys/socket.h>
#include<commons/log.h>
#include<commons/config.h>

typedef enum {
    HANDSHAKE,
    MENSAJE,
    PAQUETE,
    MENSAJE_WORKER,
	MENSAJE_QUERY,
	MENSAJE_STORAGE,
	MENSAJE_MASTER,
	ID_WORKER,
	OP_EXEC_QUERY
} op_code;
typedef struct
{
	uint32_t size;
	uint32_t offset;
	void* stream;
} t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(op_code codigo);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket);
t_paquete* recibir_paquete(int socket);
void destruir_paquete(t_paquete* paquete);

#endif