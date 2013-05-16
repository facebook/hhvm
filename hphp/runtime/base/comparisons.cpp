/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend/zend_functions.h"
#include "hphp/runtime/base/zend/zend_string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool less_or_equal(CVarRef v1, CVarRef v2) {
  // To be PHP-compatible, when comparing two arrays or two objects we
  // cannot assume that "($x <= $y)" is equivalent to "!($x > $y)".
  if (v1.is(KindOfArray) && v2.is(KindOfArray)) {
    Array a1 = v1.toArray();
    Array a2 = v2.toArray();
    return a1.less(a2) || a1.equal(a2);
  }
  if (v1.is(KindOfObject) && v2.is(KindOfObject)) {
    Object o1 = v1.toObject();
    Object o2 = v2.toObject();
    return o1.less(o2) || o1.equal(o2);
  }
  return !more(v1, v2);
}

bool more_or_equal(CVarRef v1, CVarRef v2) {
  // To be PHP-compatible, when comparing two arrays or two objects we
  // cannot assume that "($x >= $y)" is equivalent to "!($x < $y)".
  if (v1.is(KindOfArray) && v2.is(KindOfArray)) {
    Array a1 = v1.toArray();
    Array a2 = v2.toArray();
    return a1.more(a2) || a1.equal(a2);
  }
  if (v1.is(KindOfObject) && v2.is(KindOfObject)) {
    Object o1 = v1.toObject();
    Object o2 = v2.toObject();
    return o1.more(o2) || o1.equal(o2);
  }
  return !less(v1, v2);
}

bool equal(int v1, const StringData *v2) {
  return equal((int64_t)v1, v2);
}

bool equal(int64_t v1, const StringData *v2) {
  int64_t lval; double dval;
  DataType ret = v2->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return v1 == lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 == dval;
  } else {
    return v1 == 0;
  }
}

bool equalAsStr(bool v1, const StringData *v2) {
  return same(toString(v1), v2);
}

bool equalAsStr(int v1, const StringData *v2) {
  char tmpbuf[12];
  char *p;
  int is_negative;
  int len;
  const StringData *sd = String::GetIntegerStringData(v1);
  if (sd) {
    p = (char *)sd->data();
    len = sd->size();
  } else {
    p = conv_10(v1, &is_negative, &tmpbuf[11], &len);
  }
  if (len != v2->size()) {
    return false;
  }
  return memcmp(p, v2->data(), len) == 0;
}

bool equalAsStr(int64_t v1, const StringData *v2) {
  char tmpbuf[21];
  char *p;
  int is_negative;
  int len;
  const StringData *sd = String::GetIntegerStringData(v1);
  if (sd) {
    p = (char *)sd->data();
    len = sd->size();
  } else {
    p = conv_10(v1, &is_negative, &tmpbuf[20], &len);
  }
  if (len != v2->size()) {
    return false;
  }
  return memcmp(p, v2->data(), len) == 0;
}

bool equalAsStr(int64_t v1, litstr  v2) {
  char tmpbuf[21];
  char *p;
  int is_negative;
  int len;
  const StringData *sd = String::GetIntegerStringData(v1);
  if (sd) {
    p = (char *)sd->data();
    len = sd->size();
  } else {
    tmpbuf[20] = '\0';
    p = conv_10(v1, &is_negative, &tmpbuf[20], &len);
  }
  return strcmp(p, v2) == 0;
}

bool equalAsStr(double v1, const StringData *v2) {
  return equalAsStr(v2, String(v1));
}

bool equalAsStr(double v1, litstr  v2) {
  return same(String(v1), v2);
}

bool less(int v1, const StringData *v2) {
  return less((int64_t)v1, v2);
}

bool less(int64_t v1, const StringData *v2) {
  int64_t lval; double dval;
  DataType ret = v2->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return v1 < lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 < dval;
  } else {
    return v1 < 0;
  }
}

bool more(int v1, const StringData *v2) {
  return more((int64_t)v1, v2);
}

bool more(int64_t v1, const StringData *v2) {
  int64_t lval; double dval;
  DataType ret = v2->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return v1 > lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 > dval;
  } else {
    return v1 > 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
