#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define DEFAULT_ITERS 1000000

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    char            flag;
    char            data;
} shm_t;

int iterations;
shm_t *shm;

void *thread1_func(void *arg) {
    char buf = 0;
    struct timeval t0, t1;
    double total = 0.0;

    for (int i = 0; i < iterations; ++i) {
        gettimeofday(&t0, NULL);
        pthread_mutex_lock(&shm->mutex);
        shm->data = buf;
        shm->flag = 1;
        pthread_cond_signal(&shm->cond);
        while (shm->flag != 0) pthread_cond_wait(&shm->cond, &shm->mutex);
        pthread_mutex_unlock(&shm->mutex);
        gettimeofday(&t1, NULL);
        total += (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec) * 1e-6;
    }
    double avg = total / iterations;
    printf("shm_comm iterations=%d avg_time=%.9f sec\n", iterations, avg);
    return NULL;
}

void *thread2_func(void *arg) {
    char buf;
    for (int i = 0; i < iterations; ++i) {
        pthread_mutex_lock(&shm->mutex);
        while (shm->flag != 1) pthread_cond_wait(&shm->cond, &shm->mutex);
        buf = shm->data;
        shm->flag = 0;
        pthread_cond_signal(&shm->cond);
        pthread_mutex_unlock(&shm->mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {
    iterations = DEFAULT_ITERS;
    if (argc >= 2) iterations = atoi(argv[1]);

    shm = malloc(sizeof(*shm));
    pthread_mutex_init(&shm->mutex, NULL);
    pthread_cond_init(&shm->cond, NULL);
    shm->flag = 0;
    shm->data = 0;

    pthread_t t1, t2;
    pthread_create(&t2, NULL, thread2_func, NULL);
    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&shm->mutex);
    pthread_cond_destroy(&shm->cond);
    free(shm);
    return 0;
} 