/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <functional>
#include <vector>

#include <folly/ExceptionWrapper.h>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

// Basic callback for name resolution
class NaiveResolutionCallback : public DNSResolver::ResolutionCallback {
 public:
  using Handler = std::function<void(std::vector<DNSResolver::Answer> &&,
                                     const folly::exception_wrapper &&)>;

  explicit NaiveResolutionCallback(Handler handler) : handler_(handler) {
  }

  void resolutionSuccess(
      std::vector<DNSResolver::Answer> answers) noexcept override;
  void resolutionError(const folly::exception_wrapper &exp) noexcept override;

 private:
  Handler handler_;
};

} // namespace proxygen
