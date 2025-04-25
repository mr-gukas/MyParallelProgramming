#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

int cmp_int(const void *a, const void *b) {
    int ai = *(const int*)a;
    int bi = *(const int*)b;
    return (ai > bi) - (ai < bi);
}

struct sort_args {
    int *arr;
    long start, end;
};

void *thread_sort(void *arg) {
    struct sort_args *a = arg;
    qsort(a->arr + a->start, a->end - a->start, sizeof(int), cmp_int);
    return NULL;
}

int main(int argc, char **argv) {
    long N = 1000000;
    int P = 4;
    if (argc >= 2) N = atol(argv[1]);
    if (argc >= 3) P = atoi(argv[2]);

    int *arr = malloc(N * sizeof(int));
    if (!arr) { fprintf(stderr, "malloc failed N=%ld\n", N); return 1; }
    srand((unsigned)time(NULL));
    for (long i = 0; i < N; ++i) arr[i] = rand();

    int *copy = malloc(N * sizeof(int));
    memcpy(copy, arr, N * sizeof(int));
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    qsort(copy, N, sizeof(int), cmp_int);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t_seq = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)*1e-9;
    printf("seq_sort N=%ld time=%.6f\n", N, t_seq);
    free(copy);

    pthread_t *threads = malloc(P * sizeof(pthread_t));
    struct sort_args *args = malloc(P * sizeof(struct sort_args));
    long base = N / P;
    long rem  = N % P;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    long offset = 0;
    for (int i = 0; i < P; ++i) {
        long len = base + (i < rem ? 1 : 0);
        args[i].arr   = arr;
        args[i].start = offset;
        args[i].end   = offset + len;
        pthread_create(&threads[i], NULL, thread_sort, &args[i]);
        offset += len;
    }
    for (int i = 0; i < P; ++i) pthread_join(threads[i], NULL);

    int *buffer = malloc(N * sizeof(int));
    long curr_len = args[0].end - args[0].start;
    memcpy(buffer, arr, curr_len * sizeof(int));
    int *src = buffer;
    int *dst = malloc(N * sizeof(int));
    for (int i = 1; i < P; ++i) {
        long len_i = args[i].end - args[i].start;
        long p = 0, q = args[i].start, r = 0;
        while (p < curr_len && q < args[i].end) {
            if (src[p] <= arr[q]) dst[r++] = src[p++];
            else                  dst[r++] = arr[q++];
        }
        while (p < curr_len)           dst[r++] = src[p++];
        while (q < args[i].end)       dst[r++] = arr[q++];
        int *tmp = src; src = dst; dst = tmp;
        curr_len += len_i;
    }
    if (src != buffer) memcpy(buffer, src, curr_len * sizeof(int));
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t_par = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)*1e-9;
    printf("par_sort   N=%ld P=%d time=%.6f\n", N, P, t_par);

    free(arr);
    free(buffer);
    free(dst);
    free(threads);
    free(args);
    return 0;
} 