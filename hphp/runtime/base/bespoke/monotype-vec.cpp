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
#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-uncounted.h"

#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/word-mem.h"

namespace HPHP { namespace bespoke {

namespace {

//////////////////////////////////////////////////////////////////////////////

constexpr size_t kMinSizeIndex = 2;
constexpr size_t kMinSizeBytes = 48;

static_assert(sizeof(MonotypeVec) < kMinSizeBytes);
static_assert(sizeof(EmptyMonotypeVec) < kMinSizeBytes);
static_assert(kMinSizeBytes == kSizeIndex2Size[kMinSizeIndex]);

uint16_t packSizeIndexAndAuxBits(uint8_t idx, uint8_t aux) {
  return (static_cast<uint16_t>(idx) << 8) | aux;
}

inline constexpr uint32_t sizeClassParams2MonotypeVecCapacity(size_t index) {
  if (index < kMinSizeIndex) return 0;
  auto const bytes = kSizeIndex2Size[index];
  return (bytes - sizeof(MonotypeVec)) / sizeof(Value);
}

alignas(64) constexpr uint32_t kSizeIndex2MonotypeVecCapacity[] = {
#define SIZE_CLASS(index, ...) \
  sizeClassParams2MonotypeVecCapacity(index),
  SIZE_CLASSES
#undef SIZE_CLASS
};

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////
// Static initialization
//////////////////////////////////////////////////////////////////////////////

namespace {

using StaticVec = std::aligned_storage<kMinSizeBytes, 16>::type;

StaticVec s_emptyMonotypeVec;
StaticVec s_emptyMonotypeVecMarked;

const LayoutFunctions* monotypeVecVtable() {
  static auto const result = fromArray<MonotypeVec>();
  return &result;
}

const LayoutFunctions* emptyMonotypeVecVtable() {
  static auto const result = fromArray<EmptyMonotypeVec>();
  return &result;
}

constexpr LayoutIndex getLayoutIndex(DataType type) {
  auto constexpr base = uint16_t(kMonotypeVecLayoutByte << 8);
  return LayoutIndex{uint16_t(base + uint8_t(type))};
}

constexpr LayoutIndex getEmptyLayoutIndex() {
  auto constexpr type = kExtraInvalidDataType;
  auto constexpr base = uint16_t(kEmptyMonotypeVecLayoutByte << 8);
  return LayoutIndex{uint16_t(base + uint8_t(type))};
}

Layout::LayoutSet getEmptyParentLayouts() {
  Layout::LayoutSet result;
#define DT(name, value) {                            \
    auto const type = KindOf##name;                  \
    if (type != KindOfUninit) {                      \
      result.insert(MonotypeVecLayout::Index(type)); \
    }                                                \
  }
  DATATYPES
#undef DT
  return result;
}

Layout::LayoutSet getMonotypeParentLayouts(DataType dt) {
  Layout::LayoutSet result;
  if (hasPersistentFlavor(dt) && !isRefcountedType(dt)) {
    result.insert(MonotypeVecLayout::Index(dt_with_rc(dt)));
  }
  result.insert(TopMonotypeVecLayout::Index());
  return result;
}

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

bool EmptyMonotypeVec::checkInvariants() const {
  assertx(isStatic());
  assertx(kindIsValid());
  assertx(size() == 0);
  assertx(isVecType());
  assertx(layoutIndex() == getEmptyLayoutIndex());

  // Check that EmptyMonotypeVec puns a MonotypeVec modulo its type().
  // Cast without assertions to avoid an infinite loop of invariant checks.
  auto const DEBUG_ONLY mad = reinterpret_cast<const MonotypeVec*>(this);
  assertx(mad->type() == kExtraInvalidDataType);
  assertx(mad->sizeIndex() == kMinSizeIndex);

  return true;
}

size_t EmptyMonotypeVec::HeapSize(const EmptyMonotypeVec* ead) {
  return kMinSizeBytes;
}

void EmptyMonotypeVec::Scan(const EmptyMonotypeVec* ead, type_scan::Scanner&) {
}

ArrayData* EmptyMonotypeVec::EscalateToVanilla(const EmptyMonotypeVec* ead,
                                               const char* reason) {
  logEscalateToVanilla(ead, reason);
  auto const legacy = ead->isLegacyArray();
  return legacy ? staticEmptyMarkedVec() : staticEmptyVec();
}

void EmptyMonotypeVec::ConvertToUncounted(EmptyMonotypeVec*,
                                          DataWalker::PointerMap*) {
  // All EmptyMonotypeVecs are static, so we should never make them uncounted.
  always_assert(false);
}

void EmptyMonotypeVec::ReleaseUncounted(EmptyMonotypeVec* ead) {
  // All EmptyMonotypeVecs are static, so we should never release them.
  always_assert(false);
}

void EmptyMonotypeVec::Release(EmptyMonotypeVec* ead) {
  // All EmptyMonotypeVecs are static, so we should never release them.
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

ArrayData* EmptyMonotypeVec::AppendMove(EmptyMonotypeVec* ead, TypedValue v) {
  auto const mad = MonotypeVec::MakeReserve(ead->isLegacyArray(), 1, type(v));
  auto const res = MonotypeVec::AppendMove(mad, v);
  assertx(mad == res);
  return res;
}

ArrayData* EmptyMonotypeVec::Pop(EmptyMonotypeVec* ead, Variant& value) {
  value = uninit_null();
  return ead;
}

ArrayData* EmptyMonotypeVec::PreSort(EmptyMonotypeVec* ead, SortFunction sf) {
  always_assert(false);
}

ArrayData* EmptyMonotypeVec::PostSort(EmptyMonotypeVec* ead, ArrayData* vad) {
  always_assert(false);
}

ArrayData* EmptyMonotypeVec::SetLegacyArray(EmptyMonotypeVec* eadIn,
                                            bool copy, bool legacy) {
  return GetVec(legacy);
}


//////////////////////////////////////////////////////////////////////////////
// MonotypeVec
//////////////////////////////////////////////////////////////////////////////

template <typename CountableFn, typename MaybeCountableFn>
void MonotypeVec::forEachCountableValue(CountableFn c, MaybeCountableFn mc) {
  auto const dt = type();
  if (isRefcountedType(dt)) {
    if (hasPersistentFlavor(dt)) {
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

template <bool Static>
MonotypeVec* MonotypeVec::MakeReserve(
    bool legacy, uint32_t capacity, DataType dt) {
  auto const bytes = sizeof(MonotypeVec) + capacity * sizeof(Value);
  auto const index = std::max(MemoryManager::size2Index(bytes), kMinSizeIndex);
  auto const alloc = [&]{
    if (!Static) return tl_heap->objMallocIndex(index);
    auto const size = MemoryManager::sizeIndex2Size(index);
    return RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size);
  }();

  auto const mad = static_cast<MonotypeVec*>(alloc);
  auto const aux = packSizeIndexAndAuxBits(
      index, legacy ? ArrayData::kLegacyArray : 0);

  mad->initHeader_16(HeaderKind::BespokeVec, OneReference, aux);
  mad->setLayoutIndex(getLayoutIndex(dt));
  mad->m_size = 0;

  assertx(mad->checkInvariants());
  return mad;
}

MonotypeVec* MonotypeVec::MakeFromVanilla(ArrayData* ad, DataType dt) {
  assertx(ad->isVanillaVec());
  auto result = ad->isStatic()
    ? MakeReserve<true>(ad->isLegacyArray(), ad->size(), dt)
    : MakeReserve<false>(ad->isLegacyArray(), ad->size(), dt);

  PackedArray::IterateV(ad, [&](auto v) {
    auto const next = AppendMove(result, v);
    tvIncRefGen(v);
    assertx(result == next);
    result = As(next);
  });

  if (ad->isStatic()) {
    auto const aux = packSizeIndexAndAuxBits(
      result->sizeIndex(), result->auxBits());
    result->initHeader_16(HeaderKind::BespokeVec, StaticValue, aux);
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

Value& MonotypeVec::valueRefUnchecked(size_t idx) {
  return rawData()[idx];
}

const Value& MonotypeVec::valueRefUnchecked(size_t idx) const {
  return const_cast<MonotypeVec*>(this)->valueRefUnchecked(idx);
}

TypedValue MonotypeVec::typedValueUnchecked(size_t idx) const {
  return make_tv_of_type(valueRefUnchecked(idx), type());
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
ArrayData* MonotypeVec::escalateWithCapacity(
    size_t capacity, const char* reason) const {
  assertx(capacity >= size());
  logEscalateToVanilla(this, reason);

  auto const ad = PackedArray::MakeReserveVec(capacity);
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
  assertx(isVecType());
  assertx(size() <= capacity());
  assertx(sizeIndex() >= kMinSizeIndex);

  // In order to make fewer virtual calls, we may use MonotypeVec's methods
  // on an array which is actually an EmptyMonotypeVec.
  if (layoutIndex() != getEmptyLayoutIndex()) {
    assertx(isRealType(type()));
    assertx(layoutIndex() == getLayoutIndex(type()));
    assertx(IMPLIES(!empty(), tvIsPlausible(typedValueUnchecked(0))));
  } else {
    reinterpret_cast<const EmptyMonotypeVec*>(this)->checkInvariants();
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
  return mad->escalateWithCapacity(mad->size(), reason);
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
  auto const newType = hasPersistentFlavor(oldType)
    ? dt_with_persistence(oldType)
    : oldType;
  madIn->setLayoutIndex(getLayoutIndex(newType));
}

void MonotypeVec::ReleaseUncounted(MonotypeVec* mad) {
  for (uint32_t i = 0; i < mad->size(); i++) {
    auto const tv = mad->typedValueUnchecked(i);
    DecRefUncounted(tv);
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

  auto const dt = type();
  if (dt == KindOfClsMeth) return LvalInt(this, k);

  auto const cow = cowCheck();
  auto const mad = cow ? copy() : this;
  mad->setLayoutIndex(getLayoutIndex(dt_modulo_persistence(dt)));

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

ArrayData* MonotypeVec::setIntImpl(int64_t k, TypedValue v) {
  assertx(cowCheck() || notCyclic(v));

  if (UNLIKELY(size_t(k) >= size())) {
    throwOOBArrayKeyException(k, this);
  }

  auto const dt = type();
  if (!equivDataTypes(dt, v.type())) {
    auto const vad = EscalateToVanilla(this, __func__);
    auto const res = vad->setMove(k, v);
    assertx(vad == res);
    if (decReleaseCheck()) Release(this);
    return res;
  }

  auto const cow = cowCheck();
  auto const mad = cow ? copy() : this;
  if (dt != v.type()) {
    mad->setLayoutIndex(getLayoutIndex(dt_with_rc(dt)));
  }
  mad->valueRefUnchecked(k) = val(v);
  if (cow && decReleaseCheck()) Release(this);

  return mad;
}

ArrayData* MonotypeVec::SetIntMove(MonotypeVec* madIn, int64_t k, TypedValue v) {
  return madIn->setIntImpl(k, v);
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

ArrayData* MonotypeVec::appendImpl(TypedValue v) {
  auto const dt = type();
  if (dt != kExtraInvalidDataType && !equivDataTypes(dt, v.type())) {
    auto const ad = escalateWithCapacity(size() + 1, __func__);
    auto const result = PackedArray::AppendMove(ad, v);
    assertx(ad == result);
    return result;
  }

  auto const mad = prepareForInsert();
  if (dt == kExtraInvalidDataType) {
    mad->setLayoutIndex(getLayoutIndex(v.type()));
  } else if (dt != v.type()) {
    mad->setLayoutIndex(getLayoutIndex(dt_with_rc(dt)));
  }
  mad->valueRefUnchecked(mad->m_size++) = val(v);

  return mad;
}

ArrayData* MonotypeVec::AppendMove(MonotypeVec* madIn, TypedValue v) {
  auto const res = madIn->appendImpl(v);
  if (res != madIn && madIn->decReleaseCheck()) Release(madIn);
  return res;
}

ArrayData* MonotypeVec::Pop(MonotypeVec* madIn, Variant& value) {
  if (UNLIKELY(madIn->m_size) == 0) {
    value = uninit_null();
    return madIn;
  }

  auto const mad = madIn->cowCheck() ? madIn->copy() : madIn;
  auto const newSize = mad->size() - 1;
  value = Variant::attach(mad->typedValueUnchecked(newSize));
  mad->m_size = newSize;
  return mad;
}

ArrayData* MonotypeVec::PreSort(MonotypeVec* mad, SortFunction sf) {
  return mad->escalateWithCapacity(mad->size(), sortFunctionName(sf));
}

ArrayData* MonotypeVec::PostSort(MonotypeVec* mad, ArrayData* vad) {
  auto const result = MakeFromVanilla(vad, mad->type());
  PackedArray::Release(vad);
  return result;
}

ArrayData* MonotypeVec::SetLegacyArray(MonotypeVec* madIn,
                                       bool copy, bool legacy) {
  if (madIn->empty()) EmptyMonotypeVec::GetVec(legacy);
  auto const mad = copy ? madIn->copy() : madIn;
  mad->setLegacyArrayInPlace(legacy);
  return mad;
}

//////////////////////////////////////////////////////////////////////////////

using namespace jit;

// EmptyMonotypeVecLayout()

ArrayLayout EmptyMonotypeVecLayout::appendType(Type val) const {
  if (!val.maybe(TInitCell)) return ArrayLayout::Bottom();
  return val.isKnownDataType()
    ? ArrayLayout(MonotypeVecLayout::Index(val.toDataType()))
    : ArrayLayout(TopMonotypeVecLayout::Index());
}

ArrayLayout EmptyMonotypeVecLayout::removeType(Type key) const {
  return ArrayLayout(this);
}

ArrayLayout EmptyMonotypeVecLayout::setType(Type key, Type val) const {
  return ArrayLayout::Bottom();
}

std::pair<Type, bool> EmptyMonotypeVecLayout::elemType(Type key) const {
  return {TBottom, false};
}

std::pair<Type, bool> EmptyMonotypeVecLayout::firstLastType(
    bool isFirst, bool isKey) const {
  return {TBottom, false};
}

Type EmptyMonotypeVecLayout::iterPosType(Type pos, bool isKey) const {
  return TBottom;
}

// MonotypeVecLayout(ValType)

ArrayLayout MonotypeVecLayout::appendType(Type val) const {
  return setType(TInt, val);
}

ArrayLayout MonotypeVecLayout::removeType(Type key) const {
  return ArrayLayout(this);
}

ArrayLayout MonotypeVecLayout::setType(Type key, Type val) const {
  if (!key.maybe(TInt))       return ArrayLayout::Bottom();
  if (!val.maybe(TInitCell))  return ArrayLayout::Bottom();
  if (!val.isKnownDataType()) return ArrayLayout::Top();

  auto const base = val.toDataType();
  if (!equivDataTypes(base, m_fixedType)) return ArrayLayout::Top();

  auto const valType = base == m_fixedType ? base : dt_with_rc(base);
  return ArrayLayout(Index(valType));
}

std::pair<Type, bool> MonotypeVecLayout::elemType(Type key) const {
  return {Type(m_fixedType), false};
}

std::pair<Type, bool> MonotypeVecLayout::firstLastType(
    bool isFirst, bool isKey) const {
  return {isKey ? TInt : Type(m_fixedType), false};
}

Type MonotypeVecLayout::iterPosType(Type pos, bool isKey) const {
  return isKey ? TInt : Type(m_fixedType);
}

//////////////////////////////////////////////////////////////////////////////
// Layout creation and usage API
//////////////////////////////////////////////////////////////////////////////

void MonotypeVec::InitializeLayouts() {
  new TopMonotypeVecLayout();

  // Create all the potentially internal concrete layouts first
#define DT(name, value) {                                              \
    auto const type = KindOf##name;                                    \
    if (type == dt_modulo_persistence(type) && type != KindOfUninit) { \
      new MonotypeVecLayout(type);                                     \
    }                                                                  \
  }
  DATATYPES
#undef DT

#define DT(name, value) {                                              \
    auto const type = KindOf##name;                                    \
    if (type != dt_modulo_persistence(type) && type != KindOfUninit) { \
      new MonotypeVecLayout(type);                                     \
    }                                                                  \
  }
  DATATYPES
#undef DT

  new EmptyMonotypeVecLayout();

  auto const init = [&](EmptyMonotypeVec* ead, bool legacy) {
    // For EmptyMonotypeVecs, we use the minimum size index so that MonotypeVec
    // functions can always safely read the size index. It will never be used
    // for capacity decisions in this case, as EmptyMonotypeVecs are always
    // static.
    auto const aux = packSizeIndexAndAuxBits(
        kMinSizeIndex, legacy ? ArrayData::kLegacyArray : 0);
    ead->initHeader_16(HeaderKind::BespokeVec, StaticValue, aux);
    ead->m_size = 0;
    ead->setLayoutIndex(getEmptyLayoutIndex());
    assertx(ead->checkInvariants());
  };

  init(EmptyMonotypeVec::GetVec(false), false);
  init(EmptyMonotypeVec::GetVec(true), true);
}

TopMonotypeVecLayout::TopMonotypeVecLayout()
  : AbstractLayout(
      Index(),
      "MonotypeVec<Top>",
      {AbstractLayout::GetBespokeTopIndex()},
      monotypeVecVtable())
{}

LayoutIndex TopMonotypeVecLayout::Index() {
  return MonotypeVecLayout::Index(kInvalidDataType);
}

EmptyMonotypeVecLayout::EmptyMonotypeVecLayout()
  : ConcreteLayout(
      Index(),
      "MonotypeVec<Empty>",
      {getEmptyParentLayouts()},
      emptyMonotypeVecVtable())
{}

LayoutIndex EmptyMonotypeVecLayout::Index() {
  return getEmptyLayoutIndex();
}

MonotypeVecLayout::MonotypeVecLayout(DataType type)
  : ConcreteLayout(
      Index(type),
      folly::sformat("MonotypeVec<{}>", tname(type)),
      getMonotypeParentLayouts(type),
      monotypeVecVtable())
  , m_fixedType(type)
{}

LayoutIndex MonotypeVecLayout::Index(DataType type) {
  return getLayoutIndex(type);
}

bool isMonotypeVecLayout(LayoutIndex index) {
  auto const byte = index.byte();
  return byte == kMonotypeVecLayoutByte || byte == kEmptyMonotypeVecLayoutByte;
}

}}
