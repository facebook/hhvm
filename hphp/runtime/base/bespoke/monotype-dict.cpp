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

#include "hphp/runtime/base/bespoke/monotype-dict.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/logging-profile.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <algorithm>
#include <atomic>

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

uint16_t packSizeIndexAndAuxBits(uint8_t index, uint8_t aux) {
  return (static_cast<uint16_t>(index) << 8) | aux;
}

constexpr LayoutIndex kBaseLayoutIndex = {1 << 10};

constexpr size_t kEmptySizeIndex = 0;
static_assert(kSizeIndex2Size[kEmptySizeIndex] == sizeof(EmptyMonotypeDict));

std::aligned_storage<sizeof(EmptyMonotypeDict), 16>::type s_emptyDict;
std::aligned_storage<sizeof(EmptyMonotypeDict), 16>::type s_emptyDArray;
std::aligned_storage<sizeof(EmptyMonotypeDict), 16>::type s_emptyMarkedDict;
std::aligned_storage<sizeof(EmptyMonotypeDict), 16>::type s_emptyMarkedDArray;

constexpr LayoutIndex getEmptyLayoutIndex() {
  return LayoutIndex{uint16_t(kBaseLayoutIndex.raw)};
}
constexpr LayoutIndex getIntLayoutIndex(DataType type) {
  auto constexpr offset = 1 * (1 << 8);
  return LayoutIndex{uint16_t(kBaseLayoutIndex.raw + int(type) + offset)};
}
constexpr LayoutIndex getStrLayoutIndex(DataType type) {
  auto constexpr offset = 2 * (1 << 8);
  return LayoutIndex{uint16_t(kBaseLayoutIndex.raw + int(type) + offset)};
}
constexpr LayoutIndex getStaticStrLayoutIndex(DataType type) {
  auto constexpr offset = 3 * (1 << 8);
  return LayoutIndex{uint16_t(kBaseLayoutIndex.raw + int(type) + offset)};
}

}

//////////////////////////////////////////////////////////////////////////////

EmptyMonotypeDict* EmptyMonotypeDict::As(ArrayData* ad) {
  auto const result = reinterpret_cast<EmptyMonotypeDict*>(ad);
  assertx(result->checkInvariants());
  return result;
}
const EmptyMonotypeDict* EmptyMonotypeDict::As(const ArrayData* ad) {
  return As(const_cast<ArrayData*>(ad));
}
EmptyMonotypeDict* EmptyMonotypeDict::GetDict(bool legacy) {
  auto const mem = legacy ? &s_emptyMarkedDict : &s_emptyDict;
  return reinterpret_cast<EmptyMonotypeDict*>(mem);
}
EmptyMonotypeDict* EmptyMonotypeDict::GetDArray(bool legacy) {
  auto const mem = legacy ? &s_emptyMarkedDArray : &s_emptyDArray;
  return reinterpret_cast<EmptyMonotypeDict*>(mem);
}

bool EmptyMonotypeDict::checkInvariants() const {
  assertx(isStatic());
  assertx(m_size == 0);
  assertx(isDArray() || isDictType());
  assertx(layoutIndex() == getEmptyLayoutIndex());
  return true;
}

//////////////////////////////////////////////////////////////////////////////

size_t EmptyMonotypeDict::HeapSize(const Self*) {
  return sizeof(EmptyMonotypeDict);
}
void EmptyMonotypeDict::Scan(const Self* ad, type_scan::Scanner& scanner) {
}
ArrayData* EmptyMonotypeDict::EscalateToVanilla(
    const Self* ad, const char* reason) {
  auto const legacy = ad->isLegacyArray();
  return ad->isDictType()
    ? (legacy ? staticEmptyMarkedDictArray() : staticEmptyDictArray())
    : (legacy ? staticEmptyMarkedDArray() : staticEmptyDArray());
}
void EmptyMonotypeDict::ConvertToUncounted(
    Self* ad, DataWalker::PointerMap* seen) {
}
void EmptyMonotypeDict::ReleaseUncounted(Self* ad) {
}
void EmptyMonotypeDict::Release(Self* ad) {
  tl_heap->objFreeIndex(ad, kEmptySizeIndex);
}

//////////////////////////////////////////////////////////////////////////////
// Accessors

bool EmptyMonotypeDict::IsVectorData(const Self* ad) {
  return true;
}
TypedValue EmptyMonotypeDict::GetInt(const Self* ad, int64_t k) {
  return make_tv<KindOfUninit>();
}
TypedValue EmptyMonotypeDict::GetStr(const Self* ad, const StringData* k) {
  return make_tv<KindOfUninit>();
}
TypedValue EmptyMonotypeDict::GetKey(const Self* ad, ssize_t pos) {
  always_assert(false);
}
TypedValue EmptyMonotypeDict::GetVal(const Self* ad, ssize_t pos) {
  always_assert(false);
}
ssize_t EmptyMonotypeDict::GetIntPos(const Self* ad, int64_t k) {
  return 0;
}
ssize_t EmptyMonotypeDict::GetStrPos(const Self* ad, const StringData* k) {
  return 0;
}

ssize_t EmptyMonotypeDict::IterBegin(const Self* ad) {
  return 0;
}
ssize_t EmptyMonotypeDict::IterLast(const Self* ad) {
  return 0;
}
ssize_t EmptyMonotypeDict::IterEnd(const Self* ad) {
  return 0;
}
ssize_t EmptyMonotypeDict::IterAdvance(const Self* ad, ssize_t prev) {
  return 0;
}
ssize_t EmptyMonotypeDict::IterRewind(const Self* ad, ssize_t prev) {
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Mutations

namespace {
template <typename Key>
ArrayData* makeStrMonotypeDict(HeaderKind kind, StringData* k, TypedValue v) {
  auto const dt = dt_modulo_persistence(type(v));
  auto const mad = MonotypeDict<Key>::MakeReserve(kind, 1, dt);
  auto const result = MonotypeDict<Key>::SetStr(mad, k, v);
  assertx(result == mad);
  return result;
}
}

arr_lval EmptyMonotypeDict::LvalInt(Self* ad, int64_t k) {
  throwOOBArrayKeyException(k, ad);
}
arr_lval EmptyMonotypeDict::LvalStr(Self* ad, StringData* k) {
  throwOOBArrayKeyException(k, ad);
}
arr_lval EmptyMonotypeDict::ElemInt(Self* ad, int64_t k) {
  throwOOBArrayKeyException(k, ad);
}
arr_lval EmptyMonotypeDict::ElemStr(Self* ad, StringData* k) {
  throwOOBArrayKeyException(k, ad);
}

ArrayData* EmptyMonotypeDict::SetInt(Self* ad, int64_t k, TypedValue v) {
  auto const dt = dt_modulo_persistence(type(v));
  auto const mad = MonotypeDict<int64_t>::MakeReserve(ad->m_kind, 1, dt);
  auto const result = MonotypeDict<int64_t>::SetInt(mad, k, v);
  assertx(result == mad);
  return result;
}
ArrayData* EmptyMonotypeDict::SetStr(Self* ad, StringData* k, TypedValue v) {
  return k->isStatic() ? makeStrMonotypeDict<LowStringPtr>(ad->m_kind, k, v)
                       : makeStrMonotypeDict<StringData*>(ad->m_kind, k, v);
}
ArrayData* EmptyMonotypeDict::RemoveInt(Self* ad, int64_t k) {
  return ad;
}
ArrayData* EmptyMonotypeDict::RemoveStr(Self* ad, const StringData* k) {
  return ad;
}

ArrayData* EmptyMonotypeDict::Append(Self* ad, TypedValue v) {
  return SetInt(ad, 0, v);
}
ArrayData* EmptyMonotypeDict::Pop(Self* ad, Variant& ret) {
  ret = uninit_null();
  return ad;
}

//////////////////////////////////////////////////////////////////////////////

ArrayData* EmptyMonotypeDict::ToDVArray(Self* ad, bool copy) {
  return GetDArray(false);
}
ArrayData* EmptyMonotypeDict::ToHackArr(Self* ad, bool copy) {
  return GetDict(false);
}
ArrayData* EmptyMonotypeDict::SetLegacyArray(Self* ad, bool copy, bool legacy) {
  return ad->isDArray() ? GetDArray(legacy) : GetDict(legacy);
}

//////////////////////////////////////////////////////////////////////////////

namespace {

constexpr uint8_t kArrSize = 16;
constexpr uint8_t kElmSize = 16;
constexpr uint8_t kIndexSize = 2;

constexpr size_t kMinNumElms = 6;
constexpr size_t kMinNumIndices = 8;
constexpr uint8_t kMinSizeIndex = 7;

constexpr size_t kMaxNumElms = 0xc000;
constexpr size_t kMinSize = kArrSize +
                            kElmSize * kMinNumElms +
                            kIndexSize * kMinNumIndices;

constexpr bool isValidSizeIndex(uint8_t index) {
  assertx(index >= kMinSizeIndex);
  assertx((index - kMinSizeIndex) % kSizeClassesPerDoubling == 0);
  return true;
}
constexpr size_t scaleBySizeIndex(size_t base, uint8_t index) {
  assertx(isValidSizeIndex(index));
  return base << ((index - kMinSizeIndex) / kSizeClassesPerDoubling);
}

static_assert(kMaxNumElms % kMinNumElms == 0);
static_assert(scaleBySizeIndex(1, kMinSizeIndex) == 1);
static_assert(kSizeIndex2Size[kMinSizeIndex] == kMinSize);

template <typename Key>
constexpr LayoutIndex getLayoutIndex(DataType type) {
  if constexpr (std::is_same<Key, int64_t>::value) {
    return getIntLayoutIndex(type);
  } else if constexpr (std::is_same<Key, StringData*>::value) {
    return getStrLayoutIndex(type);
  } else {
    static_assert(std::is_same<Key, LowStringPtr>::value);
    return getStaticStrLayoutIndex(type);
  }
}

template <typename Key>
constexpr strhash_t getHash(Key key) {
  if constexpr (std::is_same<Key, int64_t>::value) {
    return hash_int64(key) | STRHASH_MSB;
  } else if constexpr (std::is_same<Key, StringData*>::value) {
    return key->hash();
  } else {
    static_assert(std::is_same<Key, LowStringPtr>::value);
    return key->hashStatic();
  }
}

template <typename Key>
constexpr Key getTombstone() {
  if constexpr (std::is_same<Key, int64_t>::value) {
    return std::numeric_limits<Key>::min();
  } else {
    static_assert(std::is_same<Key, StringData*>::value ||
                  std::is_same<Key, LowStringPtr>::value);
    return nullptr;
  }
}

template <typename Key>
void incRefKey(Key key) {
  if constexpr (std::is_same<Key, StringData*>::value) {
    key->incRefCount();
  }
}

template <typename Key>
void decRefKey(Key key) {
  if constexpr (std::is_same<Key, StringData*>::value) {
    decRefStr(key);
  }
}

template <typename Key>
Key coerceKey(int64_t input) {
  if constexpr (std::is_same<Key, int64_t>::value) {
    return input;
  }
  return getTombstone<Key>();
}

template <typename Key>
Key coerceKey(const StringData* input) {
  if constexpr (std::is_same<Key, StringData*>::value) {
    return const_cast<StringData*>(input);
  } else if constexpr (std::is_same<Key, LowStringPtr>::value) {
    return input->isStatic() ? input : lookupStaticString(input);
  }
  return getTombstone<Key>();
}

template <typename Key>
bool keysEqual(Key a, Key b) {
  if constexpr (std::is_same<Key, StringData*>::value) {
    return a == b || a->same(b);
  } else {
    static_assert(std::is_same<Key, int64_t>::value ||
                  std::is_same<Key, LowStringPtr>::value);
    return a == b;
  }
}

}

//////////////////////////////////////////////////////////////////////////////

template <typename Key>
uint8_t MonotypeDict<Key>::ComputeSizeIndex(size_t size) {
  assertx(0 < size);
  assertx(size <= kMaxNumElms);
  auto capacity = kMinNumElms;
  auto result = kMinSizeIndex;
  while (capacity < size) {
    capacity *= 2;
    result += kSizeClassesPerDoubling;
  }
  return result;
}

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::MakeReserve(
    HeaderKind kind, size_t size, DataType dt) {
  auto const index = ComputeSizeIndex(size);
  auto const mem = tl_heap->objMallocIndex(index);
  auto const ad = reinterpret_cast<MonotypeDict<Key>*>(mem);

  auto const aux = packSizeIndexAndAuxBits(index, 0);
  ad->initHeader_16(kind, OneReference, aux);
  ad->setLayoutIndex(getLayoutIndex<Key>(dt));
  ad->m_extra_lo16 = 0;
  ad->m_size = 0;
  ad->initHash();

  assertx(ad->checkInvariants());
  return ad;
}

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::MakeFromVanilla(
    ArrayData* ad, DataType dt) {
  assertx(ad->size() <= kMaxNumElms);
  assertx(ad->hasVanillaMixedLayout());
  auto const kind = ad->isDArray() ? HeaderKind::BespokeDArray
                                   : HeaderKind::BespokeDict;
  auto result = MakeReserve(kind, ad->size(), dt);
  result->setLegacyArrayInPlace(ad->isLegacyArray());

  IterateKVNoInc(ad, [&](auto k, auto v) {
    auto const next = tvIsString(k) ? SetStr(result, val(k).pstr, v)
                                    : SetInt(result, val(k).num, v);
    assertx(result == next);
    result = As(next);
  });

  if (ad->isStatic()) {
    auto const size = HeapSize(result);
    auto const copy = static_cast<Self*>(
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

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::As(ArrayData* ad) {
  auto const result = reinterpret_cast<MonotypeDict<Key>*>(ad);
  assertx(result->checkInvariants());
  return result;
}
template <typename Key>
const MonotypeDict<Key>* MonotypeDict<Key>::As(const ArrayData* a) {
  return As(const_cast<ArrayData*>(a));
}

template <typename Key>
bool MonotypeDict<Key>::checkInvariants() const {
  static_assert(kArrSize == sizeof(*this));
  static_assert(kElmSize == sizeof(Elm));
  static_assert(kIndexSize == sizeof(Index));
  assertx(isRealType(type()));
  assertx(type() == dt_modulo_persistence(type()));
  assertx(isDArray() || isDictType());
  assertx(isValidSizeIndex(sizeIndex()));
  assertx(layoutIndex() == getLayoutIndex<Key>(type()));
  assertx(size() <= used());
  assertx(IMPLIES(!isZombie(), used() <= numElms()));
  return true;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key> template <typename Result>
Result MonotypeDict<Key>::find(Key key, strhash_t hash) {
  auto constexpr IsAdd = std::is_same<Result, Add>::value;
  auto constexpr IsGet = std::is_same<Result, Get>::value;
  auto constexpr IsRemove = std::is_same<Result, Remove>::value;
  auto constexpr IsUpdate = std::is_same<Result, Update>::value;
  static_assert(IsAdd || IsGet || IsRemove || IsUpdate);
  assertx(IsAdd || key != getTombstone<Key>());

  auto const data = indices();
  auto const mask = numIndices() - 1;
  auto i = uint32_t(hash) & mask;
  auto delta = 1;
  auto first = 0;

  for (auto index = data[i]; index != kEmptyIndex;
       i = (i + delta) & mask, delta++, index = data[i]) {
    if constexpr (IsAdd) continue;

    if (index == kTombstoneIndex) {
      if constexpr (IsUpdate) {
        if (!first) first = i + 1;
      }
      continue;
    }

    auto const elm = elmAtIndex(index);
    if (keysEqual(elm->key, key)) {
      if constexpr (IsGet) return {elm};
      if constexpr (IsRemove) return {safe_cast<ssize_t>(i)};
      if constexpr (IsUpdate) return {elm, &data[i]};
    }
  }

  if constexpr (IsAdd) return {&data[i]};
  if constexpr (IsGet) return {nullptr};
  if constexpr (IsRemove) return {-1};
  if constexpr (IsUpdate) return {nullptr, &data[first ? first - 1 : i]};
}

template <typename Key>
typename MonotypeDict<Key>::Index*
MonotypeDict<Key>::findForAdd(strhash_t hash) {
  return find<Add>(getTombstone<Key>(), hash).index;
}

template <typename Key>
const typename MonotypeDict<Key>::Elm*
MonotypeDict<Key>::findForGet(Key key, strhash_t hash) const {
  auto const mad = const_cast<MonotypeDict<Key>*>(this);
  return mad->template find<Get>(key, hash).elm;
}

template <typename Key>
TypedValue MonotypeDict<Key>::getImpl(Key key) const {
  if (key == getTombstone<Key>()) return make_tv<KindOfUninit>();
  auto const result = findForGet(key, getHash(key));
  return result ? TypedValue { result->val, type() } : make_tv<KindOfUninit>();
}

template <typename Key>
ssize_t MonotypeDict<Key>::getPosImpl(Key key) const {
  if (key == getTombstone<Key>()) return used();
  auto const result = findForGet(key, getHash(key));
  return result ? ptrdiff_t(result - elms()) / sizeof(Elm) : used();
}

template <typename Key>
ArrayData* MonotypeDict<Key>::removeImpl(Key key) {
  if (key == getTombstone<Key>()) return this;
  auto const hash_pos = find<Remove>(key, getHash(key)).hash_pos;
  if (hash_pos < 0) return this;

  auto const mad = cowCheck() ? copy() : this;
  auto& index = mad->indices()[hash_pos];
  auto const elm = mad->elmAtIndex(mad->indices()[hash_pos]);
  assertx(keysEqual(elm->key, key));

  decRefKey(elm->key);
  tvDecRefGen(TypedValue { elm->val, type() });
  elm->key = getTombstone<Key>();
  index = kTombstoneIndex;
  mad->m_size--;
  return mad;
}

template <typename Key> template <typename K>
arr_lval MonotypeDict<Key>::elemImpl(Key key, K k) {
  if (key == getTombstone<Key>()) throwOOBArrayKeyException(k, this);
  auto const old = findForGet(key, getHash(key));
  if (old == nullptr) throwOOBArrayKeyException(k, this);

  auto const mad = cowCheck() ? copy() : this;
  auto const elm = old - elms() + mad->elms();
  assertx(keysEqual(elm->key, key));

  static_assert(folly::kIsLittleEndian);
  auto const type_ptr = reinterpret_cast<DataType*>(&mad->m_extra_hi16);
  assertx(*type_ptr == mad->type());
  return arr_lval{mad, type_ptr, const_cast<Value*>(&elm->val)};
}

template <typename Key> template <typename K>
ArrayData* MonotypeDict<Key>::setImpl(Key key, K k, TypedValue v) {
  if (key == getTombstone<Key>() || used() == kMaxNumElms ||
      dt_modulo_persistence(v.type()) != type()) {
    auto const ad = escalateWithCapacity(size() + 1);
    auto const result = ad->set(k, v);
    assertx(ad == result);
    return result;
  }
  tvIncRefGen(v);
  auto const result = prepareForInsert();
  auto const update = result->template find<Update>(key, getHash(key));
  if (update.elm != nullptr) {
    tvDecRefGen(TypedValue { update.elm->val, type() });
    update.elm->val = v.val();
  } else {
    incRefKey(key);
    *update.index = safe_cast<Index>(result->used());
    *result->elmAtIndex(*update.index) = { key, v.val() };
    result->m_extra_lo16++;
    result->m_size++;
  }
  return result;
}

template <typename Key>
template <typename KeyFn, typename CountedFn,
          typename MaybeCountedFn, typename UncountedFn>
void MonotypeDict<Key>::forEachElm(
    KeyFn k, CountedFn c, MaybeCountedFn m, UncountedFn u) const {
  auto const dt = type();
  auto const limit = used();
  if (m_size == limit) {
    if (static_cast<data_type_t>(dt) & kHasPersistentMask) {
      assertx(isRefcountedType(dt));
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        k(i, elm->key);
        m(TypedValue { elm->val, dt });
      }
    } else if (isRefcountedType(dt)) {
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        k(i, elm->key);
        c(TypedValue { elm->val, dt });
      }
    } else {
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        k(i, elm->key);
        u(TypedValue { elm->val, dt });
      }
    }
  } else {
    if (static_cast<data_type_t>(dt) & kHasPersistentMask) {
      assertx(isRefcountedType(dt));
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        if (elm->key == getTombstone<Key>()) continue;
        k(i, elm->key);
        m(TypedValue { elm->val, dt });
      }
    } else if (isRefcountedType(dt)) {
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        if (elm->key == getTombstone<Key>()) continue;
        k(i, elm->key);
        c(TypedValue { elm->val, dt });
      }
    } else {
      for (auto i = 0; i < limit; i++) {
        auto const elm = elmAtIndex(i);
        if (elm->key == getTombstone<Key>()) continue;
        k(i, elm->key);
        u(TypedValue { elm->val, dt });
      }
    }
  }
}

template <typename Key> template <typename ElmFn>
void MonotypeDict<Key>::forEachElm(ElmFn e) const {
  auto const limit = used();
  if (m_size == limit) {
      for (auto i = 0; i < limit; i++) {
        e(i, elmAtIndex(i));
      }
  } else {
    for (auto i = 0; i < limit; i++) {
      auto const elm = elmAtIndex(i);
      if (elm->key != getTombstone<Key>()) e(i, elm);
    }
  }
}

template <typename Key>
void MonotypeDict<Key>::incRefElms() {
  forEachElm(
    [&](auto i, auto k) { incRefKey(k); },
    [&](auto v) { reinterpret_cast<Countable*>(val(v).pcnt)->incRefCount(); },
    [&](auto v) { val(v).pcnt->incRefCount(); },
    [&](auto v) {}
  );
}

template <typename Key>
void MonotypeDict<Key>::decRefElms() {
  auto const dt = type();
  forEachElm(
    [&](auto i, auto k) { decRefKey(k); },
    [&](auto v) {
      auto const countable = reinterpret_cast<Countable*>(val(v).pcnt);
      if (countable->decReleaseCheck()) destructorForType(dt)(countable);
    },
    [&](auto v) {
      auto const countable = val(v).pcnt;
      if (countable->decReleaseCheck()) destructorForType(dt)(countable);
    },
    [&](auto v) {}
  );
}

template <typename Key>
void MonotypeDict<Key>::copyHash(const Self* other) {
  static_assert(kMinNumIndices * sizeof(Index) % 16 == 0);
  assertx(uintptr_t(indices()) % 16 == 0);

  memcpy16_inline(indices(), other->indices(), numIndices() * sizeof(Index));
}

template <typename Key>
void MonotypeDict<Key>::initHash() {
  static_assert(kEmptyIndex == -1);
  static_assert(kMinNumIndices * sizeof(Index) % 16 == 0);
  assertx(uintptr_t(indices()) % 16 == 0);

  auto const data = indices();
  auto cur = reinterpret_cast<int64_t*>(data);
  auto end = reinterpret_cast<int64_t*>(data + numIndices());
  for (; cur < end; cur++) *cur = -1;
}

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::copy() {
  auto const mem = tl_heap->objMallocIndex(sizeIndex());
  auto const ad = reinterpret_cast<MonotypeDict<Key>*>(mem);

  // Adding 24 bytes so that we can copy elements in 32-byte groups.
  // We might overwrite ad's indices here, but they're not initialized yet.
  auto const bytes = sizeof(*this) + sizeof(Elm) * used() + 24;
  bcopy32_inline(ad, this, bytes);

  auto const aux = packSizeIndexAndAuxBits(sizeIndex(), auxBits());
  ad->initHeader_16(m_kind, OneReference, aux);
  ad->copyHash(this);
  ad->incRefElms();

  assertx(ad->checkInvariants());
  return ad;
}

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::prepareForInsert() {
  auto const copy = cowCheck();
  if (used() == numElms()) {
    return resize(sizeIndex() + kSizeClassesPerDoubling, copy);
  }
  return copy ? this->copy() : this;
}

template <typename Key>
MonotypeDict<Key>* MonotypeDict<Key>::resize(uint8_t index, bool copy) {
  auto const mem = tl_heap->objMallocIndex(index);
  auto const ad = reinterpret_cast<MonotypeDict<Key>*>(mem);

  // Adding 24 bytes so that we can copy elements in 32-byte groups.
  // We might overwrite ad's indices here, but they're not initialized yet.
  auto const bytes = sizeof(*this) + sizeof(Elm) * used() + 24;
  bcopy32_inline(ad, this, bytes);

  auto const aux = packSizeIndexAndAuxBits(index, auxBits());
  ad->initHeader_16(m_kind, OneReference, aux);
  ad->initHash();

  ad->forEachElm([&](auto i, auto elm) {
    *ad->findForAdd(getHash(elm->key)) = i;
  });

  if (copy) {
    ad->incRefElms();
  } else {
    setZombie();
  }

  assertx(ad->checkInvariants());
  return ad;
}

template <typename Key>
ArrayData* MonotypeDict<Key>::escalateWithCapacity(size_t capacity) const {
  assertx(capacity >= size());
  auto ad = isDictType() ? MixedArray::MakeReserveDict(capacity)
                         : MixedArray::MakeReserveDArray(capacity);
  ad->setLegacyArrayInPlace(isLegacyArray());

  auto const dt = type();
  forEachElm([&](auto i, auto elm) {
    auto const tv = TypedValue { elm->val, dt };
    tvIncRefGen(tv);
    auto const result = [&]{
      if constexpr (std::is_same<Key, int64_t>::value) {
        return MixedArray::SetInt(ad, elm->key, tv);
      } else if constexpr (std::is_same<Key, StringData*>::value) {
        return MixedArray::SetStr(ad, elm->key, tv);
      } else {
        static_assert(std::is_same<Key, LowStringPtr>::value);
        auto const key = const_cast<StringData*>(elm->key.get());
        return MixedArray::SetStr(ad, key, tv);
      }
    }();
    assertx(ad == result);
    ad = result;
  });

  return ad;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key>
typename MonotypeDict<Key>::Elm* MonotypeDict<Key>::elms() {
  return reinterpret_cast<Elm*>(reinterpret_cast<char*>(this + 1));
}

template <typename Key>
const typename MonotypeDict<Key>::Elm* MonotypeDict<Key>::elms() const {
  return const_cast<MonotypeDict<Key>*>(this)->elms();
}

template <typename Key>
typename MonotypeDict<Key>::Elm* MonotypeDict<Key>::elmAtIndex(Index index) {
  assertx(0 <= index);
  assertx(index < numElms());
  return &elms()[index];
}

template <typename Key>
const typename MonotypeDict<Key>::Elm*
MonotypeDict<Key>::elmAtIndex(Index index) const {
  return const_cast<MonotypeDict<Key>*>(this)->elmAtIndex(index);
}

template <typename Key>
typename MonotypeDict<Key>::Index* MonotypeDict<Key>::indices() {
  auto const elms = sizeof(Elm) * numElms();
  return reinterpret_cast<Index*>(reinterpret_cast<char*>(this + 1) + elms);
}

template <typename Key>
const typename MonotypeDict<Key>::Index* MonotypeDict<Key>::indices() const {
  return const_cast<MonotypeDict<Key>*>(this)->indices();
}

template <typename Key>
DataType MonotypeDict<Key>::type() const {
  return DataType(int8_t(m_extra_hi16 & 0xff));
}

template <typename Key>
uint32_t MonotypeDict<Key>::used() const {
  return m_extra_lo16;
}

template <typename Key>
uint8_t MonotypeDict<Key>::sizeIndex() const {
  return m_aux16 >> 8;
}

template <typename Key>
size_t MonotypeDict<Key>::numElms() const {
  return scaleBySizeIndex(kMinNumElms, sizeIndex());
}

template <typename Key>
size_t MonotypeDict<Key>::numIndices() const {
  return scaleBySizeIndex(kMinNumIndices, sizeIndex());
}

template <typename Key>
void MonotypeDict<Key>::setZombie() {
  m_extra_lo16 = uint16_t(-1);
}

template <typename Key>
bool MonotypeDict<Key>::isZombie() const {
  return m_extra_lo16 == uint16_t(-1);
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key>
size_t MonotypeDict<Key>::HeapSize(const Self* mad) {
  return scaleBySizeIndex(kMinSize, mad->sizeIndex());
}

template <typename Key>
void MonotypeDict<Key>::Scan(const Self* mad, type_scan::Scanner& scanner) {
  auto const dt = mad->type();
  mad->forEachElm([&](auto i, auto elm) {
    if constexpr (std::is_same<Key, StringData*>::value) scanner.scan(elm->key);
    if (isRefcountedType(dt)) scanner.scan(elm->val.pcnt);
  });
}

template <typename Key>
ArrayData* MonotypeDict<Key>::EscalateToVanilla(
    const Self* mad, const char* reason) {
  return mad->escalateWithCapacity(mad->size());
}

template <typename Key>
void MonotypeDict<Key>::ConvertToUncounted(
    Self* mad, DataWalker::PointerMap* seen) {
  auto const dt = mad->type();

  mad->forEachElm([&](auto i, auto elm) {
    auto const elm_mut = const_cast<Elm*>(elm);
    if constexpr (std::is_same<Key, StringData*>::value) {
      auto dt_mut = KindOfString;
      ConvertTvToUncounted(tv_lval(&dt_mut, &elm_mut->val), seen);
    }
    auto dt_mut = dt;
    ConvertTvToUncounted(tv_lval(&dt_mut, &elm_mut->val), seen);
    assertx(equivDataTypes(dt_mut, dt));
  });
}

template <typename Key>
void MonotypeDict<Key>::ReleaseUncounted(Self* mad) {
  auto const dt = mad->type();

  mad->forEachElm([&](auto i, auto elm) {
    if constexpr (std::is_same<Key, StringData*>::value) {
      if (elm->key->isUncounted()) StringData::ReleaseUncounted(elm->key);
    }
    auto tv = TypedValue { elm->val, dt };
    ReleaseUncountedTv(&tv);
  });
}

template <typename Key>
void MonotypeDict<Key>::Release(Self* mad) {
  mad->fixCountForRelease();
  assertx(mad->isRefCounted());
  assertx(mad->hasExactlyOneRef());
  if (!mad->isZombie()) mad->decRefElms();
  tl_heap->objFreeIndex(mad, mad->sizeIndex());
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key>
bool MonotypeDict<Key>::IsVectorData(const Self* mad) {
  if (mad->empty()) return true;
  if constexpr (std::is_same<Key, int64_t>::value) {
    auto next = 0;
    auto const limit = mad->used();
    for (auto i = 0; i < limit; i++) {
      auto const elm = mad->elmAtIndex(i);
      if (elm->key == getTombstone<Key>()) continue;
      if (elm->key != next) return false;
      next++;
    }
    return true;
  }
  return false;
}

template <typename Key>
TypedValue MonotypeDict<Key>::GetInt(const Self* mad, int64_t k) {
  return mad->getImpl(coerceKey<Key>(k));
}

template <typename Key>
TypedValue MonotypeDict<Key>::GetStr(const Self* mad, const StringData* k) {
  return mad->getImpl(coerceKey<Key>(k));
}

template <typename Key>
TypedValue MonotypeDict<Key>::GetKey(const Self* mad, ssize_t pos) {
  auto const elm = mad->elmAtIndex(pos);
  assertx(elm->key != getTombstone<Key>());
  if constexpr (std::is_same<Key, int64_t>::value) {
    return make_tv<KindOfInt64>(elm->key);
  } else if constexpr (std::is_same<Key, StringData*>::value) {
    return make_tv<KindOfString>(elm->key);
  } else {
    static_assert(std::is_same<Key, LowStringPtr>::value);
    auto const key = const_cast<StringData*>(elm->key.get());
    return make_tv<KindOfPersistentString>(key);
  }
}

template <typename Key>
TypedValue MonotypeDict<Key>::GetVal(const Self* mad, ssize_t pos) {
  auto const elm = mad->elmAtIndex(pos);
  assertx(elm->key != getTombstone<Key>());
  return { elm->val, mad->type() };
}

template <typename Key>
ssize_t MonotypeDict<Key>::GetIntPos(const Self* mad, int64_t k) {
  return mad->getPosImpl(coerceKey<Key>(k));
}

template <typename Key>
ssize_t MonotypeDict<Key>::GetStrPos(const Self* mad, const StringData* k) {
  return mad->getPosImpl(coerceKey<Key>(k));
}

template <typename Key>
ssize_t MonotypeDict<Key>::IterBegin(const Self* mad) {
  return IterAdvance(mad, -1);
}
template <typename Key>
ssize_t MonotypeDict<Key>::IterLast(const Self* mad) {
  return IterRewind(mad, mad->used());
}
template <typename Key>
ssize_t MonotypeDict<Key>::IterEnd(const Self* mad) {
  return mad->used();
}
template <typename Key>
ssize_t MonotypeDict<Key>::IterAdvance(const Self* mad, ssize_t pos) {
  auto const limit = mad->used();
  for (pos++; pos < limit; pos++) {
    if (mad->elmAtIndex(pos)->key != getTombstone<Key>()) return pos;
  }
  return limit;
}
template <typename Key>
ssize_t MonotypeDict<Key>::IterRewind(const Self* mad, ssize_t pos) {
  for (pos--; pos >= 0; pos--) {
    if (mad->elmAtIndex(pos)->key != getTombstone<Key>()) return pos;
  }
  return mad->used();
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key>
arr_lval MonotypeDict<Key>::LvalInt(Self* mad, int64_t k) {
  auto const vad = EscalateToVanilla(mad, __func__);
  auto const result = vad->lval(k);
  assertx(result.arr == vad);
  return result;
}

template <typename Key>
arr_lval MonotypeDict<Key>::LvalStr(Self* mad, StringData* k) {
  auto const vad = EscalateToVanilla(mad, __func__);
  auto const result = vad->lval(k);
  assertx(result.arr == vad);
  return result;
}

template <typename Key>
arr_lval MonotypeDict<Key>::ElemInt(Self* mad, int64_t k) {
  return mad->elemImpl(coerceKey<Key>(k), k);
}

template <typename Key>
arr_lval MonotypeDict<Key>::ElemStr(Self* mad, StringData* k) {
  return mad->elemImpl(coerceKey<Key>(k), k);
}

template <typename Key>
ArrayData* MonotypeDict<Key>::SetInt(Self* mad, int64_t k, TypedValue v) {
  return mad->setImpl(coerceKey<Key>(k), k, v);
}

template <typename Key>
ArrayData* MonotypeDict<Key>::SetStr(Self* mad, StringData* k, TypedValue v) {
  return mad->setImpl(coerceKey<Key>(k), k, v);
}

template <typename Key>
ArrayData* MonotypeDict<Key>::RemoveInt(Self* mad, int64_t k) {
  return mad->removeImpl(coerceKey<Key>(k));
}

template <typename Key>
ArrayData* MonotypeDict<Key>::RemoveStr(Self* mad, const StringData* k) {
  return mad->removeImpl(coerceKey<Key>(k));
}

template <typename Key>
ArrayData* MonotypeDict<Key>::Append(Self* mad, TypedValue v) {
  auto nextKI = int64_t{0};
  if constexpr (std::is_same<Key, int64_t>::value) {
    mad->forEachElm([&](auto i, auto elm) {
      if (elm->key >= nextKI && nextKI >= 0) {
        nextKI = static_cast<uint64_t>(elm->key) + 1;
      }
    });
  }
  return nextKI < 0 ? mad : SetInt(mad, nextKI, v);
}

template <typename Key>
ArrayData* MonotypeDict<Key>::Pop(Self* mad, Variant& value) {
  if (mad->empty()) {
    value = uninit_null();
    return mad;
  }
  auto const pos = IterLast(mad);
  auto const key = GetKey(mad, pos);
  value = tvAsCVarRef(GetVal(mad, pos));
  return tvIsString(key) ? RemoveStr(mad, val(key).pstr)
                         : RemoveInt(mad, val(key).num);
}

template <typename Key>
ArrayData* MonotypeDict<Key>::ToDVArray(Self* madIn, bool copy) {
  if (madIn->isDArray()) return madIn;
  auto const mad = copy ? madIn->copy() : madIn;
  mad->m_kind = HeaderKind::BespokeDArray;
  assertx(mad->checkInvariants());
  return mad;
}

template <typename Key>
ArrayData* MonotypeDict<Key>::ToHackArr(Self* madIn, bool copy) {
  if (madIn->isDictType()) return madIn;
  auto const mad = copy ? madIn->copy() : madIn;
  mad->m_kind = HeaderKind::BespokeDict;
  mad->setLegacyArrayInPlace(false);
  assertx(mad->checkInvariants());
  return mad;
}

template <typename Key>
ArrayData* MonotypeDict<Key>::SetLegacyArray(
    Self* madIn, bool copy, bool legacy) {
  auto const mad = copy ? madIn->copy() : madIn;
  mad->setLegacyArrayInPlace(legacy);
  return mad;
}

//////////////////////////////////////////////////////////////////////////////

void EmptyMonotypeDict::InitializeLayouts() {
  auto const base = Layout::ReserveIndices(1 << 10);
  always_assert(base == kBaseLayoutIndex);

  static auto const empty_vtable = fromArray<EmptyMonotypeDict>();
  new Layout(getEmptyLayoutIndex(), "MonotypeDict<Empty,Empty>", &empty_vtable);

  static auto const int_vtable = fromArray<MonotypeDict<int64_t>>();
  static auto const str_vtable = fromArray<MonotypeDict<StringData*>>();
  static auto const s32_vtable = fromArray<MonotypeDict<LowStringPtr>>();

#define DT(name, value) \
  if (dt_modulo_persistence(KindOf##name) == KindOf##name) { \
    auto ints = getIntLayoutIndex(KindOf##name); \
    auto strs = getStrLayoutIndex(KindOf##name); \
    auto s32s = getStaticStrLayoutIndex(KindOf##name); \
    new Layout(ints, "MonotypeDict<Int,"#name">", &int_vtable); \
    new Layout(strs, "MonotypeDict(Str,"#name">", &str_vtable); \
    new Layout(s32s, "MonotypeDict<StaticStr,"#name">", &s32_vtable); \
  }
DATATYPES
#undef DT

  auto const init = [&](EmptyMonotypeDict* ad, HeaderKind kind, bool legacy) {
    ad->initHeader_16(kind, StaticValue, legacy ? kLegacyArray : 0);
    ad->setLayoutIndex(getEmptyLayoutIndex());
    ad->m_size = 0;
  };
  init(GetDict(false), HeaderKind::BespokeDict, false);
  init(GetDArray(false), HeaderKind::BespokeDArray, false);
  init(GetDict(true), HeaderKind::BespokeDict, true);
  init(GetDArray(true), HeaderKind::BespokeDArray, true);
}

ArrayData* MakeMonotypeDictFromVanilla(
    ArrayData* ad, DataType dt, KeyTypes kt) {
  if (ad->size() > kMaxNumElms) {
    ad->incRefCount();
    return ad;
  }
  switch (kt) {
    case KeyTypes::Ints:
      return MonotypeDict<int64_t>::MakeFromVanilla(ad, dt);
    case KeyTypes::Strings:
      return MonotypeDict<StringData*>::MakeFromVanilla(ad, dt);
    case KeyTypes::StaticStrings:
      return MonotypeDict<LowStringPtr>::MakeFromVanilla(ad, dt);
    default: always_assert(false);
  }
}

//////////////////////////////////////////////////////////////////////////////

}}
