/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/ext/ext_ctype.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(ctype);
///////////////////////////////////////////////////////////////////////////////

static bool ctype(CVarRef v, int (*iswhat)(int)) {
  if (v.isInteger()) {
    int64 n = v.toInt64();
    if (n <= 255 && n >= 0) {
      return iswhat(n);
    }

    if (n >= -128 && n < 0) {
      return iswhat(n + 256);
    }

    return ctype(v.toString(), iswhat);
  }

  if (v.isString()) {
    String s = v.toString();
    if (!s.empty()) {
      const char *p = s.data();
      const char *e = s.data() + s.size();
      while (p < e) {
        if (!iswhat((int)*(unsigned char *)(p++))) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

bool f_ctype_alnum(CVarRef text) {
  return ctype(text, isalnum);
}

bool f_ctype_alpha(CVarRef text) {
  return ctype(text, isalpha);
}

bool f_ctype_cntrl(CVarRef text) {
  return ctype(text, iscntrl);
}

bool f_ctype_digit(CVarRef text) {
  return ctype(text, isdigit);
}

bool f_ctype_graph(CVarRef text) {
  return ctype(text, isgraph);
}

bool f_ctype_lower(CVarRef text) {
  return ctype(text, islower);
}

bool f_ctype_print(CVarRef text) {
  return ctype(text, isprint);
}

bool f_ctype_punct(CVarRef text) {
  return ctype(text, ispunct);
}

bool f_ctype_space(CVarRef text) {
  return ctype(text, isspace);
}

bool f_ctype_upper(CVarRef text) {
  return ctype(text, isupper);
}

bool f_ctype_xdigit(CVarRef text) {
  return ctype(text, isxdigit);
}

///////////////////////////////////////////////////////////////////////////////
}
