#include <commons/string.h>

// UTILS
char *concat(char *start, char *end)
{
    return string_from_format("%s%s", start, end);
}

char **words(char *text)
{
    return string_split(text, " ");
}

void times_do(int n, void (*closure)(int))
{
    for (int i = 0; i < n; i++)
    {
        closure(i);
    }
}

int array_length(void** array) {
    int n = 0;
    while (array[n])
    {
        n++;
    }
    return n;
}