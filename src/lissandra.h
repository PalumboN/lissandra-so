#ifndef LISSANDRA
#define LISSANDRA

#include "file_system.h"
#include "memtable.h"

char* lissandra_select(Memtable* memtable, char* table, int key);
char* lissandra_insert(Memtable* memtable, char* table, Registry* data);
char* lissandra_create(char* tableName, TableMetadata metadata);
char* lissandra_drop(char* tableName);

char* resolve_query(Memtable* memtable, char* query);

#endif