#ifndef MAINSTORAGE_H_
#define MAINSTORAGE_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "conexiones_servidor.h"
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdbool.h>
#include <commons/txt.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <commons/crypto.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <operaciones_archivos.h>
#include <archivos.h>


//VARIABLES GLOBALES PARA TRATAR ARCHIVOS
t_log* logger;
t_config* config;
t_config* superblock_cfg;

//-------Variables globales para el superblock y el bitmap
t_config* superblock_cfg;
int fs_size;
int block_size;

t_bitarray* bitmap;
int bitmap_fd;
void* bitmap_mem;
int cantidad_bloques;



int cantidad_workers = 0;

#endif