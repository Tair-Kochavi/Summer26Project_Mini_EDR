#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>


pid_t trace_target(pid_t pid) {
    int status;
    struct user_regs_struct regs;

    wait(&status);
    while (!WIFEXITED(status)) {
        //wait for the program to use a syscall and then print the syscall number
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        if (!WIFEXITED(status)) {
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            std::cout << "syscall used: " << regs.orig_rax << "\n";
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
    }

    return status;
}

pid_t run_target(char* args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execve(args[0], args, NULL);
    }
    if (pid > 0) {
        return trace_target(pid);
    }
    else {
        std::cerr << "fork failed\n";
        return -1;
    }
}




int main(int argc, char* argv[]) {
    char **args_for_exec = argv + 1;
    if (run_target(args_for_exec) == -1) {
        return -1;
    }
    return 0;
}