# NAME: JIAXUAN XUE, Felix Gan
# EMAIL: nimbusxue1015@g.ucla.edu, felixgan@g.ucla.edu
# ID: 705142227, 205127385

CC = gcc
CFLAGS = -Wall -Wextra

default:
	echo '#!/bin/bash' > lab3b
	echo 'python3 lab3b.py $$1' >> lab3b
	echo 'exit $$?' >> lab3b
	chmod 755 lab3b
	
.SILENT:

dist: 
	tar -czvf lab3b-705142227.tar.gz lab3b.py Makefile README

.PHONY: all dist clean

clean:
	rm -rf *.tar.gz *.dSYM lab3b
