#ifndef SQL_INTRUSIVE_LIST_ITERATOR_H_
#define SQL_INTRUSIVE_LIST_ITERATOR_H_
/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file intrusive_list_iterator.h

  Iterator utilities for working with intrusive pointers.
*/

#include "my_dbug.h"

/**
  An iterator that follows a 'next' pointer with an accessor function.
  @tparam T The type of the object holding the intrusive list.
  @tparam GetNextPointer The accessor function, returning a pointer to the
  next object in the list.

  @note Due to the nature of intrusive 'next' pointers it's not possible to
  free an intrusive pointee while iterating over an intrusive list with
  the pre-increment operator, as the enhanced for-loop does, e.g.

  ```
  for(auto elem : elems)
    delete *elem;
  ```

  Will cause a core dump. However, the following is possible:

  ```
  auto it = container.begin();
  while(it != container.end()) delete *(it++);
  ```
*/
template <typename T, T *(*GetNextPointer)(const T *)>
class NextFunctionIterator {
 public:
  using value_type = T *;
  /**
    Constructs an iterator.

    @param start The object that the iterator will start iterating
    from.
  */
  explicit NextFunctionIterator(T *start) : m_current(start) {}

  /// Constructs a past-the-end iterator.
  NextFunctionIterator() : m_current(nullptr) {}

  NextFunctionIterator &operator++() {
    DBUG_ASSERT(m_current != nullptr);
    m_current = GetNextPointer(m_current);
    return *this;
  }

  NextFunctionIterator operator++(int) {
    auto pre_increment(*this);
    ++(*this);
    return pre_increment;
  }

  T *operator*() const { return m_current; }

  bool operator==(const NextFunctionIterator &other) const {
    return m_current == other.m_current;
  }

  bool operator!=(const NextFunctionIterator &other) const {
    return !((*this) == other);
  }

 private:
  T *m_current;
};

/**
  Helper template for the case when the 'next' member can be used directly,
  typically when it's public and the class definition is known.
*/
template <typename T, T *T::*Member>
T *GetMember(const T *t) {
  return t->*Member;
}

/**
  An iterator that follows the 'next' pointer in an intrusive list.
  Conforms to the ForwardIterator named requirement.

  @tparam T The type of the object holding the intrusive list.
  @tparam NextPointer The intrusive list's "next" pointer member.
*/
template <typename T, T *T::*NextPointer>
class IntrusiveListIterator
    : public NextFunctionIterator<T, GetMember<T, NextPointer>> {
 public:
  IntrusiveListIterator() = default;
  explicit IntrusiveListIterator(T *t)
      : NextFunctionIterator<T, GetMember<T, NextPointer>>(t) {}
};

/**
  Adds a collection interface on top of an iterator. The iterator must support a
  default constructor constructing a past-the-end iterator.

  @tparam IteratorType The iterator's class.
*/
template <typename IteratorType>
class IteratorContainer {
 public:
  using Type = typename IteratorType::value_type;
  explicit IteratorContainer(Type first) : m_first(first) {}

  IteratorType begin() { return IteratorType(m_first); }
  IteratorType end() { return IteratorType(); }

 private:
  Type m_first;
};

template <typename T>
using GetNextPointerFunction = T *(*)(const T *);

/**
  Convenience alias for instantiating a container directly from the accessor
  function.
*/
template <typename T, GetNextPointerFunction<T> Fn>
using NextFunctionContainer = IteratorContainer<NextFunctionIterator<T, Fn>>;

/*
  We inline the NextFunctionContainer definition below. We want to define this
  alias as NextFunctionContainer<T, &GetMember<T, NextPointer>>, but VS2019
  fails with C2996. It is likely a compiler bug.
*/
template <typename T, T *T::*NextPointer>
using IntrusiveListContainer =
    IteratorContainer<NextFunctionIterator<T, &GetMember<T, NextPointer>>>;

#endif  // SQL_INTRUSIVE_LIST_ITERATOR_H_
