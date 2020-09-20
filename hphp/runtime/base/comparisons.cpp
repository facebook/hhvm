/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/zend-functions.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
// Variant

bool same(const Variant& v1, bool v2) {
  return v1.isBoolean() && v2 == v1.getBoolean();
}

bool same(const Variant& v1, int64_t v2) {
  auto const cell = v1.asTypedValue();
  if (isIntType(cell->m_type)) {
    return v2 == cell->m_data.num;
  }
  return false;
}

bool same(const Variant& v1, double d) {
  return v1.isDouble() && v1.getDouble() == d;
}

bool same(const Variant& v1, const StringData* v2) {
  bool null1 = v1.isNull();
  bool null2 = (v2 == nullptr);
  if (null1 && null2) return true;
  if (null1 || null2) return false;
  if (!v1.isString()) return false;
  auto const sdata = v1.getStringData();
  return sdata == v2 || v2->same(sdata);
}

bool same(const Variant& v1, const String& v2) {
  const StringData* sd = v2.get();
  return same(v1, sd);
}

//////////////////////////////////////////////////////////////////////

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

bool lessEqual(int64_t v1, const StringData *v2) {
  int64_t lval; double dval;
  DataType ret = v2->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return v1 <= lval;
  }
  if (ret == KindOfDouble) {
    return (double)v1 <= dval;
  }
  return v1 <= 0;
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

bool moreEqual(int64_t v1, const StringData *v2) {
  int64_t lval; double dval;
  DataType ret = v2->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return v1 >= lval;
  }
  if (ret == KindOfDouble) {
    return (double)v1 >= dval;
  }
  return v1 >= 0;
}

int64_t compare(const StringData* v1, int64_t v2) {
  assertx(v1);
  int64_t lval;
  double dval;
  auto ret = v1->isNumericWithVal(lval, dval, 1);
  if (ret == KindOfInt64) {
    return compare(lval, v2);
  } else if (ret == KindOfDouble) {
    return compare(dval, (double)v2);
  }
  return compare((int64_t)0, v2);
}

//////////////////////////////////////////////////////////////////////
}
