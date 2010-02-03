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

#include <cpp/ext/ext_math.h>
#include <cpp/base/zend/zend_math.h>
#include <cpp/base/zend/zend_multiply.h>

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
  if (number.is(KindOfDouble)) {
    return fabs(number.toDouble());
  }
  int64 ret = number.toInt64();
  return ret >= 0 ? ret : -ret;
}

double f_round(CVarRef val, int64 precision /* = 0 */) {
  if (val.isInteger() && precision >= 0) {
    return val.toInt64();
  }
  double ret = val.toDouble();
  PHP_ROUND_WITH_FUZZ(ret, precision);
  return ret;
}

Numeric f_pow(CVarRef base, CVarRef exp) {
  if (base.isInteger() && exp.isInteger()) {
    int64 i = exp.toInt64();
    if (i == 0) return 1LL;
    int64 l2 = base.toInt64();
    if (l2 == 0) return 0LL;

    // calculate pow(long,long) in O(log exp) operations, bail if overflow
    int64 l1 = 1;
    while (i >= 1) {
      int overflow;
      double dval = 0.0;
      if (i % 2) {
        --i;
        ZEND_SIGNED_MULTIPLY_LONG(l1, l2, l1, dval, overflow);
        if (overflow) return dval * pow(l2, i);
      } else {
        i /= 2;
        ZEND_SIGNED_MULTIPLY_LONG(l2, l2, l2, dval, overflow);
        if (overflow) return (double)l1 * pow(dval,i);
      }
      if (i == 0) {
        return l1;
      }
    }
  }
  return pow(base.toDouble(), exp.toDouble());
}

///////////////////////////////////////////////////////////////////////////////

void f_srand(CVarRef seed /* = null_variant */) {
  if (seed.isNull()) {
    return srand(GENERATE_SEED());
  }
  return srand(seed.toInt32());
}

int64 f_rand(int64 min /* = 0 */, int64 max /* = RAND_MAX */) {
  int number = rand();
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
