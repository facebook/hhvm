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

#include <test/test_ext_misc.h>
#include <runtime/ext/ext_misc.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtMisc::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_connection_aborted);
  RUN_TEST(test_connection_status);
  RUN_TEST(test_connection_timeout);
  RUN_TEST(test_constant);
  RUN_TEST(test_define);
  RUN_TEST(test_defined);
  RUN_TEST(test_die);
  RUN_TEST(test_exit);
  RUN_TEST(test_eval);
  RUN_TEST(test_get_browser);
  RUN_TEST(test___halt_compiler);
  RUN_TEST(test_highlight_file);
  RUN_TEST(test_show_source);
  RUN_TEST(test_highlight_string);
  RUN_TEST(test_ignore_user_abort);
  RUN_TEST(test_pack);
  RUN_TEST(test_php_check_syntax);
  RUN_TEST(test_php_strip_whitespace);
  RUN_TEST(test_sleep);
  RUN_TEST(test_usleep);
  RUN_TEST(test_time_nanosleep);
  RUN_TEST(test_time_sleep_until);
  RUN_TEST(test_uniqid);
  RUN_TEST(test_unpack);
  RUN_TEST(test_sys_getloadavg);
  RUN_TEST(test_token_get_all);
  RUN_TEST(test_token_name);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtMisc::test_connection_aborted() {
  VERIFY(!f_connection_aborted());
  return Count(true);
}

bool TestExtMisc::test_connection_status() {
  VERIFY(f_connection_status() == k_CONNECTION_NORMAL);
  return Count(true);
}

bool TestExtMisc::test_connection_timeout() {
  VERIFY(!f_connection_timeout());
  return Count(true);
}

bool TestExtMisc::test_constant() {
  f_constant("a");
  return Count(true);
}

bool TestExtMisc::test_define() {
  // a function that's never called
  return Count(true);
}

bool TestExtMisc::test_defined() {
  VERIFY(!f_defined("a"));
  return Count(true);
}

bool TestExtMisc::test_die() {
  // can't really test this
  return Count(true);
}

bool TestExtMisc::test_exit() {
  // can't really test this
  return Count(true);
}

bool TestExtMisc::test_eval() {
  try {
    f_eval("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_get_browser() {
  try {
    f_get_browser();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test___halt_compiler() {
  f___halt_compiler();
  return Count(true);
}

bool TestExtMisc::test_highlight_file() {
  try {
    f_highlight_file("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_show_source() {
  try {
    f_show_source("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_highlight_string() {
  try {
    f_highlight_string("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_ignore_user_abort() {
  f_ignore_user_abort("a");
  return Count(true);
}

bool TestExtMisc::test_pack() {
  // covered in TestCodeRun::TestExtMisc
  return Count(true);
}

bool TestExtMisc::test_php_check_syntax() {
  try {
    f_php_check_syntax("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_php_strip_whitespace() {
  try {
    f_php_strip_whitespace("a");
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtMisc::test_sleep() {
  f_sleep(1);
  return Count(true);
}

bool TestExtMisc::test_usleep() {
  f_usleep(1);
  return Count(true);
}

bool TestExtMisc::test_time_nanosleep() {
  f_time_nanosleep(0, 100);
  return Count(true);
}

bool TestExtMisc::test_time_sleep_until() {
  struct timeval tm;
  gettimeofday((struct timeval *)&tm, NULL);
  double timestamp = tm.tv_sec + tm.tv_usec / 1000000.0 + 2;
  f_time_sleep_until(timestamp);
  return Count(true);
}

bool TestExtMisc::test_uniqid() {
  VERIFY(!f_uniqid().empty());
  return Count(true);
}

bool TestExtMisc::test_unpack() {
  // covered in TestCodeRun::TestExtMisc
  return Count(true);
}

bool TestExtMisc::test_sys_getloadavg() {
  VERIFY(f_sys_getloadavg().size() == 3);
  return Count(true);
}

bool TestExtMisc::test_token_get_all() {
  String src = "blarb <?php 1";
  VS(f_token_get_all(src), CREATE_VECTOR3(CREATE_VECTOR3(311, "blarb ", 1),
                                          CREATE_VECTOR3(367, "<?php ", 1),
                                          CREATE_VECTOR3(305, "1", 1)));
  return Count(true);
}
bool TestExtMisc::test_token_name() {
    VS(f_token_name(258), "T_REQUIRE_ONCE");
  return Count(true);
}
