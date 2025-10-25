#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include <time.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <string.h>
t_list* archivos_storage;
//-------ESTRUCTURAS DE DATOS PARA ARCHIVOS
t_dictionary* archivos_por_nombre;
typedef struct {
    char* nombre;
    char* tipo;               // Ej: "txt", "bin", etc.
    time_t fecha_creacion;
    time_t fecha_modificacion;
    char* permisos;           // Ej: "rwxr-xr-x"
    char* propietario;        // Usuario dueño del archivo
    uint32_t tamanio;         // Tamaño en bytes
    char* ubicacion;          // Ruta o identificador de ubicación en Storage
} t_archivo_metadata;

typedef struct {
    t_archivo_metadata metadata;
    void* datos;              // Puntero a los datos del archivo (puede ser array de bloques, etc.)
} t_archivo;

t_archivo* crear_archivo(const char* nombre, const char* tipo, const char* propietario, const char* permisos, const char* ubicacion);


#endif //ARCHIVOS_H