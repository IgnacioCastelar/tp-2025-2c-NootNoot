#include "conexiones_servidor.h"


int iniciar_servidor(char* puerto)
{
	int err;
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, puerto, &hints, &servinfo);

	if(err != 0)
	{
		log_info(logger, "Hubo algun tipo de error.");
	}

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
							servinfo->ai_socktype,
							servinfo->ai_protocol);

	// Asociamos el socket a un puerto

	err = setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	int resultado_bind = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	if (resultado_bind == -1)
    {
        log_error(logger, "ERROR al asociar el socket con el puerto");
        //abort();
		exit(-1);
    }

	int resultado_listen = listen(socket_servidor, SOMAXCONN);

	if (resultado_listen == -1)
    {
        log_error(logger, "ERROR al escuchar las conexiones entrantes");
        //abort();
		exit(-1);
    }

	// Escuchamos las conexiones entrantes


	log_info(logger, "Listo para escuchar a mi cliente");
	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);;
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}