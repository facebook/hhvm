/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/smalllocks.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/server/server-stats.h"
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>
#include "hphp/runtime/base/shared-store-stats.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StoreValue {
  StoreValue() : var(nullptr), sAddr(nullptr), expiry(0), size(0), sSize(0) {}
  StoreValue(const StoreValue& v) : var(v.var), sAddr(v.sAddr),
                                    expiry(v.expiry), size(v.size),
                                    sSize(v.sSize) {}
  void set(APCHandle *v, int64_t ttl);
  bool expired() const;

  // Mutable fields here are so that we can deserialize the object from disk
  // while holding a const pointer to the StoreValue. Mostly a hacky workaround
  // for how we use TBB
  mutable APCHandle *var;
  char *sAddr; // For file storage
  int64_t expiry;
  mutable int32_t size;
  int32_t sSize; // For file storage, negative means serailized object
  mutable SmallLock lock;

  bool inMem() const {
    return var != nullptr;
  }
  bool inFile() const {
    return sAddr != nullptr;
  }

  int32_t getSerializedSize() const {
    return abs(sSize);
  }
  bool isSerializedObj() const {
    return sSize < 0;
  }
};

struct ConcurrentTableSharedStore {
  struct KeyValuePair {
    KeyValuePair() : value(nullptr), sAddr(nullptr) {}
    litstr key;
    int len;
    APCHandle *value;
    char *sAddr;
    int32_t sSize;

    bool inMem() const {
      return value != nullptr;
    }
  };

  static std::string GetSkeleton(const String& key);

  explicit ConcurrentTableSharedStore(int id)
    : m_id(id)
    , m_lockingFlag(false)
    , m_purgeCounter(0)
  {}

  ConcurrentTableSharedStore(const ConcurrentTableSharedStore&) = delete;
  ConcurrentTableSharedStore&
    operator=(const ConcurrentTableSharedStore&) = delete;

  int size() const { return m_vars.size(); }
  bool get(const String& key, Variant &value);
  bool store(const String& key, CVarRef val, int64_t ttl,
                     bool overwrite = true, bool limit_ttl = true);
  int64_t inc(const String& key, int64_t step, bool &found);
  bool cas(const String& key, int64_t old, int64_t val);
  bool exists(const String& key);
  bool erase(const String& key, bool expired = false);
  bool clear();

  void prime(const std::vector<KeyValuePair> &vars);
  bool constructPrime(const String& v, KeyValuePair& item, bool serialized);
  bool constructPrime(CVarRef v, KeyValuePair& item);
  void primeDone();

  // debug support
  void dump(std::ostream & out, bool keyOnly, int waitSeconds);

private:

  // Fake a StringData as a char* with the high bit set.
  // charHashCompare below will properly handle the value and reuse the
  // hash value of the StringData

  inline static char* tagStringData(StringData* s) {
    return reinterpret_cast<char*>(-reinterpret_cast<intptr_t>(s));
  }

  inline static StringData* getStringData(const char* s) {
    assert(reinterpret_cast<intptr_t>(s) < 0);
    return reinterpret_cast<StringData*>(-reinterpret_cast<intptr_t>(s));
  }

  inline static bool isTaggedStringData(const char* s) {
    return reinterpret_cast<intptr_t>(s) < 0;
  }

  struct charHashCompare {
    bool equal(const char *s1, const char *s2) const {
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
    size_t hash(const char *s) const {
      assert(s);
      return isTaggedStringData(s) ? getStringData(s)->hash() : hash_string(s);
    }
  };

  typedef tbb::concurrent_hash_map<const char*, StoreValue, charHashCompare>
    Map;
  typedef std::pair<const char*, time_t> ExpirationPair;
  typedef tbb::concurrent_hash_map<const char*, int, charHashCompare>
    ExpMap;

  class ExpirationCompare {
  public:
    bool operator()(const ExpirationPair &p1, const ExpirationPair &p2) {
      return p1.second > p2.second;
    }
  };

private:
  APCHandle* construct(CVarRef v) {
    return APCHandle::Create(v, false);
  }

  bool eraseImpl(const String& key, bool expired);

  void eraseAcc(Map::accessor &acc) {
    const char *pkey = acc->first;
    m_vars.erase(acc);
    free((void *)pkey);
  }

  // Should be called outside m_lock
  void purgeExpired();

  void addToExpirationQueue(const char* key, int64_t etime);

  bool handleUpdate(const String& key, APCHandle* svar);
  bool handlePromoteObj(const String& key, APCHandle* svar, CVarRef valye);
  APCHandle* unserialize(const String& key, const StoreValue* sval);

private:
  int m_id;
  Map m_vars;
  // Read lock is acquired whenever using concurrent ops
  // Write lock is acquired for whole table operations
  ReadWriteMutex m_lock;
  bool m_lockingFlag; // flag to enable temporary locking

  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  ExpMap m_expMap;
  std::atomic<uint64_t> m_purgeCounter;
};

//////////////////////////////////////////////////////////////////////

}

#endif
