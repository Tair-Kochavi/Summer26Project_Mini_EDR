#include "nw_blacklist.h"


namespace {
    const std::unordered_set<std::string> blacklist = {};
}



bool check_connection(const std::string &connection) {
    if (blacklist.find(connection) != blacklist.end()) { //connection found in the blacklist
        return true;
    }
    return false;
}
