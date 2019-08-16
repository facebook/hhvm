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

#include "hphp/runtime/base/record-array.h"
#include "hphp/runtime/base/tv-refcount.h"

namespace HPHP {
RecordArray::RecordArray(const RecordDesc* record)
  : ArrayData(ArrayData::kRecordKind)
  , RecordBase(record) {
  new (const_cast<ExtraFieldMap*>(extraFieldMap())) ExtraFieldMap();
  static_assert(sizeof(RecordArray) == sizeof(RecordBase) + sizeof(ArrayData),
                "RecordArray must not have any fields of its own");
}

size_t RecordArray::heapSize() const {
  return sizeWithFields(m_record);
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

// TODO: T47449944: methods below this are stubs
tv_rval RecordArray::NvGetInt(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

tv_rval RecordArray::NvTryGetInt(const ArrayData*, int64_t) {
  throw_not_implemented("This method on RecordArray");
}

tv_rval RecordArray::NvGetStr(const ArrayData*, const StringData*) {
  throw_not_implemented("This method on RecordArray");
}

tv_rval RecordArray::NvTryGetStr(const ArrayData*, const StringData*) {
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

ArrayData* RecordArray::SetStr(ArrayData*, StringData*, Cell) {
  throw_not_implemented("This method on RecordArray");
}

ArrayData* RecordArray::SetStrInPlace(ArrayData*, StringData*, Cell) {
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

size_t RecordArray::Vsize(const ArrayData*) {
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

ArrayData* RecordArray::ToShape(ArrayData*, bool) {
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
