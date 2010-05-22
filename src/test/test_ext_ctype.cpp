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

#include <test/test_ext_ctype.h>
#include <runtime/ext/ext_ctype.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtCtype::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_ctype_alnum);
  RUN_TEST(test_ctype_alpha);
  RUN_TEST(test_ctype_cntrl);
  RUN_TEST(test_ctype_digit);
  RUN_TEST(test_ctype_graph);
  RUN_TEST(test_ctype_lower);
  RUN_TEST(test_ctype_print);
  RUN_TEST(test_ctype_punct);
  RUN_TEST(test_ctype_space);
  RUN_TEST(test_ctype_upper);
  RUN_TEST(test_ctype_xdigit);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtCtype::test_ctype_alnum() {
  VERIFY(f_ctype_alnum("abc123"));
  VERIFY(!f_ctype_alnum("!@#$^"));
  return Count(true);
}

bool TestExtCtype::test_ctype_alpha() {
  VERIFY(f_ctype_alpha("abcdef"));
  VERIFY(!f_ctype_alpha("abc123"));
  return Count(true);
}

bool TestExtCtype::test_ctype_cntrl() {
  VERIFY(f_ctype_cntrl("\t\n\r"));
  VERIFY(!f_ctype_cntrl("abc123"));
  return Count(true);
}

bool TestExtCtype::test_ctype_digit() {
  VERIFY(f_ctype_digit("123456"));
  VERIFY(!f_ctype_digit("abc123"));
  return Count(true);
}

bool TestExtCtype::test_ctype_graph() {
  VERIFY(f_ctype_graph("!@#$^"));
  VERIFY(!f_ctype_graph("\b"));
  return Count(true);
}

bool TestExtCtype::test_ctype_lower() {
  VERIFY(f_ctype_lower("abcdef"));
  VERIFY(!f_ctype_lower("ABCDEF"));
  return Count(true);
}

bool TestExtCtype::test_ctype_print() {
  VERIFY(f_ctype_print("!@#$^"));
  VERIFY(!f_ctype_print("\b"));
  return Count(true);
}

bool TestExtCtype::test_ctype_punct() {
  VERIFY(f_ctype_punct("!@#$^"));
  VERIFY(!f_ctype_punct("ABCDEF"));
  return Count(true);
}

bool TestExtCtype::test_ctype_space() {
  VERIFY(f_ctype_space(" "));
  VERIFY(!f_ctype_space("a "));
  return Count(true);
}

bool TestExtCtype::test_ctype_upper() {
  VERIFY(f_ctype_upper("ABCDEF"));
  VERIFY(!f_ctype_upper("abcdef"));
  return Count(true);
}

bool TestExtCtype::test_ctype_xdigit() {
  VERIFY(f_ctype_xdigit("ABCDEF"));
  VERIFY(!f_ctype_xdigit("GHIJKL"));
  return Count(true);
}
