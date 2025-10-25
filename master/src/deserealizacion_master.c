#include <deserealizacion_master.h>

t_paths *recibir_paquete_query(int socket_query)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    //recv(socket_query, &(paquete->codigo_operacion), sizeof(int), 0);

    recv(socket_query, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);

    recv(socket_query, paquete->buffer->stream, paquete->buffer->size, 0);

    t_paths *path = paquete_deserializar(paquete->buffer);

    return path;
}

t_paths *paquete_deserializar(t_buffer *buffer)
{
    t_paths *path = malloc(sizeof(t_paths));
    
    void *stream = buffer->stream;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&(path->prioridad), stream, sizeof(uint8_t));
    stream += sizeof(uint8_t);

    memcpy(&(path->length_path_query), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    path->path_query = malloc(path->length_path_query);
    memcpy(path->path_query, stream, path->length_path_query);

    return path;
}


t_buffer *llenar_buffer_para_worker(t_paths *path)
{
    t_buffer *buffer = buffer_create(
        sizeof(uint8_t) + //prioridad
        sizeof(uint32_t) + path->length_path_query //nombre y la longitud del nombre
    );

    buffer_add_uint8(buffer, path->prioridad);
    buffer_add_string(buffer, path->length_path_query, path->path_query);

    return buffer;
}

void crear_paquete_y_enviar_worker(int socket_cliente, t_buffer *buffer) // se saco el parametro codigo de operacion
{
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = PAQUETE;
    paquete->buffer = buffer;

    // Armamos el stream a enviar
    void *a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));

    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    // Por último enviamos
    send(socket_cliente, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void *recibir_id_worker(int socket_worker)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    int result_error = -1;
    int result_ok = 0;

    recv(socket_worker, &(paquete->codigo_operacion), sizeof(int), 0); //esto se volvio a descomentar -- en revision si volverlo a comentar

    recv(socket_worker, &(paquete->buffer->size), sizeof(uint32_t), 0);
    paquete->buffer->stream = malloc(paquete->buffer->size);

    recv(socket_worker, paquete->buffer->stream, paquete->buffer->size, 0);

    if(paquete == NULL)
    {
        log_error(logger, "Error al recibir handshake de CPU");
		send(socket_worker, &result_error, sizeof(int), 0);
		return NULL;
    }
    if(paquete->codigo_operacion != ID_WORKER)
    {
        log_error(logger, "Código de operación desconocido para recibir el identificador de CPU");
		send(socket_worker, &result_error, sizeof(int), 0);
		eliminar_paquete(paquete);
		return NULL;
    }

    //t_paths *path = paquete_deserializar(paquete->buffer);
    int id_cpu = paquete_deserealizar_int(paquete->buffer);

    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return id_cpu;
}

int paquete_deserealizar_int(t_buffer buffer)
{
    int id;
    
    void *stream = buffer->stream;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&id, stream, sizeof(int));
    stream += sizeof(int);

    return id;
}