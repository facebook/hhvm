/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.h>

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace proxygen {

HTTPDefaultSessionCodecFactory::HTTPDefaultSessionCodecFactory(
    const AcceptorConfiguration& accConfig)
    : accConfig_(accConfig) {
  // set up codec defaults in the case of plaintext connections
  if (accConfig.plaintextProtocol == http2::kProtocolCleartextString) {
    alwaysUseHTTP2_ = true;
  }
}

std::unique_ptr<HTTPCodec> HTTPDefaultSessionCodecFactory::getCodec(
    const std::string& nextProtocol, TransportDirection direction, bool isTLS) {
  if (!isTLS && alwaysUseHTTP2_) {
    auto codec = std::make_unique<HTTP2Codec>(direction);
    codec->setStrictValidation(useStrictValidation());
    if (accConfig_.headerIndexingStrategy) {
      codec->setHeaderIndexingStrategy(accConfig_.headerIndexingStrategy);
    }
    return codec;
  } else if (nextProtocol.empty() ||
             HTTP1xCodec::supportsNextProtocol(nextProtocol)) {
    auto codec = std::make_unique<HTTP1xCodec>(
        direction, accConfig_.forceHTTP1_0_to_1_1, useStrictValidation());
    if (!isTLS) {
      codec->setAllowedUpgradeProtocols(
          accConfig_.allowedPlaintextUpgradeProtocols);
    }
    return codec;
  } else if (nextProtocol == http2::kProtocolString ||
             nextProtocol == http2::kProtocolDraftString ||
             nextProtocol == http2::kProtocolExperimentalString) {
    auto codec = std::make_unique<HTTP2Codec>(direction);
    codec->setStrictValidation(useStrictValidation());
    if (accConfig_.headerIndexingStrategy) {
      codec->setHeaderIndexingStrategy(accConfig_.headerIndexingStrategy);
    }
    return codec;
  } else {
    VLOG(2) << "Client requested unrecognized next protocol " << nextProtocol;
  }

  return nullptr;
}
} // namespace proxygen
