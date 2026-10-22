#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <cstring>

int main() {
    std::cout << "=== [TEST 3] FILE & MEMORY READING EDGE CASES (IN CHILD) ===\n";

    pid_t pid = fork();
    if (pid == 0) {
        // 1. Unmapped syscall (Should be completely invisible in EDR logs)
        std::cout << "--- [3.1] Calling unmapped getpid() ---\n";
        pid_t my_pid = getpid();
        std::cout << "Child PID is: " << my_pid << " (Syscall 39 should NOT appear above)\n";

        // 2. Legacy open (Syscall 2 -> rdi) on blocked file
        std::cout << "--- [3.2] Legacy SYS_open on secret.txt ---\n";
        long fd1 = syscall(SYS_open, "secret.txt", O_RDONLY, 0);
        if (fd1 < 0) perror("Legacy open blocked as expected");

        // 3. Relative path bypass attempt on openat (Syscall 257 -> rsi)
        std::cout << "--- [3.3] Relative path bypass on dir/subdir/no_open.txt ---\n";
        int fd2 = open("dir/subdir/no_open.txt", O_RDONLY);
        if (fd2 < 0) perror("Relative openat blocked as expected");

        // 4. Null pointer trap
        std::cout << "--- [3.4] Passing NULL pointer to SYS_openat ---\n";
        syscall(SYS_openat, AT_FDCWD, nullptr, O_RDONLY, 0);

        // 5. 4096-byte non-null-terminated string (PATH_MAX overflow)
        std::cout << "--- [3.5] Passing 4096-byte unterminated string ---\n";
        char* bad_str = static_cast<char*>(mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        memset(bad_str, 'A', 4096);
        open(bad_str, O_RDONLY);

        // 3.6: NEW - Trailing slash bypass attempt on blacklisted files
        std::cout << "--- [3.6] Trailing slash bypass on secret.txt/ and dir/no_open.txt/// ---\n";
        int fd3 = open("secret.txt/", O_RDONLY);
        if (fd3 < 0) perror("Trailing slash 1 blocked as expected");
        int fd4 = open("dir/no_open.txt///", O_RDONLY);
        if (fd4 < 0) perror("Trailing slash 2 blocked as expected");

        return 0;
    }

    waitpid(pid, nullptr, 0);
    std::cout << "=== [TEST 3 COMPLETE] ===\n";
    return 0;
}