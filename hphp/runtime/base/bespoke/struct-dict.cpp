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

#include "hphp/runtime/base/bespoke/struct-dict.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/tv-uncounted.h"

#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

namespace {

uint8_t s_numStructLayouts = 0;
folly::SharedMutex s_keySetLock;
std::unordered_map<KeyOrder, LayoutIndex, KeyOrderHash> s_keySetToIdx;

const LayoutFunctions* structArrayVtable() {
  static auto const result = fromArray<StructDict>();
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
  return folly::sformat("StructDict<{}>", base.substr(1, base.size() - 2));
}

}

//////////////////////////////////////////////////////////////////////////////

StructDict* StructDict::As(ArrayData* ad) {
  auto const result = reinterpret_cast<StructDict*>(ad);
  assertx(result->checkInvariants());
  return result;
}

const StructDict* StructDict::As(const ArrayData* ad) {
  return As(const_cast<ArrayData*>(ad));
}

bool StructDict::checkInvariants() const {
  static_assert(sizeof(StructDict) == 16);
  assertx(layout()->index() == layoutIndex());
  assertx(layout()->sizeIndex() == sizeIndex());
  assertx(layout()->numFields() == numFields());
  assertx(layout()->typeOffset() == typeOffset());
  assertx(layout()->valueOffset() == valueOffsetInValueSize() * sizeof(Value));
  assertx(layoutIndex().byte() == kStructLayoutByte);
  return true;
}

size_t StructLayout::typeOffsetForSlot(Slot slot) const {
  return sizeof(StructDict) + typeOffset() + slot;
}

size_t StructLayout::valueOffsetForSlot(Slot slot) const {
  return sizeof(StructDict) + valueOffset() + slot * sizeof(Value);
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
  auto const result = new (malloc(bytes)) StructLayout(index, ko);
  s_keySetToIdx.emplace(ko, index);
  return result;
}

const StructLayout* StructLayout::Deserialize(
    LayoutIndex index, const KeyOrder& ko) {
  auto const layout = GetLayout(ko, true);
  always_assert(layout != nullptr);
  always_assert(layout->index() == index);
  return layout;
}

StructLayout::StructLayout(LayoutIndex index, const KeyOrder& ko)
  : ConcreteLayout(index, describeStructLayout(ko),
                   {AbstractLayout::GetBespokeTopIndex()},
                   structArrayVtable())
  , m_key_order(ko)
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
  m_type_offset = numFields();
  m_value_offset = (m_type_offset + numFields() + 7) & ~7;
  auto const bytes = sizeof(StructDict) +
                     m_value_offset +
                     numFields() * sizeof(Value);
  m_size_index = MemoryManager::size2Index(bytes);
}

uint8_t StructDict::sizeIndex() const {
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
StructDict* StructDict::MakeReserve(HeaderKind kind,
                                    bool legacy,
                                    const StructLayout* layout) {
  assertx(layout);
  auto const sizeIdx = layout->sizeIndex();
  auto const alloc = [&] {
    if (!Static) return tl_heap->objMallocIndex(sizeIdx);
    auto const size = MemoryManager::sizeIndex2Size(sizeIdx);
    return RO::EvalLowStaticArrays ? low_malloc(size) : uncounted_malloc(size);
  }();

  auto const sad = static_cast<StructDict*>(alloc);
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

size_t StructDict::numFields() const {
  return (m_extra_lo16 >> 8) & 0xff;
}

size_t StructDict::valueOffsetInValueSize() const {
  return m_extra_lo16 & 0xff;
}

const StructLayout* StructLayout::As(const Layout* l) {
  assertx(dynamic_cast<const StructLayout*>(l));
  return reinterpret_cast<const StructLayout*>(l);
}

using namespace jit;

std::pair<Type, bool> StructLayout::elemType(Type key) const {
  if (key <= TInt) return {TBottom, false};
  if (key.hasConstVal(TStr)) {
    auto const keyVal = key.strVal();
    auto const slot = keySlot(keyVal);
    return slot == kInvalidSlot ? std::pair{TBottom, false} :
                                  std::pair{TInitCell, false};
  }
  return {TInitCell, false};
}

ArrayLayout StructLayout::setType(Type key, Type val) const {
  if (key.maybe(TInt)) return ArrayLayout::Vanilla();
  if (!key.maybe(TStr)) return ArrayLayout::Top();
  if (!val.maybe(TInitCell)) return ArrayLayout::Bottom();
  if (!key.hasConstVal()) return ArrayLayout::Top();
  auto const keyStr = key.strVal();
  auto const slot = keySlot(keyStr);
  if (slot == kInvalidSlot) return ArrayLayout::Vanilla();
  return ArrayLayout(this);
}

StructDict* StructDict::MakeFromVanilla(ArrayData* ad,
                                        const StructLayout* layout) {
  if (!ad->isVanillaDict()) return nullptr;;

  assertx(layout);
  auto const kind = HeaderKind::BespokeDict;
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

StructDict* StructDict::AllocStructDict(const StructLayout* layout) {
  return MakeReserve<false>(HeaderKind::BespokeDict, false, layout);
}

StructDict* StructDict::MakeStructDict(
    const StructLayout* layout, uint32_t size,
    const uint8_t* slots, const TypedValue* tvs) {

  auto const result = MakeReserve<false>(
      HeaderKind::BespokeDict, false, layout);
  result->m_size = size;

  auto const positions = result->rawPositions();
  assertx(uintptr_t(positions) % 8 == 0);
  assertx(uintptr_t(slots) % 8 == 0);
  memcpy8(positions, slots, size);

  auto const types = result->rawTypes();
  auto const vals = result->rawValues();

  for (auto i = 0; i < size; i++) {
    assertx(slots[i] <= layout->numFields());
    auto const& tv = tvs[size - i - 1];
    types[slots[i]] = type(tv);
    vals[slots[i]] = val(tv);
  }

  assertx(result->checkInvariants());
  assertx(result->layout() == layout);
  assertx(result->size() == size);
  return result;
}

const StructLayout* StructDict::layout() const {
  return StructLayout::As(Layout::FromIndex(layoutIndex()));
}

DataType* StructDict::rawTypes() {
  assertx(typeOffset() == layout()->typeOffset());
  return reinterpret_cast<DataType*>(
      reinterpret_cast<char*>(this + 1) + typeOffset());
}

const DataType* StructDict::rawTypes() const {
  return const_cast<StructDict*>(this)->rawTypes();
}

Value* StructDict::rawValues() {
  return reinterpret_cast<Value*>(
      reinterpret_cast<Value*>(this + 1) + valueOffsetInValueSize());
}

const Value* StructDict::rawValues() const {
  return const_cast<StructDict*>(this)->rawValues();
}

uint8_t* StructDict::rawPositions() {
  return reinterpret_cast<uint8_t*>(this + 1);
}

const uint8_t* StructDict::rawPositions() const {
  return const_cast<StructDict*>(this)->rawPositions();
}

TypedValue StructDict::typedValueUnchecked(Slot slot) const {
  return make_tv_of_type(rawValues()[slot], rawTypes()[slot]);
}

ArrayData* StructDict::escalateWithCapacity(size_t capacity,
                                             const char* reason) const {
  assertx(capacity >= size());
  logEscalateToVanilla(this, reason);

  auto ad = MixedArray::MakeReserveDict(capacity);
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

void StructDict::ConvertToUncounted(StructDict* sad,
                                    DataWalker::PointerMap* seen) {
  for (Slot i = 0; i < sad->numFields(); i++) {
    auto const lval = tv_lval(&sad->rawTypes()[i], &sad->rawValues()[i]);
    ConvertTvToUncounted(lval, seen);
  }
}

void StructDict::ReleaseUncounted(StructDict* sad) {
  for (Slot i = 0; i < sad->numFields(); i++) {
    auto const tv = sad->typedValueUnchecked(i);
    DecRefUncounted(tv);
  }
}

void StructDict::Release(StructDict* sad) {
  sad->fixCountForRelease();
  assertx(sad->isRefCounted());
  assertx(sad->hasExactlyOneRef());
  sad->decRefValues();
  tl_heap->objFreeIndex(sad, sad->sizeIndex());
}

bool StructDict::IsVectorData(const StructDict* sad) {
  return sad->empty();
}

TypedValue StructDict::NvGetInt(const StructDict*, int64_t) {
  return make_tv<KindOfUninit>();
}

TypedValue StructDict::NvGetStr(const StructDict* sad, const StringData* k) {
  auto const layout = sad->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) return make_tv<KindOfUninit>();
  return sad->typedValueUnchecked(slot);
}

TypedValue StructDict::GetPosKey(const StructDict* sad, ssize_t pos) {
  auto const layout = sad->layout();
  auto const slot = sad->getSlotInPos(pos);
  auto const k = layout->field(slot).key;
  return make_tv<KindOfPersistentString>(k);
}

TypedValue StructDict::GetPosVal(const StructDict* sad, ssize_t pos) {
  auto const slot = sad->getSlotInPos(pos);
  return sad->typedValueUnchecked(slot);
}

ssize_t StructDict::IterBegin(const StructDict*) {
  return 0;
}

ssize_t StructDict::IterLast(const StructDict* sad) {
  return sad->empty() ? 0 : sad->size() - 1;
}

ssize_t StructDict::IterEnd(const StructDict* sad) {
  return sad->size();
}

ssize_t StructDict::IterAdvance(const StructDict* sad, ssize_t pos) {
  return pos < sad->size() ? pos + 1 : pos;
}

ssize_t StructDict::IterRewind(const StructDict* sad, ssize_t pos) {
  return pos > 0 ? pos - 1 : sad->size();
}

arr_lval StructDict::LvalInt(StructDict* sad, int64_t k) {
  throwOOBArrayKeyException(k, sad);
}

arr_lval StructDict::LvalStr(StructDict* sad, StringData* key) {
  auto const layout = sad->layout();
  auto const slot = layout->keySlot(key);
  if (slot == kInvalidSlot) throwOOBArrayKeyException(key, sad);
  auto const& currType = sad->rawTypes()[slot];
  if (currType == KindOfUninit) throwOOBArrayKeyException(key, sad);
  auto const newad = sad->cowCheck() ? sad->copy() : sad;
  return { newad, &newad->rawTypes()[slot], &newad->rawValues()[slot] };
}

tv_lval StructDict::ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
  if (throwOnMissing) throwOOBArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

arr_lval StructDict::elemImpl(StringData* k, bool throwOnMissing) {
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

tv_lval StructDict::ElemStr(tv_lval lvalIn, StringData* k, bool throwOnMissing) {
  auto sadIn = As(lvalIn.val().parr);
  auto const lval = sadIn->elemImpl(k, throwOnMissing);
  if (lval.arr != sadIn) {
    lvalIn.type() = dt_with_rc(lvalIn.type());
    lvalIn.val().parr = lval.arr;
    if (sadIn->decReleaseCheck()) Release(sadIn);
  }
  return lval;
}

ArrayData* StructDict::SetIntMove(StructDict* sad, int64_t k, TypedValue v) {
  auto const vad = sad->escalateWithCapacity(sad->size() + 1, __func__);
  auto const res = MixedArray::SetIntMove(vad, k, v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructDict::SetStrMove(StructDict* sadIn,
                                  StringData* k,
                                  TypedValue v) {
  auto const layout = sadIn->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) {
    auto const vad = sadIn->escalateWithCapacity(sadIn->size() + 1, __func__);
    auto const res = vad->setMove(k, v);
    assertx(vad == res);
    if (sadIn->decReleaseCheck()) Release(sadIn);
    return res;
  }
  return SetStrInSlot(sadIn, slot, v);
}

ArrayData* StructDict::SetStrInSlot(StructDict* sadIn, Slot slot,
                                    TypedValue v) {
  assertx(slot != kInvalidSlot);
  assertx(slot < sadIn->numFields());
  auto const cow = sadIn->cowCheck();
  auto const sad = cow ? sadIn->copy() : sadIn;
  StructDict::SetStrInSlotInPlace(sad, slot, v);
  if (cow) sadIn->decRefCount();
  return sad;
}

void StructDict::SetStrInSlotInPlace(StructDict* sad, Slot slot,
                                     TypedValue v) {
  assertx(sad->hasExactlyOneRef());
  auto& oldType = sad->rawTypes()[slot];
  auto& oldVal = sad->rawValues()[slot];
  if (oldType == KindOfUninit) {
    sad->addNextSlot(slot);
  } else {
    tvDecRefGen(make_tv_of_type(oldVal, oldType));
  }
  oldType = type(v);
  oldVal = val(v);
}

StructDict* StructDict::copy() const {
  auto const sizeIdx = sizeIndex();
  auto const sad = static_cast<StructDict*>(tl_heap->objMallocIndex(sizeIdx));
  auto const heapSize = MemoryManager::sizeIndex2Size(sizeIdx);
  assertx(heapSize % 16 == 0);
  memcpy16_inline(sad, this, heapSize);
  auto const aux = packSizeIndexAndAuxBits(sizeIdx, auxBits());
  sad->initHeader_16(m_kind, OneReference, aux);
  sad->incRefValues();
  return sad;
}

void StructDict::incRefValues() {
  for (auto pos = 0; pos < m_size; pos++) {
    auto const tv = typedValueUnchecked(getSlotInPos(pos));
    tvIncRefGen(tv);
  }
}

void StructDict::decRefValues() {
  for (auto pos = 0; pos < m_size; pos++) {
    auto const tv = typedValueUnchecked(getSlotInPos(pos));
    tvDecRefGen(tv);
  }
}

ArrayData* StructDict::RemoveInt(StructDict* sad, int64_t) {
  return sad;
}

ArrayData* StructDict::RemoveStr(StructDict* sadIn, const StringData* k) {
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

ArrayData* StructDict::AppendMove(StructDict* sad, TypedValue v) {
  auto const vad = sad->escalateWithCapacity(sad->size() + 1, __func__);
  auto const res = vad->appendMove(v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructDict::Pop(StructDict* sadIn, Variant& value) {
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

ArrayData* StructDict::PreSort(StructDict* sad, SortFunction sf) {
  return sad->escalateWithCapacity(sad->size(), sortFunctionName(sf));
}

ArrayData* StructDict::PostSort(StructDict* sad, ArrayData* vad) {
  auto const result = MakeFromVanilla(vad, sad->layout());
  if (!result) return vad;
  MixedArray::Release(vad);
  return result;
}

ArrayData* StructDict::SetLegacyArray(StructDict* sadIn,
                                       bool copy, bool legacy) {
  auto const sad = copy ? sadIn->copy() : sadIn;
  sad->setLegacyArrayInPlace(legacy);
  return sad;
}

////////////////////////////////////////////////////////////////////////////

size_t StructDict::HeapSize(const StructDict* sad) {
  return MemoryManager::sizeIndex2Size(sad->sizeIndex());
}

void StructDict::Scan(const StructDict* sad, type_scan::Scanner& scanner) {
  auto const types = sad->rawTypes();
  auto const vals = sad->rawValues();
  for (Slot i = 0; i < sad->numFields(); i++) {
    if (isRefcountedType(types[i])) {
      scanner.scan(vals[i].pcnt);
    }
  }
}

ArrayData* StructDict::EscalateToVanilla(const StructDict* sad,
                                         const char* reason) {
  return sad->escalateWithCapacity(sad->size(), reason);
}

void StructDict::addNextSlot(Slot slot) {
  assertx(slot < RO::EvalBespokeStructDictMaxNumKeys);
  rawPositions()[m_size++] = slot;
}

void StructDict::removeSlot(Slot slot) {
  auto const pos = rawPositions();
  auto idx = 0;
  for (size_t i = 0; i < m_size; i++) {
    auto const curr = pos[i];
    if (curr == slot) continue;
    pos[idx++] = curr;
  }
  m_size--;
}

Slot StructDict::getSlotInPos(size_t pos) const {
  assertx(pos < m_size);
  assertx(pos < RO::EvalBespokeStructDictMaxNumKeys);
  return rawPositions()[pos];
}

//////////////////////////////////////////////////////////////////////////////

}}
