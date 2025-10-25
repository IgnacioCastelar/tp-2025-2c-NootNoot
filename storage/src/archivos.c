#include "archivos.h"

t_archivo* crear_archivo(const char* nombre, const char* tipo, const char* propietario, const char* permisos, const char* ubicacion);

t_archivo* crear_archivo(const char* nombre, const char* tipo, const char* propietario, const char* permisos, const char* ubicacion) {
    t_archivo* archivo = malloc(sizeof(t_archivo));
    archivo->metadata.nombre = strdup(nombre);
    archivo->metadata.tipo = strdup(tipo);
    archivo->metadata.fecha_creacion = time(NULL);
    archivo->metadata.fecha_modificacion = time(NULL);
    archivo->metadata.permisos = strdup(permisos);
    archivo->metadata.propietario = strdup(propietario);
    archivo->metadata.tamanio = 0;
    archivo->metadata.ubicacion = strdup(ubicacion);
    archivo->datos = NULL; // Inicialmente vacío

    list_add(archivos_storage, archivo);
    dictionary_put(archivos_por_nombre, archivo->metadata.nombre, archivo);

    return archivo;
}
