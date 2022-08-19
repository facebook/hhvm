#ifndef INCLUDE_ADD_WITH_SATURATE_H_
#define INCLUDE_ADD_WITH_SATURATE_H_

/*
   Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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
  @file include/add_with_saturate.h
  Some utilities for saturating add.
 */

#include <limits>
#include <type_traits>

// Returns a + b, saturated to the range of a.
template <class T, class U>
inline T AddWithSaturate(T a, U b) {
  static_assert(!std::is_signed<T>::value, "values must be unsigned");
  static_assert(!std::is_signed<U>::value, "values must be unsigned");

  auto result = a + b;
  if (result < a || result > std::numeric_limits<T>::max()) {
    // Value wrapped around or was promoted to a larger type; saturate to the
    // maximum value.
    return std::numeric_limits<T>::max();
  }
  return result;
}

// Effectively does b += a; with saturation.
template <class T, class U>
inline void AddWithSaturate(T a, U *b) {
  *b = AddWithSaturate(*b, a);
}

#endif  // INCLUDE_ADD_WITH_SATURATE_H_
