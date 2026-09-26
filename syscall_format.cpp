
#include "syscall_format.h"

#include <iostream>
#include <unordered_map>

namespace {
    const std::unordered_map<unsigned long long, std::pair<std::string, std::string>> sys_map = {
        {0, {"read", "data read"}},
        {1, {"write", "data output"}},
        {2, {"open", "file access"}},
        {3, {"close", "resource release"}},
        {59, {"execve", "process execution"}},
        {257, {"openat", "file access"}}
    };
}

/**
 *
 * @param num the syscall number
 * @return string explaining which syscall was used
 */
std::pair<std::string, std::string> get_syscall_name(const unsigned long long num) {
    auto it = sys_map.find(num);
    if (it != sys_map.end()) {  //syscall number found in sys_map
        return it->second;
    }
    return {"SYS_" + std::to_string(num), ""};
}

/**
 *
 * @param info syscall information
 */
void log_syscall(const syscall_info& info) {
    std::pair<std::string, std::string> sys_info = get_syscall_name(info.sys_num);
    std::cout << "[SYSCALL] " << sys_info.first << " | ";
    std::cout << "INFO: " << sys_info.second << " | ";
    if (info.path != "") {
        std::cout << "PATH: " << info.path << "\n";
    }
}
