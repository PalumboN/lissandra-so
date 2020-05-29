#ifndef FILE_SYSTEM
#define FILE_SYSTEM

#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include "registry.h"

typedef struct {
    char* consistency;
    int partitions;
    long compactionTime
} TableMetadata;

void set_base_path(char* base);

char* blocks_base_path();

t_list* all_files_in(char* directory);


bool exist_table(char* table);

t_bitarray* read_bitmap();

t_config* table_metadata(char* table);

t_list* search_key_in_partitions(char* table, int key);
t_list* read_all_temp_files(char* table);

int next_block();
int read_block_size();
int read_blocks_count();
void create_db_blocks();
char* read_block(int blockNumber);
void write_block(int blockNumber, char* data);

t_list* write_data(char* data);

int read_partition_size(char* table, int partition);
char** read_partition_blocks(char* table, int partition);
void write_in_partition(char* table, Registry* registry);

void create_new_temp(char* table, t_list* registries);
void create_new_table(char* table, TableMetadata metadata);

void delete_table(char* table);

#endif