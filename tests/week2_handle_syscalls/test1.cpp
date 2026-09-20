#include <fcntl.h>
#include <unistd.h>

int main() {
    // Triggers an explicit 'open' system call
    int fd = open("target_secret.txt", O_CREAT | O_RDWR, 0644);
    if (fd != -1) {
        close(fd);
    }
    return 0;
}