#ifndef UTILS
#define UTILS

#define lambda(l_ret_type, l_arguments, l_body)         \
  ({                                                    \
    l_ret_type l_anonymous_functions_name l_arguments   \
      l_body                                            \
    &l_anonymous_functions_name;                        \
  })

#define OK "OK"

char* concat(char* start, char* end);
char** words(char* text);
void times_do(int n, void (*closure)(int));
int array_length(void** array);

#endif