#ifndef OPERACIONES_ARCHIVOS_H
#define OPERACIONES_ARCHIVOS_H

#include "main_storage.h"
#include "archivos.h"



t_archivo* abrir_archivo(char* nombre);
int leer_archivo(t_archivo* archivo, void* buffer, uint32_t tamanio, uint32_t posicion);
int escribir_archivo(t_archivo* archivo, const void* buffer, uint32_t tamanio, uint32_t posicion);
int renombrar_archivo(t_archivo* archivo, char* nuevo_nombre);
int mover_archivo(t_archivo* archivo, const char* nueva_ubicacion);
int truncar_archivo(t_archivo* archivo, uint32_t nuevo_tamanio);
int borrar_archivo(char* nombre);
int cerrar_archivo(t_archivo* archivo);
int reposicionar_archivo(t_archivo* archivo, uint32_t nueva_posicion);

#endif //OPERACIONES_ARCHIVOS_H

