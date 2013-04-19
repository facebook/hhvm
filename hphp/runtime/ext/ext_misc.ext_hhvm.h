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

long fh_connection_aborted() asm("_ZN4HPHP20f_connection_abortedEv");

long fh_connection_status() asm("_ZN4HPHP19f_connection_statusEv");

long fh_connection_timeout() asm("_ZN4HPHP20f_connection_timeoutEv");

TypedValue* fh_constant(TypedValue* _rv, Value* name) asm("_ZN4HPHP10f_constantERKNS_6StringE");

bool fh_define(Value* name, TypedValue* value, bool case_insensitive) asm("_ZN4HPHP8f_defineERKNS_6StringERKNS_7VariantEb");

bool fh_defined(Value* name, bool autoload) asm("_ZN4HPHP9f_definedERKNS_6StringEb");

TypedValue* fh_die(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP5f_dieERKNS_7VariantE");

TypedValue* fh_exit(TypedValue* _rv, TypedValue* status) asm("_ZN4HPHP6f_exitERKNS_7VariantE");

TypedValue* fh_eval(TypedValue* _rv, Value* code_str) asm("_ZN4HPHP6f_evalERKNS_6StringE");

TypedValue* fh_get_browser(TypedValue* _rv, Value* user_agent, bool return_array) asm("_ZN4HPHP13f_get_browserERKNS_6StringEb");

void fh___halt_compiler() asm("_ZN4HPHP17f___halt_compilerEv");

TypedValue* fh_highlight_file(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP16f_highlight_fileERKNS_6StringEb");

TypedValue* fh_show_source(TypedValue* _rv, Value* filename, bool ret) asm("_ZN4HPHP13f_show_sourceERKNS_6StringEb");

TypedValue* fh_highlight_string(TypedValue* _rv, Value* str, bool ret) asm("_ZN4HPHP18f_highlight_stringERKNS_6StringEb");

long fh_ignore_user_abort(bool setting) asm("_ZN4HPHP19f_ignore_user_abortEb");

TypedValue* fh_pack(TypedValue* _rv, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP6f_packEiRKNS_6StringERKNS_5ArrayE");

bool fh_php_check_syntax(Value* filename, TypedValue* error_message) asm("_ZN4HPHP18f_php_check_syntaxERKNS_6StringERKNS_14VRefParamValueE");

Value* fh_php_strip_whitespace(Value* _rv, Value* filename) asm("_ZN4HPHP22f_php_strip_whitespaceERKNS_6StringE");

long fh_sleep(int seconds) asm("_ZN4HPHP7f_sleepEi");

void fh_usleep(int micro_seconds) asm("_ZN4HPHP8f_usleepEi");

TypedValue* fh_time_nanosleep(TypedValue* _rv, int seconds, int nanoseconds) asm("_ZN4HPHP16f_time_nanosleepEii");

bool fh_time_sleep_until(double timestamp) asm("_ZN4HPHP18f_time_sleep_untilEd");

Value* fh_uniqid(Value* _rv, Value* prefix, bool more_entropy) asm("_ZN4HPHP8f_uniqidERKNS_6StringEb");

TypedValue* fh_unpack(TypedValue* _rv, Value* format, Value* data) asm("_ZN4HPHP8f_unpackERKNS_6StringES2_");

Value* fh_sys_getloadavg(Value* _rv) asm("_ZN4HPHP16f_sys_getloadavgEv");

Value* fh_token_get_all(Value* _rv, Value* source) asm("_ZN4HPHP15f_token_get_allERKNS_6StringE");

Value* fh_token_name(Value* _rv, long token) asm("_ZN4HPHP12f_token_nameEl");

TypedValue* fh_hphp_process_abort(TypedValue* _rv, TypedValue* magic) asm("_ZN4HPHP20f_hphp_process_abortERKNS_7VariantE");

Value* fh_hphp_to_string(Value* _rv, TypedValue* v) asm("_ZN4HPHP16f_hphp_to_stringERKNS_7VariantE");

} // namespace HPHP
