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
bool HPHP::f_hphpd_install_user_command(HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_hphpd_install_user_commandERKNS_6StringES2_

(return value) => rax
cmd => rdi
clsname => rsi
*/

bool fh_hphpd_install_user_command(Value* cmd, Value* clsname) asm("_ZN4HPHP28f_hphpd_install_user_commandERKNS_6StringES2_");

/*
HPHP::Array HPHP::f_hphpd_get_user_commands()
_ZN4HPHP25f_hphpd_get_user_commandsEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphpd_get_user_commands(Value* _rv) asm("_ZN4HPHP25f_hphpd_get_user_commandsEv");

/*
void HPHP::f_hphpd_break(bool)
_ZN4HPHP13f_hphpd_breakEb

condition => rdi
*/

void fh_hphpd_break(bool condition) asm("_ZN4HPHP13f_hphpd_breakEb");

/*
HPHP::Variant HPHP::f_hphpd_get_client(HPHP::String const&)
_ZN4HPHP18f_hphpd_get_clientERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_hphpd_get_client(TypedValue* _rv, Value* name) asm("_ZN4HPHP18f_hphpd_get_clientERKNS_6StringE");

/*
HPHP::Variant HPHP::f_hphpd_client_ctrl(HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_hphpd_client_ctrlERKNS_6StringES2_

(return value) => rax
_rv => rdi
name => rsi
op => rdx
*/

TypedValue* fh_hphpd_client_ctrl(TypedValue* _rv, Value* name, Value* op) asm("_ZN4HPHP19f_hphpd_client_ctrlERKNS_6StringES2_");


} // !HPHP

