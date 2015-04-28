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

#include <mutex>
#include <set>
#include <vector>

#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/vm/treadmill.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

bool check_noTTL(const char* key, size_t keyLen) {
  for (auto& listElem : apcExtension::NoTTLPrefix) {
    auto const prefix = listElem.c_str();
    auto const prefixLen = listElem.size();
    if (keyLen >= prefixLen && memcmp(key, prefix, prefixLen) == 0) {
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void StoreValue::set(APCHandle* v, int64_t ttl) {
  data = v;
  expire = ttl ? time(nullptr) + ttl : 0;
}

bool StoreValue::expired() const {
  // When data is right(), expire is not valid to read, instead it's a lock.
  if (!data.left()) return false;
  return expire && time(nullptr) >= expire;
}

//////////////////////////////////////////////////////////////////////

EntryInfo::Type EntryInfo::getAPCType(const APCHandle* handle) {
  DataType type = handle->type();

  switch (type) {
    DT_UNCOUNTED_CASE:
      return EntryInfo::Type::Uncounted;

    case KindOfString:
      if (handle->isUncounted()) {
        return EntryInfo::Type::UncountedString;
      }
      return EntryInfo::Type::APCString;
    case KindOfArray:
      if (handle->isUncounted()) {
        return EntryInfo::Type::UncountedArray;
      }
      if (handle->isSerializedArray()) {
        return EntryInfo::Type::SerializedArray;
      }
      return EntryInfo::Type::APCArray;
    case KindOfObject:
      if (handle->isSerializedObj()) {
        return EntryInfo::Type::SerializedObject;
      }
      return EntryInfo::Type::APCObject;

    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      return EntryInfo::Type::Unknown;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool ConcurrentTableSharedStore::clear() {
  WriteLock l(m_lock);
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
       ++iter) {
    iter->second.data.match(
      [&] (APCHandle* handle) {
        handle->unreferenceRoot(iter->second.dataSize);
      },
      [&] (char*) {}
    );
    const void* vpKey = iter->first;
    free(const_cast<void*>(vpKey));
  }
  m_vars.clear();
  return true;
}

bool ConcurrentTableSharedStore::erase(const String& key) {
  return eraseImpl(key, false, 0);
}

void ConcurrentTableSharedStore::eraseAcc(Map::accessor& acc) {
  APCStats::getAPCStats().removeKey(strlen(acc->first));
  const void* vpkey = acc->first;
  m_vars.erase(acc);
  free(const_cast<void*>(vpkey));
}

/*
 * The Map::accessor here establishes a write lock, which means that other
 * threads, protected by read locks through Map::const_accessor, will not
 * read erased values from APC.
 *
 * The ReadLock here is to sync with clear(), which only has a WriteLock,
 * not a specific accessor.
 */
bool ConcurrentTableSharedStore::eraseImpl(const String& key,
                                           bool expired,
                                           int64_t oldestLive) {
  if (key.isNull()) return false;

  ReadLock l(m_lock);
  Map::accessor acc;
  if (!m_vars.find(acc, tagStringData(key.get()))) {
    return false;
  }
  if (expired && !acc->second.expired()) {
    return false;
  }

  auto& storeVal = acc->second;

  storeVal.data.match(
    [&] (APCHandle* var) {
      APCStats::getAPCStats().removeAPCValue(storeVal.dataSize, var,
        storeVal.expire == 0, expired);
      if (expired && storeVal.expire < oldestLive && var->isUncounted()) {
        APCTypedValue::fromHandle(var)->deleteUncounted();
      } else {
        var->unreferenceRoot(storeVal.dataSize);
      }

      eraseAcc(acc);
    },
    [&] (char* file) {
      assert(!expired);  // primed keys never say true to expired()
      eraseAcc(acc);
    }
  );

  return true;
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
    eraseImpl(tmp.first, true, oldestLive); // XXX allocating a String
    free((void *)tmp.first);
    ++i;
  }
}

// Should be called outside m_lock
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
  auto const pair = APCObject::MakeAPCObject(svar, value);
  if (!pair.handle) return false;
  auto const converted = pair.handle;
  auto const size = pair.size;

  Map::accessor acc;
  if (!m_vars.find(acc, tagStringData(key.get()))) {
    // There is a chance another thread deletes the key when this thread is
    // converting the object. In that case, we just bail
    converted->unreferenceRoot(size);
    return false;
  }

  // Our handle may not be same as `svar' here because some other thread may
  // have updated it already, check before updating.
  auto& sval = acc->second;
  auto const handle = sval.data.left();
  if (handle == svar && handle->isSerializedObj()) {
    sval.data = converted;
    APCStats::getAPCStats().updateAPCValue(
      converted, size, handle, sval.dataSize, sval.expire == 0, false);
    handle->unreferenceRoot(sval.dataSize);
    sval.dataSize = size;
    return true;
  }

  converted->unreferenceRoot(size);
  return false;
}

APCHandle* ConcurrentTableSharedStore::unserialize(const String& key,
                                                   StoreValue* sval) {
  auto const sAddr = sval->data.right();
  assert(sAddr != nullptr);

  try {
    auto const sType =
      apcExtension::EnableApcSerialize
        ? VariableUnserializer::Type::APCSerialize
        : VariableUnserializer::Type::Serialize;

    VariableUnserializer vu(sAddr, sval->getSerializedSize(), sType);
    Variant v;
    unserializeVariant(v, &vu);
    auto const pair = APCHandle::Create(v, sval->isSerializedObj());
    sval->data = pair.handle;
    sval->dataSize = pair.size;
    APCStats::getAPCStats().addAPCValue(pair.handle, pair.size, true);
    return pair.handle;
  } catch (ResourceExceededException&) {
    throw;
  } catch (Exception& e) {
    raise_notice("APC Primed fetch failed: key %s (%s).",
                 key.c_str(), e.getMessage().c_str());
    return nullptr;
  }
}

bool ConcurrentTableSharedStore::get(const String& key, Variant& value) {
  const StoreValue *sval;
  APCHandle *svar = nullptr;
  ReadLock l(m_lock);
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
        if (auto const handle = sval->data.left()) {
          svar = handle;
        } else {
          std::lock_guard<SmallLock> sval_lock(sval->lock);

          if (auto const handle = sval->data.left()) {
            svar = handle;
          } else {
            /*
             * Note that unserialize can run arbitrary php code via a __wakeup
             * routine, which could try to access this same key, and we're
             * holding various locks here.  This is only for promoting primed
             * values to in-memory values, so it's basically not a real
             * problem, but ... :)
             */
            svar = unserialize(key, const_cast<StoreValue*>(sval));
            if (!svar) return false;
          }
        }

        if (apcExtension::AllowObj && svar->type() == KindOfObject &&
            !svar->objAttempted()) {
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

int64_t ConcurrentTableSharedStore::inc(const String& key, int64_t step,
                                        bool& found) {
  found = false;
  ReadLock l(m_lock);

  Map::accessor acc;
  if (!m_vars.find(acc, tagStringData(key.get()))) {
    return 0;
  }
  auto& sval = acc->second;
  if (sval.expired()) return 0;

  /*
   * Inc only works on KindOfDouble or KindOfInt64, which are never kept in
   * file-backed storage from priming.  So we don't need to try to deserialize
   * anything or handle the case that sval.data is file-backed.
   */
  auto const oldHandle = sval.data.left();
  if (oldHandle == nullptr) return 0;
  if (oldHandle->type() != KindOfInt64 &&
      oldHandle->type() != KindOfDouble) {
    return 0;
  }

  auto const ret = oldHandle->toLocal().toInt64() + step;
  auto const pair = APCHandle::Create(Variant(ret), false);
  APCStats::getAPCStats().updateAPCValue(pair.handle, pair.size,
                                         oldHandle, sval.dataSize,
                                         sval.expire == 0, false);
  oldHandle->unreferenceRoot(sval.dataSize);
  sval.data = pair.handle;
  sval.dataSize = pair.size;
  found = true;
  return ret;
}

bool ConcurrentTableSharedStore::cas(const String& key, int64_t old,
                                     int64_t val) {
  ReadLock l(m_lock);

  Map::accessor acc;
  if (!m_vars.find(acc, tagStringData(key.get()))) {
    return false;
  }

  auto& sval = acc->second;
  if (sval.expired()) return false;

  auto const oldHandle = sval.data.match(
    [&] (APCHandle* h) {
      return h;
    },
    [&] (char* file) {
      return unserialize(key, &sval);
    }
  );
  if (!oldHandle || oldHandle->toLocal().toInt64() != old) {
    return false;
  }

  auto const pair = APCHandle::Create(Variant(val), false);
  APCStats::getAPCStats().updateAPCValue(pair.handle, pair.size,
                                         oldHandle, sval.dataSize,
                                         sval.expire == 0, false);
  oldHandle->unreferenceRoot(sval.dataSize);
  sval.data = pair.handle;
  sval.dataSize = pair.size;
  return true;
}

bool ConcurrentTableSharedStore::exists(const String& key) {
  const StoreValue *sval;
  ReadLock l(m_lock);
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

bool ConcurrentTableSharedStore::add(const String& key,
                                     const Variant& val,
                                     int64_t ttl) {
  return storeImpl(key, val, ttl, false, true);
}

void ConcurrentTableSharedStore::set(const String& key,
                                     const Variant& val,
                                     int64_t ttl) {
  storeImpl(key, val, ttl, true, true);
}

void ConcurrentTableSharedStore::setWithoutTTL(const String& key,
                                               const Variant& val) {
  storeImpl(key, val, 0, true, false);
}

bool ConcurrentTableSharedStore::storeImpl(const String& key,
                                           const Variant& value,
                                           int64_t ttl,
                                           bool overwrite,
                                           bool limit_ttl) {
  StoreValue *sval;
  auto svar = APCHandle::Create(value, false);
  auto keyLen = key.size();
  ReadLock l(m_lock);
  char* const kcp = strdup(key.data());
  bool present;
  time_t expiry = 0;
  bool overwritePrime = false;
  {
    Map::accessor acc;
    APCHandle* current = nullptr;
    present = !m_vars.insert(acc, kcp);
    sval = &acc->second;
    if (present) {
      free(kcp);
      if (overwrite || sval->expired()) {
        sval->data.match(
          [&] (APCHandle* handle) {
            current = handle;
            // If ApcTTLLimit is set, then only primed keys can have
            // expire == 0.
            overwritePrime = sval->expire == 0;
          },
          [&] (char*) {
            // Was inFile, but won't be anymore.
            sval->data = nullptr;
            sval->dataSize = 0;
            overwritePrime = true;
          }
        );
      } else {
        svar.handle->unreferenceRoot(svar.size);
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
      if (sval->expire == 0 && adjustedTtl != 0) {
        APCStats::getAPCStats().removeAPCValue(
          sval->dataSize, current, true, sval->expired());
        APCStats::getAPCStats().addAPCValue(svar.handle, svar.size, false);
      } else {
        APCStats::getAPCStats().updateAPCValue(
          svar.handle, svar.size, current, sval->dataSize,
          sval->expire == 0, sval->expired());
      }
      current->unreferenceRoot(sval->dataSize);
    } else {
      APCStats::getAPCStats().addAPCValue(svar.handle, svar.size, present);
    }

    sval->set(svar.handle, adjustedTtl);
    sval->dataSize = svar.size;
    expiry = sval->expire;
  }

  if (expiry) {
    addToExpirationQueue(key.data(), expiry);
  }
  if (apcExtension::ExpireOnSets) {
    purgeExpired();
  }

  return true;
}

void ConcurrentTableSharedStore::prime(const std::vector<KeyValuePair>& vars) {
  ReadLock l(m_lock);
  // we are priming, so we are not checking existence or expiration
  for (unsigned int i = 0; i < vars.size(); i++) {
    const KeyValuePair &item = vars[i];
    Map::accessor acc;
    auto const keyLen = strlen(item.key);
    auto const copy = strdup(item.key);
    if (m_vars.insert(acc, copy)) {
      APCStats::getAPCStats().addPrimedKey(keyLen);
    } else {
      free(copy);

      // We're going to overwrite what was there.
      auto& sval = acc->second;
      sval.data.match(
        [&] (APCHandle* handle) {
          handle->unreferenceRoot(sval.dataSize);
        },
        [&] (char*) {}
      );
      sval.data     = nullptr;
      sval.dataSize = 0;
      sval.expire   = 0;
    }

    if (item.inMem()) {
      APCStats::getAPCStats().addAPCValue(item.value, item.sSize, true);
      acc->second.set(item.value, 0);
      acc->second.dataSize = item.sSize;
    } else {
      acc->second.data     = item.sAddr;
      acc->second.dataSize = item.sSize;
      APCStats::getAPCStats().addInFileValue(std::abs(acc->second.dataSize));
    }
  }
}

bool ConcurrentTableSharedStore::constructPrime(const String& v,
                                                KeyValuePair& item,
                                                bool serialized) {
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid &&
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
  auto pair = APCHandle::Create(v, serialized);
  item.value = pair.handle;
  item.sSize = pair.size;
  return true;
}

bool ConcurrentTableSharedStore::constructPrime(const Variant& v,
                                                KeyValuePair& item) {
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid &&
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
  auto pair = APCHandle::Create(v, false);
  item.value = pair.handle;
  item.sSize = pair.size;
  return true;
}

void ConcurrentTableSharedStore::primeDone() {
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid) {
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
    auto const copy = strdup(iter->c_str());
    if (!m_vars.insert(acc, copy)) {
      free(copy);
      return;
    }
    auto const pair = APCHandle::Create(Variant(1), false);
    acc->second.set(pair.handle, 0);
    acc->second.dataSize = pair.size;
    APCStats::getAPCStats().addAPCValue(pair.handle, pair.size, true);
  }
}

///////////////////////////////////////////////////////////////////////////////
// debugging and info/stats support

std::vector<EntryInfo> ConcurrentTableSharedStore::getEntriesInfo() {
  auto entries = std::vector<EntryInfo>{};

  int64_t curr_time = time(nullptr);
  entries.reserve(m_vars.size() + 1000);

  {
    WriteLock l(m_lock);
    for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
      const auto key = iter->first;
      const auto sval = &iter->second;

      int32_t size;
      auto type = EntryInfo::Type::Unknown;
      auto const inMem = sval->data.match(
        [&] (APCHandle* handle) {
          size = sval->dataSize;
          type = EntryInfo::getAPCType(handle);
          return true;
        },
        [&] (char*) {
          size = sval->getSerializedSize();
          return false;
        }
      );

      int64_t ttl = 0;
      if (inMem && sval->expire) {
        ttl = sval->expire - curr_time;
        if (ttl == 0) ttl = 1; // don't want to confuse with primed keys
      }

      entries.emplace_back(key, inMem, size, ttl, type);
    }
  }
  std::sort(entries.begin(), entries.end(),
            [] (const EntryInfo& e1, const EntryInfo& e2) {
              return e1.key < e2.key; });
  return entries;
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

      auto const value = sval->data.match(
        [&] (APCHandle* handle) {
          return handle->toLocal();
        },
        [&] (char* sAddr) {
          // we need unserialize and serialize again because the format was
          // APCSerialize
          return apc_unserialize(sAddr, sval->getSerializedSize());
        }
      );

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

void ConcurrentTableSharedStore::dump(std::ostream& out, DumpMode dumpMode) {
  Logger::Info("dumping apc");

  switch (dumpMode) {
  case DumpMode::KeyAndValue:
    dumpKeyAndValue(out);
    break;

  case DumpMode::KeyOnly:
    for (auto& e : getEntriesInfo()) {
      out << e.key << '\n';
    }
    break;

  case DumpMode::KeyAndMeta:
    {
      out << "key inmem size ttl type\n";
      for (auto& entry : getEntriesInfo()) {
        out << entry.key << " "
            << static_cast<int32_t>(entry.inMem) << " "
            << entry.size << " "
            << entry.ttl << " "
            << static_cast<int32_t>(entry.type) << '\n';
      }
    }
    break;
  }

  Logger::Info("dumping apc done");
}

void ConcurrentTableSharedStore::dumpRandomKeys(std::ostream &out,
                                                uint32_t count) {
  for (; count > 0; count--) {
    m_vars.getRandomAPCEntry(out);
  }
}

template<typename Key, typename T, typename HashCompare>
void ConcurrentTableSharedStore
      ::APCMap<Key,T,HashCompare>
      ::getRandomAPCEntry(std::ostream &out) {
#if TBB_VERSION_MAJOR == 4 && TBB_VERSION_MINOR == 0
  while (this->my_size > 0) {
    auto randIndex = rand() % this->my_size;
    auto mahBucket = this->segment_index_of(randIndex);
    auto segmentIndex = randIndex - this->segment_base(mahBucket);
    auto bucketPtr = this->my_table[mahBucket];
    {
      bucketPtr->mutex.lock();
      SCOPE_EXIT{ bucketPtr->mutex.unlock(); };
      auto nodePtr = (bucketPtr + segmentIndex)->node_list;
      if (nodePtr != nullptr &&
          nodePtr != tbb::interface5::internal::rehash_req &&
          nodePtr != tbb::interface5::internal::empty_rehashed) {
        uintptr_t apcPairPtr =
         (reinterpret_cast<uintptr_t>(nodePtr) +
          sizeof(tbb::interface5::internal::hash_map_node_base));
        auto apcPair =
          (reinterpret_cast<std::pair<const char *, StoreValue>*>
                    (apcPairPtr));
        out << apcPair->first << "," << apcPair->second.dataSize << "\n";
        return;
      }
    }
  }
#else
  out << "Incompatible TBB library\n";
#endif
  return;
}

//////////////////////////////////////////////////////////////////////
}
