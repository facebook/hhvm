/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <proxygen/lib/http/codec/HTTPCodecFactory.h>

namespace proxygen {

class DefaultHTTPCodecFactory : public HTTPCodecFactory {
 public:
  explicit DefaultHTTPCodecFactory(bool forceHTTP1xCodecTo1_1);
  ~DefaultHTTPCodecFactory() override = default;

  /**
   * Get a codec instance
   */
  std::unique_ptr<HTTPCodec> getCodec(const std::string& nextProtocol,
                                      TransportDirection direction,
                                      bool isTLS) override;

  void setForceHTTP1xCodecTo1_1(bool forceHTTP1xCodecTo1_1) {
    forceHTTP1xCodecTo1_1_ = forceHTTP1xCodecTo1_1;
  }

 protected:
  bool forceHTTP1xCodecTo1_1_{false};
};

} // namespace proxygen
