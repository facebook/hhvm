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

#include "hphp/runtime/base/memory-manager.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>

char* xdebug_sprintf(const char* fmt, ...) {
  int size = 1;
  va_list args;

  auto const orig_locale = setlocale(LC_ALL, nullptr);
  setlocale(LC_ALL, "C");
  SCOPE_EXIT { setlocale(LC_ALL, orig_locale); };
  auto new_str = (char*)HPHP::req::malloc_noptrs(size);
  for (;;) {
    va_start(args, fmt);
    auto const n = vsnprintf(new_str, size, fmt, args);
    va_end(args);

    if (n > -1 && n < size) {
      // Success case.
      break;
    } else if (n < 0) {
      // vsnprintf returned an error, free the allocated string and bail.
      if (new_str) {
        HPHP::req::free(new_str);
        new_str = nullptr;
      }
      break;
    } else {
      // vsnprintf indicated the supplied buffer was too small. In this case,
      // the return value of vsnprintf indicates the number of characters
      // the buffer was required to hold, not including the terminating NULL
      // char, so set size = n + 1 (to include enough space for the NULL),
      // reallocate the buffer and try again.
      size = n + 1;
      new_str = (char*)HPHP::req::realloc_noptrs(new_str, size);
    }
  }
  return new_str;
}

char* xdstrdup(const char* str) {
  if (str == nullptr) {
    return nullptr;
  }

  auto const size = strlen(str) + 1;
  auto const dup = (char*)HPHP::req::malloc_noptrs(size);
  memcpy(dup, str, size);
  return dup;
}
