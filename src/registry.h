#ifndef REGISTRY
#define REGISTRY

typedef struct {
   long timestamp;
   int key;
   char value[50];
} Registry;

Registry* new_data(long timestamp, int key, char* value);
Registry* new_data_now(int key, char* value);

char* stringify(Registry* registry);

void print_data(Registry* data);

#endif