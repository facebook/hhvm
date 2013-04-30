/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_asio.h>
#include <runtime/ext/ext_asio.h>

IMPLEMENT_SEP_EXTENSION_TEST(Asio);
///////////////////////////////////////////////////////////////////////////////

bool TestExtAsio::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_asio_set_on_failed_callback);
  RUN_TEST(test_WaitHandle);
  RUN_TEST(test_StaticWaitHandle);
  RUN_TEST(test_StaticResultWaitHandle);
  RUN_TEST(test_StaticExceptionWaitHandle);
  RUN_TEST(test_WaitableWaitHandle);
  RUN_TEST(test_BlockableWaitHandle);
  RUN_TEST(test_ContinuationWaitHandle);
  RUN_TEST(test_GenArrayWaitHandle);
  RUN_TEST(test_SetResultToRefWaitHandle);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtAsio::test_asio_set_on_failed_callback() {
  return Count(true);
}

bool TestExtAsio::test_WaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_StaticWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_StaticResultWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_StaticExceptionWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_WaitableWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_BlockableWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_ContinuationWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_GenArrayWaitHandle() {
  return Count(true);
}

bool TestExtAsio::test_SetResultToRefWaitHandle() {
  return Count(true);
}
