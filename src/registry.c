#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

typedef struct {
   long timestamp;
   int key;
   char value[50];
} Registry;

Registry* new_data(long timestamp, int key, char* value) {
    Registry* data = malloc(sizeof(Registry));
    data -> timestamp = timestamp;
    data -> key = key;
    strcpy(data -> value, value);
    return data;
}

Registry* new_data_now(int key, char* value) {
    return new_data(time(NULL), key, value);
}

char* stringify(Registry* registry) {
    return string_from_format("%ld;%d;%s", registry->timestamp, registry->key, registry->value); 
}

// Registry* parse(char* data) {
//     char** splittedData = string_split(data, ";");
//     return new_data(splittedData[0], splittedData[1], splittedData[2]);
// }

void print_data(Registry* registry) {
    printf("tymestamp = %ld key = %d  value = %s\n", registry->timestamp, registry->key, registry->value); 
}
