#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int main() {
    std::cout << "=== [TEST 1] STARTING HYDRA LIFECYCLE & SIGNAL TEST ===\n";

    // PHASE 1: Child dies via uncaught Segmentation Fault (SIGSEGV)
    pid_t c1 = fork();
    if (c1 == 0) {
        std::cout << "[Child 1] About to trigger SIGSEGV...\n";
        int* bad_ptr = nullptr;
        *bad_ptr = 42; // Crash! Triggers WIFSIGNALED
        return 0;
    }
    waitpid(c1, nullptr, 0);
    std::cout << "=== [PHASE 1 PASSED] EDR survived Child 1 SIGSEGV ===\n";

    // PHASE 2: Child dies via external SIGKILL
    pid_t c2 = fork();
    if (c2 == 0) {
        std::cout << "[Child 2] Going to sleep, waiting to be killed...\n";
        sleep(10);
        return 0;
    }
    usleep(100000); // Give child 100ms to enter sleep syscall
    kill(c2, SIGKILL); // Murder child 2 -> Triggers WIFSIGNALED
    waitpid(c2, nullptr, 0);
    std::cout << "=== [PHASE 2 PASSED] EDR survived Child 2 SIGKILL ===\n";

    // PHASE 3: vfork() and normal exit (WIFEXITED)
    pid_t c3 = vfork();
    if (c3 == 0) {
        // Inside vfork, safely invoke a raw syscall then _exit immediately
        open("safe_vfork.txt", O_RDONLY);
        _exit(0);
    }
    waitpid(c3, nullptr, 0);
    std::cout << "=== [PHASE 3 PASSED] EDR tracked vfork() and exited cleanly ===\n";

    std::cout << "=== [TEST 1 COMPLETE] If EDR exits now, WIFSIGNALED works! ===\n";
    return 0;
}