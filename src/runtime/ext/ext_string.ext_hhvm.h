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
HPHP::String HPHP::f_implode(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP9f_implodeERKNS_7VariantES2_

(return value) => rax
_rv => rdi
arg1 => rsi
arg2 => rdx
*/

Value* fh_implode(Value* _rv, TypedValue* arg1, TypedValue* arg2) asm("_ZN4HPHP9f_implodeERKNS_7VariantES2_");

/*
HPHP::Variant HPHP::f_strtok(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
str => rsi
token => rdx
*/

TypedValue* fh_strtok(TypedValue* _rv, Value* str, TypedValue* token) asm("_ZN4HPHP8f_strtokERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_str_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_replace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP13f_str_replaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_str_ireplace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
search => rsi
replace => rdx
subject => rcx
count => r8
*/

TypedValue* fh_str_ireplace(TypedValue* _rv, TypedValue* search, TypedValue* replace, TypedValue* subject, TypedValue* count) asm("_ZN4HPHP14f_str_ireplaceERKNS_7VariantES2_S2_RKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_substr_replace(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_

(return value) => rax
_rv => rdi
str => rsi
replacement => rdx
start => rcx
length => r8
*/

TypedValue* fh_substr_replace(TypedValue* _rv, TypedValue* str, TypedValue* replacement, TypedValue* start, TypedValue* length) asm("_ZN4HPHP16f_substr_replaceERKNS_7VariantES2_S2_S2_");

/*
HPHP::Variant HPHP::f_strtr(HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

TypedValue* fh_strtr(TypedValue* _rv, Value* str, TypedValue* from, TypedValue* to) asm("_ZN4HPHP7f_strtrERKNS_6StringERKNS_7VariantES5_");

/*
HPHP::String HPHP::f_convert_cyr_string(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
str => rsi
from => rdx
to => rcx
*/

Value* fh_convert_cyr_string(Value* _rv, Value* str, Value* from, Value* to) asm("_ZN4HPHP20f_convert_cyr_stringERKNS_6StringES2_S2_");

/*
HPHP::String HPHP::f_hebrev(HPHP::String const&, int)
_ZN4HPHP8f_hebrevERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrev(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP8f_hebrevERKNS_6StringEi");

/*
HPHP::String HPHP::f_hebrevc(HPHP::String const&, int)
_ZN4HPHP9f_hebrevcERKNS_6StringEi

(return value) => rax
_rv => rdi
hebrew_text => rsi
max_chars_per_line => rdx
*/

Value* fh_hebrevc(Value* _rv, Value* hebrew_text, int max_chars_per_line) asm("_ZN4HPHP9f_hebrevcERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_setlocale(int, int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
category => rdx
locale => rcx
_argv => r8
*/

TypedValue* fh_setlocale(TypedValue* _rv, long long _argc, int category, TypedValue* locale, Value* _argv) asm("_ZN4HPHP11f_setlocaleEiiRKNS_7VariantERKNS_5ArrayE");

/*
HPHP::Array HPHP::f_localeconv()
_ZN4HPHP12f_localeconvEv

(return value) => rax
_rv => rdi
*/

Value* fh_localeconv(Value* _rv) asm("_ZN4HPHP12f_localeconvEv");

/*
HPHP::Variant HPHP::f_sscanf(int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
str => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_sscanf(TypedValue* _rv, long long _argc, Value* str, Value* format, Value* _argv) asm("_ZN4HPHP8f_sscanfEiRKNS_6StringES2_RKNS_5ArrayE");

/*
HPHP::String HPHP::f_number_format(double, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_

(return value) => rax
_rv => rdi
number => xmm0
decimals => rsi
dec_point => rdx
thousands_sep => rcx
*/

Value* fh_number_format(Value* _rv, double number, int decimals, Value* dec_point, Value* thousands_sep) asm("_ZN4HPHP15f_number_formatEdiRKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_substr_compare(HPHP::String const&, HPHP::String const&, int, int, bool)
_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib

(return value) => rax
_rv => rdi
main_str => rsi
str => rdx
offset => rcx
length => r8
case_insensitivity => r9
*/

TypedValue* fh_substr_compare(TypedValue* _rv, Value* main_str, Value* str, int offset, int length, bool case_insensitivity) asm("_ZN4HPHP16f_substr_compareERKNS_6StringES2_iib");

/*
HPHP::Variant HPHP::f_strrchr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_strrchr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_strrchrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_strstr(HPHP::String const&, HPHP::Variant const&, bool)
_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
before_needle => rcx
*/

TypedValue* fh_strstr(TypedValue* _rv, Value* haystack, TypedValue* needle, bool before_needle) asm("_ZN4HPHP8f_strstrERKNS_6StringERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_stristr(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
*/

TypedValue* fh_stristr(TypedValue* _rv, Value* haystack, TypedValue* needle) asm("_ZN4HPHP9f_stristrERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_strpbrk(HPHP::String const&, HPHP::String const&)
_ZN4HPHP9f_strpbrkERKNS_6StringES2_

(return value) => rax
_rv => rdi
haystack => rsi
char_list => rdx
*/

TypedValue* fh_strpbrk(TypedValue* _rv, Value* haystack, Value* char_list) asm("_ZN4HPHP9f_strpbrkERKNS_6StringES2_");

/*
HPHP::Variant HPHP::f_strpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP8f_strposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_stripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_stripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_striposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_strrpos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strrpos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP9f_strrposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_strripos(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
*/

TypedValue* fh_strripos(TypedValue* _rv, Value* haystack, TypedValue* needle, int offset) asm("_ZN4HPHP10f_strriposERKNS_6StringERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_substr_count(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP14f_substr_countERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
length => r8
*/

TypedValue* fh_substr_count(TypedValue* _rv, Value* haystack, Value* needle, int offset, int length) asm("_ZN4HPHP14f_substr_countERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_strspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP8f_strspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP8f_strspnERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_strcspn(HPHP::String const&, HPHP::String const&, int, int)
_ZN4HPHP9f_strcspnERKNS_6StringES2_ii

(return value) => rax
_rv => rdi
str1 => rsi
str2 => rdx
start => rcx
length => r8
*/

TypedValue* fh_strcspn(TypedValue* _rv, Value* str1, Value* str2, int start, int length) asm("_ZN4HPHP9f_strcspnERKNS_6StringES2_ii");

/*
HPHP::Variant HPHP::f_count_chars(HPHP::String const&, long long)
_ZN4HPHP13f_count_charsERKNS_6StringEx

(return value) => rax
_rv => rdi
str => rsi
mode => rdx
*/

TypedValue* fh_count_chars(TypedValue* _rv, Value* str, long long mode) asm("_ZN4HPHP13f_count_charsERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_str_word_count(HPHP::String const&, long long, HPHP::String const&)
_ZN4HPHP16f_str_word_countERKNS_6StringExS2_

(return value) => rax
_rv => rdi
str => rsi
format => rdx
charlist => rcx
*/

TypedValue* fh_str_word_count(TypedValue* _rv, Value* str, long long format, Value* charlist) asm("_ZN4HPHP16f_str_word_countERKNS_6StringExS2_");

/*
void HPHP::f_parse_str(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE

str => rdi
arr => rsi
*/

void fh_parse_str(Value* str, TypedValue* arr) asm("_ZN4HPHP11f_parse_strERKNS_6StringERKNS_14VRefParamValueE");


} // !HPHP

