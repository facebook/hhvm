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

#ifndef incl_HPHP_CONCURRENT_SHARED_STORE_H_
#define incl_HPHP_CONCURRENT_SHARED_STORE_H_

#include <atomic>
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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/apc/snapshot-loader.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/vm/treadmill.h"

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
  using HandleOrSerial = Either<APCHandle*,char*,either_policy::high_bit>;

  StoreValue() = default;
  StoreValue(const StoreValue& o)
    : tagged_data{o.data()}
    , expireTime{o.expireTime.load(std::memory_order_relaxed)}
    , dataSize{o.dataSize}
    , kind(o.kind)
    , readOnly(o.readOnly)
    , c_time{o.c_time}
    , mtime{o.mtime}
    // Copy everything except the lock
  {
    hotIndex.store(o.hotIndex.load(std::memory_order_relaxed),
                   std::memory_order_relaxed);
  }

  HandleOrSerial data() const {
    return tagged_data.load(std::memory_order_acquire);
  }
  void clearData() {
    tagged_data.store(nullptr, std::memory_order_release);
  }
  void setHandle(APCHandle* v) {
    kind = v->kind();
    tagged_data.store(v, std::memory_order_release);
  }
  APCKind getKind() const {
    assertx(data().left());
    assertx(data().left()->kind() == kind);
    return kind;
  }
  Variant toLocal() const {
    return data().left()->toLocal();
  }
  void set(APCHandle* v, int64_t ttl);
  bool expired() const;
  bool deferredExpire() const;
  uint32_t rawExpire() const {
    return expireTime.load(std::memory_order_acquire);
  }

  int32_t getSerializedSize() const {
    assertx(data().right() != nullptr);
    return abs(dataSize);
  }

  bool isSerializedObj() const {
    assertx(data().right() != nullptr);
    return dataSize < 0;
  }

  /* Special invalid indices; used to classify cache lookup misses. */
  static constexpr HotCacheIdx kHotCacheUnknown = -1;
  static constexpr HotCacheIdx kHotCacheKnownIneligible = -2;

  // Mutable fields here are so that we can deserialize the object from disk
  // while holding a const pointer to the StoreValue, or remember a cache entry.

  /*
   * Each entry in APC is either
   *  (a) an APCHandle* or,
   *  (b) a char* to serialized prime data; unserializes to (a) on first access.
   * All primed values have an expiration time of zero, but make use of a
   * lock during their initial (b) -> (a) conversion, so these two fields
   * are unioned. Readers must check for (a)/(b) using data.left/right and
   * acquire our lock for case (b). Writers must ensure any left -> right tag
   * update happens after all other modifictions to our StoreValue.
   *
   * Note also that 'expire' may not be safe to read even if data.left() is
   * valid, due to non-atomicity of updates; use 'expired()'.
   *
   * Note: expiration, creation, and modification times are stored unsigned
   * in 32-bits as seconds since the Epoch to save cache-line space.
   * HHVM might get confused after 2106 :)
   */
  mutable std::atomic<HandleOrSerial> tagged_data{HandleOrSerial()};
  union { mutable std::atomic<uint32_t> expireTime; mutable SmallLock lock; };
  int32_t dataSize{0};  // For file storage, negative means serialized object
  // Reference to any HotCache entry to be cleared if the value is treadmilled.
  mutable std::atomic<HotCacheIdx> hotIndex{kHotCacheUnknown};
  APCKind kind;  // Only valid if data is an APCHandle*.
  bool readOnly{false}; // Set for primed entries that will never change.
  char padding[2];  // Make APCMap nodes cache-line sized (it static_asserts).
  mutable std::atomic<int64_t> expireRequestIdx{Treadmill::kInvalidRequestIdx};
  uint32_t c_time{0}; // Creation time; 0 for primed values
  uint32_t mtime{0}; // Modification time
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
    UncountedArray,
    SerializedArray,
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
  };

  EntryInfo(const char* apckey,
            bool inMem,
            int32_t size,
            int64_t ttl,
            Type type,
            int64_t c_time,
            int64_t mtime)
    : key(apckey)
    , inMem(inMem)
    , size(size)
    , ttl(ttl)
    , type(type)
    , c_time(c_time)
    , mtime(mtime)
  {}

  static Type getAPCType(const APCHandle* handle);

  std::string key;
  bool inMem;
  int32_t size;
  int64_t ttl;
  Type type;
  int64_t c_time;
  int64_t mtime;
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
  struct KeyValuePair {
    KeyValuePair() : value(nullptr), sAddr(nullptr), readOnly(false) {}
    const char* key;
    APCHandle* value;
    char* sAddr;
    int32_t sSize;
    bool readOnly;
    bool inMem() const { return value != nullptr; }
  };

  ConcurrentTableSharedStore() = default;
  ConcurrentTableSharedStore(const ConcurrentTableSharedStore&) = delete;
  ConcurrentTableSharedStore&
    operator=(const ConcurrentTableSharedStore&) = delete;

  /*
   * Retrieve a value from the store.  Returns false if the value wasn't
   * contained in the table (or was expired).
   */
  bool get(const String& key, Variant& value);

  /*
   * Add a value to the store if no (unexpired) value with this key is already
   * present.
   *
   * The requested ttl is limited by the ApcTTLLimit.
   *
   * Returns: true if the value was added, including if we've replaced an
   * expired value.
   */
  bool add(const String& key, const Variant& val, int64_t ttl);

  /*
   * Set the value for `key' to `val'.  If there was an existing value, it is
   * overwritten.
   *
   * The requested ttl is limited by the ApcTTLLimit, unless we're overwriting
   * a primed key.
   */
  void set(const String& key, const Variant& val, int64_t ttl);

  /*
   * Set the value for `key' to `val', without any TTL, even if it wasn't
   * a primed key.
   */
  void setWithoutTTL(const String& key, const Variant& val);

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
   * Clear the entire APC table.
   */
  bool clear();

  /*
   * The API for priming APC.  Poorly documented.
   */
  void prime(std::vector<KeyValuePair>&& vars);
  bool constructPrime(const String& v, KeyValuePair& item, bool serialized);
  bool constructPrime(const Variant& v, KeyValuePair& item);
  void primeDone();
  // Returns false on failure (in particular, for the old file format).
  bool primeFromSnapshot(const char* filename);
  // Evict any file-backed APC values from OS page cache.
  void adviseOut();

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
  using ExpMap = tbb::concurrent_hash_map<intptr_t,int>;

  struct ExpirationCompare {
    bool operator()(const ExpirationPair& p1, const ExpirationPair& p2) const {
      return p1.second > p2.second;
    }
  };

private:
  bool eraseImpl(const char*, bool, int64_t, ExpMap::accessor* expAcc);
  bool storeImpl(const String&, const Variant&, int64_t, bool, bool);
  void purgeExpired();
  bool handlePromoteObj(const String&, APCHandle*, const Variant&);
  APCHandle* unserialize(const String&, StoreValue*);
  void dumpKeyAndValue(std::ostream&);
  static EntryInfo makeEntryInfo(const char*, StoreValue*, int64_t curr_time);

private:
  Map m_vars;
  folly::SharedMutex m_lock;
  /*
   * m_expQueue is a queue of keys to be expired. We purge items from
   * it every n (configurable) apc_stores.
   *
   * We can't (easily) remove items from m_expQueue, so if we add a
   * new entry every time an item is updated we could end up with a
   * lot of copies of the same key in the queue. To avoid that, we use
   * m_expMap, and only add an entry to the queue if there isn't one
   * already.
   *
   * In the current implementation, that means that if an element is
   * updated before it expires, when its entry in m_expQueue is
   * processed, it does nothing; and from then on, the item has no
   * entry in the queue. I think this is intentional, because items
   * that are updated frequently (or at all) are probably read
   * frequently; so it will be expired naturally. It also means that
   * we don't bother updating the queue every time for keys that are
   * updated frequently.
   *
   * This implementation uses the apc key's address as the key into
   * m_expMap, and as the identifier in ExpirationPair. We ensure that
   * the m_expMap entry is removed before the apc key is freed, and
   * guarantee that the key is valid as a char* if it exists in
   * m_expMap. If the entry subsequently pops off m_expQueue, we check
   * to see if its in m_expMap, and only try to purge it from apc if
   * its found.
   *
   * Note that its possible that the apc key was freed and
   * reallocated, and the entry in m_expQueue doesn't correspond to
   * the new key; but thats fine - if the key really has expired, it
   * will be purged, and if not, nothing will happen.
   */
  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  ExpMap m_expMap;
  std::atomic<uint64_t> m_purgeCounter{0};

  std::unique_ptr<SnapshotLoader> m_snapshotLoader;
};

//////////////////////////////////////////////////////////////////////

}

#endif
