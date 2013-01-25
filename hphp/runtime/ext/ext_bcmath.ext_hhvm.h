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
bool HPHP::f_bcscale(long)
_ZN4HPHP9f_bcscaleEl

(return value) => rax
scale => rdi
*/

bool fh_bcscale(long scale) asm("_ZN4HPHP9f_bcscaleEl");

/*
HPHP::String HPHP::f_bcadd(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP7f_bcaddERKNS_6StringES2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
scale => rcx
*/

Value* fh_bcadd(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcaddERKNS_6StringES2_l");

/*
HPHP::String HPHP::f_bcsub(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP7f_bcsubERKNS_6StringES2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
scale => rcx
*/

Value* fh_bcsub(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcsubERKNS_6StringES2_l");

/*
long HPHP::f_bccomp(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP8f_bccompERKNS_6StringES2_l

(return value) => rax
left => rdi
right => rsi
scale => rdx
*/

long fh_bccomp(Value* left, Value* right, long scale) asm("_ZN4HPHP8f_bccompERKNS_6StringES2_l");

/*
HPHP::String HPHP::f_bcmul(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP7f_bcmulERKNS_6StringES2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
scale => rcx
*/

Value* fh_bcmul(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcmulERKNS_6StringES2_l");

/*
HPHP::String HPHP::f_bcdiv(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP7f_bcdivERKNS_6StringES2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
scale => rcx
*/

Value* fh_bcdiv(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcdivERKNS_6StringES2_l");

/*
HPHP::String HPHP::f_bcmod(HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_bcmodERKNS_6StringES2_

(return value) => rax
_rv => rdi
left => rsi
right => rdx
*/

Value* fh_bcmod(Value* _rv, Value* left, Value* right) asm("_ZN4HPHP7f_bcmodERKNS_6StringES2_");

/*
HPHP::String HPHP::f_bcpow(HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP7f_bcpowERKNS_6StringES2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
scale => rcx
*/

Value* fh_bcpow(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcpowERKNS_6StringES2_l");

/*
HPHP::Variant HPHP::f_bcpowmod(HPHP::String const&, HPHP::String const&, HPHP::String const&, long)
_ZN4HPHP10f_bcpowmodERKNS_6StringES2_S2_l

(return value) => rax
_rv => rdi
left => rsi
right => rdx
modulus => rcx
scale => r8
*/

TypedValue* fh_bcpowmod(TypedValue* _rv, Value* left, Value* right, Value* modulus, long scale) asm("_ZN4HPHP10f_bcpowmodERKNS_6StringES2_S2_l");

/*
HPHP::Variant HPHP::f_bcsqrt(HPHP::String const&, long)
_ZN4HPHP8f_bcsqrtERKNS_6StringEl

(return value) => rax
_rv => rdi
operand => rsi
scale => rdx
*/

TypedValue* fh_bcsqrt(TypedValue* _rv, Value* operand, long scale) asm("_ZN4HPHP8f_bcsqrtERKNS_6StringEl");


} // !HPHP

