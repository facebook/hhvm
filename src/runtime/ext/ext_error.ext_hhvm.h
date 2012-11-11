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
HPHP::Array HPHP::f_debug_backtrace(bool)
_ZN4HPHP17f_debug_backtraceEb

(return value) => rax
_rv => rdi
provide_object => rsi
*/

Value* fh_debug_backtrace(Value* _rv, bool provide_object) asm("_ZN4HPHP17f_debug_backtraceEb");

/*
void HPHP::f_debug_print_backtrace()
_ZN4HPHP23f_debug_print_backtraceEv

*/

void fh_debug_print_backtrace() asm("_ZN4HPHP23f_debug_print_backtraceEv");

/*
HPHP::Array HPHP::f_error_get_last()
_ZN4HPHP16f_error_get_lastEv

(return value) => rax
_rv => rdi
*/

Value* fh_error_get_last(Value* _rv) asm("_ZN4HPHP16f_error_get_lastEv");

/*
bool HPHP::f_error_log(HPHP::String const&, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_error_logERKNS_6StringEiS2_S2_

(return value) => rax
message => rdi
message_type => rsi
destination => rdx
extra_headers => rcx
*/

bool fh_error_log(Value* message, int message_type, Value* destination, Value* extra_headers) asm("_ZN4HPHP11f_error_logERKNS_6StringEiS2_S2_");

/*
long long HPHP::f_error_reporting(HPHP::Variant const&)
_ZN4HPHP17f_error_reportingERKNS_7VariantE

(return value) => rax
level => rdi
*/

long long fh_error_reporting(TypedValue* level) asm("_ZN4HPHP17f_error_reportingERKNS_7VariantE");

/*
bool HPHP::f_restore_error_handler()
_ZN4HPHP23f_restore_error_handlerEv

(return value) => rax
*/

bool fh_restore_error_handler() asm("_ZN4HPHP23f_restore_error_handlerEv");

/*
bool HPHP::f_restore_exception_handler()
_ZN4HPHP27f_restore_exception_handlerEv

(return value) => rax
*/

bool fh_restore_exception_handler() asm("_ZN4HPHP27f_restore_exception_handlerEv");

/*
HPHP::Variant HPHP::f_set_error_handler(HPHP::Variant const&, int)
_ZN4HPHP19f_set_error_handlerERKNS_7VariantEi

(return value) => rax
_rv => rdi
error_handler => rsi
error_types => rdx
*/

TypedValue* fh_set_error_handler(TypedValue* _rv, TypedValue* error_handler, int error_types) asm("_ZN4HPHP19f_set_error_handlerERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_set_exception_handler(HPHP::Variant const&)
_ZN4HPHP23f_set_exception_handlerERKNS_7VariantE

(return value) => rax
_rv => rdi
exception_handler => rsi
*/

TypedValue* fh_set_exception_handler(TypedValue* _rv, TypedValue* exception_handler) asm("_ZN4HPHP23f_set_exception_handlerERKNS_7VariantE");

/*
void HPHP::f_hphp_set_error_page(HPHP::String const&)
_ZN4HPHP21f_hphp_set_error_pageERKNS_6StringE

page => rdi
*/

void fh_hphp_set_error_page(Value* page) asm("_ZN4HPHP21f_hphp_set_error_pageERKNS_6StringE");

/*
void HPHP::f_hphp_throw_fatal_error(HPHP::String const&)
_ZN4HPHP24f_hphp_throw_fatal_errorERKNS_6StringE

error_msg => rdi
*/

void fh_hphp_throw_fatal_error(Value* error_msg) asm("_ZN4HPHP24f_hphp_throw_fatal_errorERKNS_6StringE");

/*
void HPHP::f_hphp_clear_unflushed()
_ZN4HPHP22f_hphp_clear_unflushedEv

*/

void fh_hphp_clear_unflushed() asm("_ZN4HPHP22f_hphp_clear_unflushedEv");

/*
HPHP::Array HPHP::f_hphp_debug_caller_info()
_ZN4HPHP24f_hphp_debug_caller_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphp_debug_caller_info(Value* _rv) asm("_ZN4HPHP24f_hphp_debug_caller_infoEv");

/*
bool HPHP::f_trigger_error(HPHP::String const&, int)
_ZN4HPHP15f_trigger_errorERKNS_6StringEi

(return value) => rax
error_msg => rdi
error_type => rsi
*/

bool fh_trigger_error(Value* error_msg, int error_type) asm("_ZN4HPHP15f_trigger_errorERKNS_6StringEi");

/*
bool HPHP::f_user_error(HPHP::String const&, int)
_ZN4HPHP12f_user_errorERKNS_6StringEi

(return value) => rax
error_msg => rdi
error_type => rsi
*/

bool fh_user_error(Value* error_msg, int error_type) asm("_ZN4HPHP12f_user_errorERKNS_6StringEi");


} // !HPHP

