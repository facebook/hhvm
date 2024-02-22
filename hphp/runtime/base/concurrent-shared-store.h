/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <iosfwd>

#include <folly/SharedMutex.h>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>

#include "hphp/util/either.h"
#include "hphp/util/smalllocks.h"

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/request-id.h"
#include "hphp/runtime/server/server-stats.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This is the in-APC representation of a value, in ConcurrentTableSharedStore.
 */
struct StoreValue {
  /*
   * Index into cache layer. Valid indices are >= 0 and never invalidated.
   */
  using HotCacheIdx = int32_t;

  StoreValue() = default;
  StoreValue(const StoreValue& o)
    : m_data{o.m_data.load(std::memory_order_acquire)}
    , expireTime{o.expireTime.load(std::memory_order_relaxed)}
    , dataSize{o.dataSize}
    , kind(o.kind)
    , bumpTTL{o.bumpTTL.load(std::memory_order_relaxed)}
    , c_time{o.c_time}
    , maxExpireTime{o.maxExpireTime.load(std::memory_order_relaxed)}
  {
    hotIndex.store(o.hotIndex.load(std::memory_order_relaxed),
                   std::memory_order_relaxed);
  }

  APCHandle* data() const {
    auto data = m_data.load(std::memory_order_acquire);
    assertx(data != nullptr);
    return data;
  }
  void setHandle(APCHandle* v) {
    kind = v->kind();
    m_data.store(v, std::memory_order_release);
  }
  APCKind getKind() const {
    assertx(data());
    assertx(data()->kind() == kind);
    return kind;
  }
  Variant toLocal(bool pure) const {
    return data()->toLocal(pure);
  }
  void set(APCHandle* v, int64_t expireTTL, int64_t maxTTL, int64_t bumpTTL);
  bool expired() const;
  uint32_t rawExpire() const {
    return expireTime.load(std::memory_order_acquire);
  }
  uint32_t queueExpire() const;

  /* Special invalid indices; used to classify cache lookup misses. */
  static constexpr HotCacheIdx kHotCacheUnknown = -1;
  static constexpr HotCacheIdx kHotCacheKnownIneligible = -2;

  // Mutable fields here are so that we can deserialize the object from disk
  // while holding a const pointer to the StoreValue, or remember a cache entry.

  /*
   * Each entry in APC is an APCHandle*
   *
   * Note also that 'expire' may not be safe to read even if data is
   * valid, due to non-atomicity of updates; use 'expired()'.
   *
   * Note: expiration, creation, and modification times are stored unsigned
   * in 32-bits as seconds since the Epoch to save cache-line space.
   * HHVM might get confused after 2106 :)
   */
  mutable std::atomic<APCHandle*> m_data;
  mutable std::atomic<uint32_t> expireTime{};
  int32_t dataSize{0};  // For file storage, negative means serialized object
  // Reference to any HotCache entry to be cleared if the value is treadmilled.
  mutable std::atomic<HotCacheIdx> hotIndex{kHotCacheUnknown};
  APCKind kind;
  mutable std::atomic<uint16_t> bumpTTL{0};
  mutable std::atomic<RequestId> expireRequestId{RequestId()};
  uint32_t c_time{0}; // Creation time; 0 for primed values
  mutable std::atomic<uint32_t> maxExpireTime{};
};

//////////////////////////////////////////////////////////////////////

/*
 * Hold info about an entry in APC.  Used as a temporary holder to expose
 * information about APC entries.
 */
struct EntryInfo {
  enum class Type {
    Unknown,
    Uncounted,
    UncountedString,
    APCString,
    APCArray,
    APCObject,
    SerializedObject,
    UncountedVec,
    UncountedDict,
    UncountedKeyset,
    SerializedVec,
    SerializedDict,
    SerializedKeyset,
    APCVec,
    APCDict,
    APCKeyset,
    APCRFunc,
    APCRClsMeth,
  };

  EntryInfo(const char* apckey,
            int32_t size,
            int64_t ttl,
            int64_t maxTTL,
            uint16_t bumpTTL,
            Type type,
            int64_t c_time,
            bool inHotCache)
    : key(apckey)
    , size(size)
    , ttl(ttl)
    , maxTTL(maxTTL)
    , bumpTTL(bumpTTL)
    , type(type)
    , c_time(c_time)
    , inHotCache(inHotCache)
  {}

  static Type getAPCType(const APCHandle* handle);

  std::string key;
  int32_t size;
  int64_t ttl;
  int64_t maxTTL;
  uint16_t bumpTTL;
  Type type;
  int64_t c_time;
  bool inHotCache;
};

//////////////////////////////////////////////////////////////////////

/*
 * This is the backing store for APC.  Maintains a key to value mapping, where
 * each value optionally has a time-to-live.
 *
 * After a value reaches its TTL, it's considered "expired", and most
 * operations on the table will act like it's not present (exceptions to this
 * should be documented below).  TTL function arguments to this module are
 * specified in seconds.
 */
struct ConcurrentTableSharedStore {
  ConcurrentTableSharedStore() = default;
  ConcurrentTableSharedStore(const ConcurrentTableSharedStore&) = delete;
  ConcurrentTableSharedStore&
    operator=(const ConcurrentTableSharedStore&) = delete;

  /*
   * Retrieve a value from the store.  Returns false if the value wasn't
   * contained in the table (or was expired).
   */
  bool get(const String& key, Variant& value, bool pure = false);

  /*
   * Add a value to the store if no (unexpired) value with this key is already
   * present.
   *
   * The requested ttl is limited by the ApcTTLLimit.
   *
   * Returns: true if the value was added, including if we've replaced an
   * expired value.
   */
  bool add(const String& key, const Variant& val, int64_t max_ttl, int64_t bump_ttl, bool pure = false);

  /*
   * Set the value for `key' to `val'.  If there was an existing value, it is
   * overwritten.
   *
   * The requested ttl is limited by the ApcTTLLimit.
   */
  void set(const String& key, const Variant& val, int64_t max_ttl, int64_t bump_ttl, bool pure = false);

  /*
   * Increment the value for the key `key' by step, iff it is present,
   * non-expired, and a numeric (KindOfInt64 or KindOfDouble) value.  Sets
   * `found' to true if the increment is performed, false otherwise.
   *
   * Returns: the new value for the key, or zero if the key was not found.
   */
  int64_t inc(const String& key, int64_t step, bool& found);

  /*
   * Attempt to atomically compare and swap the value for the key `key' from
   * `oldVal' to `newVal'.  If the key is present, non-expired, and has the
   * same value as `oldVal' (after conversions), set it to `newVal' and return
   * true.  Otherwise returns false.
   */
  bool cas(const String& key, int64_t oldVal, int64_t newVal);

  /*
   * Returns: true if this key exists in the store, and is not expired.
   */
  bool exists(const String& key);

  /*
   * Extend the expiration time to now + new_ttl if that is longer than
   * the current TTL (or to infinity if new_ttl is zero). Returns true if
   * it succeeds (the key exists in apc, is unexpired, and the expiration
   * was actually adjusted), false otherwise.
   */
  bool extendTTL(const String& key, int64_t new_ttl);

  /*
   * Returns the size of an entry if it exists. Sets `found` to true if it
   * exists and false if not.
   */
  int64_t size(const String& key, bool& found);

  /*
   * Remove the specified key, if it exists in the table.
   *
   * Returns: false if the key was not in the table, true if the key was in the
   * table **even if it was expired**.
   */
  bool eraseKey(const String& key);

  /*
   * Schedule deletion of expired entries.
   */
  void purgeExpired();
  void purgeDeferred(req::vector<StringData*>&&);

  /*
   * Clear the entire APC table.
   */
  bool clear();

  /*
   * Init
   */
  void init();

  /*
   * Debugging. Return information about the table.
   *
   * This is extremely expensive and not recommended for use outside of
   * development or debugging scenarios.
   */
  hphp_fast_string_set debugGetKeys();

  hphp_fast_string_map<HPHP::Optional<std::string>>
  debugGetEntries(const HPHP::Optional<std::string>& prefix,
                        HPHP::Optional<uint32_t> count);

  hphp_fast_string_map<EntryInfo> debugGetMetadata();

  /*
   * Debugging.  Dump information about the table to an output stream.
   *
   * This is extremely expensive and not recommended for use outside of
   * development scenarios.
   */
  enum class DumpMode {
    KeyOnly,
    KeyAndValue,
    KeyAndMeta
  };
  void dump(std::ostream& out, DumpMode dumpMode);
  /**
   * Dump up to count keys that begin with the given prefix. This is a subset
   * of what the dump `KeyAndValue` command would do.
   */
  void dumpPrefix(std::ostream& out, const std::string &prefix, uint32_t count);
  /**
   * Dump all non-primed keys that begin with one of the prefixes. Different
   * keys are separated by \n in the output stream. Keys containing \r or \n
   * will not be included.
   */
  void dumpKeysWithPrefixes(std::ostream& out,
                            const std::vector<std::string>& prefixes);
  /*
   * Dump random key and entry size to output stream
   */
  void dumpRandomKeys(std::ostream& out, uint32_t count);

  /*
   * Return 'count' randomly chosen entries, possibly with duplicates. If the
   * store is empty or this operation is not supported, returns an empty vector.
   */
  std::vector<EntryInfo> sampleEntriesInfo(uint32_t count);
  /*
   * Return a list of entries with consideration of memory usage. Roughly one
   * sample every 'bytes' of memory is used.
   */
  std::vector<std::tuple<EntryInfo, uint32_t>> sampleEntriesInfoBySize(uint32_t bytes);

  /*
   * Debugging.  Access information about all the entries in this table.
   *
   * This is extremely expensive and not recommended for use outside of
   * development scenarios.
   */
  std::vector<EntryInfo> getEntriesInfo();

private:
  // Fake a StringData as a char* with the high bit set.  charHashCompare below
  // will properly handle the value and reuse the hash value of the StringData.

  static char* tagStringData(StringData* s) {
    return reinterpret_cast<char*>(-reinterpret_cast<intptr_t>(s));
  }

  static StringData* getStringData(const char* s) {
    assertx(reinterpret_cast<intptr_t>(s) < 0);
    return reinterpret_cast<StringData*>(-reinterpret_cast<intptr_t>(s));
  }

  inline static bool isTaggedStringData(const char* s) {
    return reinterpret_cast<intptr_t>(s) < 0;
  }

private:
  struct CharHashCompare {
    bool equal(const char* s1, const char* s2) const {
      assertx(s1 && s2);
      // tbb implementation call equal with the second pointer being the
      // value in the table and thus not a StringData*. We are asserting
      // to make sure that is the case
      assertx(!isTaggedStringData(s2));
      if (isTaggedStringData(s1)) {
        s1 = getStringData(s1)->data();
      }
      return strcmp(s1, s2) == 0;
    }
    size_t hash(const char* s) const {
      assertx(s);
      return isTaggedStringData(s) ? getStringData(s)->hash() :
             StringData::hash(s, strlen(s));
    }
  };

private:
  template<typename Key, typename T, typename HashCompare>
  struct APCMap :
      tbb::concurrent_hash_map<Key,T,HashCompare,APCAllocator<char>> {
    // Append a random entry to 'entries'. The map must be non-empty and not
    // concurrently accessed. Returns false if this operation is not supported.
    bool getRandomAPCEntry(std::vector<EntryInfo>& entries);

    using node = typename tbb::concurrent_hash_map<Key,T,HashCompare,
                                                   APCAllocator<char>>::node;
    static_assert(sizeof(node) == 64, "Node should be cache-line sized");
  };

  using Map = APCMap<const char*,StoreValue,CharHashCompare>;
  using ExpirationPair = std::pair<intptr_t,time_t>;
  enum class ExpNil {};
  using ExpSet = tbb::concurrent_hash_map<intptr_t,ExpNil>;

  struct ExpirationCompare {
    bool operator()(const ExpirationPair& p1, const ExpirationPair& p2) const {
      return p1.second > p2.second;
    }
  };

private:
  bool checkExpire(const String& keyStr, Map::const_accessor& acc);
  bool eraseImpl(const char*, bool, ExpSet::accessor* expAcc);
  bool storeImpl(const String&, const Variant&, int64_t, int64_t, bool, bool);
  bool handlePromoteObj(const String&, APCHandle*, const Variant&, bool);
  void dumpKeyAndValue(std::ostream&);
  static EntryInfo makeEntryInfo(const char*, StoreValue*, int64_t curr_time);

private:
  Map m_vars;
  mutable folly::SharedMutex m_lock;
  /*
   * m_expQueue is a queue of keys to be expired. We purge items from
   * it every n (configurable) apc_stores.
   *
   * We can't (easily) remove items from m_expQueue, so if we add a
   * new entry every time an item is updated we could end up with a
   * lot of copies of the same key in the queue. To avoid that, we use
   * m_expSet, and only add an entry to the queue if there isn't one
   * already.
   *
   * In the current implementation, that means that if an element is
   * updated before it expires, when its entry in m_expQueue is
   * processed, it does nothing; except being put back into the queue
   * again with the new expiry time.
   *
   * This implementation uses the apc key's address as the key into
   * m_expSet, and as the identifier in ExpirationPair. We ensure that
   * the m_expSet entry is removed before the apc key is freed, and
   * guarantee that the key is valid as a char* if it exists in
   * m_expSet. If the entry subsequently pops off m_expQueue, we check
   * to see if its in m_expSet, and only try to purge it from apc if
   * its found.
   *
   * Note that its possible that the apc key was freed and
   * reallocated, and the entry in m_expQueue doesn't correspond to
   * the new key; but thats fine - if the key really has expired, it
   * will be purged, and if not, nothing will happen.
   */
  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  ExpSet m_expSet;
  std::atomic<time_t> m_lastPurgeTime{0};
};

//////////////////////////////////////////////////////////////////////

}
