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

#include "hphp/runtime/base/bespoke/monotype-vec.h"

#include "hphp/runtime/base/bespoke/entry-types.h"
#include "hphp/runtime/base/bespoke/bespoke-top.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/util/word-mem.h"

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

namespace {

uint16_t packSizeIndexAndAuxBits(uint8_t idx, uint8_t aux) {
  return (static_cast<uint16_t>(idx) << 8) | aux;
}

inline constexpr uint32_t sizeClassParams2MonotypeVecCapacity(
  size_t index,
  size_t lg_grp,
  size_t lg_delta,
  size_t ndelta
) {
  static_assert(sizeof(MonotypeVec) <=
                kSizeIndex2Size[MonotypeVec::kMinSizeIndex]);

  if (index < MonotypeVec::kMinSizeIndex) return 0;
  return (((size_t{1} << lg_grp) + (ndelta << lg_delta)) - sizeof(MonotypeVec))
      / sizeof(Value);
}

alignas(64) constexpr uint32_t kSizeIndex2MonotypeVecCapacity[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  sizeClassParams2MonotypeVecCapacity(index, lg_grp, lg_delta, ndelta),
  SIZE_CLASSES
#undef SIZE_CLASS
};

//////////////////////////////////////////////////////////////////////////////
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Static initialization
//////////////////////////////////////////////////////////////////////////////

namespace {

using StaticVec = std::aligned_storage<sizeof(EmptyMonotypeVec), 16>::type;

StaticVec s_emptyMonotypeVec;
StaticVec s_emptyMonotypeVArray;
StaticVec s_emptyMonotypeVecMarked;
StaticVec s_emptyMonotypeVArrayMarked;

static_assert(sizeof(DataType) == 1);
constexpr LayoutIndex kBaseLayoutIndex = {1 << 8};
auto const s_monotypeVecVtable = fromArray<MonotypeVec>();
auto const s_emptyMonotypeVecVtable = fromArray<EmptyMonotypeVec>();

constexpr LayoutIndex getLayoutIndex(DataType type) {
  return LayoutIndex{uint16_t(kBaseLayoutIndex.raw + uint8_t(type))};
}

constexpr LayoutIndex getEmptyLayoutIndex() {
  return getLayoutIndex(kInvalidDataType);
}

}

MonotypeVecLayout::MonotypeVecLayout(LayoutIndex index, DataType type)
  : ConcreteLayout(index, makeDescription(type), &s_monotypeVecVtable,
                   {BespokeTop::GetLayoutIndex()}, /*liveable=*/ true)
  , m_fixedType(type)
{}

std::string MonotypeVecLayout::makeDescription(DataType type) {
  return folly::sformat("MonotypeVec<{}>", tname(type));
}

EmptyMonotypeVecLayout::EmptyMonotypeVecLayout(LayoutIndex index)
  : ConcreteLayout(index, "MonotypeVec<Empty>", &s_emptyMonotypeVecVtable,
                   {BespokeTop::GetLayoutIndex()}, /*liveable=*/ true)
{}

void MonotypeVec::InitializeLayouts() {
  auto const base = Layout::ReserveIndices(1 << 8);
  always_assert(base == kBaseLayoutIndex);
  new EmptyMonotypeVecLayout(getEmptyLayoutIndex());
#define DT(name, value)                                              \
  new MonotypeVecLayout(getLayoutIndex(KindOf##name), KindOf##name);
  DATATYPES
#undef DT

  auto const init = [&](EmptyMonotypeVec* ead, HeaderKind hk, bool legacy) {
    auto const aux =
      packSizeIndexAndAuxBits(0, legacy ? ArrayData::kLegacyArray : 0);
    ead->initHeader_16(hk, StaticValue, aux);
    ead->m_size = 0;
    ead->setLayoutIndex(getEmptyLayoutIndex());
    assertx(ead->checkInvariants());
  };

  init(EmptyMonotypeVec::GetVec(false), HeaderKind::BespokeVec, false);
  init(EmptyMonotypeVec::GetVArray(false), HeaderKind::BespokeVArray, false);
  init(EmptyMonotypeVec::GetVec(true), HeaderKind::BespokeVec, true);
  init(EmptyMonotypeVec::GetVArray(true), HeaderKind::BespokeVArray, true);
}

//////////////////////////////////////////////////////////////////////////////
// EmptyMonotypeVec
//////////////////////////////////////////////////////////////////////////////

EmptyMonotypeVec* EmptyMonotypeVec::As(ArrayData* ad) {
  auto const ead = reinterpret_cast<EmptyMonotypeVec*>(ad);
  assertx(ead->checkInvariants());
  return ead;
}

const EmptyMonotypeVec* EmptyMonotypeVec::As(const ArrayData* ad) {
  return EmptyMonotypeVec::As(const_cast<ArrayData*>(ad));
}

EmptyMonotypeVec* EmptyMonotypeVec::GetVec(bool legacy) {
  auto const src = legacy ? &s_emptyMonotypeVecMarked : &s_emptyMonotypeVec;
  return reinterpret_cast<EmptyMonotypeVec*>(src);
}

EmptyMonotypeVec* EmptyMonotypeVec::GetVArray(bool legacy) {
  auto const src = legacy ? &s_emptyMonotypeVArrayMarked
                          : &s_emptyMonotypeVArray;
  return reinterpret_cast<EmptyMonotypeVec*>(src);
}

bool EmptyMonotypeVec::checkInvariants() const {
  assertx(isStatic());
  assertx(kindIsValid());
  assertx(size() == 0);
  assertx(isVecType() || isVArray());
  assertx(layoutIndex() == getEmptyLayoutIndex());

  return true;
}

size_t EmptyMonotypeVec::HeapSize(const EmptyMonotypeVec* ead) {
  return sizeof(EmptyMonotypeVec);
}

void EmptyMonotypeVec::Scan(const EmptyMonotypeVec* ead, type_scan::Scanner&) {
}

ArrayData* EmptyMonotypeVec::EscalateToVanilla(const EmptyMonotypeVec* ead,
                                               const char* reason) {
  auto const legacy = ead->isLegacyArray();
  return ead->isVecType()
    ? (legacy ? staticEmptyMarkedVec() : staticEmptyVec())
    : (legacy ? staticEmptyMarkedVArray() : staticEmptyVArray());
}

void EmptyMonotypeVec::ConvertToUncounted(EmptyMonotypeVec*,
                                          DataWalker::PointerMap*) {
}

void EmptyMonotypeVec::ReleaseUncounted(EmptyMonotypeVec* ead) {
}

void EmptyMonotypeVec::Release(EmptyMonotypeVec* ead) {
  // All EmptyMonotypeVecs are static, and should therefore never be released.
  always_assert(false);
}

bool EmptyMonotypeVec::IsVectorData(const EmptyMonotypeVec*) {
  return true;
}

TypedValue EmptyMonotypeVec::NvGetInt(const EmptyMonotypeVec*, int64_t) {
  return make_tv<KindOfUninit>();
}

TypedValue EmptyMonotypeVec::NvGetStr(const EmptyMonotypeVec*,
                                    const StringData*) {
  return make_tv<KindOfUninit>();
}

TypedValue EmptyMonotypeVec::GetPosKey(const EmptyMonotypeVec*, ssize_t) {
  always_assert(false);
}

TypedValue EmptyMonotypeVec::GetPosVal(const EmptyMonotypeVec*, ssize_t) {
  always_assert(false);
}

ssize_t EmptyMonotypeVec::GetIntPos(const EmptyMonotypeVec* ead, int64_t k) {
  return 0;
}

ssize_t EmptyMonotypeVec::GetStrPos(const EmptyMonotypeVec* ead,
                                    const StringData*) {
  return ead->size();
}

arr_lval EmptyMonotypeVec::LvalInt(EmptyMonotypeVec* ead, int64_t k) {
  throwOOBArrayKeyException(k, ead);
}

arr_lval EmptyMonotypeVec::LvalStr(EmptyMonotypeVec* ead, StringData* k) {
  throwInvalidArrayKeyException(k, ead);
}

tv_lval EmptyMonotypeVec::ElemInt(
    tv_lval lval, int64_t k, bool throwOnMissing) {
  if (throwOnMissing) throwOOBArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

tv_lval EmptyMonotypeVec::ElemStr(
    tv_lval lval, StringData* k, bool throwOnMissing) {
  if (throwOnMissing) throwInvalidArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

ArrayData* EmptyMonotypeVec::SetInt(EmptyMonotypeVec* eadIn, int64_t k,
                                    TypedValue) {
  throwOOBArrayKeyException(k, eadIn);
}

ArrayData* EmptyMonotypeVec::SetStr(EmptyMonotypeVec* eadIn, StringData* k,
                                    TypedValue) {
  throwInvalidArrayKeyException(k, eadIn);
}

ArrayData* EmptyMonotypeVec::SetIntMove(EmptyMonotypeVec* eadIn, int64_t k,
                                        TypedValue) {
  throwOOBArrayKeyException(k, eadIn);
}

ArrayData* EmptyMonotypeVec::SetStrMove(EmptyMonotypeVec* eadIn, StringData* k,
                                        TypedValue) {
  throwInvalidArrayKeyException(k, eadIn);
}

ArrayData* EmptyMonotypeVec::RemoveInt(EmptyMonotypeVec* eadIn, int64_t) {
  return eadIn;
}

ArrayData* EmptyMonotypeVec::RemoveStr(EmptyMonotypeVec* eadIn,
                                       const StringData*) {
  return eadIn;
}

ssize_t EmptyMonotypeVec::IterBegin(const EmptyMonotypeVec*) {
  return 0;
}

ssize_t EmptyMonotypeVec::IterLast(const EmptyMonotypeVec*) {
  return 0;
}

ssize_t EmptyMonotypeVec::IterEnd(const EmptyMonotypeVec*) {
  return 0;
}

ssize_t EmptyMonotypeVec::IterAdvance(const EmptyMonotypeVec*, ssize_t) {
  return 0;
}

ssize_t EmptyMonotypeVec::IterRewind(const EmptyMonotypeVec*, ssize_t pos) {
  return 0;
}

ArrayData* EmptyMonotypeVec::Append(EmptyMonotypeVec* ead, TypedValue v) {
  auto const mad = MonotypeVec::MakeReserve(
      ead->m_kind, ead->isLegacyArray(), 1, type(v));
  auto const res = MonotypeVec::Append(mad, v);
  assertx(mad == res);
  return res;
}

ArrayData* EmptyMonotypeVec::AppendMove(EmptyMonotypeVec* ead, TypedValue v) {
  auto const dt = dt_modulo_persistence(type(v));
  auto const mad = MonotypeVec::MakeReserve(
      ead->m_kind, ead->isLegacyArray(), 1, dt);
  auto const res = MonotypeVec::AppendMove(mad, v);
  assertx(mad == res);
  return res;
}

ArrayData* EmptyMonotypeVec::Pop(EmptyMonotypeVec* ead, Variant& value) {
  value = uninit_null();
  return ead;
}

ArrayData* EmptyMonotypeVec::ToDVArray(EmptyMonotypeVec* eadIn, bool copy) {
  assertx(copy);
  assertx(eadIn->isVecType());
  return GetVArray(eadIn->isLegacyArray());
}

ArrayData* EmptyMonotypeVec::ToHackArr(EmptyMonotypeVec* eadIn, bool copy) {
  assertx(copy);
  assertx(eadIn->isVArray());
  return GetVec(false);
}

ArrayData* EmptyMonotypeVec::PreSort(EmptyMonotypeVec* ead, SortFunction sf) {
  always_assert(false);
}

ArrayData* EmptyMonotypeVec::PostSort(EmptyMonotypeVec* ead, ArrayData* vad) {
  always_assert(false);
}

ArrayData* EmptyMonotypeVec::SetLegacyArray(EmptyMonotypeVec* eadIn,
                                            bool copy, bool legacy) {
  if (eadIn->isVecType()) {
    return GetVec(legacy);
  } else {
    assertx(eadIn->isVArray());
    return GetVArray(legacy);
  }
}


//////////////////////////////////////////////////////////////////////////////
// MonotypeVec
//////////////////////////////////////////////////////////////////////////////

template <typename CountableFn, typename MaybeCountableFn>
void MonotypeVec::forEachCountableValue(CountableFn c, MaybeCountableFn mc) {
  auto const dt = type();
  if (isRefcountedType(dt)) {
    if (static_cast<data_type_t>(dt) & kHasPersistentMask) {
      for (auto i = 0; i < m_size; i++) {
        mc(dt, valueRefUnchecked(i).pcnt);
      }
    } else {
      for (auto i = 0; i < m_size; i++) {
        c(dt, reinterpret_cast<Countable*>(valueRefUnchecked(i).pcnt));
      }
    }
  }
}

void MonotypeVec::decRefValues() {
  forEachCountableValue(
    [&](auto t, auto v) { if (v->decReleaseCheck()) destructorForType(t)(v); },
    [&](auto t, auto v) { if (v->decReleaseCheck()) destructorForType(t)(v); }
  );
}

void MonotypeVec::incRefValues() {
  forEachCountableValue(
    [&](auto t, auto v) { v->incRefCount(); },
    [&](auto t, auto v) { v->incRefCount(); }
  );
}

MonotypeVec* MonotypeVec::MakeReserve(
    HeaderKind hk, bool legacy, uint32_t capacity, DataType dt) {
  auto const bytes = sizeof(MonotypeVec) + capacity * sizeof(Value);
  auto const index = std::max(MemoryManager::size2Index(bytes), kMinSizeIndex);

  auto const mad = static_cast<MonotypeVec*>(tl_heap->objMallocIndex(index));
  auto const aux = packSizeIndexAndAuxBits(
      index, legacy ? ArrayData::kLegacyArray : 0);

  mad->initHeader_16(hk, OneReference, aux);
  mad->setLayoutIndex(getLayoutIndex(dt));
  mad->m_size = 0;

  assertx(mad->checkInvariants());
  return mad;
}

MonotypeVec* MonotypeVec::MakeFromVanilla(ArrayData* ad, DataType dt) {
  assertx(ad->hasVanillaPackedLayout());
  auto const kind = ad->isVArray() ? HeaderKind::BespokeVArray
                                   : HeaderKind::BespokeVec;
  auto result = MakeReserve(kind, ad->isLegacyArray(), ad->size(), dt);

  PackedArray::IterateVNoInc(ad, [&](auto v) {
    auto const next = Append(result, v);
    assertx(result == next);
    result = As(next);
  });

  if (ad->isStatic()) {
    auto const size = HeapSize(result);
    auto const copy = static_cast<MonotypeVec*>(
        RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size));
    memcpy16_inline(copy, result, size);
    auto const aux = packSizeIndexAndAuxBits(
      result->sizeIndex(), result->auxBits());
    copy->initHeader_16(kind, StaticValue, aux);
    Release(result);
    result = copy;
  }

  assertx(result->checkInvariants());
  return result;
}

Value* MonotypeVec::rawData() {
  return reinterpret_cast<Value*>(this + 1);
}

const Value* MonotypeVec::rawData() const {
  return const_cast<MonotypeVec*>(this)->rawData();
}

Value& MonotypeVec::valueRefUnchecked(uint32_t idx) {
  return rawData()[idx];
}

const Value& MonotypeVec::valueRefUnchecked(uint32_t idx) const {
  return const_cast<MonotypeVec*>(this)->valueRefUnchecked(idx);
}

TypedValue MonotypeVec::typedValueUnchecked(uint32_t idx) const {
  return { valueRefUnchecked(idx), type() };
}

uint8_t MonotypeVec::sizeIndex() const {
  return m_aux16 >> 8;
}

size_t MonotypeVec::capacity() const {
  return kSizeIndex2MonotypeVecCapacity[sizeIndex()];
}

MonotypeVec* MonotypeVec::copyHelper(uint8_t newSizeIndex, bool incRef) const {
  auto const mad =
    static_cast<MonotypeVec*>(tl_heap->objMallocIndex(newSizeIndex));

  // Copy elements 16 bytes at a time. We may copy an extra value this way,
  // but our heap allocations are in multiples of 16 bytes, so it is safe.
  assertx(HeapSize(this) % 16 == 0);
  auto const bytes = sizeof(MonotypeVec) + m_size * sizeof(Value);
  memcpy16_inline(mad, this, size_t(bytes + 15) & ~size_t(15));

  mad->initHeader_16(m_kind, OneReference,
                     packSizeIndexAndAuxBits(newSizeIndex, auxBits()));

  if (incRef) mad->incRefValues();

  return mad;
}

MonotypeVec* MonotypeVec::copy() const {
  return copyHelper(sizeIndex(), true);
}

MonotypeVec* MonotypeVec::grow() {
  auto const cow = cowCheck();
  auto const mad = copyHelper(sizeIndex() + kSizeClassesPerDoubling, cow);
  if (!cow) m_size = 0;
  return mad;
}

DataType MonotypeVec::type() const {
  return DataType(int8_t(m_extra_hi16 & 0xff));
}

MonotypeVec* MonotypeVec::prepareForInsert() {
  if (m_size == capacity()) return grow();
  if (cowCheck()) return copy();
  return this;
}

/*
 * Escalates the MonotypeVec to a vanilla PackedArray with the contents of the
 * current array and the specified capacity. The new array will match the
 * MonotypeVec in size, contents, and legacy bit status.
 */
ArrayData* MonotypeVec::escalateWithCapacity(size_t capacity) const {
  assertx(capacity >= size());
  auto const ad = isVecType() ? PackedArray::MakeReserveVec(capacity)
                              : PackedArray::MakeReserveVArray(capacity);
  for (uint32_t i = 0; i < size(); i++) {
    auto const tv = typedValueUnchecked(i);
    tvCopy(tv, PackedArray::LvalUncheckedInt(ad, i));
    tvIncRefGen(tv);
  }
  ad->setLegacyArrayInPlace(isLegacyArray());
  ad->m_size = size();
  return ad;
}

bool MonotypeVec::checkInvariants() const {
  assertx(kindIsValid());
  assertx(isVecType() || isVArray());
  assertx(size() <= capacity());
  assertx(sizeIndex() >= kMinSizeIndex);
  assertx(layoutIndex() != getEmptyLayoutIndex());
  assertx(layoutIndex() == getLayoutIndex(type()));
  assertx(isRealType(type()));
  if (size() > 0) {
    assertx(tvIsPlausible(typedValueUnchecked(0)));
  }

  return true;
}

MonotypeVec* MonotypeVec::As(ArrayData* ad) {
  auto const mad = reinterpret_cast<MonotypeVec*>(ad);
  assertx(mad->checkInvariants());
  return mad;
}

const MonotypeVec* MonotypeVec::As(const ArrayData* ad) {
  auto const mad = reinterpret_cast<const MonotypeVec*>(ad);
  assertx(mad->checkInvariants());
  return mad;
}

size_t MonotypeVec::HeapSize(const MonotypeVec* mad) {
  return MemoryManager::sizeIndex2Size(mad->sizeIndex());
}

void MonotypeVec::Scan(const MonotypeVec* mad, type_scan::Scanner& scan) {
  if (isRefcountedType(mad->type())) {
    static_assert(sizeof(MaybeCountable*) == sizeof(Value));
    scan.scan(mad->rawData()->pcnt, mad->size() * sizeof(Value));
  }
}

ArrayData* MonotypeVec::EscalateToVanilla(const MonotypeVec* mad,
                                          const char* reason) {
  return mad->escalateWithCapacity(mad->size());
}

void MonotypeVec::ConvertToUncounted(MonotypeVec* madIn,
                                     DataWalker::PointerMap* seen) {
  auto const oldType = madIn->type();
  for (uint32_t i = 0; i < madIn->size(); i++) {
    DataType dt = oldType;
    auto const lval = tv_lval(&dt, &madIn->rawData()[i]);
    ConvertTvToUncounted(lval, seen);
    assertx(equivDataTypes(dt, madIn->type()));
  }
  auto narrowedType = static_cast<data_type_t>(oldType) & kHasPersistentMask
    ? dt_with_persistence(oldType)
    : oldType;
  madIn->setLayoutIndex(getLayoutIndex(narrowedType));
}

void MonotypeVec::ReleaseUncounted(MonotypeVec* mad) {
  for (uint32_t i = 0; i < mad->size(); i++) {
    auto tv = mad->typedValueUnchecked(i);
    ReleaseUncountedTv(&tv);
  }
}

void MonotypeVec::Release(MonotypeVec* mad) {
  mad->fixCountForRelease();
  assertx(mad->isRefCounted());
  assertx(mad->hasExactlyOneRef());
  mad->decRefValues();
  tl_heap->objFreeIndex(mad, mad->sizeIndex());
}

bool MonotypeVec::IsVectorData(const MonotypeVec* mad) {
  return true;
}

TypedValue MonotypeVec::NvGetInt(const MonotypeVec* mad, int64_t k) {
  if (size_t(k) >= mad->size()) return make_tv<KindOfUninit>();
  return mad->typedValueUnchecked(k);
}

TypedValue MonotypeVec::NvGetStr(const MonotypeVec* mad, const StringData*) {
  return make_tv<KindOfUninit>();
}

TypedValue MonotypeVec::GetPosKey(const MonotypeVec* mad, ssize_t pos) {
  assertx(pos < mad->size());
  return make_tv<KindOfInt64>(pos);
}

TypedValue MonotypeVec::GetPosVal(const MonotypeVec* mad, ssize_t pos) {
  assertx(size_t(pos) < mad->size());
  return mad->typedValueUnchecked(pos);
}

ssize_t MonotypeVec::GetIntPos(const MonotypeVec* mad, int64_t k) {
  return LIKELY(size_t(k) < mad->size()) ? k : mad->size();
}

ssize_t MonotypeVec::GetStrPos(const MonotypeVec* mad, const StringData*) {
  return mad->size();
}

arr_lval MonotypeVec::LvalInt(MonotypeVec* mad, int64_t k) {
  auto const vad = EscalateToVanilla(mad, __func__);
  auto const res = vad->lval(k);
  assertx(res.arr == vad);
  return res;
}

arr_lval MonotypeVec::LvalStr(MonotypeVec* mad, StringData* k) {
  throwInvalidArrayKeyException(k, mad);
}

arr_lval MonotypeVec::elemImpl(int64_t k, bool throwOnMissing) {
  if (size_t(k) >= size()) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
  if (type() == KindOfClsMeth) {
    // If we have a ClsMeth, we need to return a proper lval, so we escalate to
    // vanilla.
    return LvalInt(this, k);
  }
  auto const cow = cowCheck();
  auto const mad = cow ? copy() : this;
  mad->adjustTypePersistence(dt_with_rc(mad->type()));
  static_assert(folly::kIsLittleEndian);
  auto const type_ptr = reinterpret_cast<DataType*>(&mad->m_extra_hi16);
  assertx(*type_ptr == mad->type());
  return arr_lval{mad, type_ptr, &mad->valueRefUnchecked(k)};
}

tv_lval MonotypeVec::ElemInt(tv_lval lvalIn, int64_t k, bool throwOnMissing) {
  auto const madIn = As(lvalIn.val().parr);
  auto const lval = madIn->elemImpl(k, throwOnMissing);
  if (lval.arr != madIn) {
    lvalIn.type() = dt_with_rc(lvalIn.type());
    lvalIn.val().parr = lval.arr;
    if (madIn->decReleaseCheck()) Release(madIn);
  }
  return lval;
}

tv_lval MonotypeVec::ElemStr(tv_lval lval, StringData* k, bool throwOnMissing) {
  if (throwOnMissing) throwInvalidArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

bool MonotypeVec::adjustTypePersistence(DataType dt) {
  auto const mtype = type();
  if (dt == mtype) return true;
  if (!equivDataTypes(mtype, dt)) return false;
  setLayoutIndex(getLayoutIndex(dt_with_rc(dt)));
  return true;
}

template <bool Move>
ArrayData* MonotypeVec::setIntImpl(int64_t k, TypedValue v) {
  assertx(cowCheck() || notCyclic(v));

  if (UNLIKELY(size_t(k) >= size())) {
    throwOOBArrayKeyException(k, this);
  }

  if (!adjustTypePersistence(v.m_type)) {
    auto const vad = EscalateToVanilla(this, __func__);
    auto const res = vad->set(k, v);
    assertx(vad == res);
    if constexpr (Move) {
      if (decReleaseCheck()) Release(this);
      tvDecRefGen(v);
    }
    return res;
  }

  if constexpr (!Move) {
    tvIncRefGen(v);
  }
  auto const cow = cowCheck();
  auto const mad = cow ? copy() : this;
  mad->valueRefUnchecked(k) = val(v);
  if constexpr (Move) {
    if (cow && decReleaseCheck()) Release(this);
  }

  return mad;
}

ArrayData* MonotypeVec::SetInt(MonotypeVec* madIn, int64_t k, TypedValue v) {
  return madIn->setIntImpl<false>(k, v);
}

ArrayData* MonotypeVec::SetIntMove(MonotypeVec* madIn, int64_t k, TypedValue v) {
  return madIn->setIntImpl<true>(k, v);
}

ArrayData* MonotypeVec::SetStr(MonotypeVec* madIn, StringData* k, TypedValue) {
  throwInvalidArrayKeyException(k, madIn);
}

ArrayData* MonotypeVec::SetStrMove(MonotypeVec* madIn, StringData* k, TypedValue) {
  throwInvalidArrayKeyException(k, madIn);
}

ArrayData* MonotypeVec::RemoveInt(MonotypeVec* madIn, int64_t k) {
  if (UNLIKELY(size_t(k) >= madIn->m_size)) return madIn;
  if (LIKELY(size_t(k) + 1 == madIn->m_size)) {
    auto const mad = madIn->cowCheck() ? madIn->copy() : madIn;
    mad->m_size--;
    tvDecRefGen(mad->typedValueUnchecked(mad->m_size));
    return mad;
  }

  if (madIn->isVecType()) {
    throwVecUnsetException();
  } else {
    throwVarrayUnsetException();
  }
}

ArrayData* MonotypeVec::RemoveStr(MonotypeVec* madIn, const StringData*) {
  return madIn;
}

ssize_t MonotypeVec::IterBegin(const MonotypeVec* mad) {
  return 0;
}

ssize_t MonotypeVec::IterLast(const MonotypeVec* mad) {
  return mad->size() > 0 ? mad->size() - 1 : 0;
}

ssize_t MonotypeVec::IterEnd(const MonotypeVec* mad) {
  return mad->size();
}

ssize_t MonotypeVec::IterAdvance(const MonotypeVec* mad, ssize_t pos) {
  return pos < mad->size() ? pos + 1 : pos;
}

ssize_t MonotypeVec::IterRewind(const MonotypeVec* mad, ssize_t pos) {
  return pos > 0 ? pos - 1 : mad->size();
}

template <bool Move>
ArrayData* MonotypeVec::appendImpl(TypedValue v) {
  if (!adjustTypePersistence(v.m_type)) {
    // Type doesn't match; escalate to vanilla
    auto const ad = escalateWithCapacity(size() + 1);
    auto const res = PackedArray::Append(ad, v);
    assertx(ad == res);
    return res;
  }

  auto const mad = prepareForInsert();
  mad->valueRefUnchecked(mad->m_size++) = val(v);

  if constexpr (Move) {
    if (mad != this && decReleaseCheck()) Release(this);
  } else {
    tvIncRefGen(v);
  }

  return mad;
}

ArrayData* MonotypeVec::Append(MonotypeVec* madIn, TypedValue v) {
  return madIn->appendImpl<false>(v);
}

ArrayData* MonotypeVec::AppendMove(MonotypeVec* madIn, TypedValue v) {
  return madIn->appendImpl<true>(v);
}

ArrayData* MonotypeVec::Pop(MonotypeVec* madIn, Variant& value) {
  auto const mad = madIn->cowCheck() ? madIn->copy() : madIn;
  if (UNLIKELY(mad->m_size) == 0) {
    value = uninit_null();
    return mad;
  }

  auto const newSize = mad->size() - 1;
  TypedValue tv;
  tv.m_data = mad->valueRefUnchecked(newSize);
  tv.m_type = mad->type();
  value = Variant::wrap(tv);
  tvDecRefGen(tv);
  mad->m_size = newSize;

  return mad;
}

ArrayData* MonotypeVec::ToDVArray(MonotypeVec* madIn, bool copy) {
  if (madIn->isVArray()) return madIn;
  auto const mad = copy ? madIn->copy() : madIn;
  mad->m_kind = HeaderKind::BespokeVArray;
  assertx(mad->checkInvariants());

  return mad;
}

ArrayData* MonotypeVec::ToHackArr(MonotypeVec* madIn, bool copy) {
  if (madIn->isVecType()) return madIn;
  auto const mad = copy ? madIn->copy() : madIn;
  mad->setLegacyArrayInPlace(false);
  mad->m_kind = HeaderKind::BespokeVec;
  assertx(mad->checkInvariants());

  return mad;
}

ArrayData* MonotypeVec::PreSort(MonotypeVec* mad, SortFunction sf) {
  return mad->escalateWithCapacity(mad->size());
}

ArrayData* MonotypeVec::PostSort(MonotypeVec* mad, ArrayData* vad) {
  auto const result = MakeFromVanilla(vad, mad->type());
  PackedArray::Release(vad);
  return result;
}

ArrayData* MonotypeVec::SetLegacyArray(MonotypeVec* madIn,
                                       bool copy, bool legacy) {
  auto const mad = copy ? madIn->copy() : madIn;
  mad->setLegacyArrayInPlace(legacy);
  return mad;
}

}}
