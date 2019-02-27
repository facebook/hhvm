/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_EXT_DEBUGGER_H_
#define incl_HPHP_EXT_DEBUGGER_H_

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(hphpd_auth_token);
String HHVM_FUNCTION(hphp_debug_session_auth);
void HHVM_FUNCTION(hphpd_break, bool condition = true);
bool HHVM_FUNCTION(hphp_debug_break, bool condition = true);
bool HHVM_FUNCTION(hphp_debugger_attached);
bool HHVM_FUNCTION(hphp_debugger_set_option, const String& option, bool value);
bool HHVM_FUNCTION(hphp_debugger_get_option, const String& option);
Array HHVM_FUNCTION(debugger_get_info);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DEBUGGER_H_
