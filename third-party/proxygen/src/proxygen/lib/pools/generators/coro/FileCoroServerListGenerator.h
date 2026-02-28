/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/pools/generators/coro/CoroServerListGenerator.h>

namespace proxygen {

/*
 * The base class for a CoroServerListGenerator implementation that gets the
 * server list from a file with some encoding.
 */
class FileCoroServerListGenerator : public CoroServerListGenerator {
 public:
  explicit FileCoroServerListGenerator(std::string fileName);

  FileCoroServerListGenerator(FileCoroServerListGenerator&&) noexcept = default;
  FileCoroServerListGenerator& operator=(
      FileCoroServerListGenerator&&) noexcept = default;

  // Forbidden copy constructor and assignment operator
  FileCoroServerListGenerator(FileCoroServerListGenerator const&) = delete;
  FileCoroServerListGenerator& operator=(FileCoroServerListGenerator const&) =
      delete;

  ~FileCoroServerListGenerator() override = default;

  folly::coro::Task<std::vector<ServerConfig>> listServers() override;

 protected:
  virtual std::vector<ServerConfig> parseFile(const std::string& contents) = 0;

  std::string fileName_;
};

} // namespace proxygen
