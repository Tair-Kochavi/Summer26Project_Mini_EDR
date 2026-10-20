#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    std::cout << "=== [TEST 5] SHELLCODE PREVENTER (W^X) & EXECVE ===\n";

    // PHASE 1: Legal RW allocation (Should succeed)
    std::cout << "--- [5.1] Allocating legal RW memory via mmap ---\n";
    void* legal_mem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (legal_mem == MAP_FAILED) {
        std::cerr << "[FAIL] EDR accidentally blocked legal RW mmap!\n";
    } else {
        std::cout << "[PASS] Legal RW mmap succeeded.\n";
    }

    // PHASE 2: Illegal RWX allocation via mmap (Should be BLOCKED)
    std::cout << "--- [5.2] Attempting illegal RWX shellcode allocation via mmap ---\n";
    void* rwx_mem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (rwx_mem == MAP_FAILED) {
        perror("[PASS] Illegal RWX mmap blocked");
    } else {
        std::cerr << "[FAIL] EDR allowed RWX mmap!\n";
        munmap(rwx_mem, 4096);
    }

    // PHASE 3: Illegal RWX escalation via mprotect on legal_mem (Should be BLOCKED)
    std::cout << "--- [5.3] Attempting stealth RW -> RWX escalation via mprotect ---\n";
    if (mprotect(legal_mem, 4096, PROT_READ | PROT_WRITE | PROT_EXEC) < 0) {
        perror("[PASS] Illegal RWX mprotect blocked");
    } else {
        std::cerr << "[FAIL] EDR allowed RWX mprotect escalation!\n";
    }
    munmap(legal_mem, 4096);

    // PHASE 4: Fork + execve under Hydra
    std::cout << "--- [5.4] Forking and calling execve(/bin/echo) ---\n";
    pid_t pid = fork();
    if (pid == 0) {
        char* args[] = {const_cast<char*>("/bin/echo"), const_cast<char*>("=== [TEST 5 COMPLETE] Hydra + Execve Passed! ==="), nullptr};
        execve(args[0], args, nullptr);
        return 0;
    }
    waitpid(pid, nullptr, 0);
    return 0;
}