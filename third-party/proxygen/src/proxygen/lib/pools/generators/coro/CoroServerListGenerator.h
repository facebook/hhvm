/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/coro/Task.h>
#include <proxygen/lib/pools/generators/MemberGroupConfig.h>
#include <proxygen/lib/pools/generators/ServerConfig.h>
#include <proxygen/lib/pools/generators/ServerListGenerator.h>

namespace proxygen {

/**
 * CoroServerListGenerator is an abstract class that provides an API for
 * obtaining a list of servers.
 *
 * The same CoroServerListGenerator may be invoked periodically, and the list of
 * servers returned may change over time.
 */
class CoroServerListGenerator : public ServerListGeneratorIf {
 public:
  explicit CoroServerListGenerator(folly::EventBase* eventBase = nullptr)
      : ServerListGeneratorIf(eventBase) {
  }

  CoroServerListGenerator(CoroServerListGenerator&&) = default;
  CoroServerListGenerator& operator=(CoroServerListGenerator&&) = default;

  // Forbidden copy constructor and assignment operator
  CoroServerListGenerator(CoroServerListGenerator const&) = delete;
  CoroServerListGenerator& operator=(CoroServerListGenerator const&) = delete;

  ~CoroServerListGenerator() override = default;

  /**
   * Generate the list of servers and invoke the callback when completed.
   */
  virtual folly::coro::Task<std::vector<ServerConfig>> listServers() = 0;
};

} // namespace proxygen
