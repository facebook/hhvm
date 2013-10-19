/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_APD_H_
#define incl_HPHP_EXT_APD_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_override_function(
  const String& name, const String& args, const String& code);
bool f_rename_function(const String& orig_name, const String& new_name);
void f_apd_set_browser_trace();
String f_apd_set_pprof_trace(
  const String& dumpdir = null_string, const String& frament = null_string);
bool f_apd_set_session_trace_socket(
  const String& ip_or_filename, int domain, int port, int mask);
void f_apd_stop_trace();
bool f_apd_breakpoint();
bool f_apd_continue();
bool f_apd_echo(const String& output);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_APD_H_
