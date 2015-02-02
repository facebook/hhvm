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

#include <folly/Format.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static AssertFailLogger s_logger;

__thread AssertDetailImpl* AssertDetailImpl::s_head = nullptr;

///////////////////////////////////////////////////////////////////////////////

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

void assert_fail_log_detail(AssertDetailImpl* adi) {
  if (!adi) return;
  assert_fail_log_detail(adi->m_next);

  auto out = adi->run();

  if (s_logger) {
    s_logger(folly::format(
      "{:-^80}\n", adi->m_name
    ).str().c_str(), out);
  }

  fprintf(stderr, "%s", folly::format(
    "\n{:-^80}\n{}\n", adi->m_name, out
  ).str().c_str());
}

void assert_fail_log(const char* title, const std::string& msg) {
  if (s_logger) s_logger(title, msg);
  fprintf(stderr, "\nAssertion failure: %s\n%s\n", title, msg.c_str());

  assert_fail_log_detail(AssertDetailImpl::s_head);
  fprintf(stderr, "\n");
}

void register_assert_fail_logger(AssertFailLogger l) {
  s_logger = l;
}

///////////////////////////////////////////////////////////////////////////////
}
