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

bool fh_bcscale(long scale) asm("_ZN4HPHP9f_bcscaleEl");

Value* fh_bcadd(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcaddERKNS_6StringES2_l");

Value* fh_bcsub(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcsubERKNS_6StringES2_l");

long fh_bccomp(Value* left, Value* right, long scale) asm("_ZN4HPHP8f_bccompERKNS_6StringES2_l");

Value* fh_bcmul(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcmulERKNS_6StringES2_l");

Value* fh_bcdiv(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcdivERKNS_6StringES2_l");

Value* fh_bcmod(Value* _rv, Value* left, Value* right) asm("_ZN4HPHP7f_bcmodERKNS_6StringES2_");

Value* fh_bcpow(Value* _rv, Value* left, Value* right, long scale) asm("_ZN4HPHP7f_bcpowERKNS_6StringES2_l");

TypedValue* fh_bcpowmod(TypedValue* _rv, Value* left, Value* right, Value* modulus, long scale) asm("_ZN4HPHP10f_bcpowmodERKNS_6StringES2_S2_l");

TypedValue* fh_bcsqrt(TypedValue* _rv, Value* operand, long scale) asm("_ZN4HPHP8f_bcsqrtERKNS_6StringEl");

} // namespace HPHP
