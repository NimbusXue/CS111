# NAME: JIAXUAN XUE
# EMAIL: nimbusxue1015@g.ucla.edu
# ID: 705142227

default: 
	gcc  -Wall -Wextra -lm -lmraa  -g lab4c_tcp.c -o lab4c_tcp
	gcc  -Wall -Wextra -lm -lmraa  -g -lssl -lcrypto lab4c_tls.c -o lab4c_tls

clean:
	rm -f lab4c-705142227.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czvf lab4c-705142227.tar.gz Makefile README lab4c_tcp.c lab4c_tls.c 
