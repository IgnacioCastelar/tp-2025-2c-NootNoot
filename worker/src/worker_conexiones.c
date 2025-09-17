#include "worker_conexiones.h"
#include "paquete.h"
#include "handshake.h"
#include "conexiones_cliente.h"
#include "conexiones_servidor.h"
#include <unistd.h>
#include <string.h>

int worker_conectar_master(ConfigWorker* cfg, const char* worker_id, t_log* logger) {
    int fd = crear_conexion(cfg->ip_master, cfg->puerto_master, logger);
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

int worker_conectar_storage(ConfigWorker* cfg, t_log* logger, int* out_block_size) {
    int fd = crear_conexion(cfg->ip_storage, cfg->puerto_storage, logger);
    if (fd < 0) {
        log_error(logger, "No se pudo conectar al Storage %s:%d", cfg->ip_storage, cfg->puerto_storage);
        return -1;
    }
    log_info(logger, "Conectado al Storage en %s:%d (fd=%d)", cfg->ip_storage, cfg->puerto_storage, fd);

    t_paquete* hs = crear_paquete_handshake("WORKER", "");
    enviar_paquete(hs, fd);
    destruir_paquete(hs);
    

    t_paquete* resp = recibir_paquete(fd);
    if (resp && resp->buffer->size >= sizeof(int)) {
        int block_size = 0;
        memcpy(&block_size, resp->buffer->stream, sizeof(int));
        if (out_block_size) *out_block_size = block_size;
        log_info(logger, "Storage informó BLOCK_SIZE = %d", block_size);
    } else {
        log_warning(logger, "No se recibió BLOCK_SIZE válido desde Storage");
    }
    destruir_paquete(resp);

    return fd;
}