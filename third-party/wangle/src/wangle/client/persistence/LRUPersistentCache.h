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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <map>
#include <thread>

#include <folly/Executor.h>
#include <folly/json/dynamic.h>
#include <folly/synchronization/SaturatingSemaphore.h>
#include <wangle/client/persistence/LRUInMemoryCache.h>
#include <wangle/client/persistence/PersistentCache.h>
#include <wangle/client/persistence/PersistentCacheCommon.h>

namespace wangle {

/**
 * The underlying persistence layer interface.  Implementations may
 * write to file, db, /dev/null, etc.
 */
class CachePersistence {
 public:
  virtual ~CachePersistence() = default;

  /**
   * Persist a folly::dynamic array of key value pairs at the
   * specified version.  Returns true if persistence succeeded.
   */
  bool persistVersionedData(
      const folly::dynamic& kvPairs,
      const CacheDataVersion& version) {
    auto result = persist(kvPairs);
    if (result) {
      persistedVersion_ = version;
    }
    return result;
  }

  /**
   * Get the last version of the data that was successfully persisted.
   */
  virtual CacheDataVersion getLastPersistedVersion() const {
    return persistedVersion_;
  }

  /**
   * Force set a persisted version.  This is primarily for when a persistence
   * layer acts as the initial source of data for some version tracking cache.
   */
  virtual void setPersistedVersion(CacheDataVersion version) noexcept {
    persistedVersion_ = version;
  }

  /**
   * Persist a folly::dynamic array of key value pairs.
   * Returns true on success.
   */
  virtual bool persist(const folly::dynamic& kvPairs) noexcept = 0;

  /**
   * Returns a list of key value pairs that are present in this
   * persistence store.
   */
  virtual folly::Optional<folly::dynamic> load() noexcept = 0;

  /**
   * Clears Persistent cache
   */
  virtual void clear() = 0;

 private:
  CacheDataVersion persistedVersion_{kDefaultInitCacheDataVersion};
};

/**
 * A PersistentCache implementation that used a CachePersistence for
 * storage. In memory structure fronts the persistence and the cache
 * operations happen on it. Loading from and syncing to persistence are
 * hidden from clients. Sync to persistence happens asynchronously on
 * a separate thread at a configurable interval. Syncs to persistence
 * on destruction as well.
 *
 * The in memory structure is an EvictingCacheMap which causes this class
 * to evict entries in an LRU fashion.
 *
 * NOTE NOTE NOTE: Although this class aims to be a cache for arbitrary,
 * it relies heavily on folly::toJson, folly::dynamic and convertTo for
 * serialization and deserialization. So It may not suit your need until
 * true support arbitrary types is written.
 */
template <typename K, typename V, typename MutexT = std::mutex>
class LRUPersistentCache
    : public PersistentCache<K, V>,
      public std::enable_shared_from_this<LRUPersistentCache<K, V, MutexT>> {
 public:
  using Ptr = std::shared_ptr<LRUPersistentCache<K, V, MutexT>>;

  /**
   * LRUPersistentCache constructor
   *
   * On write failures, the sync will happen again up to nSyncRetries times.
   * Once failed nSyncRetries amount of time, then it will give up and not
   * attempt to sync again until another update occurs.
   *
   * On reaching capacity limit, LRU items are evicted.
   */
  explicit LRUPersistentCache(
      PersistentCacheConfig config,
      std::unique_ptr<CachePersistence> persistence = nullptr);

  /**
   * LRUPersistentCache Destructor
   *
   * Signals the syncer thread to stop, waits for any pending syncs to
   * be done.
   */
  ~LRUPersistentCache() override;

  /**
   * Loads the cache inline on the calling thread, and starts of the syncer
   * thread that periodically syncs the cache to persistence if the cache is
   * running thread mode.
   *
   * If persistence is specified from constructor, the cache is initially loaded
   * with the contents from it. If load fails, then cache starts empty.
   */
  void init();

  /**
   * Check if there are updates that need to be synced to persistence
   */
  bool hasPendingUpdates();

  /**
   * PersistentCache operations
   */
  folly::Optional<V> get(const K& key) override {
    return blockingAccessInMemCache().get(key);
  }

  void put(const K& key, const V& val) override;

  bool remove(const K& key) override {
    return blockingAccessInMemCache().remove(key);
  }

  void clear(bool clearPersistence = false) override {
    blockingAccessInMemCache().clear();
    if (clearPersistence) {
      auto persistence = getPersistence();
      if (persistence) {
        persistence->clear();
      }
    }
  }

  size_t size() override {
    return blockingAccessInMemCache().size();
  }

 private:
  LRUPersistentCache(const LRUPersistentCache&) = delete;
  LRUPersistentCache& operator=(const LRUPersistentCache&) = delete;

  /**
   * Helper to set persistence that will load the persistence data
   * into memory and optionally sync versions
   */
  void setPersistenceHelper(bool syncVersion) noexcept;

  /**
   * Load the contents of the persistence passed to constructor in to the
   * in-memory cache. Failure to read will result in no changes to the
   * in-memory data.  That is, if in-memory entries exist, and loading
   * fails, the in-memory data remains and will sync down to the underlying
   * persistence layer on the next sync.
   *
   * Failure to read inclues IO errors and deserialization errors.
   *
   * @returns the in memory cache's new version
   */
  folly::Optional<CacheDataVersion> load(
      CachePersistence& persistence) noexcept;

  /**
   * The syncer thread's function. Syncs to the persistence, if necessary,
   * after every syncInterval_ seconds.
   */
  void sync();
  void oneShotSync();
  static void* syncThreadMain(void* arg);

  /**
   * Helper to sync routine above that actualy does the serialization
   * and writes to persistence.
   *
   * @returns boolean, true on successful serialization and write to
   *                    persistence, false otherwise
   */
  bool syncNow(CachePersistence& persistence);

  /**
   * Helper to get the persistence layer under lock since it will be called
   * by syncer thread and setters call from any thread.
   */
  std::shared_ptr<CachePersistence> getPersistence();

  /**
   * Block the caller thread until persistence has been loaded into the
   * in-memory cache. Return cache_ after persistence loading is done.
   */
  LRUInMemoryCache<K, V, MutexT>& blockingAccessInMemCache();

 private:
  // Our threadsafe in memory cache
  LRUInMemoryCache<K, V, MutexT> cache_;

  // used to signal syncer thread
  bool stopSyncer_{false};
  // mutex used to synchronize syncer_ on destruction, tied to stopSyncerCV_
  std::mutex stopSyncerMutex_;
  // condvar used to wakeup syncer on exit
  std::condition_variable stopSyncerCV_;

  // We do not schedule the same task more than one to the executor
  std::atomic_flag executorScheduled_ = ATOMIC_FLAG_INIT;

  // sync interval in milliseconds
  const std::chrono::milliseconds syncInterval_{
      client::persistence::DEFAULT_CACHE_SYNC_INTERVAL};
  // limit on no. of sync attempts
  const int nSyncRetries_{client::persistence::DEFAULT_CACHE_SYNC_RETRIES};
  // Sync try count across executor tasks
  int nSyncTries_{0};
  std::chrono::steady_clock::time_point lastExecutorScheduleTime_;

  // persistence layer
  // we use a shared pointer since the syncer thread might be operating on
  // it when the client decides to set a new one
  std::shared_ptr<CachePersistence> persistence_;
  // for locking access to persistence set/get
  MutexT persistenceLock_;

  // thread for periodic sync
  std::thread syncer_;

  // executor for periodic sync.
  std::shared_ptr<folly::Executor> executor_;

  // Semaphore to synchronize persistence loading and operations on the cache.
  folly::SaturatingSemaphore<true /* MayBlock */> persistenceLoadedSemaphore_;

  // Whether the persistence will be loaded inline.
  const bool inlinePersistenceLoading_;
};

} // namespace wangle

#include <wangle/client/persistence/LRUPersistentCache-inl.h>
