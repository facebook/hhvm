/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Singleton.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/synchronization/CallOnce.h>

namespace facebook::memcache::mcrouter {

/**
 * CPU Thread pool that is shared between router intances.
 *
 * Should not be used directly, use AuxiliaryCPUThreadPoolSingleton instead.
 * Thread pool is lazily initialized on first call to getThreadPool().
 */
class AuxiliaryCPUThreadPool {
 public:
  folly::CPUThreadPoolExecutor& getThreadPool();

 private:
  std::unique_ptr<folly::CPUThreadPoolExecutor> threadPool_;
  folly::once_flag initFlag_;
};

using AuxiliaryCPUThreadPoolSingleton =
    folly::Singleton<AuxiliaryCPUThreadPool>;

} // namespace facebook::memcache::mcrouter
