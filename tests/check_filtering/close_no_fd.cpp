#include <unistd.h>
#include <iostream>

int main() {
    std::cout << "Attempting to close a non-existent file descriptor...\n";
    close(9999); // Syscall 3 (In your map, but invalid argument)
    return 0;
}