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
  } else if (m_nextHelperIdx == IterNextIndex::VanillaDict) {
    assertx(ad->isVanillaDict());
  } else if (m_nextHelperIdx == IterNextIndex::VanillaDictPointer) {
    assertx(ad->isVanillaDict());
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
  if (m_nextHelperIdx == IterNextIndex::VanillaDictPointer) {
    UNUSED auto const dict = VanillaDict::as(ad);
    assertx(m_mixed_elm < m_mixed_end);
    assertx(m_mixed_end == dict->data() + dict->iterLimit());
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

NEVER_INLINE void clearOutputLocal(TypedValue* local) {
  tvDecRefCountable(local);
  local->m_type = KindOfNull;
}

}

template <bool BaseConst>
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);

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
    if (BaseConst) {
      aiter.m_mixed_elm = mixed->data() + mixed->getIterBeginNotEmpty();
      aiter.m_mixed_end = mixed->data() + mixed->iterLimit();
      aiter.setArrayNext(IterNextIndex::VanillaDictPointer);
      tvDup(*aiter.m_mixed_elm->datatv(), *valOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::VanillaDict);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  aiter.m_pos = ad->iter_begin();
  aiter.m_end = ad->iter_end();
  aiter.setArrayNext(IterNextIndex::Array);
  tvDup(ad->nvGetVal(aiter.m_pos), *valOut);
  return 1;
}

IterInitArr new_iter_array_helper(bool baseConst) {
  return baseConst ? new_iter_array<true> : new_iter_array<false>;
}

template<bool BaseConst>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);

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
    if (BaseConst) {
      aiter.m_mixed_elm = mixed->data() + mixed->getIterBeginNotEmpty();
      aiter.m_mixed_end = mixed->data() + mixed->iterLimit();
      aiter.setArrayNext(IterNextIndex::VanillaDictPointer);
      tvDup(*aiter.m_mixed_elm->datatv(), *valOut);
      tvDup(aiter.m_mixed_elm->getKey(), *keyOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::VanillaDict);
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
int64_t iter_next_vanilla_vec(Iter* it, ArrayData* ad,
                              TypedValue* valOut, TypedValue* keyOut) {
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

// "virtual" method implementation of *IterNext* for VanillaVec iterators over
// unaligned typed values. Since these values are stored one after the other,
// we can just increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_vec_pointer(Iter* it, ArrayData* ad,
                                      TypedValue* valOut, TypedValue* keyOut) {
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

// "virtual" method implementation of *IterNext* for VanillaDict iterators.
// Since we know the array type, we can do "while (elm[pos].isTombstone())"
// inline here, and we can use VanillaDict helpers to extract the key and value.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_dict(Iter* it, ArrayData* ad,
                               TypedValue* valOut, TypedValue* keyOut) {
  auto& iter     = *unwrap(it);
  ssize_t pos    = iter.getPos();
  auto const arr = VanillaDict::as(ad);

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

// "virtual" method implementation of *IterNext* for VanillaDictPointer.
// Since we know the base is VanillaDict and free of tombstones, we can simply
// increment the element pointer and compare it to the end pointer.
//
// HasKey is true for key-value iters. HasKey is true iff keyOut != nullptr.
//
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_vanilla_dict_pointer(Iter* it, ArrayData* ad,
                                       TypedValue* valOut, TypedValue* keyOut) {
  auto& iter = *unwrap(it);
  auto elm = iter.m_mixed_elm;

  do {
    if ((++elm) == iter.m_mixed_end) {
      iter.kill();
      return 0;
    }
  } while (UNLIKELY(elm->isTombstone()));

  iter.m_mixed_elm = elm;
  setOutputLocal(*elm->datatv(), valOut);
  if constexpr (HasKey) setOutputLocal(elm->getKey(), keyOut);
  return 1;
}

// "virtual" method implementation of *IterNext* for StructDict iterators.
template<bool HasKey>
int64_t iter_next_struct_dict(Iter* it, ArrayData* ad,
                              TypedValue* valOut, TypedValue* keyOut) {
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

// Generic next implementation. This method is used for both value and key-value
// iterators; for value iterators, keyOut is nullptr.
// The result is false (= 0) if iteration is done, or true (= 1) otherwise.
template<bool HasKey>
int64_t iter_next_array_generic(Iter* it, const ArrayData* ad,
                                TypedValue* valOut, TypedValue* keyOut) {
  auto& iter = *unwrap(it);
  auto const pos = ad->iter_advance(iter.getPos());
  if (pos == iter.getEnd()) {
    iter.kill();
    return 0;
  }

  iter.setPos(pos);
  tvSet(ad->nvGetVal(pos), *valOut);
  if constexpr (HasKey) tvSet(ad->nvGetKey(pos), *keyOut);
  return 1;
}

/*
 * This macro takes a name (e.g. VanillaVec) and a helper that's templated
 * on <bool HasKey> (e.g. iter_next_vanilla_vec) and produces the two helpers
 * that we'll call from the iter_next dispatch methods below.
 */
#define VTABLE_METHODS(name, fn)                                         \
  int64_t iterNext##name(Iter* it, ArrayData* ad, TypedValue* valOut) {  \
    TRACE(2, "iterNext" #name ": I %p\n", it);                           \
    return fn<false>(it, ad, valOut, nullptr);                           \
  }                                                                      \
  int64_t iterNextK##name(                                               \
      Iter* it, ArrayData* ad, TypedValue* valOut, TypedValue* keyOut) { \
    TRACE(2, "iterNextK" #name ": I %p\n", it);                          \
    return fn<true>(it, ad, valOut, keyOut);                             \
  }                                                                      \

VTABLE_METHODS(VanillaVec,         iter_next_vanilla_vec);
VTABLE_METHODS(VanillaVecPointer,  iter_next_vanilla_vec_pointer);
VTABLE_METHODS(VanillaDict,        iter_next_vanilla_dict);
VTABLE_METHODS(VanillaDictPointer, iter_next_vanilla_dict_pointer);
VTABLE_METHODS(StructDict,         iter_next_struct_dict);
VTABLE_METHODS(Array,              iter_next_array_generic);

#undef VTABLE_METHODS

using IterNextHelper  = int64_t (*)(Iter*, ArrayData*, TypedValue*);
using IterNextKHelper = int64_t (*)(Iter*, ArrayData*, TypedValue*, TypedValue*);

// The order of these function pointers must match the order that their
// corresponding IterNextIndex enum members were declared in.

const IterNextHelper g_iterNextHelpers[] = {
  &iterNextVanillaVec,
  &iterNextVanillaDict,
  &iterNextArray,
  &iterNextVanillaDictPointer,
  &iterNextVanillaVecPointer,
  &iterNextStructDict,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  &iterNextKVanillaVec,
  &iterNextKVanillaDict,
  &iterNextKArray,
  &iterNextKVanillaDictPointer,
  &iterNextKVanillaVecPointer,
  &iterNextKStructDict,
};

}

int64_t iter_next_array(Iter* iter, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "iter_next_array: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  auto const index = unwrap(iter)->getHelperIndex();
  IterNextHelper helper = g_iterNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, ad, valOut);
}

int64_t iter_next_array_key(Iter* iter,
                            ArrayData* ad,
                            TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "iter_array_next_key_ind: I %p\n", iter);
  assertx(unwrap(iter)->checkInvariants(ad));
  assertx(tvIsPlausible(*valOut));
  assertx(tvIsPlausible(*keyOut));
  auto const index = unwrap(iter)->getHelperIndex();
  IterNextKHelper helper = g_iterNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, ad, valOut, keyOut);
}


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
