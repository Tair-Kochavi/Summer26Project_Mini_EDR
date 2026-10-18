#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    // Stage 1: Safe allocation (Read/Write)
    std::cout << "Attempting legal mmap (Read/Write)...\n";
    void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (ptr == MAP_FAILED) {
        perror("Initial mmap failed");
        return 1;
    }
    
    // Stage 2: Safe permission shift (Read-Only)
    std::cout << "Attempting legal mprotect (Downgrading to Read-Only)...\n";
    if (mprotect(ptr, 4096, PROT_READ) < 0) {
        perror("mprotect failed");
    } else {
        std::cout << "SUCCESS: Memory permissions changed safely.\n";
    }
    
    munmap(ptr, 4096);
    return 0;
}