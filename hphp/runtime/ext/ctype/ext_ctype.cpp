/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static bool ctype(const Variant& v, int (*iswhat)(int)) {
  if (v.isInteger()) {
    int64_t n = v.toInt64();
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

// Use lambdas wrapping the ctype.h functions because of linker weirdness on
// OS X Mavericks.

bool HHVM_FUNCTION(ctype_alnum, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isalnum(i); });
}

bool HHVM_FUNCTION(ctype_alpha, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isalpha(i); });
}

bool HHVM_FUNCTION(ctype_cntrl, const Variant& text) {
  return ctype(text, [] (int i) -> int { return iscntrl(i); });
}

bool HHVM_FUNCTION(ctype_digit, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isdigit(i); });
}

bool HHVM_FUNCTION(ctype_graph, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isgraph(i); });
}

bool HHVM_FUNCTION(ctype_lower, const Variant& text) {
  return ctype(text, [] (int i) -> int { return islower(i); });
}

bool HHVM_FUNCTION(ctype_print, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isprint(i); });
}

bool HHVM_FUNCTION(ctype_punct, const Variant& text) {
  return ctype(text, [] (int i) -> int { return ispunct(i); });
}

bool HHVM_FUNCTION(ctype_space, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isspace(i); });
}

bool HHVM_FUNCTION(ctype_upper, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isupper(i); });
}

bool HHVM_FUNCTION(ctype_xdigit, const Variant& text) {
  return ctype(text, [] (int i) -> int { return isxdigit(i); });
}

///////////////////////////////////////////////////////////////////////////////

class CtypeExtension : public Extension {
 public:
  CtypeExtension() : Extension("ctype") {}
  virtual void moduleInit() {
    HHVM_FE(ctype_alnum);
    HHVM_FE(ctype_alpha);
    HHVM_FE(ctype_cntrl);
    HHVM_FE(ctype_digit);
    HHVM_FE(ctype_graph);
    HHVM_FE(ctype_lower);
    HHVM_FE(ctype_print);
    HHVM_FE(ctype_punct);
    HHVM_FE(ctype_space);
    HHVM_FE(ctype_upper);
    HHVM_FE(ctype_xdigit);

    loadSystemlib();
  }
} s_ctype_extension;

///////////////////////////////////////////////////////////////////////////////
}
