#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    std::cout << "=== [TEST 4] NETWORK HOOK & ENDIANNESS GAUNTLET ===\n";
    int sock4 = socket(AF_INET, SOCK_STREAM, 0);
    int sock6 = socket(AF_INET6, SOCK_STREAM, 0);

    // PHASE 1: Invalid sockaddr pointer (NULL)
    std::cout << "--- [4.1] Passing NULL sockaddr pointer to connect() ---\n";
    connect(sock4, nullptr, sizeof(struct sockaddr_in));

    // PHASE 2: Blocked IPv4 (198.51.100.1:8080)
    std::cout << "--- [4.2] Attempting blocked IPv4 (198.51.100.1:8080) ---\n";
    struct sockaddr_in addr4_bad{};
    addr4_bad.sin_family = AF_INET;
    addr4_bad.sin_port = htons(8080);
    inet_pton(AF_INET, "198.51.100.1", &addr4_bad.sin_addr);
    if (connect(sock4, (struct sockaddr*)&addr4_bad, sizeof(addr4_bad)) < 0) {
        perror("Blocked IPv4 result (Expect 'Operation not permitted')");
    }

    // PHASE 3: Blocked IPv6 (2001:db8::1:443)
    std::cout << "--- [4.3] Attempting blocked IPv6 (2001:db8::1:443) ---\n";
    struct sockaddr_in6 addr6_bad{};
    addr6_bad.sin6_family = AF_INET6;
    addr6_bad.sin6_port = htons(443);
    inet_pton(AF_INET6, "2001:db8::1", &addr6_bad.sin6_addr);
    if (connect(sock6, (struct sockaddr*)&addr6_bad, sizeof(addr6_bad)) < 0) {
        perror("Blocked IPv6 result (Expect 'Operation not permitted')");
    }

    // PHASE 4: Legal IPv4 (127.0.0.1:9999 - will likely fail with Connection Refused from OS, NOT blocked by EDR)
    std::cout << "--- [4.4] Attempting legal IPv4 (127.0.0.1:9999) ---\n";
    struct sockaddr_in addr4_good{};
    addr4_good.sin_family = AF_INET;
    addr4_good.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &addr4_good.sin_addr);
    if (connect(sock4, (struct sockaddr*)&addr4_good, sizeof(addr4_good)) < 0) {
        perror("Legal IPv4 result (Expect 'Connection refused' from OS, NOT Operation not permitted)");
    }

    close(sock4);
    close(sock6);
    std::cout << "=== [TEST 4 COMPLETE] ===\n";
    return 0;
}