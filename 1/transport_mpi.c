/*
 * Параллельное решение уравнения переноса
 * методом «схема крест» с использованием MPI.
 *
 * Компиляция:
 *   mpicc -O2 -o transport_mpi transport_mpi.c -lm
 *
 * Запуск:
 *   mpirun -np <P> ./transport_mpi [M] [K]
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const double a = 1.0;
static const double X = 1.0;
static const double T = 1.0;

double phi(double x) {
    return sin(M_PI * x);
}
double psi(double t) {
    return 0.0;
}
double f_src(double t, double x) {
    return 0.0;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int M = 1000, K = 1000;
    if (argc >= 2) M = atoi(argv[1]);
    if (argc >= 3) K = atoi(argv[2]);

    double h = X / M;
    double tau = T / K;
    double lambda = a * tau / h;
    if (rank == 0 && fabs(lambda) > 1.0) {
        fprintf(stderr, "Внимание: условие Куранта λ=%.3f>1, схема может быть неустойчива.\n", lambda);
    }

    int numPoints = M + 1;
    int base  = numPoints / size;
    int rem   = numPoints % size;
    int local_n = (rank < rem) ? base + 1 : base;
    int start   = (rank < rem)
                  ? rank * (base + 1)
                  : rem * (base + 1) + (rank - rem) * base;

    double *u_old = malloc((local_n + 2) * sizeof(double));
    double *u_cur = malloc((local_n + 2) * sizeof(double));
    double *u_new = malloc((local_n + 2) * sizeof(double));
    if (!u_old || !u_cur || !u_new) {
        fprintf(stderr, "Ошибка выделения памяти на rank %d.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 1; i <= local_n; ++i) {
        int gm = start + i - 1;
        double x = gm * h;
        u_old[i] = phi(x);
    }
    if (start == 0) {
        u_old[1] = phi(0.0);
        u_old[0] = phi(0.0);
    }
    if (start + local_n - 1 == M) {
        u_old[local_n]   = 0.0;
        u_old[local_n+1] = 0.0;
    }

    int left  = rank - 1, right = rank + 1;
    if (left  < 0)     left  = MPI_PROC_NULL;
    if (right >= size) right = MPI_PROC_NULL;
    MPI_Sendrecv(&u_old[local_n], 1, MPI_DOUBLE, right, 0,
                 &u_old[0],      1, MPI_DOUBLE, left,  0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(&u_old[1],       1, MPI_DOUBLE, left,  1,
                 &u_old[local_n+1],1, MPI_DOUBLE, right, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    double t1 = tau;
    if (start == 0) {
        u_cur[1] = psi(t1);
    }
    for (int i = 1; i <= local_n; ++i) {
        int gm = start + i - 1;
        if (gm == 0) continue;
        if (gm == M) {
            u_cur[i] = 0.0;
        } else {
            double x = gm * h;
            u_cur[i] = u_old[i]
                       - lambda * (u_old[i] - u_old[i-1])
                       + tau * f_src(0.0, x);
        }
    }
    if (start == 0)      u_cur[0]        = psi(t1);
    if (start + local_n - 1 == M) u_cur[local_n+1] = 0.0;

    MPI_Barrier(MPI_COMM_WORLD);
    double t_start = MPI_Wtime();

    for (int k = 1; k < K; ++k) {
        double t_k  = k * tau;
        double t_k1 = (k + 1) * tau;
        MPI_Request reqs[4];
        MPI_Irecv(&u_cur[0],          1, MPI_DOUBLE, left,  0,
                  MPI_COMM_WORLD, &reqs[0]);
        MPI_Irecv(&u_cur[local_n+1],  1, MPI_DOUBLE, right, 1,
                  MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(&u_cur[1],          1, MPI_DOUBLE, left,  1,
                  MPI_COMM_WORLD, &reqs[2]);
        MPI_Isend(&u_cur[local_n],    1, MPI_DOUBLE, right, 0,
                  MPI_COMM_WORLD, &reqs[3]);

        for (int i = 2; i <= local_n-1; ++i) {
            int gm = start + i - 1;
            double x = gm * h;
            u_new[i] = u_old[i]
                       - lambda * (u_cur[i+1] - u_cur[i-1])
                       + 2.0 * tau * f_src(t_k, x);
        }

        MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);

        if (start == 0) {
            u_new[1] = psi(t_k1);
        } else {
            int gm = start;
            double x = gm * h;
            u_new[1] = u_old[1]
                       - lambda * (u_cur[2] - u_cur[0])
                       + 2.0 * tau * f_src(t_k, x);
        }
        if (start + local_n - 1 == M) {
            u_new[local_n] = 0.0;
        } else {
            int gm = start + local_n - 1;
            double x = gm * h;
            u_new[local_n] = u_old[local_n]
                             - lambda * (u_cur[local_n+1] - u_cur[local_n-1])
                             + 2.0 * tau * f_src(t_k, x);
        }

        double *tmp = u_old; u_old = u_cur; u_cur = u_new; u_new = tmp;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_end = MPI_Wtime();
    double elapsed = t_end - t_start;

    if (rank == 0) {
        printf("MPI-параллельная реализация:\n");  
        printf("  Процессы: %d, M=%d, K=%d, λ=%.3f\n", 
               size, M, K, lambda);
        printf("  Время решения: %.6f с\n", elapsed);
    }

    free(u_old);
    free(u_cur);
    free(u_new);
    MPI_Finalize();
    return 0;
} 