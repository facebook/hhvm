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
HPHP::Variant HPHP::f_apache_note(HPHP::String const&, HPHP::String const&)
_ZN4HPHP13f_apache_noteERKNS_6StringES2_

(return value) => rax
_rv => rdi
note_name => rsi
note_value => rdx
*/

TypedValue* fh_apache_note(TypedValue* _rv, Value* note_name, Value* note_value) asm("_ZN4HPHP13f_apache_noteERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_apache_request_headers()
_ZN4HPHP24f_apache_request_headersEv

(return value) => rax
_rv => rdi
*/

Value* fh_apache_request_headers(Value* _rv) asm("_ZN4HPHP24f_apache_request_headersEv");

/*
HPHP::Array HPHP::f_apache_response_headers()
_ZN4HPHP25f_apache_response_headersEv

(return value) => rax
_rv => rdi
*/

Value* fh_apache_response_headers(Value* _rv) asm("_ZN4HPHP25f_apache_response_headersEv");

/*
bool HPHP::f_apache_setenv(HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP15f_apache_setenvERKNS_6StringES2_b

(return value) => rax
variable => rdi
value => rsi
walk_to_top => rdx
*/

bool fh_apache_setenv(Value* variable, Value* value, bool walk_to_top) asm("_ZN4HPHP15f_apache_setenvERKNS_6StringES2_b");

/*
HPHP::Array HPHP::f_getallheaders()
_ZN4HPHP15f_getallheadersEv

(return value) => rax
_rv => rdi
*/

Value* fh_getallheaders(Value* _rv) asm("_ZN4HPHP15f_getallheadersEv");

/*
bool HPHP::f_virtual(HPHP::String const&)
_ZN4HPHP9f_virtualERKNS_6StringE

(return value) => rax
filename => rdi
*/

bool fh_virtual(Value* filename) asm("_ZN4HPHP9f_virtualERKNS_6StringE");

/*
HPHP::Variant HPHP::f_apache_get_config()
_ZN4HPHP19f_apache_get_configEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_apache_get_config(TypedValue* _rv) asm("_ZN4HPHP19f_apache_get_configEv");

/*
HPHP::Variant HPHP::f_apache_get_scoreboard()
_ZN4HPHP23f_apache_get_scoreboardEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_apache_get_scoreboard(TypedValue* _rv) asm("_ZN4HPHP23f_apache_get_scoreboardEv");

/*
HPHP::Variant HPHP::f_apache_get_rewrite_rules()
_ZN4HPHP26f_apache_get_rewrite_rulesEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_apache_get_rewrite_rules(TypedValue* _rv) asm("_ZN4HPHP26f_apache_get_rewrite_rulesEv");


} // !HPHP

