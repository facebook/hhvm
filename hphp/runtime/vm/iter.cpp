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

// We don't want JIT iterators to take up too much space on the stack.
static_assert(sizeof(IterImpl) == 32, "");
static_assert(sizeof(Iter) == 32, "");

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

std::string show(IterSpecialization type) {
  if (!type.specialized) return "Unspecialized";
  return type.keyTypes().show();
}

//////////////////////////////////////////////////////////////////////

bool IterImpl::checkInvariants(const ArrayData* ad) const {
  TRACE(3, "IterImpl::checkInvariants: %lx %lx %lx (ad = %lx)\n",
        size_t(m_typeFields), m_pos, m_end, uintptr_t(ad));

  assertx(ad != nullptr);

  // Check that array's vtable index is compatible with the array's layout.
  if (m_nextHelperIdx == IterNextIndex::VanillaVec) {
    assertx(ad->isVanillaVec());
  } else if (m_nextHelperIdx == IterNextIndex::VanillaVecPointer) {
    assertx(ad->isVanillaVec());
  } else if (m_nextHelperIdx == IterNextIndex::ArrayMixed) {
    assertx(ad->isVanillaDict());
  } else if (m_nextHelperIdx == IterNextIndex::ArrayMixedPointer) {
    assertx(ad->isVanillaDict());
    assertx(ad->size() == VanillaDict::as(ad)->iterLimit());
  } else if (m_nextHelperIdx == IterNextIndex::StructDict) {
    assertx(!ad->isVanilla());
    assertx(isStructDict(BespokeArray::asBespoke(ad)));
  } else {
    // We'd like to assert the converse, too: a packed or mixed array should
    // a next helper that makes use of its layout. However, this condition
    // can fail: e.g. an APC array base can be promoted to a packed or mixed
    // array, with iteration still using array-generic code.
    assertx(m_nextHelperIdx == IterNextIndex::Array);
  }

  // Check the consistency of the pos and end fields.
  if (m_nextHelperIdx == IterNextIndex::ArrayMixedPointer) {
    assertx(m_mixed_elm < m_mixed_end);
    assertx(m_mixed_end == VanillaDict::as(ad)->data() + ad->size());
  } else if (m_nextHelperIdx == IterNextIndex::VanillaVecPointer) {
    assertx(m_unaligned_elm < m_unaligned_end);
    assertx(m_unaligned_end == VanillaVec::entries(const_cast<ArrayData*>(ad)) + ad->size());
  } else {
    assertx(m_pos < m_end);
    assertx(m_end == ad->iter_end());
  }
  return true;
}

void IterImpl::kill() {
  if (!debug) return;
  // IterImpl is not POD, so we memset each POD field separately.
  memset(&m_typeFields, kIterTrashFill, sizeof(m_typeFields));
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

/*
 * For the specialized iterator helpers below, we need to peek into the raw
 * IterImpl to manipulate its fields. We could make all of these methods
 * friends of the Iter struct, but that involves listing them in the class.
 *
 * To give us more flexibility to modify these helpers, we instead create this
 * method that exposes the underlying IterImpl by casting.
 */
IterImpl* unwrap(Iter* iter) { return &iter->m_iter; }

namespace {

/*
 * liter_value_cell* will store a copy of the current value at the address
 * given by 'out'. liter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */

inline void liter_value_cell_local_impl(Iter* iter,
                                        TypedValue* out,
                                        const ArrayData* ad) {
  auto const& arrIter = *unwrap(iter);
  tvSet(arrIter.nvSecondLocal(ad), *out);
}

inline void liter_key_cell_local_impl(Iter* iter,
                                      TypedValue* out,
                                      const ArrayData* ad) {
  auto const& arrIter = *unwrap(iter);
  tvSet(arrIter.nvFirstLocal(ad), *out);
}

NEVER_INLINE void clearOutputLocal(TypedValue* local) {
  tvDecRefCountable(local);
  local->m_type = KindOfNull;
}

}

template <IterTypeOp Type>
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  auto constexpr BaseConst = Type != IterTypeOp::LocalBaseMutable;

  auto const size = ad->size();
  if (UNLIKELY(size == 0)) {
    dest->kill();
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->type()))) {
    clearOutputLocal(valOut);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = *unwrap(dest);

  if (BaseConst && !ad->isVanilla()) {
    auto const bad = BespokeArray::asBespoke(ad);
    TRACE(2, "%s: Got bespoke array: %s\n", __func__, describe(bad).data());
    if (isStructDict(bad)) {
      aiter.m_pos = 0;
      aiter.m_end = size;
      aiter.setArrayNext(IterNextIndex::StructDict);
      auto const sad = StructDict::As(ad);
      tvDup(StructDict::GetPosVal(sad, 0), *valOut);
      return 1;
    }
  }

  if (LIKELY(ad->isVanillaVec())) {
    if (BaseConst && VanillaVec::stores_unaligned_typed_values) {
      // We can use a pointer iterator for vanilla vecs storing unaligned
      // tvs because there is no associated key we need to track.
      aiter.m_unaligned_elm = VanillaVec::entries(ad);
      aiter.m_unaligned_end = aiter.m_unaligned_elm + size;
      aiter.setArrayNext(IterNextIndex::VanillaVecPointer);
      tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
      return 1;
    }
    aiter.m_pos = 0;
    aiter.m_end = size;
    aiter.setArrayNext(IterNextIndex::VanillaVec);
    tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
    return 1;
  }

  if (LIKELY(ad->isVanillaDict())) {
    auto const mixed = VanillaDict::as(ad);
    if (BaseConst && LIKELY(mixed->iterLimit() == size)) {
      aiter.m_mixed_elm = mixed->data();
      aiter.m_mixed_end = aiter.m_mixed_elm + size;
      aiter.setArrayNext(IterNextIndex::ArrayMixedPointer);
      mixed->getArrayElm(0, valOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  aiter.m_pos = ad->iter_begin();
  aiter.m_end = ad->iter_end();
  aiter.setArrayNext(IterNextIndex::Array);
  tvDup(ad->nvGetVal(aiter.m_pos), *valOut);
  return 1;
}

IterInitArr new_iter_array_helper(IterTypeOp type) {
  switch (type) {
    case IterTypeOp::LocalBaseConst:
      return new_iter_array<IterTypeOp::LocalBaseConst>;
    case IterTypeOp::LocalBaseMutable:
      return new_iter_array<IterTypeOp::LocalBaseMutable>;
  }
  always_assert(false);
}

template<IterTypeOp Type>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  auto constexpr BaseConst = Type != IterTypeOp::LocalBaseMutable;

  auto const size = ad->size();
  if (UNLIKELY(size == 0)) {
    dest->kill();
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
  auto& aiter = *unwrap(dest);

  if (BaseConst && !ad->isVanilla()) {
    auto const bad = BespokeArray::asBespoke(ad);
    TRACE(2, "%s: Got bespoke array: %s\n", __func__, describe(bad).data());
    if (isStructDict(bad)) {
      aiter.m_pos = 0;
      aiter.m_end = size;
      aiter.setArrayNext(IterNextIndex::StructDict);
      auto const sad = StructDict::As(ad);
      tvDup(StructDict::GetPosVal(sad, 0), *valOut);
      tvCopy(StructDict::GetPosKey(sad, 0), *keyOut);
      return 1;
    }
  }

  if (ad->isVanillaVec()) {
    aiter.m_pos = 0;
    aiter.m_end = size;
    aiter.setArrayNext(IterNextIndex::VanillaVec);
    tvDup(VanillaVec::GetPosVal(ad, 0), *valOut);
    tvCopy(make_tv<KindOfInt64>(0), *keyOut);
    return 1;
  }

  if (ad->isVanillaDict()) {
    auto const mixed = VanillaDict::as(ad);
    if (BaseConst && LIKELY(mixed->iterLimit() == size)) {
      aiter.m_mixed_elm = mixed->data();
      aiter.m_mixed_end = aiter.m_mixed_elm + size;
      aiter.setArrayNext(IterNextIndex::ArrayMixedPointer);
      mixed->getArrayElm(0, valOut, keyOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut, keyOut);
    return 1;
  }

  aiter.m_pos = ad->iter_begin();
  aiter.m_end = ad->iter_end();
  aiter.setArrayNext(IterNextIndex::Array);
  tvDup(ad->nvGetVal(aiter.m_pos), *valOut);
  tvDup(ad->nvGetKey(aiter.m_pos), *keyOut);
  return 1;
}

IterInitArrKey new_iter_array_key_helper(IterTypeOp type) {
  switch (type) {
    case IterTypeOp::LocalBaseConst:
      return new_iter_array_key<IterTypeOp::LocalBaseConst>;
    case IterTypeOp::LocalBaseMutable:
      return new_iter_array_key<IterTypeOp::LocalBaseMutable>;
  }
  always_assert(false);
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

// Generic next implementation. This method is used for both value and key-value
// iterators; for value iterators, keyOut is nullptr.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
NEVER_INLINE
int64_t liter_array_next_cold(Iter* iter,
                              const ArrayData* ad,
                              TypedValue* valOut,
                              TypedValue* keyOut) {
  auto const ai = unwrap(iter);
  if (ai->nextLocal(ad)) {
    ai->kill();
    return 0;
  }
  liter_value_cell_local_impl(iter, valOut, ad);
  if (keyOut) liter_key_cell_local_impl(iter, keyOut, ad);
  return 1;
}

uint64_t liter_object_next_impl(ObjectData* obj,
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

}

// "virtual" method implementation of *IterNext* for ArrayMixedPointer.
// Since we know the base is mixed and free of tombstones, we can simply
// increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_mixed_pointer(Iter* it, TypedValue* valOut,
                                TypedValue* keyOut, ArrayData* arr) {
  auto& iter = *unwrap(it);
  auto const elm = iter.m_mixed_elm + 1;
  if (elm == iter.m_mixed_end) {
    iter.kill();
    return 0;
  }

  iter.m_mixed_elm = elm;
  setOutputLocal(*elm->datatv(), valOut);
  if (HasKey) setOutputLocal(elm->getKey(), keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for VanillaVec iterators
// over unaligned typed values. Since these values are stored one after the other,
// we can just increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_unaligned_pointer(Iter* it, TypedValue* valOut,
                                    TypedValue* keyOut, ArrayData* ad) {
  always_assert(!HasKey);
  assertx(VanillaVec::checkInvariants(ad));

  auto& iter = *unwrap(it);
  auto const elm = iter.m_unaligned_elm + 1;
  if (elm == iter.m_unaligned_end) {
    iter.kill();
    return 0;
  }

  iter.m_unaligned_elm = elm;
  setOutputLocal(*elm, valOut);
  return 1;
}

namespace {

// "virtual" method implementation of *IterNext* for VanillaVec iterators.
// Since we know the array is packed, we just need to increment the position
// and do a bounds check. The key is the position; for the value, we index.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_packed_impl(Iter* it, TypedValue* valOut,
                              TypedValue* keyOut, ArrayData* ad) {
  auto& iter = *unwrap(it);
  assertx(VanillaVec::checkInvariants(ad));

  ssize_t pos = iter.getPos() + 1;
  if (UNLIKELY(pos == iter.getEnd())) {
    iter.kill();
    return 0;
  }

  iter.setPos(pos);
  setOutputLocal(VanillaVec::GetPosVal(ad, pos), valOut);
  if constexpr (HasKey) setOutputLocal(make_tv<KindOfInt64>(pos), keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for ArrayMixed iterators.
// Since we know the array is mixed, we can do "while (elm[pos].isTombstone())"
// inline here, and we can use VanillaDict helpers to extract the key and value.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_mixed_impl(Iter* it, TypedValue* valOut,
                             TypedValue* keyOut, ArrayData* arrData) {
  auto& iter     = *unwrap(it);
  ssize_t pos    = iter.getPos();
  auto const arr = VanillaDict::as(arrData);

  do {
    if ((++pos) == iter.getEnd()) {
      iter.kill();
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));

  iter.setPos(pos);
  decRefOutputLocal(valOut);

  if constexpr (HasKey) {
    decRefOutputLocal(keyOut);
    arr->getArrayElm(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

// "virtual" method implementation of *IterNext* for StructDict iterators.
// See iter_next_mixed_impl for docs for template args and return value.
template<bool HasKey>
int64_t iter_next_struct_dict(Iter* it, TypedValue* valOut,
                              TypedValue* keyOut, ArrayData* ad) {
  auto& iter = *unwrap(it);
  auto const sad = StructDict::As(ad);

  ssize_t pos = iter.getPos() + 1;
  if (UNLIKELY(pos == iter.getEnd())) {
    iter.kill();
    return 0;
  }

  iter.setPos(pos);
  setOutputLocal(StructDict::GetPosVal(sad, pos), valOut);
  if constexpr (HasKey) setOutputLocal(StructDict::GetPosKey(sad, pos), keyOut);
  return 1;
}

}

int64_t literNextArray(Iter* it, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "literNextArray: I %p\n", it);
  return liter_array_next_cold(it, ad, valOut, nullptr);
}

int64_t literNextKArray(Iter* it, TypedValue* valOut, TypedValue* keyOut, ArrayData* ad) {
  TRACE(2, "literNextKArray: I %p\n", it);
  return liter_array_next_cold(it, ad, valOut, keyOut);
}

int64_t literNextObject(Iter*, TypedValue*, ArrayData*) {
  always_assert(false);
}
int64_t literNextKObject(Iter*, TypedValue*, TypedValue*, ArrayData*) {
  always_assert(false);
}

/*
 * This macro takes a name (e.g. VanillaVec) and a helper that's templated
 * on <bool HasKey> (e.g. iter_next_packed_impl) and produces the two helpers
 * that we'll call from the iter_next dispatch methods below.
 */
#define VTABLE_METHODS(name, fn)                                         \
  int64_t literNext##name(Iter* it, TypedValue* valOut, ArrayData* ad) { \
    TRACE(2, "literNext" #name ": I %p\n", it);                          \
    return fn<false>(it, valOut, nullptr, ad);                           \
  }                                                                      \
  int64_t literNextK##name(                                              \
      Iter* it, TypedValue* valOut, TypedValue* keyOut, ArrayData* ad) { \
    TRACE(2, "literNextK" #name ": I %p\n", it);                         \
    return fn<true>(it, valOut, keyOut, ad);                             \
  }                                                                      \

VTABLE_METHODS(VanillaVec,         iter_next_packed_impl);
VTABLE_METHODS(VanillaVecPointer,  iter_next_unaligned_pointer);
VTABLE_METHODS(ArrayMixed,         iter_next_mixed_impl);
VTABLE_METHODS(ArrayMixedPointer,  iter_next_mixed_pointer);
VTABLE_METHODS(StructDict,         iter_next_struct_dict);

#undef VTABLE_METHODS

using IterNextHelper  = int64_t (*)(Iter*, TypedValue*);
using IterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*);
using LIterNextHelper  = int64_t (*)(Iter*, TypedValue*, ArrayData*);
using LIterNextKHelper = int64_t (*)(Iter*, TypedValue*, TypedValue*, ArrayData*);

// The order of these function pointers must match the order that their
// corresponding IterNextIndex enum members were declared in.

const LIterNextHelper g_literNextHelpers[] = {
  &literNextVanillaVec,
  &literNextArrayMixed,
  &literNextArray,
  &literNextObject,
  &literNextArrayMixedPointer,
  &literNextVanillaVecPointer,
  &literNextStructDict,
};

const LIterNextKHelper g_literNextKHelpers[] = {
  &literNextKVanillaVec,
  &literNextKArrayMixed,
  &literNextKArray,
  &literNextKObject,
  &literNextKArrayMixedPointer,
  &literNextKVanillaVecPointer,
  &literNextKStructDict,
};

int64_t liter_array_next_ind(Iter* iter, TypedValue* valOut, ArrayData* ad) {
  TRACE(2, "liter_array_next_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  auto const index = unwrap(iter)->getHelperIndex();
  LIterNextHelper helper = g_literNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, ad);
}

int64_t liter_array_next_key_ind(Iter* iter,
                                 TypedValue* valOut,
                                 TypedValue* keyOut,
                                 ArrayData* ad) {
  TRACE(2, "liter_array_next_key_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  auto const index = unwrap(iter)->getHelperIndex();
  LIterNextKHelper helper = g_literNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, keyOut, ad);
}

int64_t liter_object_next_ind(TypedValue* valOut, ObjectData* obj) {
  TRACE(2, "liter_object_next_ind: obj %p\n", obj);
  assertx(tvIsPlausible(*valOut));
  return liter_object_next_impl(obj, valOut, nullptr);
}

int64_t liter_object_next_key_ind(TypedValue* valOut,
                                  TypedValue* keyOut,
                                  ObjectData* obj) {
  TRACE(2, "liter_object_next_key_ind: obj %p\n", obj);
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  return liter_object_next_impl(obj, valOut, keyOut);
}

///////////////////////////////////////////////////////////////////////////////

}
