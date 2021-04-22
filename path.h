//
//  path.h
//  
//  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
//

#ifndef _path_h
#define _path_h

#include "graph.h"


#define path_size(p)	((p)->size)
#define path_len(p)	((p)->len)

typedef struct {
	int size;
	int max_size;
	int len;
	int pad;
	int *nodes;
} path_t;


void path_new(path_t *p, int size);
void path_clear(path_t *p);
void path_print(path_t *p, char *str);
int path_add_node(path_t *p, int node, graph_t *g, int check);
void path_add_all_nodes(path_t *p, graph_t *g);
void path_drop_tail(path_t *p, graph_t *g);
void path_copy(path_t *dst, path_t *src);
void path_destroy(path_t *p);

#endif
