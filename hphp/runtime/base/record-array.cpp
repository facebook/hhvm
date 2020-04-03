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
  assertx(isRecordArrayKind());
  assertx(checkCount());
  assertx(m_pos == 0);
  DEBUG_ONLY auto const extra = extraFieldMap();
  assertx(extra->checkInvariants());
  assertx(extra->getSize() == extra->iterLimit());
  return true;
}

RecordArray* RecordArray::asRecordArray(ArrayData* in) {
  assertx(in->isRecordArrayKind());
  auto ad = static_cast<RecordArray*>(in);
  assertx(ad->checkInvariants());
  return ad;
}

const RecordArray* RecordArray::asRecordArray(const ArrayData* in) {
  return asRecordArray(const_cast<ArrayData*>(in));
}

RecordArray* RecordArray::copyRecordArray(AllocMode mode) const {
  auto const ra = copyRecordBase(this, mode);
  auto const extra = extraFieldMap();
  if (mode == AllocMode::Request) {
    ra->extraFieldMap() = extra;
    extra->incRefCount();
  } else {
    ra->extraFieldMap() = MixedArray::asMixed(MixedArray::CopyStatic(extra));
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
  decRefArr(ad->extraFieldMap());
  tl_heap->objFree(ad, ad->heapSize());
  AARCH64_WALKABLE_FRAME();
}

Slot RecordArray::checkFieldForWrite(const StringData* key, TypedValue val) const {
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

void RecordArray::updateField(StringData* key, TypedValue val, Slot idx) {
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

ArrayData* RecordArray::SetStr(ArrayData* base, StringData* key, TypedValue val) {
  assertx(base->cowCheck() || base->notCyclic(val));
  auto ra = asRecordArray(base);
  auto const idx = ra->checkFieldForWrite(key, val);
  if (ra->cowCheck()) ra = ra->copyRecordArray(AllocMode::Request);
  ra->updateField(key, val, idx);
  return ra;
}

ArrayData* RecordArray::SetStrMove(ArrayData* base, StringData* key, TypedValue val) {
  auto const result = SetStr(base, key, val);
  if (result != base && base->decReleaseCheck()) RecordArray::Release(base);
  tvDecRefGen(val);
  return result;
}

size_t RecordArray::Vsize(const ArrayData*) { always_assert(false); }

MixedArray* RecordArray::ToMixedHeader(const RecordArray* old) {
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

MixedArray* RecordArray::ToMixed(const ArrayData* adIn) {
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
  MixedArray::IterateKV(extra, [&](TypedValue k, TypedValue v) {
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

ArrayData* RecordArray::SetInt(ArrayData* adIn, int64_t k, TypedValue v) {
  return PromoteForOp(adIn,
    [&] (MixedArray* mixed) { return mixed->addVal(k, v); },
    "SetInt"
  );
}

ArrayData* RecordArray::SetIntMove(ArrayData* adIn, int64_t k, TypedValue v) {
  auto const result = SetInt(adIn, k, v);
  assertx(result != adIn);
  if (adIn->decReleaseCheck()) RecordArray::Release(adIn);
  tvDecRefGen(v);
  return result;
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

TypedValue RecordArray::GetPosKey(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("GetPosKey");
  assertx(pos < ad->m_size);
  auto const ra = asRecordArray(ad);
  auto const rec = ra->record();
  if (pos < rec->numFields()) {
    auto const name = const_cast<StringData*>(rec->field(pos).name());
    return make_tv<KindOfPersistentString>(name);
  }
  auto const extra = ra->extraFieldMap();
  return MixedArray::GetPosKey(extra, pos - rec->numFields());
}

TypedValue RecordArray::GetPosVal(const ArrayData* ad, ssize_t pos) {
  raise_recordarray_unsupported_op_notice("GetPosVal");
  assertx(pos < ad->m_size);
  assertx(pos >= 0);
  auto const ra = asRecordArray(ad);
  auto const rec = ra->record();
  if (pos < rec->numFields()) {
    return *ra->rvalAt(pos);
  }
  return MixedArray::GetPosVal(ra->extraFieldMap(), pos - rec->numFields());
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

arr_lval RecordArray::LvalInt(ArrayData* ad, int64_t k, bool /*copy*/) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) { return MixedArray::LvalInt(mixed, k, false); },
    "LvalInt"
  );
}

arr_lval RecordArray::LvalStr(ArrayData* ad, StringData* k, bool copy) {
  auto ra = asRecordArray(ad);
  if (copy) ra = ra->copyRecordArray(AllocMode::Request);
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

ArrayData* RecordArray::EscalateForSort(ArrayData* ad, SortFunction) {
  return ToMixed(ad);
}

// Record-arrays always escalate to mixed-arrays before sorts.
void RecordArray::Ksort(ArrayData*, int, bool) {
  always_assert(false);
}

void RecordArray::Sort(ArrayData*, int, bool) {
  always_assert(false);
}

void RecordArray::Asort(ArrayData*, int, bool) {
  always_assert(false);
}

bool RecordArray::Uksort(ArrayData*, const Variant&) {
  always_assert(false);
  return false;
}

bool RecordArray::Usort(ArrayData*, const Variant&) {
  always_assert(false);
  return false;
}

bool RecordArray::Uasort(ArrayData*, const Variant&) {
  always_assert(false);
  return false;
}

ArrayData* RecordArray::Copy(const ArrayData* ad) {
  return asRecordArray(ad)->copyRecordArray(AllocMode::Request);
}

ArrayData* RecordArray::CopyStatic(const ArrayData* ad) {
  return asRecordArray(ad)->copyRecordArray(AllocMode::Static);
}

ArrayData* RecordArray::Append(ArrayData* ad, TypedValue v) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) { return mixed->AppendImpl(mixed, v, false); },
    "Append"
  );
}

ArrayData* RecordArray::PlusEq(ArrayData* ad, const ArrayData* elems) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return mixed->PlusEq(mixed, elems);
    },
    "PlusEq"
  );
}

ArrayData* RecordArray::Merge(ArrayData* ad, const ArrayData* elems) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return mixed->Merge(mixed, elems);
    },
    "Merge"
  );
}

ArrayData* RecordArray::Pop(ArrayData* ad, Variant& v) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return mixed->Pop(mixed, v);
    },
    "Pop"
  );
}

ArrayData* RecordArray::Dequeue(ArrayData* ad, Variant& v) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return mixed->Dequeue(mixed, v);
    },
    "Dequeue"
  );
}

ArrayData* RecordArray::Prepend(ArrayData* ad, TypedValue v) {
  return PromoteForOp(ad,
    [&] (MixedArray* mixed) {
      return mixed->Prepend(mixed, v);
    },
    "Prepend"
  );
}

void RecordArray::Renumber(ArrayData*) {
  throw_not_supported("Renumber", "Record-array");
}

void RecordArray::OnSetEvalScalar(ArrayData* ad) {
  auto const ra = asRecordArray(ad);
  auto const numRecFields = ra->record()->numFields();
  auto const fields = const_cast<TypedValue*>(ra->fieldVec());
  for (auto idx = 0; idx < numRecFields; ++idx) {
    tvAsVariant(&fields[idx]).setEvalScalar();
  }
  MixedArray::OnSetEvalScalar(ra->extraFieldMap());
}

ArrayData* RecordArray::ToPHPArray(ArrayData* ad, bool) {
  return ad;
}

///////////////////////////////////////////////////////////////////////////////

}
