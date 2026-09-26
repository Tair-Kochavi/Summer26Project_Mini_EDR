
#include "blacklist.h"
#include <unordered_set>

namespace {
    const std::unordered_set<std::string> blacklist = {"secret.txt", "no_open.txt"};
}


bool check_path(const std::string& path) {
    size_t pos = path.find_last_of('/');
    std::string name = path;         //if  / wasn't found - the path is the file name
    if (pos != std::string::npos) {     //if it was found - the file name appears after the last /
        name = path.substr(pos + 1);
    }
    if (blacklist.find(name) != blacklist.end()) {
        return true;
    }
    return false;
}
