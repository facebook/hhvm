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
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/apc-handle-defs.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

template <typename A, typename B, typename C>
ALWAYS_INLINE
APCHandle::Pair
APCArray::MakeSharedImpl(ArrayData* arr,
                         APCHandleLevel level,
                         A shared, B uncounted, C serialized) {
  if (level == APCHandleLevel::Outer) {
    // only need to call traverseData() on the toplevel array
    DataWalker walker(DataWalker::LookupFeature::HasObjectOrResource);
    DataWalker::DataFeature features = walker.traverseData(arr);
    if (features.isCircular) {
      String s = apc_serialize(Variant{arr});
      return serialized(s.get());
    }

    if (apcExtension::UseUncounted && !features.hasObjectOrResource &&
        !arr->empty()) {
      return {uncounted(), getMemSize(arr) + sizeof(APCTypedValue)};
    }
  }

  return shared();
}

APCHandle::Pair
APCArray::MakeSharedArray(ArrayData* arr, APCHandleLevel level,
                          bool unserializeObj) {
  assertx(arr->isPHPArray());
  return MakeSharedImpl(
    arr,
    level,
    [&]() {
      return arr->isVectorData()
        ? MakePacked(arr, APCKind::SharedPackedArray, unserializeObj)
        : MakeHash(arr, APCKind::SharedArray, unserializeObj);
    },
    [&]() { return MakeUncountedArray(arr); },
    [&](StringData* s) { return APCString::MakeSerializedArray(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedVec(ArrayData* vec, APCHandleLevel level,
                        bool unserializeObj) {
  assertx(vec->isVecArray());
  return MakeSharedImpl(
    vec,
    level,
    [&]() { return MakePacked(vec, APCKind::SharedVec, unserializeObj); },
    [&]() { return MakeUncountedVec(vec); },
    [&](StringData* s) { return APCString::MakeSerializedVec(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedDict(ArrayData* dict, APCHandleLevel level,
                         bool unserializeObj) {
  assertx(dict->isDict());
  return MakeSharedImpl(
    dict,
    level,
    [&]() { return MakeHash(dict, APCKind::SharedDict, unserializeObj); },
    [&]() { return MakeUncountedDict(dict); },
    [&](StringData* s) { return APCString::MakeSerializedDict(s); }
  );
}

APCHandle::Pair
APCArray::MakeSharedKeyset(ArrayData* keyset, APCHandleLevel level,
                           bool unserializeObj) {
  assertx(keyset->isKeyset());
  return MakeSharedImpl(
    keyset,
    level,
    [&]() { return MakePacked(keyset, APCKind::SharedKeyset, unserializeObj); },
    [&]() { return MakeUncountedKeyset(keyset); },
    [&](StringData* s) { return APCString::MakeSerializedKeyset(s); }
  );
}

APCHandle::Pair APCArray::MakeSharedEmptyArray() {
  void* p = malloc_huge(sizeof(APCArray));
  APCArray* arr = new (p) APCArray(PackedCtor{},
                                   APCKind::SharedPackedArray,
                                   0);
  return {arr->getHandle(), sizeof(APCArray)};
}

APCHandle::Pair APCArray::MakeHash(ArrayData* arr, APCKind kind,
                                   bool unserializeObj) {
  auto num = arr->size();
  auto cap = num > 2 ? folly::nextPowTwo(num) : 2;

  auto size = sizeof(APCArray) + sizeof(int) * cap + sizeof(Bucket) * num;
  auto p = malloc_huge(size);
  APCArray* ret = new (p) APCArray(HashedCtor{}, kind, cap);

  for (int i = 0; i < cap; i++) ret->hash()[i] = -1;

  try {
    IterateKV(
      arr,
      [&](Cell k, TypedValue v) {
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
    free_huge(p);
    throw;
  }

  return {ret->getHandle(), size};
}

APCHandle* APCArray::MakeUncountedArray(ArrayData* array) {
  assertx(apcExtension::UseUncounted);
  assertx(array->isPHPArray());
  APCTypedValue* value;
  if (array->isPacked()) {
    ArrayData* data = PackedArray::MakeUncounted(array, sizeof(APCTypedValue));
    auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
    value = new(mem) APCTypedValue(APCTypedValue::UncountedArr{}, data);
  } else {
    assert(array->isMixed());
    ArrayData* data = MixedArray::MakeUncounted(array, sizeof(APCTypedValue));
    auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
    value = new(mem) APCTypedValue(APCTypedValue::UncountedArr{}, data);
  }
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedVec(ArrayData* vec) {
  assertx(apcExtension::UseUncounted);
  assertx(vec->isVecArray());
  auto data = PackedArray::MakeUncounted(vec, sizeof(APCTypedValue));
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedVec{}, data);
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedDict(ArrayData* dict) {
  assertx(apcExtension::UseUncounted);
  assertx(dict->isDict());
  auto data = MixedArray::MakeUncounted(dict, sizeof(APCTypedValue));
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedDict{}, data);
  return value->getHandle();
}

APCHandle* APCArray::MakeUncountedKeyset(ArrayData* keyset) {
  assertx(apcExtension::UseUncounted);
  assertx(keyset->isKeyset());
  auto data = SetArray::MakeUncounted(keyset, sizeof(APCTypedValue));
  auto mem = reinterpret_cast<APCTypedValue*>(data) - 1;
  auto value = new(mem) APCTypedValue(APCTypedValue::UncountedKeyset{}, data);
  return value->getHandle();
}

APCHandle::Pair APCArray::MakePacked(ArrayData* arr, APCKind kind,
                                     bool unserializeObj) {
  auto num_elems = arr->size();
  auto size = sizeof(APCArray) + sizeof(APCHandle*) * num_elems;
  auto p = malloc_huge(size);
  auto ret = new (p) APCArray(PackedCtor{}, kind, num_elems);

  size_t i = 0;
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
    free_huge(p);
    throw;
  }

  return {ret->getHandle(), size};
}

void APCArray::Delete(APCHandle* handle) {
  auto const arr = APCArray::fromHandle(handle);
  arr->~APCArray();
  free_huge(arr);
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
