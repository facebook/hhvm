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

#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/ext/ext_apc.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/vm/treadmill.h"
#include <mutex>
#include <set>
#include <vector>

using std::set;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static bool check_key_prefix(const std::vector<std::string>& list,
                             const char *key, size_t keyLen) {
  for (unsigned int i = 0; i < list.size(); ++i) {
    const char *prefix = list[i].c_str();
    size_t prefixLen = list[i].size();
    if (keyLen >= prefixLen && memcmp(key, prefix, prefixLen) == 0) {
      return true;
    }
  }
  return false;
}
static bool check_noTTL(const char *key, size_t keyLen) {
  return check_key_prefix(apcExtension::NoTTLPrefix, key, keyLen);
}

std::string ConcurrentTableSharedStore::GetSkeleton(const String& key) {
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

EntryInfo::EntryType EntryInfo::getAPCType(const APCHandle* handle) {
  DataType type = handle->getType();
  if (!IS_REFCOUNTED_TYPE(type)) {
    return EntryInfo::EntryType::Uncounted;
  }
  switch (type) {
  case KindOfString:
    if (handle->getUncounted()) {
      return EntryInfo::EntryType::UncountedString;
    }
    return EntryInfo::EntryType::APCString;
  case KindOfArray:
    if (handle->getUncounted()) {
      return EntryInfo::EntryType::UncountedArray;
    } else if (handle->getSerializedArray()) {
      return EntryInfo::EntryType::SerializedArray;
    }
    return EntryInfo::EntryType::APCArray;
  case KindOfObject:
    if (handle->getIsObj()) {
      return EntryInfo::EntryType::APCObject;
    }
    return EntryInfo::EntryType::SerializedObject;
  default:
    return EntryInfo::EntryType::Unknown;
  }
}

bool ConcurrentTableSharedStore::clear() {
  if (apcExtension::ConcurrentTableLockFree) {
    return false;
  }
  WriteLock l(m_lock);
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
       ++iter) {
    if (iter->second.inMem()) {
      iter->second.var->unreferenceRoot(iter->second.size);
    }
    free((void *)iter->first);
  }
  m_vars.clear();
  return true;
}

bool ConcurrentTableSharedStore::erase(const String& key,
                                       bool expired /* = false */) {
  return eraseImpl(key, expired);
}

/**
 * The Map::accessor here establishes a write lock, which means that other
 * threads, protected by read locks through Map::const_accessor, will not
 * read erased values from APC.
 * The ReadLock here is to sync with clear(), which only has a WriteLock,
 * not a specific accessor.
 */
bool ConcurrentTableSharedStore::eraseImpl(const String& key,
                                           bool expired,
                                           int64_t oldestLive) {
  if (key.isNull()) return false;
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  Map::accessor acc;
  if (m_vars.find(acc, tagStringData(key.get()))) {
    if (expired && !acc->second.expired()) {
      return false;
    }
    if (acc->second.inMem()) {
      APCStats::getAPCStats().removeAPCValue(
          acc->second.size, acc->second.var,
          acc->second.expiry == 0, expired);
      if (expired && acc->second.expiry < oldestLive &&
          acc->second.var->getUncounted()) {
        APCTypedValue::fromHandle(acc->second.var)->deleteUncounted();
      } else {
        acc->second.var->unreferenceRoot(acc->second.size);
      }
    } else {
      assert(acc->second.inFile());
      assert(acc->second.expiry == 0);
    }
    if (expired && acc->second.inFile()) {
      // a primed key expired, do not erase the table entry
      acc->second.var = nullptr;
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
  if (m_purgeCounter.fetch_add(1, std::memory_order_relaxed) %
      apcExtension::PurgeFrequency != 0) {
    return;
  }
  time_t now = time(nullptr);
  int64_t oldestLive = apcExtension::UseUncounted ?
      HPHP::Treadmill::getOldestStartTime() : 0;
  ExpirationPair tmp;
  int i = 0;
  while (apcExtension::PurgeRate < 0 || i < apcExtension::PurgeRate) {
    if (!m_expQueue.try_pop(tmp)) {
      break;
    }
    if (tmp.second > now) {
      m_expQueue.push(tmp);
      break;
    }
    if (apcExtension::UseFileStorage &&
        strcmp(tmp.first, apcExtension::FileStorageFlagKey.c_str()) == 0) {
      s_apc_file_storage.adviseOut();
      addToExpirationQueue(apcExtension::FileStorageFlagKey.c_str(),
                           time(nullptr) +
                           apcExtension::FileStorageAdviseOutPeriod);
      continue;
    }
    m_expMap.erase(tmp.first);
    eraseImpl(tmp.first, true, oldestLive);
    free((void *)tmp.first);
    ++i;
  }
}

void ConcurrentTableSharedStore::addToExpirationQueue(const char* key,
                                                      int64_t etime) {
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

bool ConcurrentTableSharedStore::handlePromoteObj(const String& key,
                                                  APCHandle* svar,
                                                  const Variant& value) {
  size_t size = 0;
  APCHandle *converted = APCObject::MakeAPCObject(svar, size, value);
  if (converted) {
    Map::accessor acc;
    if (!m_vars.find(acc, tagStringData(key.get()))) {
      // There is a chance another thread deletes the key when this thread is
      // converting the object. In that case, we just bail
      converted->unreferenceRoot(size);
      return false;
    }
    // A write lock was acquired during find
    StoreValue *sval = &acc->second;
    APCHandle *sv = sval->var;
    // sv may not be same as svar here because some other thread may have
    // updated it already, check before updating
    if (sv == svar && !sv->getIsObj()) {
      sval->var = converted;
      APCStats::getAPCStats().updateAPCValue(
          converted, size, sv, sval->size, sval->expiry == 0, false);
      sv->unreferenceRoot(sval->size);
      sval->size = size;
      return true;
    }
    converted->unreferenceRoot(size);
  }
  return false;
}

APCHandle* ConcurrentTableSharedStore::unserialize(const String& key,
                                                   const StoreValue* sval) {
  try {
    VariableUnserializer::Type sType =
      apcExtension::EnableApcSerialize ?
      VariableUnserializer::Type::APCSerialize :
      VariableUnserializer::Type::Serialize;

    VariableUnserializer vu(sval->sAddr, sval->getSerializedSize(), sType);
    Variant v;
    v.unserialize(&vu);
    size_t size = 0;
    sval->var = APCHandle::Create(v, size, sval->isSerializedObj());
    sval->size = size;
    APCStats::getAPCStats().addAPCValue(sval->var, size, true);
    return sval->var;
  } catch (Exception &e) {
    raise_notice("APC Primed fetch failed: key %s (%s).",
                 key.c_str(), e.getMessage().c_str());
    return nullptr;
  }
}

bool ConcurrentTableSharedStore::get(const String& key, Variant &value) {
  const StoreValue *sval;
  APCHandle *svar = nullptr;
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  bool expired = false;
  bool promoteObj = false;
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, tagStringData(key.get()))) {
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

        if (apcExtension::AllowObj && svar->is(KindOfObject) &&
            !svar->getObjAttempted()) {
          // Hold ref here for later promoting the object
          svar->reference();
          promoteObj = true;
        }
        value = svar->toLocal();
      }
    }
  }
  if (expired) {
    eraseImpl(key, true, apcExtension::UseUncounted ?
                              HPHP::Treadmill::getOldestStartTime() : 0);
    return false;
  }

  if (promoteObj)  {
    handlePromoteObj(key, svar, value);
    // release the extra ref
    svar->unreference();
  }
  return true;
}

static int64_t get_int64_value(StoreValue* sval) {
  Variant v;
  if (sval->inMem()) {
    v = sval->var->toLocal();
  } else {
    assert(sval->inFile());
    v = apc_unserialize(sval->sAddr, sval->getSerializedSize());
  }
  return v.toInt64();
}

int64_t ConcurrentTableSharedStore::inc(const String& key, int64_t step,
                                        bool &found) {
  found = false;
  int64_t ret = 0;
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  StoreValue *sval;
  {
    Map::accessor acc;
    if (m_vars.find(acc, tagStringData(key.get()))) {
      sval = &acc->second;
      if (!sval->expired()) {
        ret = get_int64_value(sval) + step;
        size_t size = 0;
        APCHandle *svar = construct(Variant(ret), size);
        APCStats::getAPCStats().updateAPCValue(
            svar, size, sval->var, sval->size, sval->expiry == 0, false);
        sval->var->unreferenceRoot(sval->size);
        sval->var = svar;
        sval->size = size;
        found = true;
      }
    }
  }
  return ret;
}

bool ConcurrentTableSharedStore::cas(const String& key, int64_t old,
                                     int64_t val) {
  bool success = false;
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  StoreValue *sval;
  {
    Map::accessor acc;
    if (m_vars.find(acc, tagStringData(key.get()))) {
      sval = &acc->second;
      if (!sval->expired() && get_int64_value(sval) == old) {
        size_t size = 0;
        APCHandle *var = construct(Variant(val), size);
        APCStats::getAPCStats().updateAPCValue(
            var, size, sval->var, sval->size, sval->expiry == 0, false);
        sval->var->unreferenceRoot(sval->size);
        sval->var = var;
        sval->size = size;
        success = true;
      }
    }
  }
  return success;
}

bool ConcurrentTableSharedStore::exists(const String& key) {
  const StoreValue *sval;
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  bool expired = false;
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, tagStringData(key.get()))) {
      return false;
    } else {
      sval = &acc->second;
      if (sval->expired()) {
        // Because it only has a read lock on the data, deletion from
        // expiration has to happen after the lock is released
        expired = true;
      }
    }
  }
  if (expired) {
    eraseImpl(key, true, apcExtension::UseUncounted ?
                              HPHP::Treadmill::getOldestStartTime() : 0);
    return false;
  }
  return true;
}

static int64_t adjust_ttl(int64_t ttl, bool overwritePrime) {
  if (apcExtension::TTLLimit > 0 && !overwritePrime) {
    if (ttl == 0 || ttl > apcExtension::TTLLimit) {
      return apcExtension::TTLLimit;
    }
  }
  return ttl;
}

bool ConcurrentTableSharedStore::store(const String& key, const Variant& value,
                                       int64_t ttl,
                                       bool overwrite /* = true */,
                                       bool limit_ttl /* = true */) {
  StoreValue *sval;
  size_t size = 0;
  APCHandle* svar = construct(value, size);
  auto keyLen = key.size();
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  const char *kcp = strdup(key.data());
  bool present;
  time_t expiry = 0;
  bool overwritePrime = false;
  {
    Map::accessor acc;
    APCHandle* current = nullptr;
    present = !m_vars.insert(acc, kcp);
    sval = &acc->second;
    if (present) {
      free((void*)kcp);
      if (overwrite || sval->expired()) {
        // if ApcTTLLimit is set, then only primed keys can have expiry == 0
        overwritePrime = (sval->expiry == 0);
        if (sval->inMem()) {
          current = sval->var;
        } else {
          sval->sAddr = nullptr;
          sval->sSize = 0;
        }
      } else {
        svar->unreferenceRoot(size);
        return false;
      }
    } else {
      APCStats::getAPCStats().addKey(keyLen);
    }
    int64_t adjustedTtl = adjust_ttl(ttl, overwritePrime || !limit_ttl);
    if (check_noTTL(key.data(), key.size())) {
      adjustedTtl = 0;
    }
    if (current) {
      if (sval->expiry == 0 && adjustedTtl != 0) {
        APCStats::getAPCStats().removeAPCValue(
            sval->size, current, true, sval->expired());
        APCStats::getAPCStats().addAPCValue(svar, size, false);
      } else {
        APCStats::getAPCStats().updateAPCValue(
            svar, size, current, sval->size,
            sval->expiry == 0, sval->expired());
      }
      current->unreferenceRoot(sval->size);
    } else {
      APCStats::getAPCStats().addAPCValue(svar, size, present);
    }
    sval->set(svar, adjustedTtl);
    sval->size = size;
    expiry = sval->expiry;
  }
  if (expiry) {
    addToExpirationQueue(key.data(), expiry);
  }
  if (apcExtension::ExpireOnSets) {
    purgeExpired();
  }
  return true;
}

void ConcurrentTableSharedStore::prime(const std::vector<KeyValuePair> &vars) {
  ConditionalReadLock l(m_lock, !apcExtension::ConcurrentTableLockFree ||
                                m_lockingFlag);
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const KeyValuePair &item = vars[i];
    Map::accessor acc;
    auto keyLen = strlen(item.key);
    const char *copy = strdup(item.key);
    if (m_vars.insert(acc, copy)) {
      APCStats::getAPCStats().addPrimedKey(keyLen);
    }
    if (item.inMem()) {
      APCStats::getAPCStats().addAPCValue(item.value, item.sSize, true);
      acc->second.set(item.value, 0);
      acc->second.size = item.sSize;
    } else {
      acc->second.sAddr = item.sAddr;
      acc->second.sSize = item.sSize;
      APCStats::getAPCStats().addInFileValue(std::abs(acc->second.sSize));
    }
  }
}

bool ConcurrentTableSharedStore::constructPrime(const String& v,
                                                KeyValuePair& item,
                                                bool serialized) {
  if (s_apc_file_storage.getState() !=
      SharedStoreFileStorage::StorageState::Invalid &&
      (!v.get()->isStatic() || serialized)) {
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
  size_t size = 0;
  item.value = APCHandle::Create(v, size, serialized);
  item.sSize = size;
  return true;
}

bool ConcurrentTableSharedStore::constructPrime(const Variant& v,
                                                KeyValuePair& item) {
  if (s_apc_file_storage.getState() !=
      SharedStoreFileStorage::StorageState::Invalid &&
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
  size_t size = 0;
  item.value = APCHandle::Create(v, size, false);
  item.sSize = size;
  return true;
}

void ConcurrentTableSharedStore::primeDone() {
  if (s_apc_file_storage.getState() !=
      SharedStoreFileStorage::StorageState::Invalid) {
    s_apc_file_storage.seal();
    s_apc_file_storage.hashCheck();
    // Schedule the adviseOut instead of doing it immediately, so that the
    // initial accesses to the primed keys are not too bad. Still, for
    // the keys in file, a deserialization from memory is required on first
    // access.
    addToExpirationQueue(apcExtension::FileStorageFlagKey.c_str(),
                         time(nullptr) +
                         apcExtension::FileStorageAdviseOutPeriod);
  }

  for (auto iter = apcExtension::CompletionKeys.begin();
       iter != apcExtension::CompletionKeys.end(); ++iter) {
    Map::accessor acc;
    const char *copy = strdup(iter->c_str());
    if (m_vars.insert(acc, copy)) {
      size_t size = 0;
      auto handle = this->construct(1, size);
      acc->second.set(handle, 0);
      acc->second.size = size;
      APCStats::getAPCStats().addAPCValue(handle, size, true);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// debugging and info/stats support

void ConcurrentTableSharedStore::getEntriesInfo(
    std::vector<EntryInfo>& entries) {
  int64_t curr_time = time(nullptr);
  entries.reserve(m_vars.size() + 1000);
  {
    WriteLock l(m_lock);
    for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
      const auto key = iter->first;
      const auto sval = &iter->second;

      int32_t size;
      EntryInfo::EntryType type = EntryInfo::EntryType::Unknown;
      if (sval->inMem()) {
        size = sval->size;
        type = EntryInfo::getAPCType(sval->var);
      } else {
        size = sval->getSerializedSize();
      }
      int64_t ttl = 0;
      if (sval->expiry) {
        ttl = sval->expiry - curr_time;
        if (ttl == 0) ttl = 1; // don't want to confuse with primed keys
      }
      entries.emplace_back(key, sval->var != nullptr, size, ttl, type);
    }
  }
}

void ConcurrentTableSharedStore::dumpKeyAndValue(std::ostream & out) {
  WriteLock l(m_lock);
  out << "Total " << m_vars.size() << std::endl;
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
    const char *key = iter->first;
    out << key;
    out << " #### ";
    const StoreValue *sval = &iter->second;
    if (!sval->expired()) {
      VariableSerializer vs(VariableSerializer::Type::Serialize);
      Variant value;
      if (sval->inMem()) {
        value = sval->var->toLocal();
      } else {
        assert(sval->inFile());
        // we need unserialize and serialize again because the format was
        // APCSerialize
        value = apc_unserialize(sval->sAddr, sval->getSerializedSize());
      }
      try {
        String valS(vs.serialize(value, true));
        out << valS.toCppString();
      } catch (const Exception &e) {
        out << "Exception: " << e.what();
      }
    }
    out << std::endl;
  }
}

void ConcurrentTableSharedStore::dumpKeyOnly(
    std::ostream & out, std::vector<EntryInfo>& entries) {
  for (auto& entry: entries) {
    out << entry.key << std::endl;
  }
}

void ConcurrentTableSharedStore::dumpKeyAndMeta(
    std::ostream & out, std::vector<EntryInfo>& entries) {
  out << "key inmem size ttl type" << std::endl;
  for (auto& entry: entries) {
    out << entry.key << " "
        << static_cast<int32_t>(entry.inMem) << " "
        << entry.size << " "
        << entry.ttl << " "
        << static_cast<int32_t>(entry.type) << std::endl;
  }
}

void ConcurrentTableSharedStore::dump(std::ostream & out,
                                      enum DumpMode dumpMode,
                                      int waitSeconds) {
  if (apcExtension::ConcurrentTableLockFree) {
    m_lockingFlag = true;
    int begin = time(nullptr);
    Logger::Info("waiting %d seconds before dump", waitSeconds);
    while (time(nullptr) - begin < waitSeconds) {
      sleep(1);
    }
  }
  Logger::Info("dumping apc");
  if (dumpMode == DumpMode::keyAndValue) {
    dumpKeyAndValue(out);
  } else {
    std::vector<EntryInfo> entries;
    getEntriesInfo(entries);
    if (dumpMode == DumpMode::keyOnly) {
      dumpKeyOnly(out, entries);
    } else {
      assert(dumpMode == DumpMode::keyAndMeta);
      dumpKeyAndMeta(out, entries);
    }
  }
  Logger::Info("dumping apc done");
  if (apcExtension::ConcurrentTableLockFree) {
    m_lockingFlag = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
