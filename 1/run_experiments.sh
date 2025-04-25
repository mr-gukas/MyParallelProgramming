#!/bin/bash
set -e

# Скрипт для измерения времени работы transport_seq и transport_mpi
# Генерирует results.csv с полями: P,M,K,mode,time

# Число процессов для MPI
Ps=(2 4 8)
# Значения M (число отрезков по x)
Ms=(1000 2000 5000 10000)

# Заголовок для CSV
echo "P,M,K,mode,time" > results.csv

# Последовательная реализация (mode=seq, P=1)
for M in "${Ms[@]}"; do
  for K in $M $((2*M)); do
    echo "Running sequential: M=$M, K=$K"
    t=$(./transport_seq $M $K | awk '/Время решения:/ {print $3}')
    echo "1,$M,$K,seq,$t" >> results.csv
  done
 done

# MPI реализация (mode=mpi, P>1)
for M in "${Ms[@]}"; do
  for K in $M $((2*M)); do
    for P in "${Ps[@]}"; do
      echo "Running MPI: P=$P, M=$M, K=$K"
      t=$(mpirun -np $P ./transport_mpi $M $K | awk '/Время решения:/ {print $3}')
      echo "$P,$M,$K,mpi,$t" >> results.csv
    done
  done
 done 