/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/NaiveResolutionCallback.h"

#include <string>

namespace proxygen {

using folly::exception_wrapper;
using folly::make_exception_wrapper;
using DNSAnswer = DNSResolver::Answer;
using DNSException = DNSResolver::Exception;

using DNSAnswers = std::vector<DNSAnswer>;

namespace {
const std::string kNoAddr("No valid address or name found");
static bool isDNSAnswer(const DNSAnswer& a) {
  return (a.type == DNSAnswer::AT_ADDRESS || a.type == DNSAnswer::AT_NAME);
}
} // namespace

// public static
exception_wrapper NaiveResolutionCallback::makeNoNameException() noexcept {
  return make_exception_wrapper<DNSException>(DNSResolver::NODATA, kNoAddr);
}

// public
void NaiveResolutionCallback::resolutionSuccess(DNSAnswers answers) noexcept {
  folly::exception_wrapper ex;

  bool have_answer = std::any_of(answers.begin(), answers.end(), isDNSAnswer);
  if (!have_answer) {
    auto err = NaiveResolutionCallback::makeNoNameException();
    ex = std::move(err);
  }

  handler_(std::move(answers), std::move(ex));
  delete this;
}

// public
void NaiveResolutionCallback::resolutionError(
    const exception_wrapper& exp) noexcept {
  handler_({}, std::move(exp));
  delete this;
}

} // namespace proxygen
