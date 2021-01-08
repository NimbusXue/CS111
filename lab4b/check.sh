#!/bin/bash

./lab4b --period=7 --scale=C --log=log.txt <<-EOF
SCALE=F
SCALE=C
PERIOD=5
STOP
START
LOG just to check
OFF
EOF

if [ $? -ne 0 ]
then
    echo "Error, wrong exit code, program did not exit properly"
fi

if [ ! -s log.txt ]
then
    echo "Error, log file was not created"
fi

for input in SCALE=F SCALE=C PERIOD=5 STOP START "LOG just to check" OFF SHUTDOWN
    do
        grep "$input" log.txt > /dev/null
        if [ $? -ne 0 ]
        then
            echo "$input not found in the log file"
        else
            echo "$input correctly logged"
        fi
    done

rm -f log.txt
