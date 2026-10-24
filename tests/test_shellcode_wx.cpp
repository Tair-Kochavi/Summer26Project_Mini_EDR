#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    std::cout << "=== [TEST 5] SHELLCODE PREVENTER (W^X) & EXECVE ===\n";

    // 5.1: Legal RW allocation
    std::cout << "--- [5.1] Allocating legal RW memory via mmap ---\n";
    void* legal_mem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (legal_mem != MAP_FAILED) std::cout << "[PASS] Legal RW mmap succeeded.\n";

    // 5.2: Illegal RWX allocation via mmap
    std::cout << "--- [5.2] Attempting illegal RWX shellcode allocation via mmap ---\n";
    void* rwx_mem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (rwx_mem == MAP_FAILED) perror("[PASS] Illegal RWX mmap blocked");

    // 5.3: Illegal RWX escalation via mprotect
    std::cout << "--- [5.3] Attempting stealth RW -> RWX escalation via mprotect ---\n";
    if (mprotect(legal_mem, 4096, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("[PASS] Illegal RWX mprotect blocked");
    }
    munmap(legal_mem, 4096);

    // 5.4: NEW - Attempting to execve a blacklisted binary ("dir/secret.txt")
    std::cout << "--- [5.4] Attempting blocked execve(dir/secret.txt) ---\n";
    pid_t bad_pid = fork();
    if (bad_pid == 0) {
        char* bad_args[] = {const_cast<char*>("dir/secret.txt"), nullptr};
        if (execve(bad_args[0], bad_args, nullptr) < 0) {
            perror("[PASS] Blocked execve failed as expected");
        }
        return 0;
    }
    waitpid(bad_pid, nullptr, 0);

    // 5.5: Allowed Fork + execve(/bin/echo) -> Must show TARGET: /bin/echo!
    std::cout << "--- [5.5] Forking and calling allowed execve(/bin/echo) ---\n";
    pid_t pid = fork();
    if (pid == 0) {
        char* args[] = {const_cast<char*>("/bin/echo"), const_cast<char*>("=== [TEST 5 COMPLETE] Hydra + Execve Passed! ==="), nullptr};
        execve(args[0], args, nullptr);
        return 0;
    }
    waitpid(pid, nullptr, 0);
    return 0;
}