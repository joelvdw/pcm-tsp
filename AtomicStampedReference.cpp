//
//  AtomicStampedReference.cpp
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _ASR_CPP_
#define _ASR_CPP_

#include "AtomicStampedReference.h"

template <class T>
AtomicStampedReference<T>::AtomicStampedReference(T* reference, long int stamp) {
  this->reference = { reference, stamp };
}
template <class T>
T* AtomicStampedReference<T>::get(long int stamp[]) {
  stamp[0] = this->reference.stamp;
  return this->reference.reference;
}
template <class T>
T* AtomicStampedReference<T>::getReference() {
  return this->reference.reference;
}
template <class T>
long AtomicStampedReference<T>::getStamp() {
  return this->reference.stamp;
}
template <class T>
void AtomicStampedReference<T>::set(T* reference, long int stamp) {
  this->reference = { reference, stamp };
}
template <class T>
bool AtomicStampedReference<T>::compareAndSet(T* expectedReference, T* newReference, long int expectedStamp, long int newStamp) {
  __stamped_reference<T> expected = { expectedReference, expectedStamp };
  __stamped_reference<T> desired = { newReference, newStamp };

  return __atomic_compare_exchange(&reference, &expected, &desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#endif
