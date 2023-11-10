/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include <fizz/server/FizzServerContext.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <proxygen/lib/http/session/HQSession.h>
#include <quic/QuicConstants.h>
#include <quic/fizz/client/handshake/QuicPskCache.h>
#include <quic/state/TransportSettings.h>

namespace quic::samples {

struct HTTPVersion {
  std::string version{"1.1"};
  std::string canonical{"http/1.1"};
  uint16_t major{1};
  uint16_t minor{1};
  bool parse(const std::string&);
};

std::ostream& operator<<(std::ostream& o, const HTTPVersion& v);

/**
 * Params for both clients and servers
 */
struct HQBaseParams {
  // Transport section
  std::string host;
  uint16_t port{0};
  folly::Optional<folly::SocketAddress> localAddress;
  std::vector<quic::QuicVersion> quicVersions{
      quic::QuicVersion::MVFST,
      quic::QuicVersion::MVFST_EXPERIMENTAL,
      quic::QuicVersion::QUIC_V1,
      quic::QuicVersion::QUIC_V1_ALIAS};
  std::vector<std::string> supportedAlpns{proxygen::kH3,
                                          proxygen::kHQ,
                                          proxygen::kH3FBCurrentDraft,
                                          proxygen::kH3AliasV1,
                                          proxygen::kH3CurrentDraft,
                                          proxygen::kHQCurrentDraft};
  quic::TransportSettings transportSettings;
  std::string congestionControlName;
  std::optional<quic::CongestionControlType> congestionControl;
  bool sendKnobFrame{false};

  // HTTP section
  std::string protocol{"h3"};
  HTTPVersion httpVersion;

  std::chrono::milliseconds txnTimeout{std::chrono::seconds(5)};

  // QLogger section
  std::string qLoggerPath;
  bool prettyJson{false};

  // Fizz options
  std::string certificateFilePath;
  std::string keyFilePath;
  std::string pskFilePath;
  std::shared_ptr<quic::QuicPskCache> pskCache;
  fizz::server::ClientAuthMode clientAuth{fizz::server::ClientAuthMode::None};

  // Transport knobs
  std::string transportKnobs;
};

struct HQServerParams : public HQBaseParams {
  size_t serverThreads{0};
  std::string ccpConfig;
  folly::Optional<int64_t> rateLimitPerThread;
};

struct HQInvalidParam {
  std::string name;
  std::string value;
  std::string errorMsg;
};

using HQInvalidParams = std::vector<HQInvalidParam>;

} // namespace quic::samples
