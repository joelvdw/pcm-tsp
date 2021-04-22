#  Copyright (c) 2012 Marcelo Pasin. All rights reserved.

CFLAGS=-O3 -Weverything
LDFLAGS=-O3 -lm

tsp:	tsp.o path.o graph.o

omp:
	make tsp CFLAGS="-fopenmp -O3" LDFLAGS="-fopenmp -O3"

clean:
	rm -f *.o tsp

# DO NOT DELETE

graph.o: graph.h
path.o: path.h graph.h
tsp.o: graph.h path.h
