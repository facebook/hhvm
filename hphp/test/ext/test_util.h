/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_TEST_UTIL_H_
#define incl_HPHP_TEST_UTIL_H_

#include <string>

#include "hphp/test/ext/test_base.h"

///////////////////////////////////////////////////////////////////////////////

struct TestUtil : TestBase {
  TestUtil();

  bool RunTests(const std::string& which) override;

  bool TestLFUTable();
  bool TestSharedString();
  bool TestCanonicalize();
  bool TestHDF();
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_CPP_BASE_H_
