#!/usr/bin/env bash

make clean

make

echo "\c" > tmp/time_runs.txt
for ((i=100;i<=500;i+=50)); do
    for ((p=6;p>=0;p--)); do
        np=$((2**p))
        echo "begin np=$np i=$i"
        mpirun -np $np ./main $i >> tmp/time_runs.txt
    done
done