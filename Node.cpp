#ifndef __NODE_CPP_
#define __NODE_CPP_

#include <cstddef>
#include "AtomicStampedReference.h"
#include "Node.h"

template <class T>
Node<T>::Node(T* v) {
  this->value = v;
  this->nextref = new AtomicStampedReference<Node<T>>(NULL, 0L);
}

template <class T>
void Node<T>::free() {
  delete nextref;
}

#endif