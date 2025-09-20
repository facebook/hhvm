/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/pools/generators/coro/JsonFileCoroServerListGenerator.h"

#include <folly/SocketAddress.h>
#include <folly/json/dynamic.h>
#include <folly/json/json.h>

namespace proxygen {

JsonFileCoroServerListGenerator::JsonFileCoroServerListGenerator(
    std::string fileName, std::string poolName)
    : FileCoroServerListGenerator(std::move(fileName)),
      poolName_(std::move(poolName)) {
  if (poolName_.empty()) {
    throw std::invalid_argument("poolName cannot be empty");
  }
}

std::vector<ServerConfig> JsonFileCoroServerListGenerator::parseFile(
    const std::string& contents) {

  std::vector<ServerConfig> servers;
  folly::dynamic parsedJson = folly::parseJson(contents);
  folly::dynamic poolMembers = parsedJson.getDefault(poolName_, -1);
  // If we cannot parse out an arrray out of that.
  if (!poolMembers.isArray()) {
    throw std::invalid_argument("Cannot find a valid pool " + poolName_ +
                                " in file " + fileName_);
  }
  // Now we have an array.
  for (const auto& e : poolMembers) {
    folly::SocketAddress address;
    address.setFromHostPort(e.asString());
    servers.emplace_back(address.getAddressStr(), address);
  }

  return servers;
}

} // namespace proxygen
