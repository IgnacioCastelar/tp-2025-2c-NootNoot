#include "query_config.h"
#include <stdlib.h>
#include <string.h>

ConfigQueryControl* query_control_config_create(char* path) {
    t_config* config_file = config_create(path);

    ConfigQueryControl* config = malloc(sizeof(ConfigQueryControl));

    config->ip_master = strdup(config_get_string_value(config_file, "IP_MASTER"));
    config->puerto_master = config_get_int_value(config_file, "PUERTO_MASTER");
    config->log_level = strdup(config_get_string_value(config_file, "LOG_LEVEL"));
    config_destroy(config_file);
    return config;
}

void query_control_config_destroy(ConfigQueryControl* config) {
    if (!config) return;

    free(config->ip_master);
    free(config->ip_storage);
    free(config->log_level);
    free(config);
}