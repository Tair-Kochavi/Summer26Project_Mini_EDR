#include "nw_blacklist.h"
#include <unordered_set>

namespace {
    const std::unordered_set<std::string> blacklist = {"198.51.100.1:8080", "2001:db8::1:443"};
}



bool check_connection(const std::string &connection) {
    if (blacklist.find(connection) != blacklist.end()) { //connection found in the blacklist
        return true;
    }
    return false;
}
