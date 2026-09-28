#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>

int main() {
    // Create a dummy directory to resolve the relative path
    mkdir("hidden_dir", 0777); 
    
    std::cout << "Attempting to open hidden_dir/no_open.txt...\n";
    int fd = open("hidden_dir/no_open.txt", O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        perror("Open failed");
    } else {
        std::cout << "SUCCESS: File opened! (This should not happen)\n";
        close(fd);
    }
    return 0;
}