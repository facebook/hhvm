/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/comparisons.h>
#include <runtime/base/zend/zend_functions.h>
#include <runtime/base/zend/zend_string.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool not_more(CVarRef v1, CVarRef v2) {
  if (v1.is(KindOfArray) && v2.is(KindOfArray)) {
    return !v1.toArray().more(v2.toArray(), false);
  }
  return !more(v1, v2);
}

bool not_less(CVarRef v1, CVarRef v2) {
  if (v1.is(KindOfArray) && v2.is(KindOfArray)) {
    return !v1.toArray().less(v2.toArray(), true);
  }
  return !less(v1, v2);
}

bool equal(char v1, CStrRef v2) {
  return equal((int64)v1, v2);
}

bool equal(short v1, CStrRef v2) {
  return equal((int64)v1, v2);
}

bool equal(int v1, CStrRef v2) {
  return equal((int64)v1, v2);
}

bool equal(int64 v1, CStrRef v2) {
  DataType ret = KindOfNull;
  int64 lval; double dval;
  ret = is_numeric_string(v2.data(), v2.size(), &lval, &dval, 1);
  if (ret == KindOfInt64) {
    return v1 == lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 == dval;
  } else {
    return v1 == 0;
  }
}

bool less(char v1, CStrRef v2) {
  return less((int64)v1, v2);
}

bool less(short v1, CStrRef v2) {
  return less((int64)v1, v2);
}

bool less(int v1, CStrRef v2) {
  return less((int64)v1, v2);
}

bool less(int64 v1, CStrRef v2) {
  DataType ret = KindOfNull;
  int64 lval; double dval;
  ret = is_numeric_string(v2.data(), v2.size(), &lval, &dval, 1);
  if (ret == KindOfInt64) {
    return v1 < lval;
  } else if (ret == KindOfDouble) {
    return (double)v1 < dval;
  } else {
    return v1 < 0;
  }
}

bool more(char v1, CStrRef v2) {
  return more((int64)v1, v2);
}

bool more(short v1, CStrRef v2) {
  return more((int64)v1, v2);
}

bool more(int v1, CStrRef v2) {
  return more((int64)v1, v2);
}

bool more(int64 v1, CStrRef v2) {
  DataType ret = KindOfNull;
  int64 lval; double dval;
  ret = is_numeric_string(v2.data(), v2.size(), &lval, &dval, 1);
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
