#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

int main() {
    std::cout << "Attempting blocked mmap (Read/Write/Execute)...\n";
    
    void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
    } else {
        std::cout << "SUCCESS: Memory allocated! (This should not happen)\n";
        munmap(ptr, 4096);
    }
    return 0;
}