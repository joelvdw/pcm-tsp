//
//  ThreadPool.h
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <functional>
#include <thread>
#include <vector>
#include "graph.h"
#include "ConcurrentReuseQueue.h"
#include "path.h"

class ThreadPool {
private:
	std::vector<std::thread> pool;

public:
  ThreadPool(std::function<void(int id, int nbThread, graph_t* g, ConcurrentReuseQueue<path_t>* queue)> func, int nbThread, graph_t* g, ConcurrentReuseQueue<path_t>* queue);
  void joinAll();
  void free();
};

#include "ThreadPool.cpp"
#endif