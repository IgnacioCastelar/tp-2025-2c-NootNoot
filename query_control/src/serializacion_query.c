#include <serializacion_query.h>

t_buffer *llenar_buffer_master(t_paths *path)
{
    t_buffer *buffer = buffer_create(
        sizeof(uint8_t) + //prioridad
        sizeof(uint32_t) + path->length_path_query //nombre y la longitud del nombre
    );

    buffer_add_uint8(buffer, path->prioridad);
    buffer_add_string(buffer, path->length_path_query, path->path_query);

    return buffer;
}