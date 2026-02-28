/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AuxiliaryCPUThreadPool.h"
#include <folly/Singleton.h>

namespace facebook::memcache::mcrouter {

// Number of CPU threads to run in CPU thread pool.
constexpr size_t kNumCPUThreads = 5;

namespace {
folly::Singleton<AuxiliaryCPUThreadPool> gAuxiliaryCPUThreadPool;
} // namespace

folly::CPUThreadPoolExecutor& AuxiliaryCPUThreadPool::getThreadPool() {
  folly::call_once(initFlag_, [&] {
    threadPool_ = std::make_unique<folly::CPUThreadPoolExecutor>(
        kNumCPUThreads,
        std::make_shared<folly::NamedThreadFactory>("mcr-cpuaux-"));
  });

  return *threadPool_;
}

} // namespace facebook::memcache::mcrouter
