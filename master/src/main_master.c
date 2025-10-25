#include <main_master.h>

t_log *logger;
t_config *config;
t_list *workers_conectados;

int main(int argc, char *argv[])
{
	logger = log_create("/home/utnso/tp-2025-2c-NootNoot/master/logger_master.log", "master", 1, LOG_LEVEL_INFO);
	// logger = log_create("logger_master.log", "master", 1, LOG_LEVEL_INFO);
	config = config_create("/home/utnso/tp-2025-2c-NootNoot/master/ejemplo_master.config");
	// config = config_create("ejemplo_master.config");

	char *puerto = config_get_string_value(config, "PUERTO_ESCUCHA");

	int socket_server = iniciar_servidor(puerto);

	iniciar_conexiones(socket_server);

	return EXIT_SUCCESS;
}

void iniciar_conexiones(int socket_server)
{
	while (1)
	{
		int socket_cliente = esperar_cliente(socket_server);
		int *socket_para_hilo = malloc(sizeof(int));
		*socket_para_hilo = socket_cliente;

		pthread_t hilo_conexiones;
		int verificador_hilo = pthread_create(&hilo_conexiones, NULL, atender_conexiones_cliente, socket_para_hilo);
		pthread_detach(hilo_conexiones);

		if (verificador_hilo == 0)
		{
			log_info(logger, "El hilo se creo correctamente, estamos en el hilo que aitnede las conexiones!!.");
		}
		else
		{
			log_info(logger, "Oh! Ocurrio un error al momento de crear el hilo.");
		}

		pthread_detach(hilo_conexiones);
	}
}

void *atender_conexiones_cliente(void *args_conexion)
{
	int fd_cliente = *(int *)args_conexion;
	free(args_conexion);

	pthread_t hilo_conexion_query;

	int *fd_para_hilo = malloc(sizeof(int));
	*fd_para_hilo = fd_cliente;

	pthread_create(&hilo_conexion_query, NULL, atender_querys, fd_para_hilo);
	pthread_detach(hilo_conexion_query);

	pthread_t hilo_conexion_worker;
	pthread_create(&hilo_conexion_worker, NULL, atender_worker, fd_para_hilo);

	while (1)
	{
		int id_worker = recibir_id_worker(fd_cliente);
		t_list *cpus_conectadas;
	}

	return NULL;
}

void *atender_querys(void *socket_server)
{
	int socket_server_deHilo = *(int *)socket_server;
	free(socket_server);

	int cod_op = recibir_operacion(socket_server_deHilo);
	switch (cod_op)
	{
	case MENSAJE_MASTER:
		t_paths *path_recibido = recibir_paquete_query(socket_server_deHilo);
		log_info(logger, "Se recibió ruta de query: %s", path_recibido->path_query);
		log_info(logger, "Se recibió prioridad: %d", path_recibido->prioridad);

		planificador_corto_plazo(path_recibido->path_query);

		break;
	case -1:
		log_error(logger, "el cliente se desconecto. Terminando servidor");
		return EXIT_FAILURE;
	default:
		log_warning(logger, "Operacion desconocida. No quieras meter la pata");
		break;
	}
	return NULL;
}

void *atender_worker(void *socket_server)
{
	int contador_worker = 0;
	int socket_server_deHilo = *(int *)socket_server;
	free(socket_server);
	while (1)
	{
		int cod_op = recibir_operacion(socket_server_deHilo);
		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(socket_server_deHilo);
		case ID_WORKER:
			t_cpu_conectada *worker_conectada;
			worker_conectada->id_cpu = recibir_id_worker(socket_server_deHilo);
			contador_worker++;
			log_info(logger, "## Se conecta el Worker %d - Cantidad total de Workers: %d", worker_conectada->id_cpu, contador_worker);
			list_add(workers_conectados, worker_conectada->id_cpu);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return NULL;
}

/*
void iterator(char *value)
{
	log_info(logger, "%s", value);
}

*/