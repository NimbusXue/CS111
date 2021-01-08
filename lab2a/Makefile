# NAME: JIAXUAN XUE
# EMAIL: nimbusxue1015@g.ucla.edu
# ID: 705142227

build: lab2_add.c lab2_list.c SortedList.c SortedList.h
	gcc -Wall -Wextra -pthread lab2_add.c -o lab2_add
	gcc -Wall -Wextra -pthread lab2_list.c SortedList.c -o lab2_list

tests: build
	rm -f lab2_add.csv lab2_list.csv
	chmod +x check.sh 
	chmod +x check2.sh 
	./check.sh >lab2_add.csv
	-./check2.sh >lab2_list.csv 2>/dev/null

graphs: lab2_add.csv lab2_list.csv
	chmod +x lab2_add.gp lab2_list.gp
	./lab2_add.gp  
	./lab2_list.gp  

clean:
	rm -f lab2_add lab2_list lab2a-705142227.tar.gz 

dist: tests graphs clean
	tar -czvf lab2a-705142227.tar.gz lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add.csv lab2_list.csv Makefile lab2_add.gp lab2_list.gp check.sh check2.sh  README lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png 

