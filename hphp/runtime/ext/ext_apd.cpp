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

#include "hphp/runtime/ext/ext_apd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_override_function(const String& name, const String& args,
                         const String& code) {
  throw NotSupportedException(__func__, "dynamic coding is not supported");
}

bool f_rename_function(const String& orig_name, const String& new_name) {
  throw NotSupportedException(__func__, "dynamic coding is not supported");
}

void f_apd_set_browser_trace() {
  throw NotSupportedException(__func__, "apd is not supported");
}

String f_apd_set_pprof_trace(const String& dumpdir /* = null_string */,
                             const String& frament /* = null_string */) {
  throw NotSupportedException(__func__, "apd is not supported");
}

bool f_apd_set_session_trace_socket(const String& ip_or_filename, int domain,
                                    int port, int mask) {
  throw NotSupportedException(__func__, "apd is not supported");
}

void f_apd_stop_trace() {
  throw NotSupportedException(__func__, "apd is not supported");
}

bool f_apd_breakpoint() {
  throw NotSupportedException(__func__, "apd is not supported");
}

bool f_apd_continue() {
  throw NotSupportedException(__func__, "apd is not supported");
}

bool f_apd_echo(const String& output) {
  throw NotSupportedException(__func__, "apd is not supported");
}

///////////////////////////////////////////////////////////////////////////////
}
