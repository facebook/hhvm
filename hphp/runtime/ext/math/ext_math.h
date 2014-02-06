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
#include "hphp/runtime/base/zend-math.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_PHP_ROUND_HALF_UP;
extern const int64_t k_PHP_ROUND_HALF_DOWN;
extern const int64_t k_PHP_ROUND_HALF_EVEN;
extern const int64_t k_PHP_ROUND_HALF_ODD;

double HHVM_FUNCTION(pi);

Variant HHVM_FUNCTION(abs, CVarRef number);
bool HHVM_FUNCTION(is_finite, double val);
bool HHVM_FUNCTION(is_infinite, double val);
bool HHVM_FUNCTION(is_nan, double val);
Variant HHVM_FUNCTION(ceil, CVarRef number);
Variant HHVM_FUNCTION(floor, CVarRef number);
Variant HHVM_FUNCTION(round, CVarRef val, int64_t precision = 0,
                int64_t mode = k_PHP_ROUND_HALF_UP);
double HHVM_FUNCTION(deg2rad, double number);
double HHVM_FUNCTION(rad2deg, double number);
String HHVM_FUNCTION(decbin, int64_t number);
String HHVM_FUNCTION(dechex, int64_t number);
String HHVM_FUNCTION(decoct, int64_t number);
Variant HHVM_FUNCTION(bindec, CVarRef binary_string);
Variant HHVM_FUNCTION(hexdec, CVarRef hex_string);
Variant HHVM_FUNCTION(octdec, CVarRef octal_string);
Variant HHVM_FUNCTION(base_convert, CVarRef number, int64_t frombase, int64_t tobase);
Variant HHVM_FUNCTION(pow, CVarRef base, CVarRef exp);
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
void HHVM_FUNCTION(srand, CVarRef seed = null_variant);
TypedValue* HHVM_FUNCTION(rand, ActRec* ar);
int64_t HHVM_FUNCTION(mt_getrandmax);
void HHVM_FUNCTION(mt_srand, CVarRef seed = null_variant);
TypedValue* HHVM_FUNCTION(mt_rand, ActRec* ar);
double HHVM_FUNCTION(lcg_value);

///////////////////////////////////////////////////////////////////////////////

int64_t hphp_rand(int64_t min = 0, int64_t max = RAND_MAX);
int64_t hphp_mt_rand(int64_t min = 0, int64_t max = MT_RAND_MAX);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_MATH_H_
