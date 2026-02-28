/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AuxiliaryIOThreadPool.h"
#include <folly/Singleton.h>

namespace facebook::memcache::mcrouter {

// Number of IO threads to run in IO thread pool.
constexpr size_t kNumIOThreads = 5;

namespace {
folly::Singleton<AuxiliaryIOThreadPool> gAuxiliaryIOThreadPool;
} // namespace

folly::IOThreadPoolExecutorBase& AuxiliaryIOThreadPool::getThreadPool() {
  folly::call_once(initFlag_, [&] {
    threadPool_ = std::make_unique<folly::IOThreadPoolExecutor>(
        kNumIOThreads,
        std::make_shared<folly::NamedThreadFactory>("mcr-ioaux-"));
  });

  return *threadPool_;
}

} // namespace facebook::memcache::mcrouter
