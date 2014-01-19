/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-typed-value.h"
#include "hphp/runtime/base/apc-string.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/ext/ext_apc.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

APCHandle* APCArray::MakeShared(ArrayData* arr,
                                bool inner,
                                bool unserializeObj) {
  if (!inner) {
    // only need to call traverseData() on the toplevel array
    DataWalker walker(DataWalker::LookupFeature::Default);
    DataWalker::DataFeature features = walker.traverseData(arr);
    if (features.isCircular() || features.hasCollection()) {
      String s = apc_serialize(arr);
      APCHandle* handle = APCString::MakeShared(KindOfArray, s.get());
      handle->setSerializedArray();
      handle->mustCache();
      return handle;
    }
  }

  if (arr->isVectorData()) {
    return APCArray::MakePackedShared(arr, unserializeObj);
  }

  return APCArray::MakeShared(arr, unserializeObj);
}

APCHandle* APCArray::MakeShared() {
  void* p = malloc(sizeof(APCArray));
  APCArray* arr = new (p) APCArray(static_cast<size_t>(0));
  return arr->getHandle();
}

APCHandle* APCArray::MakeShared(ArrayData* arr,
                                bool unserializeObj) {
  auto num = arr->size();
  auto cap = num > 2 ? Util::roundUpToPowerOfTwo(num) : 2;

  void* p = malloc(sizeof(APCArray) +
                   sizeof(int) * cap +
                   sizeof(Bucket) * num);
  APCArray* ret = new (p) APCArray(static_cast<unsigned int>(cap));

  for (int i = 0; i < cap; i++) ret->hash()[i] = -1;

  try {
    for (ArrayIter it(arr); !it.end(); it.next()) {
      auto key = APCHandle::Create(it.first(), false, true,
                                   unserializeObj);
      auto val = APCHandle::Create(it.secondRef(), false, true,
                                   unserializeObj);
      if (val->shouldCache()) {
        ret->mustCache();
      }
      ret->add(key, val);
    }
  } catch (...) {
    delete ret;
    throw;
  }

  return ret->getHandle();
}

APCHandle* APCArray::MakePackedShared(ArrayData* arr,
                                      bool unserializeObj) {
  size_t num_elems = arr->size();
  void* p = malloc(sizeof(APCArray) + sizeof(APCHandle*) * num_elems);
  auto ret = new (p) APCArray(static_cast<size_t>(num_elems));

  try {
    size_t i = 0;
    for (ArrayIter it(arr); !it.end(); it.next()) {
      APCHandle* val = APCHandle::Create(it.secondRef(),
                                         false, true,
                                         unserializeObj);
      if (val->shouldCache()) {
        ret->mustCache();
      }
      ret->vals()[i++] = val;
    }
    assert(i == num_elems);
  } catch (...) {
    delete ret;
    throw;
  }

  return ret->getHandle();
}

Variant APCArray::MakeArray(APCHandle* handle) {
  if (handle->getSerializedArray()) {
    StringData* serArr = APCString::fromHandle(handle)->getStringData();
    return apc_unserialize(serArr->data(), serArr->size());
  }
  return APCLocalArray::Make(APCArray::fromHandle(handle));
}

void APCArray::Delete(APCHandle* handle) {
  handle->getSerializedArray() ? delete APCString::fromHandle(handle)
                               : delete APCArray::fromHandle(handle);
}

APCArray::~APCArray() {
  if (isPacked()) {
     APCHandle** v = vals();
     for (size_t i = 0, n = m_size; i < n; i++) {
       v[i]->decRef();
     }
  } else {
    Bucket* bks = buckets();
    for (int i = 0; i < m.m_num; i++) {
      bks[i].key->decRef();
      bks[i].val->decRef();
    }
  }
}

void APCArray::add(APCHandle *key, APCHandle *val) {
  int pos = m.m_num;
  // NOTE: no check on duplication because we assume the original array has no
  // duplication
  Bucket* bucket = buckets() + pos;
  bucket->key = key;
  bucket->val = val;
  m.m_num++;
  int hash_pos;
  if (!IS_REFCOUNTED_TYPE(key->getType())) {
    APCTypedValue *k = APCTypedValue::fromHandle(key);
    hash_pos = (key->is(KindOfInt64) ?
        k->getInt64() : k->getStringData()->hash()) & m.m_capacity_mask;
  } else {
    assert(key->is(KindOfString));
    APCString *k = APCString::fromHandle(key);
    hash_pos = k->getStringData()->hash() & m.m_capacity_mask;
  }

  int& hp = hash()[hash_pos];
  bucket->next = hp;
  hp = pos;
}

ssize_t APCArray::indexOf(const StringData* key) const {
  strhash_t h = key->hash();
  ssize_t bucket = hash()[h & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    if (!IS_REFCOUNTED_TYPE(b[bucket].key->getType())) {
      APCTypedValue *k = APCTypedValue::fromHandle(b[bucket].key);
      if (!b[bucket].key->is(KindOfInt64) &&
          key->same(k->getStringData())) {
        return bucket;
      }
    } else {
      assert(b[bucket].key->is(KindOfString));
      APCString *k = APCString::fromHandle(b[bucket].key);
      if (key->same(k->getStringData())) {
        return bucket;
      }
    }
    bucket = b[bucket].next;
  }
  return -1;
}

ssize_t APCArray::indexOf(int64_t key) const {
  ssize_t bucket = hash()[key & m.m_capacity_mask];
  Bucket* b = buckets();
  while (bucket != -1) {
    if (b[bucket].key->is(KindOfInt64) &&
        key == APCTypedValue::fromHandle(b[bucket].key)->getInt64()) {
      return bucket;
    }
    bucket = b[bucket].next;
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
}
