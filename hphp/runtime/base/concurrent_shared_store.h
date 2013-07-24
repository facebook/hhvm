/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_CONCURRENT_SHARED_STORE_H_
#define incl_HPHP_CONCURRENT_SHARED_STORE_H_

#define TBB_PREVIEW_CONCURRENT_PRIORITY_QUEUE 1

#include "hphp/runtime/base/shared_store_base.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/shared_variant.h"
#include "hphp/runtime/base/runtime_option.h"
#include "hphp/runtime/base/type_conversions.h"
#include "hphp/runtime/base/builtin_functions.h"
#include "hphp/runtime/server/server_stats.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_priority_queue.h"
#include "hphp/runtime/base/shared_store_stats.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ConcurrentThreadSharedStore

class ConcurrentTableSharedStore : public SharedStore {
public:
  ConcurrentTableSharedStore(int id)
    : SharedStore(id), m_lockingFlag(false), m_purgeCounter(0) {}

  virtual int size() {
    return m_vars.size();
  }
  virtual bool get(CStrRef key, Variant &value);
  virtual bool store(CStrRef key, CVarRef val, int64_t ttl,
                     bool overwrite = true);
  virtual int64_t inc(CStrRef key, int64_t step, bool &found);
  virtual bool cas(CStrRef key, int64_t old, int64_t val);
  virtual bool exists(CStrRef key);

  virtual void prime(const std::vector<SharedStore::KeyValuePair> &vars);
  virtual bool constructPrime(CStrRef v, KeyValuePair& item,
                              bool serialized);
  virtual bool constructPrime(CVarRef v, KeyValuePair& item);
  virtual void primeDone();

  // debug support
  virtual void dump(std::ostream & out, bool keyOnly, int waitSeconds);

protected:
  virtual SharedVariant* construct(CVarRef v) {
    return SharedVariant::Create(v, false);
  }

  struct charHashCompare {
    bool equal(const char *s1, const char *s2) const {
      assert(s1 && s2);
      return strcmp(s1, s2) == 0;
    }
    size_t hash(const char *s) const {
      assert(s);
      return hash_string(s);
    }
  };

  typedef tbb::concurrent_hash_map<const char*, StoreValue, charHashCompare>
    Map;

  virtual bool clear();

  virtual bool eraseImpl(CStrRef key, bool expired);

  void eraseAcc(Map::accessor &acc) {
    const char *pkey = acc->first;
    m_vars.erase(acc);
    free((void *)pkey);
  }

  Map m_vars;
  // Read lock is acquired whenever using concurrent ops
  // Write lock is acquired for whole table operations
  ReadWriteMutex m_lock;
  bool m_lockingFlag; // flag to enable temporary locking

  typedef std::pair<const char*, time_t> ExpirationPair;
  class ExpirationCompare {
  public:
    bool operator()(const ExpirationPair &p1, const ExpirationPair &p2) {
      return p1.second > p2.second;
    }
  };

  tbb::concurrent_priority_queue<ExpirationPair,
                                 ExpirationCompare> m_expQueue;
  typedef tbb::concurrent_hash_map<const char*, int, charHashCompare>
    ExpMap;
  ExpMap m_expMap;

  std::atomic<uint64_t> m_purgeCounter;

  // Should be called outside m_lock
  void purgeExpired();

  void addToExpirationQueue(const char* key, int64_t etime);

  bool handleUpdate(CStrRef key, SharedVariant* svar);
  bool handlePromoteObj(CStrRef key, SharedVariant* svar, CVarRef valye);
private:
  SharedVariant* unserialize(CStrRef key, const StoreValue* sval);
};

///////////////////////////////////////////////////////////////////////////////
}


#endif /* incl_HPHP_CONCURRENT_SHARED_STORE_H_ */
