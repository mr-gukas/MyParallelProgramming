/*
 * Последовательное решение уравнения переноса
 * методом «схема крест».
 *
 *   ∂u/∂t + a ∂u/∂x = f(t,x), 0≤t≤T, 0≤x≤X
 *   u(0,x) = φ(x), u(t,0) = ψ(t), u(t,X) = 0
 *
 * Схема:
 *   (u^{k+1}_m - u^{k-1}_m)/(2τ) + a*(u^k_{m+1} - u^k_{m-1})/(2h) = f(t_k,x_m)
 * Первая временная итерация (k=0→1):
 *   (u^1_m - u^0_m)/τ + a*(u^0_m - u^0_{m-1})/h = f(0,x_m)
 *
 * Компиляция:
 *   gcc -O2 -o transport_seq transport_seq.c -lm
 *
 * Запуск:
 *   ./transport_seq [M] [K]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

// Параметры задачи
static const double a = 1.0;   // скорость переноса
static const double X = 1.0;   // длина по x
static const double T = 1.0;   // время моделирования

// Начальное условие φ(x)
double phi(double x) {
    return sin(M_PI * x);
}
// Граничное условие ψ(t)
double psi(double t) {
    return 0.0;
}
// Источник f(t,x)
double f_src(double t, double x) {
    return 0.0;
}

int main(int argc, char *argv[]) {
    int M = 1000, K = 1000;
    if (argc >= 2) M = atoi(argv[1]);
    if (argc >= 3) K = atoi(argv[2]);

    double h = X / M;
    double tau = T / K;
    double lambda = a * tau / h;
    if (fabs(lambda) > 1.0) {
        fprintf(stderr, "Внимание! Условие Куранта λ=%.3f>1, схема может быть неустойчива.\n", lambda);
    }

    double *u_old = malloc((M+1) * sizeof(double));
    double *u_cur = malloc((M+1) * sizeof(double));
    double *u_new = malloc((M+1) * sizeof(double));
    if (!u_old || !u_cur || !u_new) {
        fprintf(stderr, "Ошибка выделения памяти.\n");
        return EXIT_FAILURE;
    }

    for (int m = 0; m <= M; ++m) {
        double x = m * h;
        u_old[m] = phi(x);
    }
    u_old[0] = psi(0.0);
    u_old[M] = 0.0;

    u_cur[0] = psi(tau);
    for (int m = 1; m < M; ++m) {
        double x = m * h;
        u_cur[m] = u_old[m]
                   - lambda * (u_old[m] - u_old[m-1])
                   + tau * f_src(0.0, x);
    }
    u_cur[M] = 0.0;

    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int k = 1; k < K; ++k) {
        double t_k = k * tau;
        double t_k1 = (k + 1) * tau;
        u_new[0] = psi(t_k1);
        u_new[M] = 0.0;
        for (int m = 1; m < M; ++m) {
            double x = m * h;
            u_new[m] = u_old[m]
                       - lambda * (u_cur[m+1] - u_cur[m-1])
                       + 2.0 * tau * f_src(t_k, x);
        }
        double *tmp = u_old;
        u_old = u_cur;
        u_cur = u_new;
        u_new = tmp;
    }

    gettimeofday(&t1, NULL);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_usec - t0.tv_usec) * 1e-6;

    printf("Последовательная реализация:\n");
    printf("  M=%d, K=%d, lambda=%.3f\n", M, K, lambda);
    printf("  Время решения: %.6f с\n", elapsed);

    free(u_old);
    free(u_cur);
    free(u_new);
    return 0;
} 