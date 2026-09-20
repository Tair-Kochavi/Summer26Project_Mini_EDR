#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#define PATH_MAX 4096


/**
 *
 * @param regs register values of the child process after it called a syscall
 * @param pid the pid of the child process
 */
void handle_syscall(const user_regs_struct& regs, pid_t pid) {
    switch (regs.orig_rax) {
        case 2:  //open syscall
            std::cout << "memory address of path: " << regs.rdi <<"\n";
            break;
        case 59: //execve syscall
            std::cout << "memory address of path: " << regs.rdi <<"\n";
            break;
        case 257: //openat syscall
            std::cout << "memory address of path: " << regs.rsi << "\n";
            break;
    }
}




/**
 *
 * @param pid child process pid
 * @param addr address of the path to the file the child process wants to use
 * @return a string of the content in addr (path to the file)
 */
std::string get_path(pid_t pid, unsigned long long addr) {
    int bytes = 0;
    std::string path = "";
    long to_append = 0;
    while (bytes < PATH_MAX) {
        long res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
        bytes += 8;
        for (int i = 0; i < 8; i++) {
            long mask = 0x0000000F;
            to_append = res & mask;
            if (to_append == 0) {
                break;
            }
            path += std::to_string(to_append);

            res = res >> 8;
        }
    }
    if (to_append != 0) {  //not null terminated - path too long

    }
    return path;
}




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
            handle_syscall(regs, pid);
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
        if (execve(args[0], args, NULL) == -1) {
            exit(1);
        }
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