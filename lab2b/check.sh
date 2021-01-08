#!/bin/bash
./lab2_list --threads=1 --iterations=1000 --sync=m
./lab2_list --threads=2 --iterations=1000 --sync=m
./lab2_list --threads=4 --iterations=1000 --sync=m
./lab2_list --threads=8 --iterations=1000 --sync=m
./lab2_list --threads=12 --iterations=1000 --sync=m
./lab2_list --threads=16 --iterations=1000 --sync=m
./lab2_list --threads=24 --iterations=1000 --sync=m

./lab2_list --threads=1 --iterations=1000 --sync=s
./lab2_list --threads=2 --iterations=1000 --sync=s
./lab2_list --threads=4 --iterations=1000 --sync=s
./lab2_list --threads=8 --iterations=1000 --sync=s
./lab2_list --threads=12 --iterations=1000 --sync=s
./lab2_list --threads=16 --iterations=1000 --sync=s
./lab2_list --threads=24 --iterations=1000 --sync=s

for i in 1, 4, 8, 12, 16
do
    for j in 1, 2, 4, 8, 16
    do
        ./lab2_list  --threads=$i --iterations=$j --yield=id --lists=4
    done
done

for i in 1, 4, 8, 12, 16
do
    for j in 10, 20, 40, 80
    do
        for k in m, s
        do
            ./lab2_list --threads=$i --iterations=$j --yield=id --sync=$k --lists=4
        done
    done
done


for i in 1, 2, 4, 8, 12
do
    for j in 4, 8, 16
    do
        for k in m, s
        do
            ./lab2_list --threads=$i --iterations=1000 --sync=$k --lists=$j
        done
    done
done
