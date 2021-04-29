//
//  ConcurrentReuseQueue.h
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _CRQ_H_
#define _CRQ_H_

#include <cstddef>
#include "AtomicStampedReference.h"
#include "Node.h"

template <class T>
class ConcurrentReuseQueue {
private:
	AtomicStampedReference<Node<T>>* headref;
	AtomicStampedReference<Node<T>>* tailref;
	// thread_local Node<T> freelist = NULL;

	// Node<T> allocate(T value);
	// void free(Node<T> node);

public:
	ConcurrentReuseQueue();

  void enqueue(T* value);
	T* dequeue();
  void close();
};

#include "ConcurrentReuseQueue.cpp"
#endif