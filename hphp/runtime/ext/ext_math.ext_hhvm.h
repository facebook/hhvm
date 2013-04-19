/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

double fh_pi() asm("_ZN4HPHP4f_piEv");

TypedValue* fh_min(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_max(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fh_abs(TypedValue* _rv, TypedValue* number) asm("_ZN4HPHP5f_absERKNS_7VariantE");

bool fh_is_finite(double val) asm("_ZN4HPHP11f_is_finiteEd");

bool fh_is_infinite(double val) asm("_ZN4HPHP13f_is_infiniteEd");

bool fh_is_nan(double val) asm("_ZN4HPHP8f_is_nanEd");

double fh_ceil(double value) asm("_ZN4HPHP6f_ceilEd");

double fh_floor(double value) asm("_ZN4HPHP7f_floorEd");

double fh_round(TypedValue* val, long precision, long mode) asm("_ZN4HPHP7f_roundERKNS_7VariantEll");

double fh_deg2rad(double number) asm("_ZN4HPHP9f_deg2radEd");

double fh_rad2deg(double number) asm("_ZN4HPHP9f_rad2degEd");

Value* fh_decbin(Value* _rv, long number) asm("_ZN4HPHP8f_decbinEl");

Value* fh_dechex(Value* _rv, long number) asm("_ZN4HPHP8f_dechexEl");

Value* fh_decoct(Value* _rv, long number) asm("_ZN4HPHP8f_decoctEl");

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP8f_bindecERKNS_6StringE");

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP8f_hexdecERKNS_6StringE");

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP8f_octdecERKNS_6StringE");

TypedValue* fh_base_convert(TypedValue* _rv, Value* number, long frombase, long tobase) asm("_ZN4HPHP14f_base_convertERKNS_6StringEll");

TypedValue* fh_pow(TypedValue* _rv, TypedValue* base, TypedValue* exp) asm("_ZN4HPHP5f_powERKNS_7VariantES2_");

double fh_exp(double arg) asm("_ZN4HPHP5f_expEd");

double fh_expm1(double arg) asm("_ZN4HPHP7f_expm1Ed");

double fh_log10(double arg) asm("_ZN4HPHP7f_log10Ed");

double fh_log1p(double number) asm("_ZN4HPHP7f_log1pEd");

double fh_log(double arg, double base) asm("_ZN4HPHP5f_logEdd");

double fh_cos(double arg) asm("_ZN4HPHP5f_cosEd");

double fh_cosh(double arg) asm("_ZN4HPHP6f_coshEd");

double fh_sin(double arg) asm("_ZN4HPHP5f_sinEd");

double fh_sinh(double arg) asm("_ZN4HPHP6f_sinhEd");

double fh_tan(double arg) asm("_ZN4HPHP5f_tanEd");

double fh_tanh(double arg) asm("_ZN4HPHP6f_tanhEd");

double fh_acos(double arg) asm("_ZN4HPHP6f_acosEd");

double fh_acosh(double arg) asm("_ZN4HPHP7f_acoshEd");

double fh_asin(double arg) asm("_ZN4HPHP6f_asinEd");

double fh_asinh(double arg) asm("_ZN4HPHP7f_asinhEd");

double fh_atan(double arg) asm("_ZN4HPHP6f_atanEd");

double fh_atanh(double arg) asm("_ZN4HPHP7f_atanhEd");

double fh_atan2(double y, double x) asm("_ZN4HPHP7f_atan2Edd");

double fh_hypot(double x, double y) asm("_ZN4HPHP7f_hypotEdd");

double fh_fmod(double x, double y) asm("_ZN4HPHP6f_fmodEdd");

double fh_sqrt(double arg) asm("_ZN4HPHP6f_sqrtEd");

long fh_getrandmax() asm("_ZN4HPHP12f_getrandmaxEv");

void fh_srand(TypedValue* seed) asm("_ZN4HPHP7f_srandERKNS_7VariantE");

long fh_rand(long min, long max) asm("_ZN4HPHP6f_randEll");

long fh_mt_getrandmax() asm("_ZN4HPHP15f_mt_getrandmaxEv");

void fh_mt_srand(TypedValue* seed) asm("_ZN4HPHP10f_mt_srandERKNS_7VariantE");

long fh_mt_rand(long min, long max) asm("_ZN4HPHP9f_mt_randEll");

double fh_lcg_value() asm("_ZN4HPHP11f_lcg_valueEv");

} // namespace HPHP
