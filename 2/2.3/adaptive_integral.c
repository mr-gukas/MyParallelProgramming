/*
 * Compilation:
 *    gcc -O2 -o adaptive_integral adaptive_integral.c -lpthread -lm
 *
 * Usage:
 *    ./adaptive_integral a b eps num_threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    double a, b, fa, fb, fm, S, tol;
} Task;

static Task *stack = NULL;
static int stack_top = 0;
static int stack_cap = 0;
static pthread_mutex_t stack_mutex = PTHREAD_MUTEX_INITIALIZER;

static double global_result = 0.0;
static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

double f(double x) {
    return sin(1.0/x);
}

double simpson(double a, double b, double fa, double fb, double fm) {
    return (fa + 4.0*fm + fb) * (b - a) / 6.0;
}

void push_task(Task t) {
    pthread_mutex_lock(&stack_mutex);
    if (stack_top == stack_cap) {
        int newcap = stack_cap ? stack_cap * 2 : 1;
        stack = realloc(stack, newcap * sizeof(Task));
        stack_cap = newcap;
    }
    stack[stack_top++] = t;
    pthread_mutex_unlock(&stack_mutex);
}

int pop_task(Task *t) {
    pthread_mutex_lock(&stack_mutex);
    if (stack_top == 0) {
        pthread_mutex_unlock(&stack_mutex);
        return 0;
    }
    *t = stack[--stack_top];
    pthread_mutex_unlock(&stack_mutex);
    return 1;
}

void *worker(void *arg) {
    Task task;
    while (pop_task(&task)) {
        double a = task.a, b = task.b;
        double m  = 0.5 * (a + b);
        double lm = 0.5 * (a + m), rm = 0.5 * (m + b);
        double fl = f(lm), fr = f(rm);
        double Sleft  = simpson(a,  m,  task.fa, task.fm, fl);
        double Sright = simpson(m,  b,  task.fm, task.fb, fr);
        if (fabs(Sleft + Sright - task.S) < 15.0 * task.tol) {
            double val = Sleft + Sright + (Sleft + Sright - task.S) / 15.0;
            pthread_mutex_lock(&result_mutex);
            global_result += val;
            pthread_mutex_unlock(&result_mutex);
        } else {
            Task t1 = {a,    m, task.fa, task.fm, fl, Sleft,  task.tol / 2.0};
            Task t2 = {m,    b, task.fm, task.fb, fr, Sright, task.tol / 2.0};
            push_task(t1);
            push_task(t2);
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s a b eps num_threads\n", argv[0]);
        return EXIT_FAILURE;
    }
    double a   = atof(argv[1]);
    double b   = atof(argv[2]);
    double eps = atof(argv[3]);
    int    P   = atoi(argv[4]);
    if (a <= 0.0 || b <= a || eps <= 0.0 || P <= 0) {
        fprintf(stderr, "Invalid arguments\n");
        return EXIT_FAILURE;
    }
    double fa = f(a), fb = f(b);
    double m  = 0.5 * (a + b), fm = f(m);
    double S  = simpson(a, b, fa, fb, fm);
    push_task((Task){a, b, fa, fb, fm, S, eps});

    pthread_t *threads = malloc(P * sizeof(pthread_t));
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int i = 0; i < P; ++i) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    for (int i = 0; i < P; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&t1, NULL);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec) * 1e-6;

    printf("Result integral = %.9f\n", global_result);
    printf("Elapsed time = %.6f sec\n", elapsed);

    free(stack);
    free(threads);
    pthread_mutex_destroy(&stack_mutex);
    pthread_mutex_destroy(&result_mutex);
    return EXIT_SUCCESS;
} 