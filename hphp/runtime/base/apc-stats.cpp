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

#include "hphp/runtime/base/apc-stats.h"

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/apc-object.h"
#include "hphp/runtime/base/apc-rclass-meth.h"
#include "hphp/runtime/base/apc-rfunc.h"
#include "hphp/runtime/ext/apc/ext_apc.h"

#include "hphp/util/trace.h"


namespace HPHP {

TRACE_SET_MOD(apc);


///////////////////////////////////////////////////////////////////////////////

namespace {

// NEVER includes the size of TypedValue pointed to by the lval. The type and
// value may be stored in some exotic layout instead of in a TypedValue shape,
// so we only want to account for any memory pointed to from the value.
//
// (For example, for an int lval, we should return 0, and for a string lval,
// we should return the number of bytes in the string's payload.)
size_t getIndirectMemSize(tv_rval lval, bool recurse = true) {
  const auto type = lval.type();
  if (isArrayLikeType(type)) {
    if (!recurse) return 0;
    auto a = val(lval).parr;
    return a->isStatic() ? 0 : getMemSize(a);
  }
  if (type == KindOfString) {
    return getMemSize(val(lval).pstr);
  }
  if (!isRefcountedType(type)) {
    return 0;
  }
  assertx(!"Unsupported Variant type for getMemSize()");
  return 0;
}

}

size_t getMemSize(const APCHandle* handle) {
  switch (handle->kind()) {
    case APCKind::Uninit:
    case APCKind::Null:
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double:
    case APCKind::PersistentClass:
    case APCKind::ClassEntity:
    case APCKind::LazyClass:
    case APCKind::PersistentFunc:
    case APCKind::PersistentClsMeth:
    case APCKind::FuncEntity:
    case APCKind::ClsMeth:
    case APCKind::StaticArray:
    case APCKind::StaticBespoke:
    case APCKind::StaticString:
      return sizeof(APCHandle);

    case APCKind::UncountedString:
      return sizeof(APCTypedValue) +
             getMemSize(APCTypedValue::fromHandle(handle)->getStringData());
    case APCKind::SharedString:
      return getMemSize(APCString::fromHandle(handle));

    case APCKind::SerializedVec:
    case APCKind::SerializedDict:
    case APCKind::SerializedKeyset:
      return getMemSize(APCString::fromHandle(handle));

    case APCKind::UncountedArray:
    case APCKind::UncountedBespoke: {
      auto const ad = APCTypedValue::fromHandle(handle)->getArrayData();
      return sizeof(APCTypedValue) + getMemSize(ad);
    }

    case APCKind::SharedVec:
    case APCKind::SharedLegacyVec:
    case APCKind::SharedDict:
    case APCKind::SharedLegacyDict:
    case APCKind::SharedKeyset:
      return getMemSize(APCArray::fromHandle(handle));

    case APCKind::SerializedObject:
      return getMemSize(APCString::fromHandle(handle));

    case APCKind::SharedObject:
    case APCKind::SharedCollection:
      return getMemSize(APCObject::fromHandle(handle));

    case APCKind::RFunc:
      return getMemSize(APCRFunc::fromHandle(handle));
    case APCKind::RClsMeth:
      return getMemSize(APCRClsMeth::fromHandle(handle));
  }
  assertx(!"Unsupported APCHandle Type in getMemSize");
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
  if (obj->isPersistent()) {
    auto size = sizeof(APCObject) +
                sizeof(APCHandle*) * obj->m_propCount;
    auto prop = obj->persistentProps();
    auto const propEnd = prop + obj->m_propCount;
    for (; prop != propEnd; ++prop) {
      if (*prop) size += getMemSize(*prop);
    }
    return size;
  }

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

size_t getMemSize(const APCRFunc* rfunc) {
  return sizeof(APCRFunc) + getMemSize(rfunc->m_generics);
}

size_t getMemSize(const APCRClsMeth* rclsmeth) {
  return sizeof(APCRClsMeth) + getMemSize(rclsmeth->m_generics);
}

size_t getMemSize(const ArrayData* arr, bool recurse) {
  switch (arr->kind()) {
  case ArrayData::ArrayKind::kVecKind: {
    auto size = PackedArray::heapSize(arr);
    for (uint32_t i = 0; i < arr->m_size; ++i) {
      auto const tv = PackedArray::NvGetInt(arr, i);
      size += getIndirectMemSize(&tv);
    }
    return size;
  }
  case ArrayData::ArrayKind::kDictKind: {
    auto const mixed = MixedArray::asMixed(arr);
    auto size = sizeof(MixedArray) +
                sizeof(MixedArray::Elm) * (mixed->capacity() - mixed->m_used);
    auto elms = mixed->data();
    auto last = elms + mixed->m_used;
    for (auto ptr = elms; ptr != last; ++ptr) {
      if (MixedArray::isTombstone(ptr->data.m_type)) {
        size += sizeof(MixedArray::Elm);
        continue;
      }
      // TODO(kshaunak): I think our key-size accounting is wrong. We count the
      // direct size within the MixedArray for int64 keys but only the indirect
      // size for string keys. We should use the object's heap size to get all
      // the direct memory sizes, like we do for arrays, above.
      size += ptr->hasStrKey() ? getMemSize(ptr->skey) : sizeof(int64_t);
      size += getIndirectMemSize(&ptr->data, recurse);
      size += sizeof(TypedValueAux);
    }
    return size;
  }
  case ArrayData::ArrayKind::kKeysetKind: {
    auto const set = SetArray::asSet(arr);
    auto size = sizeof(SetArray) +
                sizeof(SetArray::Elm) + (set->capacity() - set->m_used);
    auto const elms = set->data();
    auto const used = set->m_used;
    for (uint32_t i = 0; i < used; ++i) {
      auto const& elm = elms[i];
      if (elm.isTombstone()) {
        size += sizeof(SetArray::Elm);
      } else {
        size += elm.hasStrKey() ? getMemSize(elm.strKey()) : sizeof(int64_t);
      }
    }
    return size;
  }
  default:
    assertx(!"Unsupported Array type in getMemSize");
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<APCStats> APCStats::s_apcStats = nullptr;

void APCStats::Create() {
  FTRACE(3, "APCStats::Create() called\n");

  s_apcStats = std::make_unique<APCStats>();
}

APCStats::APCStats() : m_valueSize(nullptr)
                     , m_keySize(nullptr)
                     , m_inFileSize(nullptr)
                     , m_livePrimedSize(nullptr)
                     , m_pendingDeleteSize(nullptr)
                     , m_entries(nullptr)
                     , m_primedEntries(nullptr)
                     , m_livePrimedEntries(nullptr)
                     , m_uncountedEntries(nullptr)
                     , m_uncountedBlocks(nullptr)
                     , m_detailedStats(nullptr) {
  m_valueSize = ServiceData::createCounter("apc.value_size.sum");
  m_keySize = ServiceData::createCounter("apc.key_size.sum");
  m_inFileSize = ServiceData::createCounter("apc.in_file_size.sum");
  m_livePrimedSize = ServiceData::createCounter("apc.primed_live_size.sum");
  m_pendingDeleteSize =
    ServiceData::createCounter("apc.pending_delete_size.sum");
  m_entries = ServiceData::createCounter("apc.entries");
  m_primedEntries = ServiceData::createCounter("apc.primed_entries");
  m_livePrimedEntries =
      ServiceData::createCounter("apc.primed_live_entries");
  m_uncountedEntries = ServiceData::createCounter("apc.uncounted_entries");
  m_uncountedBlocks =
    ServiceData::createCounter("apc.uncounted_blocks.mayNotBeAPCValues");
  if (RuntimeOption::EnableAPCStats) {
    m_detailedStats = new APCDetailedStats();
  }
}

APCStats::~APCStats() {
  if (m_detailedStats) delete m_detailedStats;
}

std::string APCStats::getStatsInfo() const {
  std::string info("APC info\nValue size: ");
  info += std::to_string(m_valueSize->getValue()) +
          "\nKey size: " +
          std::to_string(m_keySize->getValue()) +
          "\nMapped to file data size: " +
          std::to_string(m_inFileSize->getValue()) +
          "\nIn memory primed data size: " +
          std::to_string(m_livePrimedSize->getValue()) +
          "\nEntries count: " +
          std::to_string(m_entries->getValue()) +
          "\nPrimed entries count: " +
          std::to_string(m_primedEntries->getValue()) +
          "\nIn memory primed entries count: " +
          std::to_string(m_livePrimedEntries->getValue()) +
          "\nIn total uncounted entries count: " +
          std::to_string(m_uncountedEntries->getValue()) +
          "\nIn memory uncounted blocks: " +
          std::to_string(m_uncountedBlocks->getValue());
  if (apcExtension::UseUncounted) {
    info += "\nPending deletes via treadmill size: " +
            std::to_string(m_pendingDeleteSize->getValue());
  }
  if (m_detailedStats) {
    info += m_detailedStats->getStatsInfo();
  }
  return info + "\n";
}

const StaticString s_num_entries("num_entries");
const StaticString s_primedEntries("primed_entries");
const StaticString s_primedLiveEntries("primed_live_entries");
const StaticString s_valuesSize("values_size");
const StaticString s_keysSize("keys_size");
const StaticString s_primedInFileSize("primed_in_file_size");
const StaticString s_primeLiveSize("primed_live_size");
const StaticString s_pendingDeleteSize("pending_delete_size");
const StaticString s_uncountedEntries("uncounted_entries");
const StaticString s_uncountedBlocks("uncounted_blocks");

void APCStats::collectStats(std::map<const StringData*, int64_t>& stats) const {
  stats.insert(
      std::pair<const StringData*, int64_t>(s_num_entries.get(),
                                            m_entries->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_primedEntries.get(),
                                            m_primedEntries->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_primedLiveEntries.get(),
                                            m_livePrimedEntries->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>( s_uncountedEntries.get(),
                                            m_uncountedEntries->getValue()));
  stats.insert(
    std::pair<const StringData*, int64_t> ( s_uncountedBlocks.get(),
                                      m_uncountedBlocks->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_valuesSize.get(),
                                            m_valueSize->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_keysSize.get(),
                                            m_keySize->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_primedInFileSize.get(),
                                            m_inFileSize->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_primeLiveSize.get(),
                                            m_livePrimedSize->getValue()));
  stats.insert(
      std::pair<const StringData*, int64_t>(s_pendingDeleteSize.get(),
                                            m_pendingDeleteSize->getValue()));
  if (m_detailedStats) {
    m_detailedStats->collectStats(stats);
  }
}

APCDetailedStats::APCDetailedStats() : m_uncounted(nullptr)
                                     , m_apcString(nullptr)
                                     , m_uncString(nullptr)
                                     , m_serVec(nullptr)
                                     , m_serDict(nullptr)
                                     , m_serKeyset(nullptr)
                                     , m_apcVec(nullptr)
                                     , m_apcDict(nullptr)
                                     , m_apcKeyset(nullptr)
                                     , m_uncVec(nullptr)
                                     , m_uncDict(nullptr)
                                     , m_uncKeyset(nullptr)
                                     , m_serObject(nullptr)
                                     , m_apcObject(nullptr)
                                     , m_apcRFunc(nullptr)
                                     , m_apcRClsMeth(nullptr)
                                     , m_setValues(nullptr)
                                     , m_delValues(nullptr)
                                     , m_replValues(nullptr)
                                     , m_expValues(nullptr) {
  m_uncounted = ServiceData::createCounter("apc.type_uncounted");
  m_apcString = ServiceData::createCounter("apc.type_apc_string");
  m_uncString = ServiceData::createCounter("apc.type_unc_string");
  m_serVec = ServiceData::createCounter("apc.type_ser_vec");
  m_serDict = ServiceData::createCounter("apc.type_ser_dict");
  m_serKeyset = ServiceData::createCounter("apc.type_ser_keyset");
  m_apcVec = ServiceData::createCounter("apc.type_apc_vec");
  m_apcDict = ServiceData::createCounter("apc.type_apc_dict");
  m_apcKeyset = ServiceData::createCounter("apc.type_apc_keyset");
  m_uncVec = ServiceData::createCounter("apc.type_unc_vec");
  m_uncDict = ServiceData::createCounter("apc.type_unc_dict");
  m_uncKeyset = ServiceData::createCounter("apc.type_unc_keyset");
  m_serObject = ServiceData::createCounter("apc.type_ser_object");
  m_apcObject = ServiceData::createCounter("apc.type_apc_object");
  m_apcRFunc = ServiceData::createCounter("apc.type_apc_rfunc");
  m_apcRClsMeth = ServiceData::createCounter("apc.type_apc_rclsmeth");

  m_setValues = ServiceData::createCounter("apc.set_values");
  m_delValues = ServiceData::createCounter("apc.deleted_values");
  m_replValues = ServiceData::createCounter("apc.replaced_values");
  m_expValues = ServiceData::createCounter("apc.expired_values");
}

const StaticString s_typeUncounted("type_uncounted");
const StaticString s_typeAPCString("type_apc_string");
const StaticString s_typeUncountedString("type_unc_string");
const StaticString s_typeSerVec("type_ser_vec");
const StaticString s_typeSerDict("type_ser_dict");
const StaticString s_typeSerKeyset("type_ser_keyset");
const StaticString s_typeAPCVec("type_apc_vec");
const StaticString s_typeAPCDict("type_apc_dict");
const StaticString s_typeAPCKeyset("type_apc_keyset");
const StaticString s_typUncountedVec("type_unc_vec");
const StaticString s_typUncountedDict("type_unc_dict");
const StaticString s_typUncountedKeyset("type_unc_keyset");
const StaticString s_typeSerObject("type_ser_object");
const StaticString s_typeAPCObject("type_apc_object");
const StaticString s_typeAPCRFunc("type_apc_rfunc");
const StaticString s_typeAPCRClsMeth("type_apc_rclsmeth");
const StaticString s_setValueCount("set_values_count");
const StaticString s_deleteValuesCount("deleted_values_count");
const StaticString s_replacedValueCount("replaced_values_count");
const StaticString s_expiredValueCount("expired_values_count");

std::string APCDetailedStats::getStatsInfo() const {
  return "\nPrimitve and static strings count: " +
         std::to_string(m_uncounted->getValue()) +
         "\nAPC strings count: " +
         std::to_string(m_apcString->getValue()) +
         "\nUncounted strings count: " +
         std::to_string(m_uncString->getValue()) +
         "\nSerialized vec count: " +
         std::to_string(m_serVec->getValue()) +
         "\nSerialized dict count: " +
         std::to_string(m_serDict->getValue()) +
         "\nSerialized keyset count: " +
         std::to_string(m_serKeyset->getValue()) +
         "\nAPC vec count: " +
         std::to_string(m_apcVec->getValue()) +
         "\nAPC dict count: " +
         std::to_string(m_apcDict->getValue()) +
         "\nAPC keyset count: " +
         std::to_string(m_apcKeyset->getValue()) +
         "\nUncounted vec count: " +
         std::to_string(m_uncVec->getValue()) +
         "\nUncounted dict count: " +
         std::to_string(m_uncDict->getValue()) +
         "\nUncounted keyset count: " +
         std::to_string(m_uncKeyset->getValue()) +
         "\nSerialized object count: " +
         std::to_string(m_serObject->getValue()) +
         "\nAPC object count: " +
         std::to_string(m_apcObject->getValue()) +
         "\nAPC rfunc count: " +
         std::to_string(m_apcRFunc->getValue()) +
         "\nAPC rclsmeth count: " +
         std::to_string(m_apcRClsMeth->getValue()) +
         "\add count: " +
         std::to_string(m_setValues->getValue()) +
         "\ndelete count: " +
         std::to_string(m_delValues->getValue()) +
         "\nreplaced count: " +
         std::to_string(m_replValues->getValue()) +
         "\nexpired count: " +
         std::to_string(m_expValues->getValue()) +
         "\n";
}

void APCDetailedStats::collectStats(
    std::map<const StringData*, int64_t>& stats) const {
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeUncounted.get(),
                                              m_uncounted->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCString.get(),
                                              m_apcString->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeUncountedString.get(),
                                              m_uncString->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeSerVec.get(),
                                              m_serVec->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeSerDict.get(),
                                              m_serDict->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeSerKeyset.get(),
                                              m_serKeyset->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCVec.get(),
                                              m_apcVec->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCDict.get(),
                                              m_apcDict->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCKeyset.get(),
                                              m_apcKeyset->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typUncountedVec.get(),
                                              m_uncVec->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typUncountedDict.get(),
                                              m_uncDict->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typUncountedKeyset.get(),
                                              m_uncKeyset->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeSerObject.get(),
                                              m_serObject->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCObject.get(),
                                              m_apcObject->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCRFunc.get(),
                                              m_apcRFunc->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_typeAPCRClsMeth.get(),
                                              m_apcRClsMeth->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_setValueCount.get(),
                                              m_setValues->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_deleteValuesCount.get(),
                                              m_delValues->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_replacedValueCount.get(),
                                              m_replValues->getValue()));
  stats.insert(
        std::pair<const StringData*, int64_t>(s_expiredValueCount.get(),
                                              m_expValues->getValue()));
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

ServiceData::ExportedCounter*
APCDetailedStats::counterFor(const APCHandle* handle) {
  switch (handle->kind()) {
    case APCKind::Uninit:
    case APCKind::Null:
    case APCKind::Bool:
    case APCKind::Int:
    case APCKind::Double:
    case APCKind::PersistentClass:
    case APCKind::ClassEntity:
    case APCKind::PersistentFunc:
    case APCKind::PersistentClsMeth:
    case APCKind::FuncEntity:
    case APCKind::LazyClass:
    case APCKind::ClsMeth:
    case APCKind::StaticArray:
    case APCKind::StaticBespoke:
    case APCKind::StaticString:
      return m_uncounted;

    case APCKind::UncountedString:
      return m_uncString;

    case APCKind::SharedString:
      return m_apcString;

    case APCKind::UncountedArray:
    case APCKind::UncountedBespoke: {
      switch (handle->type()) {
        case KindOfPersistentVec:    return m_uncVec;
        case KindOfPersistentDict:   return m_uncDict;
        case KindOfPersistentKeyset: return m_uncKeyset;
        default: always_assert(false);
      }
    }

    case APCKind::SerializedVec:
      return m_serVec;

    case APCKind::SerializedDict:
      return m_serDict;

    case APCKind::SerializedKeyset:
      return m_serKeyset;

    case APCKind::SharedVec:
    case APCKind::SharedLegacyVec:
      return m_apcVec;

    case APCKind::SharedDict:
    case APCKind::SharedLegacyDict:
      return m_apcDict;

    case APCKind::SharedKeyset:
      return m_apcKeyset;

    case APCKind::SerializedObject:
      return m_serObject;

    case APCKind::SharedObject:
    case APCKind::SharedCollection:
      return m_apcObject;

    case APCKind::RFunc:
      return m_apcRFunc;

    case APCKind::RClsMeth:
      return m_apcRClsMeth;
  }
  not_reached();
}

void APCDetailedStats::addType(const APCHandle* handle) {
  counterFor(handle)->increment();
}

void APCDetailedStats::removeType(const APCHandle* handle) {
  counterFor(handle)->decrement();
}

///////////////////////////////////////////////////////////////////////////////

}
