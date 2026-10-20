#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>

#include "blacklist.h"
#include "syscall_format.h"
#include "nw_blacklist.h"
#define PATH_MAX 4096
#define MAX_V4 16  //max IP address characters for IPv4
#define MAX_V6 46 //max IP address characters for IPv6


std::string get_path(pid_t pid, unsigned long long addr);
void process_file(syscall_info& info, unsigned long long addr, pid_t pid, user_regs_struct& regs);
std::string get_connection(pid_t pid, unsigned long long addr);
bool peek_for_connection(pid_t pid, unsigned long long addr, char* info, int offset);
void process_network(syscall_info& info, unsigned long long addr, pid_t pid,
    user_regs_struct& regs);
void check_prot(unsigned long long flags, syscall_info& info, user_regs_struct& regs, pid_t pid);


/**
 *
 * @param regs register values of the child process after it called a syscall
 * @param pid the pid of the child process
 */
void handle_syscall(syscall_info& info, user_regs_struct& regs, pid_t pid) {
    info.sys_num = regs.orig_rax;
    info.pid = pid;
    switch (regs.orig_rax) {
        case 2:  //open syscall
        {
           process_file(info, regs.rdi, pid, regs);
        }
            break;
        case 59: //execve syscall
        {
            process_file(info, regs.rdi, pid, regs);
        }
            break;
        case 257: //openat syscall
        {
           process_file(info, regs.rsi, pid, regs);
        }
            break;
        case 42: //connect syscall
        {
            process_network(info, regs.rsi, pid, regs);
        }
            break;
        case 9: //mmap syscall
        {
            check_prot(regs.rdx, info, regs, pid);
        }
            break;
        case 10: //mprotect syscall
        {
            check_prot(regs.rdx, info, regs, pid);
        }
            break;
        default:
            info.warning = "";
            info.target = "";
    }
}


void check_prot(unsigned long long flags, syscall_info& info, user_regs_struct& regs, pid_t pid) {
    info.warning = "";
    info.target = "";
    if ((flags & PROT_WRITE) && (flags & PROT_EXEC)) {
        info.blocked = true;
        info.error_code = -EPERM;
        regs.orig_rax = -1;   //intentionally changing syscall service num to out of range num
        ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    }
}


void process_network(syscall_info& info, unsigned long long addr, pid_t pid,
                     user_regs_struct& regs) {
    std::string connection = get_connection(pid, addr);
    if (connection == "") {
        info.warning = "invalid network pointer";
    } else {
        info.warning = "";
    }
    if (check_connection(connection)) { //process tried making an illegal connection
        info.blocked = true;
        info.error_code = -EPERM; //connection is in the blacklist therefore not permitted
        regs.orig_rax = -1;   //intentionally changing syscall service num to out of range num
        ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    }
    info.target = connection;
}


void process_file(syscall_info& info, unsigned long long addr, pid_t pid,
    user_regs_struct& regs){
    std::string path = get_path(pid, addr);
    if (path == "") {
        info.warning = "attempted to access illegal path";
    } else {
        info.warning = "";
    }
    if (check_path(path)) {
        info.blocked = true;
        info.error_code = -EACCES;
        regs.orig_rax = -1;   //intentionally changing syscall service num to out of range num
        ptrace(PTRACE_SETREGS, pid, NULL, &regs);
    }
    info.target = path;
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
    char info[24];
    if (!peek_for_connection(pid, addr, info, 0)) {
        return "";
    }

    uint16_t port, ipv;
    memcpy(&port, info + 2, 2); //copying the port from the info buffer
    memcpy(&ipv, info, 2); //copying the IP version from the info buffer

    if (ipv == AF_INET) { //checking in the first two bytes if the version is IPv4
        char buff[MAX_V4];
        inet_ntop(AF_INET, info + 4, buff, MAX_V4);
        return std::string(buff) + ":" + std::to_string(ntohs(port));
    }

    //handling IPv6
    addr += 8;
    if (!peek_for_connection(pid, addr, info, 8)) {
        return "";
    }

    addr += 8;
    if (!peek_for_connection(pid, addr, info, 16)) {
        return "";
    }

    char buff[MAX_V6];
    inet_ntop(AF_INET6, info + 8, buff, MAX_V6);
    return std::string(buff) + ":" + std::to_string(ntohs(port));
}


bool peek_for_connection(pid_t pid, unsigned long long addr, char* info, int offset) {
    errno = 0;
    long res = ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
    if (res == -1 && errno != 0) {       //error occurred in ptrace
        return false;
    }
    memcpy(info + offset, &res, 8);
    return true;
}





pid_t trace_target(pid_t pid) {
    int status;
    struct user_regs_struct regs;
    std::unordered_map<pid_t, syscall_info> processes;
    syscall_info info;
    processes.insert({pid, info});

    wait(&status);
    ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEEXEC);
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

    while (!processes.empty()) {

        pid_t curr = waitpid(-1, &status, __WALL);
        int signal = WSTOPSIG(status);

        if (WIFEXITED(status) || WIFSIGNALED(status)) { //process exited or terminated by an uncaught signal
            processes.erase(curr);
            continue;               //in order to not get to PTRACE_SYSCALL
        }
        syscall_info& current_info = processes.at(curr);
        if (WIFSTOPPED(status)) {
            int event = status >> 16;
            if (event == PTRACE_EVENT_CLONE || event == PTRACE_EVENT_FORK || event == PTRACE_EVENT_VFORK) {
                syscall_info new_info;
                unsigned long long new_pid;
                if (ptrace(PTRACE_GETEVENTMSG, curr, 0, &new_pid) == 0) {
                    processes.insert({new_pid, new_info});
                }
            }
            //if execve succeeded, reset the current_info
            else if (event == PTRACE_EVENT_EXEC) {
                current_info.blocked = false;
                current_info.warning = "";
                current_info.target = "";
            }
            else if (signal == SIGTRAP) {
                //handle syscall entry
                if (!current_info.in_syscall) {
                    ptrace(PTRACE_GETREGS, curr, NULL, &regs);
                    handle_syscall(current_info, regs, curr);
                    current_info.in_syscall = true;
                }
                //handle syscall exit
                else {
                    ptrace(PTRACE_GETREGS, curr, NULL, &regs);
                    if (current_info.blocked) {
                        regs.rax = current_info.error_code;   //set error code
                        ptrace(PTRACE_SETREGS, curr, NULL, &regs);
                    }
                    current_info.ret_val = static_cast<long>(regs.rax);
                    log_syscall(current_info);
                    current_info.in_syscall = false;
                    //resetting current_info
                    current_info.blocked = false;
                    current_info.warning = "";
                    current_info.target = "";
                }
            }
        }
        if (signal == SIGTRAP || signal == SIGSTOP) { //if it's a syscall boundry or new process creation
            signal = 0;
        }
        ptrace(PTRACE_SYSCALL, curr, NULL, (void*)(intptr_t)signal);
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