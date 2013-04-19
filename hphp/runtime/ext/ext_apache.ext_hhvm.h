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

TypedValue* fh_apache_note(TypedValue* _rv, Value* note_name, Value* note_value) asm("_ZN4HPHP13f_apache_noteERKNS_6StringES2_");

Value* fh_apache_request_headers(Value* _rv) asm("_ZN4HPHP24f_apache_request_headersEv");

Value* fh_apache_response_headers(Value* _rv) asm("_ZN4HPHP25f_apache_response_headersEv");

bool fh_apache_setenv(Value* variable, Value* value, bool walk_to_top) asm("_ZN4HPHP15f_apache_setenvERKNS_6StringES2_b");

Value* fh_getallheaders(Value* _rv) asm("_ZN4HPHP15f_getallheadersEv");

bool fh_virtual(Value* filename) asm("_ZN4HPHP9f_virtualERKNS_6StringE");

TypedValue* fh_apache_get_config(TypedValue* _rv) asm("_ZN4HPHP19f_apache_get_configEv");

TypedValue* fh_apache_get_scoreboard(TypedValue* _rv) asm("_ZN4HPHP23f_apache_get_scoreboardEv");

TypedValue* fh_apache_get_rewrite_rules(TypedValue* _rv) asm("_ZN4HPHP26f_apache_get_rewrite_rulesEv");

} // namespace HPHP
