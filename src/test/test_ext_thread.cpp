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

#include <test/test_ext_thread.h>
#include <runtime/ext/ext_thread.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtThread::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_hphp_service_thread_started);
  RUN_TEST(test_hphp_thread_is_warmup_enabled);
  RUN_TEST(test_hphp_thread_set_warmup_enabled);
  RUN_TEST(test_hphp_get_thread_id);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtThread::test_hphp_service_thread_started() {
  // need to test with whole server integration
  return Count(true);
}

bool TestExtThread::test_hphp_thread_is_warmup_enabled() {
  return Count(true);
}

bool TestExtThread::test_hphp_thread_set_warmup_enabled() {
  return Count(true);
}

bool TestExtThread::test_hphp_get_thread_id() {
  f_hphp_get_thread_id();
  return Count(true);
}
