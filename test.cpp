#include <cstddef>
#include <type_traits>
#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <vector>
#include <mutex>
#include <random>
#include "ConcurrentReuseQueue.h"
#include "ThreadPool.h"
#include "graph.h"
#include "path.h"

ConcurrentReuseQueue<int>* queue;
pthread_mutex_t mutex;
std::mutex mtx;
std::condition_variable cv;
int nbV = 500;
int nbT = 50;
int cpt = 0;
int cptT = 0;
thread_local int locCpt = 0;

void task1(int id, int nbT, graph_t* g, ConcurrentReuseQueue<path_t>* test) {
    pthread_mutex_lock(&mutex);
    std::cout << id << " starts..." << std::endl;
    pthread_mutex_unlock(&mutex);

    if (g == NULL || test == NULL) { }
        
    locCpt = 0;
    int* vals = new int[nbV];
    for (int i = 0; i < nbV; i++) {
        vals[i] = i+(id*nbV);
        queue->enqueue(&vals[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand()%5));
        locCpt += 1;
    }
    for (int i = 0; i < nbV; i++) {
        int* v = queue->dequeue();

        pthread_mutex_lock(&mutex);
        cpt += *v;
        pthread_mutex_unlock(&mutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand()%5));
    }

    pthread_mutex_lock(&mutex);
    std::cout << id << " ends : " << locCpt << std::endl;
    pthread_mutex_unlock(&mutex);

    mtx.lock();
    cptT += 1;

    if(cptT == nbT){
        mtx.unlock();
        cv.notify_all();
    } else {
        mtx.unlock();
        std::unique_lock<std::mutex> lck(mtx);
        while (cptT != nbT) cv.wait(lck);
    }

    delete vals;
}

int main()
{
    queue = new ConcurrentReuseQueue<int>();
    pthread_mutex_init(&mutex, NULL);
    ThreadPool* p = new ThreadPool(task1,nbT,NULL,NULL);

    std::cout << "Wait..." << std::endl;

    p->joinAll();

    std::cout << (((nbT*nbV)-1) * ((nbT*nbV)/2)) << " - " << cpt << std::endl;
    
    p->free();

    pthread_mutex_destroy(&mutex);
}
