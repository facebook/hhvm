/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/variant.hpp>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <proxygen/httpserver/samples/hq/HQParams.h>

namespace quic::samples {

enum class HQMode { INVALID, CLIENT, SERVER };

std::ostream& operator<<(std::ostream& o, const HQMode& m);

struct HQToolClientParams : public HQBaseParams {
  std::string outdir;
  bool logResponse;
  bool logResponseHeaders;

  folly::Optional<folly::SocketAddress> remoteAddress;
  bool earlyData;
  std::chrono::milliseconds connectTimeout;
  proxygen::HTTPHeaders httpHeaders;
  std::string httpBody;
  proxygen::HTTPMethod httpMethod;
  std::vector<folly::StringPiece> httpPaths;
  bool migrateClient{false};
  bool sendRequestsSequentially;
  std::vector<folly::StringPiece> requestGaps;
};

struct HQToolServerParams : public HQServerParams {
  uint16_t h2port;
  folly::Optional<folly::SocketAddress> localH2Address;
  size_t httpServerThreads;
  std::chrono::milliseconds httpServerIdleTimeout;
  std::vector<int> httpServerShutdownOn;
  bool httpServerEnableContentCompression;
  bool h2cEnabled;
};

struct HQToolParams {
  void setMode(HQMode m) {
    mode = m;
    if (mode == HQMode::CLIENT) {
      params = HQToolClientParams();
    } else if (mode == HQMode::SERVER) {
      params = HQToolServerParams();
    }
  }

  [[nodiscard]] const HQBaseParams& baseParams() const {
    if (mode == HQMode::CLIENT) {
      return (HQBaseParams&)boost::get<HQToolClientParams>(params);
    } else if (mode == HQMode::SERVER) {
      return (HQBaseParams&)boost::get<HQToolServerParams>(params);
    }
    LOG(FATAL) << "Uninit";
  }

  HQBaseParams& baseParams() {
    if (mode == HQMode::CLIENT) {
      return (HQBaseParams&)boost::get<HQToolClientParams>(params);
    } else if (mode == HQMode::SERVER) {
      return (HQBaseParams&)boost::get<HQToolServerParams>(params);
    }
    LOG(FATAL) << "Uninit";
  }

  HQMode mode{HQMode::INVALID};
  std::string logprefix;
  std::string logdir;
  bool logRuntime;
  boost::variant<HQToolClientParams, HQToolServerParams> params;
};

/**
 * A Builder class for HQToolParams that will build HQToolParams from command
 * line parameters processed by GFlag.
 */
class HQToolParamsBuilderFromCmdline {
 public:
  using value_type = std::map<std::string, std::string>::value_type;
  using initializer_list = std::initializer_list<value_type>;

  explicit HQToolParamsBuilderFromCmdline(initializer_list);

  [[nodiscard]] bool valid() const noexcept;

  explicit operator bool() const noexcept {
    return valid();
  }

  [[nodiscard]] const HQInvalidParams& invalidParams() const noexcept;

  HQToolParams build() noexcept;

 private:
  HQInvalidParams invalidParams_;
  HQToolParams hqParams_;
};

// Initialized the parameters from the cmdline flags
const folly::Expected<HQToolParams, HQInvalidParams>
initializeParamsFromCmdline(
    HQToolParamsBuilderFromCmdline::initializer_list initial = {});

// Output convenience
std::ostream& operator<<(std::ostream&, HQToolParams&);

} // namespace quic::samples
