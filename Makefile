CFLAGS= -Wall -lm -coverage -g -std=c99
BINARIES=ftserver
DIR=src/include


server: src/ftserver.c
	-pkill ftserver
	gcc -I$(DIR) -o ftserver src/ftserver.c $(CFLAGS)
	# ./ftserver $(PORT)
	# mv *.gcno coverage

client: src/ftclient.py
	python3 src/ftclient.py $(CPORT) $(DPORT)

clean:
	-pkill ftserver
	rm -f *.o *.gcov *.gcda *.gcno *.so *.out *.log $(BINARIES)
