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
  if (suite == "TestCodeRun") {
    RUN_TESTSUITE(TestCodeRun);
    return;
  }
  if (suite == "TestCodeRunStatic") {
    suite = "TestCodeRun";
    TestCodeRun::FastMode = false;
    RUN_TESTSUITE(TestCodeRun);
    return;
  }
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
  if (hhvm) {
    const char *vmFilter = 0;

    if (suite == "TestCodeRunVM" || suite == "TestCodeRunRepo") {
      suite = "TestCodeRun";
      Option::EnableEval = Option::FullEval;
      RuntimeOption::EvalJit = false;
      TestCodeRun::Filter = vmFilter;
      RUN_TESTSUITE(TestCodeRun);
      return;
    }
    if (suite == "TestCodeRunJit" || suite == "TestCodeRunRepoJit" ||
        suite == "TestCodeRunJitIR" || suite == "TestCodeRunRepoJitIR") {
      suite = "TestCodeRun";
      Option::EnableEval = Option::FullEval;
      RuntimeOption::EvalJit = true;
      if (suite == "TestCodeRunJitIR" || suite == "TestCodeRunRepoJitIR") {
        RuntimeOption::EvalJitUseIR = true;
      }
      TestCodeRun::Filter = vmFilter;
      RUN_TESTSUITE(TestCodeRun);
      return;
    }
  }
  if (hhvm) {
    fprintf(stderr, "%s is not supported with USE_HHVM=1\n", suite.c_str());
    exit(-1);
  }
  if (suite == "TestServer") {
    RUN_TESTSUITE(TestServer);
    return;
  }
  if (suite == "TestPerformance") {
    RUN_TESTSUITE(TestPerformance);
    return;
  }

  // fast unit tests
  if (set != "TestExt") {
#include <test/test_base_fast.inc>
  }
  if (set == "QuickTests") {
    return;
  }

  // complete extension tests
#include "test_ext.inc"

  if (suite == "" && set != "NoCodeRun" && set != "TestExt") {
    RUN_TESTSUITE(TestCodeRun);
  }
}
