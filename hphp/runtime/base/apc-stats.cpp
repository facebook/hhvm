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

#include "hphp/runtime/base/apc-stats.h"

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {

size_t getMemSize(const TypedValue* tv) {
  if (!IS_REFCOUNTED_TYPE(tv->m_type)) {
    return sizeof(Variant);
  }
  if (tv->m_type == KindOfString) {
    return getMemSize(tv->m_data.pstr);
  }
  if (tv->m_type == KindOfArray) {
    return getMemSize(tv->m_data.parr);
  }
  assert(!"Unsupported Variant type for getMemSize()");
  return 0;
}

}

size_t getMemSize(const APCHandle* handle) {
  auto t = handle->getType();
  if (!IS_REFCOUNTED_TYPE(t)) {
    return sizeof(APCHandle);
  }
  if (t == KindOfString) {
    if (handle->getUncounted()) {
      return sizeof(APCTypedValue) +
             getMemSize(APCTypedValue::fromHandle(handle)->getStringData());
    }
    return getMemSize(APCString::fromHandle(handle));
  }
  if (t == KindOfArray) {
    if (handle->getSerializedArray()) {
      return getMemSize(APCString::fromHandle(handle));
    }
    if (handle->getUncounted()) {
      return sizeof(APCTypedValue) +
             getMemSize(APCTypedValue::fromHandle(handle)->getArrayData());
    }
    return getMemSize(APCArray::fromHandle(handle));
  }
  if (t == KindOfObject) {
    if (handle->getIsObj()) {
      return getMemSize(APCObject::fromHandle(handle));
    }
    return getMemSize(APCString::fromHandle(handle));
  }

  assert(!"Unsupported APCHandle Type in getMemSize");
  return 0;
}

size_t getMemSize(const APCArray* arr) {
  auto memSize = sizeof(APCArray);
  auto size = arr->size();
  if (arr->isPacked()) {
    memSize += sizeof(APCHandle*) * size;
    for (auto i = 0; i < size; i++) {
      memSize += getMemSize(arr->getValue(i));
    }
  } else {
    memSize += sizeof(int) * (arr->m.m_capacity_mask + 1) +
               sizeof(APCArray::Bucket) * size;
    auto b = arr->buckets();
    for (auto i = 0; i < size; i++) {
      memSize += getMemSize(b[i].key);
      memSize += getMemSize(b[i].val);
    }
  }
  return memSize;
}

size_t getMemSize(const APCObject* obj) {
  auto size = sizeof(APCObject) +
              sizeof(APCObject::Prop) * obj->m_propCount;
  auto prop = obj->props();
  auto const propEnd = prop + obj->m_propCount;
  // we don't add property names and class names (or Class*) in Prop
  // assuming that is static data not owned or accounted by the APCObject
  for (; prop != propEnd; ++prop) {
    if (prop->val) size += getMemSize(prop->val);
  }
  return size;
}

size_t getMemSize(const ArrayData* arr) {
  switch (arr->kind()) {
  case ArrayData::ArrayKind::kPackedKind: {
    auto size = sizeof(ArrayData) +
                (arr->m_packedCap - arr->m_size) * sizeof(TypedValue);
    auto const values = reinterpret_cast<const TypedValue*>(arr + 1);
    auto const last = values + arr->m_size;
    for (auto ptr = values; ptr != last; ++ptr) {
      size += getMemSize(ptr);
    }
    return size;
  }
  case ArrayData::ArrayKind::kMixedKind: {
    auto const mixed = MixedArray::asMixed(arr);
    auto size = sizeof(MixedArray) +
                sizeof(MixedArray::Elm) * (mixed->m_cap - mixed->m_used);
    auto elms = mixed->data();
    auto last = elms + mixed->m_used;
    for (auto ptr = elms; ptr != last; ++ptr) {
      if (MixedArray::isTombstone(ptr->data.m_type)) {
        size += sizeof(MixedArray::Elm);
        continue;
      }
      size += ptr->hasStrKey() ? getMemSize(ptr->key) : sizeof(int64_t);
      size += getMemSize(&ptr->data);
    }
    return size;
  }
  case ArrayData::ArrayKind::kEmptyKind:
    return sizeof(ArrayData);
  default:
    assert(!"Unsupported Array type in getMemSize");
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
}
