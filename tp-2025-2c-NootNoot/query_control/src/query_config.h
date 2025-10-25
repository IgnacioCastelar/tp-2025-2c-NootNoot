#ifndef QUERY_CONFIG_H
#define QUERY_CONFIG_H

#include <commons/config.h>
#include <commons/log.h>
#include <stdbool.h>

typedef struct {
    char* ip_master;
    int puerto_master;
    char* log_level;
} ConfigQueryControl;

//ConfigQueryControl* query_control_config_create(char* path, t_log* logger);

ConfigQueryControl* query_control_config_create(char* path);
void query_control_config_destroy(ConfigWorker* config);

#endif