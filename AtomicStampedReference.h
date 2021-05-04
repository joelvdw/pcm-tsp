//
//  AtomicStampedReference.h
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _ASR_H_
#define _ASR_H_

template <class T>
struct __stamped_reference {
  T* reference;
  long int stamp;
};

template <class T>
class AtomicStampedReference {
public:
  AtomicStampedReference(T* reference, long int stamp);

  T* get(long int* stamp);
  T* getReference();
  long getStamp();
  void set(T* value, long int stamp);
  bool compareAndSet(T* expectedReference, T* newReference, long int expectedStamp, long int newStamp);

private:
  __stamped_reference<T> reference;
};

#include "AtomicStampedReference.cpp"
#endif