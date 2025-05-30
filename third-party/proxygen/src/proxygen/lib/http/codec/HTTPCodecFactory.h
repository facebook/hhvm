/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <list>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/TransportDirection.h>

namespace proxygen {

/**
 * Factory for produces HTTPCodec objects.
 */
class HTTPCodecFactory {
 public:
  struct HTTP1xCodecConfig {
    bool forceHTTP1xCodecTo1_1{false};
  };

  struct HTTP2CodecConfig {
    const HeaderIndexingStrategy* headerIndexingStrategy{nullptr};
  };

  struct CodecConfig {
    // The debug setting controls header logging verbosity.
    // 0 - do not log any header names or values to stderr
    // 1 - log header names to stderr on certain errors
    // 2 - log header names and values to stderr on certain errors
    uint8_t debugLevel{0};
    // Default to false for now to match existing behavior
    bool strictValidation{false};
    HTTP1xCodecConfig h1;
    HTTP2CodecConfig h2;
  };

  HTTPCodecFactory() = default;
  explicit HTTPCodecFactory(CodecConfig config) : defaultConfig_(config) {
  }
  virtual ~HTTPCodecFactory() {
  }

  /**
   * Get a codec instance
   */
  virtual std::unique_ptr<HTTPCodec> getCodec(const std::string& protocolHint,
                                              TransportDirection direction,
                                              bool isTLS) = 0;

  static std::unique_ptr<HTTPCodec> getCodec(CodecProtocol protocol,
                                             TransportDirection direction,
                                             bool strictValidation = false);

  CodecConfig& getDefaultConfig() {
    return defaultConfig_;
  }

  void setConfigFn(std::function<CodecConfig()> configFn) {
    configFn_ = configFn;
  }

  bool useStrictValidation() {
    return configFn_().strictValidation;
  }

 protected:
  CodecConfig defaultConfig_;
  std::function<CodecConfig()> configFn_{[this] { return defaultConfig_; }};
};

} // namespace proxygen
