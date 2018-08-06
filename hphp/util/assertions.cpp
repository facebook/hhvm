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

#include "hphp/util/assertions.h"

#include <folly/Format.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static AssertFailLogger s_logger;
static bool s_assert_failed{false};

__thread AssertDetailImpl* AssertDetailImpl::s_head = nullptr;

///////////////////////////////////////////////////////////////////////////////

bool AssertDetailImpl::log_impl(const AssertDetailImpl* adi) {
  if (!adi) return false;
  log_impl(adi->m_next);

  auto const title = folly::sformat("{:-^80}\n", adi->m_name);
  auto const msg = adi->run();

  fprintf(stderr, "\n%s%s\n", title.c_str(), msg.c_str());
  if (s_logger) s_logger(title.c_str(), msg);

  return true;
}

bool AssertDetailImpl::log() { return log_impl(s_head); }

//////////////////////////////////////////////////////////////////////

static void assert_log_failure(const char* e, const std::string& msg) {
  fprintf(stderr, "\nAssertion failure: %s\n\n%s\n", e, msg.c_str());

  if (s_logger) {
    s_logger("Assertion Failure", e);
    if (!msg.empty()) {
      s_logger("Assertion Message", msg);
    }
  }
  auto const detailed = AssertDetailImpl::log();
  fprintf(stderr, "\n");

  // Reprint the original message, so readers don't necessarily have to page up
  // through all the detail to find it.  We also printed it first, just in case
  // one of the detailers wanted to segfault.
  if (detailed) {
    fprintf(stderr, "\nAssertion failure: %s\n\n%s\n", e, msg.c_str());
  }
}

void assert_fail(const char* e, const char* file,
                 unsigned int line, const char* func,
                 const std::string& msg) {
  // If we re-enter this function it's because we hit a second assertion while
  // processing the first. The second assertion is meaningless, so just
  // short-circuit straight to abort().
  if (s_assert_failed) std::abort();
  s_assert_failed = true;

  auto const assertion = folly::sformat("{}:{}: {}: assertion `{}' failed.",
                                        file, line, func, e);
  assert_log_failure(assertion.c_str(), msg);

  std::abort();
}

void register_assert_fail_logger(AssertFailLogger l) {
  s_logger = l;
}

///////////////////////////////////////////////////////////////////////////////
}
