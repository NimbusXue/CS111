# NAME: JIAXUAN XUE
# EMAIL: nimbusxue1015@g.ucla.edu
# ID: 705142227

default:
	gcc -o lab4b -g -lm -lmraa -Wall -Wextra lab4b.c

check:
	chmod +x check.sh
	./check.sh 

dist:
	tar -czvf lab4b-705142227.tar.gz lab4b.c Makefile README check.sh

clean: 
	rm -f lab4b lab4b-705142227.tar.gz