#!/bin/sh



file30="data/Co_30_1_NT_0.txt"
file100="data/Co_100_20_T_9.txt"
file300="data/Co_300_20_T_19.txt"


main="./main"

if [ ! -x "$main"  ]
then
    chmod +x "$main"
fi

if [ $1 -eq "30" ]
then
    $main $file30
elif [ $1 -eq "100" ]
then
    $main $file100
else
    $main $file300
fi
