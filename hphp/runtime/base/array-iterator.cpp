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
#include "hphp/runtime/base/array-iterator.h"

#include <algorithm>

#include <folly/Likely.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/apc-local-array-defs.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

TRACE_SET_MOD(runtime);

//////////////////////////////////////////////////////////////////////

const StaticString
  s_rewind("rewind"),
  s_valid("valid"),
  s_next("next"),
  s_key("key"),
  s_current("current");

//////////////////////////////////////////////////////////////////////

ArrayIter::ArrayIter(const ArrayData* data) {
  arrInit(data);
}

ArrayIter::ArrayIter(const Array& array) {
  arrInit(array.get());
}

ArrayIter::ArrayIter(ObjectData* obj) {
  objInit<true>(obj);
}

ArrayIter::ArrayIter(ObjectData* obj, NoInc) {
  objInit<false>(obj);
}

ArrayIter::ArrayIter(const Object& obj) {
  objInit<true>(obj.get());
}

ArrayIter::ArrayIter(const Cell c) {
  cellInit(c);
}

ArrayIter::ArrayIter(const Variant& v) {
  cellInit(*v.toCell());
}

ArrayIter::ArrayIter(const ArrayIter& iter) {
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  m_itypeAndNextHelperIdx = iter.m_itypeAndNextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
    obj->incRefCount();
  }
}

bool ArrayIter::checkInvariants(const ArrayData* ad /* = nullptr */) const {
  TRACE(3, "ArrayIter::checkInvariants: %lx %lx %lx %lx (ad = %lx)\n",
        uintptr_t(m_data), size_t(m_itypeAndNextHelperIdx),
        m_pos_diff, m_end_diff, uintptr_t(ad));

  // We can't make many assertions for iterators over objects.
  if (m_itype == ArrayIter::TypeIterator) {
    assertx(ad == nullptr);
    assertx(!hasArrayData());
    assertx(m_nextHelperIdx == IterNextIndex::Object);
    return true;
  }

  // Exactly one of the ArrayData pointers {ad, m_data} should be nullptr.
  assertx(m_itype == ArrayIter::TypeArray);
  assertx((ad == nullptr) != (m_data == nullptr));
  auto const arr = ad ? ad : m_data;

  // Check that the array's layout is reflected in its next vtable index.
  if (arr->hasPackedLayout()) {
    assertx(m_nextHelperIdx == IterNextIndex::ArrayPacked);
  } else if (arr->hasMixedLayout()) {
    assertx(m_nextHelperIdx == IterNextIndex::ArrayMixed ||
            m_nextHelperIdx == IterNextIndex::ArrayMixedNoTombstones);
  } else {
    assertx(m_nextHelperIdx == IterNextIndex::Array);
  }

  // Check the consistency of the pos and end fields.
  if (m_nextHelperIdx == IterNextIndex::ArrayMixedNoTombstones) {
    assertx(m_pos_diff < m_end_diff);
    assertx(m_end_diff == MixedArray::elmOff(arr->getSize()));
  } else {
    assertx(m_pos < m_end);
    assertx(m_end == arr->iter_end());
  }
  return true;
}

template <bool incRef /* = true */>
void ArrayIter::arrInit(const ArrayData* arr) {
  setArrayData(arr);
  if (arr && incRef) arr->incRefCount();
}

template <bool incRef>
void ArrayIter::objInit(ObjectData* obj) {
  assertx(obj);

  if (LIKELY(obj->isCollection())) {
    if (auto ad = collections::asArray(obj)) {
      ad->incRefCount();
      if (!incRef) decRefObj(obj);
      setArrayData(ad);
    } else {
      assertx(obj->collectionType() == CollectionType::Pair);
      auto arr = collections::toArray(obj);
      if (!incRef) decRefObj(obj);
      setArrayData(arr.detach());
    }
    return;
  }

  assertx(obj->instanceof(SystemLib::s_IteratorClass));
  setObject(obj);
  if (incRef) obj->incRefCount();
  try {
    obj->o_invoke_few_args(s_rewind, 0);
  } catch (...) {
    // Regardless of whether the incRef template parameter is true or
    // false, at this point this ArrayIter "owns" a reference to the
    // object and is responsible for decreffing the object when the
    // ArrayIter's lifetime ends. Normally ArrayIter's destructor takes
    // care of this, but the destructor will not get invoked if an
    // exception is thrown before the constructor finishes so we have
    // to manually handle decreffing the object here.
    this->m_data = nullptr;
    if (debug) this->m_itype = TypeUndefined;
    decRefObj(obj);
    throw;
  }
}

void ArrayIter::cellInit(const Cell c) {
  assertx(cellIsPlausible(c));
  if (LIKELY(isArrayLikeType(c.m_type))) {
    arrInit(c.m_data.parr);
  } else if (LIKELY(c.m_type == KindOfObject)) {
    objInit<true>(c.m_data.pobj);
  } else if (isClsMethType(c.m_type)) {
    raiseClsMethToVecWarningHelper();
    arrInit<false>(clsMethToVecHelper(c.m_data.pclsmeth).detach());
  } else {
    arrInit(nullptr);
  }
}

void ArrayIter::rewind() {
  assertx(hasArrayData());
  if (auto* data = getArrayData()) {
    m_pos = data->iter_begin();
  }
}

void ArrayIter::destruct() {
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (debug) m_itype = TypeUndefined;
    if (ad) decRefArr(const_cast<ArrayData*>(ad));
    return;
  }
  ObjectData* obj = getObject();
  if (debug) m_itype = TypeUndefined;
  assertx(obj);
  decRefObj(obj);
}

ArrayIter& ArrayIter::operator=(const ArrayIter& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  m_itypeAndNextHelperIdx = iter.m_itypeAndNextHelperIdx;
  if (hasArrayData()) {
    const ArrayData* ad = getArrayData();
    if (ad) const_cast<ArrayData*>(ad)->incRefCount();
  } else {
    ObjectData* obj = getObject();
    assertx(obj);
    obj->incRefCount();
  }
  return *this;
}

ArrayIter& ArrayIter::operator=(ArrayIter&& iter) {
  reset();
  m_data = iter.m_data;
  m_pos = iter.m_pos;
  m_end = iter.m_end;
  m_itypeAndNextHelperIdx = iter.m_itypeAndNextHelperIdx;
  iter.m_data = nullptr;
  return *this;
}

bool ArrayIter::endHelper() const  {
  auto obj = getObject();
  return !obj->o_invoke_few_args(s_valid, 0).toBoolean();
}

void ArrayIter::nextHelper() {
  auto obj = getObject();
  obj->o_invoke_few_args(s_next, 0);
}

Variant ArrayIter::firstHelper() {
  auto obj = getObject();
  return obj->o_invoke_few_args(s_key, 0);
}

Variant ArrayIter::second() {
  if (LIKELY(hasArrayData())) return getArrayData()->getValue(m_pos);
  return getObject()->o_invoke_few_args(s_current, 0);
}

tv_rval ArrayIter::secondRval() const {
  if (LIKELY(hasArrayData())) return getArrayData()->rvalPos(m_pos);
  raise_fatal_error("taking reference on iterator objects");
}

tv_rval ArrayIter::secondRvalPlus() {
  if (LIKELY(hasArrayData())) return getArrayData()->rvalPos(m_pos);
  throw_param_is_not_container();
}

//////////////////////////////////////////////////////////////////////

Iter* Iter::escalate() {
  if (arr().m_nextHelperIdx == IterNextIndex::ArrayMixedNoTombstones) {
    auto const base = MixedArray::elmOff(0);
    arr().m_pos = (arr().m_pos_diff - base) / sizeof(MixedArray::Elm);
    arr().m_end = (arr().m_end_diff - base) / sizeof(MixedArray::Elm);
    arr().setArrayNext(IterNextIndex::ArrayMixed);
  }
  return this;
}

template <bool Local>
bool Iter::init(TypedValue* c1) {
  assertx(!isRefType(c1->m_type));
  bool hasElems = true;
  if (isClsMethType(c1->m_type)) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      tvCastToVecInPlace(c1);
    } else {
      tvCastToVArrayInPlace(c1);
    }
  }
  if (isArrayLikeType(c1->m_type)) {
    if (!c1->m_data.parr->empty()) {
      if (Local) {
        (void) new (&arr()) ArrayIter(c1->m_data.parr, ArrayIter::local);
      } else {
        (void) new (&arr()) ArrayIter(c1->m_data.parr);
      }
      arr().setIterType(ArrayIter::TypeArray);
    } else {
      hasElems = false;
    }
  } else if (c1->m_type == KindOfObject) {
    bool isIterator;
    if (c1->m_data.pobj->isCollection()) {
      isIterator = false;
      (void) new (&arr()) ArrayIter(c1->m_data.pobj);
    } else {
      Object obj = c1->m_data.pobj->iterableObject(isIterator);
      if (isIterator) {
        (void) new (&arr()) ArrayIter(obj.detach(), ArrayIter::noInc);
      } else {
        Class* ctx = arGetContextClass(vmfp());
        auto ctxStr = ctx ? ctx->nameStr() : StrNR();
        Array iterArray(obj->o_toIterArray(ctxStr, ObjectData::EraseRefs));
        ArrayData* ad = iterArray.get();
        (void) new (&arr()) ArrayIter(ad);
      }
    }
    try {
      if (arr().end()) {
        // Iterator was empty; call the destructor on the iterator we
        // just constructed and branch to done case
        arr().~ArrayIter();
        hasElems = false;
      } else {
        arr().setIterType(
          isIterator ? ArrayIter::TypeIterator : ArrayIter::TypeArray);
      }
    } catch (...) {
      arr().~ArrayIter();
      throw;
    }
  } else {
    raise_warning("Invalid argument supplied for foreach()");
    hasElems = false;
  }
  return hasElems;
}

template bool Iter::init<false>(TypedValue*);
template bool Iter::init<true>(TypedValue*);

bool Iter::next() {
  assertx(arr().checkInvariants());
  assertx(arr().getHelperIndex() != IterNextIndex::ArrayMixedNoTombstones);
  // The emitter should never generate bytecode where the iterator
  // is at the end before IterNext is executed. However, even if
  // the iterator is at the end, it is safe to call next().
  ArrayIter* ai = &arr();
  ai->next();
  if (ai->end()) {
    // If after advancing the iterator we have reached the end, free
    // the iterator and fall through to the next instruction.
    // The ArrayIter destructor will decRef the array.
    ai->~ArrayIter();
    return false;
  }
  // If after advancing the iterator we have not reached the end,
  // jump to the location specified by the second immediate argument.
  return true;
}

bool Iter::nextLocal(const ArrayData* ad) {
  assertx(arr().checkInvariants(ad));
  assertx(arr().getHelperIndex() != IterNextIndex::ArrayMixedNoTombstones);
  auto ai = &arr();
  if (ai->nextLocal(ad)) {
    ai->~ArrayIter();
    return false;
  }
  return true;
}

void Iter::free() {
  assertx(arr().getIterType() == ArrayIter::TypeArray ||
          arr().getIterType() == ArrayIter::TypeIterator);
  arr().~ArrayIter();
}

/*
 * iter_value_cell* will store a copy of the current value at the address
 * given by 'out'. iter_value_cell* will increment the refcount of the current
 * value if appropriate.
 */

template <bool typeArray>
static inline void iter_value_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  TRACE(2, "%s: typeArray: %s, I %p, out %p\n",
           __func__, typeArray ? "true" : "false", iter, out);
  assertx((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arrIter = iter->arr();
  if (typeArray) {
    cellDup(tvToCell(arrIter.nvSecond().tv()), *out);
  } else {
    Variant val = arrIter.second();
    assertx(!isRefType(val.getRawType()));
    cellDup(*val.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

template <bool typeArray>
static inline void iter_key_cell_local_impl(Iter* iter, TypedValue* out) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  TRACE(2, "%s: I %p, out %p\n", __func__, iter, out);
  assertx((typeArray && iter->arr().getIterType() == ArrayIter::TypeArray) ||
         (!typeArray && iter->arr().getIterType() == ArrayIter::TypeIterator));
  ArrayIter& arr = iter->arr();
  if (typeArray) {
    cellCopy(arr.nvFirst(), *out);
  } else {
    Variant key = arr.first();
    cellDup(*key.asTypedValue(), *out);
  }
  tvDecRefGen(oldVal);
}

namespace {

inline void liter_value_cell_local_impl(Iter* iter,
                                        TypedValue* out,
                                        const ArrayData* ad) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  auto const& arrIter = iter->arr();
  assertx(arrIter.getIterType() == ArrayIter::TypeArray);
  assertx(!arrIter.getArrayData());
  auto const cur = arrIter.nvSecondLocal(ad);
  cellDup(tvToCell(cur.tv()), *out);
  tvDecRefGen(oldVal);
}

inline void liter_key_cell_local_impl(Iter* iter,
                                      TypedValue* out,
                                      const ArrayData* ad) {
  auto const oldVal = *out;
  assertx(!isRefType(oldVal.m_type));
  auto const& arr = iter->arr();
  assertx(arr.getIterType() == ArrayIter::TypeArray);
  assertx(!arr.getArrayData());
  cellCopy(arr.nvFirstLocal(ad), *out);
  tvDecRefGen(oldVal);
}

}

NEVER_INLINE int64_t iter_next_free_packed(Iter* iter, ArrayData* arr) {
  assertx(arr->decWillRelease());
  assertx(arr->hasPackedLayout());
  PackedArray::Release(arr);
  if (debug) iter->arr().setIterType(ArrayIter::TypeUndefined);
  return 0;
}

NEVER_INLINE int64_t iter_next_free_mixed(Iter* iter, ArrayData* arr) {
  assertx(arr->hasMixedLayout());
  assertx(arr->decWillRelease());
  MixedArray::Release(arr);
  if (debug) iter->arr().setIterType(ArrayIter::TypeUndefined);
  return 0;
}

NEVER_INLINE int64_t iter_next_free_apc(Iter* iter, APCLocalArray* arr) {
  assertx(arr->decWillRelease());
  APCLocalArray::Release(arr->asArrayData());
  if (debug) iter->arr().setIterType(ArrayIter::TypeUndefined);
  return 0;
}

/*
 * new_iter_array creates an iterator for the specified array iff the
 * array is not empty.  If new_iter_array creates an iterator, it does
 * not increment the refcount of the specified array.  If
 * new_iter_array does not create an iterator, it decRefs the array.
 */
template <bool Local>
NEVER_INLINE
int64_t new_iter_array_cold(Iter* dest, ArrayData* arr, TypedValue* valOut,
                            TypedValue* keyOut) {
  TRACE(2, "%s: I %p, arr %p\n", __func__, dest, arr);
  valOut = tvToCell(valOut);
  if (keyOut) keyOut = tvToCell(keyOut);
  if (!arr->empty()) {
    // We are transferring ownership of the array to the iterator, therefore
    // we do not need to adjust the refcount.
    if (Local) {
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::local);
      dest->arr().setIterType(ArrayIter::TypeArray);
      liter_value_cell_local_impl(dest, valOut, arr);
      if (keyOut) {
        liter_key_cell_local_impl(dest, keyOut, arr);
      }
    } else {
      (void) new (&dest->arr()) ArrayIter(arr, ArrayIter::noInc);
      dest->arr().setIterType(ArrayIter::TypeArray);
      iter_value_cell_local_impl<true>(dest, valOut);
      if (keyOut) {
        iter_key_cell_local_impl<true>(dest, keyOut);
      }
    }
    return 1LL;
  }
  // We did not transfer ownership of the array to an iterator, so we need
  // to decRef the array.
  if (!Local) decRefArr(arr);
  return 0LL;
}

template <bool Local>
int64_t new_iter_array(Iter* dest, ArrayData* ad, TypedValue* valOut) {
  TRACE(2, "%s: I %p, ad %p\n", __func__, dest, ad);
  auto const size = ad->getSize();
  if (UNLIKELY(size == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    }
    if (debug) dest->arr().setIterType(ArrayIter::TypeUndefined);
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<Local>(dest, ad, valOut, nullptr);
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = Local ? nullptr : ad;

  if (LIKELY(ad->hasPackedLayout())) {
    aiter.m_pos = 0;
    aiter.m_end = size;
    aiter.setArrayNext(IterNextIndex::ArrayPacked);
    cellDup(*tvToCell(PackedArray::GetValueRef(ad, 0)), *valOut);
    return 1;
  }

  if (LIKELY(ad->hasMixedLayout())) {
    auto const mixed = MixedArray::asMixed(ad);
    if (LIKELY(mixed->keyTypes() & MixedArray::kTombstoneKey) == 0) {
      aiter.m_pos_diff = mixed->elmOff(0);
      aiter.m_end_diff = mixed->elmOff(size);
      aiter.setArrayNext(IterNextIndex::ArrayMixedNoTombstones);
      mixed->getArrayElm(0, valOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut);
    return 1;
  }

  return new_iter_array_cold<Local>(dest, ad, valOut, nullptr);
}

template int64_t new_iter_array<false>(Iter*, ArrayData*, TypedValue*);
template int64_t new_iter_array<true>(Iter*, ArrayData*, TypedValue*);

template<bool Local>
int64_t new_iter_array_key(Iter*       dest,
                           ArrayData*  ad,
                           TypedValue* valOut,
                           TypedValue* keyOut) {
  auto const size = ad->getSize();
  if (UNLIKELY(size == 0)) {
    if (!Local) {
      if (UNLIKELY(ad->decWillRelease())) {
        if (ad->hasPackedLayout()) return iter_next_free_packed(dest, ad);
        if (ad->hasMixedLayout()) return iter_next_free_mixed(dest, ad);
      }
      ad->decRefCount();
    }
    if (debug) dest->arr().setIterType(ArrayIter::TypeUndefined);
    return 0;
  }
  if (UNLIKELY(isRefcountedType(valOut->m_type))) {
    return new_iter_array_cold<Local>(
      dest, ad, valOut, keyOut
    );
  }
  if (UNLIKELY(isRefcountedType(keyOut->m_type))) {
    return new_iter_array_cold<Local>(
      dest, ad, valOut, keyOut
    );
  }

  // We are transferring ownership of the array to the iterator, therefore
  // we do not need to adjust the refcount.
  auto& aiter = dest->arr();
  aiter.m_data = Local ? nullptr : ad;

  if (ad->hasPackedLayout()) {
    aiter.m_pos = 0;
    aiter.m_end = size;
    aiter.setArrayNext(IterNextIndex::ArrayPacked);
    cellDup(*tvToCell(PackedArray::GetValueRef(ad, 0)), *valOut);
    keyOut->m_type = KindOfInt64;
    keyOut->m_data.num = 0;
    return 1;
  }

  if (ad->hasMixedLayout()) {
    auto const mixed = MixedArray::asMixed(ad);
    if (LIKELY(mixed->keyTypes() & MixedArray::kTombstoneKey) == 0) {
      aiter.m_pos_diff = mixed->elmOff(0);
      aiter.m_end_diff = mixed->elmOff(size);
      aiter.setArrayNext(IterNextIndex::ArrayMixedNoTombstones);
      mixed->getArrayElm(0, valOut, keyOut);
      return 1;
    }
    aiter.m_pos = mixed->getIterBeginNotEmpty();
    aiter.m_end = mixed->iterLimit();
    aiter.setArrayNext(IterNextIndex::ArrayMixed);
    mixed->getArrayElm(aiter.m_pos, valOut, keyOut);
    return 1;
  }

  return new_iter_array_cold<Local>(dest, ad, valOut, keyOut);
}

template int64_t new_iter_array_key<true>(Iter* dest, ArrayData* ad,
                                          TypedValue* valOut,
                                          TypedValue* keyOut);
template int64_t new_iter_array_key<false>(Iter* dest, ArrayData* ad,
                                           TypedValue* valOut,
                                           TypedValue* keyOut);

struct FreeObj {
  FreeObj() : m_obj(0) {}
  void operator=(ObjectData* obj) { m_obj = obj; }
  ~FreeObj() { if (UNLIKELY(m_obj != nullptr)) decRefObj(m_obj); }
 private:
  ObjectData* m_obj;
};

/**
 * new_iter_object_any creates an iterator for the specified object if the
 * object is iterable and it is non-empty (has properties). If
 * new_iter_object_any creates an iterator, it does not increment the refcount
 * of the specified object. If new_iter_object does not create an iterator,
 * it decRefs the object.
 *
 * If exceptions are thrown, new_iter_object_any takes care of decRefing the
 * object.
 */
static int64_t new_iter_object_any(Iter* dest, ObjectData* obj, Class* ctx,
                                   TypedValue* valOut, TypedValue* keyOut) {
  ArrayIter::Type itType;
  {
    FreeObj fo;
    if (obj->isIterator()) {
      TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator\n",
            __func__, dest, obj, ctx);
      (void) new (&dest->arr()) ArrayIter(obj, ArrayIter::noInc);
      itType = ArrayIter::TypeIterator;
    } else {
      bool isIteratorAggregate;
      /*
       * We are not going to transfer ownership of obj to the iterator,
       * so arrange to decRef it later. The actual decRef has to happen
       * after the call to arr().end() below, because both can have visible side
       * effects (calls to valid()). Similarly it has to happen before the
       * iter_*_cell_local_impl calls below, because they call current() and
       * key() (hence the explicit scope around FreeObj fo;)
       */
      fo = obj;

      Object itObj = obj->iterableObject(isIteratorAggregate, false);
      if (isIteratorAggregate) {
        TRACE(2, "%s: I %p, obj %p, ctx %p, IteratorAggregate\n",
              __func__, dest, obj, ctx);
        (void) new (&dest->arr()) ArrayIter(itObj.detach(), ArrayIter::noInc);
        itType = ArrayIter::TypeIterator;
      } else {
        TRACE(2, "%s: I %p, obj %p, ctx %p, iterate as array\n",
              __func__, dest, obj, ctx);
        auto ctxStr = ctx ? ctx->nameStr() : StrNR();
        Array iterArray(itObj->o_toIterArray(ctxStr, ObjectData::EraseRefs));
        ArrayData* ad = iterArray.get();
        (void) new (&dest->arr()) ArrayIter(ad);
        itType = ArrayIter::TypeArray;
      }
    }
    try {
      if (dest->arr().end()) {
        // Iterator was empty; call the destructor on the iterator we just
        // constructed.
        dest->arr().~ArrayIter();
        return 0LL;
      }
    } catch (...) {
      dest->arr().~ArrayIter();
      throw;
    }
  }

  dest->arr().setIterType(itType);
  if (itType == ArrayIter::TypeIterator) {
    iter_value_cell_local_impl<false>(dest, tvToCell(valOut));
    if (keyOut) {
      iter_key_cell_local_impl<false>(dest, tvToCell(keyOut));
    }
  } else {
    iter_value_cell_local_impl<true>(dest, tvToCell(valOut));
    if (keyOut) {
      iter_key_cell_local_impl<true>(dest, tvToCell(keyOut));
    }
  }
  return 1LL;
}

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* valOut, TypedValue* keyOut) {
  TRACE(2, "%s: I %p, obj %p, ctx %p, collection or Iterator or Object\n",
        __func__, dest, obj, ctx);
  if (UNLIKELY(!obj->isCollection())) {
    return new_iter_object_any(dest, obj, ctx, valOut, keyOut);
  }

  if (auto ad = collections::asArray(obj)) {
    ad->incRefCount();
    decRefObj(obj);
    return keyOut
      ? new_iter_array_key<false>(dest, ad, valOut, keyOut)
      : new_iter_array<false>(dest, ad, valOut);
  }

  assertx(obj->collectionType() == CollectionType::Pair);
  auto arr = collections::toArray(obj);
  decRefObj(obj);
  return keyOut
    ? new_iter_array_key<false>(dest, arr.detach(), valOut, keyOut)
    : new_iter_array<false>(dest, arr.detach(), valOut);
}

NEVER_INLINE
int64_t iter_next_cold(Iter* iter, Cell* valOut, Cell* keyOut) {
  auto const ai = &iter->arr();
  assertx(ai->getIterType() == ArrayIter::TypeArray ||
         ai->getIterType() == ArrayIter::TypeIterator);
  assertx(ai->hasArrayData() || !ai->getObject()->isCollection());
  ai->next();
  if (ai->end()) {
    // The ArrayIter destructor will decRef the array
    ai->~ArrayIter();
    return 0;
  }
  if (iter->arr().getIterType() == ArrayIter::TypeArray) {
    iter_value_cell_local_impl<true>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<true>(iter, keyOut);
    }
  } else {
    iter_value_cell_local_impl<false>(iter, valOut);
    if (keyOut) {
      iter_key_cell_local_impl<false>(iter, keyOut);
    }
  }
  return 1;
}

NEVER_INLINE
int64_t liter_next_cold(Iter* iter,
                        const ArrayData* ad,
                        Cell* valOut,
                        Cell* keyOut) {
  auto const ai = &iter->arr();
  assertx(ai->getIterType() == ArrayIter::TypeArray);
  assertx(!ai->getArrayData());
  if (ai->nextLocal(ad)) {
    ai->~ArrayIter();
    return 0;
  }
  liter_value_cell_local_impl(iter, valOut, ad);
  if (keyOut) liter_key_cell_local_impl(iter, keyOut, ad);
  return 1;
}

template <bool Local>
NEVER_INLINE
static int64_t iter_next_apc_array(Iter* iter,
                                   Cell* valOut,
                                   Cell* keyOut,
                                   ArrayData* ad) {
  assertx(ad->kind() == ArrayData::kApcKind);

  auto const arrIter = &iter->arr();
  auto const arr = APCLocalArray::asApcArray(ad);
  ssize_t const pos = arr->iterAdvanceImpl(arrIter->getPos());
  if (UNLIKELY(pos == arrIter->getEnd())) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_apc(iter, arr);
      }
      arr->decRefCount();
    }
    if (debug) iter->arr().setIterType(ArrayIter::TypeUndefined);
    return 0;
  }
  arrIter->setPos(pos);

  // Note that APCLocalArray can never return KindOfRefs.
  auto const rval = APCLocalArray::RvalAtPos(arr->asArrayData(), pos);
  assertx(!isRefType(rval.type()));
  cellSet(rval.tv(), *valOut);
  if (LIKELY(!keyOut)) return 1;

  auto const key = APCLocalArray::NvGetKey(ad, pos);
  auto const oldKey = *keyOut;
  cellCopy(key, *keyOut);
  tvDecRefGen(oldKey);

  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// IterNext/IterNextK helpers

namespace {

NEVER_INLINE
int64_t iter_next_cold_inc_val(Iter* it, Cell* valOut, Cell* keyOut) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(*valOut);
  return iter_next_cold(it, valOut, keyOut);
}

NEVER_INLINE
int64_t liter_next_cold_inc_val(Iter* it,
                                Cell* valOut,
                                Cell* keyOut,
                                const ArrayData* ad) {
  /*
   * If this function is executing then valOut was already decrefed
   * during iter_next_mixed_impl.  That decref can't have had side
   * effects, because iter_next_cold would have been called otherwise.
   * So it's safe to just bump the refcount back up here, and pretend
   * like nothing ever happened.
   */
  tvIncRefGen(*valOut);
  return liter_next_cold(it, ad, valOut, keyOut);
}

NEVER_INLINE
int64_t iter_next_mixed_no_tombstones_cold(Iter* it,
                                           Cell* valOut,
                                           Cell* keyOut,
                                           MixedArrayElm* elm) {
  auto const oldVal = *valOut;
  cellDup(*tvToCell(elm->datatv()), *valOut);
  tvDecRefGen(oldVal);
  if (keyOut != nullptr) {
    auto const oldKey = *keyOut;
    cellCopy(elm->getKey(), *keyOut);
    tvDecRefGen(oldKey);
  }
  return 1;
}

NEVER_INLINE
int64_t iter_next_mixed_no_tombstones_cold_key(Iter* it,
                                               Cell* valOut,
                                               Cell* keyOut,
                                               MixedArrayElm* elm) {
  cellDup(*tvToCell(elm->datatv()), *valOut);
  if (keyOut != nullptr) {
    auto const oldKey = *keyOut;
    cellCopy(elm->getKey(), *keyOut);
    tvDecRefGen(oldKey);
  }
  return 1;
}

}

template<bool HasKey, bool Local>
ALWAYS_INLINE
int64_t iter_next_mixed_no_tombstones(Iter* it,
                                      Cell* valOut,
                                      Cell* keyOut,
                                      ArrayData* arr) {
  assertx(arr->hasMixedLayout());
  assertx(arr->size() == MixedArray::asMixed(arr)->iterLimit());

  ArrayIter& iter = it->arr();
  auto const diff = iter.m_pos_diff + sizeof(MixedArrayElm);
  assertx((diff - MixedArray::elmOff(0)) % sizeof(MixedArrayElm) == 0);

  if (diff == iter.m_end_diff) {
    if (!Local) {
      if (UNLIKELY(arr->decWillRelease())) {
        return iter_next_free_mixed(it, arr);
      }
      arr->decRefCount();
    }
    if (debug) iter.setIterType(ArrayIter::TypeUndefined);
    return 0;
  }

  iter.m_pos_diff = diff;
  auto const elm = reinterpret_cast<MixedArrayElm*>(uintptr_t(arr) + diff);

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return iter_next_mixed_no_tombstones_cold(it, valOut, keyOut, elm);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return iter_next_mixed_no_tombstones_cold_key(it, valOut, keyOut, elm);
    }
    keyOut->m_data.pcnt->decRefCount();
  }

  cellDup(*tvToCell(elm->datatv()), *valOut);
  if (HasKey) cellCopy(elm->getKey(), *keyOut);
  return 1;
}

namespace {

template<bool HasKey, bool Local>
ALWAYS_INLINE
int64_t iter_next_mixed_impl(Iter* it,
                             Cell* valOut,
                             Cell* keyOut,
                             ArrayData* arrData) {
  ArrayIter& iter    = it->arr();
  auto const arr     = MixedArray::asMixed(arrData);
  ssize_t pos        = iter.getPos();

  do {
    if ((++pos) == iter.getEnd()) {
      if (!Local) {
        if (UNLIKELY(arr->decWillRelease())) {
          return iter_next_free_mixed(it, arr->asArrayData());
        }
        arr->decRefCount();
      }
      if (debug) iter.setIterType(ArrayIter::TypeUndefined);
      return 0;
    }
  } while (UNLIKELY(arr->isTombstone(pos)));

  if (isRefcountedType(valOut->m_type)) {
    if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold(it, arrData, valOut, keyOut)
        : iter_next_cold(it, valOut, keyOut);
    }
    valOut->m_data.pcnt->decRefCount();
  }
  if (HasKey && isRefcountedType(keyOut->m_type)) {
    if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
      return Local
        ? liter_next_cold_inc_val(it, valOut, keyOut, arrData)
        : iter_next_cold_inc_val(it, valOut, keyOut);
    }
    keyOut->m_data.pcnt->decRefCount();
  }

  iter.setPos(pos);
  if (HasKey) {
    arr->getArrayElm(pos, valOut, keyOut);
  } else {
    arr->getArrayElm(pos, valOut);
  }
  return 1;
}

template<bool HasKey, bool Local>
int64_t iter_next_packed_impl(Iter* it,
                              Cell* valOut,
                              Cell* keyOut,
                              ArrayData* ad) {
  auto& iter = it->arr();
  assertx(PackedArray::checkInvariants(ad));

  ssize_t pos = iter.getPos() + 1;
  if (LIKELY(pos < iter.getEnd())) {
    if (isRefcountedType(valOut->m_type)) {
      if (UNLIKELY(valOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold(it, ad, valOut, keyOut)
          : iter_next_cold(it, valOut, keyOut);
      }
      valOut->m_data.pcnt->decRefCount();
    }
    if (HasKey && UNLIKELY(isRefcountedType(keyOut->m_type))) {
      if (UNLIKELY(keyOut->m_data.pcnt->decWillRelease())) {
        return Local
          ? liter_next_cold_inc_val(it, valOut, keyOut, ad)
          : iter_next_cold_inc_val(it, valOut, keyOut);
      }
      keyOut->m_data.pcnt->decRefCount();
    }
    iter.setPos(pos);
    cellDup(*tvToCell(PackedArray::GetValueRef(ad, pos)), *valOut);
    if (HasKey) {
      keyOut->m_data.num = pos;
      keyOut->m_type = KindOfInt64;
    }
    return 1;
  }

  // Finished iterating---we need to free the array.
  if (!Local) {
    if (UNLIKELY(ad->decWillRelease())) {
      return iter_next_free_packed(it, ad);
    }
    ad->decRefCount();
  }
  if (debug) iter.setIterType(ArrayIter::TypeUndefined);
  return 0;
}

}

int64_t iterNextArrayPacked(Iter* it, Cell* valOut) {
  TRACE(2, "iterNextArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_packed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayPacked(Iter* it, Cell* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasPackedLayout());
  return iter_next_packed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayPacked(Iter* it,
                             Cell* valOut,
                             Cell* keyOut) {
  TRACE(2, "iterNextKArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasPackedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_packed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayPacked(Iter* it,
                              Cell* valOut,
                              Cell* keyOut,
                              ArrayData* ad) {
  TRACE(2, "literNextKArrayPacked: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasPackedLayout());
  return iter_next_packed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArrayMixed(Iter* it, Cell* valOut) {
  TRACE(2, "iterNextArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_impl<false, false>(it, valOut, nullptr, ad);
}

int64_t literNextArrayMixed(Iter* it, Cell* valOut, ArrayData* ad) {
  TRACE(2, "literNextArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasMixedLayout());
  return iter_next_mixed_impl<false, true>(it, valOut, nullptr, ad);
}

int64_t iterNextKArrayMixed(Iter* it, Cell* valOut, Cell* keyOut) {
  TRACE(2, "iterNextKArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData() &&
         it->arr().getArrayData()->hasMixedLayout());
  auto const ad = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_impl<true, false>(it, valOut, keyOut, ad);
}

int64_t literNextKArrayMixed(Iter* it,
                             Cell* valOut,
                             Cell* keyOut,
                             ArrayData* ad) {
  TRACE(2, "literNextKArrayMixed: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  assertx(ad->hasMixedLayout());
  return iter_next_mixed_impl<true, true>(it, valOut, keyOut, ad);
}

int64_t iterNextArray(Iter* it, Cell* valOut) {
  TRACE(2, "iterNextArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assertx(!it->arr().getArrayData()->hasPackedLayout());
  assertx(!it->arr().getArrayData()->hasMixedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array<false>(it, valOut, nullptr, ad);
  }
  return iter_next_cold(it, valOut, nullptr);
}

int64_t literNextArray(Iter* it, Cell* valOut, ArrayData* ad) {
  TRACE(2, "literNextArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  // NB: We could have an APC local array which then promotes to a packed/mixed
  // array, so we can't assert that the array isn't packed or mixed layout here.
  if (ad->isApcArray()) {
    return iter_next_apc_array<true>(it, valOut, nullptr, ad);
  }
  return liter_next_cold(it, ad, valOut, nullptr);
}

int64_t iterNextKArray(Iter* it, Cell* valOut, Cell* keyOut) {
  TRACE(2, "iterNextKArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray &&
         it->arr().hasArrayData());
  assertx(!it->arr().getArrayData()->hasMixedLayout());
  assertx(!it->arr().getArrayData()->hasPackedLayout());

  ArrayIter& iter = it->arr();
  auto const ad = const_cast<ArrayData*>(iter.getArrayData());
  if (ad->isApcArray()) {
    return iter_next_apc_array<false>(it, valOut, keyOut, ad);
  }
  return iter_next_cold(it, valOut, keyOut);
}

int64_t literNextKArray(Iter* it, Cell* valOut, Cell* keyOut, ArrayData* ad) {
  TRACE(2, "literNextKArray: I %p\n", it);
  assertx(it->arr().getIterType() == ArrayIter::TypeArray);
  assertx(!it->arr().getArrayData());
  // NB: We could have an APC local array which then promotes to a packed/mixed
  // array, so we can't assert that the array isn't packed or mixed layout here.
  if (ad->isApcArray()) {
    return iter_next_apc_array<true>(it, valOut, keyOut, ad);
  }
  return liter_next_cold(it, ad, valOut, keyOut);
}

int64_t iterNextObject(Iter* it, Cell* valOut) {
  TRACE(2, "iterNextObject: I %p\n", it);
  // We can't just put the address of iter_next_cold in the table
  // below right now because we need to get a nullptr into the third
  // argument register for it.
  return iter_next_cold(it, valOut, nullptr);
}

int64_t literNextObject(Iter*, Cell*, ArrayData*) {
  always_assert(false);
}
int64_t literNextKObject(Iter*, Cell*, Cell*, ArrayData*) {
  always_assert(false);
}

int64_t iterNextArrayMixedNoTombstones(Iter* it, Cell* valOut) {
  TRACE(2, "iterNextArrayMixedNoTombstones: I %p\n", it);
  auto const arr = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_no_tombstones<false, false>(it, valOut, nullptr, arr);
}

int64_t iterNextKArrayMixedNoTombstones(Iter* it, Cell* valOut, Cell* keyOut) {
  TRACE(2, "iterNextKArrayMixedNoTombstones: I %p\n", it);
  auto const arr = const_cast<ArrayData*>(it->arr().getArrayData());
  return iter_next_mixed_no_tombstones<true, false>(it, valOut, keyOut, arr);
}

int64_t literNextArrayMixedNoTombstones(Iter* it, Cell* valOut,
                                        ArrayData* arr) {
  TRACE(2, "literNextArrayMixedNoTombstones: I %p\n", it);
  return iter_next_mixed_no_tombstones<false, true>(it, valOut, nullptr, arr);
}

int64_t literNextKArrayMixedNoTombstones(Iter* it, Cell* valOut,
                                         Cell* keyOut, ArrayData* arr) {
  TRACE(2, "literNextKArrayMixedNoTombstones: I %p\n", it);
  auto const ad = MixedArray::asMixed(arr);
  return iter_next_mixed_no_tombstones<true, true>(it, valOut, keyOut, ad);
}

using IterNextHelper  = int64_t (*)(Iter*, Cell*);
using IterNextKHelper = int64_t (*)(Iter*, Cell*, Cell*);

using LIterNextHelper  = int64_t (*)(Iter*, Cell*, ArrayData*);
using LIterNextKHelper = int64_t (*)(Iter*, Cell*, Cell*, ArrayData*);

const IterNextHelper g_iterNextHelpers[] = {
  &iterNextArrayPacked,
  &iterNextArrayMixed,
  &iterNextArray,
  &iterNextObject,
  &iterNextArrayMixedNoTombstones,
};

const IterNextKHelper g_iterNextKHelpers[] = {
  &iterNextKArrayPacked,
  &iterNextKArrayMixed,
  &iterNextKArray,
  &iter_next_cold, // iterNextKObject
  &iterNextKArrayMixedNoTombstones,
};

const LIterNextHelper g_literNextHelpers[] = {
  &literNextArrayPacked,
  &literNextArrayMixed,
  &literNextArray,
  &literNextObject,
  &literNextArrayMixedNoTombstones,
};

const LIterNextKHelper g_literNextKHelpers[] = {
  &literNextKArrayPacked,
  &literNextKArrayMixed,
  &literNextKArray,
  &literNextKObject,
  &literNextKArrayMixedNoTombstones,
};

int64_t iter_next_ind(Iter* iter, Cell* valOut) {
  TRACE(2, "iter_next_ind: I %p\n", iter);
  assertx(iter->arr().checkInvariants());
  assertx(cellIsPlausible(*valOut));
  auto const index = iter->arr().getHelperIndex();
  IterNextHelper helper = g_iterNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut);
}

int64_t iter_next_key_ind(Iter* iter, Cell* valOut, Cell* keyOut) {
  TRACE(2, "iter_next_key_ind: I %p\n", iter);
  assertx(iter->arr().checkInvariants());
  assertx(cellIsPlausible(*valOut));
  assertx(cellIsPlausible(*keyOut));
  auto const index = iter->arr().getHelperIndex();
  IterNextKHelper helper = g_iterNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, keyOut);
}

int64_t liter_next_ind(Iter* iter, Cell* valOut, ArrayData* ad) {
  TRACE(2, "liter_next_ind: I %p\n", iter);
  assertx(iter->arr().checkInvariants(ad));
  assertx(cellIsPlausible(*valOut));
  auto const index = iter->arr().getHelperIndex();
  LIterNextHelper helper = g_literNextHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, ad);
}

int64_t liter_next_key_ind(Iter* iter,
                           Cell* valOut,
                           Cell* keyOut,
                           ArrayData* ad) {
  TRACE(2, "liter_next_key_ind: I %p\n", iter);
  assertx(iter->arr().checkInvariants(ad));
  assertx(cellIsPlausible(*valOut));
  assertx(cellIsPlausible(*keyOut));
  auto const index = iter->arr().getHelperIndex();
  LIterNextKHelper helper = g_literNextKHelpers[static_cast<uint32_t>(index)];
  return helper(iter, valOut, keyOut, ad);
}

///////////////////////////////////////////////////////////////////////////////
}
