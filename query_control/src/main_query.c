#include<main_query.h>

t_config* config;
t_log* logger;

int main(int argc, char* argv[]) {
    
    int socket_conexion;
	char* ip;
	char* puerto;

    config = iniciar_config();
    logger = iniciar_logger();

    ip = config_get_string_value(config, "IP_MASTER");
    puerto = config_get_string_value(config, "PUERTO_MASTER");

    socket_conexion = crear_conexion(ip, puerto, logger);


    enviar_mensaje("falta", socket_conexion);

	terminar_programa(socket_conexion, logger, config);


    return 0;
}

t_config* iniciar_config(void)
{
    t_config* nuevo_config = config_create("ejemplo_query.config");

    return nuevo_config;
}

t_log* iniciar_logger(void)
{
    t_log* nuevo_logger = log_create("logger_query.log", "QueryControl", 1, LOG_LEVEL_INFO);

    return nuevo_logger;
}


void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}