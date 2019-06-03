CFLAGS= -Wall -lm -g -std=c99
BINARIES=ftserver
# DIR=src/include


server: ftserver.c
	# -pkill ftserver
	gcc -o ftserver ftserver.c $(CFLAGS)
	./ftserver $(PORT)
	# mv *.gcno coverage

client: ftclient.py
	python3 ftclient.py $(HOST) $(CPORT) $(DPORT)

clean:
	-pkill ftserver
	rm -f *.o *.gcov *.gcda *.gcno *.so *.out *.log $(BINARIES)
