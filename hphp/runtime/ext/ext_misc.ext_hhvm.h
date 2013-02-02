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
long long HPHP::f_connection_aborted()
_ZN4HPHP20f_connection_abortedEv

(return value) => rax
*/

long long fh_connection_aborted() asm("_ZN4HPHP20f_connection_abortedEv");

/*
long long HPHP::f_connection_status()
_ZN4HPHP19f_connection_statusEv

(return value) => rax
*/

long long fh_connection_status() asm("_ZN4HPHP19f_connection_statusEv");

/*
long long HPHP::f_connection_timeout()
_ZN4HPHP20f_connection_timeoutEv

(return value) => rax
*/

long long fh_connection_timeout() asm("_ZN4HPHP20f_connection_timeoutEv");

/*
HPHP::Variant HPHP::f_constant(HPHP::String const&)
_ZN4HPHP10f_constantERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_constant(TypedValue* _rv, Value* name) asm("_ZN4HPHP10f_constantERKNS_6StringE");

/*
bool HPHP::f_define(HPHP::String const&, HPHP::Variant const&, bool)
_ZN4HPHP8f_defineERKNS_6StringERKNS_7VariantEb

(return value) => rax
name => rdi
value => rsi
case_insensitive => rdx
*/

bool fh_define(Value* name, TypedValue* value, bool case_insensitive) asm("_ZN4HPHP8f_defineERKNS_6StringERKNS_7VariantEb");

/*
bool HPHP::f_defined(HPHP::String const&, bool)
_ZN4HPHP9f_definedERKNS_6StringEb

(return value) => rax
name => rdi
autoload => rsi
*/

bool fh_defined(Value* name, bool autoload) asm("_ZN4HPHP9f_definedERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_die(HPHP::Variant const&)
_ZN4HPHP5f_dieERKNS_7VariantE

(return value) => rax
_rv => rdi
status => rsi
*/

TypedValue* fh_die(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP5f_dieERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_exit(HPHP::Variant const&)
_ZN4HPHP6f_exitERKNS_7VariantE

(return value) => rax
_rv => rdi
status => rsi
*/

TypedValue* fh_exit(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP6f_exitERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_eval(HPHP::String const&)
_ZN4HPHP6f_evalERKNS_6StringE

(return value) => rax
_rv => rdi
code_str => rsi
*/

TypedValue* fh_eval(TypedValue* _rv, Value* code_str) asm("_ZN4HPHP6f_evalERKNS_6StringE");

/*
HPHP::Variant HPHP::f_get_browser(HPHP::String const&, bool)
_ZN4HPHP13f_get_browserERKNS_6StringEb

(return value) => rax
_rv => rdi
user_agent => rsi
return_array => rdx
*/

TypedValue* fh_get_browser(TypedValue* _rv, Value* user_agent, bool return_array) asm("_ZN4HPHP13f_get_browserERKNS_6StringEb");

/*
void HPHP::f___halt_compiler()
_ZN4HPHP17f___halt_compilerEv

*/

void fh___halt_compiler() asm("_ZN4HPHP17f___halt_compilerEv");

/*
HPHP::Variant HPHP::f_highlight_file(HPHP::String const&, bool)
_ZN4HPHP16f_highlight_fileERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
ret => rdx
*/

TypedValue* fh_highlight_file(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP16f_highlight_fileERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_show_source(HPHP::String const&, bool)
_ZN4HPHP13f_show_sourceERKNS_6StringEb

(return value) => rax
_rv => rdi
filename => rsi
ret => rdx
*/

TypedValue* fh_show_source(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP13f_show_sourceERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_highlight_string(HPHP::String const&, bool)
_ZN4HPHP18f_highlight_stringERKNS_6StringEb

(return value) => rax
_rv => rdi
str => rsi
ret => rdx
*/

TypedValue* fh_highlight_string(TypedValue* _rv, Value* str, bool ret) asm("_ZN4HPHP18f_highlight_stringERKNS_6StringEb");

/*
long long HPHP::f_ignore_user_abort(bool)
_ZN4HPHP19f_ignore_user_abortEb

(return value) => rax
setting => rdi
*/

long long fh_ignore_user_abort(bool setting) asm("_ZN4HPHP19f_ignore_user_abortEb");

/*
HPHP::Variant HPHP::f_pack(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP6f_packEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

TypedValue* fh_pack(TypedValue* _rv, long long _argc, Value* format, Value* _argv) asm("_ZN4HPHP6f_packEiRKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_php_check_syntax(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP18f_php_check_syntaxERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
filename => rdi
error_message => rsi
*/

bool fh_php_check_syntax(Value* filename, TypedValue* error_message) asm("_ZN4HPHP18f_php_check_syntaxERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::String HPHP::f_php_strip_whitespace(HPHP::String const&)
_ZN4HPHP22f_php_strip_whitespaceERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

Value* fh_php_strip_whitespace(Value* _rv, Value* filename) asm("_ZN4HPHP22f_php_strip_whitespaceERKNS_6StringE");

/*
long long HPHP::f_sleep(int)
_ZN4HPHP7f_sleepEi

(return value) => rax
seconds => rdi
*/

long long fh_sleep(int seconds) asm("_ZN4HPHP7f_sleepEi");

/*
void HPHP::f_usleep(int)
_ZN4HPHP8f_usleepEi

micro_seconds => rdi
*/

void fh_usleep(int micro_seconds) asm("_ZN4HPHP8f_usleepEi");

/*
HPHP::Variant HPHP::f_time_nanosleep(int, int)
_ZN4HPHP16f_time_nanosleepEii

(return value) => rax
_rv => rdi
seconds => rsi
nanoseconds => rdx
*/

TypedValue* fh_time_nanosleep(TypedValue* _rv, int seconds, int nanoseconds) asm("_ZN4HPHP16f_time_nanosleepEii");

/*
bool HPHP::f_time_sleep_until(double)
_ZN4HPHP18f_time_sleep_untilEd

(return value) => rax
timestamp => xmm0
*/

bool fh_time_sleep_until(double timestamp) asm("_ZN4HPHP18f_time_sleep_untilEd");

/*
HPHP::String HPHP::f_uniqid(HPHP::String const&, bool)
_ZN4HPHP8f_uniqidERKNS_6StringEb

(return value) => rax
_rv => rdi
prefix => rsi
more_entropy => rdx
*/

Value* fh_uniqid(Value* _rv, Value* prefix, bool more_entropy) asm("_ZN4HPHP8f_uniqidERKNS_6StringEb");

/*
HPHP::Variant HPHP::f_unpack(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8f_unpackERKNS_6StringES2_

(return value) => rax
_rv => rdi
format => rsi
data => rdx
*/

TypedValue* fh_unpack(TypedValue* _rv, Value* format, Value* data) asm("_ZN4HPHP8f_unpackERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_sys_getloadavg()
_ZN4HPHP16f_sys_getloadavgEv

(return value) => rax
_rv => rdi
*/

Value* fh_sys_getloadavg(Value* _rv) asm("_ZN4HPHP16f_sys_getloadavgEv");

/*
HPHP::Array HPHP::f_token_get_all(HPHP::String const&)
_ZN4HPHP15f_token_get_allERKNS_6StringE

(return value) => rax
_rv => rdi
source => rsi
*/

Value* fh_token_get_all(Value* _rv, Value* source) asm("_ZN4HPHP15f_token_get_allERKNS_6StringE");

/*
HPHP::String HPHP::f_token_name(long long)
_ZN4HPHP12f_token_nameEx

(return value) => rax
_rv => rdi
token => rsi
*/

Value* fh_token_name(Value* _rv, long long token) asm("_ZN4HPHP12f_token_nameEx");

/*
HPHP::Variant HPHP::f_hphp_process_abort(HPHP::Variant const&)
_ZN4HPHP20f_hphp_process_abortERKNS_7VariantE

(return value) => rax
_rv => rdi
magic => rsi
*/

TypedValue* fh_hphp_process_abort(TypedValue* _rv, TypedValue* magic) asm("_ZN4HPHP20f_hphp_process_abortERKNS_7VariantE");

/*
HPHP::String HPHP::f_hphp_to_string(HPHP::Variant const&)
_ZN4HPHP16f_hphp_to_stringERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_hphp_to_string(Value* _rv, TypedValue* v) asm("_ZN4HPHP16f_hphp_to_stringERKNS_7VariantE");


} // !HPHP

