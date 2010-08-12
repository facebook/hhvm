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

#include <runtime/base/shared/shared_store.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/shared/process_shared_variant.h>
#include <runtime/base/shared/thread_shared_variant.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/server/server_stats.h>
#include <util/lfu_table.h>
#include <tbb/concurrent_hash_map.h>
#include <queue>
#include <runtime/base/shared/shared_store_stats.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

size_t SharedStore::s_lockCount = 10000;

///////////////////////////////////////////////////////////////////////////////
// LockedSharedStore
class LockedSharedStore : public SharedStore {
public:
  LockedSharedStore(int i) : SharedStore(i) {}
  virtual void clear();
  virtual bool get(CStrRef key, Variant &value);
  virtual bool store(CStrRef key, CVarRef val, int64 ttl,
                     bool overwrite = true);
  virtual int64 inc(CStrRef key, int64 step, bool &found);
  virtual bool cas(CStrRef key, int64 old, int64 val);
  virtual void prime(const std::vector<KeyValuePair> &vars);
protected:
  virtual bool find(CStrRef key, StoreValue *&v, bool &expired) = 0;
  virtual void set(CStrRef key, SharedVariant* v, int64 ttl) = 0;
  virtual bool eraseImpl(CStrRef key, bool expired);
  virtual bool eraseLockedImpl(CStrRef key, bool expired) = 0;
  virtual void lockMap() = 0;
  virtual void readLockMap() = 0;
  virtual void unlockMap() = 0;
  virtual void readUnlockMap() = 0;
  virtual void clearImpl() = 0;

};


///////////////////////////////////////////////////////////////////////////////
// ProcessSharedStore

class ProcessSharedStore : public LockedSharedStore {
public:
  ProcessSharedStore(int id) : LockedSharedStore(id) {
    if (!s_initialized) {
      Lock lock(s_mutex);
      if (!s_initialized) {
        SharedMemoryManager::Init(RuntimeOption::ApcSharedMemorySize *
                                  1024 * 1024, true);
        s_initialized = true;
      }
    }
    std::string sid = boost::lexical_cast<std::string>(id);
    std::string mapLockName = std::string("HPHP_MapLock") + sid;
    std::string valLocksName = std::string("HPHP_VariantLocks") + sid;
    std::string mapName = std::string("HPHP_APC") + sid;
    m_mapLock = SharedMemoryManager::GetSegment()->
      find_or_construct<boost::interprocess::interprocess_upgradable_mutex>
      (mapLockName.c_str())();
    m_locks = SharedMemoryManager::GetSegment()->
      find_or_construct<ProcessSharedVariantLock>(valLocksName.c_str())
      [s_lockCount]();
    m_vars = SharedMemory<SharedMap>::OpenOrCreate(mapName.c_str());
  }
  virtual bool find(CStrRef key, StoreValue *&val, bool &expired) {
    ASSERT(expired == false);
    SharedMap::iterator iter =
      m_vars->find(SharedMemoryString(key.data(), key.size()));
    if (iter != m_vars->end()) {
      val = &iter->second;
      if (iter->second.expired()) {
        expired = true;
        return false;
      }
      return true;
    } else {
      return false;
    }
  }
  virtual void set(CStrRef key, SharedVariant* v, int64 ttl) {
    (*m_vars)[SharedMemoryString(key.data(), key.size())].set(putVar(v), ttl);
  }
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    ProcessSharedVariantLock* lock = getLock(key);
    return SharedMemoryManager::GetSegment()->construct<ProcessSharedVariant>
      (boost::interprocess::anonymous_instance)(v, lock);
  }
  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) {
    if (serialized) {
      return construct(String(str, len, AttachLiteral), f_unserialize(v));
    }
    return construct(String(str, len, AttachLiteral), v);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return construct(String(str, len, AttachLiteral), v);
  }
  virtual void lockMap() {
    m_mapLock->lock();
  }
  virtual void readLockMap() {
    m_mapLock->lock_sharable();
  }
  virtual void unlockMap() {
    m_mapLock->unlock();
  }
  virtual void readUnlockMap() {
    m_mapLock->unlock_sharable();
  }

  virtual SharedVariant* putVar(SharedVariant* v) const {
    return (SharedVariant*)((size_t)v - (size_t)this);
  }

  virtual SharedVariant* getVar(SharedVariant* v) const {
    return (SharedVariant*)((size_t)v + (size_t)this);
  }

  virtual void clearImpl() {
    for (SharedMap::const_iterator iter = m_vars->begin();
         iter != m_vars->end(); ++iter) {
      ASSERT(getVar(iter->second.var));
      getVar(iter->second.var)->decRef();
    }
    m_vars->clear();
  }

  virtual bool eraseLockedImpl(CStrRef key, bool expired) {
    SharedMap::const_iterator iter =
      m_vars->find(SharedMemoryString(key.data(), key.size()));
    if (iter != m_vars->end()) {
      if (expired) {
        if (!iter->second.expired()) {
          return false;
        }
      }
      getVar(iter->second.var)->decRef();
      m_vars->erase(iter);
      return true;
    }
    return false;
  }

  virtual int size() {
    int ret = 0;
    lockMap();
    ret = m_vars->size();
    unlockMap();
    return ret;
  }

  virtual void count(int &reachable, int &expired, int &persistent) {
    reachable = expired = persistent = 0;
    int now = time(NULL);
    lockMap();
    for (SharedMap::const_iterator iter = m_vars->begin();
         iter != m_vars->end(); ++iter) {
      ASSERT(getVar(iter->second.var));
      reachable += getVar(iter->second.var)->countReachable();

      int64 expiration = iter->second.expiry;
      if (expiration == 0) {
        persistent++;
      } else if (expiration <= now) {
        expired++;
      }
    }
    unlockMap();
  }

private:
  typedef SharedMemoryMap<SharedMemoryString, StoreValue> SharedMap;
  ProcessSharedVariantLock* getLock(CStrRef key) {
    ssize_t hash = hash_string(key.data(), key.size());
    return &m_locks[hash % s_lockCount];
  }
  SharedMap *m_vars;
  boost::interprocess::interprocess_upgradable_mutex* m_mapLock;
  ProcessSharedVariantLock *m_locks;
  static Mutex s_mutex;
  static bool s_initialized;
};

Mutex ProcessSharedStore::s_mutex;
bool ProcessSharedStore::s_initialized = false;

///////////////////////////////////////////////////////////////////////////////
// Constructor Parents

class ThreadSharedVariantFactory {
public:
  ThreadSharedVariantFactory() : m_locks(NULL) {
    if (RuntimeOption::ApcUseLockedRefs) {
      m_locks = new Mutex[SharedStore::s_lockCount];
    }
  }
  ~ThreadSharedVariantFactory() {
    if (m_locks) {
      delete [] m_locks;
    }
  }

  /**
   * If the value v already wraps a ThreadSharedVariant, we do not have to
   * regenerate it, but just bumping up the ref count.
   * However, ThreadSharedVariantLockedRefs cannot be safely reused, because
   * it may contain a lock associated with a different key.
   */
  inline SharedVariant* create(CStrRef key, CVarRef v) {
    if (RuntimeOption::ApcUseLockedRefs) {
      return new ThreadSharedVariantLockedRefs(v, false, *getLock(key));
    } else {
      SharedVariant *wrapped = v.getSharedVariant();
      if (wrapped) {
        wrapped->incRef();
        return wrapped;
      }
      return new ThreadSharedVariant(v, false);
    }
  }
  inline SharedVariant* create(litstr str, int len, CStrRef v,
                           bool serialized) {
    if (RuntimeOption::ApcUseLockedRefs) {
      return new ThreadSharedVariantLockedRefs(v, serialized,
                                               *getLock(str, len));
    } else {
      SharedVariant *wrapped = v->getSharedVariant();
      if (wrapped) {
        wrapped->incRef();
        return wrapped;
      }
      return new ThreadSharedVariant(v, serialized);
    }
  }
  inline SharedVariant* create(litstr str, int len, CVarRef v) {
    if (RuntimeOption::ApcUseLockedRefs) {
      return new ThreadSharedVariantLockedRefs(v, false,
                                               *getLock(str, len));
    } else {
      SharedVariant *wrapped = v.getSharedVariant();
      if (wrapped) {
        wrapped->incRef();
        return wrapped;
      }
      return new ThreadSharedVariant(v, false);
    }
  }
protected:
  Mutex *m_locks;

  inline Mutex* getLock(const char *data, int len) {
    ssize_t hash = hash_string(data, len);
    return &m_locks[hash % SharedStore::s_lockCount];
  }
  Mutex* getLock(CStrRef key) {
    return getLock(key.data(), key.size());
  }
};

///////////////////////////////////////////////////////////////////////////////
// Maps

class HashTableSharedStore : public LockedSharedStore,
                             private ThreadSharedVariantFactory {
public:
  HashTableSharedStore(int i) : LockedSharedStore(i) {}
  ~HashTableSharedStore() {
    clear();
  }
  virtual bool find(CStrRef key, StoreValue *&val, bool &expired) {
    if (key.isNull()) return false;

    ASSERT(expired == false);
    StringMap::iterator iter = m_vars.find(key.get());
    if (iter == m_vars.end()) {
      return false;
    } else {
      val = &iter->second;
      if (val->expired()) {
        expired = true;
        return false;
      }
      return true;
    }
  }

  virtual void set(CStrRef key, SharedVariant* v, int64 ttl) {
    if (key.isNull()) return;

    ASSERT(m_vars.find(key.get()) == m_vars.end());
    StoreValue &val = m_vars[key.get()->copy(true)];
    val.set(v, ttl);
  }


  virtual void clearImpl() {
    std::vector<StringData*> keys;
    keys.reserve(m_vars.size());
    for (StringMap::iterator iter = m_vars.begin();
         iter != m_vars.end(); ++iter) {
      iter->second.var->decRef();
      keys.push_back(iter->first);
    }
    m_vars.clear();
    for (unsigned int i = 0; i < keys.size(); i++) {
      keys[i]->destruct();
    }
  }
  virtual bool eraseLockedImpl(CStrRef key, bool expired) {
    if (key.isNull()) return false;

    StringMap::iterator iter = m_vars.find(key.get());
    if (iter != m_vars.end()) {
      if (expired && !iter->second.expired()) {
        return false;
      }
      iter->second.var->decRef();
      StringData *pkey = iter->first;
      m_vars.erase(iter);
      pkey->destruct();
      return true;
    }
    return false;
  }
  virtual int size() {
    int ret = 0;
    lockMap();
    ret = m_vars.size();
    unlockMap();
    return ret;
  }
  virtual void count(int &reachable, int &expired, int &persistent) {
    reachable = expired = persistent = 0;
    int now = time(NULL);
    lockMap();
    for (StringMap::const_iterator iter = m_vars.begin();
         iter != m_vars.end(); ++iter) {
      reachable += iter->second.var->countReachable();

      int64 expiration = iter->second.expiry;
      if (expiration == 0) {
        persistent++;
      } else if (expiration <= now) {
        expired++;
      }
    }
    unlockMap();
  }
  virtual void lockMap() {
    m_mlock.acquireWrite();
  }
  virtual void readLockMap() {
    m_mlock.acquireRead();
  }
  virtual void unlockMap() {
    m_mlock.release();
  }
  virtual void readUnlockMap() {
    m_mlock.release();
  }
  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) {
    return create(str, len, v, serialized);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return create(str, len, v);
  }
protected:
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    return create(key, v);
  }
  ReadWriteMutex m_mlock;
  struct StringHash {
    size_t operator()(StringData *s) const {
      ASSERT(s);
      return hash_string(s->data(), s->size());
    }
  };

  struct StringEqual {
    bool operator()(StringData *s1, StringData *s2) const {
      ASSERT(s1 && s2);
      return s1->compare(s2) == 0;
    }
  };

  class StringMap :
    public hphp_hash_map<StringData*, StoreValue, StringHash, StringEqual> {
  public:
    typedef hphp_hash_map<StringData*, StoreValue, StringHash, StringEqual>
      baseType;
    ~StringMap() {
      for (baseType::iterator iter = baseType::begin();
           iter != baseType::end(); ++iter) {
        iter->first->destruct();
      }
    }
  };
  StringMap m_vars;
};

class RwLockHashTableSharedStore : public HashTableSharedStore {
public:
  RwLockHashTableSharedStore(int i): HashTableSharedStore(i) {}
  virtual void lockMap() {
    m_mlock.acquireWrite();
  }
  virtual void readLockMap() {
    m_mlock.acquireRead();
  }
  virtual void unlockMap() {
    m_mlock.release();
  }
  virtual void readUnlockMap() {
    m_mlock.release();
  }
protected:
  ReadWriteMutex m_mlock;
};

class MutexHashTableSharedStore : public HashTableSharedStore {
public:
  MutexHashTableSharedStore(int i): HashTableSharedStore(i) {}
  virtual void lockMap() {
    m_mlock.lock();
  }
  virtual void readLockMap() {
    m_mlock.lock();
  }
  virtual void unlockMap() {
    m_mlock.unlock();
  }
  virtual void readUnlockMap() {
    m_mlock.unlock();
  }
protected:
  Mutex m_mlock;
};

class LfuTableSharedStore : public SharedStore,
                            private ThreadSharedVariantFactory {
public:
  LfuTableSharedStore(int id, time_t maturity, size_t maxCap, int updatePeriod)
    : SharedStore(id), m_vars(maturity, maxCap, updatePeriod) {
  }

  void set(CStrRef key, SharedVariant* v, int64 ttl, bool immortal = false) {
    class SetUpdater : public Map::AtomicUpdater {
    public:
      SetUpdater(SharedVariant *v, int64 t) : var(v), ttl(t) {}
      bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
        ASSERT(newlyCreated);
        val.set(var, ttl);
        return false;
      }
    private:
      SharedVariant *var;
      int64 ttl;
    };
    if (key.isNull()) return;
    SetUpdater updater(v, ttl);
    m_vars.atomicUpdate(key.get()->copy(true), updater, true, immortal);
  }
  virtual void clear() {
    m_vars.clear();
  }
  virtual bool eraseImpl(CStrRef key, bool expired) {
    class EraseUpdater : public Map::AtomicUpdater {
    public:
      EraseUpdater(bool exp) : res(false), expired(exp) {}
      bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
        if (expired && !val.expired()) {
          return false;
        }
        res = true;
        return true;
      }
      bool res;
    private:
      bool expired;
    };

    if (key.isNull()) return false;
    EraseUpdater updater(expired);
    m_vars.atomicUpdate(key.get(), updater, false);
    return updater.res;
  }

  void prime(const std::vector<SharedStore::KeyValuePair> &vars);

  virtual int size() {
    return m_vars.size();
  }
  virtual void count(int &reachable, int &expired, int &persistent) {
    class CountBody : public Map::AtomicReader {
    public:
      CountBody(int &r, int &e, int &p)
        : now(time(NULL)), reachable(r), expired(e), persistent(p) {}
      void read(StringData* const &k, const StoreValue &val) {
        reachable += val.var->countReachable();
        int64 expiration = val.expiry;
        if (expiration == 0) {
          persistent++;
        } else if (expiration <= now) {
          expired++;
        }
      }
    private:
      time_t now;
      int &reachable;
      int &expired;
      int &persistent;
    };
    reachable = expired = persistent = 0;
    CountBody body(reachable, expired, persistent);
    m_vars.atomicForeach(body);
  }

  virtual bool get(CStrRef key, Variant &value);
  virtual bool store(CStrRef key, CVarRef val, int64 ttl,
                     bool overwrite = true);
  virtual int64 inc(CStrRef key, int64 step, bool &found);
  virtual bool cas(CStrRef key, int64 old, int64 val);
  virtual bool check() {
    return m_vars.check();
  }
  virtual std::string reportStats(int &reachable, int indent);
  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) {
    return create(str, len, v, serialized);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return create(str, len, v);
  }
protected:
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    return create(key, v);
  }
private:
  struct StringHash {
    size_t operator()(StringData *s) const {
      ASSERT(s);
      return hash_string(s->data(), s->size());
    }
  };

  struct StringEqual {
    bool operator()(StringData *s1, StringData *s2) const {
      ASSERT(s1 && s2);
      return s1->compare(s2) == 0;
    }
  };

  class NodeDestructor {
  public:
    void operator()(const StringData *k, StoreValue &val) {
      k->destruct();
      val.var->decRef();
    }
  };

  class StringMap :
    public LFUTable<StringData*, StoreValue, StringHash, StringEqual,
                    NodeDestructor> {
  public:
    StringMap(time_t maturity, size_t maxCap, int updatePeriod)
      : LFUTable<StringData*, StoreValue, StringHash, StringEqual,
                 NodeDestructor>
    (maturity, maxCap, updatePeriod) {}

    typedef LFUTable<StringData*, StoreValue, StringHash, StringEqual,
                     NodeDestructor> baseType;
  };
  typedef StringMap Map;
  Map m_vars;
};

///////////////////////////////////////////////////////////////////////////////
// ConcurrentThreadSharedStore

class ConcurrentTableSharedStore : public SharedStore,
                                   private ThreadSharedVariantFactory {
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
  virtual void prime(const std::vector<SharedStore::KeyValuePair> &vars);
  virtual SharedVariant* construct(litstr str, int len, CStrRef v,
                                   bool serialized) {
    return create(str, len, v, serialized);
  }
  virtual SharedVariant* construct(litstr str, int len, CVarRef v) {
    return create(str, len, v);
  }
protected:
  virtual SharedVariant* construct(CStrRef key, CVarRef v) {
    return create(key, v);
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
// SharedStore

SharedStore::SharedStore(int id) : m_id(id) {
}

SharedStore::~SharedStore() {
}

std::string SharedStore::GetSkeleton(CStrRef key) {
  std::string ret;
  const char *p = key.data();
  ret.reserve(key.size());
  bool added = false; // whether consecutive numbers are replaced by # yet
  for (int i = 0; i < key.size(); i++) {
    char ch = *p++;
    if (ch >= '0' && ch <= '9') {
      if (!added) {
        ret += '#';
        added = true;
      }
    } else {
      added = false;
      ret += ch;
    }
  }
  return ret;
}

void LockedSharedStore::clear() {
  lockMap();
  clearImpl();
  unlockMap();
}

bool LockedSharedStore::get(CStrRef key, Variant &value) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  readLockMap();
  StoreValue *val;
  bool expired = false;
  if (!find(key, val, expired)) {
    readUnlockMap();
    if (expired) {
      erase(key, true);
    }
    value = false;
    if (stats) {
      ServerStats::Log("apc.miss", 1);
    }
    return false;
  }
  value = getVar(val->var)->toLocal();
  readUnlockMap();
  if (stats) ServerStats::Log("apc.hit", 1);
  return true;
}


bool ConcurrentTableSharedStore::get(CStrRef key, Variant &value) {
 bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
 const StoreValue *val;
 ReadLock l(m_lock);
 bool expired = false;
 {
   Map::const_accessor acc;
   if (!m_vars.find(acc, key.data())) {
     if (stats) ServerStats::Log("apc.miss", 1);
     return false;
   } else {
     val = &acc->second;
     if (val->expired()) {
       // Because it only has a read lock on the data, deletion from
       // expiration has to happen after the lock is released
       expired = true;
     } else {
       value = val->var->toLocal();
       if (RuntimeOption::EnableAPCSizeStats &&
           RuntimeOption::EnableAPCSizeDetail &&
           RuntimeOption::EnableAPCFetchStats) {
         SharedStoreStats::onGet(key.get(), val->var);
       }
     }
   }
 }
 if (expired) {
   if (stats) {
     ServerStats::Log("apc.miss", 1);
   }
   eraseImpl(key, true);
   return false;
 }
 if (stats) {
   ServerStats::Log("apc.hit", 1);
 }
 return true;
}


bool LfuTableSharedStore::get(CStrRef key, Variant &value) {
  class GetReader : public Map::AtomicReader {
  public:
    GetReader(Variant &v) : expired(false), value(v) {}
    void read(StringData* const &k, const StoreValue &val) {
      value = val.var->toLocal();
      expired = val.expired();
    }
    bool expired;
    Variant &value;
  };
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
  GetReader reader(value);
  if (!m_vars.atomicRead(key.get(), reader) || reader.expired) {
    if (reader.expired) {
      erase(key, true);
    }
    value = false;
    if (stats) ServerStats::Log("apc.miss", 1);
    return false;
  }
  if (stats) ServerStats::Log("apc.hit", 1);
  return true;
}

bool LockedSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                              bool overwrite /* = true */) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  lockMap();
  StoreValue *sval;
  SharedVariant* var = construct(key, val);
  bool expired = false;
  bool added = false;
  if (find(key, sval, expired) || expired) {
    if (overwrite || expired) {
      getVar(sval->var)->decRef();
      sval->set(putVar(var), ttl);
      if (stats) ServerStats::Log("apc.update", 1);
      added = true;
    }
  } else {
    set(key, var, ttl);
    added = true;
    if (stats) {
      ServerStats::Log("apc.new", 1);
      if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCKeyStats) {
        string prefix = "apc.new.";
        prefix += GetSkeleton(key);
        ServerStats::Log(prefix, 1);
      }
    }
  }

  unlockMap();

  if (!added) var->decRef();

  return added;
}


bool ConcurrentTableSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                                       bool overwrite /* = true */) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;

  StoreValue *sval;
  SharedVariant* var = construct(key, val);
  ReadLock l(m_lock);

  const char *kcp = strdup(key.data());
  bool present;
  time_t expiry;
  {
    Map::accessor acc;
    present = !m_vars.insert(acc, kcp);
    sval = &acc->second;
    if (present) {
      free((void *)kcp);
      if (overwrite || sval->expired()) {
        if (RuntimeOption::EnableAPCSizeStats) {
          SharedStoreStats::onDelete(key.get(), sval->var, true);
        }
        sval->var->decRef();
      } else {
        var->decRef();
        return false;
      }
    }
    sval->set(var, ttl);
    expiry = sval->expiry;
    if (RuntimeOption::EnableAPCSizeStats) {
      SharedStoreStats::onStore(key.get(), var, ttl);
    }
  }
  if (RuntimeOption::ApcExpireOnSets) {
    if (ttl) {
      addToExpirationQueue(key.data(), expiry);
    }
    purgeExpired();
  }
  if (stats) {
    if (present) {
      ServerStats::Log("apc.update", 1);
    } else {
      ServerStats::Log("apc.new", 1);
      if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCKeyStats) {
        string prefix = "apc.new.";
        prefix += GetSkeleton(key);
        ServerStats::Log(prefix, 1);
      }
    }
  }

  return true;
}


bool LfuTableSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                                bool overwrite /* = true */) {
  class StoreUpdater : public Map::AtomicUpdater {
  public:
    StoreUpdater(int64 t, SharedVariant *v, CStrRef k, bool ovr)
      : added(false), overwrite(ovr), ttl(t), var(v), key(k),
        newkey(key.get()->copy(true)) {}
    bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
      bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
      if (!newlyCreated) {
        if (overwrite || val.expired()) {
          val.var->decRef();
          val.set(var, ttl);
          added = true;
          if (stats) ServerStats::Log("apc.update", 1);
        }
        newkey->destruct();
      } else {
        val.set(var, ttl);
        added = true;
        if (stats) {
          ServerStats::Log("apc.new", 1);
          if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCKeyStats) {
            string prefix = "apc.new.";
            prefix += GetSkeleton(key);
            ServerStats::Log(prefix, 1);
          }
        }
      }
      return false;
    }
    StringData *newKey() { return newkey; }
    bool added;
  private:
    bool overwrite;
    int64 ttl;
    SharedVariant *var;
    CStrRef key;
    StringData *newkey;
  };
  SharedVariant* var = construct(key, val);
  StoreUpdater updater(ttl, var, key, overwrite);
  m_vars.atomicUpdate(updater.newKey(), updater, true);
  if (!updater.added) {
    var->decRef();
  }
  return updater.added;
}

void LockedSharedStore::prime(const std::vector<KeyValuePair> &vars) {
  lockMap();
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const KeyValuePair &item = vars[i];
    set(String(item.key, item.len, CopyString), item.value, 0);
  }
  unlockMap();
}


void ConcurrentTableSharedStore::prime
(const std::vector<SharedStore::KeyValuePair> &vars) {
  ReadLock l(m_lock);
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const SharedStore::KeyValuePair &item = vars[i];
    Map::accessor acc;
    const char *copy = strdup(item.key);
    m_vars.insert(acc, copy);
    acc->second.set(item.value, 0);
    if (RuntimeOption::EnableAPCSizeStats &&
        RuntimeOption::APCSizeCountPrime) {
      StringData sd(copy);
      SharedStoreStats::onStore(&sd, item.value, 0);
    }
  }
}


void LfuTableSharedStore::prime
(const std::vector<SharedStore::KeyValuePair> &vars) {
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const SharedStore::KeyValuePair &item = vars[i];
    // Primed values are immortal
    set(String(item.key, item.len, CopyString), item.value, 0, true);
  }
}

bool SharedStore::erase(CStrRef key, bool expired /* = false */) {
  bool success = eraseImpl(key, expired);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log(success ? "apc.erased" : "apc.erase", 1);
  }
  return success;
}

bool LockedSharedStore::eraseImpl(CStrRef key, bool expired /* = false */) {
  lockMap();
  bool success = eraseLockedImpl(key, expired);
  unlockMap();
  return success;
}

int64 LockedSharedStore::inc(CStrRef key, int64 step, bool &found) {
  found = false;
  int64 ret = 0;
  lockMap();
  StoreValue *val;
  bool expired = false;
  if (find(key, val, expired)) {
    Variant v = getVar(val->var)->toLocal();
    ret = v.toInt64() + step;
    v = ret;
    SharedVariant *var = construct(key, v);
    getVar(val->var)->decRef();
    val->var = putVar(var);
    found = true;
  }
  unlockMap();
  if (expired) {
    erase(key, true);
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.inc", 1);
  }
  return ret;
}


int64 ConcurrentTableSharedStore::inc(CStrRef key, int64 step, bool &found) {
  found = false;
  int64 ret = 0;
  ReadLock l(m_lock);
  StoreValue *val;
  {
    Map::accessor acc;
    if (m_vars.find(acc, key.data())) {
      val = &acc->second;
      if (val->expired()) {
        eraseAcc(acc);
      } else {
        Variant v = val->var->toLocal();
        ret = v.toInt64() + step;
        v = ret;
        SharedVariant *var = construct(key, v);
        val->var->decRef();
        val->var = var;
        found = true;
      }
    }
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.inc", 1);
  }
  return ret;
}


int64 LfuTableSharedStore::inc(CStrRef key, int64 step, bool &found) {
  class IncUpdater : public Map::AtomicUpdater {
  public:
    IncUpdater(int64 s, bool &f, CStrRef k, LfuTableSharedStore *str)
      : ret(0), step(s), found(f), key(k), store(str) {}
    bool update(StringData* const &k, StoreValue &val, bool newlyCreated) {
      if (val.expired()) {
        return true;
      }

      Variant v = val.var->toLocal();
      ret = v.toInt64() + step;
      v = ret;
      SharedVariant *var = store->construct(key, v);
      val.var->decRef();
      val.var = var;
      found = true;
      return false;
    }
    int64 ret;
  private:
    int64 step;
    bool &found;
    CStrRef key;
    LfuTableSharedStore *store;
  };

  found = false;
  IncUpdater updater(step, found, key, this);
  m_vars.atomicUpdate(key.get(), updater, false);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.inc", 1);
  }
  return updater.ret;
}

bool LockedSharedStore::cas(CStrRef key, int64 old, int64 val) {
  bool success = false;
  lockMap();
  StoreValue *sval;
  bool expired = false;
  if (find(key, sval, expired)) {
    Variant v = getVar(sval->var)->toLocal();
    if (v.toInt64() == old) {
      v = val;
      SharedVariant *var = construct(key, v);
      getVar(sval->var)->decRef();
      sval->var = putVar(var);
      success = true;
    }
  }
  unlockMap();
  if (expired) {
    erase(key, true);
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.cas", 1);
  }
  return success;
}


bool ConcurrentTableSharedStore::cas(CStrRef key, int64 old, int64 val) {
  bool success = false;
  ReadLock l(m_lock);
  StoreValue *sval;
  {
    Map::accessor acc;
    if (m_vars.find(acc, key.data())) {
      sval = &acc->second;
      Variant v = sval->var->toLocal();
      if (v.toInt64() == old) {
        v = val;
        SharedVariant *var = construct(key, v);
        sval->var->decRef();
        sval->var = var;
        success = true;
      }
    }
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.cas", 1);
  }
  return success;
}


bool LfuTableSharedStore::cas(CStrRef key, int64 old, int64 val) {
  class CasUpdater : public Map::AtomicUpdater {
  public:
    CasUpdater(LfuTableSharedStore *s, CStrRef k, int64 o, int64 v)
      : success(false), store(s), key(k), old(o), val(v) {}
    bool update(StringData* const &k, StoreValue &sval, bool newlyCreated) {
      if (sval.expired()) {
        return true;
      }
      Variant v = sval.var->toLocal();
      if (v.toInt64() == old) {
        v = val;
        SharedVariant *var = store->construct(key, v);
        sval.var->decRef();
        sval.var = var;
        success = true;
      }
      return false;
    }
    bool success;
  private:
    LfuTableSharedStore *store;
    CStrRef key;
    int64 old;
    int64 val;
  };
  CasUpdater updater(this, key, old, val);
  m_vars.atomicUpdate(key.get(), updater, false);

  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log("apc.cas", 1);
  }
  return updater.success;
}

static std::string appendElement(int indent, const char *name, int value) {
  string ret;
  for (int i = 0; i < indent; i++) {
    ret += "  ";
  }
  ret += "<"; ret += name; ret += ">";
  ret += lexical_cast<string>(value);
  ret += "</"; ret += name; ret += ">\n";
  return ret;
}

std::string SharedStore::reportStats(int &reachable, int indent) {
  int expired, persistent;
  count(reachable, expired, persistent);

  string ret;
  ret += appendElement(indent, "Key", size());
  ret += appendElement(indent, "Persistent", persistent);
  ret += appendElement(indent, "Expiration", size() - persistent);
  ret += appendElement(indent, "Expired", expired);
  ret += appendElement(indent, "Reachable", reachable);
  return ret;
}


std::string LfuTableSharedStore::reportStats(int &reachable, int indent) {
  string ret = SharedStore::reportStats(reachable, indent);
  ret += appendElement(indent, "Immortal", m_vars.immortalCount());
  ret += appendElement(indent, "Maximum Capacity", m_vars.maximumCapacity());
  return ret;
}

void StoreValue::set(SharedVariant *v, int64 ttl) {
  var = v;
  expiry = ttl ? time(NULL) + ttl : 0;
}
bool StoreValue::expired() const {
  return expiry && time(NULL) >= expiry;
}

///////////////////////////////////////////////////////////////////////////////
// SharedStores

SharedStores s_apc_store;

SharedStores::SharedStores() {
}

void SharedStores::create() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    if (RuntimeOption::ApcUseSharedMemory) {
      m_stores[i] = new ProcessSharedStore(i);
    } else {
      switch (RuntimeOption::ApcTableType) {
      case RuntimeOption::ApcHashTable:
        switch (RuntimeOption::ApcTableLockType) {
        case RuntimeOption::ApcMutex:
          m_stores[i] = new MutexHashTableSharedStore(i);
          break;
        default:
          m_stores[i] = new RwLockHashTableSharedStore(i);
        }
        break;
      case RuntimeOption::ApcLfuTable:
        {
          time_t maturity = RuntimeOption::ApcKeyMaturityThreshold;
          size_t maxCap = RuntimeOption::ApcMaximumCapacity;
          int updatePeriod = RuntimeOption::ApcKeyFrequencyUpdatePeriod;

          if (i == SHARED_STORE_DNS_CACHE) {
            maturity = RuntimeOption::DnsCacheKeyMaturityThreshold;
            maxCap = RuntimeOption::DnsCacheMaximumCapacity;
            updatePeriod = RuntimeOption::DnsCacheKeyFrequencyUpdatePeriod;
          }
          m_stores[i] = new LfuTableSharedStore(i, maturity, maxCap,
                                                updatePeriod);
        }
        break;
      case RuntimeOption::ApcConcurrentTable:
        m_stores[i] = new ConcurrentTableSharedStore(i);
        break;
      default:
        ASSERT(false);
      }
    }
  }
}

SharedStores::~SharedStores() {
  clear();
}

void SharedStores::clear() {
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    delete m_stores[i];
  }
}

void SharedStores::reset() {
  clear();
  create();
}

std::string SharedStores::reportStats(int indent) {
  string ret;
  int totalReachable = 0;
#ifdef DEBUG_APC_LEAK
  LeakDetectable::BeginLeakChecking();
#endif
  for (int i = 0; i < MAX_SHARED_STORE; i++) {
    for (int j = 0; j < indent; j++) ret += "  ";
    ret += "<SharedStore>\n";

    ret += appendElement(indent + 1, "Index", i);
    int reachable = 0;
    ret += m_stores[i]->reportStats(reachable, indent + 1);
    totalReachable += reachable;

    for (int j = 0; j < indent; j++) ret += "  ";
    ret += "</SharedStore>\n";
  }
#ifdef DEBUG_APC_LEAK
  ret += appendElement(indent, "TotalVariantsAllocated",
                       SharedVariant::TotalAllocated);
  ret += appendElement(indent, "AliveVariants",
                       SharedVariant::TotalCount);
  ret += appendElement(indent, "LeakedVariantsByReach",
                       SharedVariant::TotalCount - totalReachable);

  string dumps;
  int leaked = LeakDetectable::EndLeakChecking(dumps, 10000);
  ret += appendElement(indent, "LeakedVariantsByLeakDetectable", leaked);
  ret += dumps;
#endif
  return ret;
}

void SharedStores::Create() {
  s_apc_store.create();
}

std::string SharedStores::ReportStats(int indent) {
  return s_apc_store.reportStats(indent);
}

///////////////////////////////////////////////////////////////////////////////
}
