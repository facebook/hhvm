/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_MATH_H_
#define incl_HPHP_EXT_MATH_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-math.h"
#include <math.h>

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

#ifdef __CYGWIN__
#include <cmath>
#define isinf std::isinf
#define isnan std::isnan
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_ROUND_HALF_UP;
extern const int64_t k_PHP_ROUND_HALF_DOWN;
extern const int64_t k_PHP_ROUND_HALF_EVEN;
extern const int64_t k_PHP_ROUND_HALF_ODD;

Variant HHVM_FUNCTION(min, const Variant& value, const Array& args);
Variant HHVM_FUNCTION(max, const Variant& value, const Array& args);
Variant HHVM_FUNCTION(abs, const Variant& number);

bool HHVM_FUNCTION(is_finite, double val);
bool HHVM_FUNCTION(is_infinite, double val);
bool HHVM_FUNCTION(is_nan, double val);

Variant HHVM_FUNCTION(ceil, const Variant& number);
Variant HHVM_FUNCTION(floor, const Variant& number);
Variant HHVM_FUNCTION(round,
                      const Variant& val,
                      int64_t precision = 0,
                      int64_t mode = PHP_ROUND_HALF_UP);

double HHVM_FUNCTION(deg2rad, double number);
double HHVM_FUNCTION(rad2deg, double number);

// departure from PHP: not using "double" for these conversions
String HHVM_FUNCTION(decbin, const Variant& number);
String HHVM_FUNCTION(dechex, const Variant& number);
String HHVM_FUNCTION(decoct, const Variant& number);
Variant HHVM_FUNCTION(bindec, const Variant& binary_string);
Variant HHVM_FUNCTION(hexdec, const Variant& hex_string);
Variant HHVM_FUNCTION(octdec, const Variant& octal_string);
Variant HHVM_FUNCTION(base_convert,
                      const Variant& number,
                      int64_t frombase,
                      int64_t tobase);
Variant HHVM_FUNCTION(pow, const Variant& base, const Variant& exp);
double HHVM_FUNCTION(exp, double arg);
double HHVM_FUNCTION(expm1, double arg);
double HHVM_FUNCTION(log10, double arg);
double HHVM_FUNCTION(log1p, double number);
double HHVM_FUNCTION(log, double arg, double base = 0);

double HHVM_FUNCTION(cos, double arg);
double HHVM_FUNCTION(cosh, double arg);
double HHVM_FUNCTION(sin, double arg);
double HHVM_FUNCTION(sinh, double arg);
double HHVM_FUNCTION(tan, double arg);
double HHVM_FUNCTION(tanh, double arg);
double HHVM_FUNCTION(acos, double arg);
double HHVM_FUNCTION(acosh, double arg);
double HHVM_FUNCTION(asin, double arg);
double HHVM_FUNCTION(asinh, double arg);
double HHVM_FUNCTION(atan, double arg);
double HHVM_FUNCTION(atanh, double arg);
double HHVM_FUNCTION(atan2, double y, double x);

double HHVM_FUNCTION(hypot, double x, double y);
double HHVM_FUNCTION(fmod, double x, double y);
double HHVM_FUNCTION(sqrt, double arg);

int64_t HHVM_FUNCTION(getrandmax);
void HHVM_FUNCTION(srand, const Variant& seed = null_variant);
int64_t HHVM_FUNCTION(rand,
                      int64_t min = 0,
                      const Variant& max = null_variant);
int64_t HHVM_FUNCTION(mt_getrandmax);
void HHVM_FUNCTION(mt_srand,
                   const Variant& seed = null_variant);
int64_t HHVM_FUNCTION(mt_rand,
                      int64_t min = 0,
                      const Variant& max = null_variant);
double HHVM_FUNCTION(lcg_value);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MATH_H_
