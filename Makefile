CFLAGS=-O3 -Wall -Werror -Wextra
LDFLAGS=-O3 -lm -mcx16 -latomic -lpthread
GPP=g++ -std=c++11 $(CFLAGS)
LD=g++ -std=c++11

tsp: tsp.o path.o graph.o AtomicStampedReference.o Node.o ConcurrentReuseQueue.o
	$(LD) -o $@ $^ $(LDFLAGS)

test: test.o ConcurrentReuseQueue.o path.o graph.o
	$(LD) -o $@ $^ $(LDFLAGS)

omp:
	make tsp CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f *.o *.gch tsp test

AtomicStampedReference.o: AtomicStampedReference.cpp AtomicStampedReference.h
	$(GPP) -c $<

Node.o: Node.cpp Node.h
	$(GPP) -c $<

ThreadPool.o: ThreadPool.cpp ThreadPool.h graph.o path.o ConcurrentReuseQueue.o
	$(GPP) -c $<

ConcurrentReuseQueue.o: ConcurrentReuseQueue.cpp ConcurrentReuseQueue.h Node.o AtomicStampedReference.o
	$(GPP) -c $<

graph.o: graph.cpp graph.h
	$(GPP) -c -o $@ $<

path.o: path.cpp path.h graph.h
	$(GPP) -c -o $@ $<

tsp.o: tsp.cpp graph.h path.h
	$(GPP) -c -o $@ $<

test.o: test.cpp ThreadPool.h path.h graph.h ConcurrentReuseQueue.h
	$(GPP) -c -o $@ $<
