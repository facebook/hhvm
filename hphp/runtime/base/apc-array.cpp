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
#include "hphp/runtime/base/apc-array.h"

#include <folly/Bits.h>

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

namespace {

size_t getMemSize(DataWalker::PointerMap* seenArrs) {
  always_assert(!use_jemalloc);
  size_t total = 0;
  for (auto kv : *seenArrs) {
    total += ::HPHP::getMemSize(static_cast<ArrayData*>(kv.first), false);
  }
  return total;
}

}

///////////////////////////////////////////////////////////////////////////////

template <typename A, typename B>
ALWAYS_INLINE
APCHandle::Pair
APCArray::MakeSharedImpl(ArrayData* arr, APCHandleLevel level,
                         A shared, B serialized) {
  if (level == APCHandleLevel::Outer) {
    auto const seenArrs = apcExtension::ShareUncounted ?
      req::make_unique<DataWalker::PointerMap>() : nullptr;

    // We only need to call traverseData() on the top-level array.
    DataWalker walker(DataWalker::LookupFeature::DetectNonPersistable);
    DataWalker::DataFeature features =
      walker.traverseData(arr, seenArrs.get());
    if (features.isCircular) {
      auto const s = apc_serialize(Variant{arr});
      return serialized(s.get());
    }

    if (apcExtension::UseUncounted && !features.hasNonPersistable) {
      auto const base_size = use_jemalloc ?
        tl_heap->getAllocated() - tl_heap->getDeallocated() :
        seenArrs.get() ? getMemSize(seenArrs.get()) :
        ::HPHP::getMemSize(arr, true);
      auto const uncounted_arr = MakeUncountedArray(arr, seenArrs.get());
      auto size = use_jemalloc ?
        tl_heap->getAllocated() - tl_heap->getDeallocated() - base_size :
        base_size;
      // FIXME: the MemoryManager logic is not reliable, especially when
      // Bespoke logic in MakeUncountedArray() may even free stuff
      if (static_cast<int64_t>(size) <= 0) size = 1;
      assertx(size > 0);
      return {uncounted_arr, size + sizeof(APCTypedValue)};
    }
  }

  return shared();
}

APCHandle::Pair
APCArray::MakeSharedVec(ArrayData* vec, APCHandleLevel level,
                        bool unserializeObj) {
  assertx(vec->isVecType());
  if (auto const value = APCTypedValue::HandlePersistent(vec)) {
    return value;
  }
  return MakeSharedImpl(
    vec,
    level,
    [&] {
      auto const kind = vec->isLegacyArray() ? APCKind::SharedLegacyVec
                                             : APCKind::SharedVec;
      return MakePacked(vec, kind, unserializeObj);
    },
    [&](StringData* s) { return APCString::MakeSerializedVec(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedDict(ArrayData* dict, APCHandleLevel level,
                         bool unserializeObj) {
  assertx(dict->isDictType());
  if (auto const value = APCTypedValue::HandlePersistent(dict)) {
    return value;
  }
  return MakeSharedImpl(
    dict,
    level,
    [&] {
      auto const kind = dict->isLegacyArray() ? APCKind::SharedLegacyDict
                                              : APCKind::SharedDict;
      return MakeHash(dict, kind, unserializeObj);
    },
    [&](StringData* s) { return APCString::MakeSerializedDict(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedKeyset(ArrayData* keyset, APCHandleLevel level,
                           bool unserializeObj) {
  assertx(keyset->isKeysetType());
  if (auto const value = APCTypedValue::HandlePersistent(keyset)) {
    return value;
  }
  return MakeSharedImpl(
    keyset,
    level,
    [&]() { return MakePacked(keyset, APCKind::SharedKeyset, unserializeObj); },
    [&](StringData* s) { return APCString::MakeSerializedKeyset(s); }
  );
}

APCHandle::Pair APCArray::MakeSharedEmptyVec() {
  void* p = apc_malloc(sizeof(APCArray));
  APCArray* arr = new (p) APCArray(PackedCtor{}, APCKind::SharedVec, 0);
  return {arr->getHandle(), sizeof(APCArray)};
}

APCHandle::Pair APCArray::MakeHash(ArrayData* arr, APCKind kind,
                                   bool unserializeObj) {
  auto const num_elems = arr->size();
  auto const allocSize = sizeof(APCArray) + sizeof(APCHandle*) * num_elems * 2;
  auto mem = reinterpret_cast<char*>(apc_malloc(allocSize));
  auto ret = new (mem) APCArray(HashedCtor{}, kind, num_elems);

  size_t i = 0;
  auto size = allocSize;
  try {
    IterateKV(
      arr,
      [&](TypedValue k, TypedValue v) {
        auto const key = APCHandle::Create(
            VarNR(k), APCHandleLevel::Inner, unserializeObj);
        auto const val = APCHandle::Create(
            VarNR(v), APCHandleLevel::Inner, unserializeObj);
        ret->vals()[i++] = key.handle;
        ret->vals()[i++] = val.handle;
        size += key.size;
        size += val.size;
        return false;
      }
    );
    assertx(i == num_elems * 2);
  } catch (...) {
    ret->m_size = i / 2;
    ret->~APCArray();
    apc_sized_free(mem, allocSize);
    throw;
  }

  return {ret->getHandle(), size};
}

APCHandle* APCArray::MakeUncountedArray(
    ArrayData* ad, DataWalker::PointerMap* seen) {
  assertx(apcExtension::UseUncounted);
  auto const env = MakeUncountedEnv { seen };
  auto const data = ::HPHP::MakeUncountedArray(ad, env, true);
  return APCTypedValue::ForArray(data)->getHandle();
}

APCHandle::Pair APCArray::MakePacked(ArrayData* arr, APCKind kind,
                                     bool unserializeObj) {
  auto const num_elems = arr->size();
  auto const allocSize = sizeof(APCArray) + sizeof(APCHandle*) * num_elems;
  auto mem = reinterpret_cast<char*>(apc_malloc(allocSize));
  auto ret = new (mem) APCArray(PackedCtor{}, kind, num_elems);

  size_t i = 0;
  auto size = allocSize;
  try {
    IterateV(
      arr,
      [&](TypedValue v) {
        auto const val = APCHandle::Create(
            VarNR(v), APCHandleLevel::Inner, unserializeObj);
        ret->vals()[i++] = val.handle;
        size += val.size;
        return false;
      }
    );
    assertx(i == num_elems);
  } catch (...) {
    ret->m_size = i;
    ret->~APCArray();
    apc_sized_free(mem, allocSize);
    throw;
  }

  return {ret->getHandle(), size};
}

void APCArray::Delete(APCHandle* handle) {
  auto const arr = APCArray::fromHandle(handle);
  arr->~APCArray();
  apc_free(reinterpret_cast<char*>(arr));
}

APCArray::~APCArray() {
  // This array is refcounted, but keys/values might be uncounted strings, so
  // we must use unreferenceRoot here (corresponding to Create calls above).
  auto const num_vals = isPacked() ? m_size : m_size * 2;
  for (size_t i = 0; i < num_vals; i++) {
    vals()[i]->unreferenceRoot();
  }
}

///////////////////////////////////////////////////////////////////////////////

}
