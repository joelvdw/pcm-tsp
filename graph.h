//
//  graph.h
//
//  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
//

#ifndef _graph_h
#define _graph_h


typedef struct {
	int size;
	int pad;
	int *distances;
} graph_t;

#define graph_distance(g, i, j) ((g)->distances[i * (g)->size + j])
#define graph_size(g)		((g)->size)

void graph_new(graph_t *g, int size);
void graph_destroy(graph_t *g);
void graph_read_tsplib(graph_t *g, char *name);
void graph_print(graph_t *g);

#endif
