/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-strtod.h"

namespace HPHP {

#define SIZEOF_LONG 8
#define MAX_LENGTH_OF_LONG 20
static const char long_min_digits[] = "9223372036854775808";

#undef IS_DIGIT
#define IS_DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define IS_XDIGIT(c) (((c) >= 'A' && (c) <= 'F')||((c) >= 'a'  && (c) <= 'f'))

///////////////////////////////////////////////////////////////////////////////

StringSlice conv_10(int64_t num, char* buf_end) {
  auto p = buf_end;
  uint64_t magnitude;

  /*
   * On a 2's complement machine, negating the most negative integer
   * results in a number that cannot be represented as a signed integer.
   * Here is what we do to obtain the number's magnitude:
   *      a. add 1 to the number
   *      b. negate it (becomes positive)
   *      c. convert it to unsigned
   *      d. add 1
   */
  if (num < 0) {
    magnitude = static_cast<uint64_t>(-(num + 1)) + 1;
  } else {
    magnitude = static_cast<uint64_t>(num);
  }

  /*
   * We use a do-while loop so that we write at least 1 digit
   */
  do {
    auto const q = magnitude / 10;
    auto const r = static_cast<uint32_t>(magnitude % 10);
    *--p = r + '0';
    magnitude = q;
  } while (magnitude);

  if (num < 0) *--p = '-';
  return StringSlice{p, static_cast<uint32_t>(buf_end - p)};
}

DataType is_numeric_string(const char *str, int length, int64_t *lval,
                           double *dval, int allow_errors /* = 0 */) {
  DataType type;
  const char *ptr;
  int base = 10, digits = 0, dp_or_e = 0;
  double local_dval = 0.0;

  if (!length || ((unsigned char)(*str)) > '9') {
    return KindOfNull;
  }

  /* Skip any whitespace
   * This is much faster than the isspace() function */
  while (*str == ' ' ||
         *str == '\t' ||
         *str == '\n' ||
         *str == '\r' ||
         *str == '\v' ||
         *str == '\f') {
    str++;
    length--;
  }
  ptr = str;

  if (*ptr == '-' || *ptr == '+') {
    ptr++;
  }

  if (IS_DIGIT(*ptr)) {
    /* Handle hex numbers
     * str is used instead of ptr to disallow signs and keep old behavior */
    if (length > 2 && *str == '0' && (str[1] == 'x' || str[1] == 'X')) {
      base = 16;
      ptr += 2;
    }

    /* Skip any leading 0s */
    while (*ptr == '0') {
      ptr++;
    }

    /* Count the number of digits. If a decimal point/exponent is found,
     * it's a double. Otherwise, if there's a dval or no need to check for
     * a full match, stop when there are too many digits for a int64 */
    for (type = KindOfInt64;
         !(digits >= MAX_LENGTH_OF_LONG && (dval || allow_errors == 1));
         digits++, ptr++) {
    check_digits:
      if (IS_DIGIT(*ptr) || (base == 16 && IS_XDIGIT(*ptr))) {
        continue;
      } else if (base == 10) {
        if (*ptr == '.' && dp_or_e < 1) {
          goto process_double;
        } else if ((*ptr == 'e' || *ptr == 'E') && dp_or_e < 2) {
          const char *e = ptr + 1;

          if (*e == '-' || *e == '+') {
            ptr = e++;
          }
          if (IS_DIGIT(*e)) {
            goto process_double;
          }
        }
      }

      break;
    }

    if (base == 10) {
      if (digits >= MAX_LENGTH_OF_LONG) {
        dp_or_e = -1;
        goto process_double;
      }
    } else if (!(digits < SIZEOF_LONG * 2 ||
                 (digits == SIZEOF_LONG * 2 && ptr[-digits] <= '7'))) {
      if (dval) {
        local_dval = zend_hex_strtod(str, (const char **)&ptr);
      }
      type = KindOfDouble;
    }
  } else if (*ptr == '.' && IS_DIGIT(ptr[1])) {
  process_double:
    type = KindOfDouble;

    /* If there's a dval, do the conversion; else continue checking
     * the digits if we need to check for a full match */
    if (dval) {
      local_dval = zend_strtod(str, (const char **)&ptr);
    } else if (allow_errors != 1 && dp_or_e != -1) {
      dp_or_e = (*ptr++ == '.') ? 1 : 2;
      goto check_digits;
    }
  } else {
    return KindOfNull;
  }

  if (ptr != str + length) {
    if (!allow_errors) {
      return KindOfNull;
    }
    // if (allow_errors == -1) {
    //   zend_error(E_NOTICE, "A non well formed numeric value encountered");
    // }
  }

  if (type == KindOfInt64) {
    if (digits == MAX_LENGTH_OF_LONG - 1) {
      int cmp = strcmp(&ptr[-digits], long_min_digits);
      if (!(cmp < 0 || (cmp == 0 && *str == '-'))) {
        if (dval) {
          *dval = strtod(str, nullptr);
        }
        return KindOfDouble;
      }
    }
    if (lval) {
      *lval = strtol(str, nullptr, base);
    }
    return KindOfInt64;
  }

  if (dval) {
    *dval = local_dval;
  }
  return KindOfDouble;
}

bool is_valid_var_name(const char *var_name, int len) {
  if (!var_name ||
      (!isalpha((int)((unsigned char *)var_name)[0]) && var_name[0] != '_')) {
    return false;
  }
  for (int i = 1; i < len; i++) {
    if (!isalnum((int)((unsigned char *)var_name)[i]) && var_name[i] != '_') {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
