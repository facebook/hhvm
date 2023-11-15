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

#pragma once

#include <stdlib.h>
#include <cstring>
#include <memory>
#include <type_traits>
#include <initializer_list>

#include "hphp/util/assertions.h"
#include "hphp/util/safe-cast.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * During its lifetime, an instance of CompactVector can transition
 * between 2 states:
 *   State 0: m_val == 0
 *     This is the initial state for a newly constructed CompactVector.
 *     There are no elements and no allocated block of memory.
 *   State 1: m_val != 0
 *     In this state, m_data points to a malloced block of memory. The
 *     number of elements, the capacity of the block, and the values of
 *     all the elements reside in the allocated block of memory.
 */
template <typename T, typename Alloc = std::allocator<char>>
struct CompactVector : private std::allocator_traits<Alloc>::template rebind_alloc<char> {
  using size_type = std::size_t;
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;
  using Allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<char>;

  friend iterator begin(CompactVector& v) { return v.begin(); }
  friend iterator end(CompactVector& v) { return v.end(); }
  friend const_iterator begin(const CompactVector& v) { return v.begin(); }
  friend const_iterator end(const CompactVector& v) { return v.end(); }

  CompactVector();
  explicit CompactVector(size_type n);
  CompactVector(size_type n, const T&);
  CompactVector(CompactVector&& other) noexcept;
  CompactVector(const CompactVector& other);
  CompactVector(std::initializer_list<T> init);
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
  size_type capacity() const;
  void clear();
  void push_back(const T& val);
  void push_back(T&& val);
  template <class... Args>
  void emplace_back(Args&&... args);
  void pop_back();
  void erase(iterator);
  void erase(iterator, iterator);
  iterator insert(iterator p, const T& v) { return insert_impl(p, 1, v); }
  iterator insert(iterator p, T&& v) { return insert_impl(p, 1, std::move(v)); };
  iterator insert(iterator p, size_t num, const T& v) {
    return insert_impl(p, num, v);
  };
  template<typename U>
  iterator insert(iterator p, U i1, U i2);
  void resize(size_type sz);
  void resize(size_type sz, const value_type& value);
  void shrink_to_fit();

  // Return the element at index, if index < size. Otherwise resize
  // the vector to index and return the (default-constructed) element.
  T& ensure(size_t index) {
    if (index >= size()) resize(index+1);
    return *get(index);
  }
  T& ensure(size_t index, const value_type& d) {
    if (index >= size()) resize(index+1, d);
    return *get(index);
  }

  T& operator[](size_type index) { return *get(index); }
  const T& operator[](size_type index) const { return *get(index); }
  T& front() { return *get(0); }
  const T& front() const { return *get(0); }
  T& back() { return *get(m_data->m_len - 1); }
  const T& back() const { return *get(m_data->m_len - 1); }

  // Return the element at index, if index < size. Otherwise return
  // the default value provided.
  T get_default(size_t index, const value_type& d) const {
    if (index < size()) return *get(index);
    return d;
  }

  void reserve(size_type sz);
private:
  using Allocator::allocate;
  using Allocator::deallocate;

  struct CompactVectorData {
    uint32_t m_len;
    uint32_t m_capacity;
  };

  iterator insert_elems(iterator, size_t num);
  template <class U>
  iterator insert_impl(iterator, size_t num, U&&);
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

template <typename T, typename A>
CompactVector<T, A>::CompactVector() {
  m_data = nullptr;
}

template <typename T, typename A>
CompactVector<T, A>::CompactVector(size_type n) {
  m_data = nullptr;
  resize(n);
}

template <typename T, typename A>
CompactVector<T, A>::CompactVector(size_type n, const T& val) {
  m_data = nullptr;
  resize(n, val);
}

template <typename T, typename A>
CompactVector<T, A>::CompactVector(CompactVector&& other) noexcept
  : m_data(other.m_data) {
  other.m_data = nullptr;
}

template <typename T, typename A>
CompactVector<T, A>::CompactVector(const CompactVector& other)
  : m_data(nullptr) {
  assign(other);
}

template <typename T, typename A>
CompactVector<T, A>&
CompactVector<T, A>::operator=(const CompactVector& other) {
  if (this == &other) return *this;
  clear();
  assign(other);
  return *this;
}

template <typename T, typename A>
CompactVector<T, A>::CompactVector(std::initializer_list<T> init) :
    m_data(nullptr) {
  reserve_impl(init.size());
  for (auto const& e : init) {
    push_back(e);
  }
}

template <typename T, typename A>
void CompactVector<T, A>::assign(const CompactVector& other) {
  assert(!m_data);
  if (!other.size()) return;
  reserve_impl(other.m_data->m_len);
  auto const sz = other.m_data->m_len;
  for (size_type i = 0; i < sz; ++i) {
    push_back(other[i]);
  }
}

template <typename T, typename A>
CompactVector<T, A>& CompactVector<T, A>::operator=(CompactVector&& other) {
  std::swap(m_data, other.m_data);
  return *this;
}

template <typename T, typename A>
void CompactVector<T, A>::swap(CompactVector& other) noexcept {
  std::swap(m_data, other.m_data);
}

template <typename T, typename A>
bool CompactVector<T, A>::operator==(const CompactVector& other) const {
  auto const sz = size();
  if (sz != other.size()) return false;
  for (size_type i = 0; i < sz; ++i) {
    if (!(*get(i) == other[i])) return false;
  }
  return true;
}

template <typename T, typename A>
CompactVector<T, A>::~CompactVector() {
  clear();
}

template <typename T, typename A>
T* CompactVector<T, A>::elems() const {
  assert(m_data);
  return (T*)((char*)m_data + elems_offset);
}

template <typename T, typename A>
size_t CompactVector<T, A>::required_mem(size_type n) {
  return elems_offset + sizeof(T) * n;
}

template <typename T, typename A>
T* CompactVector<T, A>::get(size_type index) const {
  // Index into the allocated block of memory
  auto e = elems();
  assert(index < m_data->m_len);
  return e + index;
}

template <typename T, typename A>
bool CompactVector<T, A>::empty() const {
  return size() == 0;
}

template <typename T, typename A>
typename CompactVector<T, A>::size_type CompactVector<T, A>::size() const {
  return m_data ? m_data->m_len : 0;
}

template <typename T, typename A>
typename CompactVector<T, A>::size_type CompactVector<T, A>::capacity() const {
  return m_data ? m_data->m_capacity : 0;
}

template <typename T, typename A>
void CompactVector<T, A>::erase(iterator elm) {
  assert(elm - elems() < size());
  auto const e = end();
  while (++elm != e) {
    elm[-1] = std::move(*elm);
  }
  elm[-1].~T();
  m_data->m_len--;
}

template <typename T, typename A>
void CompactVector<T, A>::erase(iterator elm1, iterator elm2) {
  if (elm1 == elm2) return;
  assert(elems() <= elm1 && elm1 <= elm2 && elm2 <= end());
  auto const e = end();
  while (elm2 != e) {
    *elm1++ = std::move(*elm2++);
  }
  m_data->m_len -= elm2 - elm1;
  while (elm1 != e) {
    elm1++->~T();
  }
}

template <typename T, typename A>
bool CompactVector<T, A>::resize_helper(size_type sz) {
  auto const old_size = size();
  if (sz == old_size) return true;
  if (sz > old_size) {
    reserve(sz);
    return false;
  }
  auto elm = get(sz);
  m_data->m_len = sz;
  do {
    elm++->~T();
  } while (++sz < old_size);
  return true;
}

template <typename T, typename A>
void CompactVector<T, A>::resize(size_type sz, const value_type& v) {
  if (resize_helper(sz)) return;
  while (m_data->m_len < sz) {
    push_back(v);
  }
}

template <typename T, typename A>
void CompactVector<T, A>::resize(size_type sz) {
  if (resize_helper(sz)) return;
  while (m_data->m_len < sz) {
    push_back(T{});
  }
}

template <typename T, typename A>
void CompactVector<T, A>::shrink_to_fit() {
  if (!m_data || m_data->m_capacity == m_data->m_len) return;
  if (!m_data->m_len) {
    clear();
    return;
  }
  reserve_impl(m_data->m_len);
}

template <typename T, typename A>
void copy(CompactVector<T, A>& dest, const std::vector<T>& src) {
  dest.clear();
  dest.reserve(src.size());
  for (auto const& v : src) dest.push_back(v);
}

template <typename T, typename A>
void CompactVector<T, A>::clear() {
  if (!m_data) return;
  if (!std::is_trivially_destructible<T>::value) {
    if (auto sz = size()) {
      auto elm = elems();
      do { elm++->~T(); } while (--sz);
    }
  }
  deallocate(reinterpret_cast<char*>(m_data),
             required_mem(m_data->m_capacity));
  m_data = nullptr;
}

template <typename T, typename A>
void CompactVector<T, A>::grow() {
  reserve_impl(m_data ? m_data->m_capacity * 2LL : initial_capacity);
}

template <typename T, typename A>
void CompactVector<T, A>::reserve_impl(size_type new_capacity) {
  auto new_data = (CompactVectorData*)allocate(required_mem(new_capacity));
  if (!new_data) throw std::bad_alloc{};

  if (m_data) {
    auto len = m_data->m_len;
    auto old_data = m_data;
    auto const old_capacity = old_data->m_capacity;
    auto old_elems = elems();
    m_data = new_data;
    auto new_elems = elems();
    m_data->m_len = len;
    m_data->m_capacity = safe_cast<uint32_t>(new_capacity);
    while (len--) {
      new (new_elems++) T(std::move(*old_elems));
      old_elems++->~T();
    }
    deallocate(reinterpret_cast<char*>(old_data), required_mem(old_capacity));
  } else {
    // If there are currently no elements, all we have to do is allocate a
    // block of memory and initialize m_len and m_capacity.
    m_data = new_data;
    m_data->m_len = 0;
    m_data->m_capacity = new_capacity;
  }
}

template <typename T, typename A>
void CompactVector<T, A>::reserve(size_type new_capacity) {
  if (new_capacity > capacity()) reserve_impl(new_capacity);
}

template <typename T, typename A>
typename CompactVector<T, A>::iterator
CompactVector<T, A>::insert_elems(iterator before, size_t num) {
  if (!num) return before;
  auto const sz = size();
  assert(sz <= capacity());
  auto cap = capacity();
  if (sz + num > cap) {
    auto const pos = sz ? before - elems() : 0;
    assert(pos <= sz);
    cap <<= 1;
    if (sz + num > cap) {
      cap = sz + num;
      if (cap < initial_capacity) cap = initial_capacity;
    }
    reserve(cap);
    before = elems() + pos;
  }

  auto e = end();
  m_data->m_len += num;
  while (e != before) {
    --e;
    new (e + num) T(std::move(*e));
    e->~T();
  }
  return e;
}


template <typename T, typename A>
template <typename U>
typename CompactVector<T, A>::iterator
CompactVector<T, A>::insert_impl(iterator before, size_t num, U&& val) {
  auto e = insert_elems(before, num);
  while (num--) {
    new (e + num) T(std::forward<U>(val));
  }
  return e;
}

template <typename T, typename A>
template <typename U>
typename CompactVector<T, A>::iterator
CompactVector<T, A>::insert(iterator before, U i1, U i2) {
  auto e = insert_elems(before, i2 - i1);
  auto i = e;
  while (i1 != i2) {
    new (i++) T(*i1++);
  }
  return e;
}

template <typename T, typename A>
void CompactVector<T, A>::push_back(const T& val) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(val);
}

template <typename T, typename A>
void CompactVector<T, A>::push_back(T&& val) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(std::move(val));
}

template <typename T, typename A>
template <class... Args>
void CompactVector<T, A>::emplace_back(Args&&... args) {
  auto const sz = size();
  assert(sz <= capacity());
  if (sz == capacity()) grow();
  ++(m_data->m_len);
  new (get(sz)) T(std::forward<Args>(args)...);
}

template <typename T, typename A>
void CompactVector<T, A>::pop_back() {
  if (m_data && m_data->m_len > 0) {
    back().~T();
    --(m_data->m_len);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
