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
  return folly::sformat("StructLayout<{}>", base.substr(1, base.size() - 2));
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
  assertx(layout()->index() == layoutIndex());
  assertx(layout()->sizeIndex() == sizeIndex());
  assertx(layoutIndex().byte() == kStructLayoutByte);
  return true;
}

LayoutIndex StructLayout::Index(uint8_t idx) {
  auto constexpr base = uint16_t(kStructLayoutByte << 8);
  return LayoutIndex{uint16_t(base + idx)};
}

const StructLayout* StructLayout::getLayout(const KeyOrder& ko, bool create) {
  if (ko.empty() || !ko.valid()) return nullptr;

  {
    folly::SharedMutex::ReadHolder rlock{s_keySetLock};
    auto const it = s_keySetToIdx.find(ko);
    if (it != s_keySetToIdx.end()) {
      return StructLayout::As(Layout::FromIndex(it->second));
    }
  }

  if (!create) return nullptr;

  folly::SharedMutex::WriteHolder wlock{s_keySetLock};
  auto const it = s_keySetToIdx.find(ko);
  if (it != s_keySetToIdx.end()) {
    return StructLayout::As(Layout::FromIndex(it->second));
  }

  auto constexpr kMaxIndex = std::numeric_limits<uint8_t>::max();
  if (s_numStructLayouts == kMaxIndex) return nullptr;
  auto const nextIdx = Index(s_numStructLayouts++);
  auto const ret = new StructLayout(ko, nextIdx);
  s_keySetToIdx.emplace(ko, nextIdx);
  return ret;
}

StructLayout::StructLayout(const KeyOrder& ko, const LayoutIndex& idx)
  : ConcreteLayout(idx, describeStructLayout(ko),
                   {AbstractLayout::GetBespokeTopIndex()},
                   structArrayVtable())
{
  FixedStringMapBuilder<Slot, Slot, true> builder;
  Slot i = 0;
  for (auto const key : ko) {
    builder.add(key, i);
    m_slotToKey[i] = key;
    i++;
  }
  builder.create(m_fields);
  m_size_index = MemoryManager::size2Index(arraySize());
}

uint8_t StructArray::sizeIndex() const {
  return m_aux16 >> 8;
}

size_t StructLayout::arraySize() const {
  return sizeof(StructArray) + numFields() * sizeof(TypedValue);
}

size_t StructLayout::numFields() const {
  return m_fields.size();
}

size_t StructLayout::sizeIndex() const {
  return m_size_index;
}

Slot StructLayout::keySlot(const StringData* key) const {
  // TODO(arnabde): Use separate heterogeneous lookups for static and
  // non-static keys after converting m_fields to an F14Map.
  auto const it = m_fields.find(key);
  return it ? *it : kInvalidSlot;
}

LowStringPtr StructLayout::key(Slot slot) const {
  assertx(slot < StructArray::kMaxKeyNum);
  return m_slotToKey[slot];
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
  sad->m_extra_lo16 = 0;
  sad->m_size = 0;
  auto const dataSize = layout->numFields() * sizeof(TypedValue);
  memset(sad->rawData(), static_cast<int>(KindOfUninit), dataSize);
  assertx(sad->checkInvariants());
  return sad;
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
  auto const data = result->rawData();
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
    tvDup(v, data[slot]);
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

const StructLayout* StructArray::layout() const {
  return StructLayout::As(Layout::FromIndex(layoutIndex()));
}

TypedValue* StructArray::rawData() {
  return reinterpret_cast<TypedValue*>(this + 1);
}

const TypedValue* StructArray::rawData() const {
  return const_cast<StructArray*>(this)->rawData();
}

ArrayData* StructArray::escalateWithCapacity(size_t capacity,
                                             const char* reason) const {
  assertx(capacity >= size());
  logEscalateToVanilla(this, reason);

  auto ad = isDictType() ? MixedArray::MakeReserveDict(capacity)
                         : MixedArray::MakeReserveDArray(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());
  auto const layout = this->layout();
  auto const data = rawData();
  for (auto i = 0; i < m_size; i++) {
    auto const slot = getSlotInPos(i);
    auto const k = layout->key(slot);
    auto const& v = data[slot];
    auto const res =
      MixedArray::SetStrMove(ad, const_cast<StringData*>(k.get()), v);
    assertx(ad == res);
    tvIncRefGen(v);
    ad = res;
  }
  assertx(ad->size() == size());
  return ad;
}

void StructArray::ConvertToUncounted(StructArray* sad,
                                     DataWalker::PointerMap* seen) {
  auto const layout = sad->layout();
  auto const numFields = layout->numFields();
  for (Slot i = 0; i < numFields; i++) {
    auto const lval = tv_lval(&sad->rawData()[i]);
    ConvertTvToUncounted(lval, seen);
  }
}

void StructArray::ReleaseUncounted(StructArray* sad) {
  auto const layout = sad->layout();
  auto const numFields = layout->numFields();
  for (Slot i = 0; i < numFields; i++) {
    auto& tv = sad->rawData()[i];
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
  auto const elems = sad->rawData();
  return elems[slot];
}

TypedValue StructArray::GetPosKey(const StructArray* sad, ssize_t pos) {
  auto const layout = sad->layout();
  auto const slot = sad->getSlotInPos(pos);
  auto const k = layout->key(slot);
  return make_tv<KindOfPersistentString>(k);
}

TypedValue StructArray::GetPosVal(const StructArray* sad, ssize_t pos) {
  auto const slot = sad->getSlotInPos(pos);
  auto const data = sad->rawData();
  return data[slot];
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
  auto const& curr = sad->rawData()[slot];
  if (type(curr) == KindOfUninit) throwOOBArrayKeyException(key, sad);
  auto const newad = sad->cowCheck() ? sad->copy() : sad;
  auto const data = newad->rawData();
  auto& v = data[slot];
  return { newad, &v };
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
  auto const& curr = rawData()[slot];
  if (type(curr) == KindOfUninit) {
    if (throwOnMissing) throwOOBArrayKeyException(k, this);
    return {this, const_cast<TypedValue*>(&immutable_null_base)};
  }
  if (type(curr) == KindOfClsMeth) return LvalStr(this, k);
  auto const sad = cowCheck() ? this->copy() : this;
  auto& v = sad->rawData()[slot];
  v.m_type = dt_modulo_persistence(v.m_type);
  return arr_lval{sad, &v};
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
  auto data = sad->rawData();
  auto& old = data[slot];
  if (type(old) == KindOfUninit) {
    sad->addNextSlot(slot);
  }
  old = v;
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
  auto const data = rawData();
  for (auto i = 0; i < layout()->numFields(); i++) {
    tvIncRefGen(data[i]);
  }
}

void StructArray::decRefValues() {
  auto const data = rawData();
  for (auto i = 0; i < layout()->numFields(); i++) {
    tvDecRefGen(data[i]);
  }
}

ArrayData* StructArray::RemoveInt(StructArray* sad, int64_t) {
  return sad;
}

ArrayData* StructArray::RemoveStr(StructArray* sadIn, const StringData* k) {
  auto const layout = sadIn->layout();
  auto const slot = layout->keySlot(k);
  if (slot == kInvalidSlot) return sadIn;
  auto const& curr = sadIn->rawData()[slot];
  if (type(curr) == KindOfUninit) return sadIn;
  auto const sad = sadIn->cowCheck() ? sadIn->copy() : sadIn;
  auto& v = sad->rawData()[slot];
  tvDecRefGen(v);
  v.m_type = KindOfUninit;
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
  auto& v = sad->rawData()[slot];
  value = Variant::attach(v);
  v.m_type = KindOfUninit;
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
  auto fields = sad->rawData();
  scanner.scan(*fields, sad->size() * sizeof(TypedValue));
}

ArrayData* StructArray::EscalateToVanilla(const StructArray* sad,
                                          const char* reason) {
  return sad->escalateWithCapacity(sad->size(), reason);
}

void StructArray::addNextSlot(Slot slot) {
  assertx(slot < kMaxKeyNum);
  m_order = (m_order << kSlotSize) | slot;
  m_size++;
}

void StructArray::removeSlot(Slot slot) {
  uint64_t mask = 0;
  for (size_t i = 0; i < m_size; i++) {
    auto const curr = m_order >> (i * kSlotSize) & 0xF;
    if (curr == slot) break;
    mask = (mask << kSlotSize) | 0xF;
  }
  auto const high = (m_order >> kSlotSize) & ~mask;
  auto const low = m_order & mask;
  m_order = high | low;
  m_size--;
}

Slot StructArray::getSlotInPos(size_t pos) const {
  assertx(pos < m_size);
  auto const order = m_size - pos - 1;
  assertx(pos < kMaxKeyNum);
  return (m_order >> (order * kSlotSize)) & 0xF;
}

//////////////////////////////////////////////////////////////////////////////

}}
