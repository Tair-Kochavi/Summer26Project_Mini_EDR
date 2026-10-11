#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    struct sockaddr_in6 addr;
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(443);
    inet_pton(AF_INET6, "2001:db8::1", &addr.sin6_addr);

    std::cout << "Attempting blocked connection to 2001:db8::1:443...\n";
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
    } else {
        std::cout << "SUCCESS: Connected! (This should not happen)\n";
    }
    close(sock);
    return 0;
}