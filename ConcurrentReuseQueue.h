//
//  ConcurrentReuseQueue.h
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _CRQ_H_
#define _CRQ_H_

#include <cstddef>
#include "AtomicStampedReference.h"

template <class T>
class Node {
public:
   T value;
  AtomicStampedReference<Node<T>> nextref;

  Node(T v) {
      this.value = v;
      this.nextref = new AtomicStampedReference<Node<T>>(NULL, 0);
  }

  void free() {
    delete nextref;
  }
};

template <class T>
class ConcurrentReuseQueue {
private:
	AtomicStampedReference<Node<T>> headref;
	AtomicStampedReference<Node<T>> tailref;
	// thread_local Node<T> freelist = NULL;

	// Node<T> allocate(T value);
	// void free(Node<T> node);

public:
	ConcurrentReuseQueue();

  void enqueue(T value);
	T dequeue();
  void close();
};

#include "ConcurrentReuseQueue.cpp"
#endif