
#pragma once
#include <string>


struct syscall_info {
    unsigned long long sys_num = 0;
    long ret_val = -1; //default value
    std::string target;
    std::string warning;
    bool blocked = false;
    long error_code = 0;
    bool in_syscall = false;
    pid_t pid = 0;
};

std::pair<std::string, std::string> get_syscall_name(unsigned long long num);
void log_syscall(const syscall_info& info);