/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>

#include <folly/String.h>
#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>

namespace {
folly::StringPiece trimTrailingNulls(folly::StringPiece sp) {
  while (!sp.empty() && sp.back() == '\0') {
    sp.pop_back();
  }
  return sp;
}
} // namespace

namespace proxygen {

DefaultHTTPCodecFactory::DefaultHTTPCodecFactory(CodecConfig config)
    : HTTPCodecFactory(config) {
}

std::unique_ptr<HTTPCodec> DefaultHTTPCodecFactory::getCodec(
    const std::string& inChosenProto,
    TransportDirection direction,
    bool isTLS) {

  auto config = configFn_();
  auto chosenProto = trimTrailingNulls(folly::trimWhitespace(inChosenProto));
  auto codecProtocol = getCodecProtocolFromStr(chosenProto);
  switch (codecProtocol) {
    case CodecProtocol::HTTP_2: {
      auto codec = std::make_unique<HTTP2Codec>(direction);
      codec->setStrictValidation(config.strictValidation);
      if (config.h2.headerIndexingStrategy) {
        codec->setHeaderIndexingStrategy(config.h2.headerIndexingStrategy);
      }
      codec->setDebugLevel(config.debugLevel);
      return codec;
    }
    case CodecProtocol::HQ:
    case CodecProtocol::HTTP_3: {
      LOG(WARNING) << __func__ << " doesn't yet support H3";
      return nullptr;
    }
    case CodecProtocol::HTTP_BINARY:
      LOG(WARNING) << __func__ << " doesn't yet support HTTPBinaryCodec";
      return nullptr;
    case CodecProtocol::HTTP_1_1: {
      if (!chosenProto.empty() &&
          !HTTP1xCodec::supportsNextProtocol(chosenProto)) {
        LOG(ERROR) << "Chosen protocol \"" << chosenProto
                   << "\" is unimplemented. ";
        return nullptr;
      }

      auto codec = std::make_unique<HTTP1xCodec>(
          direction, config.h1.forceHTTP1xCodecTo1_1, config.strictValidation);
      if (!isTLS && direction == TransportDirection::DOWNSTREAM &&
          !config.h1.allowedH1UpgradeProtocols.empty()) {
        codec->setAllowedUpgradeProtocols(config.h1.allowedH1UpgradeProtocols);
      }
      return codec;
    }
    default:
      // should be unreachable, getCodecProtocolFromStr returns HTTP_1_1 by
      // default
      return nullptr;
  }
  // unreachable
  return nullptr;
}
} // namespace proxygen
