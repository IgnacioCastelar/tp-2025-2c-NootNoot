#include "operaciones_archivos.h"

//OPERACIONES PARA ARCHIVOS
// Crear un archivo y agregarlo a las colecciones

// Abrir un archivo (buscar por nombre)
t_archivo* abrir_archivo(char* nombre) {
    return dictionary_get(archivos_por_nombre, nombre);
}

// Leer datos del archivo
int leer_archivo(t_archivo* archivo, void* buffer, uint32_t tamanio, uint32_t posicion) {
    if (archivo == NULL || archivo->datos == NULL || posicion + tamanio > archivo->metadata.tamanio)
        return -1;
    memcpy(buffer, archivo->datos + posicion, tamanio);
    return tamanio;
}

// Escribir datos en el archivo
int escribir_archivo(t_archivo* archivo, const void* buffer, uint32_t tamanio, uint32_t posicion) {
    if (archivo == NULL)
        return -1;
    if (archivo->datos == NULL) {
        archivo->datos = malloc(posicion + tamanio);
        archivo->metadata.tamanio = posicion + tamanio;
    } else if (posicion + tamanio > archivo->metadata.tamanio) {
        archivo->datos = realloc(archivo->datos, posicion + tamanio);
        archivo->metadata.tamanio = posicion + tamanio;
    }
    memcpy(archivo->datos + posicion, buffer, tamanio);
    archivo->metadata.fecha_modificacion = time(NULL);
    return tamanio;
}

// Renombrar archivo
int renombrar_archivo(t_archivo* archivo, char* nuevo_nombre) {
    if (archivo == NULL)
        return -1;
    free(archivo->metadata.nombre);
    archivo->metadata.nombre = strdup(nuevo_nombre);
    // Actualizar en el diccionario
    dictionary_remove(archivos_por_nombre, archivo->metadata.nombre);
    dictionary_put(archivos_por_nombre, nuevo_nombre, archivo);
    return 0;
}

// Mover archivo
int mover_archivo(t_archivo* archivo, const char* nueva_ubicacion) {
    if (archivo == NULL)
        return -1;
    free(archivo->metadata.ubicacion);
    archivo->metadata.ubicacion = strdup(nueva_ubicacion);
    archivo->metadata.fecha_modificacion = time(NULL);
    return 0;
}

// Truncar archivo
int truncar_archivo(t_archivo* archivo, uint32_t nuevo_tamanio) {
    if (archivo == NULL)
        return -1;
    archivo->datos = realloc(archivo->datos, nuevo_tamanio);
    archivo->metadata.tamanio = nuevo_tamanio;
    archivo->metadata.fecha_modificacion = time(NULL);
    return 0;
}

// Borrar archivo
int borrar_archivo(char* nombre) {
    t_archivo* archivo = dictionary_remove(archivos_por_nombre, nombre);
    if (archivo == NULL)
        return -1;
    list_remove_element(archivos_storage, archivo);
    free(archivo->metadata.nombre);
    free(archivo->metadata.tipo);
    free(archivo->metadata.permisos);
    free(archivo->metadata.propietario);
    free(archivo->metadata.ubicacion);
    free(archivo->datos);
    free(archivo);
    return 0;
}

// Cerrar archivo (opcional, si manejas archivos abiertos)
int cerrar_archivo(t_archivo* archivo) {
    // Si tienes una estructura de archivos abiertos, aquí lo marcas como cerrado
    return 0;
}

// Reposicionar (puedes manejarlo con una estructura de archivos abiertos y posición)
int reposicionar_archivo(t_archivo* archivo, uint32_t nueva_posicion) {
    // Si tienes una estructura de archivos abiertos, actualiza la posición
    return 0;
}
