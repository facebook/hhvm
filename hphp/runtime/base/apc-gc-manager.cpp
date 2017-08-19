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
#include "hphp/runtime/base/apc-gc-manager.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/util/lock.h"

namespace HPHP {

TRACE_SET_MOD(apc);

//////////////////////////////////////////////////////////////////////

void APCGCManager::registerAllocation(void* start, void* end, APCHandle* root) {
  assertx(RuntimeOption::EvalGCForAPC);
  if (RuntimeOption::ServerExecutionMode()) {
    return; // Doesn't support server mode yet
  }

  insert(std::pair<void*, void*>(start,end), root);
}

void APCGCManager::registerPendingDeletion(APCHandle* root, const size_t size) {
  assertx(RuntimeOption::EvalGCForAPC);
  if (RuntimeOption::ServerExecutionMode()) {
    return; // Doesn't support server mode yet
  }
  if (RuntimeOption::ServerExecutionMode()) {
    // Root should be visible by default to implement asynchronous mark-sweep
    // But we don't need to do this in script mode
    WriteLock l3(visibleFromHeapLock);
    visibleFromHeap.insert(root);
  }
  // Recursively register all allocations belong to this root handle
  APCTypedValue::fromHandle(root)->registerUncountedAllocations();
  // Add root APCHandle into to-free list
  {
    WriteLock l1(candidateListLock);
    pendingSize.fetch_add(size, std::memory_order_relaxed);
    assertx(candidateList.count(root) == 0);
    candidateList.emplace(root, size);
  }
  FTRACE(1, "APCGCManager: increase used memory:{} by size:{}\n",
                                                pendingSize.load(), size);
  /* TODO Task #20074509: API for global GC invoke */
  if (pendingSize.load() > someBar()) {
    // Bar is -vEval.GCForAPCBar, 10^9 by default
    invokeGlobalGC();
    FTRACE(2, "APCGCManager: GC flag set up!\n");
  }
}

bool APCGCManager::excessedGCTriggerBar() {
  return pendingSize.load() > someBar();
}

void APCGCManager::sweep() {
  assertx(RuntimeOption::EvalGCForAPC);
  if (RuntimeOption::ServerExecutionMode()) {
    return; // Doesn't support server mode yet
  }
  FTRACE(1, "Sweep! Pending size:{}\n", apcgcPendingSize());
  WriteLock l1(candidateListLock);
  WriteLock l2(rootMapLock);
  WriteLock l3(visibleFromHeapLock);

  for (auto it = rootMap.begin() ; it != rootMap.end() ;) {
    auto cur = it++;
    APCHandle* root = cur->second;
    // If root is not visible from any heap
    if (visibleFromHeap.count(root) == 0) {
      auto candidate = candidateList.find(root);
      // If root haven't been freed before, free it
      if (candidate != candidateList.end()) {
        pendingSize.fetch_add(-candidate->second, std::memory_order_relaxed);
        // Remove the root from list, so we won't free it twice
        candidateList.erase(candidate);
        FTRACE(4, "Sweep! root:{}\n", (void*)root);
        // Free the root
        APCTypedValue::fromHandle(root)->deleteUncounted();
      }
      // Remove {allocation, root}
      rootMap.erase(cur);
    }
  }
  visibleFromHeap.clear();
}

void APCGCManager::invokeGlobalGC() {
  /* TODO Task #20074509: API for global GC invoke */
  // For now, only schedule a GC for this thread
  setSurpriseFlag(PendingGCFlag);
}

bool APCGCManager::mark(const void* ptr) {
  assertx(RuntimeOption::EvalGCForAPC);
  if (RuntimeOption::ServerExecutionMode()) {
    return false; // Doesn't support server mode yet
  }

  if (ptr == nullptr) return false;
  auto root = getRootAPCHandle(ptr); // Required l2 here
  if (root != nullptr) {
    // Require lock only when root has been found
    WriteLock l3(visibleFromHeapLock); // Require l3 after require l2
    visibleFromHeap.insert(root);
    FTRACE(4, "Mark root {} for ptr: {}\n", (const void*)root, ptr);
    return true;
  } else {
    FTRACE(4, "Root for ptr: {} not found\n", ptr);
    return false;
  }
}

void APCGCManager::freeAPCHandles(const std::vector<APCHandle*>& v) {
  assertx(RuntimeOption::EvalGCForAPC);
  if (RuntimeOption::ServerExecutionMode()) {
    // Doesn't support server mode yet
    // But it still needs to finish the Treadmill's job because Treadmill
    // won't free the handles now
    for (auto handle : v) {
      APCTypedValue::fromHandle(handle)->deleteUncounted();
    }
    return;
  }
  // This lock will be blocked by current sweeping
  WriteLock l1(candidateListLock);
  FTRACE(4, "Treadmill asks APCGCManager to free! size: {}\n",v.size());
  for (auto handle : v) {
    auto it = candidateList.find(handle);
    // If root haven't been freed before, free it
    if (it != candidateList.end()) {
      pendingSize.fetch_add(-it->second, std::memory_order_relaxed);
      // Remove the root from list, so we won't free it twice
      candidateList.erase(it);
      FTRACE(4, "Sweep! root:{}\n", (void*)handle);
      APCTypedValue::fromHandle(handle)->deleteUncounted();
    }
  }
}

void APCGCManager::insert(Allocation r, APCHandle* root) {
  WriteLock l2(rootMapLock);
  FTRACE(4, "Insert {} {} with root {}\n",
                            (void*)r.first, (void*)r.second, (void*)root);
  rootMap.emplace(r, root);
}

/*
 * Find a [stAddress,edAddress) contains ptr, return the root
*/
APCHandle* APCGCManager::getRootAPCHandle(const void* ptr) {
  ReadLock l2(rootMapLock);
  auto it = rootMap.upper_bound(Allocation(ptr, ptr));
  // it and (it-1) both possibly contain ptr
  if (ptr >= it->first.first && ptr < it->first.second) return it->second;
  if (it == rootMap.begin()) return nullptr;
  --it;
  return (ptr >= it->first.first && ptr < it->first.second)
    ? it->second
    : nullptr;
}

APCGCManager& APCGCManager::getInstance() {
  static APCGCManager mm;
  return mm;
}
//////////////////////////////////////////////////////////////////////
// All gc-apc related traverse functions here

/*
 * Recursively register {allocation, rootAPCHandle} with APCGCManager
 * according to the type of tv
*/
ALWAYS_INLINE
void RegisterUncountedTvAllocations(TypedValue& tv, APCHandle* rootAPCHandle) {
  if (isStringType(tv.m_type)) {
    assert(!tv.m_data.pstr->isRefCounted());
    if (tv.m_data.pstr->isUncounted()) {
      tv.m_data.pstr->registerUncountedAllocation(rootAPCHandle);
    }
    return;
  }
  if (isArrayLikeType(tv.m_type)) {
    auto arr = tv.m_data.parr;
    assert(!arr->isRefCounted());
    if (!arr->isStatic()) {
      if (arr->hasPackedLayout()) {
        PackedArray::RegisterUncountedAllocations(arr, rootAPCHandle);
      } else {
        MixedArray::RegisterUncountedAllocations(arr, rootAPCHandle);
      }
    }
    return;
  }
  assertx(!isRefcountedType(tv.m_type));
}

/*
 * Treadmill call this method when apc_delete()/apc_store() is trying to
 * delete/overwrite an APC uncounted data.
 * According to the type, StringData/ArrayData::registerUncountedAllocations()
 * will be called to recursively register all {allocation, root}
 * with APCGCManager
*/
void APCTypedValue::registerUncountedAllocations() {
  assert(m_handle.isUncounted());
  assert(RuntimeOption::EvalGCForAPC);
  auto kind = m_handle.kind();

  assert(kind == APCKind::UncountedString ||
         kind == APCKind::UncountedArray ||
         kind == APCKind::UncountedVec ||
         kind == APCKind::UncountedDict ||
         kind == APCKind::UncountedKeyset);
   if (kind == APCKind::UncountedString) {
     m_data.str->registerUncountedAllocation(&m_handle);
   } else if (kind == APCKind::UncountedArray) {
     assert(m_data.arr->isPHPArray());
     if (m_data.arr->hasPackedLayout()) {
       auto arr = m_data.arr;
       PackedArray::RegisterUncountedAllocations(arr, &m_handle);
       return;
     } else {
       auto arr = m_data.arr;
       MixedArray::RegisterUncountedAllocations(arr, &m_handle);
       return;
     }
   } else if (kind == APCKind::UncountedVec) {
     auto vec = m_data.vec;
     assert(vec->isVecArray());
     PackedArray::RegisterUncountedAllocations(vec, &m_handle);
     return;
   } else if (kind == APCKind::UncountedDict) {
     auto dict = m_data.dict;
     assert(dict->isDict());
     MixedArray::RegisterUncountedAllocations(dict, &m_handle);
     return;
   } else if (kind == APCKind::UncountedKeyset) {
     auto keyset = m_data.keyset;
     assert(keyset->isKeyset());
     SetArray::RegisterUncountedAllocations(keyset, &m_handle);
     return;
   }
}


// Register {allocation, rootAPCHandle} with APCGCManager
void StringData::registerUncountedAllocation(APCHandle* rootAPCHandle) {
  assert(checkSane() && isUncounted());
  assert(isFlat());
  assert(RuntimeOption::EvalGCForAPC);
  // [this, this + 1) doesn't give us address of the string
  // But we assume reference point only at the header of a StringData
  APCGCManager::getInstance().registerAllocation(this,
                                        (char*)this + this->heapSize(),
                                        rootAPCHandle);
}

/*
 * Recursively register {allocation, rootAPCHandle} with APCGCManager
 * for all allocations in ad
 */
void PackedArray::RegisterUncountedAllocations(ArrayData* ad,
                                          APCHandle* rootAPCHandle) {
  assert(checkInvariants(ad));
  assert(ad->isUncounted());

  auto const data = packedData(ad);
  auto const stop = data + ad->m_size;
  for (auto ptr = data; ptr != stop; ++ptr) {
    RegisterUncountedTvAllocations(*ptr, rootAPCHandle);
  }
  assert(!has_strong_iterator(ad));
  assert(RuntimeOption::EvalGCForAPC);
  APCGCManager::getInstance().registerAllocation(ad,
                                        (char*)ad + PackedArray::heapSize(ad),
                                        rootAPCHandle);
}

/*
 * Recursively register {allocation, rootAPCHandle} with APCGCManager
 * for all allocations in ad
 */
void MixedArray::RegisterUncountedAllocations(ArrayData* in,
                                          APCHandle* rootAPCHandle) {
  auto const ad = asMixed(in);
  assert(ad->isUncounted());
  if (!ad->isZombie()) {
    auto const data = ad->data();
    auto const stop = data + ad->m_used;

    for (auto ptr = data; ptr != stop; ++ptr) {
      if (isTombstone(ptr->data.m_type)) continue;
      if (ptr->hasStrKey()) {
        assert(!ptr->skey->isRefCounted());
        if (ptr->skey->isUncounted()) {
          ptr->skey->registerUncountedAllocation(rootAPCHandle);
        }
      }
      RegisterUncountedTvAllocations(ptr->data, rootAPCHandle);
    }
    assert(!has_strong_iterator(ad));
  }
  assert(RuntimeOption::EvalGCForAPC);
  APCGCManager::getInstance().registerAllocation(ad,
                                        (char*)ad + ad->heapSize(),
                                        rootAPCHandle);
}

/*
 * Recursively register {allocation, rootAPCHandle} with APCGCManager
 * for all allocations in ad
 */
void SetArray::RegisterUncountedAllocations(ArrayData* in,
                                        APCHandle* rootAPCHandle) {
  assert(in->isUncounted());
  auto const ad = asSet(in);

  if (!ad->isZombie()) {
    auto const elms = ad->data();
    auto const used = ad->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto& elm = elms[i];
      if (UNLIKELY(elm.isTombstone())) continue;
      assert(!elm.isEmpty());
      if (elm.hasStrKey()) {
        auto const skey = elm.strKey();
        assert(!skey->isRefCounted());
        if (skey->isUncounted()) {
          skey->registerUncountedAllocation(rootAPCHandle);
        }
      }
    }
    assert(!has_strong_iterator(ad));
  }
  assert(RuntimeOption::EvalGCForAPC);
  APCGCManager::getInstance().registerAllocation(ad,
                                        (char*)ad + ad->heapSize(),
                                        rootAPCHandle);
}
//////////////////////////////////////////////////////////////////////
}
