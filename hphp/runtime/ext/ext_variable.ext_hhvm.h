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
bool HPHP::f_is_object(HPHP::Variant const&)
_ZN4HPHP11f_is_objectERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_object(TypedValue* var) asm("_ZN4HPHP11f_is_objectERKNS_7VariantE");

/*
HPHP::String HPHP::f_gettype(HPHP::Variant const&)
_ZN4HPHP9f_gettypeERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_gettype(Value* _rv, TypedValue* v) asm("_ZN4HPHP9f_gettypeERKNS_7VariantE");

/*
HPHP::String HPHP::f_get_resource_type(HPHP::Object const&)
_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

Value* fh_get_resource_type(Value* _rv, Value* handle) asm("_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE");

/*
bool HPHP::f_settype(HPHP::VRefParamValue const&, HPHP::String const&)
_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE

(return value) => rax
var => rdi
type => rsi
*/

bool fh_settype(TypedValue* var, Value* type) asm("_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE");

/*
HPHP::Variant HPHP::f_print_r(HPHP::Variant const&, bool)
_ZN4HPHP9f_print_rERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_print_r(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP9f_print_rERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_var_export(HPHP::Variant const&, bool)
_ZN4HPHP12f_var_exportERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_var_export(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP12f_var_exportERKNS_7VariantEb");

/*
void HPHP::f_var_dump(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
expression => rsi
_argv => rdx
*/

void fh_var_dump(long long _argc, TypedValue* expression, Value* _argv) asm("_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE");

/*
void HPHP::f_debug_zval_dump(HPHP::Variant const&)
_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE

variable => rdi
*/

void fh_debug_zval_dump(TypedValue* variable) asm("_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE");

/*
HPHP::Array HPHP::f_get_defined_vars()
_ZN4HPHP18f_get_defined_varsEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_defined_vars(Value* _rv) asm("_ZN4HPHP18f_get_defined_varsEv");

/*
bool HPHP::f_import_request_variables(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_

(return value) => rax
types => rdi
prefix => rsi
*/

bool fh_import_request_variables(Value* types, Value* prefix) asm("_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_");

/*
long long HPHP::f_extract(HPHP::Array const&, int, HPHP::String const&)
_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE

(return value) => rax
var_array => rdi
extract_type => rsi
prefix => rdx
*/

long long fh_extract(Value* var_array, int extract_type, Value* prefix) asm("_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE");


} // !HPHP

