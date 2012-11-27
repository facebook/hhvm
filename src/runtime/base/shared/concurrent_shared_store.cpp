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
#include <runtime/ext/ext_apc.h>
#include <util/logger.h>
#include <util/timer.h>
#include <mutex>

using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// helpers

static void log_apc(const string& name) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCStats) {
    ServerStats::Log(name, 1);
  }
}

static void stats_on_get(StringData* key, const SharedVariant* svar) {
  if (RuntimeOption::EnableAPCSizeStats &&
      RuntimeOption::EnableAPCFetchStats) {
    SharedStoreStats::onGet(key, svar);
  }
}
static void stats_on_delete(StringData* key, const StoreValue* sval, bool exp) {
  if (RuntimeOption::EnableAPCSizeStats) {
    SharedStoreStats::removeDirect(key->size(), sval->size, exp);
    if (RuntimeOption::EnableAPCSizeGroup ||
        RuntimeOption::EnableAPCSizeDetail) {
      SharedStoreStats::onDelete(key, sval->var, false, sval->expiry == 0);
    }
  }
}
static bool check_key_prefix(const std::vector<std::string>& list,
                             const char *key) {
  for (unsigned int i = 0; i < list.size(); ++i) {
    const char *prefix = list[i].c_str();
    int len = list[i].size();
    if (memcmp(key, prefix, len) == 0) {
      // Skip the size calculation.
      return true;
    }
  }
  return false;
}
static bool check_skip(const char *key) {
  return check_key_prefix(RuntimeOption::APCSizeSkipPrefix, key);
}
static bool check_noTTL(const char *key) {
  return check_key_prefix(RuntimeOption::ApcNoTTLPrefix, key);
}

// stats_on_update should be called before updating sval with new value
static void stats_on_update(const StringData* key, const StoreValue* sval,
                            const SharedVariant* svar, int64 ttl) {
  if (RuntimeOption::EnableAPCSizeStats && !check_skip(key->data())) {
    int32 newSize = svar->getSpaceUsage();
    SharedStoreStats::updateDirect(sval->size, newSize);
    sval->size = newSize;
  }
  if (RuntimeOption::EnableAPCSizeStats &&
      (RuntimeOption::EnableAPCSizeGroup ||
       RuntimeOption::EnableAPCSizeDetail)) {
    SharedStoreStats::onDelete(key, sval->var, true, sval->expiry == 0);
    SharedStoreStats::onStore(key, svar, ttl, false);
  }
}
// stats_on_add should be called after writing sval with the value
static void stats_on_add(const StringData* key, const StoreValue* sval,
                         int64 ttl, bool prime, bool file) {
  if (RuntimeOption::EnableAPCSizeStats && !check_skip(key->data())) {
    int32 size = sval->var->getSpaceUsage();
    SharedStoreStats::addDirect(key->size(), size, prime, file);
    sval->size = size;
  }
  if (RuntimeOption::EnableAPCSizeStats &&
      (RuntimeOption::EnableAPCSizeGroup ||
       RuntimeOption::EnableAPCSizeDetail)) {
    SharedStoreStats::onStore(key, sval->var, ttl, prime);
  }
}

bool ConcurrentTableSharedStore::clear() {
  if (RuntimeOption::ApcConcurrentTableLockFree) {
    return false;
  }
  WriteLock l(m_lock);
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
       ++iter) {
    if (iter->second.inMem()) {
      iter->second.var->decRef();
    }
    free((void *)iter->first);
  }
  m_vars.clear();
  return true;
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
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  Map::accessor acc;
  if (m_vars.find(acc, key.data())) {
    if (expired && !acc->second.expired()) {
      return false;
    }
    if (acc->second.inMem()) {
      stats_on_delete(key.get(), &acc->second, expired);
      acc->second.var->decRef();
    } else {
      ASSERT(acc->second.inFile());
      ASSERT(acc->second.expiry == 0);
    }
    if (expired && acc->second.inFile()) {
      // a primed key expired, do not erase the table entry
      acc->second.var = NULL;
      acc->second.size = 0;
      acc->second.expiry = 0;
    } else {
      eraseAcc(acc);
    }
    return true;
  }
  return false;
}

// Should be called outside m_lock
void ConcurrentTableSharedStore::purgeExpired() {
  if ((atomic_add(m_purgeCounter, (uint64)1) %
       RuntimeOption::ApcPurgeFrequency) != 0) return;
  time_t now = time(NULL);
  ExpirationPair tmp;
  struct timespec tsBegin, tsEnd;
  gettime(CLOCK_MONOTONIC, &tsBegin);
  int i = 0;
  while (RuntimeOption::ApcPurgeRate < 0 || i < RuntimeOption::ApcPurgeRate) {
    if (!m_expQueue.try_pop(tmp)) {
      break;
    }
    if (tmp.second > now) {
      m_expQueue.push(tmp);
      break;
    }
    if (RuntimeOption::ApcUseFileStorage &&
        strcmp(tmp.first, RuntimeOption::ApcFileStorageFlagKey.c_str()) == 0) {
      s_apc_file_storage.adviseOut();
      addToExpirationQueue(RuntimeOption::ApcFileStorageFlagKey.c_str(),
                           time(NULL) +
                           RuntimeOption::ApcFileStorageAdviseOutPeriod);
      continue;
    }
    m_expMap.erase(tmp.first);
    eraseImpl(tmp.first, true);
    free((void *)tmp.first);
    ++i;
  }
  gettime(CLOCK_MONOTONIC, &tsEnd);
  int64 elapsed = gettime_diff_us(tsBegin, tsEnd);
  SharedStoreStats::addPurgingTime(elapsed);
  // Size could be inaccurate, but for stats reporting, it is good enough
  SharedStoreStats::setExpireQueueSize(m_expQueue.size());
}

void ConcurrentTableSharedStore::addToExpirationQueue(const char* key, int64 etime) {
  ExpMap::accessor acc;
  if (m_expMap.find(acc, key)) {
    acc->second++;
    return;
  }

  const char *copy = strdup(key);
  if (!m_expMap.insert(acc, copy)) {
    free((void *)copy);
    acc->second++;
    return;
  }
  ExpirationPair p(copy, etime);
  m_expQueue.push(p);
}

bool ConcurrentTableSharedStore::handlePromoteObj(CStrRef key,
                                                  SharedVariant* svar,
                                                  CVarRef value) {
  SharedVariant *converted = svar->convertObj(value);
  if (converted) {
    Map::accessor acc;
    if (!m_vars.find(acc, key.data())) {
      // There is a chance another thread deletes the key when this thread is
      // converting the object. In that case, we just bail
      converted->decRef();
      return false;
    }
    // A write lock was acquired during find
    StoreValue *sval = &acc->second;
    SharedVariant *sv = sval->var;
    // sv may not be same as svar here because some other thread may have
    // updated it already, check before updating
    if (sv == svar && !sv->isUnserializedObj()) {
      int64 ttl = sval->expiry ? sval->expiry - time(NULL) : 0;
      stats_on_update(key.get(), sval, converted, ttl);
      sval->var = converted;
      sv->decRef();
      return true;
    }
    converted->decRef();
  }
  return false;
}

static string std_apc_miss = "apc.miss";
static string std_apc_hit = "apc.hit";
static string std_apc_cas = "apc.cas";
static string std_apc_update = "apc.update";
static string std_apc_new = "apc.new";

SharedVariant* ConcurrentTableSharedStore::unserialize(CStrRef key,
                                                       const StoreValue* sval) {
  try {
    VariableUnserializer::Type sType =
      RuntimeOption::EnableApcSerialize ?
      VariableUnserializer::APCSerialize :
      VariableUnserializer::Serialize;

    VariableUnserializer vu(sval->sAddr, sval->getSerializedSize(), sType);
    Variant v;
    v.unserialize(&vu);
    sval->var = SharedVariant::Create(v, sval->isSerializedObj());
    stats_on_add(key.get(), sval, 0, true, true); // delayed prime
    return sval->var;
  } catch (Exception &e) {
    raise_notice("APC Primed fetch failed: key %s (%s).",
                 key.c_str(), e.getMessage().c_str());
    return NULL;
  }
}

bool ConcurrentTableSharedStore::get(CStrRef key, Variant &value) {
  const StoreValue *sval;
  SharedVariant *svar = NULL;
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  bool expired = false;
  bool promoteObj = false;
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, key.data())) {
      log_apc(std_apc_miss);
      return false;
    } else {
      sval = &acc->second;
      if (sval->expired()) {
        // Because it only has a read lock on the data, deletion from
        // expiration has to happen after the lock is released
        expired = true;
      } else {
        if (!sval->inMem()) {
          std::lock_guard<SmallLock> sval_lock(sval->lock);

          if (!sval->inMem()) {
            svar = unserialize(key, sval);
            if (!svar) return false;
          } else {
            svar = sval->var;
          }
        } else {
          svar = sval->var;
        }

        if (RuntimeOption::ApcAllowObj && svar->is(KindOfObject)) {
          // Hold ref here for later promoting the object
          svar->incRef();
          promoteObj = true;
        }
        value = svar->toLocal();
        stats_on_get(key.get(), svar);
      }
    }
  }
  if (expired) {
    log_apc(std_apc_miss);
    eraseImpl(key, true);
    return false;
  }
  log_apc(std_apc_hit);

  if (promoteObj)  {
    handlePromoteObj(key, svar, value);
    // release the extra ref
    svar->decRef();
  }
  return true;
}

static int64 get_int64_value(StoreValue* sval) {
  Variant v;
  if (sval->inMem()) {
    v = sval->var->toLocal();
  } else {
    ASSERT(sval->inFile());
    String s(sval->sAddr, sval->getSerializedSize(), AttachLiteral);
    v = apc_unserialize(s);
  }
  return v.toInt64();
}

int64 ConcurrentTableSharedStore::inc(CStrRef key, int64 step, bool &found) {
  found = false;
  int64 ret = 0;
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  StoreValue *sval;
  {
    Map::accessor acc;
    if (m_vars.find(acc, key.data())) {
      sval = &acc->second;
      if (!sval->expired()) {
        ret = get_int64_value(sval) + step;
        SharedVariant *svar = construct(Variant(ret));
        sval->var->decRef();
        sval->var = svar;
        found = true;
        log_apc(std_apc_hit);
      }
    }
  }
  return ret;
}

bool ConcurrentTableSharedStore::cas(CStrRef key, int64 old, int64 val) {
  bool success = false;
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  StoreValue *sval;
  {
    Map::accessor acc;
    if (m_vars.find(acc, key.data())) {
      sval = &acc->second;
      if (!sval->expired() && get_int64_value(sval) == old) {
        SharedVariant *var = construct(Variant(val));
        sval->var->decRef();
        sval->var = var;
        success = true;
        log_apc(std_apc_cas);
      }
    }
  }
  return success;
}

bool ConcurrentTableSharedStore::exists(CStrRef key) {
  const StoreValue *sval;
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  bool expired = false;
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, key.data())) {
      log_apc(std_apc_miss);
      return false;
    } else {
      sval = &acc->second;
      if (sval->expired()) {
        // Because it only has a read lock on the data, deletion from
        // expiration has to happen after the lock is released
        expired = true;
      } else {
        // No need toLocal() here, avoiding the copy
        if (sval->inMem()) {
          stats_on_get(key.get(), sval->var);
        }
      }
    }
  }
  if (expired) {
    log_apc(std_apc_miss);
    eraseImpl(key, true);
    return false;
  }
  log_apc(std_apc_hit);
  return true;
}

static int64 adjust_ttl(int64 ttl, bool overwritePrime) {
  if (RuntimeOption::ApcTTLLimit > 0 && !overwritePrime) {
    if (ttl == 0 || ttl > RuntimeOption::ApcTTLLimit) {
      return RuntimeOption::ApcTTLLimit;
    }
  }
  return ttl;
}

bool ConcurrentTableSharedStore::store(CStrRef key, CVarRef value, int64 ttl,
                                       bool overwrite /* = true */) {
  StoreValue *sval;
  SharedVariant* svar = construct(value);
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  const char *kcp = strdup(key.data());
  bool present;
  time_t expiry = 0;
  bool overwritePrime = false;
  {
    Map::accessor acc;
    present = !m_vars.insert(acc, kcp);
    sval = &acc->second;
    bool update = false;
    if (present) {
      free((void *)kcp);
      if (overwrite || sval->expired()) {
        // if ApcTTLLimit is set, then only primed keys can have expiry == 0
        overwritePrime = (sval->expiry == 0);
        if (sval->inMem()) {
          stats_on_update(key.get(), sval, svar,
                          adjust_ttl(ttl, overwritePrime));
          sval->var->decRef();
          update = true;
        } else {
          // mark the inFile copy invalid since we are updating the key
          sval->sAddr = NULL;
          sval->sSize = 0;
        }
      } else {
        svar->decRef();
        return false;
      }
    }
    int64 adjustedTtl = adjust_ttl(ttl, overwritePrime);
    if (check_noTTL(key.data())) {
      adjustedTtl = 0;
    }
    sval->set(svar, adjustedTtl);
    expiry = sval->expiry;
    if (!update) {
      stats_on_add(key.get(), sval, adjustedTtl, false, false);
    }
  }
  if (expiry) {
    addToExpirationQueue(key.data(), expiry);
  }
  if (RuntimeOption::ApcExpireOnSets) {
    purgeExpired();
  }
  if (present) {
    log_apc(std_apc_update);
  } else {
    log_apc(std_apc_new);
    if (RuntimeOption::EnableStats && RuntimeOption::EnableAPCKeyStats) {
      string prefix = "apc.new." + GetSkeleton(key);
      ServerStats::Log(prefix, 1);
    }
  }
  return true;
}

void ConcurrentTableSharedStore::prime
(const std::vector<SharedStore::KeyValuePair> &vars) {
  ConditionalReadLock l(m_lock, !RuntimeOption::ApcConcurrentTableLockFree ||
                                m_lockingFlag);
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const SharedStore::KeyValuePair &item = vars[i];
    Map::accessor acc;
    const char *copy = strdup(item.key);
    m_vars.insert(acc, copy);
    if (item.inMem()) {
      acc->second.set(item.value, 0);
    } else {
      acc->second.sAddr = item.sAddr;
      acc->second.sSize = item.sSize;
      continue;
    }
    if (RuntimeOption::APCSizeCountPrime) {
      StringData sd(copy);
      stats_on_add(&sd, &acc->second, 0, true, false);
    }
  }
}

bool ConcurrentTableSharedStore::constructPrime(CStrRef v, KeyValuePair& item,
                                                bool serialized) {
  if (s_apc_file_storage.getState() != SharedStoreFileStorage::StateInvalid &&
      (!v->isStatic() || serialized)) {
    // StaticString for non-object should consume limited amount of space,
    // not worth going through the file storage

    // TODO: currently we double serialize string for uniform handling later,
    // hopefully the unserialize won't be called often. We could further
    // optimize by storing more type info.
    String s = apc_serialize(v);
    char *sAddr = s_apc_file_storage.put(s.data(), s.size());
    if (sAddr) {
      item.sAddr = sAddr;
      item.sSize = serialized ? 0 - s.size() : s.size();
      return false;
    }
  }
  item.value = SharedVariant::Create(v, serialized);
  return true;
}

bool ConcurrentTableSharedStore::constructPrime(CVarRef v,
                                                KeyValuePair& item) {
  if (s_apc_file_storage.getState() != SharedStoreFileStorage::StateInvalid &&
      (IS_REFCOUNTED_TYPE(v.getType()))) {
    // Only do the storage for ref-counted type
    String s = apc_serialize(v);
    char *sAddr = s_apc_file_storage.put(s.data(), s.size());
    if (sAddr) {
      item.sAddr = sAddr;
      item.sSize = s.size();
      return false;
    }
  }
  item.value = SharedVariant::Create(v, false);
  return true;
}

void ConcurrentTableSharedStore::primeDone() {
  if (s_apc_file_storage.getState() != SharedStoreFileStorage::StateInvalid) {
    s_apc_file_storage.seal();
    s_apc_file_storage.hashCheck();
    // Schedule the adviseOut instead of doing it immediately, so that the
    // initial accesses to the primed keys are not too bad. Still, for
    // the keys in file, a deserialization from memory is required on first
    // access.
    addToExpirationQueue(RuntimeOption::ApcFileStorageFlagKey.c_str(),
                         time(NULL) +
                         RuntimeOption::ApcFileStorageAdviseOutPeriod);
  }

  for (set<string>::const_iterator iter =
         RuntimeOption::ApcCompletionKeys.begin();
       iter != RuntimeOption::ApcCompletionKeys.end(); ++iter) {
    Map::accessor acc;
    const char *copy = strdup(iter->c_str());
    if (m_vars.insert(acc, copy)) {
      acc->second.set(this->construct(1), 0);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// debugging support

void ConcurrentTableSharedStore::dump(std::ostream & out, bool keyOnly,
                                      int waitSeconds) {
  // Use write lock here to prevent concurrent ops running in parallel from
  // invalidatint the iterator.
  // This functionality is for debugging and should not be called regularly
  if (RuntimeOption::ApcConcurrentTableLockFree) {
    m_lockingFlag = true;
    int begin = time(NULL);
    Logger::Info("waiting %d seconds before dump", waitSeconds);
    while (time(NULL) - begin < waitSeconds) {
      sleep(1);
    }
  }
  WriteLock l(m_lock);
  Logger::Info("dumping apc");
  out << "Total " << m_vars.size() << std::endl;
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
    const char *key = iter->first;
    out << key;
    if (!keyOnly) {
      out << " #### ";
      const StoreValue *sval = &iter->second;
      if (!sval->expired()) {
        VariableSerializer vs(VariableSerializer::Serialize);
        Variant value;
        if (sval->inMem()) {
          value = sval->var->toLocal();
        } else {
          ASSERT(sval->inFile());
          // we need unserialize and serialize again because the format was
          // APCSerialize
          String s(sval->sAddr, sval->getSerializedSize(), AttachLiteral);
          value = apc_unserialize(s);
        }
        try {
          String valS(vs.serialize(value, true));
          out << valS->toCPPString();
        } catch (const Exception &e) {
          out << "Exception: " << e.what();
        }
      }
    }
    out << std::endl;
  }
  Logger::Info("dumping apc done");
  if (RuntimeOption::ApcConcurrentTableLockFree) {
    m_lockingFlag = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
