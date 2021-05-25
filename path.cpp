//
//  path.c
//  
//  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "path.h"
#include "graph.h"


void path_clear(path_t *p)
{
	p->size = 0;
	p->len = 0;
}


void path_new(path_t *p, int size)
{
	p->nodes = (int *) malloc((size_t) size * sizeof(int));
	if (!p->nodes) {
		perror("cannot allocate path");
		exit(1);
	}
	p->max_size = size;
	p->bitmap = 0;
	path_clear(p);
}

void path_destroy(path_t *p)
{
	free(p->nodes);
	p->nodes = 0;
	p->max_size = 0;
	p->bitmap = 0;
}


void path_print(path_t *p, char *str)
{
	int i;

	if (p->size == 0) {
		printf("%s path [0]\n", str);
		return;
	}
	printf("%s path [%d]: ", str, p->len);
	for (i=0; i<p->size; i++)
		printf(" %d", p->nodes[i]);
	printf("\n");
}


void path_add_all_nodes(path_t *p, graph_t *g)
{
	for (int i=0; i<graph_size(g); i++)
		path_add_node(p, i, g, 0);
}


int path_add_node(path_t *p, int node, graph_t *g, int check)
{
	if (check) {
		if ((p->bitmap >> node) & 1) {
			return 0;
		}
	}
	
	int i = p->size;

	if (i<p->max_size) {
		p->nodes[i] = node;
		if (i)
			p->len += graph_distance(g, p->nodes[i-1], node);
		p->size ++;
		p->bitmap |= (1 << node);
		return 1;
	}
	path_print(p, (char*)"error");
	perror("path too small");
	exit(1);
}


void path_drop_tail(path_t *p, graph_t *g)
{
	if (p->size) {
		p->size --;
		p->bitmap &= ~(1 << p->nodes[p->size]);
	}

	if (p->size)
		p->len -= graph_distance(g, p->nodes[p->size], p->nodes[p->size-1]);
	else
		p->len = 0;
}


void path_copy(path_t *dst, path_t *src)
{
	int i;

	// if (dst->max_size < src->size) {
	// 	perror("path won't fit");
	// 	exit(1);
	// }
	dst->size = src->size;
	for (i=0; i<src->size; i++)
		dst->nodes[i] = src->nodes[i];
	dst->len = src->len;
	dst->bitmap = src->bitmap;
}
