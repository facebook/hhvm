/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/CodecProtocol.h>

#include <folly/String.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

#include <glog/logging.h>

namespace proxygen {

namespace {

#ifndef CLANG_LAZY_INIT_TEST
#define CLANG_LAZY_INIT_TEST
#endif

CLANG_LAZY_INIT_TEST static const std::string http_1_1 = "http/1.1";
CLANG_LAZY_INIT_TEST static const std::string http_2 = "http/2";
CLANG_LAZY_INIT_TEST static const std::string hq = "hq";
CLANG_LAZY_INIT_TEST static const std::string h3 = "h3";
CLANG_LAZY_INIT_TEST static const std::string http_binary = "bhttp";
CLANG_LAZY_INIT_TEST static const std::string tunnel_lite = "lite";
} // namespace

extern CodecProtocol getCodecProtocolFromStr(folly::StringPiece protocolStr) {
  if (protocolStr == http_1_1) {
    return CodecProtocol::HTTP_1_1;
  } else if (protocolStr == http_2 || protocolStr == http2::kProtocolString ||
             protocolStr == http2::kProtocolCleartextString) {
    return CodecProtocol::HTTP_2;
  } else if (protocolStr.find(hq) == 0) {
    return CodecProtocol::HQ;
  } else if (protocolStr.find(h3) == 0) {
    return CodecProtocol::HTTP_3;
  } else if (protocolStr.find(http_binary) == 0) {
    return CodecProtocol::HTTP_BINARY;
  } else if (protocolStr.find(tunnel_lite) == 0) {
    return CodecProtocol::TUNNEL_LITE;
  } else {
    // return default protocol
    return CodecProtocol::HTTP_1_1;
  }
}

extern const std::string& getCodecProtocolString(CodecProtocol proto) {
  switch (proto) {
    case CodecProtocol::HTTP_1_1:
      return http_1_1;
    case CodecProtocol::HTTP_2:
      return http_2;
    case CodecProtocol::HTTP_3:
      return h3;
    case CodecProtocol::HQ:
      return hq;
    case CodecProtocol::HTTP_BINARY:
      return http_binary;
    case CodecProtocol::TUNNEL_LITE:
      return tunnel_lite;
  }
  LOG(FATAL) << "Unreachable";
}

extern bool isValidCodecProtocolStr(folly::StringPiece protocolStr) {
  return protocolStr == http_1_1 || protocolStr == http2::kProtocolString ||
         protocolStr == http2::kProtocolCleartextString ||
         protocolStr == http_2 || protocolStr == hq ||
         protocolStr == http_binary;
}

extern bool isHTTP1_1CodecProtocol(CodecProtocol protocol) {
  return protocol == CodecProtocol::HTTP_1_1;
}

extern bool isHTTP2CodecProtocol(CodecProtocol protocol) {
  return protocol == CodecProtocol::HTTP_2;
}

extern bool isHQCodecProtocol(CodecProtocol protocol) {
  return protocol == CodecProtocol::HQ || protocol == CodecProtocol::HTTP_3;
}

extern bool isHTTPBinaryCodecProtocol(CodecProtocol protocol) {
  return protocol == CodecProtocol::HTTP_BINARY;
}

extern bool isParallelCodecProtocol(CodecProtocol protocol) {
  return isHTTP2CodecProtocol(protocol);
}

extern bool serverAcceptedUpgrade(const std::string& clientUpgrade,
                                  const std::string& serverUpgrade) {
  if (clientUpgrade.empty() || serverUpgrade.empty()) {
    return false;
  }

  // Should be a comma separated list of protocols, like NPN
  std::vector<folly::StringPiece> clientProtocols;
  folly::split(',', clientUpgrade, clientProtocols, /*ignoreEmpty=*/true);

  std::vector<folly::StringPiece> serverProtocols;
  folly::split(',', serverUpgrade, serverProtocols, /*ignoreEmpty=*/true);
  for (auto& sp : serverProtocols) {
    sp = folly::trimWhitespace(sp);
  }

  for (const auto& cp : clientProtocols) {
    auto cpt = folly::trimWhitespace(cp);
    return std::any_of(
        serverProtocols.begin(), serverProtocols.end(), [cpt](const auto& sp) {
          return cpt.equals(sp, folly::AsciiCaseInsensitive{});
        });
  }

  return false;
}

const folly::Optional<HTTPCodec::ExAttributes> HTTPCodec::NoExAttributes =
    folly::none;
const folly::Optional<HTTPCodec::StreamID> HTTPCodec::NoStream = folly::none;
const folly::Optional<uint8_t> HTTPCodec::NoPadding = folly::none;

} // namespace proxygen
