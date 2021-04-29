#ifndef __NODE_H_
#define __NODE_H_

#include "AtomicStampedReference.h"

template <class T>
class Node {
public:
  T* value;
  AtomicStampedReference<Node<T>>* nextref;

  Node(T* v);
  void free();
};

#include "Node.cpp"
#endif