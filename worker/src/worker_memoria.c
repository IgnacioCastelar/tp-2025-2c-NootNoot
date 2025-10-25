#include "worker_memoria.h"
#include <stdlib.h>
#include <string.h> // Para strdup, strcmp
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h> // Para manipular nodos directamente si es necesario
#include <stdbool.h>
#include <stdint.h>

// ===================== FUNCIONES AUXILIARES LRU ======================

// Función para mover un nodo (página) al final de la lista LRU
static void _mover_pagina_al_final(t_memoria_worker* cache, t_pagina* pagina_a_mover) {
    // Buscar el nodo en la lista que contiene el puntero a 'pagina_a_mover'
    int index = -1;
    for (int i = 0; i < list_size(cache->paginas); i++) {
        if (list_get(cache->paginas, i) == pagina_a_mover) {
            index = i;
            break;
        }
    }
    if (index != -1) { // Si la página está en la lista
        t_pagina* pagina_movida = list_remove(cache->paginas, index);
        list_add(cache->paginas, pagina_movida); // Agregar al final
        log_debug(cache->logger, "LRU: Página (marco) %ld movida al final de la cola LRU.", (long)(pagina_movida - cache->frames)); // Imprime el índice del marco
    }
}

// Función para obtener la víctima LRU (el primer nodo de la lista)
static t_pagina* _obtener_victima_lru(t_memoria_worker* cache) {
    if (list_is_empty(cache->paginas)) {
        return NULL; // No hay páginas en la cola LRU
    }
    return (t_pagina*)list_get(cache->paginas, 0); // El primer elemento es el LRU
}

// ===================== CREACIÓN Y DESTRUCCIÓN ======================

t_memoria_worker* memoria_crear_cache(int tam_memoria_bytes, int tam_frame_bytes, char* algoritmo, t_log* logger) {
    t_memoria_worker* memoria = malloc(sizeof(t_memoria_worker));
    if (memoria == NULL) {
        log_error(logger, "No se pudo asignar memoria para la estructura t_memoria_worker.");
        return NULL;
    }

    memoria->paginas = list_create(); // Inicializa la lista que servirá como cola LRU
    if (memoria->paginas == NULL) {
        log_error(logger, "No se pudo crear la lista auxiliar de páginas (cola LRU).");
        free(memoria);
        return NULL;
    }

    memoria->max_frames = (tam_frame_bytes > 0) ? (tam_memoria_bytes / tam_frame_bytes) : 0;
    if (memoria->max_frames == 0) {
        log_error(logger, "Tamaño de memoria (%d bytes) es menor que el tamaño de frame (%d bytes).", tam_memoria_bytes, tam_frame_bytes);
        list_destroy(memoria->paginas);
        free(memoria);
        return NULL;
    }

    memoria->frames = calloc(memoria->max_frames, sizeof(t_pagina));
    if (memoria->frames == NULL) {
        log_error(logger, "No se pudo asignar memoria para el arreglo de frames.");
        list_destroy(memoria->paginas);
        free(memoria);
        return NULL;
    }

    memoria->puntero = 0; // Iniciar puntero del reloj (para CLOCK-M)
    memoria->algoritmo = strdup(algoritmo);
    if (memoria->algoritmo == NULL) {
        log_error(logger, "No se pudo duplicar el nombre del algoritmo.");
        free(memoria->frames);
        list_destroy(memoria->paginas);
        free(memoria);
        return NULL;
    }

    memoria->logger = logger;
    pthread_mutex_init(&memoria->mutex, NULL);

    log_info(logger, "Memoria del Worker inicializada: tam=%d bytes, frame=%d bytes, max_frames=%d, algoritmo=%s", tam_memoria_bytes, tam_frame_bytes, memoria->max_frames, algoritmo);
    return memoria;
}

void memoria_destruir_cache(t_memoria_worker* memoria) {
    if (memoria != NULL) {
        if (memoria->frames != NULL) {
            // Liberar contenido de cada frame ocupado
            for (int i = 0; i < memoria->max_frames; i++) {
                if (memoria->frames[i].ocupado && memoria->frames[i].contenido != NULL) {
                    free(memoria->frames[i].contenido);
                }
            }
            free(memoria->frames);
        }
        if (memoria->paginas != NULL) {
            list_destroy(memoria->paginas); // Destruye la lista LRU
        }
        if (memoria->algoritmo != NULL) {
            free(memoria->algoritmo);
        }
        pthread_mutex_destroy(&memoria->mutex);
        free(memoria);
    }
}

// ===================== OPERACIONES ======================

t_pagina* memoria_buscar_bloque(t_memoria_worker* memoria, int id_bloque) {
    pthread_mutex_lock(&memoria->mutex);
    // Buscar en el arreglo de frames si el bloque está presente
    for (int i = 0; i < memoria->max_frames; i++) {
        if (memoria->frames[i].ocupado && memoria->frames[i].id_bloque == id_bloque) {
            // Si se encuentra, para LRU, esta página debe moverse al final de la cola
            if (strcmp(memoria->algoritmo, "LRU") == 0) {
                 _mover_pagina_al_final(memoria, &(memoria->frames[i]));
            }
            pthread_mutex_unlock(&memoria->mutex);
            return &(memoria->frames[i]); // Retorna puntero al frame encontrado
        }
    }
    pthread_mutex_unlock(&memoria->mutex);
    return NULL; // Bloque no encontrado
}

void memoria_agregar_bloque(t_memoria_worker* cache, int id_bloque, void* contenido) {
    pthread_mutex_lock(&cache->mutex);
    // Buscar un marco libre
    for (int i = 0; i < cache->max_frames; i++) {
        if (!cache->frames[i].ocupado) {
            cache->frames[i].id_bloque = id_bloque;
            cache->frames[i].contenido = contenido;
            cache->frames[i].ocupado = true;
            cache->frames[i].bit_uso = true; // Recién cargado, se considera usado
            cache->frames[i].bit_modificado = false; // Recién cargado, no modificado
            // Para LRU: Agregar el puntero a esta página a la cola LRU (lista 'paginas')
            if (strcmp(cache->algoritmo, "LRU") == 0) {
                list_add(cache->paginas, &(cache->frames[i]));
            }
            log_debug(cache->logger, "Bloque %d agregado al marco %d.", id_bloque, i);
            pthread_mutex_unlock(&cache->mutex);
            return;
        }
    }

    // No hay marcos libres, se debe reemplazar
    log_debug(cache->logger, "No hay marcos libres. Intentando reemplazar...");
    memoria_reemplazar_bloque(cache, id_bloque, contenido);
    pthread_mutex_unlock(&cache->mutex);
}

void memoria_marcar_usado(t_memoria_worker* cache, int id_bloque, bool modificado) {
    pthread_mutex_lock(&cache->mutex);
    t_pagina* p = memoria_buscar_bloque(cache, id_bloque);
    if (p) {
        p->bit_uso = true;
        if (modificado) {
            p->bit_modificado = true;
        }
        // Si el algoritmo es LRU, mover la página al final de la cola LRU
        // Nota: memoria_buscar_bloque ya mueve la página si es LRU, por lo tanto,
        // esta línea solo se ejecutará si memoria_marcar_usado se llama
        // independientemente de un acceso de lectura/escritura que ya movió la página.
        // Para evitar doble movimiento, se podría refactorizar para que
        // memoria_buscar_bloque no mueva y que sea memoria_marcar_usado quien lo haga.
        // Pero para mantener la lógica actual, se mantiene aquí también.
        if (strcmp(cache->algoritmo, "LRU") == 0) {
             _mover_pagina_al_final(cache, p);
        }
    }
    pthread_mutex_unlock(&cache->mutex);
}

// ===================== REEMPLAZO Y FLUSH ======================

int memoria_buscar_victima(t_memoria_worker* cache) {
    pthread_mutex_lock(&cache->mutex); // Asegura acceso exclusivo durante la búsqueda

    if (cache->max_frames == 0) {
        log_error(cache->logger, "No hay marcos disponibles en la memoria del Worker.");
        pthread_mutex_unlock(&cache->mutex);
        return -1;
    }

    // --- Lógica del algoritmo de reemplazo basado en 'cache->algoritmo' ---
    if (strcmp(cache->algoritmo, "LRU") == 0) {
        // --- Algoritmo LRU usando la lista 'cache->paginas' como cola ---
        t_pagina* pagina_victima = _obtener_victima_lru(cache);
        if (pagina_victima != NULL) {
            // Encontramos la víctima en la cola LRU, ahora necesitamos su índice en el arreglo 'frames'
            // para retornarlo. La víctima es el primer nodo de la lista.
            int indice_victima = pagina_victima - cache->frames; // Cálculo del índice basado en la dirección
            log_debug(cache->logger, "LRU: Víctima encontrada en marco %d (de la cola LRU).", indice_victima);
            // Importante: Remover la víctima de la cola LRU antes de retornar su índice
            list_remove(cache->paginas, 0); // Remover el primer elemento
            pthread_mutex_unlock(&cache->mutex);
            return indice_victima; // Retornar el índice del marco
        } else {
            log_error(cache->logger, "LRU: No se encontraron marcos ocupados en la cola LRU para reemplazar.");
            pthread_mutex_unlock(&cache->mutex);
            return -1; // No debería pasar si se llama a reemplazo con caché llena
        }
    } else if (strcmp(cache->algoritmo, "CLOCK-M") == 0) {
        // --- Algoritmo CLOCK-M con "dos pasadas" ---
        // Importante: Para CLOCK-M, la lista 'cache->paginas' NO se usa como cola LRU.
        // Se itera sobre el arreglo 'cache->frames' directamente.
        bool segunda_pasada = false; // Bandera para indicar si ya se completó la primera pasada sin éxito
        int vueltas_completadas = 0; // Contador de vueltas completas (para seguridad)

        while (vueltas_completadas < 2) { // Máximo 2 vueltas: la primera y la segunda si es necesario
            // bool victima_encontrada = false; // Variable no usada, comentada o eliminada
            int iteraciones_en_vuelta = 0; // Contador para iteraciones dentro de una vuelta

            // Recorrer el arreglo de marcos desde el puntero actual
            while (iteraciones_en_vuelta < cache->max_frames) {
                t_pagina* p = &(cache->frames[cache->puntero]);

                // Si el marco no está ocupado, no puede ser víctima
                if (!p->ocupado) {
                    cache->puntero = (cache->puntero + 1) % cache->max_frames;
                    iteraciones_en_vuelta++;
                    continue;
                }

                // Algoritmo CLOCK-M: revisa (bit_uso, bit_modificado)
                // Clase 0: (R=0, M=0)
                if (!p->bit_uso && !p->bit_modificado) {
                    int victima_frame = cache->puntero;
                    cache->puntero = (cache->puntero + 1) % cache->max_frames; // Avanzar puntero para la próxima
                    log_debug(cache->logger, "CLOCK-M: Víctima encontrada en marco %d (R=0, M=0).", victima_frame);
                    pthread_mutex_unlock(&cache->mutex); // Desbloquea antes de retornar
                    return victima_frame;
                }
                // Clase 1: (R=0, M=1) - Solo en la "segunda pasada"
                if (!p->bit_uso && p->bit_modificado && segunda_pasada) {
                    int victima_frame = cache->puntero;
                    cache->puntero = (cache->puntero + 1) % cache->max_frames; // Avanzar puntero para la próxima
                    log_debug(cache->logger, "CLOCK-M: Víctima encontrada en marco %d (R=0, M=1) en segunda pasada.", victima_frame);
                    pthread_mutex_unlock(&cache->mutex); // Desbloquea antes de retornar
                    return victima_frame;
                }
                // Clases 2 (R=1, M=0) y 3 (R=1, M=1): Dar segunda oportunidad
                if (p->bit_uso) {
                    p->bit_uso = false; // Quitar el bit de uso
                    log_debug(cache->logger, "CLOCK-M: Segunda oportunidad dada al marco %d (R=1, M=x).", cache->puntero);
                }

                // Avanzar puntero circularmente
                cache->puntero = (cache->puntero + 1) % cache->max_frames;
                iteraciones_en_vuelta++;
            }

            // Si se completó una vuelta completa sin encontrar víctima en la primera pasada
            if (!segunda_pasada) {
                 segunda_pasada = true; // Marcar para iniciar la búsqueda de clase 1 en la próxima vuelta
                 log_debug(cache->logger, "CLOCK-M: Primera pasada completada. Iniciando segunda pasada.");
                 vueltas_completadas++; // Contar la primera vuelta
            } else {
                vueltas_completadas++; // Contar la vuelta (ya sea la primera o la segunda)
            }
        }

        // Si se completaron las 2 vueltas y no se encontró víctima, algo está mal
        log_error(cache->logger, "CLOCK-M: No se encontró víctima para reemplazo después de dos vueltas completas.");
        log_error(cache->logger, "Esto puede suceder si todos los marcos ocupados tienen R=1.");
        pthread_mutex_unlock(&cache->mutex); // Desbloquea antes de retornar
        return -1; // Error o condición imposible de resolver en este contexto
    } else {
        log_error(cache->logger, "Algoritmo de reemplazo desconocido o no implementado: %s", cache->algoritmo);
        pthread_mutex_unlock(&cache->mutex); // Desbloquea antes de retornar
        return -1;
    }
}

void memoria_reemplazar_bloque(t_memoria_worker* memoria, int id_bloque_nuevo, void* contenido_nuevo) {
    // Buscar víctima
    int marco_victima_idx = memoria_buscar_victima(memoria);
    if (marco_victima_idx == -1) {
        log_error(memoria->logger, "No se pudo encontrar una víctima para reemplazar el bloque %d.", id_bloque_nuevo);
        // Opcional: manejar error de forma más robusta
        free(contenido_nuevo); // Liberar contenido nuevo si no se pudo cargar
        return;
    }

    t_pagina* pagina_victima = &(memoria->frames[marco_victima_idx]);

    // Si la víctima está modificada, hacer flush
    if (pagina_victima->bit_modificado) {
        log_debug(memoria->logger, "La víctima en marco %d (Bloque Lógico %d) está modificada. Flusheando antes de reemplazar.", marco_victima_idx, pagina_victima->id_bloque);
        memoria_flush_frame(memoria, pagina_victima);
        // El flush debería haber actualizado bit_modificado a false si fue exitoso
        // En esta implementación directa, lo actualizamos internamente
        pagina_victima->bit_modificado = false;
    }

    // Liberar contenido anterior del frame
    if (pagina_victima->contenido != NULL) {
        free(pagina_victima->contenido);
    }

    // Cargar nuevo bloque en el frame víctima
    pagina_victima->id_bloque = id_bloque_nuevo;
    pagina_victima->contenido = contenido_nuevo;
    pagina_victima->ocupado = true;
    pagina_victima->bit_uso = true; // Recién cargado, se considera usado
    pagina_victima->bit_modificado = false; // Recién cargado, no modificado (hasta que se escriba)

    // Para LRU: Agregar el nuevo bloque (página) a la cola LRU (lista 'paginas')
    if (strcmp(memoria->algoritmo, "LRU") == 0) {
        list_add(memoria->paginas, pagina_victima);
    }

    log_debug(memoria->logger, "Bloque %d cargado en marco reemplazado %d.", id_bloque_nuevo, marco_victima_idx);
}

void memoria_flush_frame(t_memoria_worker* memoria, t_pagina* pagina) {
    // Enviar el contenido del frame a Storage
    // Esta función probablemente necesite información adicional como File:Tag
    // para comunicarse con Storage. Esto implica un diseño más complejo donde
    // la caché conozca File:Tag <-> marco o se maneje en otro lado (worker_query.c).
    // Por ahora, un placeholder.
    // int resultado = enviar_bloque_a_storage(file, tag, pagina->id_bloque, pagina->contenido);
    // if (resultado == 0) {
        // pagina->bit_modificado = false; // Marcar como no modificado después de flush
        log_debug(memoria->logger, "Frame (Bloque Lógico %d) flusheado exitosamente (placeholder).", pagina->id_bloque);
    // } else {
    //     log_error(memoria->logger, "Error al flushear frame (Bloque Lógico %d).", pagina->id_bloque);
    // }
}