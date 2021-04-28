CFLAGS=-O3 -Wall -Werror -Wextra
LDFLAGS=-lm
GPP=g++ -std=c++11 $(CFLAGS) -fgnu-tm
CC=gcc $(CFLAGS)

tsp: tsp.o path.o graph.o AtomicStampedReference.o ConcurrentReuseQueue.o
	$(GPP) $(LDFLAGS) -o $@ $^

omp:
	make tsp CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f *.o *.gch tsp

AtomicStampedReference.o: AtomicStampedReference.cpp AtomicStampedReference.h
	$(GPP) -c $<

ConcurrentReuseQueue.o: ConcurrentReuseQueue.cpp ConcurrentReuseQueue.h AtomicStampedReference.h
	$(GPP) -c $<

graph.o: graph.c graph.h
	$(GPP) -c $<

path.o: path.c path.h graph.h
	$(GPP) -c $<

tsp.o: tsp.cpp graph.h path.h
	$(GPP) -c $<
