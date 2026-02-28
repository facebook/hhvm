/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

class MockDNSClient : public DNSResolver::ResolutionCallback {
 public:
  void resolutionSuccess(
      std::vector<DNSResolver::Answer> answers) noexcept override {
    _resolutionSuccess(answers);
  }
  MOCK_METHOD(void, _resolutionSuccess, (std::vector<DNSResolver::Answer>));

  void resolutionError(const folly::exception_wrapper& ex) noexcept override {
    _resolutionError(ex);
  }
  MOCK_METHOD(void, _resolutionError, (const folly::exception_wrapper&));
};

} // namespace proxygen
