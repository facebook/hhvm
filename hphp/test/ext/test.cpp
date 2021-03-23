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

#include "hphp/test/ext/test.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/compiler/option.h"
#include <folly/Format.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

int Test::s_total = 0;
int Test::s_passed = 0;
int Test::s_skipped = 0;
std::string Test::s_suite;

bool Test::s_quiet = false;

TestLogger Test::logger;

///////////////////////////////////////////////////////////////////////////////

#if FOLLY_HAVE_WEAK_SYMBOLS
// RunTestsImpl is generally expected to be defined by a separate module that
// will be supplied at link time.  However, since that module will depend on us
// as a library we need to supply a default implementation here to avoid link
// errors.
FOLLY_ATTR_WEAK void Test::RunTestsImpl(bool& allPassed, std::string& suite,
                                        std::string& which, std::string&
                                        /*set*/) {
  printf("RunTestsImpl was not overridden when attempting to "
         "run test suite: %s\n", suite.c_str());
}
#endif

bool Test::RunTests(std::string &suite, std::string &which, std::string &set) {
  bool allPassed = true;
  Option::Load();

  size_t pos = suite.find("::");
  if (pos != std::string::npos) {
    which = suite.substr(pos + 2);
    suite = suite.substr(0, pos);
  }

  if (!logger.initializeRun()) {
    printf("WARNING: couldn't initialize test logging\n");
  }

  RunTestsImpl(allPassed, suite, which, set);

  if (!logger.finishRun()) {
    printf("WARNING: couldn't finish test logging\n");
  }

  if (s_skipped) {
    printf("%d/%d unit tests skipped.\n", s_skipped, s_total);
  }

  if (allPassed) {
    assert(s_total == s_passed + s_skipped);
    printf("%d/%d unit tests passed.\n", s_passed, s_total);
    return true;
  }

  printf("ERROR: %d/%d unit tests failed.\n", s_total - s_passed - s_skipped,
         s_total);
  return false;
}

bool Test::logTestResults(std::string name, std::string details, int pass,
                          int fail, int skip) {
  if (!logger.doLog()) {
    return true;
  }

  long seconds  = finish.tv_sec  - start.tv_sec;
  long useconds = finish.tv_usec - start.tv_usec;
  long mseconds = ((seconds) * 1000 + useconds / 1000.0) + 0.5; // round up

  auto summary = folly::sformat("PASSED ({})", pass);
  const char* status = "passed";

  if (skip > 0) {
    summary += folly::sformat(",SKIPPED ({})", skip);
  }

  if (fail > 0) {
    status = "failed";
    summary += folly::sformat("FAILED ({})", fail);
  }

  ArrayInit data(8, ArrayInit::Map{});
  data.set(String("type"),         "hphp");
  data.set(String("name"),         name);
  data.set(String("contacts"),     null_array);
  data.set(String("endedTime"),    time(nullptr));
  data.set(String("durationSecs"), mseconds / 1000.0);
  data.set(String("status"),       status);
  data.set(String("summary"),      std::string(summary));
  data.set(String("details"),      details);

  if (!logger.logTest(data.toArray())) {
    printf("WARNING: Logging %s failed\n", name.c_str());
    return false;
  }

  return true;
}
