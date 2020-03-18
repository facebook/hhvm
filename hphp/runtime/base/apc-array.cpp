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
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-stats.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/data-walker.h"

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

template <typename A, typename B, typename C>
ALWAYS_INLINE
APCHandle::Pair
APCArray::MakeSharedImpl(ArrayData* arr,
                         APCHandleLevel level,
                         A shared, B uncounted, C serialized) {
  if (level == APCHandleLevel::Outer) {
    auto const seenArrs = apcExtension::ShareUncounted ?
      req::make_unique<DataWalker::PointerMap>() : nullptr;
    // only need to call traverseData() on the toplevel array
    DataWalker walker(DataWalker::LookupFeature::HasObjectOrResource);
    DataWalker::DataFeature features = walker.traverseData(arr, seenArrs.get());
    if (features.isCircular) {
      auto const s = apc_serialize(Variant{arr});
      return serialized(s.get());
    }

    if (apcExtension::UseUncounted && !features.hasObjectOrResource &&
        !arr->empty()) {
      auto const base_size = use_jemalloc ?
        tl_heap->getAllocated() - tl_heap->getDeallocated() :
        seenArrs.get() ? getMemSize(seenArrs.get()) :
        ::HPHP::getMemSize(arr, true);
      auto const uncounted_arr = uncounted(seenArrs.get());
      auto const size = use_jemalloc ?
        tl_heap->getAllocated() - tl_heap->getDeallocated() - base_size :
        base_size;
      assertx(size > 0);
      return {uncounted_arr, size + sizeof(APCTypedValue)};
    }
  }

  return shared();
}

APCHandle::Pair
APCArray::MakeSharedArray(ArrayData* arr, APCHandleLevel level,
                          bool unserializeObj) {
  assertx(arr->isPHPArrayKind());
  if (auto const value = APCTypedValue::HandlePersistent(
        APCTypedValue::StaticArr{}, APCTypedValue::UncountedArr{}, arr)) {
    return value;
  }
  return MakeSharedImpl(
    arr,
    level,
    [&]() {
      auto const add_prov = [arr] (APCHandle::Pair pair) {
        if (UNLIKELY(
          RO::EvalArrayProvenance &&
          RO::EvalArrProvDVArrays
        )) {
          if (auto const tag = arrprov::getTag(arr)) {
            arrprov::setTag(APCArray::fromHandle(pair.handle), tag);
          }
        }
        return pair;
      };

      if (arr->isVArray()) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return add_prov(MakePacked(arr, APCKind::SharedVArray, unserializeObj));
      }
      if (arr->isDArray()) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return add_prov(MakeHash(arr, APCKind::SharedDArray, unserializeObj));
      }
      return arr->isVectorData()
        ? MakePacked(arr, APCKind::SharedPackedArray, unserializeObj)
        : MakeHash(arr, APCKind::SharedArray, unserializeObj);
    },
    [&](DataWalker::PointerMap* m) { return MakeUncountedArray(arr, m); },
    [&](StringData* s) { return APCString::MakeSerializedArray(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedVec(ArrayData* vec, APCHandleLevel level,
                        bool unserializeObj) {
  assertx(vec->isVecArrayKind());
  if (auto const value = APCTypedValue::HandlePersistent(
        APCTypedValue::StaticVec{}, APCTypedValue::UncountedVec{}, vec)) {
    return value;
  }
  return MakeSharedImpl(
    vec,
    level,
    [&] {
      auto const pair = MakePacked(vec, APCKind::SharedVec, unserializeObj);
      if (UNLIKELY(
        RO::EvalArrayProvenance &&
        RO::EvalArrProvHackArrays
      )) {
        if (auto const tag = arrprov::getTag(vec)) {
          arrprov::setTag(APCArray::fromHandle(pair.handle), tag);
        }
      }
      return pair;
    },
    [&](DataWalker::PointerMap* m) { return MakeUncountedVec(vec, m); },
    [&](StringData* s) { return APCString::MakeSerializedVec(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedDict(ArrayData* dict, APCHandleLevel level,
                         bool unserializeObj) {
  assertx(dict->isDictKind());
  if (auto const value = APCTypedValue::HandlePersistent(
        APCTypedValue::StaticDict{}, APCTypedValue::UncountedDict{}, dict)) {
    return value;
  }
  return MakeSharedImpl(
    dict,
    level,
    [&] {
      auto const pair = MakeHash(dict, APCKind::SharedDict, unserializeObj);
      if (UNLIKELY(
        RO::EvalArrayProvenance &&
        RO::EvalArrProvHackArrays
      )) {
        if (auto const tag = arrprov::getTag(dict)) {
          arrprov::setTag(APCArray::fromHandle(pair.handle), tag);
        }
      }
      return pair;
    },
    [&](DataWalker::PointerMap* m) { return MakeUncountedDict(dict, m); },
    [&](StringData* s) { return APCString::MakeSerializedDict(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedKeyset(ArrayData* keyset, APCHandleLevel level,
                           bool unserializeObj) {
  assertx(keyset->isKeysetKind());
  if (auto const value = APCTypedValue::HandlePersistent(
        APCTypedValue::StaticKeyset{}, APCTypedValue::UncountedKeyset{},
        keyset)) {
    return value;
  }
  return MakeSharedImpl(
    keyset,
    level,
    [&]() { return MakePacked(keyset, APCKind::SharedKeyset, unserializeObj); },
    [&](DataWalker::PointerMap*) { return MakeUncountedKeyset(keyset); },
    [&](StringData* s) { return APCString::MakeSerializedKeyset(s); }
  );
}

APCHandle::Pair APCArray::MakeSharedEmptyArray() {
  void* p = apc_malloc(sizeof(APCArray));
  APCArray* arr = new (p) APCArray(PackedCtor{},
                                   APCKind::SharedPackedArray,
                                   0);
  return {arr->getHandle(), sizeof(APCArray)};
}

APCHandle::Pair APCArray::MakeHash(ArrayData* arr, APCKind kind,
                                   bool unserializeObj) {
  auto const num = arr->size();
  auto const cap = num > 2 ? folly::nextPowTwo(num) : 2;
  auto const prov_off = arrprov::tagSize(arr);
  auto const allocSize = sizeof(APCArray)
                       + sizeof(int) * cap
                       + sizeof(Bucket) * num
                       + prov_off;
  auto p = reinterpret_cast<char*>(apc_malloc(allocSize)) + prov_off;
  APCArray* ret = new (p) APCArray(HashedCtor{}, kind, cap);

  for (int i = 0; i < cap; i++) ret->hash()[i] = -1;

  auto size = allocSize;
  try {
    IterateKV(
      arr,
      [&](TypedValue k, TypedValue v) {
        auto key = APCHandle::Create(VarNR(k), false,
                                     APCHandleLevel::Inner, unserializeObj);
        size += key.size;
        auto val = APCHandle::Create(VarNR(v), false,
                                     APCHandleLevel::Inner, unserializeObj);
        size += val.size;
        ret->add(key.handle, val.handle);
        return false;
      }
    );
  } catch (...) {
    ret->~APCArray();
    apc_sized_free(p, allocSize);
    throw;
  }

  return {ret->getHandle(), size};
}

APCHandle* APCArray::MakeUncountedArray(
    ArrayData* array,
    DataWalker::PointerMap* m) {
  assertx(apcExtension::UseUncounted);
  assertx(array->isPHPArrayKind());
  auto const value = [&] {
    if (array->isPackedKind()) {
      auto const data = PackedArray::MakeUncounted(array, true, m);
      auto const mem = reinterpret_cast<APCTypedValue*>(data) - 1;
      return new(mem) APCTypedValue(APCTypedValue::UncountedArr{}, data);
    }
    assertx(array->isMixedKind());
    auto const data = MixedArray::MakeUncounted(array, true, m);
    auto const mem = reinterpret_cast<APCTypedValue*>(data) - 1;
    return new(mem) APCTypedValue(APCTypedValue::UncountedArr{}, data);
  }();
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedVec(
    ArrayData* vec,
    DataWalker::PointerMap* m) {
  assertx(apcExtension::UseUncounted);
  assertx(vec->isVecArrayKind());
  auto data = PackedArray::MakeUncounted(vec, true, m);
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedVec{}, data);
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedDict(
    ArrayData* dict,
    DataWalker::PointerMap* m) {
  assertx(apcExtension::UseUncounted);
  assertx(dict->isDictKind());
  auto data = MixedArray::MakeUncounted(dict, true, m);
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedDict{}, data);
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedKeyset(ArrayData* keyset) {
  assertx(apcExtension::UseUncounted);
  assertx(keyset->isKeysetKind());
  auto data = SetArray::MakeUncounted(keyset, true);
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedKeyset{}, data);
  return value->getHandle();
}

APCHandle::Pair APCArray::MakePacked(ArrayData* arr, APCKind kind,
                                     bool unserializeObj) {
  auto const num_elems = arr->size();
  auto const prov_off = arrprov::tagSize(arr);
  auto const allocSize = sizeof(APCArray)
                       + sizeof(APCHandle*) * num_elems
                       + prov_off;
  auto p = reinterpret_cast<char*>(apc_malloc(allocSize)) + prov_off;
  auto ret = new (p) APCArray(PackedCtor{}, kind, num_elems);

  size_t i = 0;
  auto size = allocSize;
  try {
    IterateV(
      arr,
      [&](TypedValue v) {
        auto val = APCHandle::Create(VarNR(v), false,
                                     APCHandleLevel::Inner, unserializeObj);
        size += val.size;
        ret->vals()[i++] = val.handle;
        return false;
      }
    );
    assertx(i == num_elems);
  } catch (...) {
    ret->m_size = i;
    ret->~APCArray();
    apc_sized_free(p, allocSize);
    throw;
  }

  return {ret->getHandle(), size};
}

void APCArray::Delete(APCHandle* handle) {
  auto const arr = APCArray::fromHandle(handle);
  auto const prov_off = arrprov::tagSize(arr);
  arr->~APCArray();
  if (RuntimeOption::EvalArrayProvenance) {
    arrprov::clearTag(arr);
  }
  apc_free(reinterpret_cast<char*>(arr) - prov_off);
}

APCArray::~APCArray() {
  // This array is refcounted, but keys/values might be uncounted strings, so
  // we must use unreferenceRoot here (corresponding to Create calls above).
  if (isPacked()) {
    APCHandle** v = vals();
    for (size_t i = 0, n = m_size; i < n; i++) {
      v[i]->unreferenceRoot();
    }
  } else {
    Bucket* bks = buckets();
    for (int i = 0; i < m.m_num; i++) {
      bks[i].key->unreferenceRoot();
      bks[i].val->unreferenceRoot();
    }
  }
}

void APCArray::add(APCHandle *key, APCHandle *val) {
  int hash_pos;
  auto kind = key->kind();
  if (kind == APCKind::Int) {
    hash_pos = APCTypedValue::fromHandle(key)->getInt64();
  } else if (kind == APCKind::StaticString ||
             kind == APCKind::UncountedString) {
    hash_pos = APCTypedValue::fromHandle(key)->getStringData()->hash();
  } else {
    assertx(kind == APCKind::SharedString);
    hash_pos = APCString::fromHandle(key)->getStringData()->hash();
  }
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  int& hp = hash()[hash_pos & m.m_capacity_mask];
  int pos = m.m_num++;
  Bucket* bucket = buckets() + pos;
  bucket->key = key;
  bucket->val = val;
  bucket->next = hp;
  hp = pos;
}

ssize_t APCArray::indexOf(const StringData* key) const {
  strhash_t h = key->hash();
  Bucket* b = buckets();
  for (ssize_t bucket = hash()[h & m.m_capacity_mask]; bucket != -1;
       bucket = b[bucket].next) {
    auto kind = b[bucket].key->kind();
    if (kind == APCKind::StaticString || kind == APCKind::UncountedString) {
      auto const k = APCTypedValue::fromHandle(b[bucket].key);
      if (key->same(k->getStringData())) {
        return bucket;
      }
    } else if (kind == APCKind::SharedString) {
      auto const k = APCString::fromHandle(b[bucket].key);
      if (key->same(k->getStringData())) {
        return bucket;
      }
    }
  }
  return -1;
}

ssize_t APCArray::indexOf(int64_t key) const {
  Bucket* b = buckets();
  for (ssize_t bucket = hash()[key & m.m_capacity_mask]; bucket != -1;
       bucket = b[bucket].next) {
    auto key_handle = b[bucket].key;
    auto kind = key_handle->kind();
    if (kind == APCKind::Int &&
        key == APCTypedValue::fromHandle(key_handle)->getInt64()) {
      return bucket;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}
