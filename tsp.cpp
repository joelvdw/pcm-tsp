#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>

#include "graph.h"
#include "path.h"



static void branch_and_bound(graph_t *g, path_t *current, path_t *shortest, long* counters, char verbose)
{
	int i;
	int size = graph_size(g);

	if (verbose)
		path_print(current, (char*)"analysing");

	if (path_size(current) < size) {
		// not yet a leaf
		if (path_len(current) >= path_len(shortest)) {
			// current already >= shortest known so far, bound
			counters[path_size(current)] ++;
			if (verbose)
				path_print(current, (char*)"bound");
		} else {
			// continue branching
			for (i=1; i<size; i++) {
				if (path_add_node(current, i, g, 1)) {
					branch_and_bound(g, current, shortest, counters, verbose);
					path_drop_tail(current, g);
				}
			}
		}
	} else {
		// this is a leaf
		path_add_node(current, 0, g, 0);
		counters[path_size(current)] ++;
		if (path_len(current) < path_len(shortest)) {
			if (verbose)
				path_print(current, (char*)"shorter");
			path_copy(shortest, current);
		}
		path_drop_tail(current, g);
	}
}


static void check_argv(int argc, char *argv[])
{
	if (!(argc == 2 || (argc == 3 && !strcmp(argv[1], "-v")))) {
		fprintf(stderr, "usage: %s [-v] filename\n", argv[0]);
		exit(1);
	}
}


static long *alloc_counters(int size, char *pname)
{
	long *counters = (long *) malloc((size_t) size * sizeof(long));
	if (counters == 0) {
		fprintf(stderr, "%s: can't allocate counters\n", pname);
		exit(1);
	}
	for (int i = 0; i < size ; i++)
		counters[i] = 0;
	return counters;
}


int main(int argc, char *argv[])
{
	graph_t graph;
	path_t current, shortest;
	struct tms tb;
	time_t start;
	long *counters;
	int size;
	char *fname = argv[1];
	char verbose = 0;

	start = time(0);

	check_argv(argc, argv);
	if (argc == 3) {
		verbose = 1;
		fname = argv[2];
	}
	graph_read_tsplib(&graph, fname);
	graph_print(&graph);

	size = graph_size(&graph);
	counters = alloc_counters(size+1, argv[0]);

	path_new(&shortest, size+1);
	path_add_all_nodes(&shortest, &graph);
	path_add_node(&shortest, 0, &graph, 0);

	path_new(&current, size+1);
	path_add_node(&current, 0, &graph, 0);

	branch_and_bound(&graph, &current, &shortest, counters, verbose);
	path_print(&shortest, (char*)"shortest");

	times(&tb);
	printf("elapsed time: %lds\n", time(0) - start);
	printf("total CPU time: %lds\n", (tb.tms_utime + tb.tms_stime)/CLOCKS_PER_SEC);
	printf("total paths bound/checks:");
	for (int i = 0; i <= size ; i++)
		printf(" %ld", counters[i]);
	printf("\n");
}
