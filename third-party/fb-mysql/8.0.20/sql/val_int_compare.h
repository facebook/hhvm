/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef VAL_INT_COMPARE_INCLUDED
#define VAL_INT_COMPARE_INCLUDED

#include <functional>

#include "my_dbug.h"
#include "my_inttypes.h"

/**
  Holds value/unsigned_flag for the result of val_int(),
  so that we can compare with operator<(), operator==() and operator<=()
 */
class Integer_value {
 public:
  constexpr Integer_value(longlong val, bool unsigned_flag)
      : m_val(val), m_unsigned_flag(unsigned_flag) {}

  constexpr longlong val() const { return m_val; }
  constexpr bool is_unsigned() const { return m_unsigned_flag; }

  ulonglong val_unsigned() const {
    DBUG_ASSERT(!is_negative());
    return static_cast<ulonglong>(m_val);
  }

  constexpr bool is_negative() const { return !is_unsigned() && val() < 0; }

 private:
  const longlong m_val;
  const bool m_unsigned_flag;
};

inline bool operator<(const Integer_value &lhs, const Integer_value &rhs) {
  const bool lhs_is_neg = lhs.is_negative();
  const bool rhs_is_neg = rhs.is_negative();
  if (lhs_is_neg != rhs_is_neg)
    // Different signs, lhs is smaller if it is negative.
    return lhs_is_neg;
  if (lhs_is_neg)
    // Both are negative, compare as signed.
    return lhs.val() < rhs.val();

  // Both are non-negative, compare as unsigned
  return std::less<ulonglong>()(lhs.val(), rhs.val());
}

inline bool operator==(const Integer_value &lhs, const Integer_value &rhs) {
  const bool lhs_is_neg = lhs.is_negative();
  const bool rhs_is_neg = rhs.is_negative();
  if (lhs_is_neg != rhs_is_neg)
    // Different signs, cannot be equal
    return false;

  // Same sign, compare values.
  return lhs.val() == rhs.val();
}

inline bool operator<=(const Integer_value &lhs, const Integer_value &rhs) {
  return lhs < rhs || lhs == rhs;
}

#endif  // VAL_INT_COMPARE_INCLUDED
