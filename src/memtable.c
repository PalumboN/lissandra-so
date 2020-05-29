#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <stdio.h>

#include "utils.h"
#include "registry.h"
#include "file_system.h"

typedef t_dictionary Memtable;

void dump_data(char* table, t_list* registries) {
    create_new_temp(table, registries);
    list_destroy(registries);
}

////

Memtable* memtable_create() {
    return dictionary_create();
}

t_list* memtable_get(Memtable* memtable, char* table) {
    if (!dictionary_has_key(memtable, table))
        return list_create();
    return dictionary_get(memtable, table);
}

void memtable_allocate(Memtable* memtable, char* table) {
    t_list* registries = dictionary_get(memtable, table);
    if (registries == NULL) {
        dictionary_put(memtable, table, list_create());
    }
}

void memtable_insert(Memtable* memtable, char* table, Registry* data) {
    t_list* registries = memtable_get(memtable, table);
    list_add(registries, data);
}

void memtable_dump(Memtable* memtable) {
    dictionary_iterator(memtable, (void*)dump_data);
    dictionary_clean(memtable);    
}