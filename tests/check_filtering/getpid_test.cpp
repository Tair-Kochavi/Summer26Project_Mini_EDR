#include <iostream>
#include <unistd.h>

int main() {
    pid_t pid = getpid(); // Syscall 39 (Not in your map)
    std::cout << "My PID is: " << pid << "\n";
    return 0;
}