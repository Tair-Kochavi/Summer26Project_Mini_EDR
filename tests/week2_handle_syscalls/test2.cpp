#include <unistd.h>
#include <iostream>

int main() {
    const char* path = "/bin/echo";
    char* argv[] = { (char*)"echo", (char*)"Hello from the execve test!", nullptr };
    char* envp[] = { nullptr };

    // Triggers an explicit 'execve' system call (Syscall 59)
    execve(path, argv, envp);

    // If execve succeeds, the process memory is replaced and this line is never reached.
    std::cerr << "execve failed\n";
    return 1;
}