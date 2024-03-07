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

#include <folly/Optional.h>
#include <string>

namespace wangle {

/**
 * Interface for a persistent cache that backs up the cache on
 * storage so that it can be reused. This desribes just the key
 * operations common to any cache. Loading from and syncing to
 * the storage is in the actual implementation of the class and
 * the clients should not have to worry about it.
 */
template <typename K, typename V>
class PersistentCache {
 public:
  virtual ~PersistentCache() = default;

  /**
   * Get a value corresponding to a key
   * @param key string, the key to lookup
   *
   * @returns value associated with key
   */
  virtual folly::Optional<V> get(const K& key) = 0;

  /**
   * Set a value corresponding to a key
   * @param key string, the key to set
   * @param val string, the value to set
   *
   * overwrites value if key has a value associated in the cache
   */
  virtual void put(const K& key, const V& val) = 0;

  /**
   * Clear a cache entry associated with a key
   * @param key string, the key to lookup and clear
   *
   * @return boolean true if any elements are removed, else false
   */
  virtual bool remove(const K& key) = 0;

  /**
   * Empty the contents of the cache
   */
  virtual void clear(bool clearPersistence = false) = 0;

  /**
   * return the size of the cache
   *
   * @returns size_t, the size of the cache
   */

  virtual size_t size() = 0;
};

} // namespace wangle
