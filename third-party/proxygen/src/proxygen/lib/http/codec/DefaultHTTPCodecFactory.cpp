/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>

#include <proxygen/lib/http/codec/CodecProtocol.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace proxygen {

DefaultHTTPCodecFactory::DefaultHTTPCodecFactory(
    bool forceHTTP1xCodecTo1_1,
    const HeaderIndexingStrategy* strat,
    std::list<std::string> allowedH1UpgradeProtocols)
    : forceHTTP1xCodecTo1_1_(forceHTTP1xCodecTo1_1),
      headerIndexingStrategy_(strat),
      allowedH1UpgradeProtocols_(std::move(allowedH1UpgradeProtocols)) {
}

std::unique_ptr<HTTPCodec> DefaultHTTPCodecFactory::getCodec(
    const std::string& chosenProto, TransportDirection direction, bool isTLS) {

  auto codecProtocol = getCodecProtocolFromStr(chosenProto);
  switch (codecProtocol) {
    case CodecProtocol::HTTP_2: {
      auto codec = std::make_unique<HTTP2Codec>(direction);
      codec->setStrictValidation(useStrictValidation());
      if (headerIndexingStrategy_) {
        codec->setHeaderIndexingStrategy(headerIndexingStrategy_);
      }
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
          direction, forceHTTP1xCodecTo1_1_, useStrictValidation());
      if (!isTLS && direction == TransportDirection::DOWNSTREAM &&
          !allowedH1UpgradeProtocols_.empty()) {
        codec->setAllowedUpgradeProtocols(allowedH1UpgradeProtocols_);
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
