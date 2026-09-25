#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    // Bypasses glibc safety checks to send a raw NULL pointer directly to the kernel in the rsi register
    syscall(SYS_openat, AT_FDCWD, nullptr, O_RDONLY, 0);

    return 0;
}