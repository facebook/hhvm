/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_COMPACT_VECTOR_H_
#define incl_HPHP_COMPACT_VECTOR_H_

#include <stdlib.h>
#include <cstring>
#include <type_traits>

#include "hphp/util/assertions.h"
#include "hphp/util/safe-cast.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * During its lifetime, an instance of CompactVector can transition
 * between 2 states:
 *   State 0: m_val == 0
 *     This is the initial state for a newly constructed CompactVector.
 *     There are no elements and no malloced block of memory.
 *   State 1: m_val != 0
 *     In this state, m_data points to a malloced block of memory. The
 *     number of elements, the capacity of the block, and the values of
 *     all the elements reside in the malloced block of memory.
 */
template <typename T>
struct CompactVector {
  using size_type = std::size_t;

  CompactVector(CompactVector&& other) noexcept;
  CompactVector(const CompactVector& other);
  CompactVector& operator=(const CompactVector&) = delete;
  CompactVector();
  ~CompactVector();

  bool operator==(const CompactVector& other) const;

  bool operator!=(const CompactVector& other) const {
    return !(*this == other);
  }

  T* begin() { return m_data ? elems() : nullptr; }
  T* end() { return m_data ? elems() + size() : nullptr; }
  const T* begin() const { return m_data ? elems() : nullptr; }
  const T* end() const { return m_data ? elems() + size() : nullptr; }

  bool empty() const;
  size_type size() const;
  size_type capacity();
  void clear();
  void push_back(const T& val);
  template <class... Args>
  void emplace_back(Args&&... args);
  void pop_back();

  T& operator[](size_type index) { return *get(index); }
  const T& operator[](size_type index) const { return *get(index); }
  T& front() { return *get(0); }
  const T& front() const { return *get(0); }

private:
  struct CompactVectorData {
    uint32_t m_len;
    uint32_t m_capacity;
  };


  void grow();
  T* get(size_type index) const;
  T* elems() const;
  static size_t required_mem(size_type n);

  CompactVectorData* m_data;
  static constexpr size_type initial_capacity = 4;
};

template <typename T>
CompactVector<T>::CompactVector() {
  m_data = nullptr;
}

template <typename T>
CompactVector<T>::CompactVector(CompactVector&& other) noexcept
    : m_data(other.m_data) {
  other.m_data = nullptr;
}

template <typename T>
CompactVector<T>::CompactVector(const CompactVector& other) {
  if (!other.m_data) {
    m_data = nullptr;
    return;
  }
  auto const sz = required_mem(other.m_data->m_capacity);
  m_data = (CompactVectorData*)malloc(sz);
  memcpy(m_data, other.m_data, required_mem(other.m_data->m_len));
}

template <typename T>
bool CompactVector<T>::operator==(const CompactVector& other) const {
  auto const sz = size();
  if (sz != other.size()) return false;
  for (size_type i = 0; i < sz; ++i) {
    if (*get(i) != other[i]) return false;
  }
  return true;
}

template <typename T>
CompactVector<T>::~CompactVector() {
  clear();
}

template <typename T>
T* CompactVector<T>::elems() const {
  assert(m_data);
  return alignof(T) >= sizeof(CompactVectorData) ?
    (T*)((char*)m_data + alignof(T)) : (T*)(m_data + 1);
}

template <typename T>
size_t CompactVector<T>::required_mem(size_type n) {
  return std::max(alignof(T), sizeof(CompactVectorData)) + sizeof(T) * n;
}

template <typename T>
T* CompactVector<T>::get(size_type index) const {
  // Index into the malloced block of memory
  auto e = elems();
  assert(index < m_data->m_len);
  return e + index;
}

template <typename T>
bool CompactVector<T>::empty() const {
  return size() == 0;
}

template <typename T>
typename CompactVector<T>::size_type CompactVector<T>::size() const {
  return m_data ? m_data->m_len : 0;
}

template <typename T>
typename CompactVector<T>::size_type CompactVector<T>::capacity() {
  return m_data ? m_data->m_capacity : 0;
}

template <typename T>
void CompactVector<T>::clear() {
  if (!std::is_trivially_destructible<T>::value) {
    auto sz = size();
    auto elm = elems();
    while (sz--) elm++->~T();
  }
  free(m_data);
  m_data = nullptr;
}

template <typename T>
void CompactVector<T>::grow() {
  if (m_data) {
    auto const len = m_data->m_len;
    auto const new_capacity = m_data->m_capacity * 2LL;
    auto const old_data = m_data;

    m_data = (CompactVectorData*)malloc(required_mem(new_capacity));

    memcpy(m_data, old_data, required_mem(len));
    m_data->m_capacity = safe_cast<uint32_t>(new_capacity);
    free(old_data);
  } else {
    // If there are currently no elements, all we have to do is allocate a
    // block of memory and initialize m_len and m_capacity.
    m_data = (CompactVectorData*)malloc(required_mem(initial_capacity));
    m_data->m_len = 0;
    m_data->m_capacity = initial_capacity;
  }
}

template <typename T>
void CompactVector<T>::push_back(const T& val) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(val);
}

template <typename T>
template <class... Args>
void CompactVector<T>::emplace_back(Args&&... args) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(std::forward<Args>(args)...);
}

template <typename T>
void CompactVector<T>::pop_back() {
  if (m_data && m_data->m_len > 0) {
    // Otherwise, we just decrement the length
    --(m_data->m_len);
  }
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
