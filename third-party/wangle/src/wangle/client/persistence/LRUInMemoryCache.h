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

#include <utility>

#include <folly/Optional.h>
#include <folly/container/EvictingCacheMap.h>
#include <folly/json/dynamic.h>
#include <wangle/client/persistence/PersistentCacheCommon.h>

namespace wangle {

/**
 * A threadsafe cache map that delegates to an EvictingCacheMap and maintains
 * a version of the data.
 */
template <typename K, typename V, typename MutexT>
class LRUInMemoryCache {
 public:
  /**
   * Create with the specified capacity.
   */
  explicit LRUInMemoryCache(size_t capacity) : cache_(capacity) {}
  ~LRUInMemoryCache() = default;

  folly::Optional<V> get(const K& key);
  void put(const K& key, const V& val);
  bool remove(const K& key);
  size_t size() const;
  void clear();

  CacheDataVersion getVersion() const;

  /**
   * Loads the list of kv pairs into the cache and bumps version.
   * Returns the new cache version.
   */
  CacheDataVersion loadData(const folly::dynamic& kvPairs) noexcept;

  /**
   * Get the cache data as a list of kv pairs along with the version
   */
  folly::Optional<std::pair<folly::dynamic, CacheDataVersion>>
  convertToKeyValuePairs() noexcept;

  /**
   * Determine if the cache has changed since the specified version
   */
  bool hasChangedSince(CacheDataVersion version) const {
    return getVersion() != version;
  }

 private:
  // must be called under a write lock
  void incrementVersion() {
    ++version_;
    // if a uint64_t is incremented a billion times a second, it will still take
    // 585 years to wrap around, so don't bother
  }

  folly::EvictingCacheMap<K, V> cache_;
  // Version always starts at 1
  CacheDataVersion version_{kDefaultInitCacheDataVersion};
  // mutable so we can take read locks in const methods
  mutable MutexT cacheLock_;
};

} // namespace wangle

#include <wangle/client/persistence/LRUInMemoryCache-inl.h>
