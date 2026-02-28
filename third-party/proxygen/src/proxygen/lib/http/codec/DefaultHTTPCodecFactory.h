/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <list>
#include <proxygen/lib/http/codec/HTTPCodecFactory.h>

namespace proxygen {
class HeaderIndexingStrategy;

class DefaultHTTPCodecFactory : public HTTPCodecFactory {
 public:
  DefaultHTTPCodecFactory() = default;
  explicit DefaultHTTPCodecFactory(CodecConfig config);
  ~DefaultHTTPCodecFactory() override = default;

  /**
   * Get a codec instance
   */
  std::unique_ptr<HTTPCodec> getCodec(const std::string& nextProtocol,
                                      TransportDirection direction,
                                      bool isTLS) override;
};

} // namespace proxygen
