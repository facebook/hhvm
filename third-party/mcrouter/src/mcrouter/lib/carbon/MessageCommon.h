/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdlib>
#include <string>

namespace carbon {

class MessageCommon {
 public:
  /**
   * Gets the current trace context string.
   *
   * @return  The trace context string, or empty string if this message is
   *          not being traced.
   */
  const std::string& traceContext() const {
    return traceContext_;
  }

  /**
   * Sets the trace context for this message.
   * If trace context is present, this message will be traced.
   *
   * @param @traceContext   The trace context string.
   */
  void setTraceContext(std::string traceContext) {
    traceContext_ = std::move(traceContext);
  }

  static constexpr std::string_view kCryptoAuthTokenHeader =
      "crypto_auth_tokens";

 protected:
  std::string traceContext_;
};

} // namespace carbon
