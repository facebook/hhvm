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

#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/record-array.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {
RecordArray::RecordArray(const RecordDesc* record)
  : ArrayData(ArrayData::kRecordKind)
  , RecordBase(record) {
  new (const_cast<ExtraFieldMap*>(extraFieldMap())) ExtraFieldMap();
  auto const sizeIdx = MemoryManager::size2Index(sizeWithFields(record));
  m_aux16 = static_cast<uint16_t>(sizeIdx) << 8;
  m_size = record->numFields();
  static_assert(sizeof(RecordArray) == sizeof(RecordBase) + sizeof(ArrayData),
                "RecordArray must not have any fields of its own");
}

size_t RecordArray::heapSize() const {
  return PackedArray::heapSize(this);
}

inline bool RecordArray::kindIsValid() const {
  return m_kind == HeaderKind::RecordArray;
}

RecordArray* RecordArray::newRecordArray(const RecordDesc* rec,
                                         uint32_t initSize,
                                         const StringData* const* keys,
                                         const TypedValue* values) {
  if (!RuntimeOption::EvalHackRecordArrays) {
    raise_error("Record Arrays are not supported");
  }
  return newRecordImpl<RecordArray>(rec, initSize, keys, values);
}

const RecordArray::ExtraFieldMap* RecordArray::extraFieldMap() const {
  return reinterpret_cast<const ExtraFieldMap*>(
      uintptr_t(this + 1) + fieldSize(m_record));
}

bool RecordArray::checkInvariants() const {
  assertx(checkCount());
  return true;
}

RecordArray* RecordArray::asRecordArray(ArrayData* in) {
  assertx(in->isRecordArray());
  auto ad = static_cast<RecordArray*>(in);
  assertx(ad->checkInvariants());
  return ad;
}

const RecordArray* RecordArray::asRecordArray(const ArrayData* in) {
  return asRecordArray(const_cast<ArrayData*>(in));
}

RecordArray* RecordArray::copyRecordArray() const {
  auto const ra = copyRecordBase(this);
  auto const extra = new (const_cast<ExtraFieldMap*>(ra->extraFieldMap()))
    ExtraFieldMap(*extraFieldMap());
  for (auto& p : *extra) {
    tvIncRefGen(p.second);
  }
  return ra;
}

void RecordArray::Release(ArrayData* in) {
  in->fixCountForRelease();
  assertx(in->isRefCounted());
  assertx(in->hasExactlyOneRef());
  auto ad = asRecordArray(in);
  auto const numRecFields = ad->record()->numFields();
  auto const recFields = ad->fieldVec();
  for (auto idx = 0; idx < numRecFields; ++idx) {
    tvDecRefGen(recFields[idx]);
  }
  auto const extraMap = ad->extraFieldMap();
  for (auto& p : *extraMap) {
    // String keys should be dec-ref'd in map destructor.
    tvDecRefGen(p.second);
  }
  extraMap->~ExtraFieldMap();
  tl_heap->objFree(ad, ad->heapSize());
  AARCH64_WALKABLE_FRAME();
}

Slot RecordArray::checkFieldForWrite(const StringData* key, Cell val) const {
  auto const rec = record();
  auto const idx = rec->lookupField(key);
  if (idx != kInvalidSlot) {
    auto const& field = rec->field(idx);
    auto const& tc = field.typeConstraint();
    if (tc.isCheckable()) {
      tc.verifyRecField(&val, rec->name(), field.name());
    }
  }
  return idx;
}

void RecordArray::updateField(StringData* key, Cell val, Slot idx) {
  if (idx != kInvalidSlot) {
    assertx(idx == record()->lookupField(key));
    auto const& tv = lvalAt(idx);
    tvSet(val, tv);
  } else {
    auto const extraMap = const_cast<ExtraFieldMap*>(extraFieldMap());
    auto const keyStr = String::attach(key);
    auto it = extraMap->find(keyStr);
    if (it == extraMap->end()) {
      ++m_size;
      extraMap->emplace(keyStr, val);
    } else {
      tvSet(val, it->second);
    }
  }
}

tv_rval RecordArray::NvGetStr(const ArrayData* base, const StringData* key) {
  auto const ra = asRecordArray(base);
  auto const idx = ra->record()->lookupField(key);
  if (idx != kInvalidSlot) return ra->rvalAt(idx);
  auto const extraMap = ra->extraFieldMap();
  auto it = extraMap->find(String::attach(const_cast<StringData*>(key)));
  if (it != extraMap->end()) return tv_rval(&it->second);
  return nullptr;
}

ArrayData* RecordArray::SetStrInPlace(ArrayData* base,
                                      StringData* key, Cell val) {
  assertx(base->notCyclic(val));
  auto const ra = asRecordArray(base);
  auto const idx = ra->checkFieldForWrite(key, val);
  ra->updateField(key, val, idx);
  return ra;
}

ArrayData* RecordArray::SetStr(ArrayData* base, StringData* key, Cell val) {
  assertx(base->cowCheck() || base->notCyclic(val));
  auto ra = asRecordArray(base);
  auto const idx = ra->checkFieldForWrite(key, val);
  if (ra->cowCheck()) ra = ra->copyRecordArray();
  ra->updateField(key, val, idx);
  return ra;
}

size_t RecordArray::Vsize(const ArrayData*) { always_assert(false); }

// TODO: T47449944: methods below this are stubs
tv_rval RecordArray::NvGetInt(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

tv_rval RecordArray::NvTryGetInt(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::NvGetIntPos(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::NvGetStrPos(const ArrayData*, const StringData*) {
  throw_not_implemented("This method on RecordArray");
}

Cell RecordArray::NvGetKey(const ArrayData*, ssize_t) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetInt(ArrayData*, int64_t, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetIntInPlace(ArrayData*, int64_t, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetWithRefInt(ArrayData*, int64_t, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetWithRefIntInPlace(ArrayData*, int64_t, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetWithRefStr(ArrayData*, StringData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetWithRefStrInPlace(ArrayData*, StringData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

tv_rval RecordArray::GetValueRef(const ArrayData*, ssize_t) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::IsVectorData(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::ExistsInt(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::ExistsStr(const ArrayData*, const StringData*) {
  throw_not_implemented("This method on RecordArray");
}

arr_lval RecordArray::LvalInt(ArrayData*, int64_t, bool) {
  throw_not_implemented("This method on RecordArray");
}

arr_lval RecordArray::LvalStr(ArrayData*, StringData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

arr_lval RecordArray::LvalNew(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::RemoveInt(ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::RemoveIntInPlace(ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::RemoveStr(ArrayData*, const StringData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::RemoveStrInPlace(ArrayData*, const StringData*) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::IterEnd(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::IterBegin(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::IterLast(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::IterAdvance(const ArrayData*, ssize_t) {
  throw_not_implemented("This method on RecordArray");
}

ssize_t RecordArray::IterRewind(const ArrayData*, ssize_t) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::EscalateForSort(ArrayData*, SortFunction) {
  throw_not_implemented("This method on RecordArray");
}

void RecordArray::Ksort(ArrayData*, int, bool) {
  throw_not_implemented("This method on RecordArray");
}

void RecordArray::Sort(ArrayData*, int, bool) {
  throw_not_implemented("This method on RecordArray");
}

void RecordArray::Asort(ArrayData*, int, bool) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::Uksort(ArrayData*, const Variant&) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::Usort(ArrayData*, const Variant&) {
  throw_not_implemented("This method on RecordArray");
}

bool RecordArray::Uasort(ArrayData*, const Variant&) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Copy(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::CopyStatic(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Append(ArrayData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::AppendInPlace(ArrayData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::AppendWithRef(ArrayData*, TypedValue) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::AppendWithRefInPlace(ArrayData*, TypedValue) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::PlusEq(ArrayData*, const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Merge(ArrayData*, const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Pop(ArrayData*, Variant&) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Dequeue(ArrayData*, Variant&) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Prepend(ArrayData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

void RecordArray::Renumber(ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

void RecordArray::OnSetEvalScalar(ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::Escalate(const ArrayData*) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToPHPArray(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToPHPArrayIntishCast(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToDict(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToVec(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToKeyset(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToVArray(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::ToDArray(ArrayData*, bool) {
  throw_not_implemented("This method on RecordArray");
}
} // namespace HPHP
