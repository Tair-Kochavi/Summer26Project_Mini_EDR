
#include "syscall_format.h"

#include <unordered_map>

namespace {
    const std::unordered_map<int, std::string> sys_map = {
        {0, "read"},
        {1, "write"},
        {2, "open"},
        {3, "close"},
        {59, "execve"},
        {257, "openat"}
    };
}

/**
 *
 * @param num the syscall number
 * @return string explaining which syscall was used
 */
std::string get_syscall_name(const int num) {
    auto it = sys_map.find(num);
    if (it != sys_map.end()) {  //syscall number found in sys_map
        return it->second;
    }
    return "SYS_" + std::to_string(num);
}
