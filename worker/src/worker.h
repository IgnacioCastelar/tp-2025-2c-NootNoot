#ifndef WORKER_H
#define WORKER_H

#include "worker_config.h"
#include <commons/log.h>
#include <commons/collections/list.h>
#include "worker_memoria.h"
#include "worker_conexiones.h"
#include "worker_instrucciones.h"
#include "worker_query.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


typedef struct t_worker {
    char* id;
    ConfigWorker* config;
    t_log* logger;
    int fd_master;
    int fd_storage;
    int block_size;
    t_memoria_worker* memoria;
    t_list* archivos;
    pthread_mutex_t archivos_mutex;
} t_worker;

t_worker* worker_create(const char* path_cfg, const char* id);
void worker_destroy(t_worker* worker);

#endif