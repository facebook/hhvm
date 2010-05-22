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
#include <runtime/base/shared/shared_store.h>
#include <compiler/option.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

int Test::s_total = 0;
int Test::s_passed = 0;
int Test::s_skipped = 0;

bool Test::s_quiet = false;

///////////////////////////////////////////////////////////////////////////////

bool Test::RunTests(std::string &suite, std::string &which, std::string &set) {
  bool allPassed = true;
  SharedMemoryManager::Init(16 * 1024 * 1024, true);
  Option::Load();

  size_t pos = suite.find("::");
  if (pos != std::string::npos) {
    which = suite.substr(pos + 2);
    suite = suite.substr(0, pos);
  }

  RunTestsImpl(allPassed, suite, which, set);

  if (s_skipped) {
    printf("%d/%d unit tests skipped.\n", s_skipped, s_total);
  }

  if (allPassed) {
    ASSERT(s_total == s_passed + s_skipped);
    printf("%d/%d unit tests passed.\n", s_passed, s_total);
    return true;
  }

  printf("ERROR: %d/%d unit tests failed.\n", s_total - s_passed - s_skipped,
         s_total);
  return false;
}
