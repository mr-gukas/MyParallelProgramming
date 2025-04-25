/*
    Программа параллельного вычисления числа pi методом средних прямоугольников
    с использованием MPI.

    Компиляция:
    mpicc -o pi_mpi pi_mpi.c
    
    Запуск:
    mpirun -np <num_processes> ./pi_mpi <num_iterations> 
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static double f(double x) {
    return 4.0 / (1.0 + x * x);
}

int main(int argc, char *argv[]) {
    int rank, size;
    long long n = 1000000;
    if (argc >= 2) {
        n = atoll(argv[1]);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long local_n = n / size;
    double h = 1.0 / (double)n;
    double local_a = rank * local_n * h;

    double local_sum = 0.0;
    for (long long i = 0; i < local_n; i++) {
        double x = local_a + (i + 0.5) * h;
        local_sum += f(x);
    }
    local_sum *= h;

    double pi = 0.0;
    MPI_Reduce(&local_sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Число процессов: %d\n", size);
        printf("Количество итераций: %lld\n", n);
        printf("Вычисленное pi = %.16f\n", pi);
    }

    MPI_Finalize();
    return 0;
}
