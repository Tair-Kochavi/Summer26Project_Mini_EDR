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

    while (!WEIFEXITED(status)) {
        //wait for the program to use a syscall and then print the syscall number
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        std::cout << "syscall used: " << regs->orig_rax << "\n";
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);


        wait(&status)
    }
}

pid_t run_target(const char* path) {
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execve(path, , );
    }
    if (pid > 0) {
        trace_target(pid);
    }
    else {
        std::cerr << "fork failed\n";
    }
}




int main(int argc, char* argv[]) {
    run_target();
}