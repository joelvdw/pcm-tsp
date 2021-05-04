//
//  ConcurrentReuseQueue.cpp
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _CRQ_CPP_
#define _CRQ_CPP_

#include <cstddef>
#include "AtomicStampedReference.h"
#include "Node.h"
#include "ConcurrentReuseQueue.h"

template <class T>
ConcurrentReuseQueue<T>::ConcurrentReuseQueue() {
  Node<T>* node = new Node<T>(NULL);
  headref = new AtomicStampedReference<Node<T>>(node, 0);
  tailref = new AtomicStampedReference<Node<T>>(node, 0);
}

// template <class T>
// Node<T> ConcurrentReuseQueue<T>::allocate(T value) {
//   long stamp[1];
//   Node<T> node = freelist;
//   if (node == NULL)
//     node = new Node<T>(value);
//   else
//     freelist = node.nextref.get(stamp);

//   node.value = value;
//   return node;
// }

// template <class T>
// void ConcurrentReuseQueue<T>::free(Node<T> node) {
//   Node<T> free = freelist;
//   node.nextref.set(free, 0);
//   freelist = node;
// }

template <class T>
void ConcurrentReuseQueue<T>::enqueue(T* value) {
  Node<T>* node = new Node<T>(value); // allocate(value);
  long tailStamp[1];
  long nextStamp[1];
  long stamp[1];

  while (true) {
    Node<T>* tail = tailref->get(tailStamp);
    Node<T>* next = tail->nextref->get(nextStamp);
    if (tail == tailref->get(stamp) && stamp[0] == tailStamp[0]) {
      if (next == NULL) {
        if (tail->nextref->compareAndSet(next, node, nextStamp[0], nextStamp[0]+1)) {
          tailref->compareAndSet(tail, node, tailStamp[0], tailStamp[0]+1);
          return;
        }
      } else {
        tailref->compareAndSet(tail, next, tailStamp[0], tailStamp[0]+1);
      }
    }
  }
}

template <class T>
T* ConcurrentReuseQueue<T>::dequeue() {
  long tailStamp[1];
  long headStamp[1];
  long nextStamp[1];
  long stamp[1];

  while (true) {
    Node<T>* head = headref->get(headStamp);
    Node<T>* tail = tailref->get(tailStamp);
    Node<T>* next = head->nextref->get(nextStamp);
    if (head == headref->get(stamp) && stamp[0] == headStamp[0]) {
      if (head == tail) {
        if (next == NULL)
          return NULL;

        tailref->compareAndSet(tail, next, tailStamp[0], tailStamp[0]+1);
      } else {
        T* value = next->value;
        if (headref->compareAndSet(head, next, headStamp[0], headStamp[0]+1)) {
          head->free(); // free(head)
          
          return value;
        }
      }
    }
  }
}

template <class T>
void ConcurrentReuseQueue<T>::close() {
  Node<T>* curr = headref->getReference();
  while (curr != NULL) {
    Node<T>* tmp = curr;
    tmp.free();
    curr = curr->nextref->getReference();
    delete tmp;
  }

  // curr = freelist;
  // while (curr != NULL) {
  //   Node<T> tmp = curr;
  //   curr = curr.nextref.getReference();
  //   delete tmp;
  // }

  delete headref;
  delete tailref;
}

#endif
