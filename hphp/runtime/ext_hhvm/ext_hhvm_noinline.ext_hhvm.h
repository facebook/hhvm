/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
HPHP::Variant HPHP::fni_call_user_func_array(HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
function => rsi
params => rdx
*/

TypedValue* fh_call_user_func_array(TypedValue* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP24fni_call_user_func_arrayERKNS_7VariantERKNS_5ArrayE");

/*
bool HPHP::fni_is_bool(HPHP::Variant const&)
_ZN4HPHP11fni_is_boolERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP11fni_is_boolERKNS_7VariantE");

/*
bool HPHP::fni_is_int(HPHP::Variant const&)
_ZN4HPHP10fni_is_intERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP10fni_is_intERKNS_7VariantE");

/*
bool HPHP::fni_is_integer(HPHP::Variant const&)
_ZN4HPHP14fni_is_integerERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP14fni_is_integerERKNS_7VariantE");

/*
bool HPHP::fni_is_long(HPHP::Variant const&)
_ZN4HPHP11fni_is_longERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP11fni_is_longERKNS_7VariantE");

/*
bool HPHP::fni_is_double(HPHP::Variant const&)
_ZN4HPHP13fni_is_doubleERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP13fni_is_doubleERKNS_7VariantE");

/*
bool HPHP::fni_is_float(HPHP::Variant const&)
_ZN4HPHP12fni_is_floatERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP12fni_is_floatERKNS_7VariantE");

/*
bool HPHP::fni_is_numeric(HPHP::Variant const&)
_ZN4HPHP14fni_is_numericERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP14fni_is_numericERKNS_7VariantE");

/*
bool HPHP::fni_is_real(HPHP::Variant const&)
_ZN4HPHP11fni_is_realERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP11fni_is_realERKNS_7VariantE");

/*
bool HPHP::fni_is_string(HPHP::Variant const&)
_ZN4HPHP13fni_is_stringERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP13fni_is_stringERKNS_7VariantE");

/*
bool HPHP::fni_is_scalar(HPHP::Variant const&)
_ZN4HPHP13fni_is_scalarERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP13fni_is_scalarERKNS_7VariantE");

/*
bool HPHP::fni_is_array(HPHP::Variant const&)
_ZN4HPHP12fni_is_arrayERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP12fni_is_arrayERKNS_7VariantE");

/*
bool HPHP::fni_is_resource(HPHP::Variant const&)
_ZN4HPHP15fni_is_resourceERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP15fni_is_resourceERKNS_7VariantE");

/*
bool HPHP::fni_is_null(HPHP::Variant const&)
_ZN4HPHP11fni_is_nullERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP11fni_is_nullERKNS_7VariantE");

/*
HPHP::Variant HPHP::fni_unserialize(HPHP::String const&)
_ZN4HPHP15fni_unserializeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_unserialize(TypedValue* _rv, Value* str) asm("_ZN4HPHP15fni_unserializeERKNS_6StringE");


} // !HPHP

