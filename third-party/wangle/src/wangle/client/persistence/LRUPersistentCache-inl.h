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

#include <folly/DynamicConverter.h>
#include <folly/FileUtil.h>
#include <folly/ScopeGuard.h>
#include <folly/json.h>
#include <folly/portability/SysTime.h>
#include <folly/synchronization/Lock.h>
#include <folly/system/ThreadName.h>
#include <atomic>
#include <cerrno>
#include <functional>

namespace wangle {

template <typename K, typename V, typename MutexT>
LRUPersistentCache<K, V, MutexT>::LRUPersistentCache(
    PersistentCacheConfig config,
    std::unique_ptr<CachePersistence> persistence)
    : cache_(config.capacity),
      syncInterval_(config.syncInterval),
      nSyncRetries_(config.nSyncRetries),
      executor_(std::move(config.executor)),
      inlinePersistenceLoading_(config.inlinePersistenceLoading) {
  if (persistence) {
    std::shared_ptr<CachePersistence> sharedPersistence(std::move(persistence));
    {
      std::unique_lock writeLock(persistenceLock_);
      std::swap(persistence_, sharedPersistence);
    }
  }
}

template <typename K, typename V, typename MutexT>
LRUPersistentCache<K, V, MutexT>::~LRUPersistentCache() {
  if (executor_) {
    oneShotSync();
    return;
  }
  {
    // tell syncer to wake up and quit
    std::lock_guard<std::mutex> lock(stopSyncerMutex_);
    stopSyncer_ = true;
    stopSyncerCV_.notify_all();
  }
  if (syncer_.joinable()) {
    syncer_.join();
  }
}

template <typename K, typename V, typename MutexT>
void LRUPersistentCache<K, V, MutexT>::init() {
  if (inlinePersistenceLoading_) {
    // load the cache. be silent if load fails, we just drop the cache
    // and start from scratch.
    setPersistenceHelper(true);
  }
  if (!executor_) {
    // start the syncer thread. done at the end of construction so that the
    // cache is fully initialized before being passed to the syncer thread.
    syncer_ =
        std::thread(&LRUPersistentCache<K, V, MutexT>::syncThreadMain, this);
    return;
  }
  executor_->add([self = folly::to_weak_ptr(this->shared_from_this())]() {
    if (auto sharedSelf = self.lock()) {
      sharedSelf->setPersistenceHelper(true);
    }
  });
}

template <typename K, typename V, typename MutexT>
void LRUPersistentCache<K, V, MutexT>::put(const K& key, const V& val) {
  blockingAccessInMemCache().put(key, val);

  if (!executor_) {
    return;
  }

  // Within the same time interval as the last sync
  if (std::chrono::steady_clock::now() - lastExecutorScheduleTime_ <
      syncInterval_) {
    return;
  }

  // Sync already scheduled
  if (executorScheduled_.test_and_set()) {
    return;
  }

  lastExecutorScheduleTime_ = std::chrono::steady_clock::now();
  std::weak_ptr<LRUPersistentCache<K, V, MutexT>> weakSelf =
      this->shared_from_this();
  executor_->add([self = std::move(weakSelf)]() {
    if (auto sharedSelf = self.lock()) {
      sharedSelf->oneShotSync();
    }
  });
}

template <typename K, typename V, typename MutexT>
bool LRUPersistentCache<K, V, MutexT>::hasPendingUpdates() {
  folly::hybrid_lock readLock(persistenceLock_);
  if (!persistence_) {
    return false;
  }
  return blockingAccessInMemCache().hasChangedSince(
      persistence_->getLastPersistedVersion());
}

template <typename K, typename V, typename MutexT>
void* LRUPersistentCache<K, V, MutexT>::syncThreadMain(void* arg) {
  folly::setThreadName("lru-sync-thread");

  auto self = static_cast<LRUPersistentCache<K, V, MutexT>*>(arg);
  self->sync();
  return nullptr;
}

template <typename K, typename V, typename MutexT>
void LRUPersistentCache<K, V, MutexT>::oneShotSync() {
  // load the cache. be silent if load fails, we just drop the cache
  // and start from scratch.
  setPersistenceHelper(true);
  auto persistence = getPersistence();
  if (persistence && !syncNow(*persistence)) {
    // track failures and give up if we tried too many times
    ++nSyncTries_;
    if (nSyncTries_ == nSyncRetries_) {
      persistence->setPersistedVersion(cache_.getVersion());
      nSyncTries_ = 0;
    }
  } else {
    nSyncTries_ = 0;
  }

  executorScheduled_.clear();
}

template <typename K, typename V, typename MutexT>
void LRUPersistentCache<K, V, MutexT>::sync() {
  // load the cache. be silent if load fails, we just drop the cache
  // and start from scratch.
  setPersistenceHelper(true);
  // keep running as long the destructor signals to stop or
  // there are pending updates that are not synced yet
  std::unique_lock<std::mutex> stopSyncerLock(stopSyncerMutex_);
  int nSyncFailures = 0;
  while (true) {
    auto persistence = getPersistence();
    if (stopSyncer_) {
      if (!persistence ||
          !cache_.hasChangedSince(persistence->getLastPersistedVersion())) {
        break;
      }
    }

    if (persistence && !syncNow(*persistence)) {
      // track failures and give up if we tried too many times
      ++nSyncFailures;
      if (nSyncFailures == nSyncRetries_) {
        persistence->setPersistedVersion(cache_.getVersion());
        nSyncFailures = 0;
      }
    } else {
      nSyncFailures = 0;
    }

    if (!stopSyncer_) {
      stopSyncerCV_.wait_for(stopSyncerLock, syncInterval_);
    }
  }
}

template <typename K, typename V, typename MutexT>
bool LRUPersistentCache<K, V, MutexT>::syncNow(CachePersistence& persistence) {
  // check if we need to sync.  There is a chance that someone can
  // update cache_ between this check and the convert below, but that
  // is ok.  The persistence layer would have needed to update anyway
  // and will just get the latest version.
  if (!cache_.hasChangedSince(persistence.getLastPersistedVersion())) {
    // nothing to do
    return true;
  }

  // serialize the current contents of cache under lock
  auto serializedCacheAndVersion = cache_.convertToKeyValuePairs();
  if (!serializedCacheAndVersion) {
    LOG(ERROR) << "Failed to convert cache for serialization.";
    return false;
  }

  auto& kvPairs = std::get<0>(serializedCacheAndVersion.value());
  auto& version = std::get<1>(serializedCacheAndVersion.value());
  auto persisted =
      persistence.persistVersionedData(std::move(kvPairs), version);

  return persisted;
}

template <typename K, typename V, typename MutexT>
std::shared_ptr<CachePersistence>
LRUPersistentCache<K, V, MutexT>::getPersistence() {
  folly::hybrid_lock readLock(persistenceLock_);
  return persistence_;
}

template <typename K, typename V, typename MutexT>
void LRUPersistentCache<K, V, MutexT>::setPersistenceHelper(
    bool syncVersion) noexcept {
  std::unique_lock writeLock(persistenceLock_);
  if (persistenceLoadedSemaphore_.ready()) {
    return;
  }
  // load the persistence data into memory
  if (persistence_) {
    auto version = load(*persistence_);
    if (version && syncVersion) {
      persistence_->setPersistedVersion(*version);
    }
  }
  persistenceLoadedSemaphore_.post();
}

template <typename K, typename V, typename MutexT>
LRUInMemoryCache<K, V, MutexT>&
LRUPersistentCache<K, V, MutexT>::blockingAccessInMemCache() {
  persistenceLoadedSemaphore_.wait();
  return cache_;
}

template <typename K, typename V, typename MutexT>
folly::Optional<CacheDataVersion> LRUPersistentCache<K, V, MutexT>::load(
    CachePersistence& persistence) noexcept {
  auto kvPairs = persistence.load();
  if (!kvPairs) {
    return folly::none;
  }
  return cache_.loadData(kvPairs.value());
}
} // namespace wangle
