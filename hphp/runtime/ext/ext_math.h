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

#ifndef incl_HPHP_EXT_MATH_H_
#define incl_HPHP_EXT_MATH_H_

#include "hphp/runtime/base/base-includes.h"
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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_ROUND_HALF_UP;
extern const int64_t k_PHP_ROUND_HALF_DOWN;
extern const int64_t k_PHP_ROUND_HALF_EVEN;
extern const int64_t k_PHP_ROUND_HALF_ODD;

double f_pi();

Variant f_min(int _argc, CVarRef value, CArrRef _argv = null_array);
Variant f_max(int _argc, CVarRef value, CArrRef _argv = null_array);
Variant f_abs(CVarRef number);

bool f_is_finite(double val);
bool f_is_infinite(double val);
bool f_is_nan(double val);

Variant f_ceil(CVarRef number);
Variant f_floor(CVarRef number);
Variant f_round(CVarRef val, int64_t precision = 0,
                int64_t mode = PHP_ROUND_HALF_UP);

double f_deg2rad(double number);
double f_rad2deg(double number);

// departure from PHP: not using "double" for these conversions
String f_decbin(int64_t number);
String f_dechex(int64_t number);
String f_decoct(int64_t number);
Variant f_bindec(const String& binary_string);
Variant f_hexdec(const String& hex_string);
Variant f_octdec(const String& octal_string);
Variant f_base_convert(const String& number, int64_t frombase, int64_t tobase);
Variant f_pow(CVarRef base, CVarRef exp);
double f_exp(double arg);
double f_expm1(double arg);
double f_log10(double arg);
double f_log1p(double number);
double f_log(double arg, double base = 0);

double f_cos(double arg);
double f_cosh(double arg);
double f_sin(double arg);
double f_sinh(double arg);
double f_tan(double arg);
double f_tanh(double arg);
double f_acos(double arg);
double f_acosh(double arg);
double f_asin(double arg);
double f_asinh(double arg);
double f_atan(double arg);
double f_atanh(double arg);
double f_atan2(double y, double x);

double f_hypot(double x, double y);
double f_fmod(double x, double y);
double f_sqrt(double arg);

int64_t f_getrandmax();
void f_srand(CVarRef seed = null_variant);
int64_t f_rand(int64_t min = 0, int64_t max = RAND_MAX);
int64_t f_mt_getrandmax();
void f_mt_srand(CVarRef seed = null_variant);
int64_t f_mt_rand(int64_t min = 0, int64_t max = RAND_MAX);
double f_lcg_value();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MATH_H_
