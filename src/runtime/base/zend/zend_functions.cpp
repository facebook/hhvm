/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_strtod.h>

namespace HPHP {

#define SIZEOF_LONG 8
#define MAX_LENGTH_OF_LONG 20
static const char long_min_digits[] = "9223372036854775808";

#define IS_DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define IS_XDIGIT(c) (((c) >= 'A' && (c) <= 'F')||((c) >= 'a'  && (c) <= 'f'))

///////////////////////////////////////////////////////////////////////////////

DataType is_numeric_string(const char *str, int length, int64 *lval,
                           double *dval, int allow_errors /* = 1 */) {
  DataType type;
  const char *ptr;
  int base = 10, digits = 0, dp_or_e = 0;
  double local_dval = 0.0;

  if (!length) {
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
        local_dval = zend_strtod(str, (char **)&ptr);
      }
      type = KindOfDouble;
    }
  } else if (*ptr == '.' && IS_DIGIT(ptr[1])) {
  process_double:
    type = KindOfDouble;

    /* If there's a dval, do the conversion; else continue checking
     * the digits if we need to check for a full match */
    if (dval) {
      local_dval = strtod(str, (char **)&ptr);
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
          *dval = strtod(str, NULL);
        }
        return KindOfDouble;
      }
    }
    if (lval) {
      *lval = strtol(str, NULL, base);
    }
    return KindOfInt64;
  }

  if (dval) {
    *dval = local_dval;
  }
  return KindOfDouble;
}

#define LOWER_CASE 1
#define UPPER_CASE 2
#define NUMERIC 3

char *increment_string(char *s, int len) {
  ASSERT(s && *s);

  int carry=0;
  int pos=len-1;
  int last=0; /* Shut up the compiler warning */
  int ch;

  while (pos >= 0) {
    ch = s[pos];
    if (ch >= 'a' && ch <= 'z') {
      if (ch == 'z') {
        s[pos] = 'a';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last=LOWER_CASE;
    } else if (ch >= 'A' && ch <= 'Z') {
      if (ch == 'Z') {
        s[pos] = 'A';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last=UPPER_CASE;
    } else if (ch >= '0' && ch <= '9') {
      if (ch == '9') {
        s[pos] = '0';
        carry=1;
      } else {
        s[pos]++;
        carry=0;
      }
      last = NUMERIC;
    } else {
      carry=0;
      break;
    }
    if (carry == 0) {
      break;
    }
    pos--;
  }

  if (carry) {
    char *t = (char *) malloc(len+1+1);
    memcpy(t+1, s, len);
    t[++len] = '\0';
    switch (last) {
    case NUMERIC:
      t[0] = '1';
      break;
    case UPPER_CASE:
      t[0] = 'A';
      break;
    case LOWER_CASE:
      t[0] = 'a';
      break;
    }
    return t;
  }
  return NULL;
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
