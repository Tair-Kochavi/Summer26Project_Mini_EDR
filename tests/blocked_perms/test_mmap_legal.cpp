#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    std::cout << "Attempting legal mmap (Read/Write)...\n";

    void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
        perror("mmap failed");
    } else {
        std::cout << "SUCCESS: Memory allocated safely.\n";
        munmap(ptr, 4096);
    }
    return 0;
}