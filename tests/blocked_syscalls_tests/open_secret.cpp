#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main() {
    std::cout << "Attempting to open secret.txt...\n";
    int fd = open("secret.txt", O_RDONLY);
    if (fd < 0) {
        perror("Open failed");
    } else {
        std::cout << "SUCCESS: File opened! (This should not happen)\n";
        close(fd);
    }
    return 0;
}