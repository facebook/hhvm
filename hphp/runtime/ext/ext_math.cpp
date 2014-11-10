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

#include "hphp/runtime/ext/ext_math.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/zend-multiply.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/system/constants.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const int64_t k_PHP_ROUND_HALF_UP =   PHP_ROUND_HALF_UP;
const int64_t k_PHP_ROUND_HALF_DOWN = PHP_ROUND_HALF_DOWN;
const int64_t k_PHP_ROUND_HALF_EVEN = PHP_ROUND_HALF_EVEN;
const int64_t k_PHP_ROUND_HALF_ODD =  PHP_ROUND_HALF_ODD;

double f_pi() { return k_M_PI;}

Variant f_min(int _argc, const Variant& value,
              const Variant& second /* = null_variant */,
              const Array& _argv /* = null_array */) {
  if (_argc == 1) {
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
  } else if (_argc == 2) {
    return less(second, value) ? second : value;
  }

  Variant ret = less(second, value) ? second : value;
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant currVal = iter.secondRef();
    if (less(currVal, ret)) {
      ret = currVal;
    }
  }
  return ret;
}

Variant f_max(int _argc, const Variant& value,
              const Variant& second /* = null_variant */,
              const Array& _argv /* = null_array */) {
  if (_argc == 1) {
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
  } else if (_argc == 2) {
    return more(second, value) ? second : value;
  }

  Variant ret = more(second, value) ? second : value;
  for (ArrayIter iter(_argv); iter; ++iter) {
    Variant currVal = iter.secondRef();
    if (more(currVal, ret)) {
      ret = currVal;
    }
  }
  return ret;
}

/* Logic based on zend_operators.c::convert_scalar_to_number() */
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

Variant f_abs(const Variant& number) {
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

bool f_is_finite(double val) { return finite(val);}
bool f_is_infinite(double val) { return isinf(val);}
bool f_is_nan(double val) { return isnan(val);}

Variant f_ceil(const Variant& number) {
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

Variant f_floor(const Variant& number) {
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

Variant f_round(const Variant& val, int64_t precision /* = 0 */,
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

double f_deg2rad(double number) { return number / 180.0 * k_M_PI;}
double f_rad2deg(double number) { return number / k_M_PI * 180.0;}

String f_decbin(int64_t number) {
  return string_long_to_base(number, 2);
}
String f_dechex(int64_t number) {
  return string_long_to_base(number, 16);
}
String f_decoct(int64_t number) {
  return string_long_to_base(number, 8);
}
Variant f_bindec(const String& binary_string) {
  return string_base_to_numeric(binary_string.data(), binary_string.size(), 2);
}
Variant f_hexdec(const String& hex_string) {
  return string_base_to_numeric(hex_string.data(), hex_string.size(), 16);
}
Variant f_octdec(const String& octal_string) {
  return string_base_to_numeric(octal_string.data(), octal_string.size(), 8);
}

Variant f_base_convert(const String& number, int64_t frombase, int64_t tobase) {
  if (!string_validate_base(frombase)) {
    throw_invalid_argument("Invalid frombase: %" PRId64, frombase);
    return false;
  }
  if (!string_validate_base(tobase)) {
    throw_invalid_argument("Invalid tobase: %" PRId64, tobase);
    return false;
  }
  Variant v = string_base_to_numeric(number.data(), number.size(), frombase);
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
      // Not reachable since f_pow() deals with these base cases first.
    case KindOfRef:
    case KindOfClass:
      break;
  }

  // Unknown data type.
  raise_error("Unsupported operand types");
  not_reached();
}

Variant f_pow(const Variant& base, const Variant& exp) {
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

  // We'll have already raised a notice in convert_for_pow
  // so avoid re-raising the notice here.
  auto const silent_val_to_double = [&](const Variant& v) {
    if (v.isObject() && !v.toObject()->castableToNumber()) {
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

double f_exp(double arg) { return exp(arg);}
double f_expm1(double arg) { return expm1(arg);}
double f_log10(double arg) { return log10(arg);}
double f_log1p(double number) { return log1p(number);}
double f_log(double arg, double base /* = 0 */) {
  return base <= 0 ? log(arg) : log(arg)/log(base);
}

double f_cos(double arg) { return cos(arg);  }
double f_cosh(double arg) { return cosh(arg); }
double f_sin(double arg) { return sin(arg);  }
double f_sinh(double arg) { return sinh(arg); }
double f_tan(double arg) { return tan(arg);  }
double f_tanh(double arg) { return tanh(arg); }
double f_acos(double arg) { return acos(arg); }
double f_acosh(double arg) { return acosh(arg);}
double f_asin(double arg) { return asin(arg); }
double f_asinh(double arg) { return asinh(arg);}
double f_atan(double arg) { return atan(arg); }
double f_atanh(double arg) { return atanh(arg);}
double f_atan2(double y, double x) { return atan2(y, x);}

double f_hypot(double x, double y) { return hypot(x, y);}
double f_fmod(double x, double y) { return fmod(x, y);}
double f_sqrt(double arg) { return sqrt(arg);}

int64_t f_getrandmax() { return RAND_MAX;}

///////////////////////////////////////////////////////////////////////////////

static bool s_rand_is_seeded = false;

void f_srand(const Variant& seed /* = null_variant */) {
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

int64_t f_rand(int64_t min /* = 0 */, int64_t max /* = RAND_MAX */) {
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

int64_t f_mt_getrandmax() { return MT_RAND_MAX;}

void f_mt_srand(const Variant& seed /* = null_variant */) {
  if (seed.isNull()) {
    return math_mt_srand(math_generate_seed());
  }
  if (seed.isNumeric(true)) {
    math_mt_srand(seed.toInt32());
  } else {
    raise_warning("mt_srand() expects parameter 1 to be long");
  }
}

int64_t f_mt_rand(int64_t min /* = 0 */, int64_t max /* = RAND_MAX */) {
  return math_mt_rand(min, max);
}
double f_lcg_value() { return math_combined_lcg();}

///////////////////////////////////////////////////////////////////////////////
}
