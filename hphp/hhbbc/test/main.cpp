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
#include <gtest/gtest.h>
#include "hphp/hhvm/process-init.h"
#include "hphp/util/current-executable.h"

#include <string>

int main(int argc, char** argv) {
  std::string buf = HPHP::current_executable_path();
  if (!buf.empty()) {
    size_t idx = buf.length();
    for (int i = 0; i < 3; i++) {
      idx = buf.find_last_of('/', idx - 1);
      assert(idx != std::string::npos);
    }
    std::string slib = buf.substr(0, idx);
    slib += "/runtime/ext_hhvm/systemlib.php";
    setenv("HHVM_SYSTEMLIB", slib.c_str(), true);
  }
  testing::InitGoogleTest(&argc, argv);
  HPHP::init_for_unit_test();
  return RUN_ALL_TESTS();
}
