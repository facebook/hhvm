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

class PlainTextFileCoroServerListGenerator
    : public FileCoroServerListGenerator {
 public:
  explicit PlainTextFileCoroServerListGenerator(std::string filename);

  std::vector<ServerConfig> parseFile(const std::string& contents) override;

 private:
  std::string poolName_;
};

} // namespace proxygen
