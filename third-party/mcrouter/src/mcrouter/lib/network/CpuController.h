/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

#include <folly/io/async/EventBase.h>

#include "mcrouter/lib/network/ServerLoad.h"

namespace facebook {
namespace memcache {

struct CpuControllerOptions {
  /**
   * How frequently we should collect data.
   * 0 to disable collecting data completely.
   */
  std::chrono::milliseconds dataCollectionInterval{0};

  bool shouldEnable() const noexcept {
    return dataCollectionInterval.count() > 0;
  }
};

class CpuController : public std::enable_shared_from_this<CpuController> {
 public:
  CpuController(const CpuControllerOptions& opts, folly::EventBase& evb);

  /**
   * Gets the load on the server.
   */
  ServerLoad getServerLoad() const noexcept {
    return ServerLoad::fromPercentLoad(percentLoad_.load());
  }

  void start();
  void stop();

 private:
  // The function responsible for logging the CPU utilization.
  void cpuLoggingFn();

  // Updates cpu utilization value.
  void update(double cpuUtil);

  folly::EventBase& evb_;
  std::vector<uint64_t> prev_{8};
  std::chrono::milliseconds dataCollectionInterval_;
  std::atomic<double> percentLoad_{0.0};
  std::atomic<bool> stopController_{false};
  bool firstLoop_{true};
};

} // namespace memcache
} // namespace facebook
