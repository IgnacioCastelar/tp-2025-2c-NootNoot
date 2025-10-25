#include<main_storage.h>


// Prototipos de funciones de operaciones de archivos
/*
t_archivo* crear_archivo(const char* nombre, const char* tipo, const char* propietario, const char* permisos, const char* ubicacion);
t_archivo* abrir_archivo(const char* nombre);
int leer_archivo(t_archivo* archivo, void* buffer, uint32_t tamanio, uint32_t posicion);
int escribir_archivo(t_archivo* archivo, const void* buffer, uint32_t tamanio, uint32_t posicion);
int renombrar_archivo(t_archivo* archivo, const char* nuevo_nombre);
int mover_archivo(t_archivo* archivo, const char* nueva_ubicacion);
int truncar_archivo(t_archivo* archivo, uint32_t nuevo_tamanio);
int borrar_archivo(const char* nombre);
int cerrar_archivo(t_archivo* archivo);
int reposicionar_archivo(t_archivo* archivo, uint32_t nueva_posicion);
*/

void crear_superblock_config(const char* path, int fs_size, int block_size);

void formatear_volumen(const char* path);
// Prototipos de funciones de manejo de clientes
void* manejar_cliente(void* arg);


// Prototipos de funciones para superblock y bitmap

void inicializar_bitmap(const char* path, int cantidad_bloques);
void destruir_bitmap();

bool stringToBool(char *);

int main(char *archivo_configuracion) 
{
    
    logger = log_create("logger_storage.log", "Storage", 1, LOG_LEVEL_INFO);
	config = config_create(archivo_configuracion);
    if (config == NULL) {
    	log_error(logger, "No se pudo abrir el archivo de configuración");
    	return EXIT_FAILURE;
	}
    char * stringBooleanoDeFresh_Start = config_get_string_value(config,"FRESH_START");
    bool fresh_start = stringToBool(stringBooleanoDeFresh_Start);

    if(fresh_start){
        //formatear volumen
    }

    char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");


    if(fresh_start) {
        log_info(logger, "FRESH_START=TRUE: eliminando todo excepto el archivo de configuración y creando solo superblock.config");
        formatear_volumen(punto_montaje);

        int fs_size, block_size;
        printf("Ingrese el tamaño total del FS en bytes (FS_SIZE)(EJ.4096): ");
        scanf("%d", &fs_size);
        printf("Ingrese el tamaño de bloque en bytes (BLOCK_SIZE)(EJ.128): ");
        scanf("%d", &block_size);

        char superblock_path[256];
        snprintf(superblock_path, sizeof(superblock_path), "%s/superblock.config", punto_montaje);
        crear_superblock_config(superblock_path, fs_size, block_size); 

        superblock_cfg = config_create(superblock_path);
        fs_size = config_get_int_value(superblock_cfg, "FS_SIZE");
        block_size = config_get_int_value(superblock_cfg, "BLOCK_SIZE");
        cantidad_bloques = fs_size / block_size;
        config_destroy(superblock_cfg);
    }

	char* puerto = config_get_string_value(config, "PUERTO_ESCUCHA");
    int server_fd = iniciar_servidor(puerto);

	log_info(logger, "Servidor listo para recibir al cliente");

	while (1) {
        int cliente_fd = esperar_cliente(server_fd);
        if (cliente_fd != -1) {
            pthread_t hilo;
            int* arg = malloc(sizeof(int));
            *arg = cliente_fd;

            pthread_create(&hilo, NULL, manejar_cliente, arg);
            pthread_detach(hilo);

            log_info(logger, "Nuevo cliente conectado");
        }
    }

    destruir_bitmap(); // Al final del programa
	return EXIT_SUCCESS;
}
/*
void iterator(char* value) {
	log_info(logger,"%s", value);
}
*/

void* manejar_cliente(void* arg) {
    int cliente_fd = *(int*)arg;
    free(arg);

    char worker_id [64];
    int bytes_recibidos = recv(cliente_fd, worker_id, sizeof(worker_id)-1,0);
    if(bytes_recibidos > 0){
        worker_id[bytes_recibidos] = '\0';
        cantidad_workers++;
        log_info(logger, "se conecta el worker %s - Cantidad de workers: %d", worker_id, cantidad_workers);
    }

    while (1) {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op) {
        case MENSAJE:
            recibir_mensaje(cliente_fd);
            break;
        case -1:
            log_info(logger, "Se desconecta el worker %s - cantidad de workers: %d", worker_id, cantidad_workers); 
            close(cliente_fd);
            cantidad_workers--;
            return NULL;
        default:
            log_warning(logger, "Operación desconocida");
            break;
        }
    }
}

//GENERANDO ARCHIVOS QUE PIDEN:


// Inicializar bitmap.bin usando mmap y bitarray de commons
void inicializar_bitmap(const char* path, int cantidad_bloques) {
    int bitmap_size = cantidad_bloques / 8;
    if (cantidad_bloques % 8 != 0) bitmap_size++;

    bitmap_fd = open(path, O_RDWR | O_CREAT, 0664);
    ftruncate(bitmap_fd, bitmap_size);

    bitmap_mem = mmap(NULL, bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);
    bitmap = bitarray_create_with_mode(bitmap_mem, bitmap_size, LSB_FIRST);

    // Opcional: inicializar todos los bits en 0 (libres) si es FRESH_START
    for (int i = 0; i < cantidad_bloques; i++) {
        bitarray_clean_bit(bitmap, i);
    }
}

// Liberar recursos del bitmap
void destruir_bitmap() {
    int bitmap_size = cantidad_bloques / 8;
    if (cantidad_bloques % 8 != 0) bitmap_size++;
    msync(bitmap_mem, bitmap_size, MS_SYNC);
    munmap(bitmap_mem, bitmap_size);
    close(bitmap_fd);
    bitarray_destroy(bitmap);
}

void formatear_volumen(char* ruta_directorio) {
    DIR* directorio = opendir(ruta_directorio);
    
    struct dirent* elemento;
    char ruta_completa[512];

    if (!directorio) return;
    while ((elemento = readdir(directorio)) != NULL) {
        char * nombre = elemento->d_name;
        if (!strcmp(nombre, ".") || 
            !strcmp(nombre, "..")|| 
            strstr(nombre, "ejemplo_storage.config")||
            strstr(nombre, "superblock.config"))
        continue; //si sucede esto no se ejecuta el resto que sigue y vuelve a hacer el bucle

        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta_directorio, nombre); //construye la ruta completa, ej: /home/utnso/montaje/archivo.txt
        //info_archivos seria nuestro metadatos
        struct stat info_archivo; //structura para guardar informacion del elemento
        stat(ruta_completa, &info_archivo); //extraccion de informacion del elemento y guardado en metadatos

        if (S_ISDIR(info_archivo.st_mode)) {
            formatear_volumen(ruta_completa);
            rmdir(ruta_completa);
        } else {
            remove(ruta_completa);
        }
    }
    closedir(directorio);
}

/*
void crear_superblock_config(const char* path, int fs_size, int block_size) {
    FILE* file = fopen(path, "w");
    if (!file) return;
    fprintf(file, "FS_SIZE=%d\nBLOCK_SIZE=%d\n", fs_size, block_size);
    fclose(file);
}
*/

void crear_superblock_config(char* nombre_archivo_superblock, int fs_size, int block_size) {
    FILE* archivo = fopen(nombre_archivo_superblock, "w"); 
    if (archivo == NULL) {
        perror("Error al crear el archivo superblock.config");
        return;
    }

    fprintf(archivo, "FS_SIZE=%d\n", fs_size);
    fprintf(archivo, "BLOCK_SIZE=%d\n", block_size);

    fclose(archivo);
}



// FUNCIONES QUE NO VAN PARA EL TP, PERO SIRVEN
// PARA PODER EVALUAR EL VALOR DEL CONFIG DEVUELTO


bool stringToBool(char *valorBooleano){
    if(valorBooleano == "True") {return true} else {return false}
};

void creacion_de_initial_file(){
    //unico tag base
        //contenido
            //bloque logico con el bloque fisico numero 0 asignado
            //se completa el bloque con el caracter 0, ej. "00000..."
            //dicho file/tag no se podra borrar
}