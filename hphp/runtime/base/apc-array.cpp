/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

APCHandle::Pair
APCArray::MakeSharedArray(ArrayData* arr, APCHandleLevel level,
                          bool unserializeObj) {
  if (level == APCHandleLevel::Outer) {
    // only need to call traverseData() on the toplevel array
    DataWalker walker(DataWalker::LookupFeature::HasObjectOrResource);
    DataWalker::DataFeature features = walker.traverseData(arr);
    if (features.isCircular) {
      String s = apc_serialize(Variant{arr});
      return APCString::MakeSerializedArray(s.get());
    }

    if (apcExtension::UseUncounted && !features.hasObjectOrResource &&
        !arr->empty()) {
      return {MakeUncountedArray(arr),
              getMemSize(arr) + sizeof(APCTypedValue)};
    }
  }

  return arr->isVectorData() ? MakePacked(arr, unserializeObj) :
         MakeHash(arr, unserializeObj);
}

APCHandle::Pair APCArray::MakeSharedEmptyArray() {
  void* p = malloc(sizeof(APCArray));
  APCArray* arr = new (p) APCArray(static_cast<size_t>(0));
  return {arr->getHandle(), sizeof(APCArray)};
}

APCHandle::Pair APCArray::MakeHash(ArrayData* arr, bool unserializeObj) {
  auto num = arr->size();
  auto cap = num > 2 ? folly::nextPowTwo(num) : 2;

  auto size = sizeof(APCArray) + sizeof(int) * cap + sizeof(Bucket) * num;
  auto p = malloc(size);
  APCArray* ret = new (p) APCArray(static_cast<unsigned int>(cap));

  for (int i = 0; i < cap; i++) ret->hash()[i] = -1;

  try {
    for (ArrayIter it(arr); !it.end(); it.next()) {
      auto key = APCHandle::Create(it.first(), false, APCHandleLevel::Inner,
                                   unserializeObj);
      size += key.size;
      auto val = APCHandle::Create(it.secondRef(), false,
                                   APCHandleLevel::Inner, unserializeObj);
      size += val.size;
      ret->add(key.handle, val.handle);
    }
  } catch (...) {
    delete ret;
    throw;
  }

  return {ret->getHandle(), size};
}

APCHandle* APCArray::MakeUncountedArray(ArrayData* array) {
  assert(apcExtension::UseUncounted);
  APCTypedValue* value;
  if (array->isPacked()) {
    value = new APCTypedValue(APCTypedValue::UncountedArr{},
                              PackedArray::MakeUncounted(array));
  } else if (array->isStruct()) {
    value = new APCTypedValue(APCTypedValue::UncountedArr{},
                              StructArray::MakeUncounted(array));
  } else {
    value = new APCTypedValue(APCTypedValue::UncountedArr{},
                              MixedArray::MakeUncounted(array));
  }
  return value->getHandle();
}

APCHandle::Pair APCArray::MakePacked(ArrayData* arr, bool unserializeObj) {
  auto num_elems = arr->size();
  auto size = sizeof(APCArray) + sizeof(APCHandle*) * num_elems;
  auto p = malloc(size);
  auto ret = new (p) APCArray(static_cast<size_t>(num_elems));

  try {
    size_t i = 0;
    for (ArrayIter it(arr); !it.end(); it.next()) {
      auto val = APCHandle::Create(it.secondRef(), false,
                                   APCHandleLevel::Inner, unserializeObj);
      size += val.size;
      ret->vals()[i++] = val.handle;
    }
    assert(i == num_elems);
  } catch (...) {
    delete ret;
    throw;
  }

  return {ret->getHandle(), size};
}

void APCArray::Delete(APCHandle* handle) {
  auto const arr = APCArray::fromHandle(handle);
  arr->~APCArray();
  std::free(arr);
}

APCArray::~APCArray() {
  if (isPacked()) {
    APCHandle** v = vals();
    for (size_t i = 0, n = m_size; i < n; i++) {
      v[i]->unreference();
    }
  } else {
    Bucket* bks = buckets();
    for (int i = 0; i < m.m_num; i++) {
      bks[i].key->unreference();
      bks[i].val->unreference();
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
    assert(kind == APCKind::SharedString);
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
