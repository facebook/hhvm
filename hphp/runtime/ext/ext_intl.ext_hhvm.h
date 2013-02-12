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
long HPHP::f_intl_get_error_code()
_ZN4HPHP21f_intl_get_error_codeEv

(return value) => rax
*/

long fh_intl_get_error_code() asm("_ZN4HPHP21f_intl_get_error_codeEv");

/*
HPHP::String HPHP::f_intl_get_error_message()
_ZN4HPHP24f_intl_get_error_messageEv

(return value) => rax
_rv => rdi
*/

Value* fh_intl_get_error_message(Value* _rv) asm("_ZN4HPHP24f_intl_get_error_messageEv");

/*
HPHP::String HPHP::f_intl_error_name(long)
_ZN4HPHP17f_intl_error_nameEl

(return value) => rax
_rv => rdi
error_code => rsi
*/

Value* fh_intl_error_name(Value* _rv, long error_code) asm("_ZN4HPHP17f_intl_error_nameEl");

/*
bool HPHP::f_intl_is_failure(long)
_ZN4HPHP17f_intl_is_failureEl

(return value) => rax
error_code => rdi
*/

bool fh_intl_is_failure(long error_code) asm("_ZN4HPHP17f_intl_is_failureEl");

/*
HPHP::Variant HPHP::f_collator_asort(HPHP::Variant const&, HPHP::VRefParamValue const&, long)
_ZN4HPHP16f_collator_asortERKNS_7VariantERKNS_14VRefParamValueEl

(return value) => rax
_rv => rdi
obj => rsi
arr => rdx
sort_flag => rcx
*/

TypedValue* fh_collator_asort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP16f_collator_asortERKNS_7VariantERKNS_14VRefParamValueEl");

/*
HPHP::Variant HPHP::f_collator_compare(HPHP::Variant const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18f_collator_compareERKNS_7VariantERKNS_6StringES5_

(return value) => rax
_rv => rdi
obj => rsi
str1 => rdx
str2 => rcx
*/

TypedValue* fh_collator_compare(TypedValue* _rv, TypedValue* obj, Value* str1, Value* str2) asm("_ZN4HPHP18f_collator_compareERKNS_7VariantERKNS_6StringES5_");

/*
HPHP::Variant HPHP::f_collator_create(HPHP::String const&)
_ZN4HPHP17f_collator_createERKNS_6StringE

(return value) => rax
_rv => rdi
locale => rsi
*/

TypedValue* fh_collator_create(TypedValue* _rv, Value* locale) asm("_ZN4HPHP17f_collator_createERKNS_6StringE");

/*
HPHP::Variant HPHP::f_collator_get_attribute(HPHP::Variant const&, long)
_ZN4HPHP24f_collator_get_attributeERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
attr => rdx
*/

TypedValue* fh_collator_get_attribute(TypedValue* _rv, TypedValue* obj, long attr) asm("_ZN4HPHP24f_collator_get_attributeERKNS_7VariantEl");

/*
HPHP::Variant HPHP::f_collator_get_error_code(HPHP::Variant const&)
_ZN4HPHP25f_collator_get_error_codeERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_collator_get_error_code(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP25f_collator_get_error_codeERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_collator_get_error_message(HPHP::Variant const&)
_ZN4HPHP28f_collator_get_error_messageERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_collator_get_error_message(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP28f_collator_get_error_messageERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_collator_get_locale(HPHP::Variant const&, long)
_ZN4HPHP21f_collator_get_localeERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
type => rdx
*/

TypedValue* fh_collator_get_locale(TypedValue* _rv, TypedValue* obj, long type) asm("_ZN4HPHP21f_collator_get_localeERKNS_7VariantEl");

/*
HPHP::Variant HPHP::f_collator_get_strength(HPHP::Variant const&)
_ZN4HPHP23f_collator_get_strengthERKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_collator_get_strength(TypedValue* _rv, TypedValue* obj) asm("_ZN4HPHP23f_collator_get_strengthERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_collator_set_attribute(HPHP::Variant const&, long, long)
_ZN4HPHP24f_collator_set_attributeERKNS_7VariantEll

(return value) => rax
_rv => rdi
obj => rsi
attr => rdx
val => rcx
*/

TypedValue* fh_collator_set_attribute(TypedValue* _rv, TypedValue* obj, long attr, long val) asm("_ZN4HPHP24f_collator_set_attributeERKNS_7VariantEll");

/*
HPHP::Variant HPHP::f_collator_set_strength(HPHP::Variant const&, long)
_ZN4HPHP23f_collator_set_strengthERKNS_7VariantEl

(return value) => rax
_rv => rdi
obj => rsi
strength => rdx
*/

TypedValue* fh_collator_set_strength(TypedValue* _rv, TypedValue* obj, long strength) asm("_ZN4HPHP23f_collator_set_strengthERKNS_7VariantEl");

/*
HPHP::Variant HPHP::f_collator_sort_with_sort_keys(HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP30f_collator_sort_with_sort_keysERKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
obj => rsi
arr => rdx
*/

TypedValue* fh_collator_sort_with_sort_keys(TypedValue* _rv, TypedValue* obj, TypedValue* arr) asm("_ZN4HPHP30f_collator_sort_with_sort_keysERKNS_7VariantERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_collator_sort(HPHP::Variant const&, HPHP::VRefParamValue const&, long)
_ZN4HPHP15f_collator_sortERKNS_7VariantERKNS_14VRefParamValueEl

(return value) => rax
_rv => rdi
obj => rsi
arr => rdx
sort_flag => rcx
*/

TypedValue* fh_collator_sort(TypedValue* _rv, TypedValue* obj, TypedValue* arr, long sort_flag) asm("_ZN4HPHP15f_collator_sortERKNS_7VariantERKNS_14VRefParamValueEl");

/*
HPHP::Variant HPHP::f_idn_to_ascii(HPHP::String const&, long, long, HPHP::VRefParamValue const&)
_ZN4HPHP14f_idn_to_asciiERKNS_6StringEllRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
domain => rsi
options => rdx
variant => rcx
idna_info => r8
*/

TypedValue* fh_idn_to_ascii(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP14f_idn_to_asciiERKNS_6StringEllRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_idn_to_unicode(HPHP::String const&, long, long, HPHP::VRefParamValue const&)
_ZN4HPHP16f_idn_to_unicodeERKNS_6StringEllRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
domain => rsi
options => rdx
variant => rcx
idna_info => r8
*/

TypedValue* fh_idn_to_unicode(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP16f_idn_to_unicodeERKNS_6StringEllRKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_idn_to_utf8(HPHP::String const&, long, long, HPHP::VRefParamValue const&)
_ZN4HPHP13f_idn_to_utf8ERKNS_6StringEllRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
domain => rsi
options => rdx
variant => rcx
idna_info => r8
*/

TypedValue* fh_idn_to_utf8(TypedValue* _rv, Value* domain, long options, long variant, TypedValue* idna_info) asm("_ZN4HPHP13f_idn_to_utf8ERKNS_6StringEllRKNS_14VRefParamValueE");


} // !HPHP

