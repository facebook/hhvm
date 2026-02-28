/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/McrouterManager.h"
#include <folly/Singleton.h>
#include "mcrouter/lib/AuxiliaryCPUThreadPool.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

namespace detail {

namespace {
folly::Singleton<McrouterManager> gMcrouterManager_;
}

McrouterManager::McrouterManager() {
  scheduleSingletonCleanup();
  // Instantiate AuxiliaryCPUThreadPoolSingleton to make sure that it gets
  // destroyed after McrouterManager is destroyed.
  AuxiliaryCPUThreadPoolSingleton::try_get();
}

McrouterManager::~McrouterManager() {
  freeAllMcrouters();
}

void McrouterManager::freeAllMcrouters() {
  std::lock_guard<std::mutex> lg(mutex_);
  mcrouters_.clear();
}

std::shared_ptr<McrouterManager> McrouterManager::getSingletonInstance() {
  return gMcrouterManager_.try_get();
}
} // namespace detail

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
