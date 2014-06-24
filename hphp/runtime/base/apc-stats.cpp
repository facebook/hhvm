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
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {

size_t getMemSize(const TypedValue* tv) {
  const auto& v = tvAsCVarRef(tv);
  auto type = v.getType();
  if (!IS_REFCOUNTED_TYPE(type)) {
    return sizeof(Variant);
  }
  if (type == KindOfString) {
    return getMemSize(v.getStringData());
  }
  if (type == KindOfArray) {
    return getMemSize(v.getArrayData());
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
      (packedCodeToCap(arr->m_packedCapCode) - arr->m_size) *
      sizeof(TypedValue);
    auto const values = reinterpret_cast<const TypedValue*>(arr + 1);
    auto const last = values + arr->m_size;
    for (auto ptr = values; ptr != last; ++ptr) {
      size += getMemSize(ptr);
    }
    return size;
  }
  case ArrayData::ArrayKind::kIntMapKind:
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
      size += ptr->hasStrKey() ? getMemSize(ptr->skey) : sizeof(int64_t);
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

std::unique_ptr<APCStats> APCStats::s_apcStats = nullptr;

void APCStats::Create() {
  s_apcStats = folly::make_unique<APCStats>();
}

APCStats::APCStats() : m_valueSize(nullptr)
                     , m_keySize(nullptr)
                     , m_inFileSize(nullptr)
                     , m_livePrimedSize(nullptr)
                     , m_pendingDeleteSize(nullptr)
                     , m_entries(nullptr)
                     , m_primedEntries(nullptr)
                     , m_livePrimedEntries(nullptr)
                     , m_detailedStats(nullptr) {
  m_valueSize = ServiceData::createTimeseries(
      "apc.value_size", {ServiceData::StatsType::SUM});
  m_keySize = ServiceData::createTimeseries(
      "apc.key_size", {ServiceData::StatsType::SUM});
  m_inFileSize = ServiceData::createTimeseries(
      "apc.in_file_size", {ServiceData::StatsType::SUM});
  m_livePrimedSize = ServiceData::createTimeseries(
      "apc.primed_live_size", {ServiceData::StatsType::SUM});
  m_pendingDeleteSize = ServiceData::createTimeseries(
      "apc.pending_delete_size", {ServiceData::StatsType::SUM});

  m_entries = ServiceData::createCounter("apc.entries");
  m_primedEntries = ServiceData::createCounter("apc.primed_entries");
  m_livePrimedEntries =
      ServiceData::createCounter("apc.primed_live_entries");
  if (RuntimeOption::EnableAPCStats) {
    m_detailedStats = new APCDetailedStats();
  }
}

APCStats::~APCStats() {
  if (m_detailedStats) delete m_detailedStats;
}

APCDetailedStats::APCDetailedStats() : m_uncounted(nullptr)
                                     , m_apcString(nullptr)
                                     , m_uncString(nullptr)
                                     , m_serArray(nullptr)
                                     , m_apcArray(nullptr)
                                     , m_uncArray(nullptr)
                                     , m_serObject(nullptr)
                                     , m_apcObject(nullptr)
                                     , m_setValues(nullptr)
                                     , m_delValues(nullptr)
                                     , m_replValues(nullptr)
                                     , m_expValues(nullptr) {
  m_uncounted = ServiceData::createCounter("apc.type_uncounted");
  m_apcString = ServiceData::createCounter("apc.type_apc_string");
  m_uncString = ServiceData::createCounter("apc.type_unc_string");
  m_serArray = ServiceData::createCounter("apc.type_ser_array");
  m_apcArray = ServiceData::createCounter("apc.type_apc_array");
  m_uncArray = ServiceData::createCounter("apc.type_unc_array");
  m_serObject = ServiceData::createCounter("apc.type_ser_object");
  m_apcObject = ServiceData::createCounter("apc.type_apc_object");

  m_setValues = ServiceData::createCounter("apc.set_values");
  m_delValues = ServiceData::createCounter("apc.deleted_values");
  m_replValues = ServiceData::createCounter("apc.replaced_values");
  m_expValues = ServiceData::createCounter("apc.expired_values");
}

void APCDetailedStats::addAPCValue(APCHandle* handle) {
  m_setValues->increment();
  addType(handle);
}

void APCDetailedStats::updateAPCValue(APCHandle* handle,
                                      APCHandle* oldHandle,
                                      bool expired) {
  removeType(oldHandle);
  addType(handle);
  if (expired) {
    m_expValues->increment();
  } else {
    m_replValues->increment();
  }
}

void APCDetailedStats::removeAPCValue(APCHandle* handle, bool expired) {
  removeType(handle);
  if (expired) {
    m_expValues->increment();
  } else {
    m_delValues->increment();
  }
}

void APCDetailedStats::addType(APCHandle* handle) {
  DataType type = handle->getType();
  assert(!IS_REFCOUNTED_TYPE(type) ||
         type == KindOfString ||
         type == KindOfArray ||
         type == KindOfObject);
  if (!IS_REFCOUNTED_TYPE(type)) {
    m_uncounted->increment();
    return;
  }
  switch (type) {
  case KindOfString:
    if (handle->getUncounted()) {
      m_uncString->increment();
    } else {
      m_apcString->increment();
    }
    return;
  case KindOfArray:
    if (handle->getUncounted()) {
      m_uncArray->increment();
    } else if (handle->getSerializedArray()) {
      m_serArray->increment();
    } else {
      m_apcArray->increment();
    }
    return;
  case KindOfObject:
    if (handle->getIsObj()) {
      m_apcObject->increment();
    } else {
      m_serObject->increment();
    }
    return;
  default:
    return;
  }
}

void APCDetailedStats::removeType(APCHandle* handle) {
  DataType type = handle->getType();
  assert(!IS_REFCOUNTED_TYPE(type) ||
         type == KindOfString ||
         type == KindOfArray ||
         type == KindOfObject);
  if (!IS_REFCOUNTED_TYPE(type)) {
    m_uncounted->decrement();
    return;
  }
  switch (type) {
  case KindOfString:
    if (handle->getUncounted()) {
      m_uncString->decrement();
    } else {
      m_apcString->decrement();
    }
    return;
  case KindOfArray:
    if (handle->getUncounted()) {
      m_uncArray->decrement();
    } else if (handle->getSerializedArray()) {
      m_serArray->decrement();
    } else {
      m_apcArray->decrement();
    }
    return;
  case KindOfObject:
    if (handle->getIsObj()) {
      m_apcObject->decrement();
    } else {
      m_serObject->decrement();
    }
    return;
  default:
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////

}
