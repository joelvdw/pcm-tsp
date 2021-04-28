//
//  AtomicStampedReference.h
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _ASR_H_
#define _ASR_H_

template <class T>
class AtomicStampedReference {
public:
  AtomicStampedReference(T reference, long stamp);

  T get(long stamp[]);
  T getReference();
  long getStamp();
  void set(T value, long stamp);
  bool compareAndSet(T expectedReference, T newReference, long expectedStamp, long newStamp);

private:
  T reference;
  long stamp;
};

#include "AtomicStampedReference.cpp"
#endif