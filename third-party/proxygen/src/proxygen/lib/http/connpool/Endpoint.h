/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>

namespace proxygen {

/**
 * Simple class representing an endpoint by (hostname, port, http/s).
 * Optionally used by SessionHolder if caller wants to store sessions from
 * different endpoints in a single SessionPool.
 */
class Endpoint {
 public:
  explicit Endpoint(const std::string& hostname, uint16_t port, bool isSecure)
      : hostname_(hostname), port_(port), isSecure_(isSecure) {
    hash_ =
        std::hash<std::string>()(hostname_) ^ (port_ << 1) ^ (isSecure_ << 17);
  }

  explicit Endpoint(const folly::SocketAddress& addr, bool isSecure)
      : Endpoint(addr.getAddressStr(), addr.getPort(), isSecure){};

  const std::string& getHostname() const {
    return hostname_;
  }

  uint16_t getPort() const {
    return port_;
  }

  bool isSecure() const {
    return isSecure_;
  }

  size_t getHash() const {
    return hash_;
  }

  void describe(std::ostream& os) const {
    os << hostname_ << ":" << port_ << ":" << (isSecure_ ? "" : "in")
       << "secure";
  }

 private:
  std::string hostname_;
  uint16_t port_{0};
  std::size_t hash_{0};
  bool isSecure_{false};
};

struct EndpointHash {
  std::size_t operator()(const Endpoint& e) const {
    return e.getHash();
  }
};

struct EndpointEqual {
  bool operator()(const Endpoint& lhs, const Endpoint& rhs) const {
    return lhs.getHostname() == rhs.getHostname() &&
           lhs.getPort() == rhs.getPort() && lhs.isSecure() == rhs.isSecure();
  }
};

} // namespace proxygen
