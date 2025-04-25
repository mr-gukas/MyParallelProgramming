#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int iterations = 1000000;
    if (argc >= 2) iterations = atoi(argv[1]);

    int p1[2], p2[2];
    if (pipe(p1) < 0 || pipe(p2) < 0) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid > 0) {
        // Parent: send on p1, receive on p2
        close(p1[0]);  // close read end of p1
        close(p2[1]);  // close write end of p2
        char buf = 0;
        struct timeval t0, t1;
        double total = 0.0;

        for (int i = 0; i < iterations; ++i) {
            gettimeofday(&t0, NULL);
            write(p1[1], &buf, 1);
            read(p2[0], &buf, 1);
            gettimeofday(&t1, NULL);
            total += (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec) * 1e-6;
        }

        double avg = total / iterations;
        printf("pipe_comm iterations=%d avg_time=%.9f sec\n", iterations, avg);

        wait(NULL);
        close(p1[1]);
        close(p2[0]);
    } else {
        // Child: receive on p1, send on p2
        close(p1[1]);  // close write end of p1
        close(p2[0]);  // close read end of p2
        char buf;
        for (int i = 0; i < iterations; ++i) {
            read(p1[0], &buf, 1);
            write(p2[1], &buf, 1);
        }
        close(p1[0]);
        close(p2[1]);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
} 