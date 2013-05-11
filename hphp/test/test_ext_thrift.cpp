/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <test/test_ext_thrift.h>
#include <runtime/ext/ext_thrift.h>

IMPLEMENT_SEP_EXTENSION_TEST(Thrift);
///////////////////////////////////////////////////////////////////////////////

bool TestExtThrift::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_thrift_protocol_write_binary);
  RUN_TEST(test_thrift_protocol_read_binary);
  RUN_TEST(test_thrift_protocol_write_compact);
  RUN_TEST(test_thrift_protocol_read_compact);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtThrift::test_thrift_protocol_write_binary() {
  return Count(true);
}

bool TestExtThrift::test_thrift_protocol_read_binary() {
  return Count(true);
}

bool TestExtThrift::test_thrift_protocol_write_compact() {
  return Count(true);
}

bool TestExtThrift::test_thrift_protocol_read_compact() {
  return Count(true);
}
