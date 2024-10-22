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

namespace {

NEVER_INLINE void clearOutputLocal(TypedValue* local) {
  tvDecRefCountable(local);
  local->m_type = KindOfNull;
}

}

template<bool BaseConst>
int64_t new_iter_array(Iter* iter, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, iter, ad);

  auto const size = ad->size();
  if (UNLIKELY(size == 0)) {
    iter->kill();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->type()))) {
    clearOutputLocal(valOut);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.

  if (BaseConst && !ad->isVanilla()) {
    auto const bad = BespokeArray::asBespoke(ad);
    TRACE(2, "%s: Got bespoke array: %s\n", __func__, describe(bad).data());
    if (isStructDict(bad)) {
      iter->m_pos = 0;
      iter->m_end = size;
      auto const sad = StructDict::As(ad);
      tvDup(StructDict::GetPosVal(sad, 0), *valOut);
      return 1;
    }
  }

  if (LIKELY(ad->isVanillaVec())) {
    if (BaseConst && VanillaVec::stores_unaligned_typed_values) {
      // We can use a pointer iterator for vanilla vecs storing unaligned
      // tvs because there is no associated key we need to track.
      iter->m_unaligned_elm = VanillaVec::entries(ad);
      iter->m_unaligned_end = iter->m_unaligned_elm + size;
      tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
      return 1;
    }
    iter->m_pos = 0;
    iter->m_end = size;
    tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
    return 1;
  }

  if (LIKELY(ad->isVanillaDict())) {
    auto const dict = VanillaDict::as(ad);
    if (BaseConst) {
      iter->m_dict_elm = dict->data() + dict->getIterBeginNotEmpty();
      iter->m_dict_end = dict->data() + dict->iterLimit();
      tvDup(*iter->m_dict_elm->datatv(), *valOut);
      return 1;
    }
    iter->m_pos = dict->getIterBeginNotEmpty();
    iter->m_end = dict->iterLimit();
    dict->getArrayElm(iter->m_pos, valOut);
    return 1;
  }

  if (LIKELY(ad->isVanillaKeyset())) {
    auto const keyset = VanillaKeyset::asSet(ad);
    if (BaseConst) {
      iter->m_keyset_elm = keyset->data() + keyset->getIterBeginNotEmpty();
      iter->m_keyset_end = keyset->data() + keyset->iterLimit();
      tvDup(*iter->m_keyset_elm->datatv(), *valOut);
      return 1;
    }
    iter->m_pos = keyset->getIterBeginNotEmpty();
    iter->m_end = keyset->iterLimit();
    tvDup(*keyset->data()[iter->m_pos].datatv(), *valOut);
    return 1;
  }

  iter->m_pos = ad->iter_begin();
  iter->m_end = ad->iter_end();
  tvDup(ad->nvGetVal(iter->m_pos), *valOut);
  return 1;
}

IterInitArr new_iter_array_helper(bool baseConst) {
  return baseConst ? new_iter_array<true> : new_iter_array<false>;
}

template<bool BaseConst>
int64_t new_iter_array_key(Iter*       iter,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, iter, ad);

  auto const size = ad->size();
  if (UNLIKELY(size == 0)) {
    iter->kill();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->type()))) {
    clearOutputLocal(valOut);
  }
  if (UNLIKELY(isRefcountedType(keyOut->type()))) {
    clearOutputLocal(keyOut);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.

  if (BaseConst && !ad->isVanilla()) {
    auto const bad = BespokeArray::asBespoke(ad);
    TRACE(2, "%s: Got bespoke array: %s\n", __func__, describe(bad).data());
    if (isStructDict(bad)) {
      iter->m_pos = 0;
      iter->m_end = size;
      auto const sad = StructDict::As(ad);
      tvDup(StructDict::GetPosVal(sad, 0), *valOut);
      tvCopy(StructDict::GetPosKey(sad, 0), *keyOut);
      return 1;
    }
  }

  if (ad->isVanillaVec()) {
    iter->m_pos = 0;
    iter->m_end = size;
    tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
    tvCopy(make_tv<KindOfInt64>(0), *keyOut);
    return 1;
  }

  if (ad->isVanillaDict()) {
    auto const dict = VanillaDict::as(ad);
    if (BaseConst) {
      iter->m_dict_elm = dict->data() + dict->getIterBeginNotEmpty();
      iter->m_dict_end = dict->data() + dict->iterLimit();
      tvDup(*iter->m_dict_elm->datatv(), *valOut);
      tvDup(iter->m_dict_elm->getKey(), *keyOut);
      return 1;
    }
    iter->m_pos = dict->getIterBeginNotEmpty();
    iter->m_end = dict->iterLimit();
    dict->getArrayElm(iter->m_pos, valOut, keyOut);
    return 1;
  }

  if (ad->isVanillaKeyset()) {
    auto const keyset = VanillaKeyset::asSet(ad);
    if (BaseConst) {
      iter->m_keyset_elm = keyset->data() + keyset->getIterBeginNotEmpty();
      iter->m_keyset_end = keyset->data() + keyset->iterLimit();
      tvDup(*iter->m_keyset_elm->datatv(), *valOut);
      tvDup(*valOut, *keyOut);
      return 1;
    }
    iter->m_pos = keyset->getIterBeginNotEmpty();
    iter->m_end = keyset->iterLimit();
    tvDup(*keyset->data()[iter->m_pos].datatv(), *valOut);
    tvDup(*valOut, *keyOut);
    return 1;
  }

  iter->m_pos = ad->iter_begin();
  iter->m_end = ad->iter_end();
  tvDup(ad->nvGetVal(iter->m_pos), *valOut);
  tvDup(ad->nvGetKey(iter->m_pos), *keyOut);
  return 1;
}

IterInitArrKey new_iter_array_key_helper(bool baseConst) {
  return baseConst ? new_iter_array_key<true> : new_iter_array_key<false>;
}

/**
 * new_iter_object creates an iterator for the specified Iterator object, with
 * borrow refcount semantics.
 */
int64_t new_iter_object(ObjectData* obj, TypedValue* valOut,
                        TypedValue* keyOut) {
  TRACE(2, "%s: obj %p, Iterator\n", __func__, obj);
  assertx(obj->isIterator());

  obj->o_invoke_few_args(s_rewind, RuntimeCoeffects::fixme(), 0);

  auto const end =
    !obj->o_invoke_few_args(s_valid, RuntimeCoeffects::fixme(), 0).toBoolean();
  if (end) return 0LL;

  tvMove(
    obj->o_invoke_few_args(s_current, RuntimeCoeffects::fixme(), 0).detach(),
    *valOut
  );

  if (keyOut) {
    tvMove(
      obj->o_invoke_few_args(s_key, RuntimeCoeffects::fixme(), 0).detach(),
      *keyOut
    );
  }

  return 1LL;
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers

namespace {

// Destroy the given local. Does not do refcounting ops.
NEVER_INLINE void destroyOutputLocal(TypedValue* out) {
  destructorForType(type(out))(val(out).pcnt);
}

// Dec-ref the given local, and destroy it if we dec-ref to zero.
ALWAYS_INLINE void decRefOutputLocal(TypedValue* out) {
  if (isRefcountedType(type(out)) && val(out).pcnt->decReleaseCheck()) {
    destroyOutputLocal(out);
  }
}

// Store `tv` to the given local, dec-ref-ing and releasing the old val.
ALWAYS_INLINE void setOutputLocal(TypedValue tv, TypedValue* out) {
  decRefOutputLocal(out);
  tvDup(tv, out);
}

// "virtual" method implementation of *IterNext* for VanillaVec iterators.
// Since we know the array is packed, we just need to increment the position
// and do a bounds check. The key is the position; for the value, we index.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_vec(Iter* iter, ArrayData* ad,
                              TypedValue* valOut, TypedValue* keyOut) {
  assertx(ad->isVanillaVec());

  ssize_t pos = iter->getPos() + 1;
  if (UNLIKELY(pos == iter->getEnd())) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  setOutputLocal(VanillaVec::GetPosVal(ad, pos), valOut);
  if constexpr (HasKey) setOutputLocal(make_tv<KindOfInt64>(pos), keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaVec iterators over
// unaligned typed values. Since these values are stored one after the other,
// we can just increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_vec_pointer(Iter* iter, ArrayData* ad,
                                      TypedValue* valOut, TypedValue* keyOut) {
  assertx(ad->isVanillaVec());
  always_assert(!HasKey);

  auto const elm = iter->m_unaligned_elm + 1;
  if (elm == iter->m_unaligned_end) {
    iter->kill();
    return 0;
  }

  iter->m_unaligned_elm = elm;
  setOutputLocal(*elm, valOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaDict iterators.
// Since we know the array type, we can do "while (elm[pos].isTombstone())"
// inline here, and we can use VanillaDict helpers to extract the key and value.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_dict(Iter* iter, ArrayData* ad,
                               TypedValue* valOut, TypedValue* keyOut) {
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
  decRefOutputLocal(valOut);

  if constexpr (HasKey) {
    decRefOutputLocal(keyOut);
    arr->getArrayElm(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaDictPointer.
// Since we know the base is VanillaDict and free of tombstones, we can simply
// increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_dict_pointer(Iter* iter, ArrayData* ad,
                                       TypedValue* valOut, TypedValue* keyOut) {
  assertx(ad->isVanillaDict());
  auto elm = iter->m_dict_elm;

  do {
    if ((++elm) == iter->m_dict_end) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(elm->isTombstone()));

  iter->m_dict_elm = elm;
  setOutputLocal(*elm->datatv(), valOut);
  if constexpr (HasKey) setOutputLocal(elm->getKey(), keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaKeyset iterators.
// Since we know the array type, we can do "while (elm[pos].isTombstone())"
// inline here, and we can use VanillaKeyset helpers to extract the key and
// value.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_keyset(Iter* iter, ArrayData* ad,
                                 TypedValue* valOut, TypedValue* keyOut) {
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
  setOutputLocal(*arr->data()[pos].datatv(), valOut);
  if constexpr (HasKey) setOutputLocal(*valOut, keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaKeysetPointer.
// Since we know the base is VanillaKeyset and free of tombstones, we can simply
// increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_keyset_pointer(Iter* iter, ArrayData* ad,
                                         TypedValue* valOut,
                                         TypedValue* keyOut) {
  assertx(ad->isVanillaKeyset());
  auto elm = iter->m_keyset_elm;

  do {
    if ((++elm) == iter->m_keyset_end) {
      iter->kill();
      return 0;
    }
  } while (UNLIKELY(elm->isTombstone()));

  iter->m_keyset_elm = elm;
  setOutputLocal(*elm->datatv(), valOut);
  if constexpr (HasKey) setOutputLocal(*valOut, keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for StructDict iterators.
template<bool HasKey>
int64_t iter_next_struct_dict(Iter* iter, ArrayData* ad,
                              TypedValue* valOut, TypedValue* keyOut) {
  auto const sad = StructDict::As(ad);

  ssize_t pos = iter->getPos() + 1;
  if (UNLIKELY(pos == iter->getEnd())) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  setOutputLocal(StructDict::GetPosVal(sad, pos), valOut);
  if constexpr (HasKey) setOutputLocal(StructDict::GetPosKey(sad, pos), keyOut);
  return 1;
}

// Generic next implementation. This method is used for both value and key-value
// iterators; for value iterators, keyOut is nullptr.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_array_generic(Iter* iter, const ArrayData* ad,
                                TypedValue* valOut, TypedValue* keyOut) {
  auto const pos = ad->iter_advance(iter->getPos());
  if (pos == iter->getEnd()) {
    iter->kill();
    return 0;
  }

  iter->setPos(pos);
  tvSet(ad->nvGetVal(pos), *valOut);
  if constexpr (HasKey) tvSet(ad->nvGetKey(pos), *keyOut);
  return 1;
}

}

template<bool BaseConst>
int64_t iter_next_array(Iter* iter, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "iter_next_array: I %p\n", iter);
  assertx(tvIsPlausible(*valOut));

  switch (ad->kind()) {
    case ArrayData::kVecKind:
      return BaseConst && VanillaVec::stores_unaligned_typed_values
        ? iter_next_vanilla_vec_pointer<false>(iter, ad, valOut, nullptr)
        : iter_next_vanilla_vec<false>(iter, ad, valOut, nullptr);
    case ArrayData::kDictKind:
      return BaseConst
        ? iter_next_vanilla_dict_pointer<false>(iter, ad, valOut, nullptr)
        : iter_next_vanilla_dict<false>(iter, ad, valOut, nullptr);
    case ArrayData::kKeysetKind:
      return BaseConst
        ? iter_next_vanilla_keyset_pointer<false>(iter, ad, valOut, nullptr)
        : iter_next_vanilla_keyset<false>(iter, ad, valOut, nullptr);
    case ArrayData::kBespokeDictKind: {
      auto const bad = BespokeArray::asBespoke(ad);
      return isStructDict(bad)
        ? iter_next_struct_dict<false>(iter, ad, valOut, nullptr)
        : iter_next_array_generic<false>(iter, ad, valOut, nullptr);
    }
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeKeysetKind:
      return iter_next_array_generic<false>(iter, ad, valOut, nullptr);
    case ArrayData::kNumKinds:
      not_reached();
  }
}

template<bool BaseConst>
int64_t iter_next_array_key(Iter* iter,
                            ArrayData* ad,
                            TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "iter_array_next_key_ind: I %p\n", iter);
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));

  switch (ad->kind()) {
    case ArrayData::kVecKind:
      return iter_next_vanilla_vec<true>(iter, ad, valOut, keyOut);
    case ArrayData::kDictKind:
      return BaseConst
        ? iter_next_vanilla_dict_pointer<true>(iter, ad, valOut, keyOut)
        : iter_next_vanilla_dict<true>(iter, ad, valOut, keyOut);
    case ArrayData::kKeysetKind:
      return BaseConst
        ? iter_next_vanilla_keyset_pointer<true>(iter, ad, valOut, keyOut)
        : iter_next_vanilla_keyset<true>(iter, ad, valOut, keyOut);
    case ArrayData::kBespokeDictKind: {
      auto const bad = BespokeArray::asBespoke(ad);
      return isStructDict(bad)
        ? iter_next_struct_dict<true>(iter, ad, valOut, keyOut)
        : iter_next_array_generic<true>(iter, ad, valOut, keyOut);
    }
    case ArrayData::kBespokeVecKind:
    case ArrayData::kBespokeKeysetKind:
      return iter_next_array_generic<true>(iter, ad, valOut, keyOut);
    case ArrayData::kNumKinds:
      not_reached();
  }
}

template
int64_t iter_next_array<false>(Iter*, ArrayData*, TypedValue*);
template
int64_t iter_next_array<true>(Iter*, ArrayData*, TypedValue*);
template
int64_t iter_next_array_key<false>(Iter*, ArrayData*, TypedValue*, TypedValue*);
template
int64_t iter_next_array_key<true>(Iter*, ArrayData*, TypedValue*, TypedValue*);


namespace {

uint64_t iter_next_object_impl(ObjectData* obj,
                               TypedValue* valOut,
                               TypedValue* keyOut) {
  obj->o_invoke_few_args(s_next, RuntimeCoeffects::fixme(), 0);

  auto const end =
    !obj->o_invoke_few_args(s_valid, RuntimeCoeffects::fixme(), 0).toBoolean();
  if (end) return 0LL;

  tvMove(
    obj->o_invoke_few_args(s_current, RuntimeCoeffects::fixme(), 0).detach(),
    *valOut
  );

  if (keyOut) {
    tvMove(
      obj->o_invoke_few_args(s_key, RuntimeCoeffects::fixme(), 0).detach(),
      *keyOut
    );
  }

  return 1LL;
}

}

int64_t iter_next_object(ObjectData* obj, TypedValue* valOut) {
  TRACE(2, "iter_next_object: obj %p\n", obj);
  assertx(tvIsPlausible(*valOut));
  return iter_next_object_impl(obj, valOut, nullptr);
}

int64_t iter_next_object_key(ObjectData* obj,
                             TypedValue* valOut,
                             TypedValue* keyOut) {
  TRACE(2, "iter_next_object_key: obj %p\n", obj);
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  return iter_next_object_impl(obj, valOut, keyOut);
}

///////////////////////////////////////////////////////////////////////////////

}
