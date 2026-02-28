/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <folly/SocketAddress.h>
#include <proxygen/lib/pools/generators/MemberGroupConfig.h>

namespace proxygen {

struct ServerConfig {
  ServerConfig(std::string name,
               folly::SocketAddress address,
               std::map<std::string, std::string> properties = {})
      : name(std::move(name)),
        address(std::move(address)),
        properties(std::move(properties)) {
  }

  std::string name;
  folly::SocketAddress address;
  // A field for other addresses that alias the same server.
  // For example a server may have a v4 and a v6 address.
  // Most vector implementations start with a cap of 0 so minimal memory
  // would be used when unused and is why this is still separated from
  // the above preferred address.
  std::vector<folly::SocketAddress> altAddresses;
  std::map<std::string, std::string> properties;
  // Optional parameter. It's only set if a server belongs to a group, which
  // is configured in Pool Config.
  MemberGroupId groupId_{kInvalidPoolMemberGroupId};

  bool operator==(const ServerConfig& other) const {
    return name == other.name && address == other.address &&
           altAddresses == other.altAddresses &&
           properties == other.properties && groupId_ == other.groupId_;
  }

  bool operator<(const ServerConfig& other) const {
    return address < other.address;
  }
};

} // namespace proxygen
