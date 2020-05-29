#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>

#include "registry.h"
#include "utils.h"

typedef struct {
    char* consistency;
    int partitions;
    long compactionTime
} TableMetadata;


// CONSTANTS
char* BASE_PATH;
char* METADATA_BASE_PATH = "metadata/";
char* TABLE_BASE_PATH = "tables/";
char* BLOCKS_BASE_PATH = "bloques/";

char* METADATA_FILE_NAME = "Metadata";
char* BITMAP_FILE_NAME = "Bitmap.txt";

char* TEMP_EXTENSION = ".tmp";
char* BLOCK_EXTENSION = ".txt";
char* PARTITION_EXTENSION = ".txt";
char* COMPACT_TEMP_EXTENSION = ".tmpc";

char* SIZE = "SIZE";
char* BLOCKS = "BLOCKS";
char* BLOCK_SIZE = "BLOCK_SIZE​";

char* PARTITIONS = "PARTITIONS";
char* CONSISTENCY = "CONSISTENCY";
char* COMPACTION_TIME = "COMPACTION_TIME";



char* concat_dirs(char* start, char* end) {
    return concat(concat(start, end), "/");
}


// PATH
char* to_partition_file(int partition) {
    return concat(string_itoa(partition), PARTITION_EXTENSION);
}

char* to_block_file(int blockNumber) {
    return concat(string_itoa(blockNumber), BLOCK_EXTENSION);
}

char* table_path(char* tableName) {
    return concat_dirs(concat(BASE_PATH, TABLE_BASE_PATH), tableName);
}

char* metadata_base_path() {
    return concat(BASE_PATH, METADATA_BASE_PATH);
}

char* blocks_base_path() {
    return concat(BASE_PATH, BLOCKS_BASE_PATH);
}

char* partition_path(char* tableName, int partition) {
    return concat(table_path(tableName), to_partition_file(partition));
}

char* block_path(int block) {
    return concat(blocks_base_path(), to_block_file(block));
}

char* temp_path(char* tableName, char* tempFile) {
    return concat(table_path(tableName), tempFile);
}

char* table_metadata_path(char* tableName) {
    return concat(table_path(tableName), METADATA_FILE_NAME);
}

char* metadata_path() {
    return concat(metadata_base_path(), METADATA_FILE_NAME);
}

char* bitmap_path() {
    return concat(metadata_base_path(), BITMAP_FILE_NAME);
}




// FILE
void create_file(char* path) {
    FILE* file = fopen(path, "w");
    fclose(file);
}

void create_dir(char* path) {
    mkdir(path, S_IRWXU);
}

void delete_from_path(char* path) {
    system(concat("rm -rf ", path));
}


void clean_file(char* path) {
    delete_from_path(path);
    create_file(path);
}

void clean_dir(char* path) {
    delete_from_path(path);
    create_dir(path);
}


char* read_file(char* path, int size) {
    FILE* file = fopen(path, "r"); 
    char* result = string_repeat('\0', size);
    char* input = string_repeat('\0', 2);
    while(fread(input, sizeof(char), 1, file)) {
        string_append(&result, input);
    }
    fclose(file);
    free(input);
    return result;
}

void write_file(char* path, char* data) {
    FILE* file = fopen(path, "w");
    // fwrite(data, string_length(data) + 1, 1, file); 
    fwrite(data, sizeof(char), string_length(data) + 1, file); 
    fclose(file);
}


t_list* all_files_in(char* directory) {
    t_list* results = list_create();
    DIR *folder = opendir(directory);
    struct dirent *entry;
    while( (entry=readdir(folder)) )
    {
        if (!string_starts_with(entry->d_name, "."))
            list_add(results, entry->d_name);
    }
    return results;
}


// DOMAIN

bool is_temp(char* file) {
    return string_contains(file, TEMP_EXTENSION) || string_contains(file, COMPACT_TEMP_EXTENSION);   
}

t_list* all_temp_files(char* table) {
    return list_filter(all_files_in(table_path(table)), (void*) is_temp);
}

char* next_temp_file(char* table) {
    int n = list_size(all_temp_files(table)) + 1;
    return concat(string_itoa(n), TEMP_EXTENSION);
}



t_config* read_metadata() {
    return config_create(metadata_path());
}

int read_blocks_count() {
    t_config* metadata = read_metadata();
    int blocksCount = config_get_int_value(metadata, BLOCK_SIZE); //TODO: Por qué no anda BLOCKS !!?!?
    config_destroy(metadata);
    return blocksCount;
}

int read_block_size() {
    t_config* metadata = read_metadata();
    int blocksCount = config_get_int_value(metadata, BLOCK_SIZE);
    config_destroy(metadata);
    return blocksCount;
}






int bitmap_size() {
    return read_blocks_count() / 8; //TODO: validate
}

t_bitarray* read_bitmap() {
    int size = bitmap_size();
    char* bits = read_file(bitmap_path(), size);
    t_bitarray* bitmap = bitarray_create_with_mode(bits, size, MSB_FIRST);
    return bitmap;
}

void write_bitmap(int blockNumber) {
    int size = bitmap_size();
    char* bits = read_file(bitmap_path(), size);
    t_bitarray* bitmap = bitarray_create_with_mode(bits, size, MSB_FIRST);
    bitarray_set_bit(bitmap, blockNumber);
    write_file(bitmap_path(), bits);
    bitarray_destroy(bitmap);
    free(bits);
}







void write_block(int blockNumber, char* data) {
    write_bitmap(blockNumber);
    write_file(block_path(blockNumber), data);
}

char* read_block(int blockNumber) {
    return read_file(block_path(blockNumber), read_block_size());
}

int next_block() {
    t_bitarray* bitmap = read_bitmap();
    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++) //TODO: Times
    {
        if (!bitarray_test_bit(bitmap, i)) 
            return i;
    }
    bitarray_destroy(bitmap);
    return -1;
}

float size_bytes(char* data) {
    return 1.0 * sizeof(char) * string_length(data);
}

t_list* write_data(char* data) {
    float fsize = size_bytes(data);
    int blockSize = read_block_size();
    float fBlocks = fsize / blockSize;    
    int size = fsize;
    int blocksCount = fBlocks;

    if (fBlocks != blocksCount) blocksCount++;
    
    t_list* blocks = list_create();
    for (int i = 0; i < blocksCount; i++)
    {
        int n = next_block();
        write_block(n, string_substring(data, i * blockSize, blockSize)); //TODO
        list_add(blocks, n); 
    }
    return blocks;
}


void write_registry(FILE* file, Registry* data) { //TODO: Transformar a string
    fwrite(data, sizeof(Registry), 1, file); 
}

t_list* read_data_file(char* path) { //TODO: sacar
    t_list* registries = list_create();
    FILE* infile = fopen(path, "r"); 
    Registry* input;
    while(fread(input = malloc(sizeof(Registry)), sizeof(Registry), 1, infile)) {
        list_add(registries, input);
    }
    return registries;
}

/// TABLE ///
t_config* read_table_metadata(char* table) {
    return config_create(table_metadata_path(table));
}

void create_table_dir(char* tableName) {
    create_dir(table_path(tableName));
}

void create_table_metadata_file(char* tableName, TableMetadata metadata){
    create_file(table_metadata_path(tableName));
    t_config* config = read_table_metadata(tableName);
    config_set_value(config, CONSISTENCY, metadata.consistency);
    config_set_value(config, PARTITIONS, string_itoa(metadata.partitions));
    config_set_value(config, COMPACTION_TIME, string_itoa(metadata.compactionTime));
    config_save(config);
    config_destroy(config);
}


int read_table_metadata_partitions(char* table) {
    t_config* metadata = read_table_metadata(table);
    return config_get_int_value(metadata, PARTITIONS);
}

/// TABLE ///






/// PARTITION ///
int partition_for(char* table, int key) {
    return key % read_table_metadata_partitions(table);
}

t_config* read_partition(char* table, int partition) {
    return config_create(partition_path(table, partition));
}

int read_partition_size(char* table, int partition) {
    t_config* config = read_partition(table, partition);
    int size = config_get_int_value(config, SIZE);
    config_destroy(config);
    return size;
}

char** read_partition_blocks(char* table, int partition) {
    t_config* config = read_partition(table, partition);
    char** blocks = config_get_array_value(config, BLOCKS);
    config_destroy(config);
    return blocks;
}

void create_partition_file(char* tableName, int partition) { 
    create_file(partition_path(tableName, partition));
    t_config* config = read_partition(tableName, partition);
    config_set_value(config, SIZE, "0");
    config_set_value(config, BLOCKS, "[]");
    config_save(config);
    config_destroy(config);
}

void create_partition_files(char* tableName, int partitionsCount) {
    for (int i = 0; i < partitionsCount; i++) //TODO: Times
    {
        create_partition_file(tableName, i);
    }
}

void array_add(char** array, char* data) {
    array = realloc(array, sizeof(char*) * (array_length(array) + 2));
	array[array_length(array) + 1] = NULL;
    array[array_length(array)] = data;
}

void write_partition(char* table, int partition, char* data) {
    t_list* writedBlocks = write_data(data);
    int newSize = read_partition_size(table, partition) + size_bytes(data);
    char** newBlocks = read_partition_blocks(table, partition);

    list_iterate(writedBlocks, (void*)
        lambda(void, (int n), {
            array_add(newBlocks, string_itoa(n));
        })
    );
    // newBlocks[2] = "1";
    printf("LENGTH: %d\n", array_length(newBlocks));
    printf("BLOCKS: %s\n", newBlocks[0]);
    printf("BLOCKS: %s\n", newBlocks[1]);
    printf("BLOCKS: %s\n", newBlocks[2]);

    t_config* partitionData = read_partition(table, partition);
    config_set_value(partitionData, SIZE, string_itoa(newSize));
    config_set_value(partitionData, BLOCKS, newBlocks);
    config_save(partitionData);
    config_destroy(partitionData);
}

/// PARTITION ///


///
void set_base_path(char* base) {
    BASE_PATH = base;
}

bool exist_table(char* table) {
    return read_table_metadata(table) != NULL; // !access(table_path(table), F_OK);
}

t_list* search_key_in_partitions(char* table, int key) {
    return read_data_file(partition_path(table, partition_for(table, key))); //TODO: usar read_partition
}

t_list* read_all_temp_files(char* table) {
    t_list* registries = list_create();
    
    
    list_iterate(all_temp_files(table), (void*)
        lambda(void, (char* tempFile), {
            t_list* registries_in_temp = read_data_file(temp_path(table, tempFile));
            list_add_all(registries, registries_in_temp);
        })
    );
    
    return registries; 
}




void create_new_block(int block) {
    create_file(block_path(block));
}

void create_db_blocks() {
    clean_dir(blocks_base_path());

    int blocks_count = read_blocks_count();
    for (int i = 0; i < blocks_count; i++) //TODO: Times
    {
        create_new_block(i);
    }

    clean_file(bitmap_path());

    int size = blocks_count / 8;
    char* bits = string_new();
    t_bitarray* bitmap = bitarray_create_with_mode(bits, size, MSB_FIRST);

    bitarray_destroy(bitmap);
    free(bits);
}

void write_in_partition(char* table, Registry* registry) {
    int partition = partition_for(table, registry->key);
    char* data = stringify(registry);
    write_partition(table, partition, data);
}

//TODO: Check to use create_file
void create_new_temp(char* table, t_list* registries) {
    char* path = temp_path(table, next_temp_file(table));
    FILE* file = fopen(path, "w");
    list_iterate(registries, (void*)
        lambda(void, (Registry* data), {
            write_registry(file, data);
        })
    );
    fclose(file);
}

void create_new_table(char* table, TableMetadata metadata) {
    create_table_dir(table);
    create_table_metadata_file(table, metadata);
    create_partition_files(table, metadata.partitions);
}

void delete_table(char* table) {
    delete_from_path(table_path(table));
}
