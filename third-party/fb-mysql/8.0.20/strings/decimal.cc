/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
=======================================================================
  NOTE: this library implements SQL standard "exact numeric" type
  and is not at all generic, but rather intentinally crippled to
  follow the standard :)
=======================================================================
  Quoting the standard
  (SQL:2003, Part 2 Foundations, aka ISO/IEC 9075-2:2003)

4.4.2 Characteristics of numbers, page 27:

  An exact numeric type has a precision P and a scale S. P is a positive
  integer that determines the number of significant digits in a
  particular radix R, where R is either 2 or 10. S is a non-negative
  integer. Every value of an exact numeric type of scale S is of the
  form n*10^{-S}, where n is an integer such that ­-R^P <= n <= R^P.

  [...]

  If an assignment of some number would result in a loss of its most
  significant digit, an exception condition is raised. If least
  significant digits are lost, implementation-defined rounding or
  truncating occurs, with no exception condition being raised.

  [...]

  Whenever an exact or approximate numeric value is assigned to an exact
  numeric value site, an approximation of its value that preserves
  leading significant digits after rounding or truncating is represented
  in the declared type of the target. The value is converted to have the
  precision and scale of the target. The choice of whether to truncate
  or round is implementation-defined.

  [...]

  All numeric values between the smallest and the largest value,
  inclusive, in a given exact numeric type have an approximation
  obtained by rounding or truncation for that type; it is
  implementation-defined which other numeric values have such
  approximations.

5.3 <literal>, page 143

  <exact numeric literal> ::=
    <unsigned integer> [ <period> [ <unsigned integer> ] ]
  | <period> <unsigned integer>

6.1 <data type>, page 165:

  19) The <scale> of an <exact numeric type> shall not be greater than
      the <precision> of the <exact numeric type>.

  20) For the <exact numeric type>s DECIMAL and NUMERIC:

    a) The maximum value of <precision> is implementation-defined.
       <precision> shall not be greater than this value.
    b) The maximum value of <scale> is implementation-defined. <scale>
       shall not be greater than this maximum value.

  21) NUMERIC specifies the data type exact numeric, with the decimal
      precision and scale specified by the <precision> and <scale>.

  22) DECIMAL specifies the data type exact numeric, with the decimal
      scale specified by the <scale> and the implementation-defined
      decimal precision equal to or greater than the value of the
      specified <precision>.

6.26 <numeric value expression>, page 241:

  1) If the declared type of both operands of a dyadic arithmetic
     operator is exact numeric, then the declared type of the result is
     an implementation-defined exact numeric type, with precision and
     scale determined as follows:

   a) Let S1 and S2 be the scale of the first and second operands
      respectively.
   b) The precision of the result of addition and subtraction is
      implementation-defined, and the scale is the maximum of S1 and S2.
   c) The precision of the result of multiplication is
      implementation-defined, and the scale is S1 + S2.
   d) The precision and scale of the result of division are
      implementation-defined.
*/

#include "decimal.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <type_traits>
#include <utility>

#include "integer_digits.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sys.h" /* for my_alloca */
#include "myisampack.h"

/**
  Internally decimal numbers are stored base 10^9 (see DIG_BASE below)
  So one variable of type decimal_digit_t is limited:

      0 < decimal_digit <= DIG_MAX < DIG_BASE

  in the decimal_t:

    intg is the number of *decimal* digits (NOT number of decimal_digit_t's !)
         before the point
    frac - number of decimal digits after the point
    buf is an array of decimal_digit_t's
    len is the length of buf (length of allocated space) in decimal_digit_t's,
        not in bytes
*/
using dec1 = decimal_digit_t;
/// A wider variant of dec1, to avoid overflow in intermediate results.
using dec2 = int64_t;
/// An unsigned type with the same width as dec1.
using udec1 = std::make_unsigned<dec1>::type;

#define DIG_PER_DEC1 9
#define DIG_MASK 100000000
#define DIG_BASE 1000000000
#define DIG_MAX (DIG_BASE - 1)
#define ROUND_UP(X) (((X) + DIG_PER_DEC1 - 1) / DIG_PER_DEC1)
static const dec1 powers10[DIG_PER_DEC1 + 1] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
static const int dig2bytes[DIG_PER_DEC1 + 1] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4};
static const dec1 frac_max[DIG_PER_DEC1 - 1] = {900000000, 990000000, 999000000,
                                                999900000, 999990000, 999999000,
                                                999999900, 999999990};

static inline dec1 div_by_pow10(dec1 x, int p) {
  /*
    GCC can optimize division by a constant to a multiplication and some
    shifts, which is faster than dividing by a variable, even taking into
    account the extra cost of the switch. It is also (empirically on a Skylake)
    faster than storing the magic multiplier constants in a table and doing it
    ourselves. However, since the code is much bigger, we only use this in
    a few select places.

    Note the use of unsigned, which is faster for this specific operation.
  */
  DBUG_ASSERT(x >= 0);
  switch (p) {
    case 0:
      return static_cast<uint32_t>(x) / 1;
    case 1:
      return static_cast<uint32_t>(x) / 10;
    case 2:
      return static_cast<uint32_t>(x) / 100;
    case 3:
      return static_cast<uint32_t>(x) / 1000;
    case 4:
      return static_cast<uint32_t>(x) / 10000;
    case 5:
      return static_cast<uint32_t>(x) / 100000;
    case 6:
      return static_cast<uint32_t>(x) / 1000000;
    case 7:
      return static_cast<uint32_t>(x) / 10000000;
    case 8:
      return static_cast<uint32_t>(x) / 100000000;
    default:
      DBUG_ASSERT(false);
      return x / powers10[p];
  }
}

static inline dec1 mod_by_pow10(dec1 x, int p) {
  // See div_by_pow10 for rationale.
  DBUG_ASSERT(x >= 0);
  switch (p) {
    case 1:
      return static_cast<uint32_t>(x) % 10;
    case 2:
      return static_cast<uint32_t>(x) % 100;
    case 3:
      return static_cast<uint32_t>(x) % 1000;
    case 4:
      return static_cast<uint32_t>(x) % 10000;
    case 5:
      return static_cast<uint32_t>(x) % 100000;
    case 6:
      return static_cast<uint32_t>(x) % 1000000;
    case 7:
      return static_cast<uint32_t>(x) % 10000000;
    case 8:
      return static_cast<uint32_t>(x) % 100000000;
    default:
      DBUG_ASSERT(false);
      return x % powers10[p];
  }
}

#define sanity(d) DBUG_ASSERT((d)->len > 0)

#define FIX_INTG_FRAC_ERROR(len, intg1, frac1, error) \
  do {                                                \
    if (unlikely(intg1 + frac1 > (len))) {            \
      if (unlikely(intg1 > (len))) {                  \
        intg1 = (len);                                \
        frac1 = 0;                                    \
        error = E_DEC_OVERFLOW;                       \
      } else {                                        \
        frac1 = (len)-intg1;                          \
        error = E_DEC_TRUNCATED;                      \
      }                                               \
    } else                                            \
      error = E_DEC_OK;                               \
  } while (0)

#define ADD(to, from1, from2, carry) /* assume carry <= 1 */ \
  do {                                                       \
    dec1 a = (from1) + (from2) + (carry);                    \
    DBUG_ASSERT((carry) <= 1);                               \
    if (((carry) = a >= DIG_BASE)) /* no division here! */   \
      a -= DIG_BASE;                                         \
    (to) = a;                                                \
  } while (0)

#define ADD2(to, from1, from2, carry)             \
  do {                                            \
    dec2 a = ((dec2)(from1)) + (from2) + (carry); \
    if (((carry) = a >= DIG_BASE)) a -= DIG_BASE; \
    if (unlikely(a >= DIG_BASE)) {                \
      a -= DIG_BASE;                              \
      carry++;                                    \
    }                                             \
    (to) = (dec1)a;                               \
  } while (0)

#define SUB(to, from1, from2, carry) /* to=from1-from2 */ \
  do {                                                    \
    dec1 a = (from1) - (from2) - (carry);                 \
    if (((carry) = a < 0)) a += DIG_BASE;                 \
    (to) = a;                                             \
  } while (0)

#define SUB2(to, from1, from2, carry) /* to=from1-from2 */ \
  do {                                                     \
    dec1 a = (from1) - (from2) - (carry);                  \
    if (((carry) = a < 0)) a += DIG_BASE;                  \
    if (unlikely(a < 0)) {                                 \
      a += DIG_BASE;                                       \
      carry++;                                             \
    }                                                      \
    (to) = a;                                              \
  } while (0)

ALWAYS_INLINE static int decimal_bin_size_inline(int precision, int scale);

/*
  This is a direct loop unrolling of code that used to look like this:
  for (; *buf_beg < powers10[i--]; start++) ;

  @param   i    start index
  @param   val  value to compare against list of powers of 10

  @retval  Number of leading zeroes that can be removed from fraction.

  @note Why unroll? To get rid of lots of compiler warnings [-Warray-bounds]
        Nice bonus: unrolled code is significantly faster.
 */
static inline int count_leading_zeroes(int i, dec1 val) {
  int ret = 0;
  switch (i) {
    /* @note Intentional fallthrough in all case labels */
    case 9:
      if (val >= 1000000000) break;
      ++ret;  // Fall through.
    case 8:
      if (val >= 100000000) break;
      ++ret;  // Fall through.
    case 7:
      if (val >= 10000000) break;
      ++ret;  // Fall through.
    case 6:
      if (val >= 1000000) break;
      ++ret;  // Fall through.
    case 5:
      if (val >= 100000) break;
      ++ret;  // Fall through.
    case 4:
      if (val >= 10000) break;
      ++ret;  // Fall through.
    case 3:
      if (val >= 1000) break;
      ++ret;  // Fall through.
    case 2:
      if (val >= 100) break;
      ++ret;  // Fall through.
    case 1:
      if (val >= 10) break;
      ++ret;  // Fall through.
    case 0:
      if (val >= 1) break;
      ++ret;  // Fall through.
    default: {
      DBUG_ASSERT(false);
    }
  }
  return ret;
}

/*
  This is a direct loop unrolling of code that used to look like this:
  for (; *buf_end % powers10[i++] == 0; stop--) ;

  @param   i    start index
  @param   val  value to compare against list of powers of 10

  @retval  Number of trailing zeroes that can be removed from fraction.

  @note Why unroll? To get rid of lots of compiler warnings [-Warray-bounds]
        Nice bonus: unrolled code is significantly faster.
 */
static inline int count_trailing_zeroes(int i, dec1 val) {
  DBUG_ASSERT(val >= 0);
  uint32_t uval = val;

  int ret = 0;
  switch (i) {
    /* @note Intentional fallthrough in all case labels */
    case 0:
      if ((uval % 1) != 0) break;
      ++ret;  // Fall through.
    case 1:
      if ((uval % 10) != 0) break;
      ++ret;  // Fall through.
    case 2:
      if ((uval % 100) != 0) break;
      ++ret;  // Fall through.
    case 3:
      if ((uval % 1000) != 0) break;
      ++ret;  // Fall through.
    case 4:
      if ((uval % 10000) != 0) break;
      ++ret;  // Fall through.
    case 5:
      if ((uval % 100000) != 0) break;
      ++ret;  // Fall through.
    case 6:
      if ((uval % 1000000) != 0) break;
      ++ret;  // Fall through.
    case 7:
      if ((uval % 10000000) != 0) break;
      ++ret;  // Fall through.
    case 8:
      if ((uval % 100000000) != 0) break;
      ++ret;  // Fall through.
    case 9:
      if ((uval % 1000000000) != 0) break;
      ++ret;  // Fall through.
    default: {
      DBUG_ASSERT(false);
    }
  }
  return ret;
}

/*
  Get maximum value for given precision and scale

  SYNOPSIS
    max_decimal()
    precision/scale - see decimal_bin_size() below
    to              - decimal where where the result will be stored
                      to->buf and to->len must be set.
*/

void max_decimal(int precision, int frac, decimal_t *to) {
  int intpart;
  dec1 *buf = to->buf;
  DBUG_ASSERT(precision && precision >= frac);

  to->sign = false;
  if ((intpart = to->intg = (precision - frac))) {
    int firstdigits = intpart % DIG_PER_DEC1;
    if (firstdigits) *buf++ = powers10[firstdigits] - 1; /* get 9 99 999 ... */
    for (intpart /= DIG_PER_DEC1; intpart; intpart--) *buf++ = DIG_MAX;
  }

  if ((to->frac = frac)) {
    int lastdigits = frac % DIG_PER_DEC1;
    for (frac /= DIG_PER_DEC1; frac; frac--) *buf++ = DIG_MAX;
    if (lastdigits) *buf = frac_max[lastdigits - 1];
  }
}

static inline dec1 *remove_leading_zeroes(const decimal_t *from,
                                          int *intg_result) {
  // Round up intg so that we don't need special handling of the first word.
  int intg = ROUND_UP(from->intg) * DIG_PER_DEC1;

  // Remove all the leading words that contain only zeros.
  dec1 *buf0 = from->buf;
  while (intg > 0 && *buf0 == 0) {
    ++buf0;
    intg -= DIG_PER_DEC1;
  }

  // Now remove all the leading zeros in the first non-zero word, if there is a
  // non-zero word.
  if (intg > 0) {
    const int digits = count_digits<udec1>(*buf0);
    DBUG_ASSERT(digits <= DIG_PER_DEC1);
    intg -= DIG_PER_DEC1 - digits;
    DBUG_ASSERT(intg > 0);
  }

  DBUG_ASSERT(intg >= 0);
  DBUG_ASSERT(intg <= from->intg);
  *intg_result = intg;
  return buf0;
}

/*
  Count actual length of fraction part (without ending zeroes)

  SYNOPSIS
    decimal_actual_fraction()
    from    number for processing
*/

int decimal_actual_fraction(const decimal_t *from) {
  int frac = from->frac, i;
  const dec1 *buf0 = from->buf + ROUND_UP(from->intg) + ROUND_UP(frac) - 1;

  if (frac == 0) return 0;

  i = ((frac - 1) % DIG_PER_DEC1 + 1);
  while (frac > 0 && *buf0 == 0) {
    frac -= i;
    i = DIG_PER_DEC1;
    buf0--;
  }
  if (frac > 0) {
    frac -= count_trailing_zeroes(DIG_PER_DEC1 - ((frac - 1) % DIG_PER_DEC1),
                                  *buf0);
  }
  return frac;
}

/*
  Convert decimal to its printable string representation

  SYNOPSIS
    decimal2string()
      from            - value to convert
      to              - points to buffer where string representation
                        should be stored
      *to_len         - in:  size of to buffer (incl. terminating '\0')
                        out: length of the actually written string (excl. '\0')
      fixed_precision - 0 if representation can be variable length and
                        fixed_decimals will not be checked in this case.
                        Put number as with fixed point position with this
                        number of digits (sign counted and decimal point is
                        counted)
      fixed_decimals  - number digits after point.

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW
*/

int decimal2string(const decimal_t *from, char *to, int *to_len,
                   int fixed_precision, int fixed_decimals) {
  DBUG_ASSERT(*to_len >= 2 + from->sign);

  int intg;
  const dec1 *buf = remove_leading_zeroes(from, &intg);

  const int fixed_intg =
      fixed_precision ? (fixed_precision - fixed_decimals) : 0;
  // {intg_len, frac_len} output widths; {intg, frac} digits in input
  int intg_len = std::max(1, fixed_precision ? fixed_intg : intg);
  int frac = from->frac;
  int frac_len = fixed_precision ? fixed_decimals : frac;
  int len = from->sign + intg_len + (frac ? 1 : 0) + frac_len;
  int error = E_DEC_OK;
  if (fixed_precision) {
    if (frac > fixed_decimals) {
      error = E_DEC_TRUNCATED;
      frac = fixed_decimals;
    }
    if (intg > fixed_intg) {
      error = E_DEC_OVERFLOW;
      intg = fixed_intg;
    }
  } else if (len > --*to_len)  // reserve one byte for \0
  {
    int j = len - *to_len;  // excess printable chars
    error = (frac && j <= frac + 1) ? E_DEC_TRUNCATED : E_DEC_OVERFLOW;

    // If we need to cut more places than frac is wide, we'll end up
    // dropping the decimal point as well. Account for this.
    if (frac && j >= frac + 1) j--;

    if (j > frac) {
      intg_len = intg -= j - frac;
      frac = 0;
    } else
      frac -= j;
    frac_len = frac;
    len = from->sign + intg_len + (frac ? 1 : 0) + frac_len;
  }
  *to_len = len;
  to[len] = '\0';

  if (from->sign) *to++ = '-';

  // Prepend padding if a fixed precision was specified.
  if (fixed_precision != 0) {
    int fill = intg_len - intg;
    if (intg == 0) fill--;  // symbol 0 before digital point
    for (; fill > 0; fill--) *to++ = '0';
  }

  // Write the integer part of the decimal.
  if (intg != 0) {
    // The first word might not contain a full DIG_PER_DEC1 digits.
    const int digits_in_partial_word = intg % DIG_PER_DEC1;
    if (digits_in_partial_word != 0) {
      dec1 x = *buf++;

      // Cut the value if it is too big to fit in the buffer.
      if (x >= powers10[digits_in_partial_word]) {
        DBUG_ASSERT(error == E_DEC_OVERFLOW);
        x %= powers10[digits_in_partial_word];
      }

      to = write_digits<udec1>(x, digits_in_partial_word, to);
      intg -= digits_in_partial_word;
    }

    while (intg > 0) {
      to = write_digits<udec1>(*buf++, DIG_PER_DEC1, to);
      intg -= DIG_PER_DEC1;
    }
    DBUG_ASSERT(intg == 0);
  } else {
    *to++ = '0';
  }

  // Write the fractional part of the decimal, if there is one.
  if (frac != 0) {
    const int fill = frac_len - frac;
    *to++ = '.';

    // Write DIG_PER_DEC1 digits for all the full words.
    while (frac >= DIG_PER_DEC1) {
      to = write_digits<udec1>(*buf++, DIG_PER_DEC1, to);
      frac -= DIG_PER_DEC1;
    }

    DBUG_ASSERT(frac >= 0);
    DBUG_ASSERT(frac < DIG_PER_DEC1);

    // There can be a partial word at the end. Write only the most significant
    // digits of that word.
    if (frac > 0) {
      to = write_digits<udec1>(div_by_pow10(*buf++, DIG_PER_DEC1 - frac), frac,
                               to);
    }

    // Append padding if a fixed precision was specified.
    for (int i = 0; i < fill; ++i) *to++ = '0';
  }

  return error;
}

/*
  Return bounds of decimal digits in the number

  SYNOPSIS
    digits_bounds()
      from         - decimal number for processing
      start_result - index (from 0 ) of first decimal digits will
                     be written by this address
      end_result   - index of position just after last decimal digit
                     be written by this address
*/

static void digits_bounds(const decimal_t *from, int *start_result,
                          int *end_result) {
  int start, stop, i;
  dec1 *buf_beg = from->buf;
  dec1 *end = from->buf + ROUND_UP(from->intg) + ROUND_UP(from->frac);
  dec1 *buf_end = end - 1;

  /* find non-zero digit from number begining */
  while (buf_beg < end && *buf_beg == 0) buf_beg++;

  if (buf_beg >= end) {
    /* it is zero */
    *start_result = *end_result = 0;
    return;
  }

  /* find non-zero decimal digit from number begining */
  if (buf_beg == from->buf && from->intg) {
    start = DIG_PER_DEC1 - (i = ((from->intg - 1) % DIG_PER_DEC1 + 1));
    i--;
  } else {
    i = DIG_PER_DEC1 - 1;
    start = (int)((buf_beg - from->buf) * DIG_PER_DEC1);
  }
  if (buf_beg < end) start += count_leading_zeroes(i, *buf_beg);

  *start_result = start; /* index of first decimal digit (from 0) */

  /* find non-zero digit at the end */
  while (buf_end > buf_beg && *buf_end == 0) buf_end--;
  /* find non-zero decimal digit from the end */
  if (buf_end == end - 1 && from->frac) {
    stop = (int)(((buf_end - from->buf) * DIG_PER_DEC1 +
                  (i = ((from->frac - 1) % DIG_PER_DEC1 + 1))));
    i = DIG_PER_DEC1 - i + 1;
  } else {
    stop = (int)((buf_end - from->buf + 1) * DIG_PER_DEC1);
    i = 1;
  }
  stop -= count_trailing_zeroes(i, *buf_end);
  *end_result = stop; /* index of position after last decimal digit (from 0) */
}

/*
  Left shift for alignment of data in buffer

  SYNOPSIS
    do_mini_left_shift()
    dec     pointer to decimal number which have to be shifted
    shift   number of decimal digits on which it should be shifted
    beg/end bounds of decimal digits (see digits_bounds())

  NOTE
    Result fitting in the buffer should be garanted.
    'shift' have to be from 1 to DIG_PER_DEC1-1 (inclusive)
*/

static void do_mini_left_shift(decimal_t *dec, int shift, int beg, int last) {
  dec1 *from = dec->buf + ROUND_UP(beg + 1) - 1;
  dec1 *end = dec->buf + ROUND_UP(last) - 1;
  int c_shift = DIG_PER_DEC1 - shift;
  DBUG_ASSERT(from >= dec->buf);
  DBUG_ASSERT(end < dec->buf + dec->len);
  if (beg % DIG_PER_DEC1 < shift) *(from - 1) = (*from) / powers10[c_shift];
  for (; from < end; from++)
    *from = ((*from % powers10[c_shift]) * powers10[shift] +
             (*(from + 1)) / powers10[c_shift]);
  *from = (*from % powers10[c_shift]) * powers10[shift];
}

/*
  Right shift for alignment of data in buffer

  SYNOPSIS
    do_mini_left_shift()
    dec     pointer to decimal number which have to be shifted
    shift   number of decimal digits on which it should be shifted
    beg/end bounds of decimal digits (see digits_bounds())

  NOTE
    Result fitting in the buffer should be garanted.
    'shift' have to be from 1 to DIG_PER_DEC1-1 (inclusive)
*/

static void do_mini_right_shift(decimal_t *dec, int shift, int beg, int last) {
  dec1 *from = dec->buf + ROUND_UP(last) - 1;
  dec1 *end = dec->buf + ROUND_UP(beg + 1) - 1;
  int c_shift = DIG_PER_DEC1 - shift;
  DBUG_ASSERT(from < dec->buf + dec->len);
  DBUG_ASSERT(end >= dec->buf);
  if (DIG_PER_DEC1 - ((last - 1) % DIG_PER_DEC1 + 1) < shift)
    *(from + 1) = (*from % powers10[shift]) * powers10[c_shift];
  for (; from > end; from--)
    *from = (*from / powers10[shift] +
             (*(from - 1) % powers10[shift]) * powers10[c_shift]);
  *from = *from / powers10[shift];
}

/*
  Shift of decimal digits in given number (with rounding if it need)

  SYNOPSIS
    decimal_shift()
    dec       number to be shifted
    shift     number of decimal positions
              shift > 0 means shift to left shift
              shift < 0 meand right shift
  NOTE
    In fact it is multipling on 10^shift.
  RETURN
    E_DEC_OK          OK
    E_DEC_OVERFLOW    operation lead to overflow, number is untoched
    E_DEC_TRUNCATED   number was rounded to fit into buffer
*/

int decimal_shift(decimal_t *dec, int shift) {
  /* index of first non zero digit (all indexes from 0) */
  int beg;
  /* index of position after last decimal digit */
  int end;
  /* index of digit position just after point */
  int point = ROUND_UP(dec->intg) * DIG_PER_DEC1;
  /* new point position */
  int new_point = point + shift;
  /* length of result and new fraction in big digits*/
  int new_len, new_frac_len;
  /* return code */
  int err = E_DEC_OK;
  int new_front;

  if (shift == 0) return E_DEC_OK;

  digits_bounds(dec, &beg, &end);

  if (beg == end) {
    decimal_make_zero(dec);
    return E_DEC_OK;
  }

  /* number of digits in result */
  int digits_int = std::max(new_point - beg, 0);
  int digits_frac = std::max(end - new_point, 0);

  if ((new_len = ROUND_UP(digits_int) +
                 (new_frac_len = ROUND_UP(digits_frac))) > dec->len) {
    int lack = new_len - dec->len;
    int diff;

    if (new_frac_len < lack)
      return E_DEC_OVERFLOW; /* lack more then we have in fraction */

    /* cat off fraction part to allow new number to fit in our buffer */
    err = E_DEC_TRUNCATED;
    new_frac_len -= lack;
    diff = digits_frac - (new_frac_len * DIG_PER_DEC1);
    /* Make rounding method as parameter? */
    decimal_round(dec, dec, end - point - diff, HALF_UP);
    end -= diff;
    digits_frac = new_frac_len * DIG_PER_DEC1;

    if (end <= beg) {
      /*
        we lost all digits (they will be shifted out of buffer), so we can
        just return 0
      */
      decimal_make_zero(dec);
      return E_DEC_TRUNCATED;
    }
  }

  if (shift % DIG_PER_DEC1) {
    int l_mini_shift, r_mini_shift, mini_shift;
    int do_left;
    /*
      Calculate left/right shift to align decimal digits inside our bug
      digits correctly
    */
    if (shift > 0) {
      l_mini_shift = shift % DIG_PER_DEC1;
      r_mini_shift = DIG_PER_DEC1 - l_mini_shift;
      /*
        It is left shift so prefer left shift, but if we have not place from
        left, we have to have it from right, because we checked length of
        result
      */
      do_left = l_mini_shift <= beg;
      DBUG_ASSERT(do_left || (dec->len * DIG_PER_DEC1 - end) >= r_mini_shift);
    } else {
      r_mini_shift = (-shift) % DIG_PER_DEC1;
      l_mini_shift = DIG_PER_DEC1 - r_mini_shift;
      /* see comment above */
      do_left = !((dec->len * DIG_PER_DEC1 - end) >= r_mini_shift);
      DBUG_ASSERT(!do_left || l_mini_shift <= beg);
    }
    if (do_left) {
      do_mini_left_shift(dec, l_mini_shift, beg, end);
      mini_shift = -l_mini_shift;
    } else {
      do_mini_right_shift(dec, r_mini_shift, beg, end);
      mini_shift = r_mini_shift;
    }
    new_point += mini_shift;
    /*
      If number is shifted and correctly aligned in buffer we can
      finish
    */
    if (!(shift += mini_shift) && (new_point - digits_int) < DIG_PER_DEC1) {
      dec->intg = digits_int;
      dec->frac = digits_frac;
      return err; /* already shifted as it should be */
    }
    beg += mini_shift;
    end += mini_shift;
  }

  /* if new 'decimal front' is in first digit, we do not need move digits */
  if ((new_front = (new_point - digits_int)) >= DIG_PER_DEC1 || new_front < 0) {
    /* need to move digits */
    int d_shift;
    dec1 *to, *barier;
    if (new_front > 0) {
      /* move left */
      d_shift = new_front / DIG_PER_DEC1;
      to = dec->buf + (ROUND_UP(beg + 1) - 1 - d_shift);
      barier = dec->buf + (ROUND_UP(end) - 1 - d_shift);
      DBUG_ASSERT(to >= dec->buf);
      DBUG_ASSERT(barier + d_shift < dec->buf + dec->len);
      for (; to <= barier; to++) *to = *(to + d_shift);
      for (barier += d_shift; to <= barier; to++) *to = 0;
      d_shift = -d_shift;
    } else {
      /* move right */
      d_shift = (1 - new_front) / DIG_PER_DEC1;
      to = dec->buf + ROUND_UP(end) - 1 + d_shift;
      barier = dec->buf + ROUND_UP(beg + 1) - 1 + d_shift;
      DBUG_ASSERT(to < dec->buf + dec->len);
      DBUG_ASSERT(barier - d_shift >= dec->buf);
      for (; to >= barier; to--) *to = *(to - d_shift);
      for (barier -= d_shift; to >= barier; to--) *to = 0;
    }
    d_shift *= DIG_PER_DEC1;
    beg += d_shift;
    end += d_shift;
    new_point += d_shift;
  }

  /*
    If there are gaps then fill ren with 0.

    Only one of following 'for' loops will work becouse beg <= end
  */
  beg = ROUND_UP(beg + 1) - 1;
  end = ROUND_UP(end) - 1;
  DBUG_ASSERT(new_point >= 0);

  /* We don't want negative new_point below */
  if (new_point != 0) new_point = ROUND_UP(new_point) - 1;

  if (new_point > end) {
    do {
      dec->buf[new_point] = 0;
    } while (--new_point > end);
  } else {
    for (; new_point < beg; new_point++) dec->buf[new_point] = 0;
  }
  dec->intg = digits_int;
  dec->frac = digits_frac;
  return err;
}

/*
  Convert string to decimal

  SYNOPSIS
    string2decimal()
      from    - value to convert. Doesn't have to be \0 terminated!
      to      - decimal where where the result will be stored
                to->buf and to->len must be set.
      end     - Pointer to pointer to end of string. Will on return be
                set to the char after the last used character

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW/E_DEC_BAD_NUM/E_DEC_OOM
    In case of E_DEC_FATAL_ERROR *to is set to decimal zero
    (to make error handling easier)
*/

int string2decimal(const char *from, decimal_t *to, const char **end) {
  const char *s = from, *s1, *endp, *end_of_string = *end;
  int i, intg, frac, error, intg1, frac1;
  dec1 x, *buf;
  sanity(to);

  error = E_DEC_BAD_NUM; /* In case of bad number */
  while (s < end_of_string && my_isspace(&my_charset_latin1, *s)) s++;
  if (s == end_of_string) goto fatal_error;

  if ((to->sign = (*s == '-')))
    s++;
  else if (*s == '+')
    s++;

  s1 = s;
  while (s < end_of_string && my_isdigit(&my_charset_latin1, *s)) s++;
  intg = (int)(s - s1);
  if (s < end_of_string && *s == '.') {
    endp = s + 1;
    while (endp < end_of_string && my_isdigit(&my_charset_latin1, *endp))
      endp++;
    frac = (int)(endp - s - 1);
  } else {
    frac = 0;
    endp = s;
  }

  *end = endp;

  if (frac + intg == 0) goto fatal_error;

  error = 0;

  intg1 = ROUND_UP(intg);
  frac1 = ROUND_UP(frac);
  FIX_INTG_FRAC_ERROR(to->len, intg1, frac1, error);
  if (unlikely(error)) {
    frac = frac1 * DIG_PER_DEC1;
    if (error == E_DEC_OVERFLOW) intg = intg1 * DIG_PER_DEC1;
  }

  /* Error is guranteed to be set here */
  to->intg = intg;
  to->frac = frac;

  buf = to->buf + intg1;
  s1 = s;

  for (x = 0, i = 0; intg; intg--) {
    x += (*--s - '0') * powers10[i];

    if (unlikely(++i == DIG_PER_DEC1)) {
      *--buf = x;
      x = 0;
      i = 0;
    }
  }
  if (i) *--buf = x;

  buf = to->buf + intg1;
  for (x = 0, i = 0; frac; frac--) {
    x = (*++s1 - '0') + x * 10;

    if (unlikely(++i == DIG_PER_DEC1)) {
      *buf++ = x;
      x = 0;
      i = 0;
    }
  }
  if (i) *buf = x * powers10[DIG_PER_DEC1 - i];

  /* Handle exponent */
  if (endp + 1 < end_of_string && (*endp == 'e' || *endp == 'E')) {
    int str_error;
    longlong exponent = my_strtoll10(endp + 1, &end_of_string, &str_error);

    if (end_of_string != endp + 1) /* If at least one digit */
    {
      *end = end_of_string;
      if (str_error > 0) {
        error = E_DEC_BAD_NUM;
        goto fatal_error;
      }
      if (exponent > INT_MAX / 2 || (str_error == 0 && exponent < 0)) {
        error = E_DEC_OVERFLOW;
        goto fatal_error;
      }
      if (exponent < INT_MIN / 2 && error != E_DEC_OVERFLOW) {
        error = E_DEC_TRUNCATED;
        goto fatal_error;
      }
      if (error != E_DEC_OVERFLOW) error = decimal_shift(to, (int)exponent);
    }
  }
  /* Avoid returning negative zero, cfr. decimal_cmp() */
  if (to->sign && decimal_is_zero(to)) to->sign = false;
  return error;

fatal_error:
  decimal_make_zero(to);
  return error;
}

/**
  Add zeros behind comma to increase precision of decimal.

  @param         new_frac the new fraction
  @param[in,out] d        the decimal target

  new_frac is exected to be larger or equal than cd->frac and
  new fraction is expected to fit in d.
*/
void widen_fraction(int new_frac, decimal_t *d) {
  const int frac = d->frac;
  const int intg = d->intg;
  const int frac1 = ROUND_UP(frac);
  const int intg1 = ROUND_UP(intg);
  int new_frac1 = ROUND_UP(new_frac);

  if (new_frac < frac || intg1 + new_frac1 > d->len) {
    DBUG_ASSERT(false);
    return;
  }
  decimal_digit_t *buf = d->buf + intg1 + frac1;
  std::fill_n(buf, new_frac1 - frac1, 0);
  d->frac = new_frac;
}
/*
  Convert decimal to double

  SYNOPSIS
    decimal2double()
      from    - value to convert
      to      - result will be stored there

  RETURN VALUE
    E_DEC_OK/E_DEC_OVERFLOW/E_DEC_TRUNCATED
*/

int decimal2double(const decimal_t *from, double *to) {
  char strbuf[FLOATING_POINT_BUFFER];
  int len = sizeof(strbuf);
  int rc, error;

  rc = decimal2string(from, strbuf, &len);
  const char *end = strbuf + len;

  DBUG_PRINT("info", ("interm.: %s", strbuf));

  *to = my_strtod(strbuf, &end, &error);

  DBUG_PRINT("info", ("result: %f", *to));

  return (rc != E_DEC_OK) ? rc : (error ? E_DEC_OVERFLOW : E_DEC_OK);
}

/*
  Convert double to decimal

  SYNOPSIS
    double2decimal()
      from    - value to convert
      to      - result will be stored there

  RETURN VALUE
    E_DEC_OK/E_DEC_OVERFLOW/E_DEC_TRUNCATED
*/

int double2decimal(double from, decimal_t *to) {
  char buff[FLOATING_POINT_BUFFER];
  int res;
  DBUG_TRACE;
  const char *end = buff + my_gcvt(from, MY_GCVT_ARG_DOUBLE,
                                   (int)sizeof(buff) - 1, buff, nullptr);
  res = string2decimal(buff, to, &end);
  DBUG_PRINT("exit", ("res: %d", res));
  return res;
}

static int ull2dec(ulonglong from, decimal_t *to) {
  int intg1;
  int error = E_DEC_OK;
  ulonglong x = from;
  dec1 *buf;

  sanity(to);

  if (from == 0)
    intg1 = 1;
  else {
    /* Count the number of decimal_digit_t's we need. */
    for (intg1 = 0; from != 0; intg1++, from /= DIG_BASE)
      ;
  }
  if (unlikely(intg1 > to->len)) {
    intg1 = to->len;
    error = E_DEC_OVERFLOW;
  }
  to->frac = 0;
  to->intg = intg1 * DIG_PER_DEC1;

  for (buf = to->buf + intg1; intg1; intg1--) {
    ulonglong y = x / DIG_BASE;
    *--buf = (dec1)(x - y * DIG_BASE);
    x = y;
  }
  return error;
}

int ulonglong2decimal(ulonglong from, decimal_t *to) {
  to->sign = false;
  return ull2dec(from, to);
}

int longlong2decimal(longlong from, decimal_t *to) {
  if ((to->sign = from < 0))
    return ull2dec(from == LLONG_MIN ? static_cast<ulonglong>(from) : -from,
                   to);
  return ull2dec(from, to);
}

int decimal2ulonglong(const decimal_t *from, ulonglong *to) {
  dec1 *buf = from->buf;
  ulonglong x = 0;
  int intg, frac;

  if (from->sign) {
    *to = 0ULL;
    return E_DEC_OVERFLOW;
  }

  for (intg = from->intg; intg > 0; intg -= DIG_PER_DEC1) {
    ulonglong y = x;
    x = x * DIG_BASE + *buf++;
    if (unlikely(y > ((ulonglong)ULLONG_MAX / DIG_BASE) || x < y)) {
      *to = ULLONG_MAX;
      return E_DEC_OVERFLOW;
    }
  }
  *to = x;
  for (frac = from->frac; unlikely(frac > 0); frac -= DIG_PER_DEC1)
    if (*buf++) return E_DEC_TRUNCATED;
  return E_DEC_OK;
}

int decimal2longlong(const decimal_t *from, longlong *to) {
  dec1 *buf = from->buf;
  longlong x = 0;
  int intg, frac;

  for (intg = from->intg; intg > 0; intg -= DIG_PER_DEC1) {
    /*
      Attention: trick!
      we're calculating -|from| instead of |from| here
      because |LLONG_MIN| > LLONG_MAX
      so we can convert -9223372036854775808 correctly
    */
    if (unlikely(x < (LLONG_MIN / DIG_BASE))) {
      /*
        the decimal is bigger than any possible integer
        return border integer depending on the sign
      */
      *to = from->sign ? LLONG_MIN : LLONG_MAX;
      return E_DEC_OVERFLOW;
    }
    x = x * DIG_BASE;
    const longlong digit = *buf++;
    if (unlikely(x < LLONG_MIN + digit)) {
      /*
        the decimal is bigger than any possible integer
        return border integer depending on the sign
      */
      *to = from->sign ? LLONG_MIN : LLONG_MAX;
      return E_DEC_OVERFLOW;
    }
    x = x - digit;
  }
  /* boundary case: 9223372036854775808 */
  if (unlikely(from->sign == 0 && x == LLONG_MIN)) {
    *to = LLONG_MAX;
    return E_DEC_OVERFLOW;
  }

  *to = from->sign ? x : -x;
  for (frac = from->frac; unlikely(frac > 0); frac -= DIG_PER_DEC1)
    if (*buf++) return E_DEC_TRUNCATED;
  return E_DEC_OK;
}

#define LLDIV_MIN -1000000000000000000LL
#define LLDIV_MAX 1000000000000000000LL

/**
  Convert decimal value to lldiv_t value.
  @param      from  The decimal value to convert from.
  @param [out]  to    The lldiv_t variable to convert to.
  @return           0 on success, error code on error.
*/
int decimal2lldiv_t(const decimal_t *from, lldiv_t *to) {
  int int_part = ROUND_UP(from->intg);
  int frac_part = ROUND_UP(from->frac);
  if (int_part > 2) {
    to->rem = 0;
    to->quot = from->sign ? LLDIV_MIN : LLDIV_MAX;
    return E_DEC_OVERFLOW;
  }
  if (int_part == 2)
    to->quot = ((longlong)from->buf[0]) * DIG_BASE + from->buf[1];
  else if (int_part == 1)
    to->quot = from->buf[0];
  else
    to->quot = 0;
  to->rem = frac_part ? from->buf[int_part] : 0;
  if (from->sign) {
    to->quot = -to->quot;
    to->rem = -to->rem;
  }
  return 0;
}

/**
  Convert double value to lldiv_t valie.
  @param     nr The double value to convert from.
  @param [out] lld   The lldit_t variable to convert to.
  @return         0 on success, error code on error.

  Integer part goes into lld.quot.
  Fractional part multiplied to 1000000000 (10^9) goes to lld.rem.
  Typically used in datetime calculations to split seconds
  and nanoseconds.
*/
int double2lldiv_t(double nr, lldiv_t *lld) {
  if (nr > LLDIV_MAX) {
    lld->quot = LLDIV_MAX;
    lld->rem = 0;
    return E_DEC_OVERFLOW;
  } else if (nr < LLDIV_MIN) {
    lld->quot = LLDIV_MIN;
    lld->rem = 0;
    return E_DEC_OVERFLOW;
  }
  /* Truncate fractional part toward zero and store into "quot" */
  lld->quot = (longlong)(nr > 0 ? floor(nr) : ceil(nr));
  /* Multiply reminder to 10^9 and store into "rem" */
  lld->rem = (longlong)rint((nr - (double)lld->quot) * 1000000000);
  /*
    Sometimes the expression "(double) 0.999999999xxx * (double) 10e9"
    gives 1,000,000,000 instead of 999,999,999 due to lack of double precision.
    The callers do not expect lld->rem to be greater than 999,999,999.
    Let's catch this corner case and put the "nanounit" (e.g. nanosecond)
    value in ldd->rem back into the valid range.
  */
  if (lld->rem > 999999999LL)
    lld->rem = 999999999LL;
  else if (lld->rem < -999999999LL)
    lld->rem = -999999999LL;
  return E_DEC_OK;
}

/*
  Convert decimal to its binary fixed-length representation
  two representations of the same length can be compared with memcmp
  with the correct -1/0/+1 result

  SYNOPSIS
    decimal2bin()
      from    - value to convert
      to      - points to buffer where string representation should be stored
      precision/scale - see decimal_bin_size() below

  NOTE
    the buffer is assumed to be of the size decimal_bin_size(precision, scale)

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW

  DESCRIPTION
    for storage decimal numbers are converted to the "binary" format.

    This format has the following properties:
      1. length of the binary representation depends on the {precision, scale}
      as provided by the caller and NOT on the intg/frac of the decimal to
      convert.
      2. binary representations of the same {precision, scale} can be compared
      with memcmp - with the same result as decimal_cmp() of the original
      decimals (not taking into account possible precision loss during
      conversion).

    This binary format is as follows:
      1. First the number is converted to have a requested precision and scale.
      2. Every full DIG_PER_DEC1 digits of intg part are stored in 4 bytes
         as is
      3. The first intg % DIG_PER_DEC1 digits are stored in the reduced
         number of bytes (enough bytes to store this number of digits -
         see dig2bytes)
      4. same for frac - full decimal_digit_t's are stored as is,
         the last frac % DIG_PER_DEC1 digits - in the reduced number of bytes.
      5. If the number is negative - every byte is inversed.
      5. The very first bit of the resulting byte array is inverted (because
         memcmp compares unsigned bytes, see property 2 above)

    Example:

      1234567890.1234

    internally is represented as 3 decimal_digit_t's

      1 234567890 123400000

    (assuming we want a binary representation with precision=14, scale=4)
    in hex it's

      00-00-00-01  0D-FB-38-D2  07-5A-EF-40

    now, middle decimal_digit_t is full - it stores 9 decimal digits. It goes
    into binary representation as is:


      ...........  0D-FB-38-D2 ............

    First decimal_digit_t has only one decimal digit. We can store one digit in
    one byte, no need to waste four:

                01 0D-FB-38-D2 ............

    now, last digit. It's 123400000. We can store 1234 in two bytes:

                01 0D-FB-38-D2 04-D2

    So, we've packed 12 bytes number in 7 bytes.
    And now we invert the highest bit to get the final result:

                81 0D FB 38 D2 04 D2

    And for -1234567890.1234 it would be

                7E F2 04 C7 2D FB 2D
*/
int decimal2bin(const decimal_t *from, uchar *to, int precision, int frac) {
  dec1 mask = from->sign ? -1 : 0, *buf1 = from->buf, *stop1;
  int error = E_DEC_OK, intg = precision - frac, isize1, intg1, intg1x,
      from_intg, intg0 = intg / DIG_PER_DEC1, frac0 = frac / DIG_PER_DEC1,
      intg0x = intg - intg0 * DIG_PER_DEC1,
      frac0x = frac - frac0 * DIG_PER_DEC1, frac1 = from->frac / DIG_PER_DEC1,
      frac1x = from->frac - frac1 * DIG_PER_DEC1,
      isize0 = intg0 * sizeof(dec1) + dig2bytes[intg0x],
      fsize0 = frac0 * sizeof(dec1) + dig2bytes[frac0x],
      fsize1 = frac1 * sizeof(dec1) + dig2bytes[frac1x];
  const int orig_isize0 = isize0;
  const int orig_fsize0 = fsize0;
  uchar *orig_to = to;

  buf1 = remove_leading_zeroes(from, &from_intg);

  if (unlikely(from_intg + fsize1 == 0)) {
    mask = 0; /* just in case */
    intg = 1;
    buf1 = &mask;
  }

  intg1 = from_intg / DIG_PER_DEC1;
  intg1x = from_intg - intg1 * DIG_PER_DEC1;
  isize1 = intg1 * sizeof(dec1) + dig2bytes[intg1x];

  if (intg < from_intg) {
    buf1 += intg1 - intg0 + (intg1x > 0) - (intg0x > 0);
    intg1 = intg0;
    intg1x = intg0x;
    error = E_DEC_OVERFLOW;
  } else if (isize0 > isize1) {
    while (isize0-- > isize1) *to++ = (char)mask;
  }
  if (fsize0 < fsize1) {
    frac1 = frac0;
    frac1x = frac0x;
    error = E_DEC_TRUNCATED;
  } else if (fsize0 > fsize1 && frac1x) {
    if (frac0 == frac1) {
      frac1x = frac0x;
      fsize0 = fsize1;
    } else {
      frac1++;
      frac1x = 0;
    }
  }

  /* intg1x part */
  if (intg1x) {
    int i = dig2bytes[intg1x];
    dec1 x = mod_by_pow10(*buf1++, intg1x) ^ mask;
    switch (i) {
      case 1:
        mi_int1store(to, x);
        break;
      case 2:
        mi_int2store(to, x);
        break;
      case 3:
        mi_int3store(to, x);
        break;
      case 4:
        mi_int4store(to, x);
        break;
      default:
        DBUG_ASSERT(0);
    }
    to += i;
  }

  /* intg1+frac1 part */
  for (stop1 = buf1 + intg1 + frac1; buf1 < stop1; to += sizeof(dec1)) {
    dec1 x = *buf1++ ^ mask;
    DBUG_ASSERT(sizeof(dec1) == 4);
    mi_int4store(to, x);
  }

  /* frac1x part */
  if (frac1x) {
    dec1 x;
    int i = dig2bytes[frac1x], lim = (frac1 < frac0 ? DIG_PER_DEC1 : frac0x);
    while (frac1x < lim && dig2bytes[frac1x] == i) frac1x++;
    x = div_by_pow10(*buf1, DIG_PER_DEC1 - frac1x) ^ mask;
    switch (i) {
      case 1:
        mi_int1store(to, x);
        break;
      case 2:
        mi_int2store(to, x);
        break;
      case 3:
        mi_int3store(to, x);
        break;
      case 4:
        mi_int4store(to, x);
        break;
      default:
        DBUG_ASSERT(0);
    }
    to += i;
  }
  if (fsize0 > fsize1) {
    uchar *to_end = orig_to + orig_fsize0 + orig_isize0;

    while (fsize0-- > fsize1 && to < to_end) *to++ = (uchar)mask;
  }
  orig_to[0] ^= 0x80;

  /* Check that we have written the whole decimal and nothing more */
  DBUG_ASSERT(to == orig_to + orig_fsize0 + orig_isize0);
  return error;
}

/*
  Restores decimal from its binary fixed-length representation

  SYNOPSIS
    bin2decimal()
      from    - value to convert
      to      - result
      precision/scale - see decimal_bin_size() below
      keep_prec do not trim leading zeros

  NOTE
    see decimal2bin()
    the buffer is assumed to be of the size decimal_bin_size(precision, scale)
    If the keep_prec is true, the value will be read and returned as is,
    without precision reduction. This is used to read DECIMAL values that
    are to be indexed by multi-valued index.

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW
*/

int bin2decimal(const uchar *from, decimal_t *to, int precision, int scale,
                bool keep_prec) {
  int error = E_DEC_OK, intg = precision - scale, intg0 = intg / DIG_PER_DEC1,
      frac0 = scale / DIG_PER_DEC1, intg0x = intg - intg0 * DIG_PER_DEC1,
      frac0x = scale - frac0 * DIG_PER_DEC1, intg1 = intg0 + (intg0x > 0),
      frac1 = frac0 + (frac0x > 0);
  dec1 *buf = to->buf, mask = (*from & 0x80) ? 0 : -1;
  const uchar *stop;
  uchar *d_copy;
  int bin_size = decimal_bin_size_inline(precision, scale);

  sanity(to);
  d_copy = (uchar *)my_alloca(bin_size);
  memcpy(d_copy, from, bin_size);
  d_copy[0] ^= 0x80;
  from = d_copy;

  FIX_INTG_FRAC_ERROR(to->len, intg1, frac1, error);
  if (unlikely(error)) {
    if (intg1 < intg0 + (intg0x > 0)) {
      from += dig2bytes[intg0x] + sizeof(dec1) * (intg0 - intg1);
      frac0 = frac0x = intg0x = 0;
      intg0 = intg1;
    } else {
      frac0x = 0;
      frac0 = frac1;
    }
  }

  to->sign = (mask != 0);
  to->intg = intg0 * DIG_PER_DEC1 + intg0x;
  to->frac = frac0 * DIG_PER_DEC1 + frac0x;

  if (intg0x) {
    int i = dig2bytes[intg0x];
    dec1 x = 0;
    switch (i) {
      case 1:
        x = mi_sint1korr(from);
        break;
      case 2:
        x = mi_sint2korr(from);
        break;
      case 3:
        x = mi_sint3korr(from);
        break;
      case 4:
        x = mi_sint4korr(from);
        break;
      default:
        DBUG_ASSERT(0);
    }
    from += i;
    *buf = x ^ mask;
    if (((ulonglong)*buf) >= (ulonglong)powers10[intg0x + 1]) goto err;
    if (buf > to->buf || *buf != 0 || keep_prec)
      buf++;
    else
      to->intg -= intg0x;
  }
  for (stop = from + intg0 * sizeof(dec1); from < stop; from += sizeof(dec1)) {
    DBUG_ASSERT(sizeof(dec1) == 4);
    *buf = mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX) goto err;
    if (buf > to->buf || *buf != 0 || keep_prec)
      buf++;
    else
      to->intg -= DIG_PER_DEC1;
  }
  DBUG_ASSERT(to->intg >= 0);
  for (stop = from + frac0 * sizeof(dec1); from < stop; from += sizeof(dec1)) {
    DBUG_ASSERT(sizeof(dec1) == 4);
    *buf = mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX) goto err;
    buf++;
  }
  if (frac0x) {
    int i = dig2bytes[frac0x];
    dec1 x = 0;
    switch (i) {
      case 1:
        x = mi_sint1korr(from);
        break;
      case 2:
        x = mi_sint2korr(from);
        break;
      case 3:
        x = mi_sint3korr(from);
        break;
      case 4:
        x = mi_sint4korr(from);
        break;
      default:
        DBUG_ASSERT(0);
    }
    *buf = (x ^ mask) * powers10[DIG_PER_DEC1 - frac0x];
    if (((uint32)*buf) > DIG_MAX) goto err;
    buf++;
  }

  /*
    No digits? We have read the number zero, of unspecified precision.
    Make it a proper zero, with non-zero precision.
    Note: this is valid only if scale == 0, otherwise frac is always non-zero
  */
  if (to->intg == 0 && to->frac == 0) decimal_make_zero(to);
  return error;

err:
  decimal_make_zero(to);
  return (E_DEC_BAD_NUM);
}

/*
  Returns the size of array to hold a decimal with given precision and scale

  RETURN VALUE
    size in dec1
    (multiply by sizeof(dec1) to get the size if bytes)
*/

int decimal_size(int precision, int scale) {
  DBUG_ASSERT(scale >= 0 && precision > 0 && scale <= precision);
  return ROUND_UP(precision - scale) + ROUND_UP(scale);
}

/*
  Returns the size of array to hold a binary representation of a decimal

  RETURN VALUE
    size in bytes
*/
ALWAYS_INLINE static int decimal_bin_size_inline(int precision, int scale) {
  int intg = precision - scale, intg0 = intg / DIG_PER_DEC1,
      frac0 = scale / DIG_PER_DEC1, intg0x = intg - intg0 * DIG_PER_DEC1,
      frac0x = scale - frac0 * DIG_PER_DEC1;

  DBUG_ASSERT(scale >= 0 && precision > 0 && scale <= precision);
  DBUG_ASSERT(intg0x >= 0);
  DBUG_ASSERT(intg0x <= DIG_PER_DEC1);
  DBUG_ASSERT(frac0x >= 0);
  DBUG_ASSERT(frac0x <= DIG_PER_DEC1);
  return intg0 * sizeof(dec1) + dig2bytes[intg0x] + frac0 * sizeof(dec1) +
         dig2bytes[frac0x];
}

int decimal_bin_size(int precision, int scale) {
  return decimal_bin_size_inline(precision, scale);
}

/*
  Rounds the decimal to "scale" digits

  SYNOPSIS
    decimal_round()
      from    - decimal to round,
      to      - result buffer. from==to is allowed
      scale   - to what position to round. can be negative!
      mode    - round to nearest even or truncate

  NOTES
    scale can be negative !
    one TRUNCATED error (line XXX below) isn't treated very logical :(

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED
*/

int decimal_round(const decimal_t *from, decimal_t *to, int scale,
                  decimal_round_mode mode) {
  int frac0 = scale > 0 ? ROUND_UP(scale) : (scale + 1) / DIG_PER_DEC1,
      frac1 = ROUND_UP(from->frac), round_digit = 0,
      intg0 = ROUND_UP(from->intg), error = E_DEC_OK, len = to->len;

  dec1 *buf0 = from->buf, *buf1 = to->buf, x, y, carry = 0;
  int first_dig;

  sanity(to);

  switch (mode) {
    case HALF_UP:
    case HALF_EVEN:
      round_digit = 5;
      break;
    case CEILING:
      round_digit = from->sign ? 10 : 0;
      break;
    case FLOOR:
      round_digit = from->sign ? 0 : 10;
      break;
    case TRUNCATE:
      round_digit = 10;
      break;
    default:
      DBUG_ASSERT(0);
  }

  /*
    For my_decimal we always use len == DECIMAL_BUFF_LENGTH == 9
    For internal testing here (ifdef MAIN) we always use len == 100/4
   */
  DBUG_ASSERT(from->len == to->len);

  if (unlikely(frac0 + intg0 > len)) {
    frac0 = len - intg0;
    scale = frac0 * DIG_PER_DEC1;
    error = E_DEC_TRUNCATED;
  }

  if (scale + from->intg < 0) {
    decimal_make_zero(to);
    return E_DEC_OK;
  }

  if (to != from) {
    dec1 *p0 = buf0 + intg0 + std::max(frac1, frac0);
    dec1 *p1 = buf1 + intg0 + std::max(frac1, frac0);

    DBUG_ASSERT(p0 - buf0 <= len);
    DBUG_ASSERT(p1 - buf1 <= len);

    while (buf0 < p0) *(--p1) = *(--p0);

    buf0 = to->buf;
    buf1 = to->buf;
    to->sign = from->sign;
    to->intg = std::min(intg0, len) * DIG_PER_DEC1;
  }

  if (frac0 > frac1) {
    buf1 += intg0 + frac1;
    while (frac0-- > frac1) *buf1++ = 0;
    goto done;
  }

  if (scale >= from->frac) goto done; /* nothing to do */

  buf0 += intg0 + frac0 - 1;
  buf1 += intg0 + frac0 - 1;
  if (scale == frac0 * DIG_PER_DEC1) {
    int do_inc = false;
    DBUG_ASSERT(frac0 + intg0 >= 0);
    switch (round_digit) {
      case 0: {
        dec1 *p0 = buf0 + (frac1 - frac0);
        for (; p0 > buf0; p0--) {
          if (*p0) {
            do_inc = true;
            break;
          }
        }
        break;
      }
      case 5: {
        x = buf0[1] / DIG_MASK;
        do_inc =
            (x > 5) ||
            ((x == 5) && (mode == HALF_UP || (frac0 + intg0 > 0 && *buf0 & 1)));
        break;
      }
      default:
        break;
    }
    if (do_inc) {
      if (frac0 + intg0 > 0)
        (*buf1)++;
      else
        *(++buf1) = DIG_BASE;
    } else if (frac0 + intg0 == 0) {
      decimal_make_zero(to);
      return E_DEC_OK;
    }
  } else {
    /* TODO - fix this code as it won't work for CEILING mode */
    int pos = frac0 * DIG_PER_DEC1 - scale - 1;
    DBUG_ASSERT(frac0 + intg0 > 0);
    x = *buf1 / powers10[pos];
    y = x % 10;
    if (y > round_digit ||
        (round_digit == 5 && y == 5 && (mode == HALF_UP || (x / 10) & 1)))
      x += 10;
    *buf1 = powers10[pos] * (x - y);
  }
  /*
    In case we're rounding e.g. 1.5e9 to 2.0e9, the decimal_digit_t's inside
    the buffer are as follows.

    Before <1, 5e8>
    After  <2, 5e8>

    Hence we need to set the 2nd field to 0.
    The same holds if we round 1.5e-9 to 2e-9.
   */
  if (frac0 < frac1) {
    dec1 *buf = to->buf + ((scale == 0 && intg0 == 0) ? 1 : intg0 + frac0);
    dec1 *end = to->buf + len;

    while (buf < end) *buf++ = 0;
  }
  if (*buf1 >= DIG_BASE) {
    carry = 1;
    *buf1 -= DIG_BASE;
    while (carry && --buf1 >= to->buf) ADD(*buf1, *buf1, 0, carry);
    if (unlikely(carry)) {
      /* shifting the number to create space for new digit */
      if (frac0 + intg0 >= len) {
        frac0--;
        scale = frac0 * DIG_PER_DEC1;
        error = E_DEC_TRUNCATED; /* XXX */
      }
      for (buf1 = to->buf + intg0 + std::max(frac0, 0); buf1 > to->buf;
           buf1--) {
        /* Avoid out-of-bounds write. */
        if (buf1 < to->buf + len)
          buf1[0] = buf1[-1];
        else
          error = E_DEC_OVERFLOW;
      }
      *buf1 = 1;
      /* We cannot have more than 9 * 9 = 81 digits. */
      if (to->intg < len * DIG_PER_DEC1)
        to->intg++;
      else
        error = E_DEC_OVERFLOW;
    }
  } else {
    for (;;) {
      if (likely(*buf1)) break;
      if (buf1-- == to->buf) {
        /* making 'zero' with the proper scale */
        dec1 *p0 = to->buf + frac0 + 1;
        to->intg = 1;
        to->frac = std::max(scale, 0);
        to->sign = false;
        for (buf1 = to->buf; buf1 < p0; buf1++) *buf1 = 0;
        return E_DEC_OK;
      }
    }
  }

  /* Here we  check 999.9 -> 1000 case when we need to increase intg */
  first_dig = to->intg % DIG_PER_DEC1;
  if (first_dig && (*buf1 >= powers10[first_dig])) to->intg++;

  if (scale < 0) scale = 0;

done:
  DBUG_ASSERT(to->intg <= (len * DIG_PER_DEC1));
  to->frac = scale;
  return error;
}

static int do_add(const decimal_t *from1, const decimal_t *from2,
                  decimal_t *to) {
  int intg1 = ROUND_UP(from1->intg), intg2 = ROUND_UP(from2->intg),
      frac1 = ROUND_UP(from1->frac), frac2 = ROUND_UP(from2->frac),
      frac0 = std::max(frac1, frac2), intg0 = std::max(intg1, intg2), error;
  dec1 *buf1, *buf2, *buf0, *stop, *stop2, x, carry;

  sanity(to);

  /* is there a need for extra word because of carry ? */
  x = intg1 > intg2
          ? from1->buf[0]
          : intg2 > intg1 ? from2->buf[0] : from1->buf[0] + from2->buf[0];
  if (unlikely(x > DIG_MAX - 1)) /* yes, there is */
  {
    intg0++;
    to->buf[0] = 0; /* safety */
  }

  FIX_INTG_FRAC_ERROR(to->len, intg0, frac0, error);
  if (unlikely(error == E_DEC_OVERFLOW)) {
    max_decimal(to->len * DIG_PER_DEC1, 0, to);
    return error;
  }

  buf0 = to->buf + intg0 + frac0;

  to->sign = from1->sign;
  to->frac = std::max(from1->frac, from2->frac);
  to->intg = intg0 * DIG_PER_DEC1;
  if (unlikely(error)) {
    to->frac = std::min(to->frac, frac0 * DIG_PER_DEC1);
    frac1 = std::min(frac1, frac0);
    frac2 = std::min(frac2, frac0);
    intg1 = std::min(intg1, intg0);
    intg2 = std::min(intg2, intg0);
  }

  /* part 1 - max(frac) ... min (frac) */
  if (frac1 > frac2) {
    buf1 = from1->buf + intg1 + frac1;
    stop = from1->buf + intg1 + frac2;
    buf2 = from2->buf + intg2 + frac2;
    stop2 = from1->buf + (intg1 > intg2 ? intg1 - intg2 : 0);
  } else {
    buf1 = from2->buf + intg2 + frac2;
    stop = from2->buf + intg2 + frac1;
    buf2 = from1->buf + intg1 + frac1;
    stop2 = from2->buf + (intg2 > intg1 ? intg2 - intg1 : 0);
  }
  while (buf1 > stop) *--buf0 = *--buf1;

  /* part 2 - min(frac) ... min(intg) */
  carry = 0;
  while (buf1 > stop2) {
    ADD(*--buf0, *--buf1, *--buf2, carry);
  }

  /* part 3 - min(intg) ... max(intg) */
  buf1 = intg1 > intg2 ? ((stop = from1->buf) + intg1 - intg2)
                       : ((stop = from2->buf) + intg2 - intg1);
  while (buf1 > stop) {
    ADD(*--buf0, *--buf1, 0, carry);
  }

  if (unlikely(carry)) *--buf0 = 1;
  DBUG_ASSERT(buf0 == to->buf || buf0 == to->buf + 1);

  return error;
}

/* to=from1-from2.
   if to==0, return -1/0/+1 - the result of the comparison */
static int do_sub(const decimal_t *from1, const decimal_t *from2,
                  decimal_t *to) {
  int intg1 = ROUND_UP(from1->intg), intg2 = ROUND_UP(from2->intg),
      frac1 = ROUND_UP(from1->frac), frac2 = ROUND_UP(from2->frac);
  int frac0 = std::max(frac1, frac2), error;
  dec1 *buf1, *buf2, *buf0, *stop1, *stop2, *start1, *start2, carry = 0;

  /* let carry:=1 if from2 > from1 */
  start1 = buf1 = from1->buf;
  stop1 = buf1 + intg1;
  start2 = buf2 = from2->buf;
  stop2 = buf2 + intg2;
  if (unlikely(*buf1 == 0)) {
    while (buf1 < stop1 && *buf1 == 0) buf1++;
    start1 = buf1;
    intg1 = (int)(stop1 - buf1);
  }
  if (unlikely(*buf2 == 0)) {
    while (buf2 < stop2 && *buf2 == 0) buf2++;
    start2 = buf2;
    intg2 = (int)(stop2 - buf2);
  }
  if (intg2 > intg1)
    carry = 1;
  else if (intg2 == intg1) {
    dec1 *end1 = stop1 + (frac1 - 1);
    dec1 *end2 = stop2 + (frac2 - 1);
    while (unlikely((buf1 <= end1) && (*end1 == 0))) end1--;
    while (unlikely((buf2 <= end2) && (*end2 == 0))) end2--;
    frac1 = (int)(end1 - stop1) + 1;
    frac2 = (int)(end2 - stop2) + 1;
    while (buf1 <= end1 && buf2 <= end2 && *buf1 == *buf2) buf1++, buf2++;
    if (buf1 <= end1) {
      if (buf2 <= end2)
        carry = *buf2 > *buf1;
      else
        carry = 0;
    } else {
      if (buf2 <= end2)
        carry = 1;
      else /* short-circuit everything: from1 == from2 */
      {
        if (to == nullptr) /* decimal_cmp() */
          return 0;
        decimal_make_zero(to);
        return E_DEC_OK;
      }
    }
  }

  if (to == nullptr) /* decimal_cmp() */
    return carry == from1->sign ? 1 : -1;

  sanity(to);

  to->sign = from1->sign;

  /* ensure that always from1 > from2 (and intg1 >= intg2) */
  if (carry) {
    std::swap(from1, from2);
    std::swap(start1, start2);
    std::swap(intg1, intg2);
    std::swap(frac1, frac2);
    to->sign = 1 - to->sign;
  }

  FIX_INTG_FRAC_ERROR(to->len, intg1, frac0, error);
  buf0 = to->buf + intg1 + frac0;

  to->frac = std::max(from1->frac, from2->frac);
  to->intg = intg1 * DIG_PER_DEC1;
  if (unlikely(error)) {
    to->frac = std::min(to->frac, frac0 * DIG_PER_DEC1);
    frac1 = std::min(frac1, frac0);
    frac2 = std::min(frac2, frac0);
    intg2 = std::min(intg2, intg1);
  }
  carry = 0;

  /* part 1 - max(frac) ... min (frac) */
  if (frac1 > frac2) {
    buf1 = start1 + intg1 + frac1;
    stop1 = start1 + intg1 + frac2;
    buf2 = start2 + intg2 + frac2;
    while (frac0-- > frac1) *--buf0 = 0;
    while (buf1 > stop1) *--buf0 = *--buf1;
  } else {
    buf1 = start1 + intg1 + frac1;
    buf2 = start2 + intg2 + frac2;
    stop2 = start2 + intg2 + frac1;
    while (frac0-- > frac2) *--buf0 = 0;
    while (buf2 > stop2) {
      SUB(*--buf0, 0, *--buf2, carry);
    }
  }

  /* part 2 - min(frac) ... intg2 */
  while (buf2 > start2) {
    SUB(*--buf0, *--buf1, *--buf2, carry);
  }

  /* part 3 - intg2 ... intg1 */
  while (carry && buf1 > start1) {
    SUB(*--buf0, *--buf1, 0, carry);
  }

  while (buf1 > start1) *--buf0 = *--buf1;

  while (buf0 > to->buf) *--buf0 = 0;

  return error;
}

int decimal_intg(const decimal_t *from) {
  int res;
  remove_leading_zeroes(from, &res);
  return res;
}

int decimal_add(const decimal_t *from1, const decimal_t *from2, decimal_t *to) {
  if (likely(from1->sign == from2->sign)) return do_add(from1, from2, to);
  return do_sub(from1, from2, to);
}

int decimal_sub(const decimal_t *from1, const decimal_t *from2, decimal_t *to) {
  if (likely(from1->sign == from2->sign)) return do_sub(from1, from2, to);
  return do_add(from1, from2, to);
}

int decimal_cmp(const decimal_t *from1, const decimal_t *from2) {
  if (likely(from1->sign == from2->sign)) return do_sub(from1, from2, nullptr);

  // Reject negative zero, cfr. string2decimal()
  DBUG_ASSERT(!(decimal_is_zero(from1) && from1->sign));
  DBUG_ASSERT(!(decimal_is_zero(from2) && from2->sign));

  return from1->sign > from2->sign ? -1 : 1;
}

int decimal_is_zero(const decimal_t *from) {
  dec1 *buf1 = from->buf,
       *end = buf1 + ROUND_UP(from->intg) + ROUND_UP(from->frac);
  while (buf1 < end)
    if (*buf1++) return 0;
  return 1;
}

/*
  multiply two decimals

  SYNOPSIS
    decimal_mul()
      from_1, from_2 - factors
      to      - product

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW;

  NOTES
    in this implementation, with sizeof(dec1)=4 we have DIG_PER_DEC1=9,
    and 63-digit number will take only 7 dec1 words (basically a 7-digit
    "base 999999999" number).  Thus there's no need in fast multiplication
    algorithms, 7-digit numbers can be multiplied with a naive O(n*n)
    method.

    XXX if this library is to be used with huge numbers of thousands of
    digits, fast multiplication must be implemented.
*/
int decimal_mul(const decimal_t *from_1, const decimal_t *from_2,
                decimal_t *to) {
  if (decimal_is_zero(from_1) || decimal_is_zero(from_2)) {
    decimal_make_zero(to);
    return E_DEC_OK;
  }
  decimal_t f1 = *from_1;
  decimal_t f2 = *from_2;
  f1.buf = remove_leading_zeroes(&f1, &f1.intg);
  f2.buf = remove_leading_zeroes(&f2, &f2.intg);

  const decimal_t *from1 = &f1;
  const decimal_t *from2 = &f2;
  int intg1 = ROUND_UP(from1->intg), intg2 = ROUND_UP(from2->intg),
      frac1 = ROUND_UP(from1->frac), frac2 = ROUND_UP(from2->frac),
      intg0 = ROUND_UP(from1->intg + from2->intg), frac0 = frac1 + frac2, error,
      iii, jjj, d_to_move;
  dec1 *buf1 = from1->buf + intg1, *buf2 = from2->buf + intg2, *buf0, *start2,
       *stop2, *stop1, *start0, carry;

  sanity(to);

  iii = intg0; /* save 'ideal' values */
  jjj = frac0;
  FIX_INTG_FRAC_ERROR(to->len, intg0, frac0, error); /* bound size */
  to->sign = from1->sign != from2->sign;
  to->frac = from1->frac + from2->frac; /* store size in digits */
  to->frac = std::min(to->frac, DECIMAL_NOT_SPECIFIED);
  to->intg = intg0 * DIG_PER_DEC1;

  if (unlikely(error)) {
    to->frac = std::min(to->frac, frac0 * DIG_PER_DEC1);
    to->intg = std::min(to->intg, intg0 * DIG_PER_DEC1);
    if (unlikely(iii > intg0)) /* bounded integer-part */
    {
      iii -= intg0;
      jjj = iii >> 1;
      intg1 -= jjj;
      intg2 -= iii - jjj;
      frac1 = frac2 = 0; /* frac0 is already 0 here */
    } else               /* bounded fract part */
    {
      jjj -= frac0;
      iii = jjj >> 1;
      if (frac1 <= frac2) {
        frac1 -= iii;
        frac2 -= jjj - iii;
      } else {
        frac2 -= iii;
        frac1 -= jjj - iii;
      }
    }
  }
  start0 = to->buf + intg0 + frac0 - 1;
  start2 = buf2 + frac2 - 1;
  stop1 = buf1 - intg1;
  stop2 = buf2 - intg2;

  memset(to->buf, 0, (intg0 + frac0) * sizeof(dec1));

  for (buf1 += frac1 - 1; buf1 >= stop1; buf1--, start0--) {
    carry = 0;
    for (buf0 = start0, buf2 = start2; buf2 >= stop2; buf2--, buf0--) {
      dec1 hi, lo;
      dec2 p = ((dec2)*buf1) * ((dec2)*buf2);
      hi = (dec1)(p / DIG_BASE);
      lo = (dec1)(p - ((dec2)hi) * DIG_BASE);
      ADD2(*buf0, *buf0, lo, carry);
      carry += hi;
    }
    if (carry) {
      if (buf0 < to->buf) return E_DEC_OVERFLOW;
      ADD2(*buf0, *buf0, 0, carry);
    }
    for (buf0--; carry; buf0--) {
      if (buf0 < to->buf) return E_DEC_OVERFLOW;
      ADD(*buf0, *buf0, 0, carry);
    }
  }

  /* Now we have to check for -0.000 case */
  if (to->sign) {
    dec1 *buf = to->buf;
    dec1 *end = to->buf + intg0 + frac0;
    DBUG_ASSERT(buf != end);
    for (;;) {
      if (*buf) break;
      if (++buf == end) {
        /* We got decimal zero */
        decimal_make_zero(to);
        break;
      }
    }
  }
  buf1 = to->buf;
  d_to_move = intg0 + ROUND_UP(to->frac);
  while (!*buf1 && (to->intg > DIG_PER_DEC1)) {
    buf1++;
    to->intg -= DIG_PER_DEC1;
    d_to_move--;
  }
  if (to->buf < buf1) {
    dec1 *cur_d = to->buf;
    for (; d_to_move--; cur_d++, buf1++) *cur_d = *buf1;
  }
  return error;
}

/*
  naive division algorithm (Knuth's Algorithm D in 4.3.1) -
  it's ok for short numbers
  also we're using alloca() to allocate a temporary buffer

  XXX if this library is to be used with huge numbers of thousands of
  digits, fast division must be implemented and alloca should be
  changed to malloc (or at least fallback to malloc if alloca() fails)
  but then, decimal_mul() should be rewritten too :(
*/
static int do_div_mod(const decimal_t *from1, const decimal_t *from2,
                      decimal_t *to, decimal_t *mod, int scale_incr) {
  /*
    frac* - number of digits in fractional part of the number
    prec* - precision of the number
    intg* - number of digits in the integer part
    buf* - buffer having the actual number
    All variables ending with 0 - like frac0, intg0 etc are
    for the final result. Similarly frac1, intg1 etc are for
    the first number and frac2, intg2 etc are for the second number
   */
  int frac1 = ROUND_UP(from1->frac) * DIG_PER_DEC1, prec1 = from1->intg + frac1,
      frac2 = ROUND_UP(from2->frac) * DIG_PER_DEC1, prec2 = from2->intg + frac2,
      error = 0, i, intg0, frac0, len1, len2,
      dintg, /* Holds the estimate of number of integer digits in final result
              */
      div_mod = (!mod) /*true if this is division */;
  dec1 *buf0, *buf1 = from1->buf, *buf2 = from2->buf, *start1, *stop1, *start2,
              *stop2, *stop0, norm2, carry, dcarry, *tmp1;
  dec2 norm_factor, x, guess, y;

  if (mod) to = mod;

  sanity(to);

  /*
    removing all the leading zeroes in the second number. Leading zeroes are
    added later to the result.
   */
  i = ((prec2 - 1) % DIG_PER_DEC1) + 1;
  while (prec2 > 0 && *buf2 == 0) {
    prec2 -= i;
    i = DIG_PER_DEC1;
    buf2++;
  }
  if (prec2 <= 0) /* short-circuit everything: from2 == 0 */
    return E_DEC_DIV_ZERO;

  /*
    Remove the remanining zeroes . For ex: for 0.000000000001
    the above while loop removes 9 zeroes and the result will have 0.0001
    these remaining zeroes are removed here
   */
  prec2 -= count_leading_zeroes((prec2 - 1) % DIG_PER_DEC1, *buf2);
  DBUG_ASSERT(prec2 > 0);

  /*
   Do the same for the first number. Remove the leading zeroes.
   Check if the number is actually 0. Then remove the remaining zeroes.
   */

  i = ((prec1 - 1) % DIG_PER_DEC1) + 1;
  while (prec1 > 0 && *buf1 == 0) {
    prec1 -= i;
    i = DIG_PER_DEC1;
    buf1++;
  }
  if (prec1 <= 0) { /* short-circuit everything: from1 == 0 */
    decimal_make_zero(to);
    return E_DEC_OK;
  }
  prec1 -= count_leading_zeroes((prec1 - 1) % DIG_PER_DEC1, *buf1);
  DBUG_ASSERT(prec1 > 0);

  /* let's fix scale_incr, taking into account frac1,frac2 increase */
  if ((scale_incr -= frac1 - from1->frac + frac2 - from2->frac) < 0)
    scale_incr = 0;

  /* Calculate the integer digits in final result */
  dintg = (prec1 - frac1) - (prec2 - frac2) + (*buf1 >= *buf2);
  if (dintg < 0) {
    dintg /= DIG_PER_DEC1;
    intg0 = 0;
  } else
    intg0 = ROUND_UP(dintg);
  if (mod) {
    /* we're calculating N1 % N2.
       The result will have
         frac=max(frac1, frac2), as for subtraction
         intg=intg2
    */
    to->sign = from1->sign;
    to->frac = std::max(from1->frac, from2->frac);
    frac0 = 0;
  } else {
    /*
      we're calculating N1/N2. N1 is in the buf1, has prec1 digits
      N2 is in the buf2, has prec2 digits. Scales are frac1 and
      frac2 accordingly.
      Thus, the result will have
         frac = ROUND_UP(frac1+frac2+scale_incr)
      and
         intg = (prec1-frac1) - (prec2-frac2) + 1
         prec = intg+frac
    */
    frac0 = ROUND_UP(frac1 + frac2 + scale_incr);
    FIX_INTG_FRAC_ERROR(to->len, intg0, frac0, error);
    to->sign = from1->sign != from2->sign;
    to->intg = intg0 * DIG_PER_DEC1;
    to->frac = frac0 * DIG_PER_DEC1;
  }
  buf0 = to->buf;
  stop0 = buf0 + intg0 + frac0;
  if (likely(div_mod))
    while (dintg++ < 0 && buf0 < &to->buf[to->len]) {
      *buf0++ = 0;
    }

  len1 = (i = ROUND_UP(prec1)) + ROUND_UP(2 * frac2 + scale_incr + 1) + 1;
  len1 = std::max(len1, 3);
  if (!(tmp1 = (dec1 *)my_alloca(len1 * sizeof(dec1)))) return E_DEC_OOM;
  memcpy(tmp1, buf1, i * sizeof(dec1));
  memset(tmp1 + i, 0, (len1 - i) * sizeof(dec1));

  start1 = tmp1;
  stop1 = start1 + len1;
  start2 = buf2;
  stop2 = buf2 + ROUND_UP(prec2) - 1;

  /* removing end zeroes */
  while (*stop2 == 0 && stop2 >= start2) stop2--;
  len2 = (int)(stop2++ - start2);

  /*
    calculating norm2 (normalized *start2) - we need *start2 to be large
    (at least > DIG_BASE/2), but unlike Knuth's Alg. D we don't want to
    normalize input numbers (as we don't make a copy of the divisor).
    Thus we normalize first dec1 of buf2 only, and we'll normalize *start1
    on the fly for the purpose of guesstimation only.
    It's also faster, as we're saving on normalization of buf2
  */
  norm_factor = DIG_BASE / (*start2 + 1);
  norm2 = (dec1)(norm_factor * start2[0]);
  if (likely(len2 > 0)) norm2 += (dec1)(norm_factor * start2[1] / DIG_BASE);

  if (*start1 < *start2)
    dcarry = *start1++;
  else
    dcarry = 0;

  /* main loop */
  for (; buf0 < stop0; buf0++) {
    /* short-circuit, if possible */
    if (unlikely(dcarry == 0 && *start1 < *start2))
      guess = 0;
    else {
      /* D3: make a guess */
      x = start1[0] + ((dec2)dcarry) * DIG_BASE;
      y = start1[1];
      guess = (norm_factor * x + norm_factor * y / DIG_BASE) / norm2;
      if (unlikely(guess >= DIG_BASE)) guess = DIG_BASE - 1;
      if (likely(len2 > 0)) {
        /* hmm, this is a suspicious trick - I removed normalization here */
        if (start2[1] * guess > (x - guess * start2[0]) * DIG_BASE + y) guess--;
        if (unlikely(start2[1] * guess >
                     (x - guess * start2[0]) * DIG_BASE + y))
          guess--;
        DBUG_ASSERT(start2[1] * guess <=
                    (x - guess * start2[0]) * DIG_BASE + y);
      }

      /* D4: multiply and subtract */
      buf2 = stop2;
      buf1 = start1 + len2;
      DBUG_ASSERT(buf1 < stop1);
      for (carry = 0; buf2 > start2; buf1--) {
        dec1 hi, lo;
        x = guess * (*--buf2);
        hi = (dec1)(x / DIG_BASE);
        lo = (dec1)(x - ((dec2)hi) * DIG_BASE);
        SUB2(*buf1, *buf1, lo, carry);
        carry += hi;
      }
      carry = dcarry < carry;

      /* D5: check the remainder */
      if (unlikely(carry)) {
        /* D6: correct the guess */
        guess--;
        buf2 = stop2;
        buf1 = start1 + len2;
        for (carry = 0; buf2 > start2; buf1--) {
          ADD(*buf1, *buf1, *--buf2, carry);
        }
      }
    }
    if (likely(div_mod)) {
      DBUG_ASSERT(buf0 < to->buf + to->len);
      *buf0 = (dec1)guess;
    }
    dcarry = *start1;
    start1++;
  }
  if (mod) {
    /*
      now the result is in tmp1, it has
      intg=prec1-frac1  if there were no leading zeroes.
                        If leading zeroes were present, they have been removed
                        earlier. We need to now add them back to the result.
      frac=max(frac1, frac2)=to->frac
     */
    if (dcarry) *--start1 = dcarry;
    buf0 = to->buf;
    /* Calculate the final result's integer digits */
    dintg = (prec1 - frac1) - ((start1 - tmp1) * DIG_PER_DEC1);
    if (dintg < 0) {
      /* If leading zeroes in the fractional part were earlier stripped */
      intg0 = dintg / DIG_PER_DEC1;
    } else
      intg0 = ROUND_UP(dintg);
    frac0 = ROUND_UP(to->frac);
    error = E_DEC_OK;
    if (unlikely(frac0 == 0 && intg0 == 0)) {
      decimal_make_zero(to);
      goto done;
    }
    if (intg0 <= 0) {
      /* Add back the leading zeroes that were earlier stripped */
      if (unlikely(-intg0 >= to->len)) {
        decimal_make_zero(to);
        error = E_DEC_TRUNCATED;
        goto done;
      }
      stop1 = start1 + frac0 + intg0;
      frac0 += intg0;
      to->intg = 0;
      while (intg0++ < 0) *buf0++ = 0;
    } else {
      if (unlikely(intg0 > to->len)) {
        frac0 = 0;
        intg0 = to->len;
        error = E_DEC_OVERFLOW;
        goto done;
      }
      DBUG_ASSERT(intg0 <= ROUND_UP(from2->intg));
      stop1 = start1 + frac0 + intg0;
      to->intg = std::min(intg0 * DIG_PER_DEC1, from2->intg);
    }
    if (unlikely(intg0 + frac0 > to->len)) {
      stop1 -= frac0 + intg0 - to->len;
      frac0 = to->len - intg0;
      to->frac = frac0 * DIG_PER_DEC1;
      error = E_DEC_TRUNCATED;
    }
    DBUG_ASSERT(buf0 + (stop1 - start1) <= to->buf + to->len);
    while (start1 < stop1) *buf0++ = *start1++;
  }
done:
  if (decimal_is_zero(to)) {
    // Return "0." rather than "0.000000"
    decimal_make_zero(to);
  } else {
    tmp1 = remove_leading_zeroes(to, &to->intg);
    if (to->buf != tmp1)
      memmove(to->buf, tmp1,
              (ROUND_UP(to->intg) + ROUND_UP(to->frac)) * sizeof(dec1));
  }
  DBUG_ASSERT(to->intg + to->frac > 0);
  return error;
}

/*
  division of two decimals

  SYNOPSIS
    decimal_div()
      from1   - dividend
      from2   - divisor
      to      - quotient

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW/E_DEC_DIV_ZERO;

  NOTES
    see do_div_mod()
*/

int decimal_div(const decimal_t *from1, const decimal_t *from2, decimal_t *to,
                int scale_incr) {
  return do_div_mod(from1, from2, to, nullptr, scale_incr);
}

/*
  modulus

  SYNOPSIS
    decimal_mod()
      from1   - dividend
      from2   - divisor
      to      - modulus

  RETURN VALUE
    E_DEC_OK/E_DEC_TRUNCATED/E_DEC_OVERFLOW/E_DEC_DIV_ZERO;

  NOTES
    see do_div_mod()

  DESCRIPTION
    the modulus R in    R = M mod N

   is defined as

     0 <= |R| < |M|
     sign R == sign M
     R = M - k*N, where k is integer

   thus, there's no requirement for M or N to be integers
*/

int decimal_mod(const decimal_t *from1, const decimal_t *from2, decimal_t *to) {
  return do_div_mod(from1, from2, nullptr, to, 0);
}
