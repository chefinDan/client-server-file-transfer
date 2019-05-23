CFLAGS= -Wall -lm -coverage -std=c99
BINARIES=ftserver


server: ftserver.c
	-pkill ftserver
	gcc -o ftserver ftserver.c $(CFLAGS)
	./ftserver $(PORT)

client: ftclient.py
	python3 ftclient.py $(PORT)

clean:
	-pkill ftserver
	rm -f *.o *.gcov *.gcda *.gcno *.so *.out *.log $(BINARIES)
