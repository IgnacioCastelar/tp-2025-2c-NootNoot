#ifndef WORKER_H
#define WORKER_H

#include "worker_config.h"
#include <commons/log.h>

typedef struct {
    char* id;
    ConfigWorker* config;
    t_log* logger;
    int fd_master;
    int fd_storage;
    int block_size;
} t_worker;

t_worker* worker_create(const char* path_cfg, const char* id);
void worker_destroy(t_worker* worker);

#endif