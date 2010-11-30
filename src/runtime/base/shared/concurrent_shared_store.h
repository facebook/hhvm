/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/shared/thread_shared_variant.h>
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
  virtual void count(int &reachable, int &expired, int &persistent) {
    reachable = expired = persistent = 0;
    int now = time(NULL);
    WriteLock l(m_lock);
    for (Map::const_iterator iter = m_vars.begin();
         iter != m_vars.end(); ++iter) {
      reachable += iter->second.var->countReachable();

      int64 expiration = iter->second.expiry;
      if (expiration == 0) {
        persistent++;
      } else if (expiration <= now) {
        expired++;
      }
    }
  }
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
    return ThreadSharedVariant::Create(v, serialized);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return ThreadSharedVariant::Create(v, false);
  }

protected:
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    return ThreadSharedVariant::Create(v, false);
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

  virtual void clear() {
    WriteLock l(m_lock);
    if (RuntimeOption::EnableAPCSizeStats) {
      SharedStoreStats::onClear();
    }
    for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
         ++iter) {
      iter->second.var->decRef();
      free((void *)iter->first);
    }
    m_vars.clear();
  }

  /**
   * The Map::accessor here establishes a write lock, which means that other
   * threads, protected by read locks through Map::const_accessor, will not
   * read erased values from APC.
   * The ReadLock here is to sync with clear(), which only has a WriteLock,
   * not a specific accessor.
   */
  virtual bool eraseImpl(CStrRef key, bool expired) {
    if (key.isNull()) return false;
    ReadLock l(m_lock);
    Map::accessor acc;
    if (m_vars.find(acc, key.data())) {
      if (expired && !acc->second.expired()) {
        return false;
      }
      if (RuntimeOption::EnableAPCSizeStats) {
        SharedStoreStats::onDelete(key.get(), acc->second.var, false);
      }
      eraseAcc(acc);
      return true;
    }
    return false;
  }

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
  void purgeExpired() {
    if ((atomic_add(m_purgeCounter, (uint64)1) %
         RuntimeOption::ApcPurgeFrequency) != 0) return;
    time_t now = time(NULL);
    {
      // Check if there's work to do
      ReadLock lock(m_expirationQueueLock);
      if (m_expirationQueue.empty() || m_expirationQueue.top().second > now) {
        // No work
        return;
      }
    }
    // Purge items n at a time. The only operation under the write lock is
    // the pop
#define PURGE_RATE 256
    const char* s[PURGE_RATE];
    while (true) {
      int i;
      {
        WriteLock lock(m_expirationQueueLock);
        const ExpirationPair *p = NULL;
        for (i = 0; i < PURGE_RATE && !m_expirationQueue.empty() &&
               (p = &m_expirationQueue.top())->second < now;
             ++i, m_expirationQueue.pop()) {
          s[i] = p->first;
        }
      }
      for (int j = 0; j < i; ++j) {
        eraseImpl(s[j], true);
        free((void *)s[j]);
      }
      if (i < PURGE_RATE) {
        // No work left
        break;
      }
    }
  }

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
