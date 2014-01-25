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
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/zend-multiply.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// This is used by an opcode, so should not be ported to PHP
Variant HHVM_FUNCTION(abs, CVarRef num) {
  int64_t ival;
  double dval;
  DataType nt = num.toNumeric(ival, dval, true);
  if (nt != KindOfInt64 && nt != KindOfDouble) {
    if (num.isBoolean() || num.isNull() || num.isObject() || num.isResource() ||
      num.isString()) {
      nt = KindOfInt64;
      ival = num.toInt64();
    } else {
      return false;
    }
  }
  if (nt == KindOfInt64) {
    return (ival < 0) ? -ival : ival;
  } else {
    return fabs(dval);
  }
}

bool HHVM_FUNCTION(is_finite, double val) {
  return finite(val);
}

bool HHVM_FUNCTION(is_infinite, double val) {
  return isinf(val);
}

bool HHVM_FUNCTION(is_nan, double val) {
  return isnan(val);
}

double HHVM_FUNCTION(ceil, double number) {
  return ceil(number);
}

double HHVM_FUNCTION(floor, double number) {
  return floor(number);
}

double HHVM_FUNCTION(round, double val, int64_t precision /* = 0 */,
                            int64_t mode /* = PHP_ROUND_HALF_UP */) {
  return php_math_round(val, precision, mode);
}

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
  const String& number = tvCastToString(binary_string.asTypedValue());
  return string_base_to_numeric(number.data(), number.size(), 2);
}
Variant HHVM_FUNCTION(hexdec, CVarRef hex_string) {
  const String& number = tvCastToString(hex_string.asTypedValue());
  return string_base_to_numeric(number.data(), number.size(), 16);
}
Variant HHVM_FUNCTION(octdec, CVarRef octal_string) {
  const String& number = tvCastToString(octal_string.asTypedValue());
  return string_base_to_numeric(number.data(), number.size(), 8);
}

String HHVM_FUNCTION(base_convert, CVarRef num, int64_t frombase,
                                   int64_t tobase) {
  // We do the conversion here instead of through Native::coerceFCallArgs
  // so that it causes a fatal error instead of a warning.
  const String& number = tvCastToString(num.asTypedValue());
  if (!string_validate_base(frombase)) {
    throw_invalid_argument("Invalid frombase: %" PRId64, frombase);
    return false;
  }
  if (!string_validate_base(tobase)) {
    throw_invalid_argument("Invalid tobase: %" PRId64, tobase);
    return false;
  }
  Variant v = string_base_to_numeric(number.data(), number.size(), frombase);
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
  srand(seed.toInt32());
}

int64_t HHVM_FUNCTION(rand, int64_t min /* = 0 */,
                            int64_t max /* = -1 */) {
  if (!s_rand_is_seeded) {
    s_rand_is_seeded = true;
    srand(math_generate_seed());
  }

  if (max == -1) {
    max = HHVM_FN(getrandmax)();
  }

  int64_t number = rand();
  if (min != 0 || max != RAND_MAX) {
    RAND_RANGE(number, min, max, RAND_MAX);
  }
  return number;
}

int64_t HHVM_FUNCTION(mt_getrandmax) { return MT_RAND_MAX;}

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

int64_t HHVM_FUNCTION(mt_rand, int64_t min /* = 0 */,
                               int64_t max /* = -1 */) {
  if (max == -1) {
    max = HHVM_FN(mt_getrandmax)();
  }
  return math_mt_rand(min, max);
}
double HHVM_FUNCTION(lcg_value) { return math_combined_lcg();}

const StaticString s_PHP_ROUND_HALF_UP("PHP_ROUND_HALF_UP");
const StaticString s_PHP_ROUND_HALF_DOWN("PHP_ROUND_HALF_DOWN");
const StaticString s_PHP_ROUND_HALF_EVEN("PHP_ROUND_HALF_EVEN");
const StaticString s_PHP_ROUND_HALF_ODD("PHP_ROUND_HALF_ODD");

class mathExtension : public Extension {
 public:
  mathExtension() : Extension("math") {}
  virtual void moduleInit() {
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_UP.get(), PHP_ROUND_HALF_UP
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_DOWN.get(), PHP_ROUND_HALF_DOWN
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_EVEN.get(), PHP_ROUND_HALF_EVEN
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_ROUND_HALF_ODD.get(), PHP_ROUND_HALF_ODD
    );

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
} s_standard_math_extension;

///////////////////////////////////////////////////////////////////////////////
}
