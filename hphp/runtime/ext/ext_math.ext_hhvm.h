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

/*
double HPHP::f_pi()
_ZN4HPHP4f_piEv

(return value) => xmm0
*/

double fh_pi() asm("_ZN4HPHP4f_piEv");

/*
HPHP::Variant HPHP::f_min(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_min(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_max(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_max(TypedValue* _rv, int64_t _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_abs(HPHP::Variant const&)
_ZN4HPHP5f_absERKNS_7VariantE

(return value) => rax
_rv => rdi
number => rsi
*/

TypedValue* fh_abs(TypedValue* _rv, TypedValue* number) asm("_ZN4HPHP5f_absERKNS_7VariantE");

/*
bool HPHP::f_is_finite(double)
_ZN4HPHP11f_is_finiteEd

(return value) => rax
val => xmm0
*/

bool fh_is_finite(double val) asm("_ZN4HPHP11f_is_finiteEd");

/*
bool HPHP::f_is_infinite(double)
_ZN4HPHP13f_is_infiniteEd

(return value) => rax
val => xmm0
*/

bool fh_is_infinite(double val) asm("_ZN4HPHP13f_is_infiniteEd");

/*
bool HPHP::f_is_nan(double)
_ZN4HPHP8f_is_nanEd

(return value) => rax
val => xmm0
*/

bool fh_is_nan(double val) asm("_ZN4HPHP8f_is_nanEd");

/*
double HPHP::f_ceil(double)
_ZN4HPHP6f_ceilEd

(return value) => xmm0
value => xmm0
*/

double fh_ceil(double value) asm("_ZN4HPHP6f_ceilEd");

/*
double HPHP::f_floor(double)
_ZN4HPHP7f_floorEd

(return value) => xmm0
value => xmm0
*/

double fh_floor(double value) asm("_ZN4HPHP7f_floorEd");

/*
double HPHP::f_round(HPHP::Variant const&, long, long)
_ZN4HPHP7f_roundERKNS_7VariantEll

(return value) => xmm0
val => rdi
precision => rsi
mode => rdx
*/

double fh_round(TypedValue* val, long precision, long mode) asm("_ZN4HPHP7f_roundERKNS_7VariantEll");

/*
double HPHP::f_deg2rad(double)
_ZN4HPHP9f_deg2radEd

(return value) => xmm0
number => xmm0
*/

double fh_deg2rad(double number) asm("_ZN4HPHP9f_deg2radEd");

/*
double HPHP::f_rad2deg(double)
_ZN4HPHP9f_rad2degEd

(return value) => xmm0
number => xmm0
*/

double fh_rad2deg(double number) asm("_ZN4HPHP9f_rad2degEd");

/*
HPHP::String HPHP::f_decbin(long)
_ZN4HPHP8f_decbinEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decbin(Value* _rv, long number) asm("_ZN4HPHP8f_decbinEl");

/*
HPHP::String HPHP::f_dechex(long)
_ZN4HPHP8f_dechexEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_dechex(Value* _rv, long number) asm("_ZN4HPHP8f_dechexEl");

/*
HPHP::String HPHP::f_decoct(long)
_ZN4HPHP8f_decoctEl

(return value) => rax
_rv => rdi
number => rsi
*/

Value* fh_decoct(Value* _rv, long number) asm("_ZN4HPHP8f_decoctEl");

/*
HPHP::Variant HPHP::f_bindec(HPHP::String const&)
_ZN4HPHP8f_bindecERKNS_6StringE

(return value) => rax
_rv => rdi
binary_string => rsi
*/

TypedValue* fh_bindec(TypedValue* _rv, Value* binary_string) asm("_ZN4HPHP8f_bindecERKNS_6StringE");

/*
HPHP::Variant HPHP::f_hexdec(HPHP::String const&)
_ZN4HPHP8f_hexdecERKNS_6StringE

(return value) => rax
_rv => rdi
hex_string => rsi
*/

TypedValue* fh_hexdec(TypedValue* _rv, Value* hex_string) asm("_ZN4HPHP8f_hexdecERKNS_6StringE");

/*
HPHP::Variant HPHP::f_octdec(HPHP::String const&)
_ZN4HPHP8f_octdecERKNS_6StringE

(return value) => rax
_rv => rdi
octal_string => rsi
*/

TypedValue* fh_octdec(TypedValue* _rv, Value* octal_string) asm("_ZN4HPHP8f_octdecERKNS_6StringE");

/*
HPHP::Variant HPHP::f_base_convert(HPHP::String const&, long, long)
_ZN4HPHP14f_base_convertERKNS_6StringEll

(return value) => rax
_rv => rdi
number => rsi
frombase => rdx
tobase => rcx
*/

TypedValue* fh_base_convert(TypedValue* _rv, Value* number, long frombase, long tobase) asm("_ZN4HPHP14f_base_convertERKNS_6StringEll");

/*
HPHP::Variant HPHP::f_pow(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP5f_powERKNS_7VariantES2_

(return value) => rax
_rv => rdi
base => rsi
exp => rdx
*/

TypedValue* fh_pow(TypedValue* _rv, TypedValue* base, TypedValue* exp) asm("_ZN4HPHP5f_powERKNS_7VariantES2_");

/*
double HPHP::f_exp(double)
_ZN4HPHP5f_expEd

(return value) => xmm0
arg => xmm0
*/

double fh_exp(double arg) asm("_ZN4HPHP5f_expEd");

/*
double HPHP::f_expm1(double)
_ZN4HPHP7f_expm1Ed

(return value) => xmm0
arg => xmm0
*/

double fh_expm1(double arg) asm("_ZN4HPHP7f_expm1Ed");

/*
double HPHP::f_log10(double)
_ZN4HPHP7f_log10Ed

(return value) => xmm0
arg => xmm0
*/

double fh_log10(double arg) asm("_ZN4HPHP7f_log10Ed");

/*
double HPHP::f_log1p(double)
_ZN4HPHP7f_log1pEd

(return value) => xmm0
number => xmm0
*/

double fh_log1p(double number) asm("_ZN4HPHP7f_log1pEd");

/*
double HPHP::f_log(double, double)
_ZN4HPHP5f_logEdd

(return value) => xmm0
arg => xmm0
base => xmm1
*/

double fh_log(double arg, double base) asm("_ZN4HPHP5f_logEdd");

/*
double HPHP::f_cos(double)
_ZN4HPHP5f_cosEd

(return value) => xmm0
arg => xmm0
*/

double fh_cos(double arg) asm("_ZN4HPHP5f_cosEd");

/*
double HPHP::f_cosh(double)
_ZN4HPHP6f_coshEd

(return value) => xmm0
arg => xmm0
*/

double fh_cosh(double arg) asm("_ZN4HPHP6f_coshEd");

/*
double HPHP::f_sin(double)
_ZN4HPHP5f_sinEd

(return value) => xmm0
arg => xmm0
*/

double fh_sin(double arg) asm("_ZN4HPHP5f_sinEd");

/*
double HPHP::f_sinh(double)
_ZN4HPHP6f_sinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_sinh(double arg) asm("_ZN4HPHP6f_sinhEd");

/*
double HPHP::f_tan(double)
_ZN4HPHP5f_tanEd

(return value) => xmm0
arg => xmm0
*/

double fh_tan(double arg) asm("_ZN4HPHP5f_tanEd");

/*
double HPHP::f_tanh(double)
_ZN4HPHP6f_tanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_tanh(double arg) asm("_ZN4HPHP6f_tanhEd");

/*
double HPHP::f_acos(double)
_ZN4HPHP6f_acosEd

(return value) => xmm0
arg => xmm0
*/

double fh_acos(double arg) asm("_ZN4HPHP6f_acosEd");

/*
double HPHP::f_acosh(double)
_ZN4HPHP7f_acoshEd

(return value) => xmm0
arg => xmm0
*/

double fh_acosh(double arg) asm("_ZN4HPHP7f_acoshEd");

/*
double HPHP::f_asin(double)
_ZN4HPHP6f_asinEd

(return value) => xmm0
arg => xmm0
*/

double fh_asin(double arg) asm("_ZN4HPHP6f_asinEd");

/*
double HPHP::f_asinh(double)
_ZN4HPHP7f_asinhEd

(return value) => xmm0
arg => xmm0
*/

double fh_asinh(double arg) asm("_ZN4HPHP7f_asinhEd");

/*
double HPHP::f_atan(double)
_ZN4HPHP6f_atanEd

(return value) => xmm0
arg => xmm0
*/

double fh_atan(double arg) asm("_ZN4HPHP6f_atanEd");

/*
double HPHP::f_atanh(double)
_ZN4HPHP7f_atanhEd

(return value) => xmm0
arg => xmm0
*/

double fh_atanh(double arg) asm("_ZN4HPHP7f_atanhEd");

/*
double HPHP::f_atan2(double, double)
_ZN4HPHP7f_atan2Edd

(return value) => xmm0
y => xmm0
x => xmm1
*/

double fh_atan2(double y, double x) asm("_ZN4HPHP7f_atan2Edd");

/*
double HPHP::f_hypot(double, double)
_ZN4HPHP7f_hypotEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_hypot(double x, double y) asm("_ZN4HPHP7f_hypotEdd");

/*
double HPHP::f_fmod(double, double)
_ZN4HPHP6f_fmodEdd

(return value) => xmm0
x => xmm0
y => xmm1
*/

double fh_fmod(double x, double y) asm("_ZN4HPHP6f_fmodEdd");

/*
double HPHP::f_sqrt(double)
_ZN4HPHP6f_sqrtEd

(return value) => xmm0
arg => xmm0
*/

double fh_sqrt(double arg) asm("_ZN4HPHP6f_sqrtEd");

/*
long HPHP::f_getrandmax()
_ZN4HPHP12f_getrandmaxEv

(return value) => rax
*/

long fh_getrandmax() asm("_ZN4HPHP12f_getrandmaxEv");

/*
void HPHP::f_srand(HPHP::Variant const&)
_ZN4HPHP7f_srandERKNS_7VariantE

seed => rdi
*/

void fh_srand(TypedValue* seed) asm("_ZN4HPHP7f_srandERKNS_7VariantE");

/*
long HPHP::f_rand(long, long)
_ZN4HPHP6f_randEll

(return value) => rax
min => rdi
max => rsi
*/

long fh_rand(long min, long max) asm("_ZN4HPHP6f_randEll");

/*
long HPHP::f_mt_getrandmax()
_ZN4HPHP15f_mt_getrandmaxEv

(return value) => rax
*/

long fh_mt_getrandmax() asm("_ZN4HPHP15f_mt_getrandmaxEv");

/*
void HPHP::f_mt_srand(HPHP::Variant const&)
_ZN4HPHP10f_mt_srandERKNS_7VariantE

seed => rdi
*/

void fh_mt_srand(TypedValue* seed) asm("_ZN4HPHP10f_mt_srandERKNS_7VariantE");

/*
long HPHP::f_mt_rand(long, long)
_ZN4HPHP9f_mt_randEll

(return value) => rax
min => rdi
max => rsi
*/

long fh_mt_rand(long min, long max) asm("_ZN4HPHP9f_mt_randEll");

/*
double HPHP::f_lcg_value()
_ZN4HPHP11f_lcg_valueEv

(return value) => xmm0
*/

double fh_lcg_value() asm("_ZN4HPHP11f_lcg_valueEv");


} // !HPHP

