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

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

RecordArray::RecordArray(const RecordDesc* record)
  : ArrayData(ArrayData::kRecordKind)
  , RecordBase(record)
{
  extraFieldMap() = MixedArray::asMixed(staticEmptyDArray());
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
  if (initSize < rec->numFields()) {
    raise_error(
      "All %lu declared fields of record-array %s must be initialized"
      "in the constructor; got %d fields",
      rec->numFields(), rec->name()->data(), initSize);
  }
  for (auto i = 0; i < rec->numFields(); ++i) {
    auto const declFieldName = rec->field(i).name();
    auto const key = keys[i];
    assertx(key->isStatic());
    if (declFieldName != key) {
      raise_error("Error initializing record-array %s:"
                  "field %d should be %s; got %s",
                  rec->name()->data(), i, declFieldName->data(), key->data());
    }
    assertx(declFieldName->same(key));
  }
  return newRecordImpl<RecordArray>(rec, initSize, keys, values);
}

RecordArray::ExtraFieldMapPtr& RecordArray::extraFieldMap() const {
  return *reinterpret_cast<ExtraFieldMapPtr*>(
      uintptr_t(this + 1) + fieldSize(m_record));
}

bool RecordArray::checkInvariants() const {
  assertx(isRecordArray());
  assertx(checkCount());
  assertx(m_pos == 0);
  DEBUG_ONLY auto const extra = extraFieldMap();
  assertx(extra->checkInvariants());
  assertx(extra->getSize() == extra->iterLimit());
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
  auto const extra = extraFieldMap();
  ra->extraFieldMap() = extra;
  extra->incRefCount();
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
  decRefArr(ad->extraFieldMap());
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
    auto& extra = extraFieldMap();
    if (!MixedArray::ExistsStr(extra, key)) ++m_size;
    auto const newExtra =
      MixedArray::asMixed(MixedArray::SetStr(extra, key, val));
    if (extra != newExtra) {
      decRefArr(extra);
      extra = newExtra;
    }
  }
}

tv_rval RecordArray::NvGetStr(const ArrayData* base, const StringData* key) {
  auto const ra = asRecordArray(base);
  auto const idx = ra->record()->lookupField(key);
  if (idx != kInvalidSlot) return ra->rvalAt(idx);
  auto const extra = ra->extraFieldMap();
  return MixedArray::NvGetStr(extra, key);
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

MixedArray* RecordArray::ToMixedHeader(RecordArray* old) {
  assertx(old->checkInvariants());
  auto const extra = old->extraFieldMap();
  auto const keyType = extra->empty() ?
                       MixedArrayKeys::packStaticStrsForAux() :
                       MixedArrayKeys::packStrsForAux();
  auto const aux = keyType | ArrayData::kNotDVArray;
  auto const oldSize = old->m_size;
  auto const scale = MixedArray::computeScaleFromSize(oldSize);
  auto const ad = MixedArray::reqAlloc(scale);
  ad->m_sizeAndPos = oldSize;
  ad->initHeader_16(HeaderKind::Mixed, OneReference, aux);
  ad->m_scale_used = scale | uint64_t{oldSize} << 32; // used=oldSize
  ad->m_nextKI = 0;

  assertx(ad->m_size == oldSize);
  assertx(ad->m_pos == old->m_pos);
  assertx(ad->kind() == ArrayData::kMixedKind);
  assertx(ad->isDArray() == old->isVArray());
  assertx(ad->hasExactlyOneRef());
  assertx(ad->m_used == oldSize);
  assertx(ad->m_scale == scale);
  assertx(ad->m_nextKI == 0);
  // Can't checkInvariants yet, since we haven't populated the payload.
  return ad;
}

MixedArray* RecordArray::ToMixed(ArrayData* adIn) {
  auto const old = asRecordArray(adIn);
  auto const ad = ToMixedHeader(old);
  auto const mask = ad->mask();
  auto dstData = ad->data();
  auto const dstHash = ad->initHash(ad->scale());
  auto const rec = old->record();
  auto i = 0;
  for (; i < rec->numFields(); ++i) {
    auto const& field = rec->field(i);
    auto const k = field.name();
    assertx(k->isStatic());
    auto const h = k->hash();
    dstData->setStaticKey(const_cast<StringData*>(k), h);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    auto const& val = old->rvalAt(i);
    tvDup(*val, dstData->data);
    ++dstData;
  }
  // If a record-array has non-empty ExtraFieldMap, we conservatively set
  // MixedArrayKeys to have  on-static keys in ToMixedHeader.
  auto const extra = old->extraFieldMap();
  MixedArray::IterateKV(extra, [&](Cell k, TypedValue v) {
    auto const kstr = k.m_data.pstr;
    auto const h = kstr->hash();
    dstData->setStrKey(kstr, h);
    *ad->findForNewInsert(dstHash, mask, h) = i;
    tvDup(v, dstData->data);
    ++dstData;
    ++i;
  });
  assertx(ad->checkInvariants());
  assertx(ad->hasExactlyOneRef());
  return ad;
}

tv_rval RecordArray::NvGetInt(const ArrayData*, int64_t key) {
  // RecordArrays may never have int keys.
  // Setting int keys escalate them to mixed arrays
  return nullptr;
}

namespace {
template <typename Op>
auto PromoteForOp(ArrayData* ad, Op op, const std::string& opname) {
  raise_recordarray_promotion_notice(opname);
  auto const mixed = RecordArray::ToMixed(ad);
  return op(mixed);
}
}

ArrayData* RecordArray::SetInt(ArrayData* adIn, int64_t k, Cell v) {
  return PromoteForOp(adIn,
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); },
    "SetInt"
  );
}

ssize_t RecordArray::NvGetIntPos(const ArrayData* ad, int64_t) {
  raise_recordarray_unsupported_op_notice("NvGetIntPos");
  return ad->m_size;
}

ssize_t RecordArray::NvGetStrPos(const ArrayData* ad, const StringData* key) {
  raise_recordarray_unsupported_op_notice("NvGetStrPos");
  auto const ra = asRecordArray(ad);
  auto const idx = ra->record()->lookupField(key);
  if (LIKELY(idx != kInvalidSlot)) return idx;
  auto const extra = ra->extraFieldMap();
  auto const posInExtra = MixedArray::NvGetStrPos(extra, key);
  return ra->record()->numFields() + posInExtra;
}

Cell RecordArray::NvGetKey(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("NvGetKey");
  assertx(pos < ad->m_size);
  auto const ra = asRecordArray(ad);
  auto const rec = ra->record();
  if (pos < rec->numFields()) {
    auto const name = const_cast<StringData*>(rec->field(pos).name());
    return make_tv<KindOfPersistentString>(name);
  }
  auto const extra = ra->extraFieldMap();
  return MixedArray::NvGetKey(extra, pos - rec->numFields());
}

bool RecordArray::IsVectorData(const ArrayData*) {
  return false;
}

bool RecordArray::ExistsInt(const ArrayData*, int64_t) {
  return false;
}

bool RecordArray::ExistsStr(const ArrayData* ad, const StringData* key) {
  auto const ra = asRecordArray(ad);
  auto const idx = ra->record()->lookupField(key);
  if (idx != kInvalidSlot) return true;
  return MixedArray::ExistsStr(ra->extraFieldMap(), key);
}

arr_lval RecordArray::LvalForceNew(ArrayData* ad, bool /*copy*/) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) { return MixedArray::LvalForceNew(mixed, false); },
    "LvalNew"
  );
}

arr_lval RecordArray::LvalInt(ArrayData* ad, int64_t k, bool /*copy*/) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) { return MixedArray::LvalInt(mixed, k, false); },
    "LvalInt"
  );
}

arr_lval RecordArray::LvalStr(ArrayData* ad, StringData* k, bool copy) {
  auto ra = asRecordArray(ad);
  if (copy) ra = ra->copyRecordArray();
  auto const rec = ra->record();
  auto const idx = rec->lookupField(k);
  if (idx != kInvalidSlot) return arr_lval {ra, ra->lvalAt(idx)};
  auto& extra = ra->extraFieldMap();
  if (!MixedArray::ExistsStr(extra, k)) ra->m_size++;
  auto const ret = MixedArray::LvalStr(extra, k, extra->cowCheck());
  auto const newExtra = MixedArray::asMixed(ret.arr);
  if (extra != newExtra) {
    decRefArr(extra);
    extra = newExtra;
  }
  return arr_lval {ra, tv_lval(ret)};
}

arr_lval RecordArray::LvalSilentStr(ArrayData* ad, StringData* k, bool copy) {
  auto ra = asRecordArray(ad);
  if (copy) ra = ra->copyRecordArray();
  auto const rec = ra->record();
  auto const idx = rec->lookupField(k);
  if (idx != kInvalidSlot) return arr_lval {ra, ra->lvalAt(idx)};
  auto& extra = ra->extraFieldMap();
  auto const ret = MixedArray::LvalSilentStr(extra, k, extra->cowCheck());
  auto const newExtra = MixedArray::asMixed(ret.arr);
  if (extra != newExtra) {
    decRefArr(extra);
    extra = newExtra;
  }
  return arr_lval {ra, tv_lval(ret)};
}

arr_lval RecordArray::LvalSilentInt(ArrayData* ad, int64_t, bool) {
  return arr_lval {ad, nullptr};
}

ArrayData* RecordArray::RemoveInt(ArrayData* ad, int64_t) {
  // Record-arrays do not have int keys
  return ad;
}

ArrayData* RecordArray::RemoveStr(ArrayData* ad, const StringData* k) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return MixedArray::RemoveStrImpl(mixed, k, false);
    },
    "RemoveStr"
  );
}

ssize_t RecordArray::IterBegin(const ArrayData*) {
  raise_recordarray_unsupported_op_notice("IterBegin");
  return 0;
}

ssize_t RecordArray::IterEnd(const ArrayData* ad) {
  raise_recordarray_unsupported_op_notice("IterEnd");
  return ad->m_size;
}

ssize_t RecordArray::IterLast(const ArrayData* ad) {
  raise_recordarray_unsupported_op_notice("IterLast");
  auto const numFields = ad->m_size;
  return numFields ? numFields - 1 : 0;
}

ssize_t RecordArray::IterAdvance(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("IterAdvance");
  auto const numFields = ad->m_size;
  assertx(pos >= 0);
  assertx(pos <= numFields);
  return (pos < numFields) ? pos + 1 : pos;
}

ssize_t RecordArray::IterRewind(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("IterRewind");
  assertx(pos >= 0);
  assertx(pos <= ad->m_size);
  return (pos > 0) ? pos - 1  : pos;
}

tv_rval RecordArray::GetValueRef(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("GetValueRef");
  assertx(pos < ad->m_size);
  assertx(pos >= 0);
  auto const ra = asRecordArray(ad);
  auto const rec = ra->record();
  if (pos < rec->numFields()) {
    return ra->rvalAt(pos);
  }
  return MixedArray::GetValueRef(ra->extraFieldMap(), pos - rec->numFields());
}

// TODO: T47449944: methods below this are stubs

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

///////////////////////////////////////////////////////////////////////////////

}
