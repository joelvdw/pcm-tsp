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
#define DEFAULT_THREADS 8
#define LAST_AS_SEQ 5

path_t shortest_global;
#if SHORTEST_LOCAL
thread_local path_t shortest_thread;
#endif

path_t end = { -1, -1, -1, -1, NULL };
long* counters;
int cptT = 0;
std::mutex mtx;
std::mutex mtx_path;
std::mutex mtx_cnts;
std::condition_variable cv;


static void branch_and_bound(ConcurrentReuseQueue<path_t>* queue, graph_t *g, path_t *current, path_t *shortest, long* counters) {
  int size = graph_size(g);

  if (path_size(current) < size) {
    // not yet a leaf
    if (path_len(current) >= path_len(shortest)) {
      // current already >= shortest known so far, bound
      // mtx_cnts.lock();
      // counters[path_size(current)] ++;
      // mtx_cnts.unlock();
    } else {
      // continue branching
      path_t* next = NULL;
      for (int i=1; i<size; i++) {
        if (path_add_node(current, i, g, 1)) {
          if (next != NULL) {
            queue->enqueue(next);
            cv.notify_one();
          }

          next = new path_t();
          path_new(next, size+1);
          path_copy(next, current);
          
          path_drop_tail(current, g);
        }
      }
      if (next != NULL) {
        branch_and_bound(queue, g, next, shortest, counters);
        delete next;
      }
    }
  } else {
    // this is a leaf
    path_add_node(current, 0, g, 0);
    // mtx_cnts.lock();
    // counters[path_size(current)] ++;
    // mtx_cnts.unlock();

    mtx_path.lock();
    if (path_len(current) < path_len(shortest)) {
      path_copy(shortest, current);
    }
    mtx_path.unlock();

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
      branch_and_bound(g, current, &shortest_thread, counters);
#else
      branch_and_bound(queue, g, current, &shortest_global, counters);
#endif

      delete current;
    }

    if (stop) {
      break;
    }

    std::unique_lock<std::mutex> lck(mtx);
    cptT += 1;
    std::cout << id << " cpt : " << cptT << "/" << nbthreads << std::endl;

    if (cptT == nbthreads) {
      std::cout << "end " << id << std::endl;

      for (int i = 0; i < (nbthreads-1); ++i) {
        queue->enqueue(&end);
      }
      cv.notify_all();
      lck.unlock();
      break;
    } else {
      std::cout << "sleep " << id << std::endl;
      cv.wait(lck);
      std::cout << "wake up " << id << std::endl;
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
  counters = alloc_counters(size+1, argv[0]);

  path_new(&shortest_global, size+1);
  path_add_all_nodes(&shortest_global, &graph);
  path_add_node(&shortest_global, 0, &graph, 0);

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

  path_print(&shortest_global, (char*)"shortest");

  printf("elapsed time: %.4fs\n", (float)ms/1000.0f);
  printf("total CPU time: %.4fs\n", (float)(tb.tms_utime + tb.tms_stime)/CLOCKS_PER_SEC);
  printf("total paths bound/checks:");
  for (int i = 0; i <= size ; i++)
    printf(" %ld", counters[i]);
  printf("\n");

  free(counters);
  pool->free();
  queue->close();
  delete pool;
  delete queue; 
}
