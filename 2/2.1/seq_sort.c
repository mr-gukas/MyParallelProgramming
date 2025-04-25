#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int cmp_int(const void *a, const void *b) {
    int ai = *(const int*)a;
    int bi = *(const int*)b;
    return (ai > bi) - (ai < bi);
}

int main(int argc, char **argv) {
    long N = 1000000;
    if (argc >= 2) N = atol(argv[1]);

    int *arr = malloc(N * sizeof(int));
    if (!arr) {
        fprintf(stderr, "Memory allocation error. N=%ld\n", N);
        return EXIT_FAILURE;
    }

    srand((unsigned)time(NULL));
    for (long i = 0; i < N; ++i) {
        arr[i] = rand();
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    qsort(arr, N, sizeof(int), cmp_int);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    printf("seq_sort N=%ld time=%.6f\n", N, elapsed);

    free(arr);
    return 0;
} 