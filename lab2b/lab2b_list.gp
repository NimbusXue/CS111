#! /usr/bin/gnuplot
#NAME: JIAXUAN XUE
#ID: 705142227
#EMAIL: nimbusxue1015@g.ucla.edu


# general plot parameters
set terminal png
set datafile separator ","

# lab2b_1.png
set title "Aggregate Throughput For Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'


plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/spin' with linespoints lc rgb 'red', \
     "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list w/mutex' with linespoints lc rgb 'green'


# lab2b_2.png
set title "mean time for mutex-synchronized list operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Cost per operation(ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2): ($8) \
	title 'wait_for_lock_time' with linespoints lc rgb 'red', \
    "< grep -e 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2): ($7) \
	title 'average_time_per_operation' with linespoints lc rgb 'green'


# lab2b_3.png
set title "Successful Iterations for Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Spin-Lock' with points lc rgb 'green', \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'blue' 


# lab2b_4.png
set title "Throughput for mutex synchronized partitioned lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex lists=1' with linespoints lc rgb 'red', \
	 "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex lists=4' with linespoints lc rgb 'green', \
	 "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex lists=8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex lists=16' with linespoints lc rgb 'yellow'


# lab2b_5.png
set title "Throughput for spin-lock-synchronized partitioned lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-lock Lists = 1' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-lock Lists = 4' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-lock Lists = 8' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-lock Lists = 16' with linespoints lc rgb 'yellow'

























