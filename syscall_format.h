
#pragma once
#include <string>


struct syscall_info {
    int sys_num;
    std::string path;
};

std::pair<std::string, std::string> get_syscall_name(const int num);
void log_syscall(const syscall_info& info);