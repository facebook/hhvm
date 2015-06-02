/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_str.h"
#include "hphp/runtime/ext/xdebug/php5_xdebug/xdebug_mm.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

void xdebug_str_add(xdebug_str *xs, char *str, int f) {
  xdebug_str_addl(xs, str, strlen(str), f);
}

void xdebug_str_addl(xdebug_str *xs, char *str, int le, int f) {
  if (xs->l + le > xs->a - 1) {
    xs->d = (char*) xdrealloc(xs->d, xs->a + le + XDEBUG_STR_PREALLOC);
    xs->a = xs->a + le + XDEBUG_STR_PREALLOC;
  }
  if (!xs->l) {
    xs->d[0] = '\0';
  }
  memcpy(xs->d + xs->l, str, le);
  xs->d[xs->l + le] = '\0';
  xs->l = xs->l + le;

  if (f) {
    xdfree(str);
  }
}

void xdebug_str_chop(xdebug_str *xs, int c) {
  if (c > xs->l) {
    /* Do nothing if the chop amount is larger than the buffer size */
  } else {
    xs->l -= c;
    xs->d[xs->l] = '\0';
  }
}

void xdebug_str_free(xdebug_str *s) {
  if (s->d) {
    xdfree(s->d);
  }
}

char *xdebug_sprintf(const char* fmt, ...) {
  char   *new_str;
  int     size = 1;
  va_list args;

  char* orig_locale = setlocale(LC_ALL, nullptr);
  setlocale(LC_ALL, "C");
  new_str = (char *) xdmalloc(size);
  for (;;) {
    int n;

    va_start(args, fmt);
    n = vsnprintf(new_str, size, fmt, args);
    va_end(args);

    if (n > -1 && n < size) {
      break;
    }
    if (n < 0) {
      size *= 2;
    } else {
      size = n + 1;
    }
    new_str = (char *) xdrealloc(new_str, size);
  }
  setlocale(LC_ALL, orig_locale);
  return new_str;
}

char* xdstrdup(const char* str) {
  if (str == nullptr) {
    return nullptr;
  }

  size_t size = strlen(str) + 1;
  char* dup = (char*) xdmalloc(size);
  memcpy(dup, str, size);
  return dup;
}
