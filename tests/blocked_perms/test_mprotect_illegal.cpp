#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    // Stage 1: Safe allocation
    void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("Initial mmap failed");
        return 1;
    }

    std::cout << "Attempting blocked mprotect (Upgrading to Read/Write/Execute)...\n";
    
    // Stage 2: Malicious permission shift
    if (mprotect(ptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("mprotect failed");
    } else {
        std::cout << "SUCCESS: Memory permissions changed! (This should not happen)\n";
    }
    
    munmap(ptr, 4096);
    return 0;
}