/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef STATELESS_ALLOCATOR_INCLUDED
#define STATELESS_ALLOCATOR_INCLUDED

#include <stddef.h>
#include <limits>
#include <new>
#include <utility>  // std::forward

#include "my_compiler.h"
#include "my_dbug.h"

/**
  Functor struct which invokes my_free. Declared here as it is used as the
  defalt value for Stateless_allocator's DEALLOC_FUN template parameter.
*/
struct My_free_functor {
  void operator()(void *p, size_t) const;
};

/**
  Stateless_allocator is a C++ STL memory allocator skeleton based on
  Malloc_allocator, which assumes that a global free function can be
  used to allocate and deallocate memory, so that no state need to be
  kept by the allocator object.

  The allocation and deallocation functions must be provided as
  callable types (aka functors) which have no state and can be default
  constructed.

  Example usage:

  @verbatim
  struct My_psi_key_alloc
  {
    void* operator(size_t s)()
    {
       return my_malloc(My_psi_key, s, MYF(MY_WME | ME_FATALERROR));
    }
  };

  template <class T>
  using My_psi_key_allocator =
       Stateless_allocator<T, My_psi_key_alloc>;

  template < template<class T> class Allocator >
  using default_string=
        std::basic_string<char, std::char_traits<char>, Allocator<char> >;


  typedef default_string<My_psi_key_allocator> My_psi_key_str;

  My_psi_key_str x("foobar");
  @endverbatim

  Since a Stateless_allocator instance is always
  default-constructible, it can also be used to create instances of
  std::basic_string, even with compilers that have this libstd++ bug:
  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=56437 "basic_string
  assumes that allocators are default-constructible".

  @note allocate() throws std::bad_alloc() similarly to the default
  STL memory allocator. This is necessary - STL functions which allocate
  memory expect it. Otherwise these functions will try to use the memory,
  leading to seg faults if memory allocation was not successful.

*/

template <class T, class ALLOC_FUN, class DEALLOC_FUN = My_free_functor>
class Stateless_allocator {
 public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef T *pointer;
  typedef const T *const_pointer;

  typedef T &reference;
  typedef const T &const_reference;

  template <class T_>
  using Stateless_allocator_type =
      Stateless_allocator<T_, ALLOC_FUN, DEALLOC_FUN>;

  Stateless_allocator() = default;

  pointer address(reference r) const { return &r; }
  const_pointer address(const_reference r) const { return &r; }

  template <class U>
  Stateless_allocator(const Stateless_allocator_type<U> &) {}

  template <class U>
  Stateless_allocator &operator=(const Stateless_allocator_type<U> &) {}

  pointer allocate(size_type n,
                   const_pointer hint MY_ATTRIBUTE((unused)) = nullptr) {
    if (n == 0) return nullptr;
    if (n > max_size()) throw std::bad_alloc();

    pointer p = static_cast<pointer>(ALLOC_FUN()(n * sizeof(T)));
    if (p == nullptr) throw std::bad_alloc();
    return p;
  }

  void deallocate(pointer p, size_type n) { DEALLOC_FUN()(p, n); }

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
    typedef Stateless_allocator<U, ALLOC_FUN, DEALLOC_FUN> other;
  };
};

template <class T, class ALLOC_FUN, class DEALLOC_FUN>
bool operator==(const Stateless_allocator<T, ALLOC_FUN, DEALLOC_FUN> &,
                const Stateless_allocator<T, ALLOC_FUN, DEALLOC_FUN> &) {
  return true;
}

template <class T, class ALLOC_FUN, class DEALLOC_FUN>
bool operator!=(const Stateless_allocator<T, ALLOC_FUN, DEALLOC_FUN> &,
                const Stateless_allocator<T, ALLOC_FUN, DEALLOC_FUN> &) {
  return false;
}

#endif  // STATELESS_ALLOCATOR_INCLUDED
