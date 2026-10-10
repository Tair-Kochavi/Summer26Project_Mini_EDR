#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include "blacklist.h"
#include "syscall_format.h"
#define PATH_MAX 4096


std::string get_path(pid_t pid, unsigned long long addr);
void compress_info(syscall_info& info, unsigned long long num, unsigned long long addr, pid_t pid, user_regs_struct& regs);
std::string get_connection(pid_t pid, unsigned long long addr);

/**
 *
 * @param regs register values of the child process after it called a syscall
 * @param pid the pid of the child process
 */
void handle_syscall(syscall_info& info, user_regs_struct& regs, pid_t pid) {
    switch (regs.orig_rax) {
        case 2:  //open syscall
        {
           compress_info(info, regs.orig_rax, regs.rdi, pid, regs);
        }
            break;
        case 59: //execve syscall
        {
            compress_info(info, regs.orig_rax, regs.rdi, pid, regs);
        }
            break;
        case 257: //openat syscall
        {
           compress_info(info, regs.orig_rax, regs.rsi, pid, regs);
        }
            break;
        default: compress_info(info, regs.orig_rax, 0, -1, regs);
    }
}



void compress_info(syscall_info& info, unsigned long long num, unsigned long long addr, pid_t pid,
    user_regs_struct& regs){
    if (pid == -1) {   //if it's a general syscall - pid will be -1
        info.warning = "";
        info.target = "";
        info.sys_num = num;
        return;
    }
    std::string path = get_path(pid, addr);
    if (path == "") {
        info.warning = "attempted to access illegal path";
    } else {
        info.warning = "";
    }
    if (check_path(path)) {
        info.blocked = true;
        regs.orig_rax = -1;   //intentionally changing syscall service num to out of range num
        ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    }
    info.target = path;
    info.sys_num = num;
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



std::string get_connection(pid_t pid, unsigned long long addr) {
    std::string port = "";
    std::string IP_string = "";
    errno = 0;
    long res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);

    if (res == -1 && errno != 0) {       //error occurred in ptrace
        return "";
    }
    port = std::to_string(res & 0xFFFF0000); //extracting the port - the next two bytes

    if ((res & 0xFFFF) == AF_INET) { //checking in the first two bytes if the version is IPv4
        IP_string = std::to_string(res & 0xFFFFFFFF00000000); //extracting the IP - the next four bytes
        return (port + ":" + IP_string);
    }

    //handling IPv6
    addr += 8;
    errno = 0;
    res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
    if (res == -1 && errno != 0) {       //error occurred in ptrace
        return "";
    }

    addr += 8;
    errno = 0;
    res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
    if (res == -1 && errno != 0) {       //error occurred in ptrace
        return "";
    }

}






pid_t trace_target(pid_t pid) {
    int status;
    struct user_regs_struct regs;

    wait(&status);
    while (!WIFEXITED(status)) {
        syscall_info info;

        //wait for the program to use a syscall
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
            if (info.blocked) {
                regs.rax = info.error_code;      //set error code
                ptrace(PTRACE_SETREGS, pid, NULL, &regs);
            }
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