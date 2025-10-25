#include "worker_query.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/*static void worker_mem_simulate_access(t_worker* worker, const char* dir, const t_instruccion* instr) {
    if (!worker || !worker->config) return;
    int retardo = worker->config->retardo_memoria;
    if (retardo > 0) usleep((useconds_t)retardo * 1000);

    log_info(worker->logger, "%s en memoria simulada (PC=%d) - %s:%s base=%u size=%u",
             dir, instr->pc,
             instr->file ?: "NULL",
             instr->tag ?: "NULL",
             instr->base, instr->size);
}*/

static int worker_storage_simulate_request(t_worker* worker, const char* op, const t_instruccion* instr) {
    log_info(worker->logger, "[SIM] Storage: %s (PC=%d) - %s:%s",
             op, instr->pc,
             instr->file ?: "NULL",
             instr->tag ?: "NULL");
    return 0;
}


int worker_query_execute_instruction(t_worker* worker, t_instruccion* instr) {
    if (!worker || !instr) return -1;

    if (instr->tipo == INSTR_INVALID) {
        log_error(worker->logger, "Instrucción inválida en PC=%d — omitiendo", instr->pc);
        return -1;
    }

    int rc = 0;
    uint32_t block_size = worker->block_size;

    switch (instr->tipo) {
        case INSTR_CREATE:{
            pthread_mutex_lock(&worker->archivos_mutex);

            // Verificamos si ya existe
            t_archivo_worker* existente = worker_archivo_buscar(worker, instr->file, instr->tag);
            if (existente) {
                log_error(worker->logger,
                    "CREATE fallido: ya existe %s:%s", instr->file, instr->tag);
                pthread_mutex_unlock(&worker->archivos_mutex);
                return -1;
            }

            // Crear archivo local
            t_archivo_worker* nuevo = archivo_worker_create(instr->file, instr->tag);
            list_add(worker->archivos, nuevo);

            pthread_mutex_unlock(&worker->archivos_mutex);

            // Futuro: mandar request a Storage
            // storage_create(worker->fd_storage, instr->file, instr->tag);

            log_info(worker->logger,
                "## <QUERY_ID> - File Creado %s:%s", instr->file, instr->tag);
            break;
        }
        case INSTR_TRUNCATE:{
            int res = worker_archivo_truncar(worker, instr->file, instr->tag, instr->size);
            if (res == 0) {
                // Futuro: notificar al Storage
                // storage_truncate(worker->fd_storage, instr->file, instr->tag, instr->size);

                log_info(worker->logger,
                    "## <QUERY_ID> - File Truncado %s:%s - Nuevo tamaño: %u",
                    instr->file, instr->tag, instr->size);
            } else {
                log_error(worker->logger,
                    "TRUNCATE fallido: %s:%s (size=%u)", instr->file, instr->tag, instr->size);
                return -1;
            }
            break;
        }
        case INSTR_WRITE:{
            log_info(worker->logger, "EXEC WRITE (PC=%d) - %s:%s base=%u size=%u",
                    instr->pc, instr->file, instr->tag, instr->base, instr->size);

            // ⚠ Falta implementación real con memoria
            uint32_t remaining_w = instr->size;
            uint32_t cursor_w = 0;

            while (remaining_w > 0) {
                uint32_t offset_in_page = (instr->base + cursor_w) % worker->block_size;
                uint32_t can_write = MIN(worker->block_size - offset_in_page, remaining_w);

                // TODO: buscar frame en memoria, si no existe -> pedir a storage
                log_debug(worker->logger, "WRITE %u bytes en offset %u de %s:%s",
                        can_write, offset_in_page, instr->file, instr->tag);

                cursor_w += can_write;
                remaining_w -= can_write;
            }

            usleep(worker->config->retardo_memoria * 1000);
            break;
        }
        case INSTR_READ:{
            log_info(worker->logger, "EXEC READ (PC=%d) - %s:%s base=%u size=%u",
                    instr->pc, instr->file, instr->tag, instr->base, instr->size);

            uint32_t remaining_r = instr->size;
            uint32_t cursor_r = 0;
            uint8_t* out_buffer = malloc(remaining_r);

            while (remaining_r > 0) {
                uint32_t offset_in_page = (instr->base + cursor_r) % worker->block_size;
                uint32_t can_read = MIN(worker->block_size - offset_in_page, remaining_r);

                // TODO: buscar frame y leer
                // void* data = memoria_read(...);

                log_debug(worker->logger, "READ %u bytes en offset %u de %s:%s",
                        can_read, offset_in_page, instr->file, instr->tag);

                cursor_r += can_read;
                remaining_r -= can_read;
            }
            usleep(worker->config->retardo_memoria * 1000);
            free(out_buffer);
            break;
        }
        case INSTR_TAG:{
            log_info(worker->logger, "EXEC TAG (PC=%d) - %s:%s -> %s:%s",
                     instr->pc, instr->file, instr->tag, instr->file_destino, instr->tag_destino);
            rc = worker_storage_simulate_request(worker, "TAG", instr);
            break;
        }
        case INSTR_COMMIT:{
            log_info(worker->logger, "EXEC COMMIT (PC=%d) - %s:%s", instr->pc, instr->file, instr->tag);
            rc = worker_storage_simulate_request(worker, "COMMIT", instr);
            break;
        }
        case INSTR_FLUSH:{
            log_info(worker->logger, "EXEC FLUSH (PC=%d) - %s:%s", instr->pc, instr->file, instr->tag);
            rc = worker_storage_simulate_request(worker, "FLUSH", instr);
            break;
        }
        case INSTR_DELETE:{
            int res = worker_archivo_eliminar(worker, instr->file, instr->tag);
            if (res == 0) {
                // Futuro: notificar al Storage
                // storage_delete(worker->fd_storage, instr->file, instr->tag);

                log_info(worker->logger,
                    "## <QUERY_ID> - Tag Eliminado %s:%s", instr->file, instr->tag);
            } else {
                log_error(worker->logger,
                    "DELETE fallido: no existe %s:%s", instr->file, instr->tag);
                return -1;
            }
            break;
        }
        case INSTR_END:{
            log_info(worker->logger, "EXEC END (PC=%d) - Fin del script", instr->pc);
            break;
        }
        case INSTR_INVALID:{
            log_error(worker->logger, "Instrucción inválida en PC=%d", instr->pc);
            rc = -1;
            break;
        }
    }

    log_info(worker->logger, "INSTRUCCION_REALIZADA - PC=%d - %s",
             instr->pc, worker_instruccion_nombre_sin_param(instr));
    return rc;
}

int worker_query_execute_all(t_worker* worker, t_list* instrucciones) {
    if (!worker || !instrucciones) return -1;

    for (int i = 0; i < list_size(instrucciones); i++) {
        t_instruccion* instr = list_get(instrucciones, i);

        log_info(worker->logger, "FETCH - PC=%d - %s",
                 instr->pc, worker_instruccion_nombre_sin_param(instr));

        int rc = worker_query_execute_instruction(worker, instr);
        if (rc != 0)
            log_warning(worker->logger, "Error en PC=%d, continuando...", instr->pc);

        if (instr->tipo == INSTR_END) break;
    }
    return 0;
}

t_archivo_worker* archivo_worker_create(const char* file, const char* tag) {
    t_archivo_worker* a = malloc(sizeof(t_archivo_worker));
    a->file = strdup(file);
    a->tag = strdup(tag);
    a->size = 0;
    a->state = FILE_STATE_WORK_IN_PROGRESS;
    a->paginas = dictionary_create();
    return a;
}

void archivo_worker_destroy(void* archivo_ptr) {
    t_archivo_worker* a = (t_archivo_worker*)archivo_ptr;
    if (!a) return;

    void _destroy_entry(void* value) {
        free(value);
    }
    dictionary_destroy_and_destroy_elements(a->paginas, _destroy_entry);

    free(a->file);
    free(a->tag);
    free(a);
}

t_archivo_worker* worker_archivo_buscar(t_worker* w, const char* file, const char* tag) {
    pthread_mutex_lock(&w->archivos_mutex);
    t_archivo_worker* result = NULL;

    for (int i = 0; i < list_size(w->archivos); i++) {
        t_archivo_worker* a = list_get(w->archivos, i);
        if (strcmp(a->file, file) == 0 && strcmp(a->tag, tag) == 0) {
            result = a;
            break;
        }
    }

    pthread_mutex_unlock(&w->archivos_mutex);
    return result;
}

int worker_archivo_truncar(t_worker* w, const char* file, const char* tag, uint32_t new_size) {
    pthread_mutex_lock(&w->archivos_mutex);

    t_archivo_worker* a = worker_archivo_buscar(w, file, tag);
    if (!a) {
        pthread_mutex_unlock(&w->archivos_mutex);
        return -1;
    }

    if (a->state == FILE_STATE_COMMITED) {
        pthread_mutex_unlock(&w->archivos_mutex);
        return -1;
    }

    if (new_size % w->block_size != 0) {
        pthread_mutex_unlock(&w->archivos_mutex);
        return -1; 
    }

    uint32_t old_pages = a->size / w->block_size;
    uint32_t new_pages = new_size / w->block_size;

    // si reduce tamaño: liberar páginas sobrantes
    if (new_pages < old_pages) {
        for (uint32_t p = new_pages; p < old_pages; p++) {
            char* key = string_itoa(p);
            int* frame_ptr = dictionary_remove(a->paginas, key);
            if (frame_ptr) {
                int frame = *frame_ptr;
                if (frame >= 0 && frame < w->memoria->max_frames) {
                    w->memoria->frames[frame].ocupado = false;
                    w->memoria->frames[frame].bit_uso = false;
                    w->memoria->frames[frame].bit_modificado = false;
                }
                free(frame_ptr);
            }
            free(key);
        }
    }

    a->size = new_size;

    pthread_mutex_unlock(&w->archivos_mutex);
    return 0;
}

int worker_archivo_eliminar(t_worker* w, const char* file, const char* tag) {
    pthread_mutex_lock(&w->archivos_mutex);
    int result = -1;

    for (int i = 0; i < list_size(w->archivos); i++) {
        t_archivo_worker* a = list_get(w->archivos, i);
        if (strcmp(a->file, file) == 0 && strcmp(a->tag, tag) == 0) {
            // liberar páginas en memoria interna
            void _free_page(char* key, void* value) {
                int frame = *(int*)value;
                if (frame >= 0 && frame < w->memoria->max_frames) {
                    w->memoria->frames[frame].ocupado = false;
                    w->memoria->frames[frame].bit_uso = false;
                    w->memoria->frames[frame].bit_modificado = false;
                }
                free(value);
            }
            dictionary_iterator(a->paginas, _free_page);

            list_remove_and_destroy_element(w->archivos, i, archivo_worker_destroy);
            result = 0;
            break;
        }
    }

    pthread_mutex_unlock(&w->archivos_mutex);
    return result;
}

int worker_archivo_agregar(t_worker* w, t_archivo_worker* archivo) {
    pthread_mutex_lock(&w->archivos_mutex);

    for (int i = 0; i < list_size(w->archivos); i++) {
        t_archivo_worker* a = list_get(w->archivos, i);
        if (strcmp(a->file, archivo->file) == 0 && strcmp(a->tag, archivo->tag) == 0) {
            pthread_mutex_unlock(&w->archivos_mutex);
            return -1;
        }
    }

    list_add(w->archivos, archivo);

    pthread_mutex_unlock(&w->archivos_mutex);
    return 0;
}