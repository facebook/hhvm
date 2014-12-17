/*
 * Copyright (c) 2014 Tim Starling
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef incl_HPHP_UTIL_SCALABLE_CACHE_H
#define incl_HPHP_UTIL_SCALABLE_CACHE_H

#include "hphp/util/concurrent-lru-cache.h"
#include "hphp/util/lru-cache-key.h"
#include <limits>
#include <memory>

namespace HPHP {

/**
 * ConcurrentScalableCache is a thread-safe sharded hashtable with limited
 * size. When it is full, it evicts a rough approximation to the least recently
 * used item.
 *
 * The find() operation fills a ConstAccessor object, which is a smart pointer
 * similar to TBB's const_accessor. After eviction, destruction of the value is
 * deferred until all ConstAccessor objects are destroyed.
 *
 * Since the hash value of each key is requested multiple times, you should use
 * a key with a memoized hash function. LRUCacheKey is provided for
 * this purpose.
 */
template <class TKey, class TValue, class THash = tbb::tbb_hash_compare<TKey>>
struct ConcurrentScalableCache {
  using Shard = ConcurrentLRUCache<TKey, TValue, THash>;
  typedef typename Shard::ConstAccessor ConstAccessor;

  /**
   * Constructor
   *   - maxSize: the maximum number of items in the container
   *   - numShards: the number of child containers. If this is zero, the
   *     "hardware concurrency" will be used (typically the logical processor
   *     count).
   */
  explicit ConcurrentScalableCache(size_t maxSize, size_t numShards = 0);

  ConcurrentScalableCache(const ConcurrentScalableCache&) = delete;
  ConcurrentScalableCache& operator=(const ConcurrentScalableCache&) = delete;

  /**
   * Find a value by key, and return it by filling the ConstAccessor, which
   * can be default-constructed. Returns true if the element was found, false
   * otherwise. Updates the eviction list, making the element the
   * most-recently used.
   */
  bool find(ConstAccessor& ac, const TKey& key);

  /**
   * Insert a value into the container. Both the key and value will be copied.
   * The new element will put into the eviction list as the most-recently
   * used.
   *
   * If there was already an element in the container with the same key, it
   * will not be updated, and false will be returned. Otherwise, true will be
   * returned.
   */
  bool insert(const TKey& key, const TValue& value);

  /**
   * Clear the container. NOT THREAD SAFE -- do not use while other threads
   * are accessing the container.
   */
  void clear();

  /**
   * Get a snapshot of the keys in the container by copying them into the
   * supplied vector. This will block inserts and prevent LRU updates while it
   * completes. The keys will be inserted in a random order.
   */
  void snapshotKeys(std::vector<TKey>& keys);

  /**
   * Get the approximate size of the container. May be slightly too low when
   * insertion is in progress.
   */
  size_t size() const;

private:
  /**
   * Get the child container for a given key
   */
  Shard& getShard(const TKey& key);

  /**
   * The maximum number of elements in the container.
   */
  size_t m_maxSize;

  /**
   * The child containers
   */
  size_t m_numShards;
  typedef std::shared_ptr<Shard> ShardPtr;
  std::vector<ShardPtr> m_shards;
};

/**
 * A specialisation of ConcurrentScalableCache providing a cache with efficient
 * string keys.
 */
template <class TValue>
using ThreadSafeStringCache = ConcurrentScalableCache<
    LRUCacheKey, TValue, LRUCacheKey::HashCompare>;

template <class TKey, class TValue, class THash>
ConcurrentScalableCache<TKey, TValue, THash>::
ConcurrentScalableCache(size_t maxSize, size_t numShards)
  : m_maxSize(maxSize), m_numShards(numShards)
{
  if (m_numShards == 0) {
    m_numShards = std::thread::hardware_concurrency();
  }
  for (size_t i = 0; i < m_numShards; i++) {
    size_t s = maxSize / m_numShards;
    if (i == 0) {
      s += maxSize % m_numShards;
    }
    m_shards.emplace_back(std::make_shared<Shard>(s));
  }
}

template <class TKey, class TValue, class THash>
typename ConcurrentScalableCache<TKey, TValue, THash>::Shard&
ConcurrentScalableCache<TKey, TValue, THash>::
getShard(const TKey& key) {
  THash hashObj;
  constexpr int shift = std::numeric_limits<size_t>::digits - 16;
  size_t h = (hashObj.hash(key) >> shift) % m_numShards;
  return *m_shards.at(h);
}

template <class TKey, class TValue, class THash>
bool ConcurrentScalableCache<TKey, TValue, THash>::
find(ConstAccessor& ac, const TKey& key) {
  return getShard(key).find(ac, key);
}

template <class TKey, class TValue, class THash>
bool ConcurrentScalableCache<TKey, TValue, THash>::
insert(const TKey& key, const TValue& value) {
  return getShard(key).insert(key, value);
}

template <class TKey, class TValue, class THash>
void ConcurrentScalableCache<TKey, TValue, THash>::
clear() {
  for (size_t i = 0; i < m_numShards; i++) {
    m_shards[i]->clear();
  }
}

template <class TKey, class TValue, class THash>
void ConcurrentScalableCache<TKey, TValue, THash>::
snapshotKeys(std::vector<TKey>& keys) {
  for (size_t i = 0; i < m_numShards; i++) {
    m_shards[i]->snapshotKeys(keys);
  }
}

template <class TKey, class TValue, class THash>
size_t ConcurrentScalableCache<TKey, TValue, THash>::
size() const {
  size_t size = 0;
  for (size_t i = 0; i < m_numShards; i++) {
    size += m_shards[i]->size();
  }
  return size;
}

} // namespace HPHP

#endif
