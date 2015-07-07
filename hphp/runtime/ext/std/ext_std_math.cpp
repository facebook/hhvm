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

#include "hphp/runtime/ext/std/ext_std_math.h"

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/zend-multiply.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/system/constants.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64_t k_PHP_ROUND_HALF_UP =   PHP_ROUND_HALF_UP;
const int64_t k_PHP_ROUND_HALF_DOWN = PHP_ROUND_HALF_DOWN;
const int64_t k_PHP_ROUND_HALF_EVEN = PHP_ROUND_HALF_EVEN;
const int64_t k_PHP_ROUND_HALF_ODD =  PHP_ROUND_HALF_ODD;

const double k_M_PI       = 3.1415926535898;
const double k_M_1_PI     = 0.31830988618379;
const double k_M_2_PI     = 0.63661977236758;
const double k_M_2_SQRTPI = 1.1283791670955;
const double k_M_E        = 2.718281828459;
const double k_M_EULER    = 0.57721566490153;
const double k_M_LN10     = 2.302585092994;
const double k_M_LN2      = 0.69314718055995;
const double k_M_LNPI     = 1.1447298858494;
const double k_M_LOG10E   = 0.43429448190325;
const double k_M_LOG2E    = 1.442695040889;
const double k_M_PI_2     = 1.5707963267949;
const double k_M_PI_4     = 0.78539816339745;
const double k_M_SQRT1_2  = 0.70710678118655;
const double k_M_SQRT2    = 1.4142135623731;
const double k_M_SQRT3    = 1.7320508075689;
const double k_M_SQRTPI   = 1.7724538509055;

Variant HHVM_FUNCTION(min,
                      const Variant& value,
                      const Array& args /* = null_array */) {
  if (args.empty()) {
    const auto& cell_value = *value.asCell();
    if (UNLIKELY(!isContainer(cell_value))) {
      if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::WARN) {
        raise_warning("min(): This will return the value instead of null, "
                      "when hhvm.hack.lang.min_max_allow_degenerate=on");
      } else if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::ON) {
        return value;
      }
      raise_warning("min(): When only one parameter is given,"
                    " it must be an array");
      return init_null();
    }

    ArrayIter iter(cell_value);
    if (!iter) {
      if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::WARN) {
        raise_warning("min(): This will return null instead of false, "
                      "when hhvm.hack.lang.min_max_allow_degenerate=on");
      } else if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::ON) {
        return init_null();
      }
      raise_warning("min(): Array must contain at least one element");
      return false;
    }
    Variant ret = iter.secondRefPlus();
    ++iter;
    for (; iter; ++iter) {
      Variant currVal = iter.secondRefPlus();
      if (less(currVal, ret)) {
        ret = currVal;
      }
    }
    return ret;
  }

  Variant ret = value;
  for (ArrayIter iter(args); iter; ++iter) {
    Variant currVal = iter.secondRef();
    if (less(currVal, ret)) {
      ret = currVal;
    }
  }
  return ret;
}

Variant HHVM_FUNCTION(max,
                      const Variant& value,
                      const Array& args /* = null_array */) {
  if (args.empty()) {
    const auto& cell_value = *value.asCell();
    if (UNLIKELY(!isContainer(cell_value))) {
      if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::WARN) {
        raise_warning("max(): This will return the value instead of null, "
                      "when hhvm.hack.lang.min_max_allow_degenerate=on");
      } else if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::ON) {
        return value;
      }
      raise_warning("max(): When only one parameter is given,"
                    " it must be an array");
      return init_null();
    }

    ArrayIter iter(cell_value);
    if (!iter) {
      if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::WARN) {
        raise_warning("max(): This will return null instead of false, "
                      "when hhvm.hack.lang.min_max_allow_degenerate=on");
      } else if (RuntimeOption::MinMaxAllowDegenerate == HackStrictOption::ON) {
        return init_null();
      }
      raise_warning("max(): Array must contain at least one element");
      return false;
    }
    Variant ret = iter.secondRefPlus();
    ++iter;
    for (; iter; ++iter) {
      Variant currVal = iter.secondRefPlus();
      if (more(currVal, ret)) {
        ret = currVal;
      }
    }
    return ret;
  }

  Variant ret = value;
  for (ArrayIter iter(args); iter; ++iter) {
    Variant currVal = iter.secondRef();
    if (more(currVal, ret)) {
      ret = currVal;
    }
  }
  return ret;
}

/*
 * Logic based on zend_operators.c::convert_scalar_to_number()
 *
 * Note that this needs to work the same as some similar logic in the JIT for
 * optimized versions of some of the following functions.
 */
static DataType zend_convert_scalar_to_number(const Variant& num,
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

Variant HHVM_FUNCTION(abs, const Variant& number) {
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

Variant HHVM_FUNCTION(ceil, const Variant& number) {
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

Variant HHVM_FUNCTION(floor, const Variant& number) {
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

Variant HHVM_FUNCTION(round,
                      const Variant& val,
                      int64_t precision /* = 0 */,
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

String HHVM_FUNCTION(decbin, const Variant& number) {
  return string_long_to_base(number.toInt64(), 2);
}
String HHVM_FUNCTION(dechex, const Variant& number) {
  return string_long_to_base(number.toInt64(), 16);
}
String HHVM_FUNCTION(decoct, const Variant& number) {
  return string_long_to_base(number.toInt64(), 8);
}
Variant HHVM_FUNCTION(bindec, const Variant& binary_string) {
  String str = binary_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 2);
}
Variant HHVM_FUNCTION(hexdec, const Variant& hex_string) {
  String str = hex_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 16);
}
Variant HHVM_FUNCTION(octdec, const Variant& octal_string) {
  String str = octal_string.toString();
  return string_base_to_numeric(str.data(), str.size(), 8);
}

Variant HHVM_FUNCTION(base_convert,
                      const Variant& number,
                      int64_t frombase,
                      int64_t tobase) {
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
  return string_numeric_to_base(v, tobase);
}

static MaybeDataType convert_for_pow(const Variant& val,
                                     int64_t& ival, double& dval) {
  switch (val.getType()) {
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfResource:
    case KindOfObject:
      ival = val.toInt64();
      return KindOfInt64;

    case KindOfDouble:
      dval = val.toDouble();
      return KindOfDouble;

    case KindOfStaticString:
    case KindOfString: {
      auto dt = val.toNumeric(ival, dval, true);
      if ((dt != KindOfInt64) && (dt != KindOfDouble)) {
        ival = 0;
        return KindOfInt64;
      }
      return dt;
    }

    case KindOfArray:
      // Not reachable since HHVM_FN(pow) deals with these base cases first.
    case KindOfRef:
    case KindOfClass:
      break;
  }

  // Unknown data type.
  raise_error("Unsupported operand types");
  not_reached();
}

Variant HHVM_FUNCTION(pow, const Variant& base, const Variant& exp) {
  int64_t bint, eint;
  double bdbl, edbl;
  if (base.isArray()) return 0LL;
  MaybeDataType bt = convert_for_pow(base, bint, bdbl);
  if (exp.isArray()) return 1LL;
  MaybeDataType et = convert_for_pow(exp,  eint, edbl);
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

  auto const castableToNumber = [] (const ObjectData* obj) {
    return obj->getAttribute(ObjectData::CallToImpl) && !obj->isCollection();
  };

  // We'll have already raised a notice in convert_for_pow
  // so avoid re-raising the notice here.
  auto const silent_val_to_double = [&](const Variant& v) {
    if (v.isObject() && !castableToNumber(v.toObject().get())) {
      return 1.0;
    }
    return v.toDouble();
  };

  if (bt != KindOfDouble) {
    bdbl = silent_val_to_double(base);
  }
  if (et != KindOfDouble) {
    edbl = silent_val_to_double(exp);
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

struct RandomBuf {
  random_data data;
  char        buf[128];
  enum {
    Uninit = 0, ThreadInit, RequestInit
  }           state;
};

static __thread RandomBuf s_state;

static void randinit(uint32_t seed) {
  if (s_state.state == RandomBuf::Uninit) {
    initstate_r(seed, s_state.buf, sizeof s_state.buf, &s_state.data);
  } else {
    srandom_r(seed, &s_state.data);
  }
  s_state.state = RandomBuf::RequestInit;
}

void HHVM_FUNCTION(srand, const Variant& seed /* = null_variant */) {
  if (seed.isNull()) {
    randinit(math_generate_seed());
    return;
  }
  if (seed.isNumeric(true)) {
    randinit(seed.toInt32());
  } else {
    raise_warning("srand() expects parameter 1 to be long");
  }
}

int64_t HHVM_FUNCTION(rand,
                      int64_t min /* = 0 */,
                      const Variant& max /* = null_variant */) {
  if (s_state.state != RandomBuf::RequestInit) {
    randinit(math_generate_seed());
  }

  int32_t number;
  random_r(&s_state.data, &number);
  int64_t int_max = max.isNull() ? RAND_MAX : max.toInt64();
  if (min != 0 || int_max != RAND_MAX) {
    RAND_RANGE(number, min, int_max, RAND_MAX);
  }
  return number;
}

int64_t HHVM_FUNCTION(mt_getrandmax) { return MT_RAND_MAX;}

void HHVM_FUNCTION(mt_srand,
                   const Variant& seed /* = null_variant */) {
  if (seed.isNull()) {
    return math_mt_srand(math_generate_seed());
  }
  if (seed.isNumeric(true)) {
    math_mt_srand(seed.toInt32());
  } else {
    raise_warning("mt_srand() expects parameter 1 to be long");
  }
}

int64_t HHVM_FUNCTION(mt_rand,
                      int64_t min /* = 0 */,
                      const Variant& max /* = null_variant */) {
  return math_mt_rand(min, max.isNull() ? RAND_MAX : max.toInt64());
}

double HHVM_FUNCTION(lcg_value) { return math_combined_lcg();}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_PHP_ROUND_HALF_UP("PHP_ROUND_HALF_UP");
const StaticString s_PHP_ROUND_HALF_DOWN("PHP_ROUND_HALF_DOWN");
const StaticString s_PHP_ROUND_HALF_EVEN("PHP_ROUND_HALF_EVEN");
const StaticString s_PHP_ROUND_HALF_ODD("PHP_ROUND_HALF_ODD");

#define ICONST(nm)                                                             \
  Native::registerConstant<KindOfInt64>(makeStaticString(#nm), k_##nm)         \

#define DCONST(nm)                                                             \
  Native::registerConstant<KindOfDouble>(makeStaticString("M_"#nm), k_M_##nm)  \

void StandardExtension::requestInit() {
  if (s_state.state == RandomBuf::RequestInit) {
    s_state.state = RandomBuf::ThreadInit;
  }
}

void StandardExtension::initMath() {
  ICONST(PHP_ROUND_HALF_UP);
  ICONST(PHP_ROUND_HALF_DOWN);
  ICONST(PHP_ROUND_HALF_EVEN);
  ICONST(PHP_ROUND_HALF_ODD);

  DCONST(PI);
  DCONST(1_PI);
  DCONST(2_PI);
  DCONST(2_SQRTPI);
  DCONST(E);
  DCONST(EULER);
  DCONST(LN10);
  DCONST(LN2);
  DCONST(LNPI);
  DCONST(LOG10E);
  DCONST(LOG2E);
  DCONST(PI_2);
  DCONST(PI_4);
  DCONST(SQRT1_2);
  DCONST(SQRT2);
  DCONST(SQRT3);
  DCONST(SQRTPI);

  HHVM_FE(min);
  HHVM_FE(max);
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

  loadSystemlib("std_math");
}

///////////////////////////////////////////////////////////////////////////////
}
