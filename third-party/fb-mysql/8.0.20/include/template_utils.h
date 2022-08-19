/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TEMPLATE_UTILS_INCLUDED
#define TEMPLATE_UTILS_INCLUDED

#include <assert.h>
#include <stddef.h>
#include <type_traits>

/**
  @file include/template_utils.h
*/

/**
  Clears a container, but deletes all objects that the elements point to first.
  @tparam Container_type Container of pointers.
 */
template <typename Container_type>
void delete_container_pointers(Container_type &container) {
  typename Container_type::iterator it1 = container.begin();
  typename Container_type::iterator it2 = container.end();
  for (; it1 != it2; ++it1) {
    delete (*it1);
  }
  container.clear();
}

/**
  Clears a container, but frees all objects that the elements point to first.
  @tparam Container_type Container of pointers.
 */
template <typename Container_type>
void my_free_container_pointers(Container_type &container) {
  typename Container_type::iterator it1 = container.begin();
  typename Container_type::iterator it2 = container.end();
  for (; it1 != it2; ++it1) {
    my_free(*it1);
  }
  container.clear();
}

/**
  Casts from one pointer type, to another, without using
  reinterpret_cast or C-style cast:
    foo *f; bar *b= pointer_cast<bar*>(f);
  This avoids having to do:
    foo *f; bar *b= static_cast<bar*>(static_cast<void*>(f));
 */
template <typename T>
inline T pointer_cast(void *p) {
  return static_cast<T>(p);
}

template <typename T>
inline const T pointer_cast(const void *p) {
  return static_cast<T>(p);
}

/**
  Casts from one pointer type to another in a type hierarchy.
  In debug mode, we verify the cast is indeed legal.

  @tparam Target The descendent type, must be a pointer type.
  @tparam Source The parent type.

  @param arg The pointer to be down-cast.

  @return A pointer of type Target.
*/
template <typename Target, typename Source>
inline Target down_cast(Source *arg) {
  assert(nullptr != dynamic_cast<Target>(arg));
  return static_cast<Target>(arg);
}

/**
  Casts from one reference type to another in a type hierarchy.
  In debug mode, we verify the cast is indeed legal.

  @tparam Target The descendent type, must be a reference type.
  @tparam Source The parent type.

  @param arg The reference to be down-cast.

  @return A reference of type Target.
*/
template <typename Target, typename Source>
inline Target down_cast(Source &arg) {
  // We still use the pointer version of dynamic_cast, as the
  // reference-accepting version throws exceptions, and we don't want to deal
  // with that.
  assert(dynamic_cast<typename std::remove_reference<Target>::type *>(&arg) !=
         nullptr);
  return static_cast<Target>(arg);
}

/**
   Sometimes the compiler insists that types be the same and does not do any
   implicit conversion. For example:
   Derived1 *a;
   Derived2 *b; // Derived1 and 2 are children classes of Base
   Base *x= cond ? a : b; // Error, need to force a cast.

   Use:
   Base *x= cond ? implicit_cast<Base*>(a) : implicit_cast<Base*>(b);
   static_cast would work too, but would be less safe (allows any
   pointer-to-pointer conversion, not only up-casts).
*/
template <typename To>
inline To implicit_cast(To x) {
  return x;
}

/**
   Utility to allow returning values from functions which can fail
   (until we have std::optional).
 */
template <class VALUE_TYPE>
struct ReturnValueOrError {
  /** Value returned from function in the normal case. */
  VALUE_TYPE value;

  /** True if an error occured. */
  bool error;
};

/**
   Number of elements in a constant C array.
 */
template <class T, size_t N>
constexpr size_t array_elements(T (&)[N]) noexcept {
  return N;
}

#endif  // TEMPLATE_UTILS_INCLUDED
