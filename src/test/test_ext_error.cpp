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

#include <test/test_ext_error.h>
#include <runtime/ext/ext_error.h>
#include <runtime/base/runtime_option.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtError::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_debug_backtrace);
  RUN_TEST(test_debug_print_backtrace);
  RUN_TEST(test_error_get_last);
  RUN_TEST(test_error_log);
  RUN_TEST(test_error_reporting);
  RUN_TEST(test_restore_error_handler);
  RUN_TEST(test_restore_exception_handler);
  RUN_TEST(test_set_error_handler);
  RUN_TEST(test_set_exception_handler);
  RUN_TEST(test_trigger_error);
  RUN_TEST(test_user_error);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtError::test_debug_backtrace() {
  bool oldValue = RuntimeOption::InjectedStackTrace;
  RuntimeOption::InjectedStackTrace = false;
  Array ret = f_debug_backtrace();
#ifndef MAC_OS_X
  VERIFY(ret.size() > 0);
#endif
  RuntimeOption::InjectedStackTrace = oldValue;
  return Count(true);
}

bool TestExtError::test_debug_print_backtrace() {
  bool oldValue = RuntimeOption::InjectedStackTrace;
  RuntimeOption::InjectedStackTrace = false;
  StackTrace::Enabled = true;
  g_context->obStart();
  f_debug_print_backtrace();
  String output = g_context->obGetContents();
  g_context->obEnd();
#ifndef MAC_OS_X
  VERIFY(strstr((const char *)output, "test_debug_print_backtrace"));
#endif
  RuntimeOption::InjectedStackTrace = oldValue;
  return Count(true);
}

bool TestExtError::test_error_get_last() {
  f_error_get_last(); // dry run
  return Count(true);
}

bool TestExtError::test_error_log() {
  f_error_log(""); // dry run
  return Count(true);
}

bool TestExtError::test_error_reporting() {
  f_error_reporting(1); // dry run
  return Count(true);
}

bool TestExtError::test_restore_error_handler() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}

bool TestExtError::test_restore_exception_handler() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}

bool TestExtError::test_set_error_handler() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}

bool TestExtError::test_set_exception_handler() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}

bool TestExtError::test_trigger_error() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}

bool TestExtError::test_user_error() {
  // tested in TestCodeRun::TestErrorHandler
  return Count(true);
}
