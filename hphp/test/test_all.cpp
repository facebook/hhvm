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

#include <test/test.h>
#include <test/test_base_suite.inc>
#include <compiler/option.h>

///////////////////////////////////////////////////////////////////////////////

void Test::RunTestsImpl(bool &allPassed, std::string &suite,
                        std::string &which, std::string &set) {
  // individual test suites
  s_suite = suite;
  if (suite == "TestDebugger" || suite == "TestDebuggerJit") {
    if (suite == "TestDebuggerJit") {
      RuntimeOption::EvalJit = true;
    } else {
      RuntimeOption::EvalJit = false;
    }
    suite = "TestDebugger";
    RUN_TESTSUITE(TestDebugger);
    return;
  }
  if (suite == "TestServer") {
    Option::EnableEval = Option::FullEval;
    RUN_TESTSUITE(TestServer);
    return;
  }

  // set based tests with many suites
  if (set == "TestUnit") {
    RUN_TESTSUITE(TestParserExpr);
    RUN_TESTSUITE(TestParserStmt);
    RUN_TESTSUITE(TestCodeError);
    RUN_TESTSUITE(TestUtil);
    RUN_TESTSUITE(TestCppBase);
    return;
  }
  if (set == "TestExt") {
  // complete extension tests
#include "test_ext.inc"
    return;
  }

  printf("Unknown suite: %s\n", suite.c_str());
}
