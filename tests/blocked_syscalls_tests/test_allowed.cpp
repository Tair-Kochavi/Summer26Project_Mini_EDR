#include <fcntl.h>
#include <unistd.h>
#include <iostream>

int main() {
    std::cout << "Attempting to open safe_file.txt...\n";
    int fd = open("safe_file.txt", O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        perror("Open failed (This should not happen)");
    } else {
        std::cout << "SUCCESS: Safe file opened correctly.\n";
        close(fd);
    }
    return 0;
}