/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/math/ext_math.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/zend-multiply.h"
#include "hphp/runtime/base/zend-string.h"
#include <math.h>

const double k_M_PI = 3.1415926535898;

#if defined(__APPLE__)
#ifndef isnan
#define isnan(x)  \
  ( sizeof (x) == sizeof(float )  ? __inline_isnanf((float)(x)) \
  : sizeof (x) == sizeof(double)  ? __inline_isnand((double)(x))  \
  : __inline_isnanl ((long double)(x)))
#endif

#ifndef isinf
#define isinf(x)  \
  ( sizeof (x) == sizeof(float )  ? __inline_isinff((float)(x)) \
  : sizeof (x) == sizeof(double)  ? __inline_isinfd((double)(x))  \
  : __inline_isinfl ((long double)(x)))
#endif
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64_t k_PHP_ROUND_HALF_UP =   PHP_ROUND_HALF_UP;
const int64_t k_PHP_ROUND_HALF_DOWN = PHP_ROUND_HALF_DOWN;
const int64_t k_PHP_ROUND_HALF_EVEN = PHP_ROUND_HALF_EVEN;
const int64_t k_PHP_ROUND_HALF_ODD =  PHP_ROUND_HALF_ODD;

/* Logic based on zend_operators.c::convert_scalar_to_number() */
static DataType zend_convert_scalar_to_number(CVarRef num,
                                              int64_t &ival,
                                              double &dval) {
  DataType dt = num.toNumeric(ival, dval, true);
  if ((dt == KindOfDouble) || (dt == KindOfInt64)) {
    return dt;
  }

  if (num.isBoolean() || num.isNull() || num.isObject() || num.isResource() ||
      num.isString()) {
    ival = num.toInt64();
    return KindOfInt64;
  }

  // Fallback, callers will handle this as an error
  ival = 0;
  dval = 0.0;
  return num.getType();
}

Variant HHVM_FUNCTION(abs, CVarRef number) {
  int64_t ival;
  double dval;
  DataType k = zend_convert_scalar_to_number(number, ival, dval);
  if (k == KindOfDouble) {
    return fabs(dval);
  } else if (k == KindOfInt64) {
    return ival >= 0 ? ival : -ival;
  } else {
    return false;
  }
}

bool HHVM_FUNCTION(is_finite, double val) { return finite(val);}
bool HHVM_FUNCTION(is_infinite, double val) { return isinf(val);}
bool HHVM_FUNCTION(is_nan, double val) { return isnan(val);}

Variant HHVM_FUNCTION(ceil, CVarRef number) {
  int64_t ival;
  double dval;
  DataType k = zend_convert_scalar_to_number(number, ival, dval);
  if (k == KindOfInt64) {
    dval = (double)ival;
  } else if (k != KindOfDouble) {
    return false;
  }
  return ceil(dval);
}

Variant HHVM_FUNCTION(floor, CVarRef number) {
  int64_t ival;
  double dval;
  DataType k = zend_convert_scalar_to_number(number, ival, dval);
  if (k == KindOfInt64) {
    dval = (double)ival;
  } else if (k != KindOfDouble) {
    return false;
  }
  return floor(dval);
}

Variant HHVM_FUNCTION(round, CVarRef val, int64_t precision /* = 0 */,
                int64_t mode /* = PHP_ROUND_HALF_UP */) {
  int64_t ival;
  double dval;
  DataType k = zend_convert_scalar_to_number(val, ival, dval);
  if (k == KindOfInt64) {
    if (precision >= 0) {
     return (double)ival;
    } else {
      dval = ival;
    }
  } else if (k != KindOfDouble) {
    return false;
  }
  dval = php_math_round(dval, precision, mode);
  return dval;
}

double HHVM_FUNCTION(deg2rad, double number) { return number / 180.0 * k_M_PI;}
double HHVM_FUNCTION(rad2deg, double number) { return number / k_M_PI * 180.0;}

String HHVM_FUNCTION(decbin, int64_t number) {
  return String(string_long_to_base(number, 2), AttachString);
}
String HHVM_FUNCTION(dechex, int64_t number) {
  return String(string_long_to_base(number, 16), AttachString);
}
String HHVM_FUNCTION(decoct, int64_t number) {
  return String(string_long_to_base(number, 8), AttachString);
}
Variant HHVM_FUNCTION(bindec, CVarRef binary_string) {
  String str = binary_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 2);
}
Variant HHVM_FUNCTION(hexdec, CVarRef hex_string) {
  String str = hex_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 16);
}
Variant HHVM_FUNCTION(octdec, CVarRef octal_string) {
  String str = octal_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 8);
}

Variant HHVM_FUNCTION(base_convert, CVarRef number, int64_t frombase, int64_t tobase) {
  if (!string_validate_base(frombase)) {
    throw_invalid_argument("Invalid frombase: %" PRId64, frombase);
    return false;
  }
  if (!string_validate_base(tobase)) {
    throw_invalid_argument("Invalid tobase: %" PRId64, tobase);
    return false;
  }
  String str = number.toString();
  Variant v = string_base_to_numeric(str.data(), str.size(), frombase);
  return String(string_numeric_to_base(v, tobase), AttachString);
}

Variant HHVM_FUNCTION(pow, CVarRef base, CVarRef exp) {
  int64_t bint, eint;
  double bdbl, edbl;
  DataType bt = base.toNumeric(bint, bdbl, true);
  DataType et = exp.toNumeric(eint, edbl, true);
  if (bt == KindOfInt64 && et == KindOfInt64 && eint >= 0) {
    if (eint == 0) return 1LL;
    if (bint == 0) return 0LL;

    // calculate pow(long,long) in O(log exp) operations, bail if overflow
    int64_t l1 = 1;
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

double HHVM_FUNCTION(exp, double arg) { return exp(arg);}
double HHVM_FUNCTION(expm1, double arg) { return expm1(arg);}
double HHVM_FUNCTION(log10, double arg) { return log10(arg);}
double HHVM_FUNCTION(log1p, double number) { return log1p(number);}
double HHVM_FUNCTION(log, double arg, double base /* = 0 */) {
  return base <= 0 ? log(arg) : log(arg)/log(base);
}

double HHVM_FUNCTION(cos, double arg) { return cos(arg);  }
double HHVM_FUNCTION(cosh, double arg) { return cosh(arg); }
double HHVM_FUNCTION(sin, double arg) { return sin(arg);  }
double HHVM_FUNCTION(sinh, double arg) { return sinh(arg); }
double HHVM_FUNCTION(tan, double arg) { return tan(arg);  }
double HHVM_FUNCTION(tanh, double arg) { return tanh(arg); }
double HHVM_FUNCTION(acos, double arg) { return acos(arg); }
double HHVM_FUNCTION(acosh, double arg) { return acosh(arg);}
double HHVM_FUNCTION(asin, double arg) { return asin(arg); }
double HHVM_FUNCTION(asinh, double arg) { return asinh(arg);}
double HHVM_FUNCTION(atan, double arg) { return atan(arg); }
double HHVM_FUNCTION(atanh, double arg) { return atanh(arg);}
double HHVM_FUNCTION(atan2, double y, double x) { return atan2(y, x);}

double HHVM_FUNCTION(hypot, double x, double y) { return hypot(x, y);}
double HHVM_FUNCTION(fmod, double x, double y) { return fmod(x, y);}
double HHVM_FUNCTION(sqrt, double arg) { return sqrt(arg);}

int64_t HHVM_FUNCTION(getrandmax) { return RAND_MAX;}

///////////////////////////////////////////////////////////////////////////////

static bool s_rand_is_seeded = false;

void HHVM_FUNCTION(srand, CVarRef seed /* = null_variant */) {
  s_rand_is_seeded = true;
  if (seed.isNull()) {
    return srand(math_generate_seed());
  }
  if (seed.isNumeric(true)) {
    srand(seed.toInt32());
  } else {
    raise_warning("srand() expects parameter 1 to be long");
  }
}

TypedValue* HHVM_FUNCTION(rand, ActRec* ar) {
  TypedValue rvSpace = make_tv<KindOfInt64>(0);
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (LIKELY(count == 0 || count == 2)) {
    if (UNLIKELY(count == 2 && ((args - 0)->m_type != KindOfInt64
                                  || (args - 1)->m_type != KindOfInt64))) {
      if ((args - 0)->m_type != KindOfInt64) {
        tvCastToInt64InPlace(args-0);
      }
      if ((args - 1)->m_type != KindOfInt64) {
        tvCastToInt64InPlace(args-1);
      }
    }
    int64_t min, max;
    if (count == 0) {
      min = 0;
      max = HHVM_FN(getrandmax)();
    } else {
      min = args[-0].m_data.num;
      max = args[-1].m_data.num;
    }
    rvSpace.m_data.num = hphp_rand(min, max);
  } else {
    throw_wrong_arguments_nr("rand", count, 1, 2, 1, rv);
  }
  frame_free_locals_no_this_inl(ar, 2);
  ar->m_r = *rv;
  return &ar->m_r;
}

int64_t hphp_rand(int64_t min /* = 0 */, int64_t max /* = RAND_MAX */) {
  if (!s_rand_is_seeded) {
    s_rand_is_seeded = true;
    srand(math_generate_seed());
  }

  int64_t number = rand();
  if (min != 0 || max != RAND_MAX) {
    RAND_RANGE(number, min, max, RAND_MAX);
  }
  return number;
}

int64_t HHVM_FUNCTION(mt_getrandmax) { return MT_RAND_MAX;}

TypedValue* HHVM_FUNCTION(mt_rand, ActRec* ar) {
  TypedValue rvSpace = make_tv<KindOfInt64>(0);
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (LIKELY(count == 0 || count == 2)) {
    if (UNLIKELY(count == 2 && ((args - 0)->m_type != KindOfInt64
                                  || (args - 1)->m_type != KindOfInt64))) {
      if ((args - 0)->m_type != KindOfInt64) {
        tvCastToInt64InPlace(args-0);
      }
      if ((args - 1)->m_type != KindOfInt64) {
        tvCastToInt64InPlace(args-1);
      }
    }
    int64_t min, max;
    if (count == 0) {
      min = 0;
      max = HHVM_FN(mt_getrandmax)();
    } else {
      min = args[-0].m_data.num;
      max = args[-1].m_data.num;
    }
    rvSpace.m_data.num = hphp_mt_rand(min, max);
  } else {
    throw_wrong_arguments_nr("mt_rand", count, 1, 2, 1, rv);
  }
  frame_free_locals_no_this_inl(ar, 2);
  ar->m_r = *rv;
  return &ar->m_r;
}

void HHVM_FUNCTION(mt_srand, CVarRef seed /* = null_variant */) {
  if (seed.isNull()) {
    return math_mt_srand(math_generate_seed());
  }
  if (seed.isNumeric(true)) {
    math_mt_srand(seed.toInt32());
  } else {
    raise_warning("mt_srand() expects parameter 1 to be long");
  }
}

int64_t hphp_mt_rand(int64_t min /* = 0 */, int64_t max /* = MT_RAND_MAX */) {
  return math_mt_rand(min, max);
}
double HHVM_FUNCTION(lcg_value) { return math_combined_lcg();}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_PHP_ROUND_HALF_UP("PHP_ROUND_HALF_UP");
const StaticString s_PHP_ROUND_HALF_DOWN("PHP_ROUND_HALF_DOWN");
const StaticString s_PHP_ROUND_HALF_EVEN("PHP_ROUND_HALF_EVEN");
const StaticString s_PHP_ROUND_HALF_ODD("PHP_ROUND_HALF_ODD");
const StaticString s_M_PI("M_PI");

class StandardMathExtension : public Extension {
 public:
  StandardMathExtension() : Extension("math") {}
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_UP.get(), k_PHP_ROUND_HALF_UP
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_DOWN.get(), k_PHP_ROUND_HALF_DOWN
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_EVEN.get(), k_PHP_ROUND_HALF_EVEN
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_ODD.get(), k_PHP_ROUND_HALF_ODD
    );
    Native::registerConstant<KindOfDouble>(
      s_M_PI.get(), k_M_PI
    );

    HHVM_FE(abs);
    HHVM_FE(is_finite);
    HHVM_FE(is_infinite);
    HHVM_FE(is_nan);
    HHVM_FE(ceil);
    HHVM_FE(floor);
    HHVM_FE(round);
    HHVM_FE(deg2rad);
    HHVM_FE(rad2deg);
    HHVM_FE(decbin);
    HHVM_FE(dechex);
    HHVM_FE(decoct);
    HHVM_FE(bindec);
    HHVM_FE(hexdec);
    HHVM_FE(octdec);
    HHVM_FE(base_convert);
    HHVM_FE(pow);
    HHVM_FE(exp);
    HHVM_FE(expm1);
    HHVM_FE(log10);
    HHVM_FE(log1p);
    HHVM_FE(log);
    HHVM_FE(cos);
    HHVM_FE(cosh);
    HHVM_FE(sin);
    HHVM_FE(sinh);
    HHVM_FE(tan);
    HHVM_FE(tanh);
    HHVM_FE(acos);
    HHVM_FE(acosh);
    HHVM_FE(asin);
    HHVM_FE(asinh);
    HHVM_FE(atan);
    HHVM_FE(atanh);
    HHVM_FE(atan2);
    HHVM_FE(hypot);
    HHVM_FE(fmod);
    HHVM_FE(sqrt);
    HHVM_FE(getrandmax);
    HHVM_FE(srand);
    HHVM_FE(rand);
    HHVM_FE(mt_getrandmax);
    HHVM_FE(mt_srand);
    HHVM_FE(mt_rand);
    HHVM_FE(lcg_value);

    loadSystemlib();
  }
} s_standardmath_extension;

}
