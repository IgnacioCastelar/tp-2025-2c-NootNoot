#ifndef WORKER_MEMORIA_H
#define WORKER_MEMORIA_H

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdbool.h>
#include <pthread.h>

/* Estructura que representa un bloque/página en la memoria del Worker*/
// Mantenemos la estructura original del TP actual
typedef struct {
    int id_bloque;
    bool ocupado; // Indica si el marco está en uso
    bool bit_uso; // Bit de uso (R) - Usado por CLOCK-M
    bool bit_modificado; // Bit de modificado (M/D) - Usado por CLOCK-M
    void* contenido;
    // No se agrega timestamp, next ni prev aquí.
    // La cola LRU se manejará con la t_list* paginas externa.
} t_pagina;

/* Estructura que representa la memoria caché del Worker*/
// Mantenemos la estructura original, pero ahora sabemos que 'paginas' se usará como cola LRU
typedef struct {
    t_list* paginas; // Lista que se usará como cola LRU para LRU. Contiene punteros a t_pagina dentro de 'frames'.
    int max_frames;  // Cantidad total de marcos
    t_pagina* frames; // Arreglo fijo de marcos
    int puntero;      // Puntero para el algoritmo CLOCK-M
    pthread_mutex_t mutex; // Mutex para acceso concurrente
    t_log* logger;      // Logger
    char* algoritmo;    // "LRU", "CLOCK-M"
} t_memoria_worker;

/* Creación y destrucción */
t_memoria_worker* memoria_crear_cache(int tam_memoria_bytes, int tam_frame_bytes, char* algoritmo, t_log* logger);
void memoria_destruir_cache(t_memoria_worker* memoria);

/* Operaciones */
t_pagina* memoria_buscar_bloque(t_memoria_worker* memoria, int id_bloque);
void memoria_agregar_bloque(t_memoria_worker* memoria, int id_bloque, void* contenido);
void memoria_marcar_usado(t_memoria_worker* memoria, int id_bloque, bool modificado);

/* Reemplazo y flush */
int memoria_buscar_victima(t_memoria_worker* memoria);
void memoria_reemplazar_bloque(t_memoria_worker* memoria, int id_bloque, void* contenido);
void memoria_flush_frame(t_memoria_worker* memoria, t_pagina* pagina);

#endif // WORKER_MEMORIA_H