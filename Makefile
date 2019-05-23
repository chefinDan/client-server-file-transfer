ODIR=obj
# TESTDIR=testSuite
CFLAGS= -Wall -lm -coverage -std=c99
BINARIES=ftserver

server: ftserver.c
	gcc -o ftserver ftserver.c $(CFLAGS)
	./ftserver $(PORT) &

client: ftclient.py
	python3 ftclient.py

clean:
	rm -f *.o runTests playdom.exe playdom player player.exe  *.gcov *.gcda *.gcno *.so *.out $(BINARIES)
	pkill ftserver
