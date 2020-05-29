#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "utils.h"
#include "registry.h"
#include "memtable.h"
#include "file_system.h"



t_list* data_from_partition(char* table, int key) {
    return search_key_in_partitions(table, key);
}

t_list* data_from_temp_files(char* table) {
    return read_all_temp_files(table);
}

t_list* data_from_memtable(Memtable* memtable, char* table) {
    return memtable_get(memtable, table);
}

char* get_value(Memtable* memtable, char* table, int key) {
    t_list* registries = list_create();

    list_add_all(registries, data_from_partition(table, key));
    list_add_all(registries, data_from_temp_files(table));
    list_add_all(registries, data_from_memtable(memtable, table));

    t_list* registrieso = list_filter(registries, (void*)
        lambda(bool, (Registry* registry), {
            return registry->key == key;
        })
    );

    if (list_is_empty(registrieso))
        return NULL;

    list_sort(registrieso, (void*)
        lambda(bool, (Registry* registry1, Registry* registry2), {
            return registry1->timestamp > registry2->timestamp;
        })
    );

    Registry* result = list_get(registrieso, 0);

    list_destroy(registries);
    list_destroy(registrieso);
    
    return result->value;
}

///


char* lissandra_select(Memtable* memtable, char* table, int key) {
    if (!exist_table(table)) {
        return concat("Table doesn't exist: ", table);
    }

    return get_value(memtable, table, key);
}

char* lissandra_insert(Memtable* memtable, char* table, Registry* data) {
    if (!exist_table(table)) {
        return concat("Table doesn't exist: ", table);
    }

    memtable_allocate(memtable, table);
    memtable_insert(memtable, table, data);

    return OK;
}

char* lissandra_create(char* table, TableMetadata metadata) {
    if (exist_table(table)) {
        // TODO: Guardar en .log
        return concat("Table already exist: ", table);
    }

    create_new_table(table, metadata);

    return OK;
}

char* lissandra_drop(char* table) {
    if (!exist_table(table)) {
        return concat("Table doesn't exist: ", table);
    }

    delete_table(table);

    return OK;
}


///

char* resolve_query(Memtable* memtable, char* query) {
    char** splited_query = words(query);
        
    if (string_equals_ignore_case(splited_query[0], "SELECT")) {
        return lissandra_select(memtable, splited_query[1], atoi(splited_query[2]));
    }
       
    if (string_equals_ignore_case(splited_query[0], "INSERT")) {
        Registry* data = new_data(atoi(splited_query[4]), atoi(splited_query[2]), splited_query[3]);
        return lissandra_insert(memtable, splited_query[1], data);
    }

    return "Bad query";
}
