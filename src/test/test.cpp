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

#include <test/test.h>
#include <test/test_suite.inc>
#include <cpp/base/shared/shared_store.h>
#include <lib/option.h>
//#include <cpp/base/util/light_process.h>

///////////////////////////////////////////////////////////////////////////////

int Test::s_total = 0;
int Test::s_passed = 0;
bool Test::s_quiet = false;

///////////////////////////////////////////////////////////////////////////////

#define RUN_TESTSUITE(name)                                             \
  if (suite.empty() || suite == #name) {                                \
    if (!s_quiet) {                                                     \
      printf(#name "......\n\n");                                       \
    }                                                                   \
    name test;                                                          \
    if (test.RunTests(which)) {                                         \
      if (!s_quiet) {                                                   \
        printf("\n" #name " OK\n\n");                                   \
      }                                                                 \
    } else {                                                            \
      printf("\n" #name " #####>>> FAILED <<< #####\n\n");              \
      allPassed = false;                                                \
    }                                                                   \
  }                                                                     \

///////////////////////////////////////////////////////////////////////////////

void Test::RunTests(std::string &suite, std::string &which, std::string &set) {
  bool allPassed = true;
  SharedMemoryManager::Init(16 * 1024 * 1024, true);

  size_t pos = suite.find("::");
  if (pos != std::string::npos) {
    which = suite.substr(pos + 2);
    suite = suite.substr(0, pos);
  }

  // individual test suites
  if (suite == "TestCodeRun") {
    RUN_TESTSUITE(TestCodeRun);
    goto done;
  }
  if (suite == "TestCodeRunEval") {
    suite = "TestCodeRun";
    Option::EnableEval = Option::FullEval;
    RUN_TESTSUITE(TestCodeRun);
    goto done;
  }
  if (suite == "TestServer") {
    RUN_TESTSUITE(TestServer);
    goto done;
  }
  if (suite == "TestServerEval") {
    suite = "TestServer";
    Option::EnableEval = Option::FullEval;
    RUN_TESTSUITE(TestServer);
    goto done;
  }
  if (suite == "TestPerformance") {
    RUN_TESTSUITE(TestPerformance);
    goto done;
  }

  // fast unit tests
  if (set != "TestExt") {
#include <test/test_fast.inc>
  }
  if (set == "QuickTests") {
    goto done;
  }

  // complete extension tests
#include "test_ext.inc"

  if (suite == "" && set != "NoCodeRun" && set != "TestExt") {
    RUN_TESTSUITE(TestCodeRun);
  }

 done:
  if (allPassed) {
    ASSERT(s_total == s_passed);
    printf("%d/%d unit tests passed.\n", s_passed, s_total);
  } else {
    printf("ERROR: %d/%d unit tests failed.\n", s_total - s_passed, s_total);
  }
}
