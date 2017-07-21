/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <string>
#include <vector>
#include <folly/Format.h>

#include "hphp/util/logger.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/ext/apc/ext_apc.h"
#include "hphp/runtime/ext/apc/snapshot.h"
#include "hphp/runtime/vm/treadmill.h"

using folly::SharedMutex;

namespace HPHP {

TRACE_SET_MOD(apc);

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

#ifdef HPHP_TRACE
std::string show(const StoreValue& sval) {
  return sval.data().left() ?
    folly::sformat("size {} kind {}", sval.dataSize, (int)sval.getKind()) :
    folly::sformat("size {} serialized", std::abs(sval.dataSize));
}
#endif

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void StoreValue::set(APCHandle* v, int64_t ttl) {
  setHandle(v);
  mtime = time(nullptr);
  if (c_time == 0)  c_time = mtime;
  expire = ttl ? mtime + ttl : 0;
}

bool StoreValue::expired() const {
  // For primed values, 'expire' is not valid to read.
  if (c_time == 0) return false;
  return expire && time(nullptr) >= expire;
}

//////////////////////////////////////////////////////////////////////

EntryInfo::Type EntryInfo::getAPCType(const APCHandle* handle) {
  switch (handle->kind()) {
    case APCKind::Uninit:
    case APCKind::Null:
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double:
    case APCKind::StaticString:
    case APCKind::StaticArray:
    case APCKind::StaticVec:
    case APCKind::StaticDict:
    case APCKind::StaticKeyset:
      return EntryInfo::Type::Uncounted;
    case APCKind::UncountedString:
      return EntryInfo::Type::UncountedString;
    case APCKind::SharedString:
      return EntryInfo::Type::APCString;
    case APCKind::UncountedArray:
      return EntryInfo::Type::UncountedArray;
    case APCKind::UncountedVec:
      return EntryInfo::Type::UncountedVec;
    case APCKind::UncountedDict:
      return EntryInfo::Type::UncountedDict;
    case APCKind::UncountedKeyset:
      return EntryInfo::Type::UncountedKeyset;
    case APCKind::SerializedArray:
      return EntryInfo::Type::SerializedArray;
    case APCKind::SerializedVec:
      return EntryInfo::Type::SerializedVec;
    case APCKind::SerializedDict:
      return EntryInfo::Type::SerializedDict;
    case APCKind::SerializedKeyset:
      return EntryInfo::Type::SerializedKeyset;
    case APCKind::SharedVec:
      return EntryInfo::Type::APCVec;
    case APCKind::SharedDict:
      return EntryInfo::Type::APCDict;
    case APCKind::SharedKeyset:
      return EntryInfo::Type::APCKeyset;
    case APCKind::SharedArray:
    case APCKind::SharedPackedArray:
      return EntryInfo::Type::APCArray;
    case APCKind::SerializedObject:
      return EntryInfo::Type::SerializedObject;
    case APCKind::SharedObject:
    case APCKind::SharedCollection:
      return EntryInfo::Type::APCObject;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

/*
 * Read cache layer for frequently accessed but rarely updated APC entries.
 * Doesn't support APC values that require refcounting, i.e., it supports
 * the 'singletons' null, true, false and the uncounted array/string types.
 * Designed to be used to replicate a subset of ConcurrentTableSharedStore.
 * Keys can be filtered based on prefixes (apcExtension::HotKeyPrefix).
 * Lock-free; if a stored entry is cleared, its value must be kept alive for
 * duration of any requests in flight at that time (treadmill).
 *
 * Capacity is fixed at initialization. Does not check expiration on 'get',
 * so best used with periodic purge enabled (apcExtension::ExpireOnSets).
 *
 * TODO(11228222): Upper-bound the time between periodic purges.
 */
struct HotCache {
  /*
   * Indices are never invalidated once assigned to a key (clearing an entry
   * does not actually erase the key from underlying array, just hides it).
   */
  using Idx = StoreValue::HotCacheIdx;

  void initialize();

  /*
   * If 'key' has an associated value in the cache, returns true and
   * sets 'value' (but leaves 'idx' in an undefined state).
   * Else, returns false, leaves 'value' unchanged, but fills 'idx' with
   * information about the failure to be passed to 'store'.
   */
  bool get(const StringData* key, Variant& value, Idx& idx) const;

  /*
   * Try to add or update an entry (key, svar), if they both are eligible
   * and there is still room. 'idx' must be the result of a failed call
   * to 'get' for the same key. On success, updates sval->hotIndex to reference
   * the added/updated entry and returns true.
   */
  bool store(Idx idx, const StringData* key,
             APCHandle* svar, const StoreValue* sval);

  /*
   * Clear the entry referenced by 'sval' (set by a call to 'store') if any.
   * Return false and do nothing if 'sval' does not reference any entry.
   * Safe to call concurrently and repeatedly (idempotent). The caller must
   * ensure the effects of any previous call to 'store' for this key are visible
   * before this methods reads from 'sval' (e.g., by only calling these methods
   * under read/write locks like ConcurrentTableSharedStore::get/store/erase).
   */
  bool clearValue(const StoreValue& sval) {
    return clearValueIdx(sval.hotIndex.load(std::memory_order_relaxed));
  }

  /*
   * Consistent with 'get', only less output.
   */
  bool hasValue(const StringData* key) const;

 private:
  // Uncounted ArrayData is stored 'unwrapped' to save one dereference.
  using HotValue = Either<APCHandle*,ArrayData*,either_policy::high_bit>;
  using HotValueRaw = HotValue::Opaque;

  // Keys stored are char*, but lookup/insert always use StringData*.
  struct EqualityTester {
    bool operator()(const char* a, const StringData* b) const {
      // AtomicHashArray magic keys are < 0; valid pointers are > 0 and aligned.
      return reinterpret_cast<intptr_t>(a) > 0 &&
        wordsame(a, b->data(), b->size() + 1);
    }
  };
  struct Hasher {
    size_t operator()(const StringData* a) const { return a->hash(); }
  };

  // Allocates keys on successful insertion. They are never erased/freed.
  struct KeyConverter {
    const char* operator()(const StringData* sd) const {
      if (sd->isStatic()) return sd->data();
      auto const nbytes = sd->size() + 1;
      auto const dst = malloc_huge(nbytes);
      assert((reinterpret_cast<uintptr_t>(dst) & 7) == 0);
      memcpy(dst, sd->data(), nbytes);
      return reinterpret_cast<const char*>(dst);
    }
  };
  using HotMap = folly::AtomicHashArray<const char*, std::atomic<HotValueRaw>,
                                        Hasher, EqualityTester,
                                        HugeAllocator<char>,
                                        folly::AtomicHashArrayLinearProbeFcn,
                                        KeyConverter>;

  static bool supportedKind(const APCHandle* h) {
    return h->isSingletonKind() || h->isUncounted();
  }

  static HotValueRaw makeRawValue(APCHandle* h) {
    assert(h != nullptr && supportedKind(h));
    HotValue v = [&] {
      switch (h->kind()) {
        case APCKind::UncountedArray:
          return HotValue{APCTypedValue::fromHandle(h)->getArrayData()};
        case APCKind::UncountedVec:
          return HotValue{APCTypedValue::fromHandle(h)->getVecData()};
        case APCKind::UncountedDict:
          return HotValue{APCTypedValue::fromHandle(h)->getDictData()};
        case APCKind::UncountedKeyset:
          return HotValue{APCTypedValue::fromHandle(h)->getKeysetData()};
        default:
          return HotValue{h};
      }
    }();
    return v.toOpaque();
  }

  static bool rawValueToLocal(HotValueRaw vraw, Variant& value) {
    HotValue v = HotValue::fromOpaque(vraw);
    if (ArrayData* ad = v.right()) {
      value = Variant{
        ad, ad->toPersistentDataType(), Variant::PersistentArrInit{}
      };
      return true;
    }
    if (APCHandle* h = v.left()) {
      value = h->toLocal();
      return true;
    }
    return false;
  }

  bool clearValueIdx(Idx idx);

  // True iff the given key string is eligible to be considered hot.
  bool maybeHot(const char* key) const {
    for (auto p : apcExtension::HotPrefix) {
      if (strncmp(key, p.data(), p.size()) == 0) {
        return true;
      }
    }
    return false;
  }

  // True if the key *might* be eligible (with false positives).
  bool maybeHotFast(const StringData* sd) const {
    // TODO(11227362): Avoid size check if we can assume zero-padding.
    return sd->size() >= sizeof(uint64_t) &&
      (readPrefix64(sd->data()) & m_hotPrefixMask) == 0;
  }

  // Return the bits that were *not* set in any of the prefixes' first 8 chars.
  static uint64_t computeFastPrefixMask(std::vector<std::string> prefs) {
    uint64_t any = 0;
    for (auto& p : prefs) {
      while (p.size() < sizeof(uint64_t)) p += '\xff';
      any |= readPrefix64(p.data());
    }
    return ~any;
  }

  static uint64_t readPrefix64(const char* s) {
    uint64_t result;
    memcpy(&result, s, sizeof(result));
    return result;
  }

  HotMap::SmartPtr m_hotMap;
  uint64_t m_hotPrefixMask{~0ull};
  std::atomic<bool> m_isFull{false};
};
HotCache s_hotCache;

void HotCache::initialize() {
  if (!m_hotMap) {
    HotMap::Config config;
    config.maxLoadFactor = apcExtension::HotLoadFactor;
    /*
     * Ensure we detect a fully loaded map in time. (AHA's default of
     * 1000 * hundreds of homogeneous writers => O(100,000) entries before
     * first real check.)
     *
     * TODO(11227990): Use maxSize/(actual thread count), or make AHA dynamic.
     */
    config.entryCountThreadCacheSize = 100;
    auto const maxSize = apcExtension::HotSize;
    m_hotMap = HotMap::create(maxSize, config);

    auto const& prefs = apcExtension::HotPrefix;
    m_hotPrefixMask = computeFastPrefixMask(prefs);
  }
}

bool HotCache::hasValue(const StringData* key) const {
  if (!maybeHotFast(key)) return false;
  HotMap::const_iterator it = m_hotMap->find(key);
  return it != m_hotMap->end() &&
    !HotValue::fromOpaque(it->second.load(std::memory_order_relaxed)).isNull();
}

bool HotCache::get(const StringData* key, Variant& value, Idx& idx) const {
  if (!maybeHotFast(key)) {
    idx = StoreValue::kHotCacheKnownIneligible;
    return false;
  }
  auto it = m_hotMap->find(key);
  if (it == m_hotMap->end()) {
    idx = StoreValue::kHotCacheUnknown;
    return false;
  }
  if (rawValueToLocal(it->second.load(std::memory_order_relaxed), value)) {
    return true;
  }
  idx = it.getIndex();
  return false;
}

bool HotCache::store(Idx idx, const StringData* key,
                     APCHandle* svar, const StoreValue* sval) {
  if (idx == StoreValue::kHotCacheKnownIneligible) return false;
  if (!svar || !supportedKind(svar)) return false;
  auto raw = makeRawValue(svar);
  if (idx == StoreValue::kHotCacheUnknown) {
    if (!maybeHot(key->data())) return false;
    if (m_isFull.load(std::memory_order_relaxed)) return false;
    auto p = m_hotMap->emplace(key, raw);
    if (p.first == m_hotMap->end()) {
      m_isFull.store(true, std::memory_order_relaxed);
      return false;
    }
    idx = p.first.getIndex();
  }
  assert(idx >= 0);
  sval->hotIndex.store(idx, std::memory_order_relaxed);
  m_hotMap->findAt(idx)->second.store(raw, std::memory_order_relaxed);
  return true;
}

bool HotCache::clearValueIdx(Idx idx) {
  if (idx == StoreValue::kHotCacheUnknown) return false;
  assert(idx >= 0);
  auto it = m_hotMap->findAt(idx);
  it->second.store(HotValue(nullptr).toOpaque(), std::memory_order_relaxed);
  return true;
}

//////////////////////////////////////////////////////////////////////

bool ConcurrentTableSharedStore::clear() {
  SharedMutex::WriteHolder l(m_lock);
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end();
       ++iter) {
    s_hotCache.clearValue(iter->second);
    iter->second.data().match(
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

bool ConcurrentTableSharedStore::eraseKey(const String& key) {
  assert(!key.isNull());
  return eraseImpl(tagStringData(key.get()), false, 0, nullptr);
}

/*
 * The Map::accessor here establishes a write lock, which means that other
 * threads, protected by read locks through Map::const_accessor, will not
 * read erased values from APC.
 *
 * The ReadLock here is to sync with clear(), which only has a WriteLock,
 * not a specific accessor.
 */
bool ConcurrentTableSharedStore::eraseImpl(const char* key,
                                           bool expired,
                                           int64_t oldestLive,
                                           ExpMap::accessor* expAcc) {
  assert(key);

  SharedMutex::ReadHolder l(m_lock);
  Map::accessor acc;
  if (!m_vars.find(acc, key)) {
    return false;
  }
  if (expired && !acc->second.expired()) {
    return false;
  }

  auto& storeVal = acc->second;
  bool wasCached = s_hotCache.clearValue(storeVal);

  if (auto const var = storeVal.data().left()) {
    APCStats::getAPCStats().removeAPCValue(storeVal.dataSize, var,
                                           storeVal.expire == 0, expired);
    /*
     * As an optimization, we eagerly delete uncounted values that expired
     * long ago. But HotCache does not check expiration on every 'get', so
     * any values previously cached there must take the usual treadmill route.
     */
    if (expired && storeVal.expire < oldestLive &&
        var->isUncounted() && !wasCached) {
      APCTypedValue::fromHandle(var)->deleteUncounted();
    } else {
      var->unreferenceRoot(storeVal.dataSize);
    }
  } else {
    assert(!expired);  // primed keys never say true to expired()
  }

  FTRACE(2, "Remove {} {}\n", acc->first, show(acc->second));
  APCStats::getAPCStats().removeKey(strlen(acc->first));
  const void* vpkey = acc->first;
  /*
   * Note that we have a delicate situation here; purgeExpired obtains
   * the ExpMap accessor, and then the Map accessor, while eraseImpl
   * (called from other sites) apparently obtains the Map accessor
   * followed by the ExpMap accessor.
   *
   * This does not result in deadlock, because the Map accessor is
   * released by m_vars.erase. But we need this ordering to ensure
   * that as long as you hold an accessor to m_expMap, its key
   * converted to a char* will be a valid c-string.
   */
  m_vars.erase(acc);
  if (expAcc) {
    m_expMap.erase(*expAcc);
  } else {
    /*
     * Note that we can't just call m_expMap.erase(intptr_t(vpkey))
     * here. That will remove the element and not block, even if
     * we hold an ExpMap::accessor to the element in another thread,
     * which would allow us to proceed and free vpkey.
     */
    ExpMap::accessor eAcc;
    if (m_expMap.find(eAcc, intptr_t(vpkey))) {
      m_expMap.erase(eAcc);
    }
  }
  free(const_cast<void*>(vpkey));
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
    if (UNLIKELY(tmp.first ==
                 intptr_t(apcExtension::FileStorageFlagKey.c_str()))) {
      adviseOut();
      tmp.second = time(nullptr) + apcExtension::FileStorageAdviseOutPeriod;
      m_expQueue.push(tmp);
      continue;
    }
    ExpMap::accessor acc;
    if (m_expMap.find(acc, tmp.first)) {
      eraseImpl((char*)tmp.first, true, oldestLive, &acc);
    }
    ++i;
  }
  FTRACE(1, "Expired {} entries", i);
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
  auto const handle = sval.data().left();
  if (handle == svar && handle->kind() == APCKind::SerializedObject) {
    sval.setHandle(converted);
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
  auto const sAddr = sval->data().right();
  assert(sAddr != nullptr);
  /*
    This method is special, since another thread T may concurrently
    attempt to 'get' this entry while we're unserializing it. If T
    observes sval->data with a cleared tag, it will proceed without
    any further locking (it only has a const_accessor).

    Thus, this method must ensure that the tag of sval->data is cleared
    only *after* sval is in a fully consistent unserialized state.
   */

  try {
    auto const sType =
      apcExtension::EnableApcSerialize
        ? VariableUnserializer::Type::APCSerialize
        : VariableUnserializer::Type::Serialize;

    VariableUnserializer vu(sAddr, sval->getSerializedSize(), sType);
    if (sval->readOnly) vu.setReadOnly();
    Variant v = vu.unserialize();
    auto const pair = APCHandle::Create(v, sval->isSerializedObj(),
      APCHandleLevel::Outer, false);
    sval->dataSize = pair.size;
    sval->setHandle(pair.handle);  // Publish unserialized value (see 'get').
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

bool ConcurrentTableSharedStore::get(const String& keyStr, Variant& value) {
  FTRACE(3, "Get {}\n", keyStr.get()->data());
  HotCache::Idx hotIdx;
  if (s_hotCache.get(keyStr.get(), value, hotIdx)) return true;
  const StoreValue *sval;
  APCHandle *svar = nullptr;
  SharedMutex::ReadHolder l(m_lock);
  bool expired = false;
  bool promoteObj = false;
  auto tag = tagStringData(keyStr.get());
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, tag)) {
      return false;
    }
    sval = &acc->second;
    if (sval->expired()) {
      // Because it only has a read lock on the data, deletion from
      // expiration has to happen after the lock is released
      expired = true;
    } else {
      if (auto const handle = sval->data().left()) {
        svar = handle;
      } else {
        std::lock_guard<SmallLock> sval_lock(sval->lock);

        if (auto const handle = sval->data().left()) {
          svar = handle;
        } else {
          /*
           * Note that unserialize can run arbitrary php code via a __wakeup
           * routine, which could try to access this same key, and we're
           * holding various locks here.  This is only for promoting primed
           * values to in-memory values, so it's basically not a real
           * problem, but ... :)
           */
          svar = unserialize(keyStr, const_cast<StoreValue*>(sval));
          if (!svar) return false;
        }
      }
      assert(sval->data().left() == svar);
      APCKind kind = sval->getKind();
      if (apcExtension::AllowObj &&
          (kind == APCKind::SerializedObject ||
           kind == APCKind::SharedObject ||
           kind == APCKind::SharedCollection) &&
          !svar->objAttempted()) {
        // Hold ref here for later promoting the object
        svar->referenceNonRoot();
        promoteObj = true;
      }
      value = sval->toLocal();
      if (!promoteObj) {
        /*
         * Successful slow-case lookup => add value to cache (if key and kind
         * are eligible and there is still room for it). Another thread may be
         * updating the same key concurrently, but ConcurrentTableSharedStore's
         * per-entry lock ensures it will agree on the value.
         */
        s_hotCache.store(hotIdx, keyStr.get(), svar, sval);
      }
    }
  }
  if (expired) {
    eraseImpl(tag, true,
              apcExtension::UseUncounted ?
              HPHP::Treadmill::getOldestStartTime() : 0, nullptr);
    return false;
  }

  if (promoteObj)  {
    handlePromoteObj(keyStr, svar, value);
    // release the extra ref
    svar->unreferenceNonRoot();
  }
  return true;
}

int64_t ConcurrentTableSharedStore::inc(const String& key, int64_t step,
                                        bool& found) {
  found = false;
  SharedMutex::ReadHolder l(m_lock);

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
  auto const oldHandle = sval.data().left();
  if (oldHandle == nullptr) return 0;
  if (oldHandle->kind() != APCKind::Int &&
      oldHandle->kind() != APCKind::Double) {
    return 0;
  }

  // Currently a no-op, since HotCache doesn't store int/double.
  assert(sval.hotIndex == StoreValue::kHotCacheUnknown);
  s_hotCache.clearValue(sval);

  auto const ret = oldHandle->toLocal().toInt64() + step;
  auto const pair = APCHandle::Create(Variant(ret), false,
    APCHandleLevel::Outer, false);
  APCStats::getAPCStats().updateAPCValue(pair.handle, pair.size,
                                         oldHandle, sval.dataSize,
                                         sval.expire == 0, false);
  oldHandle->unreferenceRoot(sval.dataSize);
  sval.setHandle(pair.handle);
  sval.dataSize = pair.size;
  found = true;
  return ret;
}

bool ConcurrentTableSharedStore::cas(const String& key, int64_t old,
                                     int64_t val) {
  SharedMutex::ReadHolder l(m_lock);

  Map::accessor acc;
  if (!m_vars.find(acc, tagStringData(key.get()))) {
    return false;
  }

  auto& sval = acc->second;
  if (sval.expired()) return false;
  s_hotCache.clearValue(sval);
  auto const oldHandle =
    sval.data().match([&](APCHandle* h) { return h; },
                      [&](char* /*file*/) { return unserialize(key, &sval); });
  if (!oldHandle || oldHandle->toLocal().toInt64() != old) {
    return false;
  }

  auto const pair = APCHandle::Create(Variant(val), false,
    APCHandleLevel::Outer, false);
  APCStats::getAPCStats().updateAPCValue(pair.handle, pair.size,
                                         oldHandle, sval.dataSize,
                                         sval.expire == 0, false);
  oldHandle->unreferenceRoot(sval.dataSize);
  sval.setHandle(pair.handle);
  sval.dataSize = pair.size;
  return true;
}

bool ConcurrentTableSharedStore::exists(const String& keyStr) {
  if (s_hotCache.hasValue(keyStr.get())) return true;
  const StoreValue *sval;
  SharedMutex::ReadHolder l(m_lock);
  bool expired = false;
  auto tag = tagStringData(keyStr.get());
  {
    Map::const_accessor acc;
    if (!m_vars.find(acc, tag)) {
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
    eraseImpl(tag, true,
              apcExtension::UseUncounted ?
              HPHP::Treadmill::getOldestStartTime() : 0, nullptr);
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
  auto svar = APCHandle::Create(value, false, APCHandleLevel::Outer, false);
  auto keyLen = key.size();
  char* const kcp = strdup(key.data());
  {
  SharedMutex::ReadHolder l(m_lock);
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
      if (!overwrite && !sval->expired()) {
        svar.handle->unreferenceRoot(svar.size);
        return false;
      }
      /*
       * Simply clear the entry --- if it's truly hot, a successful non-cache
       * 'get' will soon update the entry with the new value.
       */
      s_hotCache.clearValue(*sval);
      sval->data().match(
        [&] (APCHandle* handle) {
          current = handle;
          // If ApcTTLLimit is set, then only primed keys can have
          // expire == 0.
          overwritePrime = sval->expire == 0;
        },
        [&] (char*) {
          // Was inFile, but won't be anymore.
          sval->clearData();
          sval->dataSize = 0;
          overwritePrime = true;
        }
      );
      FTRACE(2, "Update {} {}\n", acc->first, show(acc->second));
    } else {
      FTRACE(2, "Add {} {}\n", acc->first, show(acc->second));
      APCStats::getAPCStats().addKey(keyLen);
    }

    int64_t adjustedTtl = adjust_ttl(ttl, overwritePrime || !limit_ttl);
    if (adjustedTtl > apcExtension::TTLMaxFinite ||
        check_noTTL(key.data(), key.size())) {
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
    if (expiry) {
      auto ikey = intptr_t(acc->first);
      if (m_expMap.insert({ ikey, 0 })) {
        m_expQueue.push({ ikey, expiry });
      }
    }
  }
  }  // m_lock
  if (apcExtension::ExpireOnSets) {
    purgeExpired();
  }

  return true;
}

void ConcurrentTableSharedStore::prime(std::vector<KeyValuePair>&& vars) {
  SharedMutex::ReadHolder l(m_lock);
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
      sval.data().match(
        [&] (APCHandle* handle) {
          handle->unreferenceRoot(sval.dataSize);
        },
        [&] (char*) {}
      );
      sval.clearData();
      sval.dataSize = 0;
      sval.expire   = 0;
    }

    acc->second.readOnly = apcExtension::EnableConstLoad && item.readOnly;
    if (item.inMem()) {
      APCStats::getAPCStats().addAPCValue(item.value, item.sSize, true);
      acc->second.set(item.value, 0);
      acc->second.dataSize = item.sSize;
    } else {
      acc->second.tagged_data.store(item.sAddr, std::memory_order_release);
      acc->second.dataSize = item.sSize;
      APCStats::getAPCStats().addInFileValue(std::abs(acc->second.dataSize));
    }
    FTRACE(2, "Primed key {} {}\n", acc->first, show(acc->second));
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
  auto pair = APCHandle::Create(v, serialized, APCHandleLevel::Outer, false);
  item.value = pair.handle;
  item.sSize = pair.size;
  return true;
}

bool ConcurrentTableSharedStore::constructPrime(const Variant& v,
                                                KeyValuePair& item) {
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid &&
      (isRefcountedType(v.getType()))) {
    // Only do the storage for ref-counted type
    String s = apc_serialize(v);
    char *sAddr = s_apc_file_storage.put(s.data(), s.size());
    if (sAddr) {
      item.sAddr = sAddr;
      item.sSize = s.size();
      return false;
    }
  }
  auto pair = APCHandle::Create(v, false, APCHandleLevel::Outer, false);
  item.value = pair.handle;
  item.sSize = pair.size;
  return true;
}

void ConcurrentTableSharedStore::primeDone() {
  s_hotCache.initialize();
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid) {
    s_apc_file_storage.seal();
    s_apc_file_storage.hashCheck();
  }
  // Schedule the adviseOut instead of doing it immediately, so that the
  // initial accesses to the primed keys are not too bad. Still, for
  // the keys in file, a deserialization from memory is required on first
  // access.
  ExpirationPair p(intptr_t(apcExtension::FileStorageFlagKey.c_str()),
                   time(nullptr) + apcExtension::FileStorageAdviseOutPeriod);
  m_expQueue.push(p);

  for (auto iter = apcExtension::CompletionKeys.begin();
       iter != apcExtension::CompletionKeys.end(); ++iter) {
    Map::accessor acc;
    auto const copy = strdup(iter->c_str());
    if (!m_vars.insert(acc, copy)) {
      free(copy);
      return;
    }
    auto const pair =
      APCHandle::Create(Variant(1), false, APCHandleLevel::Outer, false);
    acc->second.set(pair.handle, 0);
    acc->second.dataSize = pair.size;
    APCStats::getAPCStats().addAPCValue(pair.handle, pair.size, true);
  }
}

bool ConcurrentTableSharedStore::primeFromSnapshot(const char* filename) {
  m_snapshotLoader = std::make_unique<SnapshotLoader>();
  if (!m_snapshotLoader->tryInitializeFromFile(filename)) {
    m_snapshotLoader.reset();
    return false;
  }
  // TODO(9755815): APCFileStorage is redundant when using snapshot;
  // disable it at module loading time in this case.
  Logger::Info("Loading from snapshot file %s", filename);
  m_snapshotLoader->load(*this);
  primeDone();
  return true;
}

void ConcurrentTableSharedStore::adviseOut() {
  if (s_apc_file_storage.getState() !=
      APCFileStorage::StorageState::Invalid) {
    s_apc_file_storage.adviseOut();
  }
  if (m_snapshotLoader.get()) {
    m_snapshotLoader->adviseOut();
  }
}

///////////////////////////////////////////////////////////////////////////////
// debugging and info/stats support

EntryInfo ConcurrentTableSharedStore::makeEntryInfo(const char* key,
                                                    StoreValue* sval,
                                                    int64_t curr_time) {
  int32_t size;
  auto type = EntryInfo::Type::Unknown;
  auto const inMem = sval->data().match(
      [&](APCHandle* handle) {
        size = sval->dataSize;
        type = EntryInfo::getAPCType(handle);
        return true;
      },
      [&](char*) {
        size = sval->getSerializedSize();
        return false;
      });

  int64_t ttl = 0;
  if (inMem && sval->expire) {
    ttl = sval->expire - curr_time;
    if (ttl == 0) ttl = 1; // don't want to confuse with primed keys
  }

  return EntryInfo(key, inMem, size, ttl, type, sval->c_time, sval->mtime);
}

std::vector<EntryInfo> ConcurrentTableSharedStore::getEntriesInfo() {
  auto entries = std::vector<EntryInfo>{};

  int64_t curr_time = time(nullptr);
  entries.reserve(m_vars.size() + 1000);

  {
    SharedMutex::WriteHolder l(m_lock);
    for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
      entries.push_back(
          makeEntryInfo(iter->first, &iter->second, curr_time));
    }
  }
  std::sort(entries.begin(), entries.end(),
            [] (const EntryInfo& e1, const EntryInfo& e2) {
              return e1.key < e2.key; });
  return entries;
}

void ConcurrentTableSharedStore::dumpKeyAndValue(std::ostream & out) {
  SharedMutex::WriteHolder l(m_lock);
  out << "Total " << m_vars.size() << std::endl;
  for (Map::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter) {
    const char *key = iter->first;
    out << key;
    out << " #### ";
    const StoreValue *sval = &iter->second;
    if (!sval->expired()) {
      VariableSerializer vs(VariableSerializer::Type::Serialize);

      auto const value = sval->data().match(
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

static void dumpEntriesInfo(std::vector<EntryInfo> entries, std::ostream& out) {
  out << "key inmem size ttl type\n";
  for (auto entry : entries) {
    out << entry.key << " "
        << static_cast<int32_t>(entry.inMem) << " "
        << entry.size << " "
        << entry.ttl << " "
        << static_cast<int32_t>(entry.type) << '\n';
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
    dumpEntriesInfo(getEntriesInfo(), out);
    break;
  }

  Logger::Info("dumping apc done");
}

void ConcurrentTableSharedStore::dumpRandomKeys(std::ostream& out,
                                                uint32_t count) {
  dumpEntriesInfo(sampleEntriesInfo(count), out);
}

std::vector<EntryInfo>
ConcurrentTableSharedStore::sampleEntriesInfo(uint32_t count) {
  SharedMutex::WriteHolder l(m_lock);
  if (m_vars.empty()) {
    Logger::Warning("No APC entries sampled (empty store)");
    return std::vector<EntryInfo>();
  }
  std::vector<EntryInfo> samples;
  for (; count > 0; count--) {
    if (!m_vars.getRandomAPCEntry(samples)) {
      Logger::Warning("No APC entries sampled (incompatible TBB library)");
      return std::vector<EntryInfo>();
    }
  }
  return samples;
}

template<typename Key, typename T, typename HashCompare>
bool ConcurrentTableSharedStore
      ::APCMap<Key,T,HashCompare>
      ::getRandomAPCEntry(std::vector<EntryInfo>& entries) {
  assert(!this->empty());
#if TBB_VERSION_MAJOR >= 4
  auto current = this->range();
  for (auto rnd = rand(); rnd > 0 && current.is_divisible(); rnd >>= 1) {
    // Split the range 'current' into two halves: 'current' and 'otherHalf'.
    decltype(current) otherHalf(current, tbb::split());
    // Randomly choose which half to keep.
    if (rnd & 1) {
      current = otherHalf;
    }
  }
  auto apcPair = *current.begin();
  int64_t curr_time = time(nullptr);
  entries.push_back(makeEntryInfo(apcPair.first, &apcPair.second, curr_time));
  return true;
#else
  return false;
#endif
}

//////////////////////////////////////////////////////////////////////
}
