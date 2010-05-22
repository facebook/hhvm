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

#include <runtime/ext/ext_math.h>
#include <runtime/base/zend/zend_math.h>
#include <runtime/base/zend/zend_multiply.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_min(int _argc, CVarRef value, CArrRef _argv /* = null_array */) {
  Variant ret;
  if (_argv.empty() && value.is(KindOfArray)) {
    Array v = value.toArray();
    if (!v.empty()) {
      ssize_t pos = v->iter_begin();
      if (pos != ArrayData::invalid_index) {
        ret = v->getValue(pos);
        while (true) {
          pos = v->iter_advance(pos);
          if (pos == ArrayData::invalid_index) break;
          Variant tmp = v->getValue(pos);
          if (less(tmp, ret)) {
            ret = tmp;
          }
        }
      }
    }
  } else {
    ret = value;
    if (!_argv.empty()) {
      for (ssize_t pos = _argv->iter_begin(); pos != ArrayData::invalid_index;
           pos = _argv->iter_advance(pos)) {
        Variant tmp = _argv->getValue(pos);
        if (less(tmp, ret)) {
          ret = tmp;
        }
      }
    }
  }
  return ret;
}

Variant f_max(int _argc, CVarRef value, CArrRef _argv /* = null_array */) {
  Variant ret;
  if (_argv.empty() && value.is(KindOfArray)) {
    Array v = value.toArray();
    if (!v.empty()) {
      ssize_t pos = v->iter_begin();
      if (pos != ArrayData::invalid_index) {
        ret = v->getValue(pos);
        while (true) {
          pos = v->iter_advance(pos);
          if (pos == ArrayData::invalid_index) break;
          Variant tmp = v->getValue(pos);
          if (more(tmp, ret)) {
            ret = tmp;
          }
        }
      }
    }
  } else {
    ret = value;
    if (!_argv.empty()) {
      for (ssize_t pos = _argv->iter_begin(); pos != ArrayData::invalid_index;
           pos = _argv->iter_advance(pos)) {
        Variant tmp = _argv->getValue(pos);
        if (more(tmp, ret)) {
          ret = tmp;
        }
      }
    }
  }
  return ret;
}

Variant f_abs(CVarRef number) {
  int64 ival;
  double dval;
  DataType k = number.toNumeric(ival, dval, true);
  if (k == KindOfDouble) {
    return fabs(dval);
  } else if (k == KindOfInt64) {
    return ival >= 0 ? ival : -ival;
  } else {
    return 0;
  }
}

double f_round(CVarRef val, int64 precision /* = 0 */) {
  int64 ival;
  double dval;
  DataType k = val.toNumeric(ival, dval, true);
 if (k == KindOfInt64) {
   if (precision >= 0) {
    return ival;
   } else {
     dval = ival;
   }
 } else if (k != KindOfDouble) {
   dval = val.toDouble();
 }
 PHP_ROUND_WITH_FUZZ(dval, precision);
 return dval;
}

Variant f_base_convert(CStrRef number, int64 frombase, int64 tobase) {
  if (!string_validate_base(frombase)) {
    throw_invalid_argument("Invalid frombase: %d", frombase);
    return false;
  }
  if (!string_validate_base(tobase)) {
    throw_invalid_argument("Invalid tobase: %d", tobase);
    return false;
  }
  Variant v = string_base_to_numeric(number.data(), number.size(), frombase);
  return String(string_numeric_to_base(v, tobase), AttachString);
}

Numeric f_pow(CVarRef base, CVarRef exp) {
  int64 bint, eint;
  double bdbl, edbl;
  DataType bt = base.toNumeric(bint, bdbl, true);
  DataType et = exp.toNumeric(eint, edbl, true);
  if (bt == KindOfInt64 && et == KindOfInt64) {
    if (eint == 0) return 1LL;
    if (bint == 0) return 0LL;

    // calculate pow(long,long) in O(log exp) operations, bail if overflow
    int64 l1 = 1;
    while (eint >= 1) {
      int overflow;
      double dval = 0.0;
      if (eint % 2) {
        --eint;
        ZEND_SIGNED_MULTIPLY_LONG(l1, bint, l1, dval, overflow);
        if (overflow) return dval * pow(bint, eint);
      } else {
        eint /= 2;
        ZEND_SIGNED_MULTIPLY_LONG(bint, bint, bint, dval, overflow);
        if (overflow) return (double)l1 * pow(dval, eint);
      }
      if (eint == 0) {
        return l1;
      }
    }
  }
  if (bt != KindOfDouble) {
    bdbl = base.toDouble();
  }
  if (et != KindOfDouble) {
    edbl = exp.toDouble();
  }
  return pow(bdbl, edbl);
}

///////////////////////////////////////////////////////////////////////////////

static bool s_rand_is_seeded = false;

void f_srand(CVarRef seed /* = null_variant */) {
  if (seed.isNull()) {
    return srand(GENERATE_SEED());
  }
  return srand(seed.toInt32());
}

int64 f_rand(int64 min /* = 0 */, int64 max /* = RAND_MAX */) {
  if (!s_rand_is_seeded) {
    s_rand_is_seeded = true;
    srand(GENERATE_SEED());
  }

  int64 number = rand();
  if (min != 0 || max != RAND_MAX) {
    RAND_RANGE(number, min, max, RAND_MAX);
  }
  return number;
}

void f_mt_srand(CVarRef seed /* = null_variant */) {
  if (seed.isNull()) {
    return srand(GENERATE_SEED());
  }
  return srand(seed.toInt32());
}

///////////////////////////////////////////////////////////////////////////////
}
