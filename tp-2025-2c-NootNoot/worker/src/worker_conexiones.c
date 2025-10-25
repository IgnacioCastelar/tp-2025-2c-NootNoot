#include "worker_conexiones.h"
#include "conexiones_cliente.h"
#include "handshake.h"
#include "conexiones_cliente.h"
#include "conexiones_servidor.h"
#include <unistd.h>
#include <string.h>

int worker_conectar_master(ConfigWorker* cfg, const char* worker_id, t_log* logger) {
    int fd = crear_conexion_cliente_worker(cfg->ip_master, cfg->puerto_master, logger);
    if (fd < 0) {
        log_error(logger, "No se pudo conectar al Master %s:%d", cfg->ip_master, cfg->puerto_master);
        return -1;
    }
    log_info(logger, "Conectado al Master en %s:%d (fd=%d)", cfg->ip_master, cfg->puerto_master, fd);


    t_paquete* hs = crear_paquete_handshake("WORKER", worker_id);
    enviar_paquete(hs, fd);
    destruir_paquete(hs);

    return fd;
}

int worker_conectar_storage(ConfigWorker* cfg, const char* worker_id, t_log* logger, int* out_block_size) {
    int fd = crear_conexion_cliente_worker(cfg->ip_storage, cfg->puerto_storage, logger);
    if (fd < 0) {
        log_error(logger, "No se pudo conectar al Storage %s:%d", cfg->ip_storage, cfg->puerto_storage);
        return -1;
    }

    log_info(logger, "Conectado al Storage en %s:%d (fd=%d)", cfg->ip_storage, cfg->puerto_storage, fd);

    const char* modulo = "WORKER";
    int len_modulo = strlen(modulo) + 1;
    int len_id = strlen(worker_id) + 1;

    t_paquete* paquete = crear_paquete(HANDSHAKE);
    agregar_a_paquete(paquete, &len_modulo, sizeof(int));
    agregar_a_paquete(paquete, (void*)modulo, len_modulo);
    agregar_a_paquete(paquete, &len_id, sizeof(int));
    agregar_a_paquete(paquete, (void*)worker_id, len_id);
    enviar_paquete(paquete, fd);
    destruir_paquete(paquete);

    t_paquete* resp = recibir_paquete(fd);
    if (!resp || resp->buffer->size < (int)(sizeof(int) * 2)) {
        log_error(logger, "No se recibió handshake válido desde Storage.");
        destruir_paquete(resp);
        return -1;
    }

    int offset = 0;
    int block_size = 0;
    int result_ok = -1;

    memcpy(&block_size, resp->buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&result_ok, resp->buffer->stream + offset, sizeof(int));

    destruir_paquete(resp);

    if (result_ok != 0) {
        log_error(logger, "Storage rechazó el handshake con código de error %d.", result_ok);
        return -1;
    }

    if (block_size <= 0) {
        log_error(logger, "Storage envió un BLOCK_SIZE inválido (%d).", block_size);
        return -1;
    }

    if (out_block_size) *out_block_size = block_size;
    log_info(logger, "Handshake con Storage exitoso. BLOCK_SIZE = %d", block_size);

    return fd;
}

void* worker_escuchar_master(void* arg) {
    t_worker* w = (t_worker*)arg;
    log_info(w->logger, "Hilo de escucha al Master iniciado (fd=%d)", w->fd_master);

    while (1) {
        t_paquete* paquete = recibir_paquete(w->fd_master);
        if (!paquete) {
            log_error(w->logger, "Conexión con el Master perdida. Cerrando hilo de escucha.");
            break;
        }

        switch (paquete->codigo_operacion) {
            case OP_EXEC_QUERY: {
                char* script_path = NULL;
                int start_pc = 0;

                void* stream = paquete->buffer->stream;
                int path_len = 0;
                memcpy(&path_len, stream, sizeof(int));
                stream += sizeof(int);

                script_path = malloc(path_len + 1);
                memcpy(script_path, stream, path_len);
                script_path[path_len] = '\0';
                stream += path_len;

                memcpy(&start_pc, stream, sizeof(int));

                log_info(w->logger, "Master solicita ejecutar Query: %s desde PC=%d",
                         script_path, start_pc);

                t_list* instrucciones = worker_instrucciones_cargar(script_path, start_pc, w->logger);
                if (instrucciones) {
                    worker_query_execute_all(w, instrucciones);
                    worker_instrucciones_destroy_list(instrucciones);
                } else {
                    log_error(w->logger, "Error al cargar las instrucciones del script: %s", script_path);
                }

                free(script_path);
                break;
            }

            default:
                log_warning(w->logger, "Operación desconocida del Master: %d", paquete->codigo_operacion);
                break;
        }

        destruir_paquete(paquete);
    }

    return NULL;
}