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
#ifndef incl_HPHP_UTIL_SLICE_H_
#define incl_HPHP_UTIL_SLICE_H_

#include <cinttypes>
#include <assert.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A Slice is a compact way to refer to an extent of array elements.
 * For hhvm, it's generally string slices.
 *
 * This type is designed to be passed around by value.  Methods on
 * slice are set up to match the Boost RandomAccessRange concept.  If
 * T is not const-qualified, it also models WriteableRange.
 */
template<class T>
struct Slice {
  typedef T* iterator;
  typedef T* const_iterator;
  typedef uint32_t size_type;

  Slice(T* ptr, size_type len) : ptr(ptr), len(len) {}

  const_iterator begin() const { return ptr; }
  const_iterator end() const { return ptr + len; }
  const_iterator cbegin() const { return ptr; }
  const_iterator cend() const { return ptr + len; }
  iterator begin() { return ptr; }
  iterator end() { return ptr + len; }
  size_type size() const { return len; }
  T& operator[](size_type i);
  const T& operator[](size_type i) const;
  iterator erase(iterator it);
  bool empty() const { return len == 0; }
  void clear() { len = 0; }

public:
  T* ptr;         // pointer to bytes, not necessarily \0 teriminated
  size_type len;  // number of bytes, not counting possible \0
};

typedef Slice<const char> StringSlice;
typedef Slice<char> MutableSlice;

/*
 * List extends Slice, with a capacity field, making it growable.
 * sizeof(List) == sizeof(Slice) so it should still pass by value in registers.
 */
template<class T>
struct List : Slice<T> {
  typedef typename Slice<T>::size_type size_type;

  List(T* ptr, size_type len, size_type cap)
    : Slice<T>(ptr, len), cap(cap) {
    assert(len <= cap);
    static_assert(sizeof(List<T>) == sizeof(Slice<T>), "");
  }

  // vector-like api
  size_type capacity() const { return cap; }
  void push_back(const T& value);

public:
  size_type cap;
};

template<class T>
T& Slice<T>::operator[](size_type i) {
  assert(i < len);
  return ptr[i];
}

template<class T>
const T& Slice<T>::operator[](size_type i) const {
  assert(i < len);
  return ptr[i];
}

template<class T>
typename Slice<T>::iterator Slice<T>::erase(iterator it) {
  assert(it >= begin() && it < end());
  auto last = ptr + len - 1;
  for (auto p = it; p < last; ++p) p[0] = p[1];
  len--;
  return it;
}

template<class T>
void List<T>::push_back(const T& value) {
  assert(this->len < this->cap); // caller must have reserved space.
  this->ptr[this->len++] = value;
}

//////////////////////////////////////////////////////////////////////

}

#endif
