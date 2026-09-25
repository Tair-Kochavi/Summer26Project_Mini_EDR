#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    // Allocate exactly 4096 bytes (a standard memory page)
    char* bad_string = (char*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    // Fill it entirely with 'A's without a single null terminator (\0)
    memset(bad_string, 'A', 4096);
    
    // Pass the infinite string to openat
    open(bad_string, O_RDONLY);
    
    return 0;
}