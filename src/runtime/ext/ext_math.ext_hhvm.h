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
HPHP::Variant HPHP::f_min(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_min(TypedValue* _rv, long long _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_minEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_max(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
value => rdx
_argv => rcx
*/

TypedValue* fh_max(TypedValue* _rv, long long _argc, TypedValue* value, Value* _argv) asm("_ZN4HPHP5f_maxEiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_abs(HPHP::Variant const&)
_ZN4HPHP5f_absERKNS_7VariantE

(return value) => rax
_rv => rdi
number => rsi
*/

TypedValue* fh_abs(TypedValue* _rv, TypedValue* number) asm("_ZN4HPHP5f_absERKNS_7VariantE");

/*
double HPHP::f_round(HPHP::Variant const&, long long)
_ZN4HPHP7f_roundERKNS_7VariantEx

(return value) => xmm0
val => rdi
precision => rsi
*/

double fh_round(TypedValue* val, long long precision) asm("_ZN4HPHP7f_roundERKNS_7VariantEx");

/*
HPHP::Variant HPHP::f_base_convert(HPHP::String const&, long long, long long)
_ZN4HPHP14f_base_convertERKNS_6StringExx

(return value) => rax
_rv => rdi
number => rsi
frombase => rdx
tobase => rcx
*/

TypedValue* fh_base_convert(TypedValue* _rv, Value* number, long long frombase, long long tobase) asm("_ZN4HPHP14f_base_convertERKNS_6StringExx");

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
void HPHP::f_srand(HPHP::Variant const&)
_ZN4HPHP7f_srandERKNS_7VariantE

seed => rdi
*/

void fh_srand(TypedValue* seed) asm("_ZN4HPHP7f_srandERKNS_7VariantE");

/*
long long HPHP::f_rand(long long, long long)
_ZN4HPHP6f_randExx

(return value) => rax
min => rdi
max => rsi
*/

long long fh_rand(long long min, long long max) asm("_ZN4HPHP6f_randExx");

/*
void HPHP::f_mt_srand(HPHP::Variant const&)
_ZN4HPHP10f_mt_srandERKNS_7VariantE

seed => rdi
*/

void fh_mt_srand(TypedValue* seed) asm("_ZN4HPHP10f_mt_srandERKNS_7VariantE");


} // !HPHP

