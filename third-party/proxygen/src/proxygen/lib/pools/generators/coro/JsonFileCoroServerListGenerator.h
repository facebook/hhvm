/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/pools/generators/coro/FileCoroServerListGenerator.h>

namespace proxygen {

class JsonFileCoroServerListGenerator : public FileCoroServerListGenerator {
 public:
  JsonFileCoroServerListGenerator(std::string filename, std::string poolName);

  std::vector<ServerConfig> parseFile(const std::string& contents) override;

 private:
  std::string poolName_;
};

} // namespace proxygen
