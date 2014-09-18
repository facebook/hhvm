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

#define TBB_PREVIEW_CONCURRENT_PRIORITY_QUEUE 1

#include <atomic>
#include <utility>
#include <vector>
#include <string>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>

#include "hphp/util/either.h"
#include "hphp/util/smalllocks.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/apc-stats.h"
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
    , expiry{o.expiry}
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
   * data.  The meaning of the following fields partially depends on which mode
   * the StoreValue is in.
   */
  mutable Either<APCHandle*,char*> data;
  int64_t expiry{0};
  int32_t dataSize{0};  // For file storage, negative means serialized object
  mutable SmallLock lock;
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

struct ConcurrentTableSharedStore {
  struct KeyValuePair {
    KeyValuePair() : value(nullptr), sAddr(nullptr) {}
    const char* key;
    int len;
    APCHandle* value;
    char* sAddr;
    int32_t sSize;

    bool inMem() const {
      return value != nullptr;
    }
  };

  static std::string GetSkeleton(const String& key);

  explicit ConcurrentTableSharedStore(int id)
    : m_id(id)
    , m_purgeCounter(0)
  {}

  ConcurrentTableSharedStore(const ConcurrentTableSharedStore&) = delete;
  ConcurrentTableSharedStore&
    operator=(const ConcurrentTableSharedStore&) = delete;

  int size() const { return m_vars.size(); }
  bool get(const String& key, Variant &value);
  bool store(const String& key, const Variant& val, int64_t ttl,
                     bool overwrite = true, bool limit_ttl = true);
  int64_t inc(const String& key, int64_t step, bool &found);
  bool cas(const String& key, int64_t old, int64_t val);
  bool exists(const String& key);
  bool erase(const String& key, bool expired = false);
  bool clear();

  void prime(const std::vector<KeyValuePair> &vars);
  bool constructPrime(const String& v, KeyValuePair& item, bool serialized);
  bool constructPrime(const Variant& v, KeyValuePair& item);
  void primeDone();

  // This functionality is for debugging and should not be called regularly
  enum class DumpMode {
    keyOnly=0,
    keyAndValue=1,
    keyAndMeta=2
  };
  void dump(std::ostream& out, DumpMode dumpMode, int waitSeconds);
  void getEntriesInfo(std::vector<EntryInfo>& entries);

private:

  // Fake a StringData as a char* with the high bit set.
  // charHashCompare below will properly handle the value and reuse the
  // hash value of the StringData

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
  typedef tbb::concurrent_hash_map<const char*, StoreValue, CharHashCompare>
    Map;
  typedef std::pair<const char*, time_t> ExpirationPair;
  typedef tbb::concurrent_hash_map<const char*, int, CharHashCompare>
    ExpMap;

  struct ExpirationCompare {
    bool operator()(const ExpirationPair& p1, const ExpirationPair& p2) {
      return p1.second > p2.second;
    }
  };

private:
  APCHandle* construct(const Variant& v, size_t& size) {
    return APCHandle::Create(v, size, false);
  }

  bool eraseImpl(const String& key, bool expired, int64_t oldestTime = 0);

  void eraseAcc(Map::accessor &acc) {
    const char *pkey = acc->first;
    APCStats::getAPCStats().removeKey(strlen(pkey));
    m_vars.erase(acc);
    free((void *)pkey);
  }

  // Should be called outside m_lock
  void purgeExpired();
  void addToExpirationQueue(const char* key, int64_t etime);

  bool handleUpdate(const String& key, APCHandle* svar);
  bool handlePromoteObj(
      const String& key, APCHandle* svar, const Variant& value);
  APCHandle* unserialize(const String& key, StoreValue* sval);

  // helpers for dumping APC
  static void dumpKeyOnly(std::ostream& out, std::vector<EntryInfo>& entries);
  static void dumpKeyAndMeta(
    std::ostream& out, std::vector<EntryInfo>& entries);
  // this call is outrageously expensive and hooked up to an admin command
  // to be used for rare debugging cases.
  // Do NOT use it for regular debugging or monitoring particularly on
  // production given it keeps the APC table locked for more than 30 seconds.
  // That is, the machine will not be able to respond to any request for more
  // than 30 seconds.
  // You have to be really desperate to dump few G of data to disk!!
  void dumpKeyAndValue(std::ostream& out);

private:
  int m_id;
  Map m_vars;
  // Read lock is acquired whenever using concurrent ops
  // Write lock is acquired for whole table operations
  ReadWriteMutex m_lock;
  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  ExpMap m_expMap;
  std::atomic<uint64_t> m_purgeCounter;
};

//////////////////////////////////////////////////////////////////////

}

#endif
