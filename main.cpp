#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include "syscall_format.h"
#define PATH_MAX 4096


std::string get_path(pid_t pid, unsigned long long addr);
void compress_info(syscall_info& info, unsigned long long num, const std::string& path, const std::string& warning);


/**
 *
 * @param regs register values of the child process after it called a syscall
 * @param pid the pid of the child process
 */
void handle_syscall(syscall_info& info, const user_regs_struct& regs, pid_t pid) {
    switch (regs.orig_rax) {
        case 2:  //open syscall
        {
            std::string path = get_path(pid, regs.rdi);
            if (path == "") {
                compress_info(info, regs.orig_rax, path, "attempted to access illegal path");
            }
            compress_info(info, regs.orig_rax, path, "");
        }
            break;
        case 59: //execve syscall
        {
            std::string path = get_path(pid, regs.rdi);
            if (path == "") {
                compress_info(info, regs.orig_rax, path, "attempted to access illegal path");
            }
            compress_info(info, regs.orig_rax, path, "");
        }
            break;
        case 257: //openat syscall
        {
            std::string path = get_path(pid, regs.rsi);
            if (path == "") {
                compress_info(info, regs.orig_rax, path, "attempted to access illegal path");
            }
            compress_info(info, regs.orig_rax, path, "");
        }
            break;
        default: compress_info(info, regs.orig_rax, "", "");
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
        errno = 0;
        long res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);

        if (res == -1 && errno != 0) {       //error occurred in ptrace
            return "";
        }
        bytes += 8;
        addr += 8;
        for (int i = 0; i < 8; i++) {
            long mask = 0xFF;
            to_append = res & mask;  //extracting bottom byte
            if (to_append == 0) {   //got to a null terminator
                return path;
            }
            path += static_cast<char>(to_append);

            res = res >> 8;
        }
    }
    return ""; //not null terminated - path too long
}


void compress_info(syscall_info& info, unsigned long long num, const std::string& path, const std::string& warning) {
    info.path = path;
    info.sys_num = num;
    info.warning = warning;
}




pid_t trace_target(pid_t pid) {
    int status;
    struct user_regs_struct regs;

    wait(&status);
    while (!WIFEXITED(status)) {
        syscall_info info;

        //wait for the program to use a syscall and then print the syscall number
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        if (!WIFEXITED(status)) {
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            handle_syscall(info, regs, pid);
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        if (!WIFEXITED(status)) {
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            info.ret_val = static_cast<long>(regs.rax);
            log_syscall(info);
        }
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