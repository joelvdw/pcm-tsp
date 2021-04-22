//
//  graph.c
//  
//  Copyright (c) 2012 Marcelo Pasin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "graph.h"

#define MAX_NODES	10000
#define MAX_CHARS_LINE	1000

typedef enum {
	EWT_EUC_2D = 1,
	EWT_GEO
} edge_weight_t;


void graph_new(graph_t *g, int size)
{
	g->distances = (int *) malloc((size_t) (size * size) * sizeof(int));
	if (!g->distances) {
		perror("cannot allocate distances");
		exit(1);
	}
	g->size = size;
}


void graph_destroy(graph_t *g)
{
	free(g->distances);
	g->distances = 0;
	g->size = 0;
}

static int sqdist(double x0, double y0, double x1, double y1)
{
	x0 -= x1;
	y0 -= y1;
	return (int) (.5 + sqrt(x0*x0 + y0*y0));
}

static int lldist(double lo0, double la0, double lo1, double la1)
{
	double RRR = 6378.388;
	la0 = la0 * M_PI / 180.;
	lo0 = lo0 * M_PI / 180.;
	la1 = la1 * M_PI / 180.;
	lo1 = lo1 * M_PI / 180.;
	double q1 = cos(lo0 - lo1);
	double q2 = cos(la0 - la1);
	double q3 = cos(la0 + la1);
	return (int) (RRR * acos( ( (q1+1)*q2 - (q1-1)*q3 ) /2 ) + .5);
}


void graph_read_tsplib(graph_t *g, char *name)
{
	FILE *f;
	int size = 0, i, j;
	char line[MAX_CHARS_LINE];
	struct {double x, y;} vec[MAX_NODES];
	edge_weight_t ewt = EWT_EUC_2D;
	
	f = fopen(name, "r");
	if (!f) {
		perror(name);
		exit(1);
	}
	while (1) {
		fgets(line, MAX_CHARS_LINE-1, f);
		if (!strncmp("DIMENSION", line, 10)) {
			sscanf(line+11, "%d", &size);
			if (size > MAX_NODES) {
				perror("too many points in input");
				exit(1);
			}
		} else if (!strncmp("EDGE_WEIGHT_TYPE", line, 16)) {
			char *p = line + 17;
			while (*p && isspace(*p)) p++;
			if (!strncmp("EUC_2D", p, 6)) {
				ewt = EWT_EUC_2D;
			} else if (!strncmp("GEO", p, 3)) {
				ewt = EWT_GEO;
			} else {
				fprintf(stderr, "wrong EDGE_WEIGHT_TYPE parameter '%s'\n", p);
				exit(1);
			}
		} else if (!strncmp("NODE_COORD_SECTION", line, 18))
			break;
		if (feof(f)) {
			perror(name);
			exit(1);
		}
	}
	for (i=0; i<size; i++) {
		if (fscanf(f, "%d %lf %lf", &j, &vec[i].x, &vec[i].y) != 3) {
			perror("missing data in input file");
			exit(1);
		}
		if (i != (j-1)){
			perror("wrong data in input file");
			exit(1);
		}
	}
	fclose(f);
	graph_new(g, size);

	for (i=0; i<size; i++) {
		graph_distance(g, i, i) = 0;
		for (j=0; j<i; j++) {
			int dist = 0;
			switch (ewt) {
				case EWT_GEO:
					dist = lldist(vec[i].x, vec[i].y, vec[j].x, vec[j].y);
					break;
				case EWT_EUC_2D:
					dist = sqdist(vec[i].x, vec[i].y, vec[j].x, vec[j].y);
					break;
			}
			graph_distance(g, j, i) = dist;
			graph_distance(g, i, j) = dist;
		}
	}
}


void graph_print(graph_t *g)
{
	int i, j;
	
	printf("     ");
	for (i=0; i<g->size; i++)
		printf("%5d", i);
	printf("\n");
	for (i=0; i<g->size; i++) {
		printf("%5d", i);
		for (j=0; j<g->size; j++)
			printf("%5d", graph_distance(g, i, j));
		printf("\n");
	}
}
