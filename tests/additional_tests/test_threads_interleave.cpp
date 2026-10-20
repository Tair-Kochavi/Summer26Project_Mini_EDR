#include <iostream>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

void* thread_worker(void* arg) {
    int id = *static_cast<int*>(arg);
    for (int i = 0; i < 3; i++) {
        if (id == 1) {
            // Thread 1 hammers a blocked file
            int fd = open("secret.txt", O_RDONLY);
            if (fd >= 0) {
                std::cerr << "[FAIL] Thread 1 bypassed block on secret.txt!\n";
                close(fd);
            }
        } else {
            // Thread 2 hammers an allowed file + invalid fd close (-9 EBADF)
            int fd = open("/etc/ld.so.cache", O_RDONLY);
            if (fd >= 0) close(fd);
            close(9999); // Should log return value: -9
        }
        usleep(10000);
    }
    return nullptr;
}

int main() {
    std::cout << "=== [TEST 2] STARTING CLONE & INTERLEAVING STRESS TEST ===\n";
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, nullptr, thread_worker, &id1);
    pthread_create(&t2, nullptr, thread_worker, &id2);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    std::cout << "=== [TEST 2 COMPLETE] Threads tracked and joined cleanly! ===\n";
    return 0;
}