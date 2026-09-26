
#pragma once
#include <string>


struct syscall_info {
    unsigned long long sys_num;
    std::string path;
    std::string warning;
};

std::pair<std::string, std::string> get_syscall_name(unsigned long long num);
void log_syscall(const syscall_info& info);