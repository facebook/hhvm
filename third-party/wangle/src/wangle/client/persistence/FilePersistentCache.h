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

#include <folly/FileUtil.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/json/json.h>
#include <wangle/client/persistence/FilePersistenceLayer.h>
#include <wangle/client/persistence/LRUPersistentCache.h>

namespace wangle {

/**
 * A PersistentCache implementation that used a regular file for
 * storage. In memory structure fronts the file and the cache
 * operations happen on it. Loading from and syncing to file are
 * hidden from clients. Sync to file happens asynchronously on
 * a separate thread at a configurable interval. Syncs to file
 * on destruction as well.
 *
 * NOTE NOTE NOTE: Although this class aims to be a cache for arbitrary,
 * it relies heavily on folly::toJson, folly::dynamic and convertTo for
 * serialization and deserialization. So It may not suit your need until
 * true support arbitrary types is written.
 */
template <typename K, typename V, typename M = std::mutex>
class FilePersistentCache : public PersistentCache<K, V> {
 public:
  FilePersistentCache(const std::string& file, PersistentCacheConfig config)
      : cache_(std::make_shared<LRUPersistentCache<K, V, M>>(
            std::move(config),
            std::make_unique<FilePersistenceLayer>(file))) {
    cache_->init();
  }

  ~FilePersistentCache() override {}

  /**
   * PersistentCache operations
   */
  folly::Optional<V> get(const K& key) override {
    return cache_->get(key);
  }

  void put(const K& key, const V& val) override {
    cache_->put(key, val);
  }

  bool remove(const K& key) override {
    return cache_->remove(key);
  }

  void clear(bool clearPersistence = false) override {
    cache_->clear(clearPersistence);
  }

  size_t size() override {
    return cache_->size();
  }

 private:
  FilePersistentCache(const FilePersistentCache&) = delete;
  FilePersistentCache& operator=(const FilePersistentCache&) = delete;

 private:
  typename LRUPersistentCache<K, V, M>::Ptr cache_;
};

} // namespace wangle
