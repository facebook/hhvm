/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_CONCURRENT_SHARED_STORE_H__
#define __HPHP_CONCURRENT_SHARED_STORE_H__

#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/shared/shared_variant.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/server/server_stats.h>
#include <tbb/concurrent_hash_map.h>
#include <queue>
#include <runtime/base/shared/shared_store_stats.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ConcurrentThreadSharedStore

class ConcurrentTableSharedStore : public SharedStore {
public:
  ConcurrentTableSharedStore(int id) : SharedStore(id), m_purgeCounter(0) {}

  virtual int size() {
    return m_vars.size();
  }
  virtual void count(int &reachable, int &expired, int &persistent);
  virtual bool get(CStrRef key, Variant &value);
  virtual bool store(CStrRef key, CVarRef val, int64 ttl,
                     bool overwrite = true);
  virtual int64 inc(CStrRef key, int64 step, bool &found);
  virtual bool cas(CStrRef key, int64 old, int64 val);
  virtual bool exists(CStrRef key);

  virtual void prime(const std::vector<SharedStore::KeyValuePair> &vars);

  // debug support
  virtual void dump(std::ostream & out);

  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) {
    return SharedVariant::Create(v, serialized);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return SharedVariant::Create(v, false);
  }

protected:
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    return SharedVariant::Create(v, false);
  }

  struct charHashCompare {
    bool equal(const char *s1, const char *s2) const {
      ASSERT(s1 && s2);
      return strcmp(s1, s2) == 0;
    }
    size_t hash(const char *s) const {
      ASSERT(s);
      return hash_string(s);
    }
  };

  typedef tbb::concurrent_hash_map<const char*, StoreValue, charHashCompare>
    Map;

  virtual void clear();

  virtual bool eraseImpl(CStrRef key, bool expired);

  void eraseAcc(Map::accessor &acc) {
    acc->second.var->decRef();
    const char *pkey = acc->first;
    m_vars.erase(acc);
    free((void *)pkey);
  }
  void eraseAcc(Map::const_accessor &acc) {
    acc->second.var->decRef();
    const char *pkey = acc->first;
    m_vars.erase(acc);
    free((void *)pkey);
  }

  Map m_vars;
  // Read lock is acquired whenever using concurrent ops
  // Write lock is acquired for whole table operations
  ReadWriteMutex m_lock;

  typedef std::pair<const char*, time_t> ExpirationPair;
  class ExpirationCompare {
  public:
    bool operator()(const ExpirationPair &p1, const ExpirationPair &p2) {
      return p1.second > p2.second;
    }
  };

  std::priority_queue<ExpirationPair, std::vector<ExpirationPair>,
                      ExpirationCompare> m_expirationQueue;
  ReadWriteMutex m_expirationQueueLock;
  uint64 m_purgeCounter;

  // Should be called outside m_lock
  void purgeExpired();

  void addToExpirationQueue(const char* key, int64 etime) {
    const char *copy = strdup(key);
    ExpirationPair p(copy, etime);
    WriteLock lock(m_expirationQueueLock);
    m_expirationQueue.push(p);
  }
};

///////////////////////////////////////////////////////////////////////////////
}


#endif /* __HPHP_CONCURRENT_SHARED_STORE_H__ */
