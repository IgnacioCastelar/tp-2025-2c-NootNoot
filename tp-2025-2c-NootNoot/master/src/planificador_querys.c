#include <planificador_querys.h>

/*

listaDispositivos:
    ->t_propQuery
*/

t_propQuery crear_query_para_ready(char *path)
{
    t_propQuery *query = malloc(sizeof(t_propQuery));
    query->path_query = path;
    query->id_query = 0;
    query->id_query++;
    query->estado_actual = READY;
    query->priorirdad = 0

    free(query);
    return query;
}

void planificador_corto_plazo(char *path_query)
{
    char *algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    int nivelConstProv = 0;
    while (1)
    {
        t_list *cola_querys_ready = list_create();

        t_propQuery* query_obtenida = crear_query_para_ready(path_query);
        log_info(logger, "## Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d", query_obtenida->path_query, query_obtenida->priorirdad, query_obtenida->id_query,nivelConstProv);

        list_add(cola_querys_ready, query_obtenida);

        if (strcmp(algoritmo, "FIFO") == 0)
        {
            t_propQuery *query = list_get(cola_querys_ready, 0);
            list_remove(cola_querys_ready, 0);
            asignar_a_un_worker();
        }
    }
}

void asignar_a_un_worker(t_list* workers, )