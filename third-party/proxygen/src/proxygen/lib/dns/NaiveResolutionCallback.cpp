/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/NaiveResolutionCallback.h"

#include <algorithm>

namespace proxygen {

using DNSAnswer = DNSResolver::Answer;
using DNSAnswers = std::vector<DNSAnswer>;

namespace {
static bool isDNSAnswer(const DNSAnswer& a) {
  return (a.type == DNSAnswer::AT_ADDRESS || a.type == DNSAnswer::AT_NAME);
}
} // namespace

// public
void NaiveResolutionCallback::resolutionSuccess(DNSAnswers answers) noexcept {
  folly::exception_wrapper ex;

  bool have_answer = std::ranges::any_of(answers, isDNSAnswer);
  if (!have_answer) {
    auto err = DNSResolver::makeNoNameException();
    ex = std::move(err);
  }

  handler_(std::move(answers), std::move(ex));
  delete this;
}

// public
void NaiveResolutionCallback::resolutionError(
    const folly::exception_wrapper& exp) noexcept {
  handler_({}, std::move(exp));
  delete this;
}

} // namespace proxygen
