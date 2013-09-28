/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_POINTERLIST_H_
#define incl_HPHP_POINTERLIST_H_

#include <stdlib.h>
#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define INITIAL_CAPACITY  4

struct PointerListData;

/**
 * During its lifetime, an instance of PointerList can transition
 * between 3 states:
 *   State 0: m_val == 0
 *     This is the initial state for a newly constructed PointerList.
 *     There are no elements and no malloced block of memory.
 *   State 1: (m_val & 1) != 0
 *     In this state there is exactly one element which resides directly
 *     in m_val. The element is retrieved by returning a copy of m_val
 *     with the low bit cleared.
 *   State 2: m_val != 0 && (m_val & 1) == 0
 *     In this state, m_data points to a malloced block of memory. The
 *     number of elements, the capacity of the block, and the values of
 *     all the elements reside in the malloced block of memory.
 */
template <typename T>
struct PointerList {
  union {
    PointerListData* m_data;
    uintptr_t m_val; // m_val is provided for convenience so that we don't
                     // have to repeatedly cast m_data to a uintptr_t
  };
  PointerList();
  ~PointerList();
  T* get(int index) const;
  void set(int index, T* val);
  bool empty() const;
  int size() const;
  int capacity();
  void clear();
  void grow();
  void push(T* val);
  void pop();

private:
  PointerList(const PointerList&);
  PointerList& operator=(const PointerList&);
};

struct PointerListData {
  int m_len;
  int m_capacity;
};

template <typename T>
PointerList<T>::PointerList() {
  m_val = 0;
}

template <typename T>
PointerList<T>::~PointerList() {
  clear();
}

template <typename T>
T* PointerList<T>::get(int index) const {
  if (m_val & uintptr_t(1U)) {
    // If the low bit is set, that means that this PointerList
    // contains exactly one pointer which is stored in m_val
    assert(index == 0);
    // Clear the low bit before returning the value
    return (T*)(m_val & ~(uintptr_t(1U)));
  }
  // Index into the malloced block of memory
  assert(m_data);
  assert(index >= 0 && index < m_data->m_len);
  return *((T**)(m_data+1) + index);
}

template <typename T>
void PointerList<T>::set(int index, T* val) {
  if (m_val & uintptr_t(1U)) {
    // If the low bit is set, that means that this PointerList
    // contains exactly one pointer which is stored in m_val.
    assert(index == 0);
    if (!(uintptr_t(val) & uintptr_t(1U))) {
      // If the new value's lowest bit is zero, we can store
      // the new value directly into m_val, mark the low bit,
      // and return.
      m_val = uintptr_t(val) | uintptr_t(1U);
      return;
    }
    // Otherwise the new value's lowest bit is not zero, so we can't
    // use the trick where we store the value directly into m_val. To
    // handle this case we call the grow method (which will allocate
    // a chunk of memory) and then we fall through to the case below.
    grow();
  }
  // If we reach this point, m_data should be non-NULL and it should
  // not have its low bit set.
  assert(!(m_val & uintptr_t(1U)));
  assert(m_data);
  assert(index >= 0 && index < m_data->m_len);
  // Index into the malloced block of memory
  *((T**)(m_data+1) + index) = val;
}

template <typename T>
bool PointerList<T>::empty() const {
  if (!m_val) return true;
  return size() == 0;
}

template <typename T>
int PointerList<T>::size() const {
  if (m_val) {
    if (m_val & uintptr_t(1U)) {
      // If the low bit is set, that means we have exactly one element
      return 1;
    }
    // Read the malloced block of memory to find out how many elements
    // we have
    return m_data->m_len;
  } else {
    // If m_data is NULL, that means we have no elements
    return 0;
  }
}

template <typename T>
int PointerList<T>::capacity() {
  if (m_val && !(m_val & uintptr_t(1U))) {
    // If the m_data is non-NULL and the low bit is not set, read
    // the malloced block of memory to find out how many elements
    // can be stored before we need to grow.
    return m_data->m_capacity;
  }
  // If m_data is NULL or if the low bit is set, that means that
  // storing more elements may require us to grow, so we return
  // a capacity equal to our current size.
  return size();
}

template <typename T>
void PointerList<T>::clear() {
  if (m_val) {
    if (!(m_val & uintptr_t(1U))) {
      // If the low bit is not set, we need to free the malloced block
      // of memory
      free(m_data);
    }
    // Set m_data to NULL
    m_val = 0;
  }
}

template <typename T>
void PointerList<T>::grow() {
  if (m_val) {
    int len;
    int newCapacity;
    void* elms;
    bool doFree;
    if (m_val & uintptr_t(1U)) {
      // If the low bit is set, that means we have exactly one element and
      // that our new capacity should be INITIAL_CAPACITY. We also must clear
      // the low bit so that we can memcpy this one element into the newly
      // allocated block of memory.
      len = 1;
      newCapacity = INITIAL_CAPACITY;
      m_val = m_val & ~(uintptr_t(1U));
      elms = (void*)(&m_val);
      doFree = false;
    } else {
      len = m_data->m_len;
      newCapacity = m_data->m_capacity * 2;
      elms = (void*)(m_data+1);
      doFree = true;
    }
    PointerListData * new_data = (PointerListData*)malloc(
      sizeof(PointerListData) + sizeof(T*) * newCapacity);
    new_data->m_len = len;
    new_data->m_capacity = newCapacity;
    void* newElms = (void*)(new_data+1);
    // Copy over all of the elements in the newly allocated block of memory
    memcpy(newElms, elms, sizeof(T*) * len);
    if (doFree) {
      free(m_data);
    }
    m_data = new_data;
  } else {
    // If there are currently no elements, all we have to do is allocate a
    // block of memory and initialize m_len and m_capacity.
    m_data = (PointerListData*)malloc(
      sizeof(PointerListData) + sizeof(T*) * INITIAL_CAPACITY);
    m_data->m_len = 0;
    m_data->m_capacity = INITIAL_CAPACITY;
  }
  // Sanity check that malloc always returns a chunk of memory
  // that is aligned on a 2-byte boundary at the very least.
  // This is important because we steal the low bit of m_data
  // for our own nefarious purposes.
  assert(!(uintptr_t(m_data) & uintptr_t(1U)));
}

template <typename T>
void PointerList<T>::push(T* val) {
  if (!m_val && !(uintptr_t(val) & uintptr_t(1U))) {
    // If m_data is null and the low bit on the new value is zero,
    // we can store the new value directly into m_data and mark
    // the low bit.
    m_val = (uintptr_t(val) | uintptr_t(1U));
    return;
  }
  int sz = size();
  if (sz >= capacity()) grow();
  ++(m_data->m_len);
  set(sz, val);
}

template <typename T>
void PointerList<T>::pop() {
  if (m_val) {
    if (uintptr_t(m_val) & uintptr_t(1U)) {
      // If the value being popped was stored directly into m_data,
      // we just need to null out m_data
      m_val = 0;
    } else if (m_data->m_len > 0) {
      // Otherwise, we just decrement the length
      --(m_data->m_len);
    }
  }
}

#undef INITIAL_CAPACITY

///////////////////////////////////////////////////////////////////////////////
}

#endif

