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
#include <gtest/gtest.h>
#include "hhvm/process_init.h"

#include <string>

int main(int argc, char** argv) {
  char buf[PATH_MAX];
  if (realpath("/proc/self/exe", buf)) {
    for (int i = 0; i < 4; i++) {
      char* p = strrchr(buf, '/');
      assert(p);
      *p = 0;
    }
    std::string slib = buf;
    slib += "/ext_hhvm/systemlib.php";
    setenv("HHVM_SYSTEMLIB", slib.c_str(), true);
  }
  testing::InitGoogleTest(&argc, argv);
  HPHP::init_for_unit_test();
  return RUN_ALL_TESTS();
}
