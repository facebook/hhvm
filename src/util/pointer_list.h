/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __POINTERLIST_H__
#define __POINTERLIST_H__

#include <stdlib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define INITIAL_CAPACITY  4

struct PointerListData;

template <typename T>
struct PointerList {
  PointerListData * m_data;
  PointerList();
  ~PointerList();
  T *& operator[](int index);
  const T *& operator[](int index) const;
  bool empty();
  int size();
  int capacity();
  void clear();
  void grow();
  void push(T * val);
  void pop();
};

struct PointerListData {
  int m_len;
  int m_capacity;
};

template <typename T>
PointerList<T>::PointerList() : m_data(NULL) {
}

template <typename T>
PointerList<T>::~PointerList() {
  clear();
}

template <typename T>
T *& PointerList<T>::operator[](int index) {
  return *((T**)(m_data+1) + index);
}
template <typename T>
const T *& PointerList<T>::operator[](int index) const {
  return *((T**)(m_data+1) + index);
}
template <typename T>
bool PointerList<T>::empty() {
  if (!m_data) return true;
  return (m_data->m_len == 0);
}
template <typename T>
int PointerList<T>::size() {
  if (m_data) {
    return m_data->m_len;
  } else {
    return 0;
  }
}
template <typename T>
int PointerList<T>::capacity() {
  if (m_data) {
    return m_data->m_capacity;
  } else {
    return 0;
  }
}
template <typename T>
void PointerList<T>::clear() {
  if (m_data) {
    free(m_data);
    m_data = NULL;
  }
}
template <typename T>
void PointerList<T>::grow() {
  if (m_data) {
    int newCapacity = m_data->m_capacity * 2;
    PointerListData * new_data = (PointerListData*)malloc(
      sizeof(PointerListData) + sizeof(T*) * newCapacity);
    new_data->m_len = m_data->m_len;
    new_data->m_capacity = newCapacity;
    memcpy(new_data+1, m_data+1, sizeof(T*) * m_data->m_len);
    free(m_data);
    m_data = new_data;
  } else {
    m_data = (PointerListData*)malloc(
      sizeof(PointerListData) + sizeof(T*) * INITIAL_CAPACITY);
    m_data->m_len = 0;
    m_data->m_capacity = INITIAL_CAPACITY;
  }
}
template <typename T>
void PointerList<T>::push(T * val) {
  if (!m_data || m_data->m_len == m_data->m_capacity) grow();
  (*this)[m_data->m_len] = val;
  ++(m_data->m_len);
}
template <typename T>
void PointerList<T>::pop() {
  if (m_data) {
    if (m_data->m_len > 0) {
      --(m_data->m_len);
    }
  }
}


#undef INITIAL_CAPACITY

///////////////////////////////////////////////////////////////////////////////
}

#endif

