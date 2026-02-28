/* Copyright (c) 2007, 2019, Oracle and/or its affiliates. All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License, version 2.0, as published by the Free Software Foundation.

   This library is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the library and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License, version 2.0, for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
   MA 02110-1301  USA */

/****************************************************************

  This file incorporates work covered by the following copyright and
  permission notice:

  The author of this software is David M. Gay.

  Copyright (c) 1991, 2000, 2001 by Lucent Technologies.

  Permission to use, copy, modify, and distribute this software for any
  purpose without fee is hereby granted, provided that this entire notice
  is included in all copies of any software which is or includes a copy
  or modification of this software and in all copies of the supporting
  documentation for such software.

  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
  WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
  REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
  OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.

 ***************************************************************/

#include "my_config.h"

#include <algorithm>
#include <limits>

#include "decimal.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"

#include <errno.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "m_string.h"
#include "my_dbug.h"

#ifndef EOVERFLOW
#define EOVERFLOW 84
#endif

/**
   Appears to suffice to not call malloc() in most cases.
   @todo
     see if it is possible to get rid of malloc().
     this constant is sufficient to avoid malloc() on all inputs I have tried.
*/
#define DTOA_BUFF_SIZE (460 * sizeof(void *))

/* Magic value returned by dtoa() to indicate overflow */
#define DTOA_OVERFLOW 9999

static double my_strtod_int(const char *, const char **, int *, char *, size_t);
static char *dtoa(double, int, int, int *, int *, char **, char *, size_t);
static void dtoa_free(char *, char *, size_t);

/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation using the 'f' format.

   @details
   This function is a wrapper around dtoa() to do the same as
   sprintf(to, "%-.*f", precision, x), though the conversion is usually more
   precise. The only difference is in handling [-,+]infinity and nan values,
   in which case we print '0\0' to the output string and indicate an overflow.

   @param x           the input floating point number.
   @param precision   the number of digits after the decimal point.
                      All properties of sprintf() apply:
                      - if the number of significant digits after the decimal
                        point is less than precision, the resulting string is
                        right-padded with zeros
                      - if the precision is 0, no decimal point appears
                      - if a decimal point appears, at least one digit appears
                        before it
   @param to          pointer to the output buffer. The longest string which
                      my_fcvt() can return is FLOATING_POINT_BUFFER bytes
                      (including the terminating '\0').
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      false  successful conversion
                      true   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.
   @param shorten     Whether to minimize the number of significant digits. If
                      true, write only the minimum number of digits necessary to
                      reproduce the double value when parsing the string. If
                      false, zeros are added to the end to reach the precision
                      limit.

   @return            number of written characters (excluding terminating '\0')
*/

static size_t my_fcvt_internal(double x, int precision, bool shorten, char *to,
                               bool *error) {
  int decpt, sign, len, i;
  char *res, *src, *end, *dst = to;
  char buf[DTOA_BUFF_SIZE];
  DBUG_ASSERT(precision >= 0 && precision < DECIMAL_NOT_SPECIFIED &&
              to != nullptr);

  res = dtoa(x, 5, precision, &decpt, &sign, &end, buf, sizeof(buf));

  if (decpt == DTOA_OVERFLOW) {
    dtoa_free(res, buf, sizeof(buf));
    *to++ = '0';
    *to = '\0';
    if (error != nullptr) *error = true;
    return 1;
  }

  src = res;
  len = (int)(end - src);

  if (sign) *dst++ = '-';

  if (decpt <= 0) {
    *dst++ = '0';
    *dst++ = '.';
    for (i = decpt; i < 0; i++) *dst++ = '0';
  }

  for (i = 1; i <= len; i++) {
    *dst++ = *src++;
    if (i == decpt && i < len) *dst++ = '.';
  }
  while (i++ <= decpt) *dst++ = '0';

  if (precision > 0 && !shorten) {
    if (len <= decpt) *dst++ = '.';

    for (i = precision - std::max(0, (len - decpt)); i > 0; i--) *dst++ = '0';
  }

  *dst = '\0';
  if (error != nullptr) *error = false;

  dtoa_free(res, buf, sizeof(buf));

  return dst - to;
}

/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation using the 'f' format.

   @details
   This function is a wrapper around dtoa() to do the same as
   sprintf(to, "%-.*f", precision, x), though the conversion is usually more
   precise. The only difference is in handling [-,+]infinity and nan values,
   in which case we print '0\0' to the output string and indicate an overflow.

   @param x           the input floating point number.
   @param precision   the number of digits after the decimal point.
                      All properties of sprintf() apply:
                      - if the number of significant digits after the decimal
                        point is less than precision, the resulting string is
                        right-padded with zeros
                      - if the precision is 0, no decimal point appears
                      - if a decimal point appears, at least one digit appears
                        before it
   @param to          pointer to the output buffer. The longest string which
                      my_fcvt() can return is FLOATING_POINT_BUFFER bytes
                      (including the terminating '\0').
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      false  successful conversion
                      true   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.

   @return            number of written characters (excluding terminating '\0')
*/
size_t my_fcvt(double x, int precision, char *to, bool *error) {
  return my_fcvt_internal(x, precision, false, to, error);
}

/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation using the 'f' format.

   @details
   This function is a wrapper around dtoa() to do almost the same as
   sprintf(to, "%-.*f", precision, x), though the conversion is usually more
   precise. The only difference is in handling [-,+]infinity and nan values,
   in which case we print '0\0' to the output string and indicate an overflow.

   The string always contains the minimum number of digits necessary to
   reproduce the same binary double value if the string is parsed back to a
   double value.

   @param x           the input floating point number.
   @param to          pointer to the output buffer. The longest string which
                      my_fcvt() can return is FLOATING_POINT_BUFFER bytes
                      (including the terminating '\0').
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      false  successful conversion
                      true   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.

   @return            number of written characters (excluding terminating '\0')
*/
size_t my_fcvt_compact(double x, char *to, bool *error) {
  return my_fcvt_internal(x, std::numeric_limits<double>::max_digits10, true,
                          to, error);
}

/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation with a given field width using the 'e' format
   (aka scientific notation) or the 'f' one.

   @details
   The format is chosen automatically to provide the most number of significant
   digits (and thus, precision) with a given field width. In many cases, the
   result is similar to that of sprintf(to, "%g", x) with a few notable
   differences:
   - the conversion is usually more precise than C library functions.
   - there is no 'precision' argument. instead, we specify the number of
     characters available for conversion (i.e. a field width).
   - the result never exceeds the specified field width. If the field is too
     short to contain even a rounded decimal representation, my_gcvt()
     indicates overflow and truncates the output string to the specified width.
   - float-type arguments are handled differently than double ones. For a
     float input number (i.e. when the 'type' argument is MY_GCVT_ARG_FLOAT)
     we deliberately limit the precision of conversion by FLT_DIG digits to
     avoid garbage past the significant digits.
   - unlike sprintf(), in cases where the 'e' format is preferred,  we don't
     zero-pad the exponent to save space for significant digits. The '+' sign
     for a positive exponent does not appear for the same reason.

   @param x           the input floating point number.
   @param type        is either MY_GCVT_ARG_FLOAT or MY_GCVT_ARG_DOUBLE.
                      Specifies the type of the input number (see notes above).
   @param width       field width in characters. The minimal field width to
                      hold any number representation (albeit rounded) is 7
                      characters ("-Ne-NNN").
   @param to          pointer to the output buffer. The result is always
                      zero-terminated, and the longest returned string is thus
                      'width + 1' bytes.
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      false  successful conversion
                      true   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.
   @return            number of written characters (excluding terminating '\0')

   @todo
   Check if it is possible and  makes sense to do our own rounding on top of
   dtoa() instead of calling dtoa() twice in (rare) cases when the resulting
   string representation does not fit in the specified field width and we want
   to re-round the input number with fewer significant digits. Examples:

     my_gcvt(-9e-3, ..., 4, ...);
     my_gcvt(-9e-3, ..., 2, ...);
     my_gcvt(1.87e-3, ..., 4, ...);
     my_gcvt(55, ..., 1, ...);

   We do our best to minimize such cases by:

   - passing to dtoa() the field width as the number of significant digits

   - removing the sign of the number early (and decreasing the width before
     passing it to dtoa())

   - choosing the proper format to preserve the most number of significant
     digits.
*/

size_t my_gcvt(double x, my_gcvt_arg_type type, int width, char *to,
               bool *error) {
  int decpt, sign, len, exp_len;
  char *res, *src, *end, *dst = to, *dend = dst + width;
  char buf[DTOA_BUFF_SIZE];
  bool have_space, force_e_format;
  DBUG_ASSERT(width > 0 && to != nullptr);

  /* We want to remove '-' from equations early */
  if (x < 0.) width--;

  res =
      dtoa(x, 4, type == MY_GCVT_ARG_DOUBLE ? width : std::min(width, FLT_DIG),
           &decpt, &sign, &end, buf, sizeof(buf));
  if (decpt == DTOA_OVERFLOW) {
    dtoa_free(res, buf, sizeof(buf));
    *to++ = '0';
    *to = '\0';
    if (error != nullptr) *error = true;
    return 1;
  }

  if (error != nullptr) *error = false;

  src = res;
  len = (int)(end - res);

  /*
    Number of digits in the exponent from the 'e' conversion.
     The sign of the exponent is taken into account separetely, we don't need
     to count it here.
   */
  exp_len = 1 + (decpt >= 101 || decpt <= -99) + (decpt >= 11 || decpt <= -9);

  /*
     Do we have enough space for all digits in the 'f' format?
     Let 'len' be the number of significant digits returned by dtoa,
     and F be the length of the resulting decimal representation.
     Consider the following cases:
     1. decpt <= 0, i.e. we have "0.NNN" => F = len - decpt + 2
     2. 0 < decpt < len, i.e. we have "NNN.NNN" => F = len + 1
     3. len <= decpt, i.e. we have "NNN00" => F = decpt
  */
  have_space =
      (decpt <= 0 ? len - decpt + 2
                  : decpt > 0 && decpt < len ? len + 1 : decpt) <= width;
  /*
    The following is true when no significant digits can be placed with the
    specified field width using the 'f' format, and the 'e' format
    will not be truncated.
  */
  force_e_format = (decpt <= 0 && width <= 2 - decpt && width >= 3 + exp_len);
  /*
    Assume that we don't have enough space to place all significant digits in
    the 'f' format. We have to choose between the 'e' format and the 'f' one
    to keep as many significant digits as possible.
    Let E and F be the lengths of decimal representaion in the 'e' and 'f'
    formats, respectively. We want to use the 'f' format if, and only if F <= E.
    Consider the following cases:
    1. decpt <= 0.
       F = len - decpt + 2 (see above)
       E = len + (len > 1) + 1 + 1 (decpt <= -99) + (decpt <= -9) + 1
       ("N.NNe-MMM")
       (F <= E) <=> (len == 1 && decpt >= -1) || (len > 1 && decpt >= -2)
       We also need to ensure that if the 'f' format is chosen,
       the field width allows us to place at least one significant digit
       (i.e. width > 2 - decpt). If not, we prefer the 'e' format.
    2. 0 < decpt < len
       F = len + 1 (see above)
       E = len + 1 + 1 + ... ("N.NNeMMM")
       F is always less than E.
    3. len <= decpt <= width
       In this case we have enough space to represent the number in the 'f'
       format, so we prefer it with some exceptions.
    4. width < decpt
       The number cannot be represented in the 'f' format at all, always use
       the 'e' 'one.
  */
  if ((have_space ||
       /*
         Not enough space, let's see if the 'f' format provides the most number
         of significant digits.
       */
       ((decpt <= width &&
         (decpt >= -1 || (decpt == -2 && (len > 1 || !force_e_format)))) &&
        !force_e_format)) &&

      /*
        Use the 'e' format in some cases even if we have enough space for the
        'f' one. See comment for MAX_DECPT_FOR_F_FORMAT.
      */
      (!have_space || (decpt >= -MAX_DECPT_FOR_F_FORMAT + 1 &&
                       (decpt <= MAX_DECPT_FOR_F_FORMAT || len > decpt)))) {
    /* 'f' format */
    int i;

    width -= (decpt < len) + (decpt <= 0 ? 1 - decpt : 0);

    /* Do we have to truncate any digits? */
    if (width < len) {
      if (width < decpt) {
        if (error != nullptr) *error = true;
        width = decpt;
      }

      /*
        We want to truncate (len - width) least significant digits after the
        decimal point. For this we are calling dtoa with mode=5, passing the
        number of significant digits = (len-decpt) - (len-width) = width-decpt
      */
      dtoa_free(res, buf, sizeof(buf));
      res = dtoa(x, 5, width - decpt, &decpt, &sign, &end, buf, sizeof(buf));
      src = res;
      len = (int)(end - res);
    }

    if (len == 0) {
      /* Underflow. Just print '0' and exit */
      *dst++ = '0';
      goto end;
    }

    /*
      At this point we are sure we have enough space to put all digits
      returned by dtoa
    */
    if (sign && dst < dend) *dst++ = '-';
    if (decpt <= 0) {
      if (dst < dend) *dst++ = '0';
      if (len > 0 && dst < dend) *dst++ = '.';
      for (; decpt < 0 && dst < dend; decpt++) *dst++ = '0';
    }

    for (i = 1; i <= len && dst < dend; i++) {
      *dst++ = *src++;
      if (i == decpt && i < len && dst < dend) *dst++ = '.';
    }
    while (i++ <= decpt && dst < dend) *dst++ = '0';
  } else {
    /* 'e' format */
    int decpt_sign = 0;

    if (--decpt < 0) {
      decpt = -decpt;
      width--;
      decpt_sign = 1;
    }
    width -= 1 + exp_len; /* eNNN */

    if (len > 1) width--;

    if (width <= 0) {
      /* Overflow */
      if (error != nullptr) *error = true;
      width = 0;
    }

    /* Do we have to truncate any digits? */
    if (width < len) {
      /* Yes, re-convert with a smaller width */
      dtoa_free(res, buf, sizeof(buf));
      res = dtoa(x, 4, width, &decpt, &sign, &end, buf, sizeof(buf));
      src = res;
      len = (int)(end - res);
      if (--decpt < 0) decpt = -decpt;
    }
    /*
      At this point we are sure we have enough space to put all digits
      returned by dtoa
    */
    if (sign && dst < dend) *dst++ = '-';
    if (dst < dend) *dst++ = *src++;
    if (len > 1 && dst < dend) {
      *dst++ = '.';
      while (src < end && dst < dend) *dst++ = *src++;
    }
    if (dst < dend) *dst++ = 'e';
    if (decpt_sign && dst < dend) *dst++ = '-';

    if (decpt >= 100 && dst < dend) {
      *dst++ = decpt / 100 + '0';
      decpt %= 100;
      if (dst < dend) *dst++ = decpt / 10 + '0';
    } else if (decpt >= 10 && dst < dend)
      *dst++ = decpt / 10 + '0';
    if (dst < dend) *dst++ = decpt % 10 + '0';
  }

end:
  dtoa_free(res, buf, sizeof(buf));
  *dst = '\0';

  return dst - to;
}

/**
   @brief
   Converts string to double (string does not have to be zero-terminated)

   @details
   This is a wrapper around dtoa's version of strtod().

   @param str     input string
   @param end     address of a pointer to the first character after the input
                  string. Upon return the pointer is set to point to the first
                  rejected character.
   @param error   Upon return is set to EOVERFLOW in case of underflow or
                  overflow.

   @return        The resulting double value. In case of underflow, 0.0 is
                  returned. In case overflow, signed DBL_MAX is returned.
*/

double my_strtod(const char *str, const char **end, int *error) {
  char buf[DTOA_BUFF_SIZE];
  double res;
  DBUG_ASSERT(end != nullptr &&
              ((str != nullptr && *end != nullptr) ||
               (str == nullptr && *end == nullptr)) &&
              error != nullptr);

  res = my_strtod_int(str, end, error, buf, sizeof(buf));
  return (*error == 0) ? res : (res < 0 ? -DBL_MAX : DBL_MAX);
}

double my_atof(const char *nptr) {
  int error;
  const char *end = nptr + 65535; /* Should be enough */
  return (my_strtod(nptr, &end, &error));
}

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/
/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").      */

/*
  Original copy of the software is located at http://www.netlib.org/fp/dtoa.c
  It was adjusted to serve MySQL server needs:
  * strtod() was modified to not expect a zero-terminated string.
    It now honors 'se' (end of string) argument as the input parameter,
    not just as the output one.
  * in dtoa(), in case of overflow/underflow/NaN result string now contains "0";
    decpt is set to DTOA_OVERFLOW to indicate overflow.
  * support for VAX, IBM mainframe and 16-bit hardware removed
  * we always assume that 64-bit integer type is available
  * support for Kernigan-Ritchie style headers (pre-ANSI compilers)
    removed
  * all gcc warnings ironed out
  * we always assume multithreaded environment, so we had to change
    memory allocation procedures to use stack in most cases;
    malloc is used as the last resort.
  * pow5mult rewritten to use pre-calculated pow5 list instead of
    the one generated on the fly.
*/

/*
  On a machine with IEEE extended-precision registers, it is
  necessary to specify double-precision (53-bit) rounding precision
  before invoking strtod or dtoa.  If the machine uses (the equivalent
  of) Intel 80x87 arithmetic, the call
       _control87(PC_53, MCW_PC);
  does this with many compilers.  Whether this or another call is
  appropriate depends on the compiler; for this to work, it may be
  necessary to #include "float.h" or another system-dependent header
  file.
*/

/*
  #define Honor_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
       and dtoa should round accordingly.
  #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
       and Honor_FLT_ROUNDS is not #defined.

  TODO: check if we can get rid of the above two
*/

typedef int32 Long;
typedef uint32 ULong;
typedef int64 LLong;
typedef uint64 ULLong;

typedef union {
  double d;
  ULong L[2];
} U;

#if defined(WORDS_BIGENDIAN)
#define word0(x) (x)->L[0]
#define word1(x) (x)->L[1]
#else
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#endif

#define dval(x) (x)->d

/* #define P DBL_MANT_DIG */
/* Ten_pmax= floor(P*log(2)/log(5)) */
/* Bletch= (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max= floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max= floor(P*log(FLT_RADIX)/log(10) - 1) */

#define Exp_shift 20
#define Exp_shift1 20
#define Exp_msk1 0x100000
#define Exp_mask 0x7ff00000
#define P 53
#define Bias 1023
#define Emin (-1022)
#define Exp_1 0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask 0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask 0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny1 1
#define Quick_max 14
#define Int_max 14

#ifndef Flt_Rounds
#ifdef FLT_ROUNDS
#define Flt_Rounds FLT_ROUNDS
#else
#define Flt_Rounds 1
#endif
#endif /*Flt_Rounds*/

#ifdef Honor_FLT_ROUNDS
#define Rounding rounding
#undef Check_FLT_ROUNDS
#define Check_FLT_ROUNDS
#endif

#define rounded_product(a, b) a *= b
#define rounded_quotient(a, b) a /= b

#define Big0 (Frac_mask1 | Exp_msk1 * (DBL_MAX_EXP + Bias - 1))
#define Big1 0xffffffff
#define FFFFFFFF 0xffffffffUL

/* This is tested to be enough for dtoa */

#define Kmax 15

#define Bcopy(x, y)                          \
  memcpy((char *)&x->sign, (char *)&y->sign, \
         2 * sizeof(int) + y->wds * sizeof(ULong))

/* Arbitrary-length integer */

typedef struct Bigint {
  union {
    ULong *x;            /* points right after this Bigint object */
    struct Bigint *next; /* to maintain free lists */
  } p;
  int k;      /* 2^k = maxwds */
  int maxwds; /* maximum length in 32-bit words */
  int sign;   /* not zero if number is negative */
  int wds;    /* current length in 32-bit words */
} Bigint;

/* A simple stack-memory based allocator for Bigints */

typedef struct Stack_alloc {
  char *begin;
  char *free;
  char *end;
  /*
    Having list of free blocks lets us reduce maximum required amount
    of memory from ~4000 bytes to < 1680 (tested on x86).
  */
  Bigint *freelist[Kmax + 1];
} Stack_alloc;

/*
  Try to allocate object on stack, and resort to malloc if all
  stack memory is used. Ensure allocated objects to be aligned by the pointer
  size in order to not break the alignment rules when storing a pointer to a
  Bigint.
*/

static Bigint *Balloc(int k, Stack_alloc *alloc) {
  Bigint *rv;
  DBUG_ASSERT(k <= Kmax);
  if (k <= Kmax && alloc->freelist[k]) {
    rv = alloc->freelist[k];
    alloc->freelist[k] = rv->p.next;
  } else {
    int x, len;

    x = 1 << k;
    len = MY_ALIGN(sizeof(Bigint) + x * sizeof(ULong), SIZEOF_CHARP);

    if (alloc->free + len <= alloc->end) {
      rv = (Bigint *)alloc->free;
      alloc->free += len;
    } else
      rv = (Bigint *)malloc(len);

    rv->k = k;
    rv->maxwds = x;
  }
  rv->sign = rv->wds = 0;
  rv->p.x = (ULong *)(rv + 1);
  return rv;
}

/*
  If object was allocated on stack, try putting it to the free
  list. Otherwise call free().
*/

static void Bfree(Bigint *v, Stack_alloc *alloc) {
  char *gptr = (char *)v; /* generic pointer */
  if (gptr < alloc->begin || gptr >= alloc->end)
    free(gptr);
  else if (v->k <= Kmax) {
    /*
      Maintain free lists only for stack objects: this way we don't
      have to bother with freeing lists in the end of dtoa;
      heap should not be used normally anyway.
    */
    v->p.next = alloc->freelist[v->k];
    alloc->freelist[v->k] = v;
  }
}

/*
  This is to place return value of dtoa in: tries to use stack
  as well, but passes by free lists management and just aligns len by
  the pointer size in order to not break the alignment rules when storing a
  pointer to a Bigint.
*/

static char *dtoa_alloc(int i, Stack_alloc *alloc) {
  char *rv;
  int aligned_size = MY_ALIGN(i, SIZEOF_CHARP);
  if (alloc->free + aligned_size <= alloc->end) {
    rv = alloc->free;
    alloc->free += aligned_size;
  } else
    rv = static_cast<char *>(malloc(i));
  return rv;
}

/*
  dtoa_free() must be used to free values s returned by dtoa()
  This is the counterpart of dtoa_alloc()
*/

static void dtoa_free(char *gptr, char *buf, size_t buf_size) {
  if (gptr < buf || gptr >= buf + buf_size) free(gptr);
}

/* Bigint arithmetic functions */

/* Multiply by m and add a */

static Bigint *multadd(Bigint *b, int m, int a, Stack_alloc *alloc) {
  int i, wds;
  ULong *x;
  ULLong carry, y;
  Bigint *b1;

  wds = b->wds;
  x = b->p.x;
  i = 0;
  carry = a;
  do {
    y = *x * (ULLong)m + carry;
    carry = y >> 32;
    *x++ = (ULong)(y & FFFFFFFF);
  } while (++i < wds);
  if (carry) {
    if (wds >= b->maxwds) {
      b1 = Balloc(b->k + 1, alloc);
      Bcopy(b1, b);
      Bfree(b, alloc);
      b = b1;
    }
    b->p.x[wds++] = (ULong)carry;
    b->wds = wds;
  }
  return b;
}

/**
  Converts a string to Bigint.

  Now we have nd0 digits, starting at s, followed by a
  decimal point, followed by nd-nd0 digits.
  Unless nd0 == nd, in which case we have a number of the form:
     ".xxxxxx"    or    "xxxxxx."

  @param s     Input string, already partially parsed by my_strtod_int().
  @param nd0   Number of digits before decimal point.
  @param nd    Total number of digits.
  @param y9    Pre-computed value of the first nine digits.
  @param alloc Stack allocator for Bigints.
 */
static Bigint *s2b(const char *s, int nd0, int nd, ULong y9,
                   Stack_alloc *alloc) {
  Bigint *b;
  int i, k;
  Long x, y;

  x = (nd + 8) / 9;
  for (k = 0, y = 1; x > y; y <<= 1, k++)
    ;
  b = Balloc(k, alloc);
  b->p.x[0] = y9;
  b->wds = 1;

  i = 9;
  if (9 < nd0) {
    s += 9;
    do
      b = multadd(b, 10, *s++ - '0', alloc);
    while (++i < nd0);
    s++; /* skip '.' */
  } else
    s += 10;
  /* now do the fractional part */
  for (; i < nd; i++) b = multadd(b, 10, *s++ - '0', alloc);
  return b;
}

static int hi0bits(ULong x) {
  int k = 0;

  if (!(x & 0xffff0000)) {
    k = 16;
    x <<= 16;
  }
  if (!(x & 0xff000000)) {
    k += 8;
    x <<= 8;
  }
  if (!(x & 0xf0000000)) {
    k += 4;
    x <<= 4;
  }
  if (!(x & 0xc0000000)) {
    k += 2;
    x <<= 2;
  }
  if (!(x & 0x80000000)) {
    k++;
    if (!(x & 0x40000000)) return 32;
  }
  return k;
}

static int lo0bits(ULong *y) {
  int k;
  ULong x = *y;

  if (x & 7) {
    if (x & 1) return 0;
    if (x & 2) {
      *y = x >> 1;
      return 1;
    }
    *y = x >> 2;
    return 2;
  }
  k = 0;
  if (!(x & 0xffff)) {
    k = 16;
    x >>= 16;
  }
  if (!(x & 0xff)) {
    k += 8;
    x >>= 8;
  }
  if (!(x & 0xf)) {
    k += 4;
    x >>= 4;
  }
  if (!(x & 0x3)) {
    k += 2;
    x >>= 2;
  }
  if (!(x & 1)) {
    k++;
    x >>= 1;
    if (!x) return 32;
  }
  *y = x;
  return k;
}

/* Convert integer to Bigint number */

static Bigint *i2b(int i, Stack_alloc *alloc) {
  Bigint *b;

  b = Balloc(1, alloc);
  b->p.x[0] = i;
  b->wds = 1;
  return b;
}

/* Multiply two Bigint numbers */

static Bigint *mult(Bigint *a, Bigint *b, Stack_alloc *alloc) {
  Bigint *c;
  int k, wa, wb, wc;
  ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
  ULong y;
  ULLong carry, z;

  if (a->wds < b->wds) {
    c = a;
    a = b;
    b = c;
  }
  k = a->k;
  wa = a->wds;
  wb = b->wds;
  wc = wa + wb;
  if (wc > a->maxwds) k++;
  c = Balloc(k, alloc);
  for (x = c->p.x, xa = x + wc; x < xa; x++) *x = 0;
  xa = a->p.x;
  xae = xa + wa;
  xb = b->p.x;
  xbe = xb + wb;
  xc0 = c->p.x;
  for (; xb < xbe; xc0++) {
    if ((y = *xb++)) {
      x = xa;
      xc = xc0;
      carry = 0;
      do {
        z = *x++ * (ULLong)y + *xc + carry;
        carry = z >> 32;
        *xc++ = (ULong)(z & FFFFFFFF);
      } while (x < xae);
      *xc = (ULong)carry;
    }
  }
  for (xc0 = c->p.x, xc = xc0 + wc; wc > 0 && !*--xc; --wc)
    ;
  c->wds = wc;
  return c;
}

/*
  Precalculated array of powers of 5: tested to be enough for
  vasting majority of dtoa_r cases.
*/

static ULong powers5[] = {
    625UL,

    390625UL,

    2264035265UL, 35UL,

    2242703233UL, 762134875UL,  1262UL,

    3211403009UL, 1849224548UL, 3668416493UL, 3913284084UL, 1593091UL,

    781532673UL,  64985353UL,   253049085UL,  594863151UL,  3553621484UL,
    3288652808UL, 3167596762UL, 2788392729UL, 3911132675UL, 590UL,

    2553183233UL, 3201533787UL, 3638140786UL, 303378311UL,  1809731782UL,
    3477761648UL, 3583367183UL, 649228654UL,  2915460784UL, 487929380UL,
    1011012442UL, 1677677582UL, 3428152256UL, 1710878487UL, 1438394610UL,
    2161952759UL, 4100910556UL, 1608314830UL, 349175UL};

static Bigint p5_a[] = {
    /*  { x } - k - maxwds - sign - wds */
    {{powers5}, 1, 1, 0, 1},       {{powers5 + 1}, 1, 1, 0, 1},
    {{powers5 + 2}, 1, 2, 0, 2},   {{powers5 + 4}, 2, 3, 0, 3},
    {{powers5 + 7}, 3, 5, 0, 5},   {{powers5 + 12}, 4, 10, 0, 10},
    {{powers5 + 22}, 5, 19, 0, 19}};

#define P5A_MAX (sizeof(p5_a) / sizeof(*p5_a) - 1)

static Bigint *pow5mult(Bigint *b, int k, Stack_alloc *alloc) {
  Bigint *b1, *p5, *p51 = nullptr;
  int i;
  static int p05[3] = {5, 25, 125};
  bool overflow = false;

  if ((i = k & 3)) b = multadd(b, p05[i - 1], 0, alloc);

  if (!(k >>= 2)) return b;
  p5 = p5_a;
  for (;;) {
    if (k & 1) {
      b1 = mult(b, p5, alloc);
      Bfree(b, alloc);
      b = b1;
    }
    if (!(k >>= 1)) break;
    /* Calculate next power of 5 */
    if (overflow) {
      p51 = mult(p5, p5, alloc);
      Bfree(p5, alloc);
      p5 = p51;
    } else if (p5 < p5_a + P5A_MAX)
      ++p5;
    else if (p5 == p5_a + P5A_MAX) {
      p5 = mult(p5, p5, alloc);
      overflow = true;
    }
  }
  if (p51) Bfree(p51, alloc);
  return b;
}

static Bigint *lshift(Bigint *b, int k, Stack_alloc *alloc) {
  int i, k1, n, n1;
  Bigint *b1;
  ULong *x, *x1, *xe, z;

  n = k >> 5;
  k1 = b->k;
  n1 = n + b->wds + 1;
  for (i = b->maxwds; n1 > i; i <<= 1) k1++;
  b1 = Balloc(k1, alloc);
  x1 = b1->p.x;
  for (i = 0; i < n; i++) *x1++ = 0;
  x = b->p.x;
  xe = x + b->wds;
  if (k &= 0x1f) {
    k1 = 32 - k;
    z = 0;
    do {
      *x1++ = *x << k | z;
      z = *x++ >> k1;
    } while (x < xe);
    if ((*x1 = z)) ++n1;
  } else
    do
      *x1++ = *x++;
    while (x < xe);
  b1->wds = n1 - 1;
  Bfree(b, alloc);
  return b1;
}

static int cmp(Bigint *a, Bigint *b) {
  ULong *xa, *xa0, *xb, *xb0;
  int i, j;

  i = a->wds;
  j = b->wds;
  if (i -= j) return i;
  xa0 = a->p.x;
  xa = xa0 + j;
  xb0 = b->p.x;
  xb = xb0 + j;
  for (;;) {
    if (*--xa != *--xb) return *xa < *xb ? -1 : 1;
    if (xa <= xa0) break;
  }
  return 0;
}

static Bigint *diff(Bigint *a, Bigint *b, Stack_alloc *alloc) {
  Bigint *c;
  int i, wa, wb;
  ULong *xa, *xae, *xb, *xbe, *xc;
  ULLong borrow, y;

  i = cmp(a, b);
  if (!i) {
    c = Balloc(0, alloc);
    c->wds = 1;
    c->p.x[0] = 0;
    return c;
  }
  if (i < 0) {
    c = a;
    a = b;
    b = c;
    i = 1;
  } else
    i = 0;
  c = Balloc(a->k, alloc);
  c->sign = i;
  wa = a->wds;
  xa = a->p.x;
  xae = xa + wa;
  wb = b->wds;
  xb = b->p.x;
  xbe = xb + wb;
  xc = c->p.x;
  borrow = 0;
  do {
    y = (ULLong)*xa++ - *xb++ - borrow;
    borrow = y >> 32 & (ULong)1;
    *xc++ = (ULong)(y & FFFFFFFF);
  } while (xb < xbe);
  while (xa < xae) {
    y = *xa++ - borrow;
    borrow = y >> 32 & (ULong)1;
    *xc++ = (ULong)(y & FFFFFFFF);
  }
  while (!*--xc) wa--;
  c->wds = wa;
  return c;
}

static double ulp(U *x) {
  Long L;
  U u;

  L = (word0(x) & Exp_mask) - (P - 1) * Exp_msk1;
  word0(&u) = L;
  word1(&u) = 0;
  return dval(&u);
}

static double b2d(Bigint *a, int *e) {
  ULong *xa, *xa0, w, y, z;
  int k;
  U d;
#define d0 word0(&d)
#define d1 word1(&d)

  xa0 = a->p.x;
  xa = xa0 + a->wds;
  y = *--xa;
  k = hi0bits(y);
  *e = 32 - k;
  if (k < Ebits) {
    d0 = Exp_1 | y >> (Ebits - k);
    w = xa > xa0 ? *--xa : 0;
    d1 = y << ((32 - Ebits) + k) | w >> (Ebits - k);
    goto ret_d;
  }
  z = xa > xa0 ? *--xa : 0;
  if (k -= Ebits) {
    d0 = Exp_1 | y << k | z >> (32 - k);
    y = xa > xa0 ? *--xa : 0;
    d1 = z << k | y >> (32 - k);
  } else {
    d0 = Exp_1 | y;
    d1 = z;
  }
ret_d:
#undef d0
#undef d1
  return dval(&d);
}

static Bigint *d2b(U *d, int *e, int *bits, Stack_alloc *alloc) {
  Bigint *b;
  int de, k;
  ULong *x, y, z;
  int i;
#define d0 word0(d)
#define d1 word1(d)

  b = Balloc(1, alloc);
  x = b->p.x;

  z = d0 & Frac_mask;
  d0 &= 0x7fffffff; /* clear sign bit, which we ignore */
  if ((de = (int)(d0 >> Exp_shift))) z |= Exp_msk1;
  if ((y = d1)) {
    if ((k = lo0bits(&y))) {
      x[0] = y | z << (32 - k);
      z >>= k;
    } else
      x[0] = y;
    i = b->wds = (x[1] = z) ? 2 : 1;
  } else {
    k = lo0bits(&z);
    x[0] = z;
    i = b->wds = 1;
    k += 32;
  }
  if (de) {
    *e = de - Bias - (P - 1) + k;
    *bits = P - k;
  } else {
    *e = de - Bias - (P - 1) + 1 + k;
    *bits = 32 * i - hi0bits(x[i - 1]);
  }
  return b;
#undef d0
#undef d1
}

static double ratio(Bigint *a, Bigint *b) {
  U da, db;
  int k, ka, kb;

  dval(&da) = b2d(a, &ka);
  dval(&db) = b2d(b, &kb);
  k = ka - kb + 32 * (a->wds - b->wds);
  if (k > 0)
    word0(&da) += k * Exp_msk1;
  else {
    k = -k;
    word0(&db) += k * Exp_msk1;
  }
  return dval(&da) / dval(&db);
}

static const double tens[] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,
                              1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
                              1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

static const double bigtens[] = {1e16, 1e32, 1e64, 1e128, 1e256};
static const double tinytens[] = {
    1e-16, 1e-32, 1e-64, 1e-128,
    9007199254740992. * 9007199254740992.e-256 /* = 2^106 * 1e-53 */
};
/*
  The factor of 2^53 in tinytens[4] helps us avoid setting the underflow
  flag unnecessarily.  It leads to a song and dance at the end of strtod.
*/
#define Scale_Bit 0x10
#define n_bigtens 5

/*
  strtod for IEEE--arithmetic machines.

  This strtod returns a nearest machine number to the input decimal
  string (or sets errno to EOVERFLOW). Ties are broken by the IEEE round-even
  rule.

  Inspired loosely by William D. Clinger's paper "How to Read Floating
  Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].

  Modifications:

   1. We only require IEEE (not IEEE double-extended).
   2. We get by with floating-point arithmetic in a case that
     Clinger missed -- when we're computing d * 10^n
     for a small integer d and the integer n is not too
     much larger than 22 (the maximum integer k for which
     we can represent 10^k exactly), we may be able to
     compute (d*10^k) * 10^(e-k) with just one roundoff.
   3. Rather than a bit-at-a-time adjustment of the binary
     result in the hard case, we use floating-point
     arithmetic to determine the adjustment to within
     one bit; only in really hard cases do we need to
     compute a second residual.
   4. Because of 3., we don't need a large table of powers of 10
     for ten-to-e (just some small tables, e.g. of 10^k
     for 0 <= k <= 22).
*/

static double my_strtod_int(const char *s00, const char **se, int *error,
                            char *buf, size_t buf_size) {
  int scale;
  int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c = 0, dsign, e, e1, esign, i, j, k,
                                            nd, nd0, nf, nz, nz0, sign;
  const char *s, *s0, *s1, *end = *se;
  double aadj, aadj1;
  U aadj2, adj, rv, rv0;
  Long L;
  ULong y, z;
  Bigint *bb = nullptr, *bb1, *bd = nullptr, *bd0, *bs = nullptr,
         *delta = nullptr;
#ifdef Honor_FLT_ROUNDS
  int rounding;
#endif
  Stack_alloc alloc;

  *error = 0;

  alloc.begin = alloc.free = buf;
  alloc.end = buf + buf_size;
  memset(alloc.freelist, 0, sizeof(alloc.freelist));

  sign = nz0 = nz = 0;
  dval(&rv) = 0.;
  for (s = s00; s < end; s++) switch (*s) {
      case '-':
        sign = 1;
        // Fall through.
      case '+':
        s++;
        goto break2;
      case '\t':
      case '\n':
      case '\v':
      case '\f':
      case '\r':
      case ' ':
        continue;
      default:
        goto break2;
    }
break2:
  if (s >= end) goto ret0;

  if (*s == '0') {
    nz0 = 1;
    while (++s < end && *s == '0')
      ;
    if (s >= end) goto ret;
  }
  s0 = s;
  y = z = 0;
  for (nd = nf = 0; s < end && (c = *s) >= '0' && c <= '9'; nd++, s++)
    if (nd < 9)
      y = 10 * y + c - '0';
    else if (nd < 16)
      z = 10 * z + c - '0';
  nd0 = nd;
  if (s < end && c == '.') {
    if (++s < end) c = *s;
    if (!nd) {
      for (; s < end; ++s) {
        c = *s;
        if (c != '0') break;
        nz++;
      }
      if (s < end && c > '0' && c <= '9') {
        s0 = s;
        nf += nz;
        nz = 0;
      } else
        goto dig_done;
    }
    for (; s < end; ++s) {
      c = *s;
      if (c < '0' || c > '9') break;
      /*
        Here we are parsing the fractional part.
        We can stop counting digits after a while: the extra digits
        will not contribute to the actual result produced by s2b().
        We have to continue scanning, in case there is an exponent part.
       */
      if (nd < 2 * DBL_DIG) {
        nz++;
        if (c -= '0') {
          nf += nz;
          for (i = 1; i < nz; i++)
            if (nd++ < 9)
              y *= 10;
            else if (nd <= DBL_DIG + 1)
              z *= 10;
          if (nd++ < 9)
            y = 10 * y + c;
          else if (nd <= DBL_DIG + 1)
            z = 10 * z + c;
          nz = 0;
        }
      }
    }
  }
dig_done:
  e = 0;
  if (s < end && (c == 'e' || c == 'E')) {
    if (!nd && !nz && !nz0) goto ret0;
    s00 = s;
    esign = 0;
    if (++s < end) switch (c = *s) {
        case '-':
          esign = 1;
          // Fall through.
        case '+':
          if (++s < end) c = *s;
      }
    if (s < end && c >= '0' && c <= '9') {
      while (s < end && c == '0') c = *++s;
      if (s < end && c > '0' && c <= '9') {
        L = c - '0';
        s1 = s;
        // Avoid overflow in loop body below.
        while (++s < end && (c = *s) >= '0' && c <= '9' &&
               L < (std::numeric_limits<Long>::max() - 255) / 10) {
          L = 10 * L + c - '0';
        }
        if (s - s1 > 8 || L > 19999)
          /* Avoid confusion from exponents
           * so large that e might overflow.
           */
          e = 19999; /* safe for 16 bit ints */
        else
          e = (int)L;
        if (esign) e = -e;
      } else
        e = 0;
    } else
      s = s00;
  }
  if (!nd) {
    if (!nz && !nz0) {
    ret0:
      s = s00;
      sign = 0;
    }
    goto ret;
  }
  e1 = e -= nf;

  /*
    Now we have nd0 digits, starting at s0, followed by a
    decimal point, followed by nd-nd0 digits.  The number we're
    after is the integer represented by those digits times
    10**e
   */

  if (!nd0) nd0 = nd;
  k = nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
  dval(&rv) = y;
  if (k > 9) {
    dval(&rv) = tens[k - 9] * dval(&rv) + z;
  }
  bd0 = nullptr;
  if (nd <= DBL_DIG
#ifndef Honor_FLT_ROUNDS
      && Flt_Rounds == 1
#endif
  ) {
    if (!e) goto ret;
    if (e > 0) {
      if (e <= Ten_pmax) {
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign) {
          rv.d = -rv.d;
          sign = 0;
        }
#endif
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
      i = DBL_DIG - nd;
      if (e <= Ten_pmax + i) {
        /*
          A fancier test would sometimes let us do
          this for larger i values.
        */
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign) {
          rv.d = -rv.d;
          sign = 0;
        }
#endif
        e -= i;
        dval(&rv) *= tens[i];
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
    }
#ifndef Inaccurate_Divide
    else if (e >= -Ten_pmax) {
#ifdef Honor_FLT_ROUNDS
      /* round correctly FLT_ROUNDS = 2 or 3 */
      if (sign) {
        rv.d = -rv.d;
        sign = 0;
      }
#endif
      /* rv = */ rounded_quotient(dval(&rv), tens[-e]);
      goto ret;
    }
#endif
  }
  e1 += nd - k;

  scale = 0;
#ifdef Honor_FLT_ROUNDS
  if ((rounding = Flt_Rounds) >= 2) {
    if (sign)
      rounding = rounding == 2 ? 0 : 2;
    else if (rounding != 2)
      rounding = 0;
  }
#endif

  /* Get starting approximation = rv * 10**e1 */

  if (e1 > 0) {
    if ((i = e1 & 15)) dval(&rv) *= tens[i];
    if (e1 &= ~15) {
      if (e1 > DBL_MAX_10_EXP) {
      ovfl:
        *error = EOVERFLOW;
        /* Can't trust HUGE_VAL */
#ifdef Honor_FLT_ROUNDS
        switch (rounding) {
          case 0: /* toward 0 */
          case 3: /* toward -infinity */
            word0(&rv) = Big0;
            word1(&rv) = Big1;
            break;
          default:
            word0(&rv) = Exp_mask;
            word1(&rv) = 0;
        }
#else  /*Honor_FLT_ROUNDS*/
        word0(&rv) = Exp_mask;
        word1(&rv) = 0;
#endif /*Honor_FLT_ROUNDS*/
        if (bd0) goto retfree;
        goto ret;
      }
      e1 >>= 4;
      for (j = 0; e1 > 1; j++, e1 >>= 1)
        if (e1 & 1) dval(&rv) *= bigtens[j];
      /* The last multiplication could overflow. */
      word0(&rv) -= P * Exp_msk1;
      dval(&rv) *= bigtens[j];
      if ((z = word0(&rv) & Exp_mask) > Exp_msk1 * (DBL_MAX_EXP + Bias - P))
        goto ovfl;
      if (z > Exp_msk1 * (DBL_MAX_EXP + Bias - 1 - P)) {
        /* set to largest number (Can't trust DBL_MAX) */
        word0(&rv) = Big0;
        word1(&rv) = Big1;
      } else
        word0(&rv) += P * Exp_msk1;
    }
  } else if (e1 < 0) {
    e1 = -e1;
    if ((i = e1 & 15)) dval(&rv) /= tens[i];
    if ((e1 >>= 4)) {
      if (e1 >= 1 << n_bigtens) goto undfl;
      if (e1 & Scale_Bit) scale = 2 * P;
      for (j = 0; e1 > 0; j++, e1 >>= 1)
        if (e1 & 1) dval(&rv) *= tinytens[j];
      if (scale &&
          (j = 2 * P + 1 - ((word0(&rv) & Exp_mask) >> Exp_shift)) > 0) {
        /* scaled rv is denormal; zap j low bits */
        if (j >= 32) {
          word1(&rv) = 0;
          if (j >= 53)
            word0(&rv) = (P + 2) * Exp_msk1;
          else
            word0(&rv) &= 0xffffffff << (j - 32);
        } else
          word1(&rv) &= 0xffffffff << j;
      }
      if (!dval(&rv)) {
      undfl:
        dval(&rv) = 0.;
        if (bd0) goto retfree;
        goto ret;
      }
    }
  }

  /* Now the hard part -- adjusting rv to the correct value.*/

  /* Put digits into bd: true value = bd * 10^e */

  bd0 = s2b(s0, nd0, nd, y, &alloc);

  for (;;) {
    bd = Balloc(bd0->k, &alloc);
    Bcopy(bd, bd0);
    bb = d2b(&rv, &bbe, &bbbits, &alloc); /* rv = bb * 2^bbe */
    bs = i2b(1, &alloc);

    if (e >= 0) {
      bb2 = bb5 = 0;
      bd2 = bd5 = e;
    } else {
      bb2 = bb5 = -e;
      bd2 = bd5 = 0;
    }
    if (bbe >= 0)
      bb2 += bbe;
    else
      bd2 -= bbe;
    bs2 = bb2;
#ifdef Honor_FLT_ROUNDS
    if (rounding != 1) bs2++;
#endif
    j = bbe - scale;
    i = j + bbbits - 1; /* logb(rv) */
    if (i < Emin)       /* denormal */
      j += P - Emin;
    else
      j = P + 1 - bbbits;
    bb2 += j;
    bd2 += j;
    bd2 += scale;
    i = bb2 < bd2 ? bb2 : bd2;
    if (i > bs2) i = bs2;
    if (i > 0) {
      bb2 -= i;
      bd2 -= i;
      bs2 -= i;
    }
    if (bb5 > 0) {
      bs = pow5mult(bs, bb5, &alloc);
      bb1 = mult(bs, bb, &alloc);
      Bfree(bb, &alloc);
      bb = bb1;
    }
    if (bb2 > 0) bb = lshift(bb, bb2, &alloc);
    if (bd5 > 0) bd = pow5mult(bd, bd5, &alloc);
    if (bd2 > 0) bd = lshift(bd, bd2, &alloc);
    if (bs2 > 0) bs = lshift(bs, bs2, &alloc);
    delta = diff(bb, bd, &alloc);
    dsign = delta->sign;
    delta->sign = 0;
    i = cmp(delta, bs);
#ifdef Honor_FLT_ROUNDS
    if (rounding != 1) {
      if (i < 0) {
        /* Error is less than an ulp */
        if (!delta->p.x[0] && delta->wds <= 1) {
          /* exact */
          break;
        }
        if (rounding) {
          if (dsign) {
            adj.d = 1.;
            goto apply_adj;
          }
        } else if (!dsign) {
          adj.d = -1.;
          if (!word1(&rv) && !(word0(&rv) & Frac_mask)) {
            y = word0(&rv) & Exp_mask;
            if (!scale || y > 2 * P * Exp_msk1) {
              delta = lshift(delta, Log2P, &alloc);
              if (cmp(delta, bs) <= 0) adj.d = -0.5;
            }
          }
        apply_adj:
          if (scale && (y = word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1)
            word0(&adj) += (2 * P + 1) * Exp_msk1 - y;
          dval(&rv) += adj.d * ulp(&rv);
        }
        break;
      }
      adj.d = ratio(delta, bs);
      if (adj.d < 1.) adj.d = 1.;
      if (adj.d <= 0x7ffffffe) {
        /* adj = rounding ? ceil(adj) : floor(adj); */
        y = adj.d;
        if (y != adj.d) {
          if (!((rounding >> 1) ^ dsign)) y++;
          adj.d = y;
        }
      }
      if (scale && (y = word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1)
        word0(&adj) += (2 * P + 1) * Exp_msk1 - y;
      adj.d *= ulp(&rv);
      if (dsign)
        dval(&rv) += adj.d;
      else
        dval(&rv) -= adj.d;
      goto cont;
    }
#endif /*Honor_FLT_ROUNDS*/

    if (i < 0) {
      /*
        Error is less than half an ulp -- check for special case of mantissa
        a power of two.
      */
      if (dsign || word1(&rv) || word0(&rv) & Bndry_mask ||
          (word0(&rv) & Exp_mask) <= (2 * P + 1) * Exp_msk1) {
        break;
      }
      if (!delta->p.x[0] && delta->wds <= 1) {
        /* exact result */
        break;
      }
      delta = lshift(delta, Log2P, &alloc);
      if (cmp(delta, bs) > 0) goto drop_down;
      break;
    }
    if (i == 0) {
      /* exactly half-way between */
      if (dsign) {
        if ((word0(&rv) & Bndry_mask1) == Bndry_mask1 &&
            word1(&rv) ==
                ((scale && (y = word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1)
                     ? (0xffffffff &
                        (0xffffffff << (2 * P + 1 - (y >> Exp_shift))))
                     : 0xffffffff)) {
          /*boundary case -- increment exponent*/
          word0(&rv) = (word0(&rv) & Exp_mask) + Exp_msk1;
          word1(&rv) = 0;
          dsign = 0;
          break;
        }
      } else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
      drop_down:
        /* boundary case -- decrement exponent */
        if (scale) {
          L = word0(&rv) & Exp_mask;
          if (L <= (2 * P + 1) * Exp_msk1) {
            if (L > (P + 2) * Exp_msk1) /* round even ==> accept rv */
              break;
            /* rv = smallest denormal */
            goto undfl;
          }
        }
        L = (word0(&rv) & Exp_mask) - Exp_msk1;
        word0(&rv) = L | Bndry_mask1;
        word1(&rv) = 0xffffffff;
        break;
      }
      if (!(word1(&rv) & LSB)) break;
      if (dsign)
        dval(&rv) += ulp(&rv);
      else {
        dval(&rv) -= ulp(&rv);
        if (!dval(&rv)) goto undfl;
      }
      dsign = 1 - dsign;
      break;
    }
    if ((aadj = ratio(delta, bs)) <= 2.) {
      if (dsign)
        aadj = aadj1 = 1.;
      else if (word1(&rv) || word0(&rv) & Bndry_mask) {
        if (word1(&rv) == Tiny1 && !word0(&rv)) goto undfl;
        aadj = 1.;
        aadj1 = -1.;
      } else {
        /* special case -- power of FLT_RADIX to be rounded down... */
        if (aadj < 2. / FLT_RADIX)
          aadj = 1. / FLT_RADIX;
        else
          aadj *= 0.5;
        aadj1 = -aadj;
      }
    } else {
      aadj *= 0.5;
      aadj1 = dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
      switch (Rounding) {
        case 2: /* towards +infinity */
          aadj1 -= 0.5;
          break;
        case 0: /* towards 0 */
        case 3: /* towards -infinity */
          aadj1 += 0.5;
      }
#else
      if (Flt_Rounds == 0) aadj1 += 0.5;
#endif /*Check_FLT_ROUNDS*/
    }
    y = word0(&rv) & Exp_mask;

    /* Check for overflow */

    if (y == Exp_msk1 * (DBL_MAX_EXP + Bias - 1)) {
      dval(&rv0) = dval(&rv);
      word0(&rv) -= P * Exp_msk1;
      adj.d = aadj1 * ulp(&rv);
      dval(&rv) += adj.d;
      if ((word0(&rv) & Exp_mask) >= Exp_msk1 * (DBL_MAX_EXP + Bias - P)) {
        if (word0(&rv0) == Big0 && word1(&rv0) == Big1) goto ovfl;
        word0(&rv) = Big0;
        word1(&rv) = Big1;
        goto cont;
      } else
        word0(&rv) += P * Exp_msk1;
    } else {
      if (scale && y <= 2 * P * Exp_msk1) {
        if (aadj <= 0x7fffffff) {
          if ((z = (ULong)aadj) <= 0) z = 1;
          aadj = z;
          aadj1 = dsign ? aadj : -aadj;
        }
        dval(&aadj2) = aadj1;
        word0(&aadj2) += (2 * P + 1) * Exp_msk1 - y;
        aadj1 = dval(&aadj2);
        adj.d = aadj1 * ulp(&rv);
        dval(&rv) += adj.d;
        if (rv.d == 0.) goto undfl;
      } else {
        adj.d = aadj1 * ulp(&rv);
        dval(&rv) += adj.d;
      }
    }
    z = word0(&rv) & Exp_mask;
    if (!scale)
      if (y == z) {
        /* Can we stop now? */
        L = (Long)aadj;
        aadj -= L;
        /* The tolerances below are conservative. */
        if (dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
          if (aadj < .4999999 || aadj > .5000001) break;
        } else if (aadj < .4999999 / FLT_RADIX)
          break;
      }
  cont:
    Bfree(bb, &alloc);
    Bfree(bd, &alloc);
    Bfree(bs, &alloc);
    Bfree(delta, &alloc);
  }
  if (scale) {
    word0(&rv0) = Exp_1 - 2 * P * Exp_msk1;
    word1(&rv0) = 0;
    dval(&rv) *= dval(&rv0);
  }
retfree:
  Bfree(bb, &alloc);
  Bfree(bd, &alloc);
  Bfree(bs, &alloc);
  Bfree(bd0, &alloc);
  Bfree(delta, &alloc);
ret:
  *se = s;
  return sign ? -dval(&rv) : dval(&rv);
}

static int quorem(Bigint *b, Bigint *S) {
  int n;
  ULong *bx, *bxe, q, *sx, *sxe;
  ULLong borrow, carry, y, ys;

  n = S->wds;
  if (b->wds < n) return 0;
  sx = S->p.x;
  sxe = sx + --n;
  bx = b->p.x;
  bxe = bx + n;
  q = *bxe / (*sxe + 1); /* ensure q <= true quotient */
  if (q) {
    borrow = 0;
    carry = 0;
    do {
      ys = *sx++ * (ULLong)q + carry;
      carry = ys >> 32;
      y = *bx - (ys & FFFFFFFF) - borrow;
      borrow = y >> 32 & (ULong)1;
      *bx++ = (ULong)(y & FFFFFFFF);
    } while (sx <= sxe);
    if (!*bxe) {
      bx = b->p.x;
      while (--bxe > bx && !*bxe) --n;
      b->wds = n;
    }
  }
  if (cmp(b, S) >= 0) {
    q++;
    borrow = 0;
    carry = 0;
    bx = b->p.x;
    sx = S->p.x;
    do {
      ys = *sx++ + carry;
      carry = ys >> 32;
      y = *bx - (ys & FFFFFFFF) - borrow;
      borrow = y >> 32 & (ULong)1;
      *bx++ = (ULong)(y & FFFFFFFF);
    } while (sx <= sxe);
    bx = b->p.x;
    bxe = bx + n;
    if (!*bxe) {
      while (--bxe > bx && !*bxe) --n;
      b->wds = n;
    }
  }
  return q;
}

/*
   dtoa for IEEE arithmetic (dmg): convert double to ASCII string.

   Inspired by "How to Print Floating-Point Numbers Accurately" by
   Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 112-126].

   Modifications:
        1. Rather than iterating, we use a simple numeric overestimate
           to determine k= floor(log10(d)).  We scale relevant
           quantities using O(log2(k)) rather than O(k) multiplications.
        2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
           try to generate digits strictly left to right.  Instead, we
           compute with fewer bits and propagate the carry if necessary
           when rounding the final digit up.  This is often faster.
        3. Under the assumption that input will be rounded nearest,
           mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
           That is, we allow equality in stopping tests when the
           round-nearest rule will give the same floating-point value
           as would satisfaction of the stopping test with strict
           inequality.
        4. We remove common factors of powers of 2 from relevant
           quantities.
        5. When converting floating-point integers less than 1e16,
           we use floating-point arithmetic rather than resorting
           to multiple-precision integers.
        6. When asked to produce fewer than 15 digits, we first try
           to get by with floating-point arithmetic; we resort to
           multiple-precision integer arithmetic only if we cannot
           guarantee that the floating-point calculation has given
           the correctly rounded result.  For k requested digits and
           "uniformly" distributed input, the probability is
           something like 10^(k-15) that we must resort to the Long
           calculation.
 */

static char *dtoa(double dd, int mode, int ndigits, int *decpt, int *sign,
                  char **rve, char *buf, size_t buf_size) {
  /*
    Arguments ndigits, decpt, sign are similar to those
    of ecvt and fcvt; trailing zeros are suppressed from
    the returned string.  If not null, *rve is set to point
    to the end of the return value.  If d is +-Infinity or NaN,
    then *decpt is set to DTOA_OVERFLOW.

    mode:
          0 ==> shortest string that yields d when read in
                and rounded to nearest.
          1 ==> like 0, but with Steele & White stopping rule;
                e.g. with IEEE P754 arithmetic , mode 0 gives
                1e23 whereas mode 1 gives 9.999999999999999e22.
          2 ==> max(1,ndigits) significant digits.  This gives a
                return value similar to that of ecvt, except
                that trailing zeros are suppressed.
          3 ==> through ndigits past the decimal point.  This
                gives a return value similar to that from fcvt,
                except that trailing zeros are suppressed, and
                ndigits can be negative.
          4,5 ==> similar to 2 and 3, respectively, but (in
                round-nearest mode) with the tests of mode 0 to
                possibly return a shorter string that rounds to d.
                With IEEE arithmetic and compilation with
                -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
                as modes 2 and 3 when FLT_ROUNDS != 1.
          6-9 ==> Debugging modes similar to mode - 4:  don't try
                fast floating-point estimate (if applicable).

      Values of mode other than 0-9 are treated as mode 0.

    Sufficient space is allocated to the return value
    to hold the suppressed trailing zeros.
  */

  int bbits, b2, b5, be, dig, i, ieps, ilim = 0, ilim0, ilim1 = 0, j, j1, k, k0,
                                       k_check, leftright, m2, m5, s2, s5,
                                       spec_case, try_quick;
  Long L;
  int denorm;
  ULong x;
  Bigint *b, *b1, *delta, *mlo = nullptr, *mhi, *S;
  U d2, eps, u;
  double ds;
  char *s, *s0;
#ifdef Honor_FLT_ROUNDS
  int rounding;
#endif
  Stack_alloc alloc;

  alloc.begin = alloc.free = buf;
  alloc.end = buf + buf_size;
  memset(alloc.freelist, 0, sizeof(alloc.freelist));

  u.d = dd;
  if (word0(&u) & Sign_bit) {
    /* set sign for everything, including 0's and NaNs */
    *sign = 1;
    word0(&u) &= ~Sign_bit; /* clear sign bit */
  } else
    *sign = 0;

  /* If infinity, set decpt to DTOA_OVERFLOW, if 0 set it to 1 */
  if (((word0(&u) & Exp_mask) == Exp_mask && (*decpt = DTOA_OVERFLOW)) ||
      (!dval(&u) && (*decpt = 1))) {
    /* Infinity, NaN, 0 */
    char *res = (char *)dtoa_alloc(2, &alloc);
    res[0] = '0';
    res[1] = '\0';
    if (rve) *rve = res + 1;
    return res;
  }

#ifdef Honor_FLT_ROUNDS
  if ((rounding = Flt_Rounds) >= 2) {
    if (*sign)
      rounding = rounding == 2 ? 0 : 2;
    else if (rounding != 2)
      rounding = 0;
  }
#endif

  b = d2b(&u, &be, &bbits, &alloc);
  if ((i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask >> Exp_shift1)))) {
    dval(&d2) = dval(&u);
    word0(&d2) &= Frac_mask1;
    word0(&d2) |= Exp_11;

    /*
      log(x)       ~=~ log(1.5) + (x-1.5)/1.5
      log10(x)      =  log(x) / log(10)
                   ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
      log10(d)= (i-Bias)*log(2)/log(10) + log10(d2)

      This suggests computing an approximation k to log10(d) by

      k= (i - Bias)*0.301029995663981
           + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );

      We want k to be too large rather than too small.
      The error in the first-order Taylor series approximation
      is in our favor, so we just round up the constant enough
      to compensate for any error in the multiplication of
      (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
      and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
      adding 1e-13 to the constant term more than suffices.
      Hence we adjust the constant term to 0.1760912590558.
      (We could get a more accurate k by invoking log10,
       but this is probably not worthwhile.)
    */

    i -= Bias;
    denorm = 0;
  } else {
    /* d is denormalized */

    i = bbits + be + (Bias + (P - 1) - 1);
    x = i > 32 ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
               : word1(&u) << (32 - i);
    dval(&d2) = x;
    word0(&d2) -= 31 * Exp_msk1; /* adjust exponent */
    i -= (Bias + (P - 1) - 1) + 1;
    denorm = 1;
  }
  ds = (dval(&d2) - 1.5) * 0.289529654602168 + 0.1760912590558 +
       i * 0.301029995663981;
  k = (int)ds;
  if (ds < 0. && ds != k) k--; /* want k= floor(ds) */
  k_check = 1;
  if (k >= 0 && k <= Ten_pmax) {
    if (dval(&u) < tens[k]) k--;
    k_check = 0;
  }
  j = bbits - i - 1;
  if (j >= 0) {
    b2 = 0;
    s2 = j;
  } else {
    b2 = -j;
    s2 = 0;
  }
  if (k >= 0) {
    b5 = 0;
    s5 = k;
    s2 += k;
  } else {
    b2 -= k;
    b5 = -k;
    s5 = 0;
  }
  if (mode < 0 || mode > 9) mode = 0;

#ifdef Check_FLT_ROUNDS
  try_quick = Rounding == 1;
#else
  try_quick = 1;
#endif

  if (mode > 5) {
    mode -= 4;
    try_quick = 0;
  }
  leftright = 1;
  switch (mode) {
    case 0:
    case 1:
      ilim = ilim1 = -1;
      i = 18;
      ndigits = 0;
      break;
    case 2:
      leftright = 0;
      // Fall through.
    case 4:
      if (ndigits <= 0) ndigits = 1;
      ilim = ilim1 = i = ndigits;
      break;
    case 3:
      leftright = 0;
      // Fall through.
    case 5:
      i = ndigits + k + 1;
      ilim = i;
      ilim1 = i - 1;
      if (i <= 0) i = 1;
  }
  s = s0 = dtoa_alloc(i, &alloc);

#ifdef Honor_FLT_ROUNDS
  if (mode > 1 && rounding != 1) leftright = 0;
#endif

  if (ilim >= 0 && ilim <= Quick_max && try_quick) {
    /* Try to get by with floating-point arithmetic. */
    i = 0;
    dval(&d2) = dval(&u);
    k0 = k;
    ilim0 = ilim;
    ieps = 2; /* conservative */
    if (k > 0) {
      ds = tens[k & 0xf];
      j = k >> 4;
      if (j & Bletch) {
        /* prevent overflows */
        j &= Bletch - 1;
        dval(&u) /= bigtens[n_bigtens - 1];
        ieps++;
      }
      for (; j; j >>= 1, i++) {
        if (j & 1) {
          ieps++;
          ds *= bigtens[i];
        }
      }
      dval(&u) /= ds;
    } else if ((j1 = -k)) {
      dval(&u) *= tens[j1 & 0xf];
      for (j = j1 >> 4; j; j >>= 1, i++) {
        if (j & 1) {
          ieps++;
          dval(&u) *= bigtens[i];
        }
      }
    }
    if (k_check && dval(&u) < 1. && ilim > 0) {
      if (ilim1 <= 0) goto fast_failed;
      ilim = ilim1;
      k--;
      dval(&u) *= 10.;
      ieps++;
    }
    dval(&eps) = ieps * dval(&u) + 7.;
    word0(&eps) -= (P - 1) * Exp_msk1;
    if (ilim == 0) {
      S = mhi = nullptr;
      dval(&u) -= 5.;
      if (dval(&u) > dval(&eps)) goto one_digit;
      if (dval(&u) < -dval(&eps)) goto no_digits;
      goto fast_failed;
    }
    if (leftright) {
      /* Use Steele & White method of only generating digits needed. */
      dval(&eps) = 0.5 / tens[ilim - 1] - dval(&eps);
      for (i = 0;;) {
        L = (Long)dval(&u);
        dval(&u) -= L;
        *s++ = '0' + (int)L;
        if (dval(&u) < dval(&eps)) goto ret1;
        if (1. - dval(&u) < dval(&eps)) goto bump_up;
        if (++i >= ilim) break;
        dval(&eps) *= 10.;
        dval(&u) *= 10.;
      }
    } else {
      /* Generate ilim digits, then fix them up. */
      dval(&eps) *= tens[ilim - 1];
      for (i = 1;; i++, dval(&u) *= 10.) {
        L = (Long)(dval(&u));
        if (!(dval(&u) -= L)) ilim = i;
        *s++ = '0' + (int)L;
        if (i == ilim) {
          if (dval(&u) > 0.5 + dval(&eps))
            goto bump_up;
          else if (dval(&u) < 0.5 - dval(&eps)) {
            while (*--s == '0')
              ;
            s++;
            goto ret1;
          }
          break;
        }
      }
    }
  fast_failed:
    s = s0;
    dval(&u) = dval(&d2);
    k = k0;
    ilim = ilim0;
  }

  /* Do we have a "small" integer? */

  if (be >= 0 && k <= Int_max) {
    /* Yes. */
    ds = tens[k];
    if (ndigits < 0 && ilim <= 0) {
      S = mhi = nullptr;
      if (ilim < 0 || dval(&u) <= 5 * ds) goto no_digits;
      goto one_digit;
    }
    for (i = 1;; i++, dval(&u) *= 10.) {
      L = (Long)(dval(&u) / ds);
      dval(&u) -= L * ds;
#ifdef Check_FLT_ROUNDS
      /* If FLT_ROUNDS == 2, L will usually be high by 1 */
      if (dval(&u) < 0) {
        L--;
        dval(&u) += ds;
      }
#endif
      *s++ = '0' + (int)L;
      if (!dval(&u)) {
        break;
      }
      if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
        if (mode > 1) {
          switch (rounding) {
            case 0:
              goto ret1;
            case 2:
              goto bump_up;
          }
        }
#endif
        dval(&u) += dval(&u);
        if (dval(&u) > ds || (dval(&u) == ds && L & 1)) {
        bump_up:
          while (*--s == '9')
            if (s == s0) {
              k++;
              *s = '0';
              break;
            }
          ++*s++;
        }
        break;
      }
    }
    goto ret1;
  }

  m2 = b2;
  m5 = b5;
  mhi = mlo = nullptr;
  if (leftright) {
    i = denorm ? be + (Bias + (P - 1) - 1 + 1) : 1 + P - bbits;
    b2 += i;
    s2 += i;
    mhi = i2b(1, &alloc);
  }
  if (m2 > 0 && s2 > 0) {
    i = m2 < s2 ? m2 : s2;
    b2 -= i;
    m2 -= i;
    s2 -= i;
  }
  if (b5 > 0) {
    if (leftright) {
      if (m5 > 0) {
        mhi = pow5mult(mhi, m5, &alloc);
        b1 = mult(mhi, b, &alloc);
        Bfree(b, &alloc);
        b = b1;
      }
      if ((j = b5 - m5)) b = pow5mult(b, j, &alloc);
    } else
      b = pow5mult(b, b5, &alloc);
  }
  S = i2b(1, &alloc);
  if (s5 > 0) S = pow5mult(S, s5, &alloc);

  /* Check for special case that d is a normalized power of 2. */

  spec_case = 0;
  if ((mode < 2 || leftright)
#ifdef Honor_FLT_ROUNDS
      && rounding == 1
#endif
  ) {
    if (!word1(&u) && !(word0(&u) & Bndry_mask) &&
        word0(&u) & (Exp_mask & ~Exp_msk1)) {
      /* The special case */
      b2 += Log2P;
      s2 += Log2P;
      spec_case = 1;
    }
  }

  /*
    Arrange for convenient computation of quotients:
    shift left if necessary so divisor has 4 leading 0 bits.

    Perhaps we should just compute leading 28 bits of S once
    a nd for all and pass them and a shift to quorem, so it
    can do shifts and ors to compute the numerator for q.
  */
  if ((i = ((s5 ? 32 - hi0bits(S->p.x[S->wds - 1]) : 1) + s2) & 0x1f))
    i = 32 - i;
  if (i > 4) {
    i -= 4;
    b2 += i;
    m2 += i;
    s2 += i;
  } else if (i < 4) {
    i += 28;
    b2 += i;
    m2 += i;
    s2 += i;
  }
  if (b2 > 0) b = lshift(b, b2, &alloc);
  if (s2 > 0) S = lshift(S, s2, &alloc);
  if (k_check) {
    if (cmp(b, S) < 0) {
      k--;
      /* we botched the k estimate */
      b = multadd(b, 10, 0, &alloc);
      if (leftright) mhi = multadd(mhi, 10, 0, &alloc);
      ilim = ilim1;
    }
  }
  if (ilim <= 0 && (mode == 3 || mode == 5)) {
    if (ilim < 0 || cmp(b, S = multadd(S, 5, 0, &alloc)) <= 0) {
      /* no digits, fcvt style */
    no_digits:
      k = -1 - ndigits;
      goto ret;
    }
  one_digit:
    *s++ = '1';
    k++;
    goto ret;
  }
  if (leftright) {
    if (m2 > 0) mhi = lshift(mhi, m2, &alloc);

    /*
      Compute mlo -- check for special case that d is a normalized power of 2.
    */

    mlo = mhi;
    if (spec_case) {
      mhi = Balloc(mhi->k, &alloc);
      Bcopy(mhi, mlo);
      mhi = lshift(mhi, Log2P, &alloc);
    }

    for (i = 1;; i++) {
      dig = quorem(b, S) + '0';
      /* Do we yet have the shortest decimal string that will round to d? */
      j = cmp(b, mlo);
      delta = diff(S, mhi, &alloc);
      j1 = delta->sign ? 1 : cmp(b, delta);
      Bfree(delta, &alloc);
      if (j1 == 0 && mode != 1 && !(word1(&u) & 1)
#ifdef Honor_FLT_ROUNDS
          && rounding >= 1
#endif
      ) {
        if (dig == '9') goto round_9_up;
        if (j > 0) dig++;
        *s++ = dig;
        goto ret;
      }
      if (j < 0 || (j == 0 && mode != 1 && !(word1(&u) & 1))) {
        if (!b->p.x[0] && b->wds <= 1) {
          goto accept_dig;
        }
#ifdef Honor_FLT_ROUNDS
        if (mode > 1) switch (rounding) {
            case 0:
              goto accept_dig;
            case 2:
              goto keep_dig;
          }
#endif /*Honor_FLT_ROUNDS*/
        if (j1 > 0) {
          b = lshift(b, 1, &alloc);
          j1 = cmp(b, S);
          if ((j1 > 0 || (j1 == 0 && dig & 1)) && dig++ == '9') goto round_9_up;
        }
      accept_dig:
        *s++ = dig;
        goto ret;
      }
      if (j1 > 0) {
#ifdef Honor_FLT_ROUNDS
        if (!rounding) goto accept_dig;
#endif
        if (dig == '9') { /* possible if i == 1 */
        round_9_up:
          *s++ = '9';
          goto roundoff;
        }
        *s++ = dig + 1;
        goto ret;
      }
#ifdef Honor_FLT_ROUNDS
    keep_dig:
#endif
      *s++ = dig;
      if (i == ilim) break;
      b = multadd(b, 10, 0, &alloc);
      if (mlo == mhi)
        mlo = mhi = multadd(mhi, 10, 0, &alloc);
      else {
        mlo = multadd(mlo, 10, 0, &alloc);
        mhi = multadd(mhi, 10, 0, &alloc);
      }
    }
  } else
    for (i = 1;; i++) {
      *s++ = dig = quorem(b, S) + '0';
      if (!b->p.x[0] && b->wds <= 1) {
        goto ret;
      }
      if (i >= ilim) break;
      b = multadd(b, 10, 0, &alloc);
    }

    /* Round off last digit */

#ifdef Honor_FLT_ROUNDS
  switch (rounding) {
    case 0:
      goto trimzeros;
    case 2:
      goto roundoff;
  }
#endif
  b = lshift(b, 1, &alloc);
  j = cmp(b, S);
  if (j > 0 || (j == 0 && dig & 1)) {
  roundoff:
    while (*--s == '9')
      if (s == s0) {
        k++;
        *s++ = '1';
        goto ret;
      }
    ++*s++;
  } else {
#ifdef Honor_FLT_ROUNDS
  trimzeros:
#endif
    while (*--s == '0')
      ;
    s++;
  }
ret:
  Bfree(S, &alloc);
  if (mhi) {
    if (mlo && mlo != mhi) Bfree(mlo, &alloc);
    Bfree(mhi, &alloc);
  }
ret1:
  Bfree(b, &alloc);
  *s = 0;
  *decpt = k + 1;
  if (rve) *rve = s;
  return s0;
}
