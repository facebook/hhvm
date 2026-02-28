/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Singleton.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/synchronization/CallOnce.h>

namespace facebook::memcache::mcrouter {

/**
 * IO Thread pool that is shared between router intances.
 *
 * Should not be used directly, use AuxiliaryIOThreadPoolSingleton instead.
 * Thread pool is lazily initialized on first call to getThreadPool().
 */
class AuxiliaryIOThreadPool {
 public:
  folly::IOThreadPoolExecutorBase& getThreadPool();

 private:
  std::unique_ptr<folly::IOThreadPoolExecutorBase> threadPool_;
  folly::once_flag initFlag_;
};

using AuxiliaryIOThreadPoolSingleton = folly::Singleton<AuxiliaryIOThreadPool>;

} // namespace facebook::memcache::mcrouter
