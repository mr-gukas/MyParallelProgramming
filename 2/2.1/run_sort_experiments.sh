#!/bin/bash
set -e

# Компиляция выполняемых файлов
echo "Компиляция seq_sort и parallel_sort..."
gcc -O2 -o seq_sort seq_sort.c
gcc -O2 -pthread -o parallel_sort parallel_sort.c

# Скрипт для измерения времени работы seq_sort и par_sort
# Формат выходного CSV: N,P,mode,time

# Размеры массива
Ns=(100000 200000 500000 1000000)
# Число потоков для par_sort
Ts=(2 4 8 16)

# Заголовок
echo "N,P,mode,time" > sort_results.csv

# Последовательная сортировка
for N in "${Ns[@]}"; do
    echo "Seq sort: N=$N"
    t=$(./seq_sort $N | awk '{print $3}' | cut -d'=' -f2 | tr -d '\r\n')
    printf "%d,1,seq,%s\n" "$N" "$t" >> sort_results.csv
done

# Параллельная сортировка
for N in "${Ns[@]}"; do
    for P in "${Ts[@]}"; do
        echo "Par sort: N=$N, threads=$P"
        t=$(./parallel_sort $N $P | awk '{print $4}' | cut -d'=' -f2 | tr -d '\r\n')
        printf "%d,%d,par,%s\n" "$N" "$P" "$t" >> sort_results.csv
    done
done 