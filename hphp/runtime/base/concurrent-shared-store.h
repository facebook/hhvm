/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>

#include "hphp/util/either.h"
#include "hphp/util/smalllocks.h"

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/server/server-stats.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This is the in-APC representation of a value, in ConcurrentTableSharedStore.
 */
struct StoreValue {
  StoreValue() = default;
  StoreValue(const StoreValue& o)
    : data{o.data}
    , expire{o.expire}
    , dataSize{o.dataSize}
    // Copy everything except the lock
  {}

  void set(APCHandle* v, int64_t ttl);
  bool expired() const;

  int32_t getSerializedSize() const {
    assert(data.right() != nullptr);
    return abs(dataSize);
  }

  bool isSerializedObj() const {
    assert(data.right() != nullptr);
    return dataSize < 0;
  }

  // Mutable fields here are so that we can deserialize the object from disk
  // while holding a const pointer to the StoreValue.

  /*
   * Each entry in APC is either an APCHandle or a pointer to serialized prime
   * data.  All primed keys have an expiration time of zero, but make use of a
   * lock during their initial file-data-to-APCHandle conversion, so these two
   * fields are unioned.
   *
   * Note: expiration times are stored in 32-bits as seconds since the Epoch.
   * HHVM might get confused after 2106 :)
   */
  mutable Either<APCHandle*,char*,either_policy::high_bit> data;
  union { uint32_t expire; mutable SmallLock lock; };
  int32_t dataSize{0};  // For file storage, negative means serialized object
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
  };

  EntryInfo(const char* apckey,
            bool inMem,
            int32_t size,
            int64_t ttl,
            Type type)
    : key(apckey)
    , inMem(inMem)
    , size(size)
    , ttl(ttl)
    , type(type)
  {}

  static Type getAPCType(const APCHandle* handle);

  std::string key;
  bool inMem;
  int32_t size;
  int64_t ttl;
  Type type;
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
    KeyValuePair() : value(nullptr), sAddr(nullptr) {}
    const char* key;
    APCHandle* value;
    char* sAddr;
    int32_t sSize;
    int len;
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
   * Remove the specified key, if it exists in the table.
   *
   * Returns: false if the key was not in the table, true if the key was in the
   * table **even if it was expired**.
   */
  bool erase(const String& key);

  /*
   * Clear the entire APC table.
   */
  bool clear();

  /*
   * The API for priming APC.  Not yet documented.
   */
  void prime(const std::vector<KeyValuePair>& vars);
  bool constructPrime(const String& v, KeyValuePair& item, bool serialized);
  bool constructPrime(const Variant& v, KeyValuePair& item);
  void primeDone();

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

  /*
   * Dump random key and entry size to output stream
   */
  void dumpRandomKeys(std::ostream &out, uint32_t count);

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
    assert(reinterpret_cast<intptr_t>(s) < 0);
    return reinterpret_cast<StringData*>(-reinterpret_cast<intptr_t>(s));
  }

  inline static bool isTaggedStringData(const char* s) {
    return reinterpret_cast<intptr_t>(s) < 0;
  }

private:
  struct CharHashCompare {
    bool equal(const char* s1, const char* s2) const {
      assert(s1 && s2);
      // tbb implementation call equal with the second pointer being the
      // value in the table and thus not a StringData*. We are asserting
      // to make sure that is the case
      assert(!isTaggedStringData(s2));
      if (isTaggedStringData(s1)) {
        s1 = getStringData(s1)->data();
      }
      return strcmp(s1, s2) == 0;
    }
    size_t hash(const char* s) const {
      assert(s);
      return isTaggedStringData(s) ? getStringData(s)->hash() : hash_string(s);
    }
  };

private:
  template<typename Key, typename T, typename HashCompare>
  class APCMap : public tbb::concurrent_hash_map<Key,T,HashCompare> {
  public:
    void getRandomAPCEntry(std::ostream &out);
  };

  using Map = APCMap<const char*,StoreValue,CharHashCompare>;
  using ExpirationPair = std::pair<const char*,time_t>;
  using ExpMap = tbb::concurrent_hash_map<const char*,int,CharHashCompare>;

  struct ExpirationCompare {
    bool operator()(const ExpirationPair& p1, const ExpirationPair& p2) const {
      return p1.second > p2.second;
    }
  };

private:
  bool eraseImpl(const String&, bool, int64_t);
  bool storeImpl(const String&, const Variant&, int64_t, bool, bool);
  void eraseAcc(Map::accessor&);
  void purgeExpired();
  void addToExpirationQueue(const char*, int64_t);
  bool handlePromoteObj(const String&, APCHandle*, const Variant&);
  APCHandle* unserialize(const String&, StoreValue*);
  void dumpKeyAndValue(std::ostream&);

private:
  Map m_vars;
  ReadWriteMutex m_lock;
  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  ExpMap m_expMap;
  std::atomic<uint64_t> m_purgeCounter{0};
};

//////////////////////////////////////////////////////////////////////

}

#endif
