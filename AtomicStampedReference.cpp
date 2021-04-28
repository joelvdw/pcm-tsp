//
//  AtomicStampedReference.cpp
//  Copyright (c) 2021 Favre & von der Weid. All rights reserved.
//

#ifndef _ASR_CPP_
#define _ASR_CPP_

#include "AtomicStampedReference.h"

template <class T>
AtomicStampedReference<T>::AtomicStampedReference(T reference, long stamp) {
  this->reference = reference;
  this->stamp = stamp;
}
template <class T>
T AtomicStampedReference<T>::get(long stamp[]) {
  stamp[0] = this->stamp;
  return this->reference;
}
template <class T>
T AtomicStampedReference<T>::getReference() {
  return this->reference;
}
template <class T>
long AtomicStampedReference<T>::getStamp() {
  return this->stamp;
}
template <class T>
void AtomicStampedReference<T>::set(T value, long stamp) {
  this->reference = value;
  this->stamp = stamp;
}
template <class T>
bool AtomicStampedReference<T>::compareAndSet(T expectedReference, T newReference, long expectedStamp, long newStamp) {
  __transaction_atomic {
    if (expectedReference == this->reference && expectedStamp == this->stamp) {
      this->reference = newReference;
      this->stamp = newStamp;
      return true;
    }
    return false;
  }
}

#endif
