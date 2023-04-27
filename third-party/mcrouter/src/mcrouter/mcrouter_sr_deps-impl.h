/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace facebook {
namespace memcache {

class HostInfo;
using HostInfoPtr = std::shared_ptr<HostInfo>;
using HostWithShard = std::pair<HostInfoPtr, int64_t>;

class HostInfoLocation {
 public:
  HostInfoLocation(){};

  const std::string& getIp() const {
    return ip_;
  }
  uint16_t getPort() const {
    return 0;
  }
  const uint16_t* getTWTaskID() const {
    return nullptr;
  }

 private:
  const std::string ip_ = "127.0.0.1";
};

class HostInfo {
 public:
  HostInfo(){};

  const HostInfoLocation& location() {
    return location_;
  }

 private:
  const HostInfoLocation location_;
};

class RoutingHintEncoder {
 public:
  static uint64_t encodeRoutingHint(const HostWithShard&) {
    return 0;
  }
};

struct SRHost {
 public:
  SRHost(){};
  const std::string& getIp() const {
    return "127.0.0.1";
  }
  uint16_t getPort() const {
    return 0;
  }
  const std::optional<uint16_t> getTwTaskId() const {
    return std::nullopt;
  }
};

} // namespace memcache
} // namespace facebook
