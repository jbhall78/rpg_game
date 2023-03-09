#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *
Malloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == NULL) {
        fprintf(stderr, "Can not allocation %ld bytes\n", size);
        exit(1);
    }
    memset(ptr, '\0', size);

    return ptr;
}

void *
Calloc(size_t elements, size_t size)
{
    void *ptr;

    if ((ptr = calloc(elements, size)) == NULL) {
        fprintf(stderr, "Can not allocate %ld elements of %ld size\n", elements, size);
        exit(1);
    }

    return ptr;
}

void
Free(void *ptr)
{
    free(ptr);
}
