/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/pools/generators/coro/PlainTextFileCoroServerListGenerator.h"

#include <sstream>
#include <string>

#include <folly/SocketAddress.h>

namespace proxygen {

PlainTextFileCoroServerListGenerator::PlainTextFileCoroServerListGenerator(
    std::string fileName)
    : FileCoroServerListGenerator(std::move(fileName)) {
}

std::vector<ServerConfig> PlainTextFileCoroServerListGenerator::parseFile(
    const std::string& contents) {
  std::vector<ServerConfig> servers;
  std::stringstream sstream(contents);
  std::string line;
  while (std::getline(sstream, line)) {
    folly::SocketAddress address;
    address.setFromHostPort(line);
    servers.emplace_back(address.getAddressStr(), address);
  }
  return servers;
}

} // namespace proxygen
