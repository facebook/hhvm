/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/zend-multiply.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/ext/std/ext_std.h"
#include "hphp/zend/zend-math.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(min, const Variant& value, const Array& args) {
  if (args.empty()) {
    const auto& cell_value = *value.asTypedValue();
    if (UNLIKELY(!isContainer(cell_value))) {
      return value;
    }

    ArrayIter iter(cell_value);
    if (!iter) {
      return init_null();
    }
    auto ret = iter.secondValPlus();
    ++iter;
    for (; iter; ++iter) {
      auto const cur = iter.secondValPlus();
      if (tvLess(cur, ret)) {
        ret = cur;
      }
    }
    return Variant(VarNR(ret));
  }

  auto ret = *value.asTypedValue();
  for (ArrayIter iter(args); iter; ++iter) {
    auto const cur = iter.secondVal();
    if (tvLess(cur, ret)) {
      ret = cur;
    }
  }
  return Variant(VarNR(ret));
}

Variant HHVM_FUNCTION(max, const Variant& value, const Array& args) {
  if (args.empty()) {
    const auto& cell_value = *value.asTypedValue();
    if (UNLIKELY(!isContainer(cell_value))) {
      return value;
    }

    ArrayIter iter(cell_value);
    if (!iter) {
      return init_null();
    }
    auto ret = iter.secondValPlus();
    ++iter;
    for (; iter; ++iter) {
      auto const cur = iter.secondValPlus();
      if (tvGreater(cur, ret)) {
        ret = cur;
      }
    }
    return Variant(VarNR(ret));
  }

  auto ret = *value.asTypedValue();
  for (ArrayIter iter(args); iter; ++iter) {
    auto const cur = iter.secondVal();
    if (tvGreater(cur, ret)) {
      ret = cur;
    }
  }
  return Variant(VarNR(ret));
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

bool HHVM_FUNCTION(is_finite, double val) { return std::isfinite(val);}
bool HHVM_FUNCTION(is_infinite, double val) { return std::isinf(val);}
bool HHVM_FUNCTION(is_nan, double val) { return std::isnan(val);}

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
    raise_invalid_argument_warning("Invalid frombase: %" PRId64, frombase);
    return false;
  }
  if (!string_validate_base(tobase)) {
    raise_invalid_argument_warning("Invalid tobase: %" PRId64, tobase);
    return false;
  }
  String str = number.toString();
  Variant v = string_base_to_numeric(str.data(), str.size(), frombase);
  return string_numeric_to_base(v, tobase);
}

Variant HHVM_FUNCTION(pow, const Variant& base, const Variant& exp) {
  if (!base.isNumeric() || !exp.isNumeric()) {
    throwMathBadTypesException(base.asTypedValue(), exp.asTypedValue());
  }

  if (base.isInteger() && exp.isInteger() && exp.asInt64Val() >= 0) {
    int64_t bint = base.asInt64Val(), eint = exp.asInt64Val();
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

  auto const as_double = [](const Variant& v) -> double {
    return v.isDouble() ? v.getDouble() : v.getInt64();
  };

  return pow(as_double(base), as_double(exp));
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
    Uninit = 0,
    RequestInit
  } state;
};

RDS_LOCAL(RandomBuf, rl_state);

static void randinit(uint32_t seed) {
  if (rl_state->state == RandomBuf::Uninit) {
    initstate_r(seed, rl_state->buf, sizeof rl_state->buf, &rl_state->data);
  } else {
    srandom_r(seed, &rl_state->data);
  }
  rl_state->state = RandomBuf::RequestInit;
}

void HHVM_FUNCTION(srand, const Variant& seed /* = uninit_variant */) {
  if (seed.isNull()) {
    randinit(math_generate_seed());
    return;
  }
  if (seed.isNumeric(true)) {
    randinit((int)seed.toInt64());
  } else {
    raise_warning("srand() expects parameter 1 to be long");
  }
}

int64_t HHVM_FUNCTION(rand,
                      int64_t min /* = 0 */,
                      const Variant& max /* = uninit_variant */) {
  if (rl_state->state != RandomBuf::RequestInit) {
    randinit(math_generate_seed());
  }

  int64_t number;
  int32_t numberIn;
  random_r(&rl_state->data, &numberIn);
  number = numberIn;
  int64_t int_max = max.isNull() ? RAND_MAX : max.toInt64();
  if (min != 0 || int_max != RAND_MAX) {
    RAND_RANGE(number, min, int_max, RAND_MAX);
  }
  return number;
}

int64_t HHVM_FUNCTION(mt_getrandmax) { return MT_RAND_MAX;}

void HHVM_FUNCTION(mt_srand,
                   const Variant& seed /* = uninit_variant */) {
  if (seed.isNull()) {
    return math_mt_srand(math_generate_seed());
  }
  if (seed.isNumeric(true)) {
    math_mt_srand((int)seed.toInt64());
  } else {
    raise_warning("mt_srand() expects parameter 1 to be long");
  }
}

int64_t HHVM_FUNCTION(mt_rand,
                      int64_t min /* = 0 */,
                      const Variant& max /* = uninit_variant */) {
  return math_mt_rand(min, max.isNull() ? RAND_MAX : max.toInt64());
}

double HHVM_FUNCTION(lcg_value) { return math_combined_lcg();}

Variant HHVM_FUNCTION(intdiv, int64_t numerator, int64_t divisor) {
  if (divisor == 0) {
    SystemLib::throwDivisionByZeroErrorObject(Strings::DIVISION_BY_ZERO);
  } else if (divisor == -1 &&
             numerator == std::numeric_limits<int64_t>::min()) {
    SystemLib::throwArithmeticErrorObject(
      "Division of PHP_INT_MIN by -1 is not an integer");
  }
  return numerator/divisor;
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::requestInitMath() {
  if (rl_state->state == RandomBuf::RequestInit) {
    rl_state->state = RandomBuf::Uninit;
  }
}

void StandardExtension::registerNativeMath() {
  HHVM_RC_INT_SAME(PHP_ROUND_HALF_UP);
  HHVM_RC_INT_SAME(PHP_ROUND_HALF_DOWN);
  HHVM_RC_INT_SAME(PHP_ROUND_HALF_EVEN);
  HHVM_RC_INT_SAME(PHP_ROUND_HALF_ODD);

  HHVM_FE(min);
  HHVM_FE(max);
  HHVM_FE(abs);
  HHVM_FE(is_finite);
  HHVM_FE(is_infinite);
  HHVM_FE(is_nan);
  HHVM_FE(ceil);
  HHVM_FE(floor);
  HHVM_FE(round);
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
  HHVM_FE(intdiv);
}

///////////////////////////////////////////////////////////////////////////////
}
