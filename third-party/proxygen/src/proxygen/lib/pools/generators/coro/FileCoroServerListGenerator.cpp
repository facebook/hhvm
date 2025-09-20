/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/FileUtil.h>
#include <proxygen/lib/pools/generators/coro/FileCoroServerListGenerator.h>

namespace proxygen {

FileCoroServerListGenerator::FileCoroServerListGenerator(std::string fileName)
    : fileName_(std::move(fileName)) {
}

folly::coro::Task<std::vector<ServerConfig>>
FileCoroServerListGenerator::listServers() {
  VLOG(4) << "Looking up server list from File Handle " << fileName_;

  std::string content;
  if (!folly::readFile(fileName_.c_str(), content)) {
    co_yield folly::coro::co_error(
        std::runtime_error("Error reading file " + fileName_));
  }

  auto servers = parseFile(content);

  VLOG(4) << "Found " << servers.size() << " usable servers from File "
          << fileName_;
  co_return servers;
}

} // namespace proxygen
