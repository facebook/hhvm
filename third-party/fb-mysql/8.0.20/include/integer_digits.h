/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef INTEGER_DIGITS_INCLUDED
#define INTEGER_DIGITS_INCLUDED

#include <assert.h>
#include <algorithm>
#include <limits>
#include <type_traits>

/**
  @file

  This file contains utilities for accessing digits of integers, and for
  converting them to strings.
*/

/**
  Helper class for #write_two_digits(), which creates a table that maps every
  integer from 0 to 99 to a two-char sequence that represents its two base 10
  digits.
*/
class TwoDigitWriter {
 public:
  constexpr TwoDigitWriter() {
    for (int i = 0; i < 100; ++i) {
      m_digits[i][0] = '0' + i / 10;
      m_digits[i][1] = '0' + i % 10;
    }
  }

  char *Write(int value, char *to) const {
    assert(value >= 0 && value < 100);
    return std::copy_n(m_digits[value], 2, to);
  }

 private:
  char m_digits[100][2]{};
};

/**
  Writes an integer, which is between 0 (inclusive) and 100 (exclusive), to a
  string in base 10. Always writes two digits, zero-padded if necessary. The
  string is not zero-terminated.

  @param value the number to write
  @param[in,out] to the destination string
  @return pointer to the character just after the last digit
*/
inline char *write_two_digits(int value, char *to) {
  static constexpr TwoDigitWriter writer;
  return writer.Write(value, to);
}

/**
  Functor that calculates the number of digits in an unsigned integer using
  binary search. The code for doing the binary search is generated and unrolled
  at compile time.

  @tparam T the unsigned integer type of the input to the functor
  @tparam MinDigits the minimum number of digits the integer is known to have
  @tparam MaxDigits the maximum number of digits the integer is known to have
*/
template <typename T, int MinDigits, int MaxDigits, typename = void>
struct DigitCounter {
  static_assert(MinDigits < MaxDigits, "");
  static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                "The input should be an unsigned integer.");

  constexpr int operator()(T x) const {
    constexpr int mid = (MinDigits + MaxDigits) / 2;
    constexpr T pivot = pow10(mid);
    if (x < pivot)
      return DigitCounter<T, MinDigits, mid>()(x);
    else
      return DigitCounter<T, mid + 1, MaxDigits>()(x);
  }

 private:
  static constexpr T pow10(int n) {
    T x = 1;
    for (int i = 0; i < n; ++i) x *= 10;
    return x;
  }
};

/**
  Counts the number of digits for the trivial case where the known minimum
  number of digits is equal to the known maximum number of digits.
*/
template <typename T, int MinDigits, int MaxDigits>
struct DigitCounter<T, MinDigits, MaxDigits,
                    typename std::enable_if<MinDigits == MaxDigits>::type> {
  constexpr int operator()(T) const { return MinDigits; }
};

/**
  Counts the number of base 10 digits in an unsigned integer.

  @param x the number whose digits to count
  @return the number of digits in the number
*/
template <typename T>
constexpr int count_digits(T x) {
  return DigitCounter<T, 1, std::numeric_limits<T>::digits10 + 1>()(x);
}

/**
  Writes an unsigned integer of the specified length to a string. The string is
  not zero-terminated.

  @param number the number to write
  @param digits the number of digits to write (the number is zero-padded if it
                is shorter)
  @param[in,out] to the destination string
  @return pointer to the character just after the last digit
*/
template <typename T>
inline char *write_digits(T number, int digits, char *to) {
  assert(digits >= count_digits(number));

  // The string is built from the end, starting with the least significant
  // digits.
  char *pos = to + digits;

  // The digits are written in groups of two in order to reduce the number of
  // the relatively expensive modulo and division by 10 operations. If it has an
  // odd number of digits, write the leftover digit separately.
  if (digits % 2 != 0) {
    *--pos = '0' + number % 10;
    number /= 10;
  }

  while (pos > to) {
    pos -= 2;
    write_two_digits(number % 100, pos);
    number /= 100;
  }

  return to + digits;
}

#endif  // INTEGER_DIGITS_INCLUDED
