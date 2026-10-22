
#include "blacklist.h"
#include <unordered_set>

namespace {
    const std::unordered_set<std::string> blacklist = {"secret.txt", "no_open.txt"};
}


/**
 *
 * @param path path to file that needs to be checked
 * @return true if the file is blacklisted and false otherwise
 */
bool check_path(const std::string& path) {
    //if there is / at the end of the path - remove it first
    std::string clean_path = path;
    while (!clean_path.empty() && clean_path.back() == '/') {
        clean_path.pop_back();
    }

    size_t pos = path.find_last_of('/');
    std::string name = clean_path;         //if  / wasn't found - the path is the file name
    if (pos != std::string::npos) {     //if it was found - the file name appears after the last /
        name = path.substr(pos + 1);
    }
    if (blacklist.find(name) != blacklist.end()) {
        return true;
    }
    return false;
}
