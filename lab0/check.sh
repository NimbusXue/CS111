#!/bin/bash
echo "input1" | ./lab0 | grep "^input1$" >output.txt
if [ $? -eq 0 ]
then
    echo 'Test case 1: passed stdin matches stdout'
else
    echo 'Test case 1: failed error: stdin does not matches stdout'
fi
rm -f output.txt

./lab0 --input not_exist.txt 2> output.txt
if [ $? -eq 2 ]
then
    echo 'Test case 2: passed correct exit code for non-exist input file'
else
    echo 'Test case 2: failed error: wrong exit code for non-exist input file'
fi
rm -f output.txt

touch output.txt
chmod -w output.txt
echo "input3" | ./lab0 --output output.txt 2> output1.txt
if [ $? -eq 3 ]
then
    echo 'Test case 3: passed correct exit code for non-writable output file'
else
    echo 'Test case 3: failed error: wrong exit code for non-writable output file'
fi
rm -f output.txt output1.txt

./lab0 --wrongArgument 2> output.txt
if [ $? -eq 1 ]
then
    echo 'Test case 4: passed correct exit code for an unrecognized argument'
else
    echo 'Test case 4: failed error: wrong exit code for an unrecognized argument'
fi
rm -f output.txt

echo "input1" | ./lab0 --segfault --catch 2> output.txt
if [ $? -eq 4 ]
then
    echo 'Test case 5: passed correct exit code for a segmentation fault'
else
    echo 'Test case 5: failed error: wrong exit code for a segmentation fault'
fi
rm -f output.txt
