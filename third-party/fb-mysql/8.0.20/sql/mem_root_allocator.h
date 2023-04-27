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

#ifndef MEM_ROOT_ALLOCATOR_INCLUDED
#define MEM_ROOT_ALLOCATOR_INCLUDED

#include <limits>
#include <new>
#include <utility>  // std::forward

#include "my_alloc.h"
#include "my_dbug.h"

/**
  Mem_root_allocator is a C++ STL memory allocator based on MEM_ROOT.

  No deallocation is done by this allocator. Calling init_sql_alloc()
  and free_root() on the supplied MEM_ROOT is the responsibility of
  the caller. Do *not* call free_root() until the destructor of any
  objects using this allocator has completed. This includes iterators.

  Example of use:
  vector<int, Mem_root_allocator<int> > v((Mem_root_allocator<int>(&mem_root)));

  @note allocate() throws std::bad_alloc() similarly to the default
  STL memory allocator. This is necessary - STL functions which allocate
  memory expect it. Otherwise these functions will try to use the memory,
  leading to seg faults if memory allocation was not successful.

  @note This allocator cannot be used for std::basic_string with RHEL 6/7
  because of this bug:
  https://bugzilla.redhat.com/show_bug.cgi?id=1546704
  "Define _GLIBCXX_USE_CXX11_ABI gets ignored by gcc in devtoolset-7"

  @note C++98 says that STL implementors can assume that allocator objects
  of the same type always compare equal. This will only be the case for
  two Mem_root_allocators that use the same MEM_ROOT. Care should be taken
  when this is not the case. Especially:
  - Using list::splice() on two lists with allocators using two different
    MEM_ROOTs causes undefined behavior. Most implementations seem to give
    runtime errors in such cases.
  - swap() on two collections with allocators using two different MEM_ROOTs
    is not well defined. At least some implementations also swap allocators,
    but this should not be depended on.
*/

template <class T>
class Mem_root_allocator {
  // This cannot be const if we want to be able to swap.
  MEM_ROOT *m_memroot;

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

  explicit Mem_root_allocator(MEM_ROOT *memroot) : m_memroot(memroot) {}

  explicit Mem_root_allocator() : m_memroot(nullptr) {}

  template <class U>
  Mem_root_allocator(const Mem_root_allocator<U> &other)
      : m_memroot(other.memroot()) {}

  template <class U>
  Mem_root_allocator &operator=(
      const Mem_root_allocator<U> &other MY_ATTRIBUTE((unused))) {
    DBUG_ASSERT(m_memroot == other.memroot());  // Don't swap memroot.
  }

  pointer allocate(size_type n,
                   const_pointer hint MY_ATTRIBUTE((unused)) = nullptr) {
    if (n == 0) return nullptr;
    if (n > max_size()) throw std::bad_alloc();

    pointer p = static_cast<pointer>(m_memroot->Alloc(n * sizeof(T)));
    if (p == nullptr) throw std::bad_alloc();
    return p;
  }

  void deallocate(pointer, size_type) {}

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
    typedef Mem_root_allocator<U> other;
  };

  MEM_ROOT *memroot() const { return m_memroot; }
};

template <class T>
bool operator==(const Mem_root_allocator<T> &a1,
                const Mem_root_allocator<T> &a2) {
  return a1.memroot() == a2.memroot();
}

template <class T>
bool operator!=(const Mem_root_allocator<T> &a1,
                const Mem_root_allocator<T> &a2) {
  return a1.memroot() != a2.memroot();
}

#endif  // MEM_ROOT_ALLOCATOR_INCLUDED
