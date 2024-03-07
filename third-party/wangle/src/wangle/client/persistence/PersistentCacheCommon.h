/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <memory>
#include <mutex>

#include <folly/Executor.h>
#include <folly/Optional.h>

namespace wangle {

namespace client::persistence {
constexpr std::chrono::milliseconds DEFAULT_CACHE_SYNC_INTERVAL =
    std::chrono::milliseconds(5000);
constexpr int DEFAULT_CACHE_SYNC_RETRIES = 3;
constexpr std::size_t DEFAULT_CACHE_CAPACITY = 100;
} // namespace client::persistence

/**
 * A counter that represents a "version" of the data.  This is used to determine
 * if two components have been synced to the same version.
 * A valid version is 1 or higher.  A version of 0 implies no version.
 */
using CacheDataVersion = uint64_t;

constexpr CacheDataVersion kDefaultInitCacheDataVersion = 1;

struct PersistentCacheConfig {
  // Max number of elements to hold in the cache.
  std::size_t capacity{client::persistence::DEFAULT_CACHE_CAPACITY};
  // How often to sync to the persistence (in ms).
  std::chrono::milliseconds syncInterval{
      client::persistence::DEFAULT_CACHE_SYNC_INTERVAL};
  // How many times to retry to sync on failure.
  int nSyncRetries{client::persistence::DEFAULT_CACHE_SYNC_RETRIES};
  // An executor to run sync operations, if not provided, a std::thread
  // dedicated to this cache will be used.
  std::shared_ptr<folly::Executor> executor;
  // Whether the persistence will be loaded inline on constructor thread
  bool inlinePersistenceLoading{true};

  PersistentCacheConfig() = default;

  struct Builder {
    Builder&& setCapacity(std::size_t cacheCapacity) &&;
    Builder&& setSyncInterval(std::chrono::milliseconds interval) &&;
    Builder&& setExecutor(std::shared_ptr<folly::Executor> executor) &&;
    Builder&& setInlinePersistenceLoading(bool loadInline) &&;
    Builder&& setSyncRetries(int retries) &&;

    PersistentCacheConfig build() &&;

   private:
    folly::Optional<std::size_t> capacity;
    std::chrono::milliseconds syncInterval{
        client::persistence::DEFAULT_CACHE_SYNC_INTERVAL};
    int nSyncRetries{client::persistence::DEFAULT_CACHE_SYNC_RETRIES};
    std::shared_ptr<folly::Executor> executor;
    bool inlinePersistenceLoading{true};
  };

 private:
  PersistentCacheConfig(
      std::size_t capacityIn,
      std::chrono::milliseconds syncIntervalIn,
      int nSyncRetriesIn,
      std::shared_ptr<folly::Executor> executorIn,
      bool inlinePersistenceLoadingIn)
      : capacity(capacityIn),
        syncInterval(syncIntervalIn),
        nSyncRetries(nSyncRetriesIn),
        executor(std::move(executorIn)),
        inlinePersistenceLoading(inlinePersistenceLoadingIn) {}
};
} // namespace wangle
