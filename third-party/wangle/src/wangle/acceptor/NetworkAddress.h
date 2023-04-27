/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/SocketAddress.h>

namespace wangle {

/**
 * A simple wrapper around SocketAddress that represents
 * a network in CIDR notation
 */
class NetworkAddress {
 public:
  /**
   * Create a NetworkAddress for an addr/prefixLen
   * @param addr         IPv4 or IPv6 address of the network
   * @param prefixLen    Prefix length, in bits
   */
  NetworkAddress(const folly::SocketAddress& addr, unsigned prefixLen)
      : addr_(addr), prefixLen_(prefixLen) {}

  /** Get the network address */
  const folly::SocketAddress& getAddress() const {
    return addr_;
  }

  /** Get the prefix length in bits */
  unsigned getPrefixLength() const {
    return prefixLen_;
  }

  /** Check whether a given address lies within the network */
  bool contains(const folly::SocketAddress& addr) const {
    return addr_.prefixMatch(addr, prefixLen_);
  }

  /** Comparison operator to enable use in ordered collections */
  bool operator<(const NetworkAddress& other) const {
    if (addr_ < other.addr_) {
      return true;
    } else if (other.addr_ < addr_) {
      return false;
    } else {
      return (prefixLen_ < other.prefixLen_);
    }
  }

  bool operator==(const NetworkAddress& other) const {
    return addr_ == other.addr_ && prefixLen_ == other.prefixLen_;
  }

 private:
  folly::SocketAddress addr_;
  unsigned prefixLen_;
};

} // namespace wangle
