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
#include <quic/congestion_control/CongestionControllerFactory.h>
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

const std::vector<std::string> kDefaultSupportedAlpns = {"h3",
                                                         "hq-interop",
                                                         "h3-fb-05",
                                                         "h3-alias-01",
                                                         "h3-alias-02",
                                                         "h3-29",
                                                         "hq-29"};

std::ostream& operator<<(std::ostream& o, const HTTPVersion& v);

/**
 * Params for both clients and servers
 */
struct HQBaseParams {
  // Transport section
  std::vector<quic::QuicVersion> quicVersions{
      quic::QuicVersion::MVFST,
      quic::QuicVersion::MVFST_ALIAS,
      quic::QuicVersion::MVFST_EXPERIMENTAL,
      quic::QuicVersion::MVFST_EXPERIMENTAL3,
      quic::QuicVersion::QUIC_V1,
      quic::QuicVersion::QUIC_V1_ALIAS,
      quic::QuicVersion::QUIC_V1_ALIAS2};
  quic::TransportSettings transportSettings;
  std::string congestionControlName;
  std::optional<quic::CongestionControlType> congestionControl;
  std::shared_ptr<quic::CongestionControllerFactory>
      congestionControllerFactory;
  bool sendKnobFrame{false};

  // HTTP section
  std::string protocol{"h3"};
  HTTPVersion httpVersion;

  std::chrono::milliseconds txnTimeout{std::chrono::seconds(5)};

  // QLogger section
  std::string qLoggerPath;
  bool prettyJson{false};

  // Transport knobs
  std::string transportKnobs;
};

struct HQServerParams : public HQBaseParams {
  size_t serverThreads{0};
  std::string ccpConfig;
  folly::Optional<int64_t> rateLimitPerThread;
  // UDP socket buffer sizes (0 = use system default)
  size_t udpSendBufferSize{0};
  size_t udpRecvBufferSize{0};
};

struct HQClientParams : public HQBaseParams {
  std::string host;
  uint8_t clientCidLength{0};
  folly::Optional<folly::SocketAddress> localAddress;

  std::string pskFilePath;
  std::shared_ptr<quic::QuicPskCache> pskCache;
};

struct HQInvalidParam {
  std::string name;
  std::string value;
  std::string errorMsg;
};

using HQInvalidParams = std::vector<HQInvalidParam>;

} // namespace quic::samples
