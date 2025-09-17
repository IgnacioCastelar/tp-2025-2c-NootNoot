#include<main_master.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) 
{
    logger = log_create("logger_master.log", "master", 1, LOG_LEVEL_INFO);
	config = config_create("config/ejemplo_master.config");

	char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");

    int server_fd = iniciar_servidor(puerto);
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);


	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			log_info(logger, "Se recibio el mensaje. Prueba de branch");
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}
/*
void iterator(char* value) {
	log_info(logger,"%s", value);
}
*/