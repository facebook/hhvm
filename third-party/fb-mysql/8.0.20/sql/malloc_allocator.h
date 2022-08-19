/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MALLOC_ALLOCATOR_INCLUDED
#define MALLOC_ALLOCATOR_INCLUDED

#include <limits>
#include <new>
#include <utility>  // std::forward

#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/psi_memory_key.h"

/**
  Malloc_allocator is a C++ STL memory allocator based on my_malloc/my_free.

  This allows for P_S instrumentation of memory allocation done by
  internally by STL container classes.

  Example usage:
  vector<int, Malloc_allocator<int>>
    v((Malloc_allocator<int>(PSI_NOT_INSTRUMENTED)));

  If the type is complicated, you can just write Malloc_allocator<>(psi_key)
  as a shorthand for Malloc_allocator<My_complicated_type>(psi_key), as all
  Malloc_allocator instances are implicitly convertible to each other
  and there is a default template parameter.

  @note allocate() throws std::bad_alloc() similarly to the default
  STL memory allocator. This is necessary - STL functions which allocates
  memory expects it. Otherwise these functions will try to use the memory,
  leading to segfaults if memory allocation was not successful.

  @note This allocator cannot be used for std::basic_string with RHEL 6/7
  because of this bug:
  https://bugzilla.redhat.com/show_bug.cgi?id=1546704
  "Define _GLIBCXX_USE_CXX11_ABI gets ignored by gcc in devtoolset-7"
*/

template <class T = void *>
class Malloc_allocator {
  // This cannot be const if we want to be able to swap.
  PSI_memory_key m_key;

 public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef T *pointer;
  typedef const T *const_pointer;

  typedef T &reference;
  typedef const T &const_reference;

  pointer address(reference r) const { return &r; }
  const_pointer address(const_reference r) const { return &r; }

  explicit Malloc_allocator(PSI_memory_key key) : m_key(key) {}

  template <class U>
  Malloc_allocator(const Malloc_allocator<U> &other MY_ATTRIBUTE((unused)))
      : m_key(other.psi_key()) {}

  template <class U>
  Malloc_allocator &operator=(
      const Malloc_allocator<U> &other MY_ATTRIBUTE((unused))) {
    DBUG_ASSERT(m_key == other.psi_key());  // Don't swap key.
  }

  pointer allocate(size_type n,
                   const_pointer hint MY_ATTRIBUTE((unused)) = nullptr) {
    if (n == 0) return nullptr;
    if (n > max_size()) throw std::bad_alloc();

    pointer p = static_cast<pointer>(
        my_malloc(m_key, n * sizeof(T), MYF(MY_WME | ME_FATALERROR)));
    if (p == nullptr) throw std::bad_alloc();
    return p;
  }

  void deallocate(pointer p, size_type) { my_free(p); }

  template <class U, class... Args>
  void construct(U *p, Args &&... args) {
    DBUG_ASSERT(p != nullptr);
    try {
      ::new ((void *)p) U(std::forward<Args>(args)...);
    } catch (...) {
      DBUG_ASSERT(false);  // Constructor should not throw an exception.
    }
  }

  void destroy(pointer p) {
    DBUG_ASSERT(p != nullptr);
    try {
      p->~T();
    } catch (...) {
      DBUG_ASSERT(false);  // Destructor should not throw an exception
    }
  }

  size_type max_size() const {
    return std::numeric_limits<size_t>::max() / sizeof(T);
  }

  template <class U>
  struct rebind {
    typedef Malloc_allocator<U> other;
  };

  PSI_memory_key psi_key() const { return m_key; }
};

template <class T>
bool operator==(const Malloc_allocator<T> &a1, const Malloc_allocator<T> &a2) {
  return a1.psi_key() == a2.psi_key();
}

template <class T>
bool operator!=(const Malloc_allocator<T> &a1, const Malloc_allocator<T> &a2) {
  return a1.psi_key() != a2.psi_key();
}

#endif  // MALLOC_ALLOCATOR_INCLUDED
