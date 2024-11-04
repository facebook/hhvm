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
#include "hphp/runtime/vm/iter.h"

#include <algorithm>

#include <folly/Likely.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/vanilla-dict.h"
#include "hphp/runtime/base/vanilla-keyset.h"
#include "hphp/runtime/base/vanilla-vec.h"

#include "hphp/runtime/base/vanilla-dict-defs.h"
#include "hphp/runtime/base/vanilla-vec-defs.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/base/bespoke/struct-dict.h"

#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/collections/hash-collection.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

// We don't want iterators to take up too much space on the stack.
static_assert(sizeof(Iter) == 16, "");

using bespoke::StructDict;

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

const std::string& describe(const BespokeArray* bad) {
  return bespoke::Layout::FromIndex(bad->layoutIndex())->describe();
}

bool isStructDict(const BespokeArray* bad) {
  return bespoke::StructLayout::IsStructLayout(bad->layoutIndex());
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void Iter::kill() {
  if (!debug) return;
  // Iter is not POD, so we memset each POD field separately.
  memset(&m_pos, kIterTrashFill, sizeof(m_pos));
  memset(&m_end, kIterTrashFill, sizeof(m_end));
}

//////////////////////////////////////////////////////////////////////

TypedValue Iter::extractBase(TypedValue base, const Class* ctx) {
  assertx(!isArrayLikeType(type(base)));
  if (!isObjectType(type(base))) {
    SystemLib::throwInvalidForeachArgumentExceptionObject();
  }

  auto const obj = val(base).pobj;
  if (LIKELY(obj->isCollection())) {
    switch (obj->collectionType()) {
      case CollectionType::ImmVector:
      case CollectionType::Vector: {
        auto const vec = static_cast<BaseVector*>(obj)->arrayData();
        vec->incRefCount();
        return make_tv<KindOfVec>(vec);
      }
      case CollectionType::ImmMap:
      case CollectionType::Map:
      case CollectionType::ImmSet:
      case CollectionType::Set: {
        auto const dict =
          static_cast<HashCollection*>(obj)->arrayData()->asArrayData();
        dict->incRefCount();
        return make_tv<KindOfDict>(dict);
      }
      case CollectionType::Pair: {
        auto const pair = static_cast<c_Pair*>(obj);
        auto vec = make_vec_array(
          tvAsCVarRef(pair->at(0)),
          tvAsCVarRef(pair->at(1))
        );
        return make_tv<KindOfVec>(vec.detach());
      }
    }
    not_reached();
  }

  bool isIterator;
  auto iterObj = obj->iterableObject(isIterator);
  if (isIterator) {
    return make_tv<KindOfObject>(iterObj.detach());
  }

  Array iterArray(obj->o_toIterArray(ctx));
  return make_tv<KindOfDict>(iterArray.detach());
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers

namespace {

// "virtual" method implementation of *IterNext* for VanillaVec iterators.
// Since we know the array is packed, we just need to increment the position
// and do a bounds check.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_vec(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaVec());

  ssize_t pos = iter->getPos() + 1;
  if (UNLIKELY(pos == iter->getEnd())) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaVec iterators over
// unaligned typed values. Since these values are stored one after the other,
// we can just increment the element pointer and compare it to the end pointer.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_vec_pointer(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaVec());

  auto const elm = iter->m_unaligned_elm + 1;
  if (elm == iter->m_unaligned_end) {
    iter->kill();
    return 0;
  }

  iter->m_unaligned_elm = elm;
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaDict iterators.
// Since we know the array type, we can do "while (elm[pos].isTombstone())"
// inline here.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_dict(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaDict());
  ssize_t pos    = iter->getPos();
  auto const arr = VanillaDict::as(ad);

  do {
    if ((++pos) == iter->getEnd()) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));

  iter->setPos(pos);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaDictPointer.
// Since we know the array type, we can do "while (elm->isTombstone())"
// inline here.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_dict_pointer(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaDict());
  auto elm = iter->m_dict_elm;

  do {
    if ((++elm) == iter->m_dict_end) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(elm->isTombstone()));

  iter->m_dict_elm = elm;
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaKeyset iterators.
// Since we know the array type, we can do "while (elm[pos].isTombstone())"
// inline here.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_keyset(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaKeyset());
  ssize_t pos    = iter->getPos();
  auto const arr = VanillaKeyset::asSet(ad);

  do {
    if ((++pos) == iter->getEnd()) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(arr->data()[pos].isTombstone()));

  iter->setPos(pos);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaKeysetPointer.
// Since we know the array type, we can do "while (elm->isTombstone())"
// inline here.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_vanilla_keyset_pointer(Iter* iter, ArrayData* ad) {
  assertx(ad->isVanillaKeyset());
  auto elm = iter->m_keyset_elm;

  do {
    if ((++elm) == iter->m_keyset_end) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(elm->isTombstone()));

  iter->m_keyset_elm = elm;
  return 1;
}

// "virtual" method implementation of *IterNext* for StructDict iterators.
int64_t iter_next_struct_dict(Iter* iter, ArrayData* ad) {
  assertx(isStructDict(BespokeArray::asBespoke(ad)));
  ssize_t pos = iter->getPos() + 1;
  if (UNLIKELY(pos == iter->getEnd())) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  return 1;
}

// Generic next implementation.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
int64_t iter_next_array_generic(Iter* iter, const ArrayData* ad) {
  auto const pos = ad->iter_advance(iter->getPos());
  if (pos == iter->getEnd()) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  return 1;
}

}

//////////////////////////////////////////////////////////////////////

template<bool BaseConst, bool WithKeys>
TypedValue iter_get_key_array(ArrayData* ad, ssize_t pos) noexcept {
  TRACE(2, "%s: ad %p pos %p\n", __func__, ad, (void*)pos);
  // Can't use this function without WithKeys.
  always_assert(WithKeys);

  switch (ad->kind()) {
    case ArrayData::kVecKind:
      return make_tv<KindOfInt64>(pos);
    case ArrayData::kDictKind: {
      auto const& elm = BaseConst
        ? *reinterpret_cast<VanillaDictElm*>(pos)
        : VanillaDict::as(ad)->data()[pos];
      return elm.getKey();
    }
    case ArrayData::kKeysetKind: {
      auto const& elm = BaseConst
        ? *reinterpret_cast<VanillaKeysetElm*>(pos)
        : VanillaKeyset::asSet(ad)->data()[pos];
      return *elm.datatv();
    }
    case ArrayData::kBespokeDictKind:
      if (isStructDict(BespokeArray::asBespoke(ad))) {
        return StructDict::GetPosKey(StructDict::As(ad), pos);
      }
      [[fallthrough]];
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeKeysetKind:
      return ad->nvGetKey(pos);
    case ArrayData::kNumKinds:
      not_reached();
  }
}

template
TypedValue iter_get_key_array<false, true>(ArrayData*, ssize_t) noexcept;
template
TypedValue iter_get_key_array<true, false>(ArrayData*, ssize_t) noexcept;
template
TypedValue iter_get_key_array<true, true>(ArrayData*, ssize_t) noexcept;

template<bool BaseConst, bool WithKeys>
TypedValue iter_get_value_array(ArrayData* ad, ssize_t pos) noexcept {
  TRACE(2, "%s: ad %p pos %p\n", __func__, ad, (void*)pos);
  // If the array is mutable, we don't care about WithKeys. We set it to true to
  // reduce the number of template instantiations.
  always_assert(BaseConst || WithKeys);

  switch (ad->kind()) {
    case ArrayData::kVecKind:
      return BaseConst && !WithKeys && VanillaVec::stores_unaligned_typed_values
        ? *reinterpret_cast<UnalignedTypedValue*>(pos)
        : VanillaVec::GetPosVal(ad, pos);
    case ArrayData::kDictKind: {
      auto const& elm = BaseConst
        ? *reinterpret_cast<VanillaDictElm*>(pos)
        : VanillaDict::as(ad)->data()[pos];
      return *elm.datatv();
    }
    case ArrayData::kKeysetKind: {
      auto const& elm = BaseConst
        ? *reinterpret_cast<VanillaKeysetElm*>(pos)
        : VanillaKeyset::asSet(ad)->data()[pos];
      return *elm.datatv();
    }
    case ArrayData::kBespokeDictKind:
      if (isStructDict(BespokeArray::asBespoke(ad))) {
        return StructDict::GetPosVal(StructDict::As(ad), pos);
      }
      [[fallthrough]];
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeKeysetKind:
      return ad->nvGetVal(pos);
    case ArrayData::kNumKinds:
      not_reached();
  }
}

template
TypedValue iter_get_value_array<false, true>(ArrayData*, ssize_t) noexcept;
template
TypedValue iter_get_value_array<true, false>(ArrayData*, ssize_t) noexcept;
template
TypedValue iter_get_value_array<true, true>(ArrayData*, ssize_t) noexcept;

template<bool BaseConst, bool WithKeys>
int64_t iter_init_array(Iter* iter, ArrayData* ad) noexcept {
  TRACE(2, "%s: I %p, ad %p\n", __func__, iter, ad);
  // If the array is mutable, we don't care about WithKeys. We set it to true to
  // reduce the number of template instantiations.
  always_assert(BaseConst || WithKeys);

  auto const size = ad->size();
  if (UNLIKELY(size == 0)) {
    iter->kill();
    return 0;
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.

  if (BaseConst && !ad->isVanilla()) {
    auto const bad = BespokeArray::asBespoke(ad);
    TRACE(2, "%s: Got bespoke array: %s\n", __func__, describe(bad).data());
    if (isStructDict(bad)) {
      iter->m_pos = 0;
      iter->m_end = size;
      return 1;
    }
  }

  if (LIKELY(ad->isVanillaVec())) {
    if (BaseConst && !WithKeys && VanillaVec::stores_unaligned_typed_values) {
      // We can use a pointer iterator for vanilla vecs storing unaligned
      // tvs if there are no associated keys we need to track.
      iter->m_unaligned_elm = VanillaVec::entries(ad);
      iter->m_unaligned_end = iter->m_unaligned_elm + size;
      return 1;
    }
    iter->m_pos = 0;
    iter->m_end = size;
    return 1;
  }

  if (LIKELY(ad->isVanillaDict())) {
    auto const dict = VanillaDict::as(ad);
    if (BaseConst) {
      iter->m_dict_elm = dict->data() + dict->getIterBeginNotEmpty();
      iter->m_dict_end = dict->data() + dict->iterLimit();
      return 1;
    }
    iter->m_pos = dict->getIterBeginNotEmpty();
    iter->m_end = dict->iterLimit();
    return 1;
  }

  if (LIKELY(ad->isVanillaKeyset())) {
    auto const keyset = VanillaKeyset::asSet(ad);
    if (BaseConst) {
      iter->m_keyset_elm = keyset->data() + keyset->getIterBeginNotEmpty();
      iter->m_keyset_end = keyset->data() + keyset->iterLimit();
      return 1;
    }
    iter->m_pos = keyset->getIterBeginNotEmpty();
    iter->m_end = keyset->iterLimit();
    return 1;
  }

  iter->m_pos = ad->iter_begin();
  iter->m_end = ad->iter_end();
  return 1;
}

template int64_t iter_init_array<false, true>(Iter*, ArrayData*) noexcept;
template int64_t iter_init_array<true, false>(Iter*, ArrayData*) noexcept;
template int64_t iter_init_array<true, true>(Iter*, ArrayData*) noexcept;

template<bool BaseConst, bool WithKeys>
int64_t iter_next_array(Iter* iter, ArrayData* ad) noexcept {
  TRACE(2, "%s: I %p, ad %p\n", __func__, iter, ad);
  // If the array is mutable, we don't care about WithKeys. We set it to true to
  // reduce the number of template instantiations.
  always_assert(BaseConst || WithKeys);

  switch (ad->kind()) {
    case ArrayData::kVecKind:
      return BaseConst && !WithKeys && VanillaVec::stores_unaligned_typed_values
        ? iter_next_vanilla_vec_pointer(iter, ad)
        : iter_next_vanilla_vec(iter, ad);
    case ArrayData::kDictKind:
      return BaseConst
        ? iter_next_vanilla_dict_pointer(iter, ad)
        : iter_next_vanilla_dict(iter, ad);
    case ArrayData::kKeysetKind:
      return BaseConst
        ? iter_next_vanilla_keyset_pointer(iter, ad)
        : iter_next_vanilla_keyset(iter, ad);
    case ArrayData::kBespokeDictKind:
      if (isStructDict(BespokeArray::asBespoke(ad))) {
        return iter_next_struct_dict(iter, ad);
      }
      [[fallthrough]];
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeKeysetKind:
      return iter_next_array_generic(iter, ad);
    case ArrayData::kNumKinds:
      not_reached();
  }
}

template int64_t iter_next_array<false, true>(Iter*, ArrayData*) noexcept;
template int64_t iter_next_array<true, false>(Iter*, ArrayData*) noexcept;
template int64_t iter_next_array<true, true>(Iter*, ArrayData*) noexcept;


TypedValue iter_get_key_object(ObjectData* obj) {
  TRACE(2, "%s: obj %p\n", __func__, obj);
  assertx(obj->isIterator());
  return obj->o_invoke_few_args(s_key, RuntimeCoeffects::fixme(), 0).detach();
}

TypedValue iter_get_value_object(ObjectData* obj) {
  TRACE(2, "%s: obj %p\n", __func__, obj);
  assertx(obj->isIterator());
  return obj->o_invoke_few_args(s_current, RuntimeCoeffects::fixme(), 0)
    .detach();
}

int64_t iter_init_object(ObjectData* obj) {
  TRACE(2, "%s: obj %p\n", __func__, obj);
  assertx(obj->isIterator());
  obj->o_invoke_few_args(s_rewind, RuntimeCoeffects::fixme(), 0);
  return obj->o_invoke_few_args(s_valid, RuntimeCoeffects::fixme(), 0)
    .toBoolean();
}

int64_t iter_next_object(ObjectData* obj) {
  TRACE(2, "%s: obj %p\n", __func__, obj);
  assertx(obj->isIterator());
  obj->o_invoke_few_args(s_next, RuntimeCoeffects::fixme(), 0);
  return obj->o_invoke_few_args(s_valid, RuntimeCoeffects::fixme(), 0)
    .toBoolean();
}

///////////////////////////////////////////////////////////////////////////////

}
