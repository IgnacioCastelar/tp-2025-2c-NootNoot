#include "worker.h"

t_worker* worker_create(const char* path_cfg, const char* id) {
    t_log* logger = log_create("worker.log", "WORKER", true, LOG_LEVEL_INFO);
    if (!logger) return NULL;

    ConfigWorker* cfg = worker_config_create((char*)path_cfg, logger);
    if (!cfg) {
        log_destroy(logger);
        return NULL;
    }

    t_worker* w = malloc(sizeof(t_worker));
    w->id = strdup(id);
    w->config = cfg;
    w->logger = logger;

    w->fd_master = worker_conectar_master(cfg, id, logger);

    w->block_size = -1;
    w->fd_storage = worker_conectar_storage(cfg, id, logger, &w->block_size);

    if (w->block_size <= 0) {
        log_error(logger, "No se pudo obtener el block_size desde Storage");
        worker_destroy(w);
        return NULL;
    }

    w->memoria = memoria_crear_cache(cfg->tam_memoria, w->block_size, 
                                     cfg->algoritmo_reemplazo, w->logger);

    log_info(logger,
             "Worker %s inicializado. Memoria=%d, FrameSize=%d, Retardo=%d, "
             "Algoritmo=%s, Scripts=%s",
             w->id, cfg->tam_memoria, w->block_size, cfg->retardo_memoria,
             cfg->algoritmo_reemplazo, cfg->path_scripts);

    return w;
}

void worker_destroy(t_worker* w) {
    if (!w) return;
    if (w->archivos) {
        list_destroy_and_destroy_elements(w->archivos, archivo_worker_destroy);
        pthread_mutex_destroy(&w->archivos_mutex);
    }
    if (w->fd_master > 0) close(w->fd_master);
    if (w->fd_storage > 0) close(w->fd_storage);
    memoria_destruir_cache(w->memoria);
    worker_config_destroy(w->config);
    log_destroy(w->logger);
    free(w->id);
    free(w);
}