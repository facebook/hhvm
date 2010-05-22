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

#ifndef __EXT_MATH_H__
#define __EXT_MATH_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_math.h>
#include <math.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline double f_pi() { return k_M_PI;}

Variant f_min(int _argc, CVarRef value, CArrRef _argv = null_array);
Variant f_max(int _argc, CVarRef value, CArrRef _argv = null_array);
Variant f_abs(CVarRef number);

inline bool f_is_finite(double val) { return finite(val);}
inline bool f_is_infinite(double val) { return isinf(val);}
inline bool f_is_nan(double val) { return isnan(val);}

inline double f_ceil(double value) { return ceil(value);}
inline double f_floor(double value) { return floor(value);}
double f_round(CVarRef val, int64 precision = 0);

inline double f_deg2rad(double number) { return number / 180.0 * k_M_PI;}
inline double f_rad2deg(double number) { return number / k_M_PI * 180.0;}

// departure from PHP: not using "double" for these conversions
inline String f_decbin(int64 number) {
  return String(string_long_to_base(number, 2), AttachString);
}
inline String f_dechex(int64 number) {
  return String(string_long_to_base(number, 16), AttachString);
}
inline String f_decoct(int64 number) {
  return String(string_long_to_base(number, 8), AttachString);
}
inline Variant f_bindec(CStrRef binary_string) {
  return string_base_to_numeric(binary_string.data(), binary_string.size(), 2);
}
inline Variant f_hexdec(CStrRef hex_string) {
  return string_base_to_numeric(hex_string.data(), hex_string.size(), 16);
}
inline Variant f_octdec(CStrRef octal_string) {
  return string_base_to_numeric(octal_string.data(), octal_string.size(), 8);
}
Variant f_base_convert(CStrRef number, int64 frombase, int64 tobase);
Numeric f_pow(CVarRef base, CVarRef exp);
inline double f_exp(double arg) { return exp(arg);}
inline double f_expm1(double arg) { return expm1(arg);}
inline double f_log10(double arg) { return log10(arg);}
inline double f_log1p(double number) { return log1p(number);}
inline double f_log(double arg, double base = 0) {
  return base <= 0 ? log(arg) : log(arg)/log(base);
}

inline double f_cos(double arg) { return cos(arg);  }
inline double f_cosh(double arg) { return cosh(arg); }
inline double f_sin(double arg) { return sin(arg);  }
inline double f_sinh(double arg) { return sinh(arg); }
inline double f_tan(double arg) { return tan(arg);  }
inline double f_tanh(double arg) { return tanh(arg); }
inline double f_acos(double arg) { return acos(arg); }
inline double f_acosh(double arg) { return acosh(arg);}
inline double f_asin(double arg) { return asin(arg); }
inline double f_asinh(double arg) { return asinh(arg);}
inline double f_atan(double arg) { return atan(arg); }
inline double f_atanh(double arg) { return atanh(arg);}
inline double f_atan2(double y, double x) { return atan2(y, x);}

inline double f_hypot(double x, double y) { return hypot(x, y);}
inline double f_fmod(double x, double y) { return fmod(x, y);}
inline double f_sqrt(double arg) { return sqrt(arg);}

inline int64 f_getrandmax() { return RAND_MAX;}
void f_srand(CVarRef seed = null_variant);
int64 f_rand(int64 min = 0, int64 max = RAND_MAX);
inline int64 f_mt_getrandmax() { return MT_RAND_MAX;}
void f_mt_srand(CVarRef seed = null_variant);
inline int64 f_mt_rand(int64 min = 0, int64 max = RAND_MAX) {
  return math_mt_rand(min, max);
}
inline double f_lcg_value() { return math_combined_lcg();}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_MATH_H__
