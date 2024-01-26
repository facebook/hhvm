/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace proxygen {

DefaultHTTPCodecFactory::DefaultHTTPCodecFactory(
    bool forceHTTP1xCodecTo1_1, const HeaderIndexingStrategy* strat)
    : forceHTTP1xCodecTo1_1_(forceHTTP1xCodecTo1_1),
      headerIndexingStrategy_(strat) {
}

std::unique_ptr<HTTPCodec> DefaultHTTPCodecFactory::getCodec(
    const std::string& chosenProto,
    TransportDirection direction,
    bool /* isTLS */) {

  if (chosenProto == proxygen::http2::kProtocolString ||
      chosenProto == proxygen::http2::kProtocolCleartextString ||
      chosenProto == proxygen::http2::kProtocolDraftString ||
      chosenProto == proxygen::http2::kProtocolExperimentalString) {
    auto codec = std::make_unique<HTTP2Codec>(direction);
    codec->setStrictValidation(useStrictValidation());
    if (headerIndexingStrategy_) {
      codec->setHeaderIndexingStrategy(headerIndexingStrategy_);
    }
    return codec;
  } else {
    if (!chosenProto.empty() &&
        !HTTP1xCodec::supportsNextProtocol(chosenProto)) {
      LOG(ERROR) << "Chosen upstream protocol "
                 << "\"" << chosenProto << "\" is unimplemented. "
                 << "Attempting to use HTTP/1.1";
    }

    return std::make_unique<HTTP1xCodec>(
        direction, forceHTTP1xCodecTo1_1_, useStrictValidation());
  }
}
} // namespace proxygen
