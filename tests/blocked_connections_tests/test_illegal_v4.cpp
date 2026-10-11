#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "198.51.100.1", &addr.sin_addr);

    std::cout << "Attempting blocked connection to 198.51.100.1:8080...\n";
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
    } else {
        std::cout << "SUCCESS: Connected! (This should not happen)\n";
    }
    close(sock);
    return 0;
}