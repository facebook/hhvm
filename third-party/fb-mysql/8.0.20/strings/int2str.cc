/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <iterator>

#include "integer_digits.h"
#include "m_string.h"  // IWYU pragma: keep

/*
  _dig_vec arrays are public because they are used in several outer places.
*/
const char _dig_vec_upper[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char _dig_vec_lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/**
  Converts a 64-bit integer value to its character form and moves it to the
  destination buffer followed by a terminating NUL. If radix is -2..-36, val is
  taken to be SIGNED, if radix is 2..36, val is taken to be UNSIGNED. That is,
  val is signed if and only if radix is. All other radixes are treated as bad
  and nothing will be changed in this case.

  For conversion to decimal representation (radix is -10 or 10) one should use
  the optimized #longlong10_to_str() function instead.

  @param val the value to convert
  @param dst the buffer where the string representation should be stored
  @param radix radix of scale of notation
  @param upcase true if we should use upper-case digits

  @return pointer to the ending NUL character, or nullptr if radix is bad
*/
char *ll2str(int64_t val, char *dst, int radix, bool upcase) {
  char buffer[65];
  const char *const dig_vec = upcase ? _dig_vec_upper : _dig_vec_lower;
  auto uval = static_cast<uint64_t>(val);

  if (radix < 0) {
    if (radix < -36 || radix > -2) return nullptr;
    if (val < 0) {
      *dst++ = '-';
      /* Avoid integer overflow in (-val) for LLONG_MIN (BUG#31799). */
      uval = 0ULL - uval;
    }
    radix = -radix;
  } else if (radix > 36 || radix < 2) {
    return nullptr;
  }

  char *p = std::end(buffer);
  do {
    *--p = dig_vec[uval % radix];
    uval /= radix;
  } while (uval != 0);

  const size_t length = std::end(buffer) - p;
  memcpy(dst, p, length);
  dst[length] = '\0';
  return dst + length;
}

/**
  Converts a 64-bit integer to its string representation in decimal notation.

  It is optimized for the normal case of radix 10/-10. It takes only the sign of
  radix parameter into account and not its absolute value.

  @param val the value to convert
  @param dst the buffer where the string representation should be stored
  @param radix 10 if val is unsigned, -10 if val is signed

  @return pointer to the ending NUL character
*/
static char *longlong10_to_str_imp(int64_t val, char *dst, int radix) {
  assert(radix == 10 || radix == -10);

  uint64_t uval = static_cast<uint64_t>(val);

  if (radix < 0) /* -10 */
  {
    if (val < 0) {
      *dst++ = '-';
      /* Avoid integer overflow in (-val) for LLONG_MIN (BUG#31799). */
      uval = uint64_t{0} - uval;
    }
  }

  char *end = write_digits(uval, count_digits(uval), dst);
  *end = '\0';
  return end;
}

bool fast_integer_to_string = false;

char *longlong10_to_str(int64_t val, char *dst, int radix) {
  if (fast_integer_to_string) {
    static_assert(sizeof(int64_t) == sizeof(long long),
                  "int64_t should be 64 bit");
    extern char *u64toa_jeaiii(uint64_t n, char *b);
    extern char *i64toa_jeaiii(int64_t i, char *b);
    if (radix < 0)
      return i64toa_jeaiii((int64_t)val, dst);
    else
      return u64toa_jeaiii((uint64_t)val, dst);
  } else {
    return longlong10_to_str_imp(val, dst, radix);
  }
}
