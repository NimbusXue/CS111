# NAME: JIAXUAN XUE
# EMAIL: nimbusxue1015@g.ucla.edu
# ID: 705142227

default:
	gcc -Wall -Wextra -g -o lab2_list -lpthread -lprofiler lab2_list.c SortedList.c

tests: default
	chmod +x check.sh
	-./check.sh >lab2b_list.csv 

profile: default
	rm -f ./raw.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so.0
	CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=routine ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf

graphs: tests
	chmod +x lab2b_list.gp
	./lab2b_list.gp

dist: graphs profile clean
	tar -czvf lab2b-705142227.tar.gz SortedList.h SortedList.c lab2_list.c lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b_list.gp check.sh Makefile README

clean: 
	rm -f lab2b-705142227.tar.gz lab2_list 