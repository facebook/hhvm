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

#include "hphp/runtime/base/bespoke/escalation-logging.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/tv-uncounted.h"
#include "hphp/runtime/base/vanilla-dict-defs.h"
#include "hphp/runtime/base/vanilla-dict.h"

#include "hphp/runtime/vm/jit/array-layout.h"
#include "hphp/runtime/vm/jit/type.h"

#include <sstream>

namespace HPHP { namespace bespoke {

//////////////////////////////////////////////////////////////////////////////

namespace {

static constexpr size_t kMaxNumStructLayouts = 1 << 14;
size_t s_numStructLayouts = 0;

struct FieldVectorHash {
  size_t operator()(const StructLayout::FieldVector& fv) const {
    auto hash = size_t {0};
    for (auto const& f : fv) {
      static_assert(sizeof(strhash_t) == 4);
      auto const here = static_cast<uint64_t>(f.key->hash()) |
                        static_cast<uint64_t>(f.required) << 32;
      hash = folly::hash::hash_combine(hash, here);
    }
    return hash;
  }
};

folly::SharedMutex s_fieldVectorLock;
std::unordered_map<StructLayout::FieldVector, LayoutIndex, FieldVectorHash>
  s_fieldVectorToIdx;

const LayoutFunctions* structDictVtable() {
  static auto const result = fromArray<StructDict>();
  return &result;
}

uint16_t packSizeIndexAndAuxBits(uint8_t idx, uint8_t aux) {
  return (static_cast<uint16_t>(idx) << 8) | aux;
}

std::string describeStructLayout(const StructLayout::FieldVector& fv) {
  std::stringstream ss;
  for (auto i = 0; i < fv.size(); ++i) {
    auto const& field = fv[i];
    if (i > 0) ss << ',';
    if (!field.required) ss << '?';
    ss << '"' << folly::cEscape<std::string>(field.key->data()) << '"';
  }
  return folly::sformat("StructDict<{}>", ss.str());
}

constexpr uint16_t indexRaw(uint16_t idx) {
  auto const hi_byte = idx >> 8;
  auto const lo_byte = idx & 0xff;
  auto const res = (hi_byte << 9) + lo_byte + 0x100;
  return static_cast<uint16_t>(res);
}

constexpr size_t colorTableLen = indexRaw(kMaxNumStructLayouts);
StructLayout::PerfectHashTable* s_hashTableSet = nullptr;

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
  assertx(layout()->valueOffset() == valueOffset());
  assertx(StructLayout::IsStructLayout(layoutIndex()));
  return true;
}

size_t StructLayout::typeOffsetForSlot(Slot slot) const {
  assertx(slot < numFields());
  return staticTypeOffset() + slot * sizeof(DataType);
}

size_t StructLayout::valueOffsetForSlot(Slot slot) const {
  assertx(slot < numFields());
  return (m_value_offset_in_values + slot) * sizeof(Value);
}

size_t StructLayout::positionOffset() const {
  return sizeof(StructDict) + numFields();
}

// As documented in bespoke/layout.h, bespoke layout bytes are constrained to
// have bit 0 (the low bit) set and bit 7 (the high bit) unset. Index() turns
// a serialize index into this form; IsStructLayout checks it.
LayoutIndex StructLayout::Index(uint16_t idx) {
  auto const result = indexRaw(idx);
  always_assert(IsStructLayout({result}));
  return {result};
}

bool StructLayout::IsStructLayout(LayoutIndex index) {
  auto const byte = index.byte();
  return (byte & 0b10000001) == 0b00000001;
}

const StructLayout* StructLayout::GetLayout(
    const FieldVector& fv, bool create) {
	if (fv.empty()) return nullptr;
  {
    folly::SharedMutex::ReadHolder rlock{s_fieldVectorLock};
    auto const it = s_fieldVectorToIdx.find(fv);
    if (it != s_fieldVectorToIdx.end()) return As(FromIndex(it->second));
  }
  if (!create) return nullptr;

  folly::SharedMutex::WriteHolder wlock{s_fieldVectorLock};
  auto const it = s_fieldVectorToIdx.find(fv);
  if (it != s_fieldVectorToIdx.end()) return As(FromIndex(it->second));

  if (s_numStructLayouts == kMaxNumStructLayouts) return nullptr;

  // We only construct this layout if it has at least one child, in order
  // to satisfy invariants in FinalizeHierarchy().
  if (s_numStructLayouts == 0) {
    new TopStructLayout();
    s_numStructLayouts++;
  }

  auto const index = Index(safe_cast<uint16_t>(s_numStructLayouts++));
  auto const bytes = sizeof(StructLayout) + sizeof(Field) * (fv.size() - 1);
  auto const result = new (malloc(bytes)) StructLayout(index, fv);
  s_fieldVectorToIdx.emplace(fv, index);
  return result;
}

const StructLayout* StructLayout::Deserialize(
    LayoutIndex index, const FieldVector& fv) {
  auto const layout = GetLayout(fv, true);
  always_assert(layout != nullptr);
  always_assert(layout->index() == index);
  return layout;
}

StructLayout::StructLayout(LayoutIndex index, const FieldVector& fv)
  : ConcreteLayout(index, describeStructLayout(fv),
                   {TopStructLayout::Index()}, structDictVtable())
{
  Slot i = 0;
  m_key_to_slot.reserve(fv.size());
  for (auto const& field : fv) {
    assertx(field.key->isStatic());
    if (field.required) m_num_required_fields++;
    m_key_to_slot.insert({StaticKey{field.key}, i});
    m_fields[i] = field;
    i++;
  }

  m_layout_index = index;
  m_num_fields = fv.size();
  assertx(numFields() == m_key_to_slot.size());

  static_assert(sizeof(Value) == 8);
  m_value_offset_in_values = (sizeof(StructDict) + 2 * numFields() + 7) / 8;

  auto const bytes = valueOffset() + numFields() * sizeof(Value);
  m_size_index = MemoryManager::size2Index(bytes);

  auto constexpr extra_offset = offsetof(StructLayout, m_extra_initializer);
  static_assert(offsetof(StructLayout, m_layout_index) - extra_offset ==
                ArrayData::offsetOfBespokeIndex() - ArrayData::offsetofExtra());
}

size_t StructLayout::numFields() const {
  return m_num_fields;
}

size_t StructLayout::sizeIndex() const {
  return m_size_index;
}

uint32_t StructLayout::extraInitializer() const {
  return m_extra_initializer;
}

Slot StructLayout::keySlot(LayoutIndex index, const StringData* key) {
  if (!key->isStatic()) {
    auto const layout = StructLayout::As(Layout::FromIndex(index));
    return layout->keySlotNonStatic(key);
  }
  return keySlotStatic(index, key);
}

Slot StructLayout::keySlotStatic(LayoutIndex index, const StringData* key) {
  assertx(key->isStatic());
  auto const& entry = s_hashTableSet[index.raw][key->color()];
  auto const match = entry.str == key;
  if (debug) {
    auto const layout = StructLayout::As(Layout::FromIndex(index));
    auto const slot = layout->keySlotNonStatic(key);
    always_assert(slot == (match ? entry.slot : kInvalidSlot));
  }
  return match ? entry.slot : kInvalidSlot;
}

Slot StructLayout::keySlot(const StringData* key) const {
  if (!key->isStatic()) return keySlotNonStatic(key);
  return keySlotStatic(index(), key);
}

NEVER_INLINE
Slot StructLayout::keySlotNonStatic(const StringData* key) const {
  auto const it = m_key_to_slot.find(NonStaticKey{key});
  return it == m_key_to_slot.end() ? kInvalidSlot : it->second;
}

const StructLayout::Field& StructLayout::field(Slot slot) const {
  assertx(slot < numFields());
  return m_fields[slot];
}

template <bool Static>
StructDict* StructDict::MakeReserve(const StructLayout* layout, bool legacy) {
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
  sad->initHeader_16(HeaderKind::BespokeDict, OneReference, aux);
  sad->m_extra = layout->extraInitializer();
  sad->m_size = 0;

  memset(sad->rawTypes(), static_cast<int>(KindOfUninit), sad->numFields());
  assertx(sad->checkInvariants());
  return sad;
}

uint8_t StructDict::sizeIndex() const {
  return m_aux16 >> 8;
}

size_t StructDict::numFields() const {
  return m_extra_lo8;
}

size_t StructDict::positionOffset() const {
  return sizeof(StructDict) + numFields();
}

size_t StructDict::typeOffset() const {
  return sizeof(StructDict);
}

size_t StructDict::valueOffset() const {
  return m_extra_hi8 * sizeof(Value);
}

void StructLayout::createColoringHashMap() const {
  auto& table = s_hashTableSet[index().raw];
  for (auto i = 0; i < kMaxColor; i++) {
    table[i] = { nullptr, 0, 0 };
  }
  for (auto i = 0; i < numFields(); i++) {
    auto const key = m_fields[i].key;
    auto const color = key->color();
    assertx(color != StringData::kInvalidColor);
    assertx(table[color].str == nullptr);
    auto const slot = safe_cast<uint8_t>(keySlotNonStatic(key));
    auto const valueOffset = safe_cast<uint16_t>(valueOffsetForSlot(slot));
    table[color] = { key, valueOffset, slot, 0 };
  }
}

StructLayout::PerfectHashTable* StructLayout::hashTableSet() {
  assertx(Layout::HierarchyFinalized());
  return s_hashTableSet;
}

StructLayout::PerfectHashTable* StructLayout::hashTable(const Layout* layout) {
  assertx(StructLayout::As(layout));
  assertx(Layout::HierarchyFinalized());
  return &s_hashTableSet[layout->index().raw];
}

const StructLayout* StructLayout::As(const Layout* l) {
  assertx(dynamic_cast<const StructLayout*>(l));
  return reinterpret_cast<const StructLayout*>(l);
}

StructDict* StructDict::MakeFromVanilla(ArrayData* ad,
                                        const StructLayout* layout) {
  if (!ad->isVanillaDict()) return nullptr;;

  assertx(layout);
  auto const result = ad->isStatic()
    ? MakeReserve<true>(layout, ad->isLegacyArray())
    : MakeReserve<false>(layout, ad->isLegacyArray());

  auto fail = false;
  auto const types = result->rawTypes();
  auto const vals = result->rawValues();
  auto required = layout->numRequiredFields();
  auto pos = result->rawPositions();

  VanillaDict::IterateKV(VanillaDict::as(ad), [&](auto k, auto v) -> bool {
    if (!tvIsString(k)) {
      fail = true;
      return true;
    }
    auto const slot = layout->keySlot(val(k).pstr);
    if (slot == kInvalidSlot) {
      fail = true;
      return true;
    }
    if (layout->field(slot).required) required--;

    *pos++ = slot;
    result->m_size++;
    types[slot] = type(v);
    vals[slot] = val(v);
    tvIncRefGen(v);
    return false;
  });

  if (fail || required) {
    if (!ad->isStatic()) Release(result);
    return nullptr;
  }

  if (ad->isStatic()) {
    auto const aux = packSizeIndexAndAuxBits(
        result->sizeIndex(), result->auxBits());
    result->initHeader_16(HeaderKind::BespokeDict, StaticValue, aux);
  }

  assertx(result->checkInvariants());
  return result;
}

StructDict* StructDict::MakeEmpty(const StructLayout* layout) {
  auto const sizeIndex = safe_cast<uint8_t>(layout->sizeIndex());
  return AllocStructDict(sizeIndex, layout->extraInitializer());
}

StructDict* StructDict::AllocStructDict(uint8_t sizeIndex, uint32_t extra) {
  auto const mem = tl_heap->objMallocIndex(sizeIndex);
  auto const sad = static_cast<StructDict*>(mem);
  auto const aux = packSizeIndexAndAuxBits(sizeIndex, 0);
  sad->initHeader_16(HeaderKind::BespokeDict, OneReference, aux);
  sad->m_extra = extra;
  sad->m_size = 0;

  memset(sad->rawTypes(), static_cast<int>(KindOfUninit), sad->numFields());
  assertx(sad->checkInvariants());
  return sad;
}

StructDict* StructDict::MakeStructDict(
    uint8_t sizeIndex, uint32_t extra, uint32_t size,
    const uint8_t* slots, const TypedValue* tvs) {
  auto const result = AllocStructDict(sizeIndex, extra);

  result->m_size = size;
  auto const positions = result->rawPositions();
  memcpy(positions, slots, size);

  auto const types = result->rawTypes();
  auto const vals = result->rawValues();

  for (auto i = 0; i < size; i++) {
    assertx(slots[i] < result->numFields());
    auto const& tv = tvs[size - i - 1];
    types[slots[i]] = type(tv);
    vals[slots[i]] = val(tv);
  }

  assertx(result->checkInvariants());
  assertx(result->size() == size);
  return result;
}

const StructLayout* StructDict::layout() const {
  return StructLayout::As(Layout::FromIndex(layoutIndex()));
}

DataType* StructDict::rawTypes() {
  assertx(typeOffset() == layout()->typeOffset());
  return reinterpret_cast<DataType*>(
      reinterpret_cast<char*>(this) + typeOffset());
}

const DataType* StructDict::rawTypes() const {
  return const_cast<StructDict*>(this)->rawTypes();
}

Value* StructDict::rawValues() {
  return reinterpret_cast<Value*>(
      reinterpret_cast<char*>(this) + valueOffset());
}

const Value* StructDict::rawValues() const {
  return const_cast<StructDict*>(this)->rawValues();
}

uint8_t* StructDict::rawPositions() {
  return reinterpret_cast<uint8_t*>(
      reinterpret_cast<char*>(this) + positionOffset());
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

  auto ad = VanillaDict::MakeReserveDict(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());

  auto const layout = this->layout();
  for (auto i = 0; i < m_size; i++) {
    auto const slot = getSlotInPos(i);
    auto const k = layout->field(slot).key;
    auto const tv = typedValueUnchecked(slot);
    auto const res =
      VanillaDict::SetStrMove(ad, const_cast<StringData*>(k.get()), tv);
    assertx(ad == res);
    tvIncRefGen(tv);
    ad = res;
  }
  assertx(ad->size() == size());
  return ad;
}

void StructDict::ConvertToUncounted(
    StructDict* sad, const MakeUncountedEnv& env) {
  auto const size = sad->size();
  auto const types = sad->rawTypes();
  auto const values = sad->rawValues();
  for (auto pos = 0; pos < size; pos++) {
    auto const slot = sad->getSlotInPos(pos);
    auto const lval = tv_lval { &types[slot], &values[slot] };
    ConvertTvToUncounted(lval, env);
  }
}

void StructDict::ReleaseUncounted(StructDict* sad) {
  auto const size = sad->size();
  for (auto pos = 0; pos < size; pos++) {
    auto const slot = sad->getSlotInPos(pos);
    DecRefUncounted(sad->typedValueUnchecked(slot));
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

NEVER_INLINE
TypedValue StructDict::NvGetStrNonStatic(
    const StructDict* sad, const StringData* k) {
  auto const structLayout = sad->layout();
  auto const slot = structLayout->keySlotNonStatic(k);
  if (slot == kInvalidSlot) return make_tv<KindOfUninit>();
  return sad->typedValueUnchecked(slot);
}

TypedValue StructDict::NvGetStr(const StructDict* sad, const StringData* k) {
  static_assert(folly::isPowTwo(StructLayout::kMaxColor + 1));
  auto const color = k->color() & StructLayout::kMaxColor;
  auto const& entry = s_hashTableSet[sad->layoutIndex().raw][color];

  // Perfect hash table miss: k is definitely missing if it's a static string;
  // else, we need to fall back to a hash table lookup for the non-static k.
  if (entry.str != k) {
    if (k->isStatic()) {
      assertx(NvGetStrNonStatic(sad, k).m_type == KindOfUninit);
      return make_tv<KindOfUninit>();
    }
    return NvGetStrNonStatic(sad, k);
  }

  // Perfect hash table hit: we can use the field offsets.
  auto const typeOffset = entry.slot + StructLayout::staticTypeOffset();
  auto const valueOffset = entry.valueOffset;
  auto const type = *reinterpret_cast<const DataType*>(
      reinterpret_cast<uintptr_t>(sad) + typeOffset);
  auto const value = *reinterpret_cast<const Value*>(
      reinterpret_cast<uintptr_t>(sad) + valueOffset);
  if constexpr (debug) {
    auto const result = NvGetStrNonStatic(sad, k);
    always_assert(result.m_type == type);
    always_assert(result.m_data.pcnt == value.pcnt);
  }
  return make_tv_of_type(value, type);
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
  auto const slot = StructLayout::keySlot(sad->layoutIndex(), key);
  if (slot == kInvalidSlot) throwOOBArrayKeyException(key, sad);
  auto const currType = sad->rawTypes()[slot];
  if (currType == KindOfUninit) throwOOBArrayKeyException(key, sad);
  auto const newad = sad->cowCheck() ? sad->copy() : sad;
  return { newad, &newad->rawTypes()[slot], &newad->rawValues()[slot] };
}

tv_lval StructDict::ElemInt(tv_lval lval, int64_t k, bool throwOnMissing) {
  if (throwOnMissing) throwOOBArrayKeyException(k, lval.val().parr);
  return const_cast<TypedValue*>(&immutable_null_base);
}

arr_lval StructDict::elemImpl(StringData* k, bool throwOnMissing) {
  auto const slot = StructLayout::keySlot(layoutIndex(), k);
  if (slot == kInvalidSlot) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
  auto const currType = rawTypes()[slot];
  if (currType == KindOfUninit) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
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
  auto const res = VanillaDict::SetIntMove(vad, k, v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructDict::SetStrMove(StructDict* sadIn,
                                  StringData* k,
                                  TypedValue v) {
  auto const slot = StructLayout::keySlot(sadIn->layoutIndex(), k);
  if (slot == kInvalidSlot) {
    auto const vad = sadIn->escalateWithCapacity(sadIn->size() + 1, __func__);
    auto const res = VanillaDict::SetStrMove(vad, k, v);
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

NEVER_INLINE
StructDict* StructDict::copy() const {
  auto const sizeIdx = sizeIndex();
  auto const sad = static_cast<StructDict*>(tl_heap->objMallocIndex(sizeIdx));
  auto const heapSize = MemoryManager::sizeIndex2Size(sizeIdx);
  assertx(heapSize % 16 == 0);
  memcpy16_inline(sad, this, heapSize);
  auto const aux = packSizeIndexAndAuxBits(sizeIdx, auxBits());
  sad->initHeader_16(HeaderKind::BespokeDict, OneReference, aux);
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

ArrayData* StructDict::RemoveIntMove(StructDict* sad, int64_t) {
  return sad;
}

ArrayData* StructDict::RemoveStrMove(StructDict* sadIn, const StringData* k) {
  auto const slot = StructLayout::keySlot(sadIn->layoutIndex(), k);
  if (slot == kInvalidSlot) return sadIn;
  if (sadIn->layout()->field(slot).required) {
    auto const vad = sadIn->escalateWithCapacity(sadIn->size(), __func__);
    if (sadIn->decReleaseCheck()) Release(sadIn);
    return VanillaDict::RemoveStrMove(vad, k);
  }
  return RemoveStrInSlot(sadIn, slot);
}

ArrayData* StructDict::RemoveStrInSlot(StructDict* sadIn, Slot slot) {
  assertx(!sadIn->layout()->field(slot).required);
  auto const currType = sadIn->rawTypes()[slot];
  if (currType == KindOfUninit) return sadIn;

  auto const sad = [&] {
    if (!sadIn->cowCheck()) return sadIn;
    auto const result = sadIn->copy();
    sadIn->decRefCount();
    return result;
  }();

  tvDecRefGen(sad->typedValueUnchecked(slot));
  auto& t = sad->rawTypes()[slot];
  t = KindOfUninit;
  sad->removeSlot(slot);

  return sad;
}

ArrayData* StructDict::AppendMove(StructDict* sad, TypedValue v) {
  auto const vad = sad->escalateWithCapacity(sad->size() + 1, __func__);
  auto const res = VanillaDict::AppendMove(vad, v);
  assertx(vad == res);
  if (sad->decReleaseCheck()) Release(sad);
  return res;
}

ArrayData* StructDict::PopMove(StructDict* sadIn, Variant& value) {
  if (UNLIKELY(sadIn->size() == 0)) {
    value = uninit_null();
    return sadIn;
  }

  auto const sad = [&] {
    if (!sadIn->cowCheck()) return sadIn;
    auto const res = sadIn->copy();
    sadIn->decRefCount();
    return res;
  }();

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
  VanillaDict::Release(vad);
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

TopStructLayout::TopStructLayout()
  : AbstractLayout(Index(), "StructDict<Top>",
                   {AbstractLayout::GetBespokeTopIndex()}, structDictVtable())
{
  assertx(!s_hashTableSet);
  s_hashTableSet = (StructLayout::PerfectHashTable*) vm_malloc(
      sizeof(StructLayout::PerfectHashTable) * colorTableLen);
}

LayoutIndex TopStructLayout::Index() {
  return StructLayout::Index(0);
}

//////////////////////////////////////////////////////////////////////////////

using namespace jit;

ArrayLayout StructLayout::appendType(Type val) const {
  return ArrayLayout::Vanilla();
}

ArrayLayout StructLayout::removeType(Type key) const {
  if (!key.hasConstVal(TStr)) return ArrayLayout::Top();
  auto const slot = keySlot(key.strVal());
  if (slot == kInvalidSlot) return ArrayLayout(this);
  auto const& f = field(slot);
  return f.required ? ArrayLayout::Vanilla() : ArrayLayout(this);
}

ArrayLayout StructLayout::setType(Type key, Type val) const {
  if (key <= TInt) return ArrayLayout::Vanilla();
  if (!key.hasConstVal(TStr)) return ArrayLayout::Top();
  auto const slot = keySlotStatic(index(), key.strVal());
  return slot == kInvalidSlot ? ArrayLayout::Vanilla() : ArrayLayout(this);
}

std::pair<Type, bool> StructLayout::elemType(Type key) const {
  if (key <= TInt) return {TBottom, false};
  if (!key.hasConstVal(TStr)) return {TInitCell, false};
  auto const slot = keySlotStatic(index(), key.strVal());
  return slot == kInvalidSlot ? std::pair{TBottom, false}
                              : std::pair{TInitCell, field(slot).required};
}

std::pair<Type, bool> StructLayout::firstLastType(
    bool isFirst, bool isKey) const {
  return {isKey ? TStaticStr : TInitCell, false};
}

Type StructLayout::iterPosType(Type pos, bool isKey) const {
  return isKey ? TStaticStr : TInitCell;
}

ArrayLayout TopStructLayout::appendType(Type val) const {
  return ArrayLayout::Vanilla();
}

ArrayLayout TopStructLayout::removeType(Type key) const {
  return ArrayLayout::Top();
}

ArrayLayout TopStructLayout::setType(Type key, Type val) const {
  return ArrayLayout::Top();
}

std::pair<Type, bool> TopStructLayout::elemType(Type key) const {
  return key <= TInt ? std::pair{TBottom, false}
                     : std::pair{TInitCell, false};
}

std::pair<Type, bool> TopStructLayout::firstLastType(
    bool isFirst, bool isKey) const {
  return {isKey ? TStaticStr : TInitCell, false};
}

Type TopStructLayout::iterPosType(Type pos, bool isKey) const {
  return isKey ? TStaticStr : TInitCell;
}

//////////////////////////////////////////////////////////////////////////////

}}
