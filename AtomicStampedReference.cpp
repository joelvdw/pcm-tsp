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
T* AtomicStampedReference<T>::get(long int* stamp) {
  __stamped_reference<T> load;
  __atomic_load(&this->reference, &load, __ATOMIC_RELAXED);
  *stamp = load.stamp;
  return load.reference;
}
template <class T>
T* AtomicStampedReference<T>::getReference() {
  long int s;
  return get(&s);
}
template <class T>
long AtomicStampedReference<T>::getStamp() {
  long int s;
  get(&s);
  return s;
}
template <class T>
void AtomicStampedReference<T>::set(T* reference, long int stamp) {
  __stamped_reference<T> load;
  load.reference = reference;
  load.stamp = stamp;
  __atomic_store(&this->reference, &load, __ATOMIC_RELAXED);
  this->reference = { reference, stamp };
}
template <class T>
bool AtomicStampedReference<T>::compareAndSet(T* expectedReference, T* newReference, long int expectedStamp, long int newStamp) {
  __stamped_reference<T> expected = { expectedReference, expectedStamp };
  __stamped_reference<T> desired = { newReference, newStamp };

  return __atomic_compare_exchange(&reference, &expected, &desired, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}

#endif
