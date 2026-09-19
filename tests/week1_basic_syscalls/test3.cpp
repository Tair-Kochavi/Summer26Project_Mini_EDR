#include <stdlib.h>

int main() {
    // Allocate 1 Megabyte of memory
    void *ptr = malloc(1024 * 1024);
    if (ptr != NULL) {
        free(ptr);
    }
    return 0;
}