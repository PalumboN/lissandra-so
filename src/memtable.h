#ifndef MEMTABLE
#define MEMTABLE

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "registry.h"

typedef t_dictionary Memtable;

Memtable* memtable_create();

t_list* memtable_get(Memtable* memtable, char* table);

void memtable_allocate(Memtable* memtable, char* table);

void memtable_insert(Memtable* memtable, char* table, Registry* data);

void memtable_dump(Memtable* memtable);

#endif