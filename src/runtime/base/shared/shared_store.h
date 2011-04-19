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

#ifndef __HPHP_SHARED_STORE_H__
#define __HPHP_SHARED_STORE_H__

#include <runtime/base/shared/shared_store_base.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/server/server_stats.h>
#include <util/logger.h>
#include <util/lfu_table.h>
#include <runtime/base/shared/shared_store_stats.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define SHARED_STORE_LOCK_CNT  10000

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
// Constructor Parents

class SharedVariantFactory {
public:
  SharedVariantFactory() : m_locks(NULL) {
  }
  ~SharedVariantFactory() {
    if (m_locks) {
      delete [] m_locks;
    }
  }

  /**
   * If the value v already wraps a SharedVariant, we do not have to
   * regenerate it, but just bumping up the ref count.
   * However, SharedVariantLockedRefs cannot be safely reused, because
   * it may contain a lock associated with a different key.
   */
  inline SharedVariant* create(CStrRef key, CVarRef v) {
    SharedVariant *wrapped = v.getSharedVariant();
    if (wrapped) {
      wrapped->incRef();
      return wrapped;
    }
    return new SharedVariant(v, false);
  }
  inline SharedVariant* create(litstr str, int len, CStrRef v,
                           bool serialized) {
    SharedVariant *wrapped = v->getSharedVariant();
    if (wrapped) {
      wrapped->incRef();
      return wrapped;
    }
    return new SharedVariant(v, serialized);
  }
  inline SharedVariant* create(litstr str, int len, CVarRef v) {
    SharedVariant *wrapped = v.getSharedVariant();
    if (wrapped) {
      wrapped->incRef();
      return wrapped;
    }
    return new SharedVariant(v, false);
  }
protected:
  Mutex *m_locks;

  inline Mutex* getLock(const char *data, int len) {
    ssize_t hash = hash_string(data, len);
    return &m_locks[hash % SHARED_STORE_LOCK_CNT];
  }
  Mutex* getLock(CStrRef key) {
    return getLock(key.data(), key.size());
  }
};

///////////////////////////////////////////////////////////////////////////////
// Maps

class HashTableSharedStore : public LockedSharedStore,
                             private SharedVariantFactory {
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
                            private SharedVariantFactory {
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
}

#endif /* __HPHP_SHARED_STORE_H__ */
