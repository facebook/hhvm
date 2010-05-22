/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_apd.h>
#include <runtime/ext/ext_apd.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtApd::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_override_function);
  RUN_TEST(test_rename_function);
  RUN_TEST(test_apd_set_browser_trace);
  RUN_TEST(test_apd_set_pprof_trace);
  RUN_TEST(test_apd_set_session_trace_socket);
  RUN_TEST(test_apd_stop_trace);
  RUN_TEST(test_apd_breakpoint);
  RUN_TEST(test_apd_continue);
  RUN_TEST(test_apd_echo);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtApd::test_override_function() {
  try {
    f_override_function("", "", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_rename_function() {
  try {
    f_rename_function("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_set_browser_trace() {
  try {
    f_apd_set_browser_trace();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_set_pprof_trace() {
  try {
    f_apd_set_pprof_trace("", "");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_set_session_trace_socket() {
  try {
    f_apd_set_session_trace_socket("", 0, 0, 0);
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_stop_trace() {
  try {
    f_apd_stop_trace();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_breakpoint() {
  try {
    f_apd_breakpoint();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_continue() {
  try {
    f_apd_continue();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtApd::test_apd_echo() {
  try {
    f_apd_echo("");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}
