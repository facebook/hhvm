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

#include <glog/logging.h>

#include <folly/Likely.h>
#include <folly/json/DynamicConverter.h>
#include <folly/synchronization/Lock.h>

namespace wangle {

template <typename K, typename V, typename M>
folly::Optional<V> LRUInMemoryCache<K, V, M>::get(const K& key) {
  // need to take a write lock since get modifies the LRU
  std::unique_lock writeLock(cacheLock_);
  auto itr = cache_.find(key);
  if (itr != cache_.end()) {
    return folly::Optional<V>(itr->second);
  }
  return folly::none;
}

template <typename K, typename V, typename M>
void LRUInMemoryCache<K, V, M>::put(const K& key, const V& val) {
  std::unique_lock writeLock(cacheLock_);
  cache_.set(key, val);
  incrementVersion();
}

template <typename K, typename V, typename M>
bool LRUInMemoryCache<K, V, M>::remove(const K& key) {
  std::unique_lock writeLock(cacheLock_);
  size_t nErased = cache_.erase(key);
  if (nErased > 0) {
    incrementVersion();
    return true;
  }
  return false;
}

template <typename K, typename V, typename M>
size_t LRUInMemoryCache<K, V, M>::size() const {
  folly::hybrid_lock readLock(cacheLock_);
  return cache_.size();
}

template <typename K, typename V, typename M>
void LRUInMemoryCache<K, V, M>::clear() {
  std::unique_lock writeLock(cacheLock_);
  if (cache_.empty()) {
    return;
  }
  cache_.clear();
  incrementVersion();
}

template <typename K, typename V, typename M>
CacheDataVersion LRUInMemoryCache<K, V, M>::getVersion() const {
  folly::hybrid_lock readLock(cacheLock_);
  return version_;
}

template <typename K, typename V, typename M>
CacheDataVersion LRUInMemoryCache<K, V, M>::loadData(
    const folly::dynamic& data) noexcept {
  bool updated = false;
  std::unique_lock writeLock(cacheLock_);
  try {
    for (const auto& kv : data) {
      cache_.set(folly::convertTo<K>(kv[0]), folly::convertTo<V>(kv[1]));
      updated = true;
    }
  } catch (const folly::TypeError& err) {
    LOG(ERROR) << "Load cache failed with type error: " << err.what();
  } catch (const std::out_of_range& err) {
    LOG(ERROR) << "Load cache failed with key error: " << err.what();
  } catch (const std::exception& err) {
    LOG(ERROR) << "Load cache failed with error: " << err.what();
  }
  if (updated) {
    // we still need to increment the version
    incrementVersion();
  }
  return version_;
}

template <typename K, typename V, typename M>
folly::Optional<std::pair<folly::dynamic, CacheDataVersion>>
LRUInMemoryCache<K, V, M>::convertToKeyValuePairs() noexcept {
  folly::hybrid_lock readLock(cacheLock_);
  try {
    folly::dynamic dynObj = folly::dynamic::array;
    for (const auto& kv : cache_) {
      dynObj.push_back(folly::toDynamic(std::make_pair(kv.first, kv.second)));
    }
    return std::make_pair(std::move(dynObj), version_);
  } catch (const std::exception& err) {
    LOG(ERROR) << "Converting cache to folly::dynamic failed with error: "
               << err.what();
  }
  return folly::none;
}

} // namespace wangle
