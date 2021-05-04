//
//  ThreadPool.cpp
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _TP_CPP_
#define _TP_CPP_

#include "ThreadPool.h"
#include <functional>
#include <thread>
#include <vector>
#include "graph.h"
#include "ConcurrentReuseQueue.h"
#include "path.h"


ThreadPool::ThreadPool(std::function<void(int id, int nbThread, graph_t* g, ConcurrentReuseQueue<path_t>* queue)> func, int nbThread, graph_t* g, ConcurrentReuseQueue<path_t>* queue){
    for (int i = 0; i < nbThread; i++) {
        pool.push_back(std::thread(func, i, nbThread, g, queue));
    }
}

void ThreadPool::joinAll(){
    for(std::thread &every_thread : pool) {
        every_thread.join();
    }
}

void ThreadPool::free(){
    pool.clear();
}

#endif