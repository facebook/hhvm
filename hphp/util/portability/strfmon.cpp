// Borrowed from Musl Libc, under the following license:
/*
* Copyright (c) 2005-2014 Rich Felker, et al.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "hphp/util/portability/strfmon.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>

static ssize_t vstrfmon_l(char *s,
                          size_t n,
                          locale_t loc,
                          const char *fmt,
                          va_list ap) {
  size_t l;
  double x;
  int fill, nogrp, negpar, nosym, left, intl;
  int lp, rp, w, fw;
  char *s0 = s;
  for (; n && *fmt; ) {
    if (*fmt != '%') {
      literal:
      *s++ = *fmt++;
      n--;
      continue;
    }
    fmt++;
    if (*fmt == '%') goto literal;

    fill = ' ';
    nogrp = 0;
    negpar = 0;
    nosym = 0;
    left = 0;
    for (; ; fmt++) {
      switch (*fmt) {
        case '=':
          fill = *++fmt;
          continue;
        case '^':
          nogrp = 1;
          continue;
        case '(':
          negpar = 1;
        case '+':
          continue;
        case '!':
          nosym = 1;
          continue;
        case '-':
          left = 1;
          continue;
      }
      break;
    }

    for (fw = 0; isdigit(*fmt); fmt++)
      fw = 10 * fw + (*fmt - '0');
    lp = 0;
    rp = 2;
    if (*fmt == '#') for (lp = 0, fmt++; isdigit(*fmt); fmt++)
      lp = 10 * lp + (*fmt - '0');
    if (*fmt == '.') for (rp = 0, fmt++; isdigit(*fmt); fmt++)
      rp = 10 * rp + (*fmt - '0');

    intl = *fmt++ == 'i';

    w = lp + 1 + rp;
    if (!left && fw>w) w = fw;

    x = va_arg(ap, double);
    l = snprintf(s, n, "%*.*f", w, rp, x);
    if (l >= n) {
      errno = E2BIG;
      return -1;
    }
    s += l;
    n -= l;
  }
  return s - s0;
}

extern "C" ssize_t strfmon_l(char* __restrict s,
                             size_t n,
                             locale_t loc,
                             const char* __restrict fmt,
                             ...) {
  va_list ap;
  ssize_t ret;

  va_start(ap, fmt);
  ret = vstrfmon_l(s, n, loc, fmt, ap);
  va_end(ap);

  return ret;
}


extern "C" ssize_t strfmon(char* __restrict s,
                           size_t n,
                           const char* __restrict fmt,
                           ...) {
  va_list ap;
  ssize_t ret;

  va_start(ap, fmt);
  ret = vstrfmon_l(s, n, _current_locale(), fmt, ap);
  va_end(ap);

  return ret;
}
