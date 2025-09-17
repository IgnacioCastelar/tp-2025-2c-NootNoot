#ifndef PAQUETE_H
#define PAQUETE_H

typedef enum {
    HANDSHAKE,
    MENSAJE,
    PAQUETE,
} op_code;

typedef struct {
    int size;
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