#!/usr/bin/env bash

make clean

make

echo "\c" > tmp/time_runs.txt
for ((p=1;p<=4;p++)); do
    for ((i=100;i<=1040;i+=50)); do
        mpirun -np $p ./main $i >> tmp/time_runs.txt
    done
done