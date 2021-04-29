CFLAGS=-O3 -Wall -Werror -Wextra
LDFLAGS=-O3 -lm
GPP=g++ -std=c++11 $(CFLAGS) -fgnu-tm
LD=g++ -std=c++11 $(LDFLAGS) -fgnu-tm

tsp: tsp.o path.o graph.o AtomicStampedReference.o ConcurrentReuseQueue.o
	$(LD) -o $@ $^ $(LDFLAGS)

omp:
	make tsp CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f *.o *.gch tsp

AtomicStampedReference.o: AtomicStampedReference.cpp AtomicStampedReference.h
	$(GPP) -c $<

ConcurrentReuseQueue.o: ConcurrentReuseQueue.cpp ConcurrentReuseQueue.h AtomicStampedReference.h
	$(GPP) -c $<

graph.o: graph.cpp graph.h
	$(GPP) -c -o $@ $<

path.o: path.cpp path.h graph.h
	$(GPP) -c -o $@ $<

tsp.o: tsp.cpp graph.h path.h
	$(GPP) -c -o $@ $<
