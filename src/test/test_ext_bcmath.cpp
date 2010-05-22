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

#include <test/test_ext_bcmath.h>
#include <runtime/ext/ext_bcmath.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtBcmath::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_bcscale);
  RUN_TEST(test_bcadd);
  RUN_TEST(test_bcsub);
  RUN_TEST(test_bccomp);
  RUN_TEST(test_bcmul);
  RUN_TEST(test_bcdiv);
  RUN_TEST(test_bcmod);
  RUN_TEST(test_bcpow);
  RUN_TEST(test_bcpowmod);
  RUN_TEST(test_bcsqrt);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtBcmath::test_bcscale() {
  VS(f_bcdiv("105", "6.55957", 3), "16.007");
  f_bcscale(3);
  VS(f_bcdiv("105", "6.55957"), "16.007");
  f_bcscale(0);
  return Count(true);
}

bool TestExtBcmath::test_bcadd() {
  VS(f_bcadd("1.234", "5"), "6");
  VS(f_bcadd("1.234", "5", 4), "6.2340");
  return Count(true);
}

bool TestExtBcmath::test_bcsub() {
  VS(f_bcsub("1.234", "5"), "-3");
  VS(f_bcsub("1.234", "5", 4), "-3.7660");
  return Count(true);
}

bool TestExtBcmath::test_bccomp() {
  VS(f_bccomp("1", "2"), -1);
  VS(f_bccomp("1.00001", "1", 3), 0);
  VS(f_bccomp("1.00001", "1", 5), 1);
  return Count(true);
}

bool TestExtBcmath::test_bcmul() {
  VS(f_bcmul("1.34747474747", "35", 3), "47.161");
  VS(f_bcmul("2", "4"), "8");
  return Count(true);
}

bool TestExtBcmath::test_bcdiv() {
  VS(f_bcdiv("105", "6.55957", 3), "16.007");
  return Count(true);
}

bool TestExtBcmath::test_bcmod() {
  VS(f_bcmod("4", "2"), "0");
  VS(f_bcmod("2", "4"), "2");
  return Count(true);
}

bool TestExtBcmath::test_bcpow() {
  VS(f_bcpow("4.2", "3", 2), "74.08");
  return Count(true);
}

bool TestExtBcmath::test_bcpowmod() {
  VS(f_bcpowmod("4", "3", "5"), "4");
  return Count(true);
}

bool TestExtBcmath::test_bcsqrt() {
  VS(f_bcsqrt("2", 3), "1.414");
  return Count(true);
}
