#include "worker.h"
#include "worker_conexiones.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

t_worker* worker_create(const char* path_cfg, const char* id) {
    t_log* logger = log_create("worker.log", "WORKER", true, LOG_LEVEL_INFO);
    if (!logger) return NULL;

    ConfigWorker* cfg = worker_config_create((char*)path_cfg, logger);
    if (!cfg) { log_destroy(logger); return NULL; }

    t_worker* w = malloc(sizeof(t_worker));
    w->id = strdup(id);
    w->config = cfg;
    w->logger = logger;

    w->fd_master = worker_conectar_master(cfg, id, logger);

    w->block_size = -1;
    w->fd_storage = worker_conectar_storage(cfg, logger, &w->block_size);

    log_info(logger,
        "Worker %s inicializado. Memoria=%d, Retardo=%d, Algoritmo=%s, Scripts=%s, BlockSize=%d",
        w->id, cfg->tam_memoria, cfg->retardo_memoria, cfg->algoritmo_reemplazo,
        cfg->path_scripts, w->block_size);

    return w;
}

void worker_destroy(t_worker* w) {
    if (!w) return;
    if (w->fd_master > 0) close(w->fd_master);
    if (w->fd_storage > 0) close(w->fd_storage);
    worker_config_destroy(w->config);
    log_destroy(w->logger);
    free(w->id);
    free(w);
}