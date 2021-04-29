#include <cstddef>
#include <type_traits>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "ConcurrentReuseQueue.h"

ConcurrentReuseQueue<int>* queue;
pthread_mutex_t mutex;
int nbV = 500;
int nbT = 50;
int cpt = 0;

void task1(int id) {
    pthread_mutex_lock(&mutex);
    std::cout << id << " starts..." << std::endl;
    pthread_mutex_unlock(&mutex);

    int* vals = new int[nbV];
    for (int i = 0; i < nbV; i++) {
        vals[i] = i+(id*nbV);
        queue->enqueue(&vals[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    for (int i = 0; i < nbV; i++) {
        int* v = queue->dequeue();

        pthread_mutex_lock(&mutex);
        cpt += *v;
        pthread_mutex_unlock(&mutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    pthread_mutex_lock(&mutex);
    std::cout << id << " ends" << std::endl;
    pthread_mutex_unlock(&mutex);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    delete vals;
}

int main(int argc, char *argv[])
{
    queue = new ConcurrentReuseQueue<int>();
    pthread_mutex_init(&mutex, NULL);

    std::vector<std::thread> pool;
    for (int i = 0; i < nbT; i++) {
        pool.push_back(std::thread(task1, i));
    }

    std::cout << "Wait..." << std::endl;

    for(std::thread &every_thread : pool) {
        every_thread.join();
    }
    std::cout << (((nbT*nbV)-1) * ((nbT*nbV)/2)) << " - " << cpt << std::endl;
    pool.clear();
    pthread_mutex_destroy(&mutex);
}
