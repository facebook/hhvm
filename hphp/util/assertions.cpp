/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/assertions.h"

#include "folly/Format.h"

namespace HPHP {

static AssertFailLogger s_logger;

void assert_fail(const char* e, const char* file,
                 unsigned int line, const char* func) {
  auto const msg = folly::format("{}:{}: {}: assertion `{}' failed.",
                                 file, line, func, e).str();
  fprintf(stderr, "%s\n", msg.c_str());
  if (s_logger) {
    s_logger("Failed Assertion", msg);
  }
  std::abort();
}

void assert_fail_log(const char* title, const std::string& msg) {
  if (s_logger) {
    s_logger(title, msg);
  }
  fprintf(stderr, "\nAssertion failure: %s\n%s\n\n", title, msg.c_str());
}

void register_assert_fail_logger(AssertFailLogger l) {
  s_logger = l;
}

}
