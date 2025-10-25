#include <main_query.h>
#include "query_config.h"

t_config *config;
t_log *logger;

int main(int argc, char *argv[])
{

    int socket_conexion;
    char *ip;
    char *puerto;

    config = iniciar_config();
    logger = iniciar_logger();

    ip = config_get_string_value(config, "IP_MASTER");
    puerto = config_get_string_value(config, "PUERTO_MASTER");

    socket_conexion = crear_conexion_cliente(ip, puerto);

    log_info(logger, "## Conexión al Master exitosa. IP: %s, Puerto: %s", ip, puerto);

    t_paths *path = malloc(sizeof(t_paths));
    if (path == NULL)
    {
        perror("Error al reservar memoria para path");
        exit(EXIT_FAILURE);
    }
    //uint8_t priori = 8;

    path->path_query = argv[1]; //"ruta2/ruta3/ruta4";
    path->length_path_query = strlen(path->path_query);
    path->prioridad = (uint8_t)atoi(argv[2]);

    t_buffer *buffer_resultado = llenar_buffer_master(path);

    crear_paquete_y_enviar_master(socket_conexion, MENSAJE_MASTER, buffer_resultado);

    log_info(logger, "## Solicitud de ejecución de Query: %s, prioridad: %s", path->path_query, argv[2]);

    free(path);
    terminar_programa(socket_conexion, logger, config);

    return 0;
}

void atender_mensajes_master(int conexion)
{
    while(1)
    {
        
    }
}


t_config *iniciar_config(void)
{
    //t_config *nuevo_config = config_create("/home/utnso/tp-2025-2c-NootNoot/query_control/ejemplo_query.config");
    t_config* nuevo_config = config_create("ejemplo_query.config");
    return nuevo_config;
}

t_log *iniciar_logger(void)
{
    //t_log *nuevo_logger = log_create("/home/utnso/tp-2025-2c-NootNoot/query_control/logger_query.log", "QueryControl", 1, LOG_LEVEL_INFO);
    t_log *nuevo_logger = log_create("logger_query.log", "QueryControl", 1, LOG_LEVEL_INFO);
    return nuevo_logger;
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
    /* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
      con las funciones de las commons y del TP mencionadas en el enunciado */
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(conexion);
}