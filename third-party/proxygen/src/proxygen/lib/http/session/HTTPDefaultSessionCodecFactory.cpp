/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>
#include <proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.h>

#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace {
proxygen::HTTPCodecFactory::CodecConfig getCodecConfigFromAcceptorConfig(
    const proxygen::AcceptorConfiguration& accConfig) {
  proxygen::HTTPCodecFactory::CodecConfig config;
  config.h1.forceHTTP1xCodecTo1_1 = accConfig.forceHTTP1_0_to_1_1;
  config.h2.headerIndexingStrategy = accConfig.headerIndexingStrategy;
  return config;
}
} // namespace

namespace proxygen {

HTTPDefaultSessionCodecFactory::HTTPDefaultSessionCodecFactory(
    std::shared_ptr<const AcceptorConfiguration> accConfig)
    : HTTPCodecFactory(getCodecConfigFromAcceptorConfig(*accConfig)),
      accConfig_{std::move(accConfig)} {
}

std::unique_ptr<HTTPCodec> HTTPDefaultSessionCodecFactory::getCodec(
    const std::string& nextProtocol, TransportDirection direction, bool isTLS) {
  DefaultHTTPCodecFactory factory(configFn_());
  if (!isTLS &&
      (accConfig_->plaintextProtocol == http2::kProtocolCleartextString)) {
    return factory.getCodec(http2::kProtocolString, direction, isTLS);
  }
  return factory.getCodec(nextProtocol, direction, isTLS);
}
} // namespace proxygen
