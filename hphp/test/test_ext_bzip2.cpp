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

#include <test/test_ext_bzip2.h>
#include <runtime/ext/ext_bzip2.h>

IMPLEMENT_SEP_EXTENSION_TEST(Bzip2);
///////////////////////////////////////////////////////////////////////////////

bool TestExtBzip2::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_bzclose);
  RUN_TEST(test_bzopen);
  RUN_TEST(test_bzread);
  RUN_TEST(test_bzwrite);
  RUN_TEST(test_bzflush);
  RUN_TEST(test_bzerrstr);
  RUN_TEST(test_bzerror);
  RUN_TEST(test_bzerrno);
  RUN_TEST(test_bzcompress);
  RUN_TEST(test_bzdecompress);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtBzip2::test_bzclose() {
  return Count(true);
}

bool TestExtBzip2::test_bzopen() {
  return Count(true);
}

bool TestExtBzip2::test_bzread() {
  return Count(true);
}

bool TestExtBzip2::test_bzwrite() {
  String str = "HipHop for";
  Variant bz = f_bzopen("test/test_ext_bzip2.tmp", "w");
  VERIFY(bz);
  VS(f_bzwrite(bz, str), 10);
  f_bzflush(bz);
  VERIFY(f_bzclose(bz));

  bz = f_bzopen("test/test_ext_bzip2.tmp", "r");
  Variant ret = f_bzread(bz, 10000);
  VS(ret, str);
  VERIFY(f_bzclose(bz));
  VS(ret, str);
  f_unlink("test/test_ext_bzip2.tmp");
  return Count(true);
}

bool TestExtBzip2::test_bzflush() {
  return Count(true);
}

bool TestExtBzip2::test_bzerrstr() {
  Variant f = f_fopen("test/test_ext_bzip2.tmp", "w");
  f_fwrite(f, "this is a test");
  f_fclose(f);
  f = f_bzopen("test/test_ext_bzip2.tmp", "r");
  f_bzread(f);
  String ret = f_bzerrstr(f);
  f_bzclose(f);
  f_unlink("test/test_ext_bzip2.tmp");
  VS(ret, "DATA_ERROR_MAGIC");
  return Count(true);
}

bool TestExtBzip2::test_bzerror() {
  Variant f = f_fopen("test/test_ext_bzip2.tmp", "w");
  f_fwrite(f, "this is a test");
  f_fclose(f);
  f = f_bzopen("test/test_ext_bzip2.tmp", "r");
  f_bzread(f);
  Variant ret = f_bzerror(f);
  f_bzclose(f);
  f_unlink("test/test_ext_bzip2.tmp");
  VS(ret, CREATE_MAP2("errno", -5, "errstr", "DATA_ERROR_MAGIC"));
  return Count(true);
}

bool TestExtBzip2::test_bzerrno() {
  Variant f = f_fopen("test/test_ext_bzip2.tmp", "w");
  f_fwrite(f, "this is a test");
  f_fclose(f);
  f = f_bzopen("test/test_ext_bzip2.tmp", "r");
  f_bzread(f);
  int ret = f_bzerrno(f);
  f_bzclose(f);
  f_unlink("test/test_ext_bzip2.tmp");
  VS(ret, -5);
  return Count(true);
}

bool TestExtBzip2::test_bzcompress() {
  return Count(true);
}

bool TestExtBzip2::test_bzdecompress() {
  String str = "HipHop for PHP transforms PHP source code into highly optimized C++.";
  String ret;
  ret = f_bzcompress(str);
  ret = f_bzcompress(ret);
  ret = f_bzcompress(ret);
  ret = f_bzdecompress(ret);
  ret = f_bzdecompress(ret);
  ret = f_bzdecompress(ret);
  VS(ret, str);
  return Count(true);
}
