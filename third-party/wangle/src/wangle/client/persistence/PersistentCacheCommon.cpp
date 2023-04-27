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

#include <wangle/client/persistence/PersistentCacheCommon.h>

namespace wangle {

PersistentCacheConfig::Builder&& PersistentCacheConfig::Builder::setCapacity(
    std::size_t cacheCapacity) && {
  capacity = cacheCapacity;
  return std::move(*this);
}

PersistentCacheConfig::Builder&&
PersistentCacheConfig::Builder::setSyncInterval(
    std::chrono::milliseconds interval) && {
  syncInterval = interval;
  return std::move(*this);
}

PersistentCacheConfig::Builder&& PersistentCacheConfig::Builder::setExecutor(
    std::shared_ptr<folly::Executor> executorIn) && {
  executor = std::move(executorIn);
  return std::move(*this);
}

PersistentCacheConfig::Builder&&
PersistentCacheConfig::Builder::setInlinePersistenceLoading(
    bool loadInline) && {
  inlinePersistenceLoading = loadInline;
  return std::move(*this);
}

PersistentCacheConfig::Builder&& PersistentCacheConfig::Builder::setSyncRetries(
    int retries) && {
  nSyncRetries = retries;
  return std::move(*this);
}

PersistentCacheConfig PersistentCacheConfig::Builder::build() && {
  return PersistentCacheConfig(
      capacity.value(),
      syncInterval,
      nSyncRetries,
      std::move(executor),
      inlinePersistenceLoading);
}
} // namespace wangle
