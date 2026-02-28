/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/services/AcceptorConfiguration.h>

namespace proxygen {

/**
 * This factory is for an HTTP server to create codecs for new connections.
 *
 * Though this factory cannot modify the passed in accConfig, the owner can
 * change parameters at runtime which affects new codecs.
 */
class HTTPDefaultSessionCodecFactory : public HTTPCodecFactory {
 public:
  explicit HTTPDefaultSessionCodecFactory(
      std::shared_ptr<const AcceptorConfiguration> accConfig);
  ~HTTPDefaultSessionCodecFactory() override = default;

  /**
   * Get a codec instance
   */
  std::unique_ptr<HTTPCodec> getCodec(const std::string& nextProtocol,
                                      TransportDirection direction,
                                      bool isTLS) override;

 protected:
  std::shared_ptr<const AcceptorConfiguration> accConfig_;
};

} // namespace proxygen
