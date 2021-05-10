#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/times.h>
#include <time.h>
#include <iostream>
#include <condition_variable>
#include <vector>
#include <mutex>

#include "graph.h"
#include "path.h"

#include "ConcurrentReuseQueue.h"
#include "ThreadPool.h"

using namespace std::chrono;

#define SHORTEST_LOCAL false
#define COUNTERS false

#define DEFAULT_THREADS 8
#define LAST_AS_SEQ 7

typedef struct compact_path {
  unsigned char path[12];
  int len;
} compact_path_t;
compact_path_t shortest_global;

#if SHORTEST_LOCAL
thread_local path_t shortest_thread;
#endif

path_t end = { -1, -1, -1, -1, NULL };
int cptT = 0;
std::mutex mtx;
std::condition_variable cv;

#if COUNTERS
long* counters;
std::mutex mtx_cnts;
#endif


static compact_path_t compact_path(path_t* path) {
  compact_path_t s;
  s.len = path->len;
  
  unsigned char tmp = 0;
  for (int i = 0; i < path->size; ++i) {
    if (i & 0x1) {
      tmp |= (path->nodes[i] & 0xf);
      s.path[i>>1] = tmp;
    } else {
      tmp = (path->nodes[i] & 0xf) << 4;
    }
  }
  if (path->size & 0x1) {
    s.path[path->size>>1] = tmp;
  }
  return s;
}

static void uncompact_path(path_t* dst, compact_path_t src, int size) {
  for (int i = 0; i < size; ++i) {
    if (i & 0x1) {
      dst->nodes[i] = (src.path[i>>1] & 0xf);
    } else {
      dst->nodes[i] = (src.path[i>>1] & 0xf0) >> 4;
    }
  }
  dst->size = size;
  dst->len = src.len;
}


static void branch_and_bound(ConcurrentReuseQueue<path_t>* queue, graph_t *g, path_t *current, compact_path_t* shortest) {
  int size = graph_size(g);

  if (path_size(current) < size) {
    // not yet a leaf
    if (path_len(current) >= shortest->len) {
      // current already >= shortest known so far, bound
#if COUNTERS
      mtx_cnts.lock();
      counters[path_size(current)] ++;
      mtx_cnts.unlock();
#endif
    } else {
      // continue branching
      path_t* next = NULL;
      for (int i=1; i<size; i++) {
        if (path_add_node(current, i, g, 1)) {
          if (path_size(current) < LAST_AS_SEQ) {
            if (next != NULL) {
              queue->enqueue(next);
              cv.notify_one();
            }

            next = new path_t();
            path_new(next, size+1);
            path_copy(next, current);
          } else {
            branch_and_bound(queue, g, current, shortest);
          }
          path_drop_tail(current, g);
        }
      }
      if (next != NULL) {
        branch_and_bound(queue, g, next, shortest);
        path_destroy(next);
        delete next;
      }
    }
  } else {
    // this is a leaf
    path_add_node(current, 0, g, 0);

#if COUNTERS
    mtx_cnts.lock();
    counters[path_size(current)] ++;
    mtx_cnts.unlock();
#endif

    compact_path_t expected;
    __atomic_load(shortest, &expected, __ATOMIC_RELAXED);
    while (path_len(current) < expected.len) {
      compact_path_t tmp = compact_path(current);
      if (__atomic_compare_exchange(&shortest_global, &expected, &tmp, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
        break;
      }
      __atomic_load(&shortest_global, &expected, __ATOMIC_RELAXED);
    }

    path_drop_tail(current, g);
  }
}


static void task(int id, int nbthreads, graph_t* g, ConcurrentReuseQueue<path_t>* queue) {
  if (id) { /* Remove warning */ }

  // TODO init shortest local

  path_t* current;
  bool stop = false;
  while (true) {
    while ((current = queue->dequeue()) != NULL) {
      if (current->size == -1) {
        stop = true;
        break;
      }

#if SHORTEST_LOCAL
      branch_and_bound(queue, g, current, &shortest_thread);
#else
      branch_and_bound(queue, g, current, &shortest_global);
#endif

      path_destroy(current);
      delete current;
    }

    if (stop) {
      break;
    }

    std::unique_lock<std::mutex> lck(mtx);
    cptT += 1;

    if (cptT == nbthreads) {
      for (int i = 0; i < (nbthreads-1); ++i) {
        queue->enqueue(&end);
      }
      cv.notify_all();
      lck.unlock();
      break;
    } else {
      cv.wait(lck);
      cptT -= 1;
      lck.unlock();
    }
  }

  // TODO merge shortest local to global
}


static void check_argv(int argc, char *argv[])
{
  if (!(argc == 2 || (argc == 3 && atoi(argv[2]) > 0))) {
    fprintf(stderr, "usage: %s filename [numberOfThreads]\n", argv[0]);
    exit(1);
  }
}

#if COUNTERS
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
#endif

int main(int argc, char *argv[]) {
  path_t shortest;
  graph_t graph;
  path_t* current = new path_t();
  struct tms tb;
  int size;
  int nbThread = DEFAULT_THREADS;
  char *fname = argv[1];

  check_argv(argc, argv);
  if (argc == 3) {
    nbThread = atoi(argv[2]);
  }

  graph_read_tsplib(&graph, fname);
  graph_print(&graph);

  size = graph_size(&graph);

  if (size > 23) {
    fprintf(stderr, "%s: Graph size too large, 23 max\n", argv[0]);
    exit(1);
  }

#if COUNTERS
  counters = alloc_counters(size+1, argv[0]);
#endif

  path_new(&shortest, size+1);
  path_add_all_nodes(&shortest, &graph);
  path_add_node(&shortest, 0, &graph, 0);
  shortest_global = compact_path(&shortest);

  path_new(current, size+1);
  path_add_node(current, 0, &graph, 0);

  ConcurrentReuseQueue<path_t>* queue = new ConcurrentReuseQueue<path_t>();
  queue->enqueue(current);

  const auto start = system_clock::now();

  ThreadPool* pool = new ThreadPool(task, nbThread, &graph, queue);
  pool->joinAll();

  long ms = duration_cast< milliseconds >(
      (system_clock::now() - start)
  ).count();
  times(&tb);

  uncompact_path(&shortest, shortest_global, shortest.size);
  path_print(&shortest, (char*)"shortest");

  printf("elapsed time: %.4fs\n", (float)ms/1000.0f);
  printf("total CPU time: %.4fs\n", (float)(tb.tms_utime + tb.tms_stime)/CLOCKS_PER_SEC);
  
#if COUNTERS
  printf("total paths bound/checks:");
  for (int i = 0; i <= size ; i++)
    printf(" %ld", counters[i]);
  printf("\n");

  free(counters);
#endif

  pool->free();
  queue->close();
  delete pool;
  delete queue; 
}
