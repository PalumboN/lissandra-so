#include <commons/string.h> 
#include <commons/config.h> 
#include <pthread.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "src/lissandra.h"
#include "src/memtable.h"
#include "src/utils.h"

void listen(int port) {
    printf("TODO: Listen %d\n", port);
}

void* dump(Memtable* memtable, int interval) {
    printf("ASD: %d\n", interval);
    while (true)
    {
        printf("DUMPING MEMTABLE\n");
        memtable_dump(memtable);
        sleep(interval / 1000);
    }
}

char* PORT = "PUERTO_ESCUCHA​";
char* DUMP_TIME = "TIEMPO_DUMP";
char* MOUNT_POINT = "PUNTO_MONTAJE​";

void main() {
    t_config* config = config_create("config");

    pthread_t thread_socket, thread_dump; 
    Memtable* memtable = memtable_create();
    set_base_path(config_get_string_value(config, MOUNT_POINT));

    pthread_create(&thread_socket, NULL, (void*) listen, config_get_int_value(config, PORT));

    pthread_create(&thread_dump, NULL, (void*) lambda(void, (int interval), {
        dump(memtable, interval);
    }), config_get_int_value(config, DUMP_TIME));


    
    char query[200];
    while (scanf("%[^\n]s", query))
    {

        if (string_equals_ignore_case(query, "q"))
            break;

        printf("RESULT: %s\n", resolve_query(memtable, query));
        
        printf("NEW QUERY: %s\n", query);
    }
    
    printf("FINISH\n");    
    printf("\n");    
}