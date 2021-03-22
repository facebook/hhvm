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

#include "hphp/runtime/base/bespoke/struct-array.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

namespace {

uint8_t s_numStructLayouts = 0;
folly::SharedMutex s_keySetLock;
std::unordered_map<KeyOrder, LayoutIndex, KeyOrderHash> s_keySetToIdx;

const LayoutFunctions* structArrayVtable() {
  static auto const result = fromArray<StructArray>();
  return &result;
}

uint16_t packSizeIndexAndAuxBits(uint8_t idx, uint8_t aux) {
  return (static_cast<uint16_t>(idx) << 8) | aux;
}

bool isStructLayout(LayoutIndex index) {
  return index.byte() == kStructLayoutByte;
}

std::string describeStructLayout(const KeyOrder& ko) {
  auto const base = ko.toString();
  return folly::sformat("StructArray<{}>", base.substr(1, base.size() - 2));
}

}

//////////////////////////////////////////////////////////////////////////////

StructArray* StructArray::As(ArrayData* ad) {
  auto const result = reinterpret_cast<StructArray*>(ad);
  assertx(result->checkInvariants());
  return result;
}

const StructArray* StructArray::As(const ArrayData* ad) {
  return As(const_cast<ArrayData*>(ad));
}

bool StructArray::checkInvariants() const {
  static_assert(sizeof(StructArray) == 16);
  assertx(layout()->index() == layoutIndex());
  assertx(layout()->sizeIndex() == sizeIndex());
  assertx(layout()->numFields() == numFields());
  assertx(layout()->typeOffset() == typeOffset());
  assertx(layout()->valueOffset() == valueOffsetInValueSize() * sizeof(Value));
  assertx(layoutIndex().byte() == kStructLayoutByte);
  return true;
}

LayoutIndex StructLayout::Index(uint8_t idx) {
  auto constexpr base = uint16_t(kStructLayoutByte << 8);
  return LayoutIndex{uint16_t(base + idx)};
}

const StructLayout* StructLayout::GetLayout(const KeyOrder& ko, bool create) {
  if (ko.empty() || !ko.valid()) return nullptr;
  {
    folly::SharedMutex::ReadHolder rlock{s_keySetLock};
    auto const it = s_keySetToIdx.find(ko);
    if (it != s_keySetToIdx.end()) return As(FromIndex(it->second));
  }
  if (!create) return nullptr;

  folly::SharedMutex::WriteHolder wlock{s_keySetLock};
  auto const it = s_keySetToIdx.find(ko);
  if (it != s_keySetToIdx.end()) return As(FromIndex(it->second));

  auto constexpr kMaxIndex = std::numeric_limits<uint8_t>::max();
  if (s_numStructLayouts == kMaxIndex) return nullptr;
  auto const index = Index(s_numStructLayouts++);
  auto const bytes = sizeof(StructLayout) + sizeof(Field) * (ko.size() - 1);
  auto const result = new (malloc(bytes)) StructLayout(ko, index);
  s_keySetToIdx.emplace(ko, index);
  return result;
}

StructLayout::StructLayout(const KeyOrder& ko, const LayoutIndex& idx)
  : ConcreteLayout(idx, describeStructLayout(ko),
                   {AbstractLayout::GetBespokeTopIndex()},
                   structArrayVtable())
{
  Slot i = 0;
  m_key_to_slot.reserve(ko.size());
  for (auto const key : ko) {
    assertx(key->isStatic());
    m_key_to_slot.insert({StaticKey{key}, i});
    m_fields[i].key = key;
    i++;
  }
  assertx(numFields() == ko.size());
  m_typeOff = numFields();
  m_valueOff = (m_typeOff + numFields() + 7) & ~7;
  auto const bytes = sizeof(StructArray) +
                     m_valueOff +
                     numFields() * sizeof(Value);
  m_size_index = MemoryManager::size2Index(bytes);
}

uint8_t StructArray::sizeIndex() const {
  return m_aux16 >> 8;
}

size_t StructLayout::numFields() const {
  return m_key_to_slot.size();
}

size_t StructLayout::sizeIndex() const {
  return m_size_index;
}

Slot StructLayout::keySlot(const StringData* key) const {
  auto const it = key->isStatic()
    ? m_key_to_slot.find(StaticKey{key})
    : m_key_to_slot.find(NonStaticKey{key});
  return it == m_key_to_slot.end() ? kInvalidSlot : it->second;
}

const StructLayout::Field& StructLayout::field(Slot slot) const {
  assertx(slot < numFields());
  return m_fields[slot];
}

template <bool Static>
StructArray* StructArray::MakeReserve(HeaderKind kind,
                                      bool legacy,
                                      const StructLayout* layout) {
  assertx(layout);
  auto const sizeIdx = layout->sizeIndex();
  auto const alloc = [&] {
    if (!Static) return tl_heap->objMallocIndex(sizeIdx);
    auto const size = MemoryManager::sizeIndex2Size(sizeIdx);
    return RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size);
  }();

  auto const sad = static_cast<StructArray*>(alloc);
  auto const aux = packSizeIndexAndAuxBits(
      sizeIdx, legacy ? ArrayData::kLegacyArray : 0);
  sad->initHeader_16(kind, OneReference, aux);
  sad->setLayoutIndex(layout->index());
  sad->m_size = 0;

  auto const numFields = layout->numFields();
  assertx(numFields <= std::numeric_limits<uint8_t>::max());
  auto const valueOffset = layout->valueOffset();
  assertx(valueOffset % 8 == 0);
  assertx((valueOffset / 8) <= std::numeric_limits<uint8_t>::max());
  sad->m_extra_lo16 = numFields << 8 | (valueOffset / 8);

  memset(sad->rawTypes(), static_cast<int>(KindOfUninit), sad->numFields());
  assertx(sad->checkInvariants());
  return sad;
}

size_t StructArray::numFields() const {
  return (m_extra_lo16 >> 8) & 0xff;
}

size_t StructArray::valueOffsetInValueSize() const {
  return m_extra_lo16 & 0xff;
}

const StructLayout* StructLayout::As(const Layout* l) {
  assertx(dynamic_cast<const StructLayout*>(l));
  return reinterpret_cast<const StructLayout*>(l);
}

StructArray* StructArray::MakeFromVanilla(ArrayData* ad,
                                          const StructLayout* layout) {
  if (!ad->hasVanillaMixedLayout()) return nullptr;;

  assertx(layout);
  auto const kind = ad->isDArray() ? HeaderKind::BespokeDArray
                                   : HeaderKind::BespokeDict;
  auto const result = ad->isStatic()
    ? MakeReserve<true>(kind, ad->isLegacyArray(), layout)
    : MakeReserve<false>(kind, ad->isLegacyArray(), layout);

  auto fail = false;
  auto const types = result->rawTypes();
  auto const vals = result->rawValues();
  MixedArray::IterateKV(MixedArray::asMixed(ad), [&](auto k, auto v) -> bool {
    if (!tvIsString(k)) {
      fail = true;
      return true;
    }
    auto const slot = layout->keySlot(val(k).pstr);
    if (slot == kInvalidSlot) {
      fail = true;
      return true;
    }
    result->addNextSlot(slot);
    types[slot] = type(v);
    vals[slot] = val(v);
    tvIncRefGen(v);
    return false;
  });

  if (fail) {
    if (!ad->isStatic()) Release(result);
    return nullptr;
  }

  if (ad->isStatic()) {
    auto const aux = packSizeIndexAndAuxBits(result->sizeIndex(),
                                             result->auxBits());
    result->initHeader_16(kind, StaticValue, aux);
  }

  assertx(result->checkInvariants());
  return result;
}

StructArray* StructArray::MakeStructDArray(
    const StructLayout* layout, uint32_t size,
    const Slot* slots, const TypedValue* vals) {
  return MakeStructImpl(layout, size, slots, vals, HeaderKind::BespokeDArray);
}

StructArray* StructArray::MakeStructDict(
    const StructLayout* layout, uint32_t size,
    const Slot* slots, const TypedValue* vals) {
  return MakeStructImpl(layout, size, slots, vals, HeaderKind::BespokeDict);
}

StructArray* StructArray::MakeStructImpl(
    const StructLayout* layout, uint32_t size,
    const Slot* slots, const TypedValue* tvs, HeaderKind hk) {

  auto const result = MakeReserve<false>(hk, false, layout);
  result->m_size = size;

  auto const types = result->rawTypes();
  auto const vals = result->rawValues();
  auto const positions = result->rawPositions();

  for (auto i = 0; i < size; i++) {
    assertx(slots[i] <= layout->numFields());
    auto const& tv = tvs[size - i - 1];
    types[slots[i]] = type(tv);
    vals[slots[i]] = val(tv);
    positions[i] = slots[i];
  }

  assertx(result->checkInvariants());
  assertx(result->layout() == layout);
  assertx(result->size() == size);
  return result;
}

const StructLayout* StructArray::layout() const {
  return StructLayout::As(Layout::FromIndex(layoutIndex()));
}

DataType* StructArray::rawTypes() {
  assertx(typeOffset() == layout()->typeOffset());
  return reinterpret_cast<DataType*>(
      reinterpret_cast<char*>(this + 1) + typeOffset());
}

const DataType* StructArray::rawTypes() const {
  return const_cast<StructArray*>(this)->rawTypes();
}

Value* StructArray::rawValues() {
  return reinterpret_cast<Value*>(
      reinterpret_cast<Value*>(this + 1) + valueOffsetInValueSize());
}

const Value* StructArray::rawValues() const {
  return const_cast<StructArray*>(this)->rawValues();
}

uint8_t* StructArray::rawPositions() {
  return reinterpret_cast<uint8_t*>(this + 1);
}

const uint8_t* StructArray::rawPositions() const {
  return const_cast<StructArray*>(this)->rawPositions();
}

TypedValue StructArray::typedValueUnchecked(Slot slot) const {
  return make_tv_of_type(rawValues()[slot], rawTypes()[slot]);
}

ArrayData* StructArray::escalateWithCapacity(size_t capacity,
                                             const char* reason) const {
  assertx(capacity >= size());
  logEscalateToVanilla(this, reason);

  auto ad = isDictType() ? MixedArray::MakeReserveDict(capacity)
                         : MixedArray::MakeReserveDArray(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());
  auto const layout = this->layout();
  for (auto i = 0; i < m_size; i++) {
    auto const slot = getSlotInPos(i);
    auto const k = layout->field(slot).key;
    auto const tv = typedValueUnchecked(slot);
    auto const res =
      MixedArray::SetStrMove(ad, const_cast<StringData*>(k.get()), tv);
    assertx(ad == res);
    tvIncRefGen(tv);
    ad = res;
  }
  assertx(ad->size() == size());
  return ad;
}

void StructArray::ConvertToUncounted(StructArray* sad,
                                     DataWalker::PointerMap* seen) {
  for (Slot i = 0; i < sad->numFields(); i++) {
    auto const lval = tv_lval(&sad->rawTypes()[i], &sad->rawValues()[i]);
    ConvertTvToUncounted(lval, seen);
  }
}

void StructArray::ReleaseUncounted(StructArray* sad) {
  for (Slot i = 0; i < sad->numFields(); i++) {
    auto tv = sad->typedValueUnchecked(i);
    ReleaseUncountedTv(&tv);
  }
}

void StructArray::Release(StructArray* sad) {
  sad->fixCountForRelease();
  assertx(sad->isRefCounted());
  assertx(sad->hasExactlyOneRef());
  sad->decRefValues();
  tl_heap->objFreeIndex(sad, sad->sizeIndex());
}

bool StructArray::IsVectorData(const StructArray* sad) {
  return sad->empty();
}

TypedValue StructArray::NvGetInt(const StructArray*, int64_t) {
  return make_tv<KindOfUninit>();
}

TypedValue StructArray::NvGetStr(const StructArray* sad, const StringData* k) {
  auto const layout = sad->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) return make_tv<KindOfUninit>();
  return sad->typedValueUnchecked(slot);
}

TypedValue StructArray::GetPosKey(const StructArray* sad, ssize_t pos) {
  auto const layout = sad->layout();
  auto const slot = sad->getSlotInPos(pos);
  auto const k = layout->field(slot).key;
  return make_tv<KindOfPersistentString>(k);
}

TypedValue StructArray::GetPosVal(const StructArray* sad, ssize_t pos) {
  auto const slot = sad->getSlotInPos(pos);
  return sad->typedValueUnchecked(slot);
}

ssize_t StructArray::IterBegin(const StructArray*) {
  return 0;
}

ssize_t StructArray::IterLast(const StructArray* sad) {
  return sad->empty() ? 0 : sad->size() - 1;
}

ssize_t StructArray::IterEnd(const StructArray* sad) {
  return sad->size();
}

ssize_t StructArray::IterAdvance(const StructArray* sad, ssize_t pos) {
  return pos < sad->size() ? pos + 1 : pos;
}

ssize_t StructArray::IterRewind(const StructArray* sad, ssize_t pos) {
  return pos > 0 ? pos - 1 : sad->size();
}

arr_lval StructArray::LvalInt(StructArray* sad, int64_t k) {
  throwOOBArrayKeyException(k, sad);
}

arr_lval StructArray::LvalStr(StructArray* sad, StringData* key) {
  auto const layout = sad->layout();
  auto const slot = layout->keySlot(key);
  if (slot == kInvalidSlot) throwOOBArrayKeyException(key, sad);
  auto const& currType = sad->rawTypes()[slot];
  if (currType == KindOfUninit) throwOOBArrayKeyException(key, sad);
  auto const newad = sad->cowCheck() ? sad->copy() : sad;
  return { newad, &newad->rawTypes()[slot], &newad->rawValues()[slot] };
}

tv_lval StructArray::ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
  if (throwOnMissing) throwOOBArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

arr_lval StructArray::elemImpl(StringData* k, bool throwOnMissing) {
  auto const layout = this->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
  auto const& currType = rawTypes()[slot];
  if (currType == KindOfUninit) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
  if (currType == KindOfClsMeth) return LvalStr(this, k);
  auto const sad = cowCheck() ? this->copy() : this;
  auto& t = sad->rawTypes()[slot];
  t = dt_modulo_persistence(t);
  return arr_lval{sad, &t, &sad->rawValues()[slot]};
}

tv_lval StructArray::ElemStr(tv_lval lvalIn, StringData* k, bool throwOnMissing) {
  auto sadIn = As(lvalIn.val().parr);
  auto const lval = sadIn->elemImpl(k, throwOnMissing);
  if (lval.arr != sadIn) {
    lvalIn.type() = dt_with_rc(lvalIn.type());
    lvalIn.val().parr = lval.arr;
    if (sadIn->decReleaseCheck()) Release(sadIn);
  }
  return lval;
}

ArrayData* StructArray::SetIntMove(StructArray* sad, int64_t k, TypedValue v) {
  auto const vad = sad->escalateWithCapacity(sad->size() + 1, __func__);
  auto const res = MixedArray::SetIntMove(vad, k, v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructArray::SetStrMove(StructArray* sadIn,
                                   StringData* k,
                                   TypedValue v) {
  auto const layout = sadIn->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) {
    auto const vad = EscalateToVanilla(sadIn, __func__);
    auto const res = vad->setMove(k, v);
    assertx(vad == res);
    if (sadIn->decReleaseCheck()) Release(sadIn);
    return res;
  }
  auto const cow = sadIn->cowCheck();
  auto const sad = cow ? sadIn->copy() : sadIn;
  auto& oldType = sad->rawTypes()[slot];
  auto& oldVal = sad->rawValues()[slot];
  if (oldType == KindOfUninit) {
    sad->addNextSlot(slot);
  }
  oldType = type(v);
  oldVal = val(v);
  if (cow) sadIn->decRefCount();
  return sad;
}

StructArray* StructArray::copy() const {
  auto const sizeIdx = sizeIndex();
  auto const sad = static_cast<StructArray*>(tl_heap->objMallocIndex(sizeIdx));
  auto const heapSize = MemoryManager::sizeIndex2Size(sizeIdx);
  assertx(heapSize % 16 == 0);
  memcpy16_inline(sad, this, heapSize);
  auto const aux = packSizeIndexAndAuxBits(sizeIdx, auxBits());
  sad->initHeader_16(m_kind, OneReference, aux);
  sad->incRefValues();
  return sad;
}

void StructArray::incRefValues() {
  for (auto pos = 0; pos < m_size; pos++) {
    auto const tv = typedValueUnchecked(getSlotInPos(pos));
    tvIncRefGen(tv);
  }
}

void StructArray::decRefValues() {
  for (auto pos = 0; pos < m_size; pos++) {
    auto const tv = typedValueUnchecked(getSlotInPos(pos));
    tvDecRefGen(tv);
  }
}

ArrayData* StructArray::RemoveInt(StructArray* sad, int64_t) {
  return sad;
}

ArrayData* StructArray::RemoveStr(StructArray* sadIn, const StringData* k) {
  auto const layout = sadIn->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) return sadIn;
  auto const& currType = sadIn->rawTypes()[slot];
  if (currType == KindOfUninit) return sadIn;
  auto const sad = sadIn->cowCheck() ? sadIn->copy() : sadIn;
  tvDecRefGen(sad->typedValueUnchecked(slot));
  auto& t = sad->rawTypes()[slot];
  t = KindOfUninit;
  sad->removeSlot(slot);
  return sad;
}

ArrayData* StructArray::AppendMove(StructArray* sad, TypedValue v) {
  auto const vad = sad->escalateWithCapacity(sad->size() + 1, __func__);
  auto const res = vad->appendMove(v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructArray::Pop(StructArray* sadIn, Variant& value) {
  if (UNLIKELY(sadIn->size() == 0)) {
    value = uninit_null();
    return sadIn;
  }

  auto const sad = sadIn->cowCheck() ? sadIn->copy() : sadIn;
  auto const pos = sad->size() - 1;
  auto const slot = sad->getSlotInPos(pos);
  value = Variant::attach(sad->typedValueUnchecked(slot));
  auto& t = sad->rawTypes()[slot];
  t = KindOfUninit;
  sad->m_size--;
  return sad;
}

ArrayData* StructArray::ToDVArray(StructArray* sadIn, bool copy) {
  if (sadIn->isDArray()) return sadIn;
  auto const sad = copy ? sadIn->copy() : sadIn;
  sad->m_kind = HeaderKind::BespokeDArray;
  assertx(sad->checkInvariants());
  return sad;
}

ArrayData* StructArray::ToHackArr(StructArray* sadIn, bool copy) {
  if (sadIn->isDictType()) return sadIn;
  auto const sad = copy ? sadIn->copy() : sadIn;
  sad->m_kind = HeaderKind::BespokeDict;
  sad->setLegacyArrayInPlace(false);
  assertx(sad->checkInvariants());
  return sad;
}

ArrayData* StructArray::PreSort(StructArray* sad, SortFunction sf) {
  return sad->escalateWithCapacity(sad->size(), sortFunctionName(sf));
}

ArrayData* StructArray::PostSort(StructArray* sad, ArrayData* vad) {
  auto const result = MakeFromVanilla(vad, sad->layout());
  if (!result) return vad;
  MixedArray::Release(vad);
  return result;
}

ArrayData* StructArray::SetLegacyArray(StructArray* sadIn,
                                       bool copy, bool legacy) {
  auto const sad = copy ? sadIn->copy() : sadIn;
  sad->setLegacyArrayInPlace(legacy);
  return sad;
}

////////////////////////////////////////////////////////////////////////////

size_t StructArray::HeapSize(const StructArray* sad) {
  return MemoryManager::sizeIndex2Size(sad->sizeIndex());
}

void StructArray::Scan(const StructArray* sad, type_scan::Scanner& scanner) {
  auto const types = sad->rawTypes();
  auto const vals = sad->rawValues();
  for (Slot i = 0; i < sad->numFields(); i++) {
    if (isRefcountedType(types[i])) {
      scanner.scan(vals[i].pcnt);
    }
  }
}

ArrayData* StructArray::EscalateToVanilla(const StructArray* sad,
                                          const char* reason) {
  return sad->escalateWithCapacity(sad->size(), reason);
}

void StructArray::addNextSlot(Slot slot) {
  assertx(slot < kMaxKeyNum);
  rawPositions()[m_size++] = slot;
}

void StructArray::removeSlot(Slot slot) {
  auto const pos = rawPositions();
  auto idx = 0;
  for (size_t i = 0; i < m_size; i++) {
    auto const curr = pos[i];
    if (curr == slot) continue;
    pos[idx++] = curr;
  }
  m_size--;
}

Slot StructArray::getSlotInPos(size_t pos) const {
  assertx(pos < m_size);
  assertx(pos < kMaxKeyNum);
  return rawPositions()[pos];
}

//////////////////////////////////////////////////////////////////////////////

}}
