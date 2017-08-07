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
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;

  friend iterator begin(CompactVector& v) { return v.begin(); }
  friend iterator end(CompactVector& v) { return v.end(); }
  friend const_iterator begin(const CompactVector& v) { return v.begin(); }
  friend const_iterator end(const CompactVector& v) { return v.end(); }

  CompactVector();
  explicit CompactVector(size_type n);
  CompactVector(size_type n, const T&);
  CompactVector(CompactVector&& other) noexcept;
  CompactVector(const CompactVector& other);
  CompactVector& operator=(CompactVector&&);
  CompactVector& operator=(const CompactVector&);
  ~CompactVector();

  void swap(CompactVector& other) noexcept;

  bool operator==(const CompactVector& other) const;

  bool operator!=(const CompactVector& other) const {
    return !(*this == other);
  }

  iterator begin() { return m_data ? elems() : nullptr; }
  iterator end() { return m_data ? elems() + size() : nullptr; }
  const_iterator begin() const { return m_data ? elems() : nullptr; }
  const_iterator end() const { return m_data ? elems() + size() : nullptr; }
  T* data() { return m_data ? elems() : nullptr; }
  const T* data() const { return m_data ? elems() : nullptr; }

  bool empty() const;
  size_type size() const;
  size_type capacity();
  void clear();
  void push_back(const T& val);
  void push_back(T&& val);
  template <class... Args>
  void emplace_back(Args&&... args);
  void pop_back();
  void erase(iterator);
  void erase(iterator, iterator);
  void resize(size_type sz);
  void resize(size_type sz, const value_type& value);
  void shrink_to_fit();

  T& operator[](size_type index) { return *get(index); }
  const T& operator[](size_type index) const { return *get(index); }
  T& front() { return *get(0); }
  const T& front() const { return *get(0); }
  T& back() { return *get(m_data->m_len - 1); }
  const T& back() const { return *get(m_data->m_len - 1); }

  void reserve(size_type sz);
private:
  struct CompactVectorData {
    uint32_t m_len;
    uint32_t m_capacity;
  };


  void assign(const CompactVector& other);
  void grow();
  T* get(size_type index) const;
  T* elems() const;
  static size_t required_mem(size_type n);
  void reserve_impl(size_type sz);
  bool resize_helper(size_type sz);

  CompactVectorData* m_data;
  static constexpr size_type initial_capacity = 4;
  /* We mainly want this so pretty.py can figure out the alignment */
  static constexpr size_type elems_offset =
    alignof(T) >= sizeof(CompactVectorData) ?
    alignof(T) : sizeof(CompactVectorData);
  /* And we need this to prevent gcc from throwing away elems_offset */
  using elems_offset_type = char[elems_offset];
};

template <typename T>
CompactVector<T>::CompactVector() {
  m_data = nullptr;
}

template <typename T>
CompactVector<T>::CompactVector(size_type n) {
  m_data = nullptr;
  resize(n);
}

template <typename T>
CompactVector<T>::CompactVector(size_type n, const T& val) {
  m_data = nullptr;
  resize(n, val);
}

template <typename T>
CompactVector<T>::CompactVector(CompactVector&& other) noexcept
    : m_data(other.m_data) {
  other.m_data = nullptr;
}

template <typename T>
CompactVector<T>::CompactVector(const CompactVector& other) : m_data(nullptr) {
  assign(other);
}

template <typename T>
CompactVector<T>& CompactVector<T>::operator=(const CompactVector& other) {
  if (this == &other) return *this;
  clear();
  assign(other);
  return *this;
}

template <typename T>
void CompactVector<T>::assign(const CompactVector& other) {
  assert(!m_data);
  if (!other.size()) return;
  reserve_impl(other.m_data->m_len);
  auto const sz = other.m_data->m_len;
  for (size_type i = 0; i < sz; ++i) {
    push_back(other[i]);
  }
}

template <typename T>
CompactVector<T>& CompactVector<T>::operator=(CompactVector&& other) {
  std::swap(m_data, other.m_data);
  return *this;
}

template <typename T>
void CompactVector<T>::swap(CompactVector& other) noexcept {
  std::swap(m_data, other.m_data);
}

template <typename T>
bool CompactVector<T>::operator==(const CompactVector& other) const {
  auto const sz = size();
  if (sz != other.size()) return false;
  for (size_type i = 0; i < sz; ++i) {
    if (!(*get(i) == other[i])) return false;
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
  return (T*)((char*)m_data + elems_offset);
}

template <typename T>
size_t CompactVector<T>::required_mem(size_type n) {
  return elems_offset + sizeof(T) * n;
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
void CompactVector<T>::erase(iterator elm) {
  assert(elm - elems() < size());
  elm->~T();
  m_data->m_len--;
  memmove(elm, elm + 1, (char*)end() - (char*)elm);
}

template <typename T>
void CompactVector<T>::erase(iterator elm1, iterator elm2) {
  if (elm1 == elm2) return;
  assert(elems() <= elm1 && elm1 <= elm2 && elm2 <= end());
  for (auto elm = elm1; elm < elm2; elm++) {
    elm->~T();
  }
  memmove(elm1, elm2, (char*)end() - (char*)elm2);
  m_data->m_len -= elm2 - elm1;
}

template <typename T>
bool CompactVector<T>::resize_helper(size_type sz) {
  auto const old_size = size();
  if (sz == old_size) return true;
  if (sz > old_size) {
    reserve_impl(sz);
    return false;
  }
  auto elm = get(sz);
  m_data->m_len = sz;
  do {
    elm++->~T();
  } while (++sz < old_size);
  return true;
}

template <typename T>
void CompactVector<T>::resize(size_type sz, const value_type& v) {
  if (resize_helper(sz)) return;
  while (m_data->m_len < sz) {
    push_back(v);
  }
}

template <typename T>
void CompactVector<T>::resize(size_type sz) {
  if (resize_helper(sz)) return;
  while (m_data->m_len < sz) {
    push_back(T{});
  }
}

template <typename T>
void CompactVector<T>::shrink_to_fit() {
  if (!m_data || m_data->m_capacity == m_data->m_len) return;
  if (!m_data->m_len) {
    clear();
    return;
  }
  reserve_impl(m_data->m_len);
}

template <typename T>
void copy(CompactVector<T>& dest, const std::vector<T>& src) {
  dest.clear();
  dest.reserve(src.size());
  for (auto const& v : src) dest.push_back(v);
}

template <typename T>
void CompactVector<T>::clear() {
  if (!std::is_trivially_destructible<T>::value) {
    if (auto sz = size()) {
      auto elm = elems();
      do { elm++->~T(); } while (--sz);
    }
  }
  free(m_data);
  m_data = nullptr;
}

template <typename T>
void CompactVector<T>::grow() {
  reserve_impl(m_data ? m_data->m_capacity * 2LL : initial_capacity);
}

template <typename T>
void CompactVector<T>::reserve_impl(size_type new_capacity) {
  if (m_data) {
    auto const len = m_data->m_len;
    auto const old_data = m_data;

    m_data = (CompactVectorData*)malloc(required_mem(new_capacity));

    memcpy(m_data, old_data, required_mem(len));
    m_data->m_capacity = safe_cast<uint32_t>(new_capacity);
    free(old_data);
  } else {
    // If there are currently no elements, all we have to do is allocate a
    // block of memory and initialize m_len and m_capacity.
    m_data = (CompactVectorData*)malloc(required_mem(new_capacity));
    m_data->m_len = 0;
    m_data->m_capacity = new_capacity;
  }
}

template <typename T>
void CompactVector<T>::reserve(size_type new_capacity) {
  if (new_capacity > capacity()) reserve_impl(new_capacity);
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
void CompactVector<T>::push_back(T&& val) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(std::move(val));
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
