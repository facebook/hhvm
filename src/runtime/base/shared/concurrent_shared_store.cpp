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

#include <runtime/base/shared/concurrent_shared_store.h>
#include <runtime/base/variable_serializer.h>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void ConcurrentTableSharedStore::count(int &reachable, int &expired,
                                       int &persistent) {
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

void ConcurrentTableSharedStore::clear() {
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
bool ConcurrentTableSharedStore::eraseImpl(CStrRef key, bool expired) {
  if (key.isNull()) return false;
  ReadLock l(m_lock);
  Map::accessor acc;
  if (m_vars.find(acc, key.data())) {
    if (expired && !acc->second.expired()) {
      return false;
    }
    if (RuntimeOption::EnableAPCSizeStats) {
      SharedStoreStats::removeDirect(key.size(), acc->second.size);
      if (RuntimeOption::EnableAPCSizeGroup) {
        SharedStoreStats::onDelete(key.get(), acc->second.var, false);
      }
    }
    eraseAcc(acc);
    return true;
  }
  return false;
}

// Should be called outside m_lock
void ConcurrentTableSharedStore::purgeExpired() {
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

bool ConcurrentTableSharedStore::get(CStrRef key, Variant &value) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
  bool statsFetch = RuntimeOption::EnableAPCSizeStats &&
                    RuntimeOption::EnableAPCFetchStats;
  const StoreValue *val;
  SharedVariant *svar = NULL;
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
        svar = val->var;
        if (RuntimeOption::ApcAllowObj) {
          // Hold ref here
          svar->incRef();
        }
        value = svar->toLocal();
        if (statsFetch) {
          SharedStoreStats::onGet(key.get(), svar);
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

  if (RuntimeOption::ApcAllowObj)  {
    bool statsDetail = RuntimeOption::EnableAPCSizeStats &&
                       RuntimeOption::EnableAPCSizeGroup;
    SharedVariant *converted = svar->convertObj(value);
    if (converted) {
      Map::accessor acc;
      m_vars.find(acc, key.data()); // start a write lock
      StoreValue *sval = &acc->second;
      SharedVariant *sv = sval->var;
      // sv may not be same as svar here because some other thread may have
      // updated it already, check before updating
      if (!sv->isUnserializedObj()) {
        if (statsDetail) {
          SharedStoreStats::onDelete(key.get(), sv, true);
        }
        sval->var = converted;
        sv->decRef();
        if (RuntimeOption::EnableAPCSizeStats) {
          int32 newSize = converted->getSpaceUsage();
          SharedStoreStats::updateDirect(sval->size, newSize);
          sval->size = newSize;
        }
        if (statsDetail) {
          int64 ttl = sval->expiry ? sval->expiry - time(NULL) : 0;
          SharedStoreStats::onStore(key.get(), converted, ttl, false);
        }
      } else {
        converted->decRef();
      }
    }
    // release the extra ref
    svar->decRef();
  }
  return true;
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


bool ConcurrentTableSharedStore::exists(CStrRef key) {
 bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
 bool statsFetch = RuntimeOption::EnableAPCSizeStats &&
                   RuntimeOption::EnableAPCFetchStats;
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
       // No need toLocal() here, avoiding the copy
       if (statsFetch) {
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

static bool check_skip(const char *key) {
  for (unsigned int i = 0; i < RuntimeOption::APCSizeSkipPrefix.size(); ++i) {
    const char *prefix = RuntimeOption::APCSizeSkipPrefix[i].c_str();
    int len = RuntimeOption::APCSizeSkipPrefix[i].size();
    if (memcmp(key, prefix, len) == 0) {
      // Skip the size calculation.
      return true;
    }
  }
  return false;
}

bool ConcurrentTableSharedStore::store(CStrRef key, CVarRef val, int64 ttl,
                                       bool overwrite /* = true */) {
  bool stats = RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats;
  bool statsDetail = RuntimeOption::EnableAPCSizeStats &&
                     RuntimeOption::EnableAPCSizeGroup;
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
        if (statsDetail) {
          SharedStoreStats::onDelete(key.get(), sval->var, true);
        }
        sval->var->decRef();
        if (RuntimeOption::EnableAPCSizeStats && !check_skip(key.data())) {
          int32 size = var->getSpaceUsage();
          SharedStoreStats::updateDirect(sval->size, size);
          sval->size = size;
        }
      } else {
        var->decRef();
        return false;
      }
    } else {
      if (RuntimeOption::EnableAPCSizeStats) {
        int32 size = var->getSpaceUsage();
        SharedStoreStats::addDirect(key.size(), size);
        sval->size = size;
      }
    }
    sval->set(var, ttl);
    expiry = sval->expiry;
    if (statsDetail) {
      SharedStoreStats::onStore(key.get(), var, ttl, false);
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
      int32 size = item.value->getSpaceUsage();
      SharedStoreStats::addDirect(strlen(copy), size);
      acc->second.size = size;
      if (RuntimeOption::EnableAPCSizeGroup) {
        StringData sd(copy);
        SharedStoreStats::onStore(&sd, item.value, 0, true);
      }
    }
  }
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

///////////////////////////////////////////////////////////////////////////////
// debugging support

void ConcurrentTableSharedStore::dump(std::ostream & out) {
  int i = 0;
  ReadLock l(m_lock);
  out << "Total " << m_vars.size() << endl;
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
       ++iter, ++i) {
    const char *key = iter->first;
    const StoreValue &val = iter->second;
    if (!val.expired()) {
      VariableSerializer vs(VariableSerializer::Serialize);
      out << i << " #### " << key << " #### ";
      Variant value = val.var->toLocal();
      try {
        String valS(vs.serialize(value, true));
        out << valS->toCPPString();
      } catch (const Exception &e) {
        out << "Exception: " << e.what();
      }
      out << endl;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
