/*
    Программа измерения времени коммуникации между двумя узлами (ping-pong).

    Компиляция:
    mpicc -o ping_pong ping_pong.c
    Запуск:
    mpirun -np 2 ./ping_pong [message_size_bytes] [iterations] 
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    double t_start, t_end, total_time = 0.0;
    int tag = 0;
    int message_size = 1;      
    int iterations = 10000;    

    if (argc >= 2) message_size = atoi(argv[1]);
    if (argc >= 3) iterations = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) fprintf(stderr, "Ошибка: требуется ровно 2 процесса.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    char *buffer = (char*)malloc(message_size);
    if (!buffer) {
        fprintf(stderr, "Ошибка: не удалось выделить буфер размером %d байт.\n", message_size);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    for (int i = 0; i < message_size; ++i) buffer[i] = (char)rank;

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < iterations; ++i) {
        if (rank == 0) {
            t_start = MPI_Wtime();
            MPI_Send(buffer, message_size, MPI_CHAR, 1, tag, MPI_COMM_WORLD);
            MPI_Recv(buffer, message_size, MPI_CHAR, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            t_end = MPI_Wtime();
            total_time += (t_end - t_start);
        } else {
            MPI_Recv(buffer, message_size, MPI_CHAR, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(buffer, message_size, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) {
        double avg_time = total_time / iterations;
        printf("Размер сообщения: %d байт\n", message_size);
        printf("Итераций: %d\n", iterations);
        printf("Среднее время оборота (round-trip): %.6f мс\n", avg_time * 1e3);
    }

    free(buffer);
    MPI_Finalize();
    return 0;
}
