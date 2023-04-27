/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/services/AcceptorConfiguration.h>

namespace proxygen {

class HTTPDefaultSessionCodecFactory : public HTTPCodecFactory {
 public:
  explicit HTTPDefaultSessionCodecFactory(
      const AcceptorConfiguration& accConfig);
  ~HTTPDefaultSessionCodecFactory() override {
  }

  /**
   * Get a codec instance
   */
  std::unique_ptr<HTTPCodec> getCodec(const std::string& nextProtocol,
                                      TransportDirection direction,
                                      bool isTLS) override;

 protected:
  const AcceptorConfiguration& accConfig_;
  folly::Optional<bool> alwaysUseHTTP2_{};
};

} // namespace proxygen
