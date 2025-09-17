#ifndef MAINQUERY_H_
#define MAINQUERY_H_

#include<commons/log.h>
#include<commons/config.h>
#include"conexiones_cliente.h"


t_config* iniciar_config(void);
t_log* iniciar_logger(void);
void terminar_programa(int conexion, t_log* logger, t_config* config);

#endif