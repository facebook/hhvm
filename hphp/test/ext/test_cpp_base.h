/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TEST_CPP_BASE_H_
#define incl_HPHP_TEST_CPP_BASE_H_

#include <string>

#include "hphp/test/ext/test_base.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * Testing CPP core library coding. Main goal of this set of unit tests is to
 * make sure runtime/base library has no memory problems while doing the right
 * things. TestCppRun is good at making sure semantically correctness of these
 * classes, but it's hard to run it under valgrind. This set of unit tests,
 * in fact including all extension tests, are written in C++ code directly
 * calling library functions, therefore we can easily run it under valgrind
 * and other tools to detect any coding problems.
 */
class TestCppBase : public TestBase {
 public:
  TestCppBase();

  virtual bool RunTests(const std::string &which);

  // building blocks
  bool TestIpBlockMap();
  bool TestIpBlockMapIni();
  bool TestSatelliteServer();
  bool TestSatelliteServerIni();
  bool TestVirtualHost();
  bool TestVirtualHostIni();
  bool TestCollectionHdf();
  bool TestCollectionIni();
  bool TestVariantArrayRef();
};

///////////////////////////////////////////////////////////////////////////////

#endif // incl_HPHP_TEST_CPP_BASE_H_
