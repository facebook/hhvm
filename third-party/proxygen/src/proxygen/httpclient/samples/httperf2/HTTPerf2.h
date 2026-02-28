/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>
#include <folly/portability/GFlags.h>
#include <string>
#include <vector>

DECLARE_string(ip);
DECLARE_string(server);
DECLARE_int32(port);

namespace proxygen {

/**
 * Interface class for varying request parameters.
 */
class VaryRequestParams {
 public:
  virtual ~VaryRequestParams() = default;

  /**
   * Get the next request URL to use.
   * Default implementation returns an empty string, which means use the default
   * URL.
   */
  virtual std::optional<std::string> getNextRequestURL() {
    return std::nullopt;
  }

  /**
   * Get the next request headers to use.
   * Default implementation returns an empty vector, which means use the default
   * headers.
   */
  virtual std::optional<std::vector<std::pair<std::string, std::string>>>
  getNextRequestHeaders() {
    return std::nullopt;
  }
};

/**
 * Run the httperf2 benchmark.
 * @param bindAddr Optional address to bind to.
 * @param varyParams Optional parameter to vary request parameters.
 *                   If nullptr, use the default parameters from flags.
 * @return Exit code.
 */
int httperf2(folly::Optional<folly::SocketAddress> bindAddr = folly::none,
             VaryRequestParams* varyParams = nullptr);

} // namespace proxygen
