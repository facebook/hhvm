/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

// NOTE: See also "hphp/zend/zend-printf.*".

#include "hphp/runtime/base/zend-printf.h"

#include <cmath>

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {

/* These definitions are coped from the Zend formatted output conversion
   files so that we only need to make minimal changes to the Zend formatted
   output conversion functions that are incorporated here.
 */
#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ADJ_WIDTH 1
#define ADJ_PRECISION 2
#define FLOAT_PRECISION 6
#define MAX_FLOAT_DIGITS 38
#define MAX_FLOAT_PRECISION 40

static char hexchars[] = "0123456789abcdef";
static char HEXCHARS[] = "0123456789ABCDEF";

#define HAVE_LOCALE_H 1

#ifdef HAVE_LOCALE_H
} // namespace HPHP

#include <locale.h>

namespace HPHP {
#define LCONV_DECIMAL_POINT (*lconv->decimal_point)
#else
#define LCONV_DECIMAL_POINT '.'
#endif

///////////////////////////////////////////////////////////////////////////////

inline static void appendstring(StringBuffer *buffer, const char *add,
                                int min_width, int max_width, char padding,
                                int alignment, int len, int neg, int expprec,
                                int always_sign) {
  int npad;
  int req_size;
  int copy_len;

  copy_len = (expprec ? (max_width < len ? max_width : len) : len);
  npad = min_width - copy_len;

  if (npad < 0) {
    npad = 0;
  }

  req_size = buffer->size() + (min_width > copy_len ? min_width : copy_len);

  buffer->appendCursor(req_size);
  if (alignment == ALIGN_RIGHT) {
    if ((neg || always_sign) && padding=='0') {
      buffer->append((neg) ? '-' : '+');
      add++;
      len--;
      copy_len--;
    }
    while (npad-- > 0) {
      buffer->append(padding);
    }
  }
  buffer->append(add, copy_len);
  if (alignment == ALIGN_LEFT) {
    while (npad--) {
      buffer->append(padding);
    }
  }
}

inline static void appendint(StringBuffer *buffer, long number,
                             int width, char padding, int alignment,
                             int always_sign) {
  char numbuf[NUM_BUF_SIZE];
  unsigned long magn, nmagn;
  unsigned int i = NUM_BUF_SIZE - 1, neg = 0;

  if (number < 0) {
    neg = 1;
    magn = ((unsigned long) -(number + 1)) + 1;
  } else {
    magn = (unsigned long) number;
  }

  /* Can't right-pad 0's on integers */
  if (alignment==0 && padding=='0') padding=' ';

  numbuf[i] = '\0';

  do {
    nmagn = magn / 10;

    numbuf[--i] = (unsigned char)(magn - (nmagn * 10)) + '0';
    magn = nmagn;
  }
  while (magn > 0 && i > 0);
  if (neg) {
    numbuf[--i] = '-';
  } else if (always_sign) {
    numbuf[--i] = '+';
  }
  appendstring(buffer, &numbuf[i], width, 0,
                       padding, alignment, (NUM_BUF_SIZE - 1) - i,
                       neg, 0, always_sign);
}

inline static void appenduint(StringBuffer *buffer,
                              unsigned long number,
                              int width, char padding, int alignment) {
  char numbuf[NUM_BUF_SIZE];
  unsigned long magn, nmagn;
  unsigned int i = NUM_BUF_SIZE - 1;

  magn = (unsigned long) number;

  /* Can't right-pad 0's on integers */
  if (alignment == 0 && padding == '0') padding = ' ';

  numbuf[i] = '\0';
  do {
    nmagn = magn / 10;
    numbuf[--i] = (unsigned char)(magn - (nmagn * 10)) + '0';
    magn = nmagn;
  } while (magn > 0 && i > 0);

  appendstring(buffer, &numbuf[i], width, 0,
               padding, alignment, (NUM_BUF_SIZE - 1) - i, 0, 0, 0);
}

inline static void appenddouble(StringBuffer *buffer,
                                double number,
                                int width, char padding,
                                int alignment, int precision,
                                int adjust, char fmt,
                                int always_sign) {
  char num_buf[NUM_BUF_SIZE];
  char *s = nullptr;
  int s_len = 0, is_negative = 0;

  if ((adjust & ADJ_PRECISION) == 0) {
    precision = FLOAT_PRECISION;
  } else if (precision > MAX_FLOAT_PRECISION) {
    precision = MAX_FLOAT_PRECISION;
  }

  if (std::isnan(number)) {
    is_negative = (number<0);
    appendstring(buffer, "NaN", 3, 0, padding,
                 alignment, 3, is_negative, 0, always_sign);
    return;
  }

  if (std::isinf(number)) {
    is_negative = (number<0);
    appendstring(buffer, "INF", 3, 0, padding,
                 alignment, 3, is_negative, 0, always_sign);
    return;
  }

  if (g_context->getThrowAllErrors()) {
    raise_notice("depends on locale: do not fold");
  }

#if defined(HAVE_LOCALE_H)
  struct lconv *lconv;
  lconv = localeconv();
# define APPENDDOUBLE_LCONV_DECIMAL_POINT (*lconv->decimal_point)
#else
# define APPENDDOUBLE_LCONV_DECIMAL_POINT '.'
#endif

  switch (fmt) {
  case 'e':
  case 'E':
  case 'f':
  case 'F':
    s = php_conv_fp((fmt == 'f')?'F':fmt,
                    number, 0, precision,
                    (fmt == 'f')?APPENDDOUBLE_LCONV_DECIMAL_POINT:'.',
                    &is_negative, &num_buf[1], &s_len);
    if (is_negative) {
      num_buf[0] = '-';
      s = num_buf;
      s_len++;
    } else if (always_sign) {
      num_buf[0] = '+';
      s = num_buf;
      s_len++;
    }
    break;

  case 'g':
  case 'G':
    if (precision == 0)
      precision = 1;
    /*
     * * We use &num_buf[ 1 ], so that we have room for the sign
     */
    s = php_gcvt(number, precision,
                 APPENDDOUBLE_LCONV_DECIMAL_POINT,
                 (fmt == 'G')?'E':'e',
                 &num_buf[1]);
    is_negative = 0;
    if (*s == '-') {
      is_negative = 1;
      s = &num_buf[1];
    } else if (always_sign) {
      num_buf[0] = '+';
      s = num_buf;
    }

    s_len = strlen(s);
    break;
  }

  appendstring(buffer, s, width, 0, padding,
               alignment, s_len, is_negative, 0, always_sign);
}

inline static void append2n(StringBuffer *buffer, long number,
                            int width, char padding, int alignment, int n,
                            char *chartable, int expprec) {
  char numbuf[NUM_BUF_SIZE];
  unsigned long num;
  unsigned int  i = NUM_BUF_SIZE - 1;
  int andbits = (1 << n) - 1;

  num = (unsigned long) number;
  numbuf[i] = '\0';

  do {
    numbuf[--i] = chartable[(num & andbits)];
    num >>= n;
  }
  while (num > 0);

  appendstring(buffer, &numbuf[i], width, 0,
               padding, alignment, (NUM_BUF_SIZE - 1) - i,
               0, expprec, 0);
}

inline static int getnumber(const char *buffer, int *pos) {
  char *endptr;
  long num = strtol(buffer + *pos, &endptr, 10);
  int i = 0;

  if (endptr != nullptr) {
    i = (endptr - buffer - *pos);
  }
  *pos += i;

  if (num >= INT_MAX || num < 0) {
    return -1;
  }
  return (int) num;
}

/**
 * New sprintf implementation for PHP.
 *
 * Modifiers:
 *
 *  " "   pad integers with spaces
 *  "-"   left adjusted field
 *   n    field size
 *  "."n  precision (floats only)
 *  "+"   Always place a sign (+ or -) in front of a number
 *
 * Type specifiers:
 *
 *  "%"   literal "%", modifiers are ignored.
 *  "b"   integer argument is printed as binary
 *  "c"   integer argument is printed as a single character
 *  "d"   argument is an integer
 *  "f"   the argument is a float
 *  "o"   integer argument is printed as octal
 *  "s"   argument is a string
 *  "x"   integer argument is printed as lowercase hexadecimal
 *  "X"   integer argument is printed as uppercase hexadecimal
 *
 * Warning: The logic for handling the tokens %%, %s in the format string
 * in this function should mimic the implementation in the tokenize function in
 * irgen-builtin.cpp which is invoked in the fast path of certain builtins like
 * Str\format. If you make any changes here, please ensure that the two
 * implementations will be in sync.
 */
String string_printf(const char *format, int len, const Array& args) {
  Array vargs = args;
  if (!vargs.isNull() && !vargs->isVectorData()) {
    vargs = Array::CreateVec();
    for (ArrayIter iter(args); iter; ++iter) {
      vargs.append(iter.second());
    }
  }

  if (len == 0) {
    return empty_string();
  }

  int size = 256 - kStringOverhead;
  StringBuffer result(size);

  int argnum = 0, currarg = 1;
  for (int inpos = 0; inpos < len; ++inpos) {
    char ch = format[inpos];

    int expprec = 0;
    if (ch != '%') {
      result.append(ch);
      continue;
    }

    if (format[inpos + 1] == '%') {
      result.append('%');
      inpos++;
      continue;
    }

    /* starting a new format specifier, reset variables */
    int alignment = ALIGN_RIGHT;
    int adjusting = 0;
    char padding = ' ';
    int always_sign = 0;
    int width, precision;
    inpos++;      /* skip the '%' */
    ch = format[inpos];

    if (isascii(ch) && !isalpha(ch)) {
      /* first look for argnum */
      int temppos = inpos;
      while (isdigit((int)format[temppos])) temppos++;
      if (format[temppos] == '$') {
        argnum = getnumber(format, &inpos);
        if (argnum <= 0) {
          raise_invalid_argument_warning("argnum: must be greater than zero");
          return String();
        }
        inpos++;  /* skip the '$' */
      } else {
        argnum = currarg++;
      }

      /* after argnum comes modifiers */
      for (;; inpos++) {
        ch = format[inpos];

        if (ch == ' ' || ch == '0') {
          padding = ch;
        } else if (ch == '-') {
          alignment = ALIGN_LEFT;
          /* space padding, the default */
        } else if (ch == '+') {
          always_sign = 1;
        } else if (ch == '\'' && inpos != len - 1) {
          padding = format[++inpos];
        } else {
          break;
        }
      }
      ch = format[inpos];

      /* after modifiers comes width */
      if (isdigit(ch)) {
        if ((width = getnumber(format, &inpos)) < 0) {
          raise_invalid_argument_warning("width: must be greater than zero "
                                 "and less than %d", INT_MAX);
          return String();
        }
        adjusting |= ADJ_WIDTH;
      } else {
        width = 0;
      }
      ch = format[inpos];

      /* after width and argnum comes precision */
      if (ch == '.') {
        ch = format[++inpos];
        if (isdigit((int)ch)) {
          if ((precision = getnumber(format, &inpos)) < 0) {
            raise_invalid_argument_warning("precision: must be greater than zero "
                                   "and less than %d", INT_MAX);
            return String();
          }
          ch = format[inpos];
          adjusting |= ADJ_PRECISION;
          expprec = 1;
        } else {
          precision = 0;
        }
      } else {
        precision = 0;
      }
    } else {
      width = precision = 0;
      argnum = currarg++;
    }

    if (argnum > vargs.size()) {
      raise_invalid_argument_warning("arguments: (too few)");
      return String();
    }

    if (ch == 'l') {
      ch = format[++inpos];
    }
    /* now we expect to find a type specifier */
    Variant tmp = vargs[argnum-1];

    switch (ch) {
    case 's': {
      String s = tmp.toString();
      appendstring(&result, s.c_str(),
                   width, precision, padding, alignment, s.size(),
                   0, expprec, 0);
      break;
    }
    case 'd':
      appendint(&result, tmp.toInt64(),
                width, padding, alignment, always_sign);
      break;
    case 'u':
      appenduint(&result, tmp.toInt64(),
                 width, padding, alignment);
      break;

    case 'g':
    case 'G':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
      appenddouble(&result, tmp.toDouble(),
                   width, padding, alignment, precision, adjusting,
                   ch, always_sign);
      break;

    case 'c':
      result.append((char)tmp.toInt64());
      break;

    case 'o':
      append2n(&result, tmp.toInt64(),
               width, padding, alignment, 3, hexchars, expprec);
      break;

    case 'x':
      append2n(&result, tmp.toInt64(),
               width, padding, alignment, 4, hexchars, expprec);
      break;

    case 'X':
      append2n(&result, tmp.toInt64(),
               width, padding, alignment, 4, HEXCHARS, expprec);
      break;

    case 'b':
      append2n(&result, tmp.toInt64(),
               width, padding, alignment, 1, hexchars, expprec);
      break;

    case '%':
      result.append('%');

      break;
    default:
      break;
    }
  }

  return result.detach();
}

}
