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

#pragma once

#include <array>
#include <cstdint>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Iterator over an array, a collection, or an object implementing the Hack
 * Iterator interface. This iterator is used by C++ code.
 *
 * Iteration in C++ normally looks like this, where "iter" invokes the "end"
 * method and "++iter" invokes the "next" method.
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 *
 * By default, iterators inc-ref their base to ensure that it won't be mutated
 * during the iteration.
 */
struct ArrayIter {
  enum NoInc { noInc = 0 };

  /*
   * Constructors.  Note that sometimes ArrayIter objects are created
   * without running their C++ constructor.  (See new_iter_array.)
   */
  ArrayIter() {
    m_data = nullptr;
  }
  explicit ArrayIter(const ArrayData* data);
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData(data);
  }
  explicit ArrayIter(const MixedArray*) = delete;
  explicit ArrayIter(const Array& array);
  explicit ArrayIter(ObjectData* obj);
  ArrayIter(ObjectData* obj, NoInc);
  explicit ArrayIter(const Object& obj);
  explicit ArrayIter(TypedValue);
  explicit ArrayIter(const Variant& v);

  // Copy ctor
  ArrayIter(const ArrayIter& iter);

  // Move ctor
  ArrayIter(ArrayIter&& iter) noexcept {
    m_data = iter.m_data;
    m_pos = iter.m_pos;
    m_end = iter.m_end;
    iter.m_data = nullptr;
  }

  // Copy assignment
  ArrayIter& operator=(const ArrayIter& iter);

  // Move assignment
  ArrayIter& operator=(ArrayIter&& iter);

  // Destructor
  ~ArrayIter() {
    destruct();
  }

  void reset() {
    destruct();
    m_data = nullptr;
  }

  explicit operator bool() { return !end(); }
  void operator++() { next(); }

  // Returns true if we've reached the end. endHelper is used for iterators
  // over objects implementing the Iterator interface.
  bool end() const {
    if (UNLIKELY(!hasArrayData())) return endHelper();
    return getArrayData() == nullptr || m_pos == m_end;
  }
  bool endHelper() const;

  // Advance the iterator's position. Assumes that end() is false. nextHelper
  // is used for iterators over objects implementing the Iterator interface.
  void next() {
    if (UNLIKELY(!hasArrayData())) return nextHelper();
    m_pos = getArrayData()->iter_advance(m_pos);
  }
  void nextHelper();

  // Return the key at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the key before returning it.
  Variant first() {
    if (UNLIKELY(!hasArrayData())) return firstHelper();
    return getArrayData()->getKey(m_pos);
  }
  Variant firstHelper();

  // TypedValue version of first. This method does NOT inc-ref the key before
  // returning it.
  TypedValue nvFirst() const {
    return getArrayData()->nvGetKey(m_pos);
  }

  // Return the value at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the value before returning it.
  Variant second();

  /*
   * Get the value at the current iterator position, without refcount ops.
   *
   * The difference between secondVal and secondValPlus is that, if called
   * when iterating an Iterable object the former will fatal and the latter
   * will throw (whereas second will invoke the current() method on the
   * Iterable object). Why this is has been lost in the mists of time.
   */
  TypedValue secondVal() const;
  TypedValue secondValPlus() const;

  // This method returns null for local iterators, and for non-local iterators
  // with an empty array base. It must be checked in end() for this reason.
  bool hasArrayData() const {
    return !((intptr_t)m_data & objectBaseTag());
  }

  const ArrayData* getArrayData() const {
    assertx(hasArrayData());
    return m_data;
  }
  void advance(ssize_t count) {
    while (!end() && count--) {
      next();
    }
  }
  void rewind();

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill();

  ObjectData* getObject() const {
    assertx(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~objectBaseTag());
  }

  // ObjectData bases have this additional bit set; ArrayData bases do not.
  static constexpr intptr_t objectBaseTag() {
    return 0b1;
  }

private:
  template <bool incRef = true>
  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  void tvInit(TypedValue);

  void destruct();

  // Set all ArrayIter fields for iteration over an array:
  //  - m_data is the array.
  //  - The type fields union is set based on the array type.
  //  - m_pos and m_end are set based on its virtual iter helpers.
  void setArrayData(const ArrayData* ad) {
    assertx((intptr_t(ad) & objectBaseTag()) == 0);
    m_data = ad;
    if (ad != nullptr) {
      m_pos = ad->iter_begin();
      m_end = ad->iter_end();
    }
  }

  // Set all ArrayIter fields for iteration over an object:
  //  - m_data is is always the object, with the lowest bit set as a flag.
  //  - We set the type fields union here.
  void setObject(ObjectData* obj) {
    assertx((intptr_t(obj) & objectBaseTag()) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | objectBaseTag());
  }

  // The iterator base. Will be null for local iterators. We set the lowest
  // bit for object iterators to distinguish them from array iterators.
  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  // Current position. Beware that when m_data is null, m_pos is uninitialized.
  // For the pointer iteration types, we use the appropriate pointers instead.
  size_t m_pos;
  size_t m_end;
};

///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Template based iteration, bypassing ArrayIter where possible

/*
 * Iterate the values of the iterable 'it'.
 *
 * If it is a collection, preCollFn will be called first, with the ObjectData
 * as a parameter. If it returns true, no further iteration will be performed.
 * This allows for certain optimizations - see eg BaseSet::addAll. Otherwise...
 *
 * If its an array or a collection, the ArrayData is passed to preArrFn, which
 * can do any necessary setup, and as with preCollFn can return true to bypass
 * any further work. Otherwise...
 *
 * The array is iterated efficiently (without ArrayIter for MixedArray,
 * PackedArray, and SetArray), and ArrFn is called for each element.
 * Otherwise...
 *
 * If its an iterable object, the object is iterated using ArrayIter, and
 * objFn is called on each element. Otherwise...
 *
 * If none of the above apply, the function returns false.
 *
 * During iteration, if objFn or arrFn returns true, iteration stops.
 *
 * There is also a supported shortcut:
 * If ObjFn is a bool, and 'it' is not an array, and not a collection,
 * IterateV will do nothing, and return the value of objFn.
 *
 * There is an overload that takes 4 arguments and passes false for ObjFn
 * as a convenient way to access this shortcut.
 */

// Overload for the case where we already know we have an array
template <typename ArrFn>
void IterateV(const ArrayData* adata, ArrFn arrFn) {
  if (adata->empty()) return;
  if (adata->isVanillaVec()) {
    PackedArray::IterateV(adata, arrFn);
  } else if (adata->isVanillaDict()) {
    MixedArray::IterateV(MixedArray::asMixed(adata), arrFn);
  } else if (adata->isVanillaKeyset()) {
    SetArray::Iterate(SetArray::asSet(adata), arrFn);
  } else {
    for (ArrayIter iter(adata); iter; ++iter) {
      if (ArrayData::call_helper(arrFn, iter.secondVal())) {
        break;
      }
    }
  }
}

template <typename PreArrFn, typename ArrFn, typename PreCollFn, typename ObjFn>
bool IterateV(const TypedValue& it,
              PreArrFn preArrFn,
              ArrFn arrFn,
              PreCollFn preCollFn,
              ObjFn objFn) {
  ArrayData* adata;
  if (LIKELY(isArrayLikeType(it.m_type))) {
    adata = it.m_data.parr;
   do_array:
    adata->incRefCount();
   do_array_no_incref:
    SCOPE_EXIT { decRefArr(adata); };
    if (ArrayData::call_helper(preArrFn, adata)) return true;
    IterateV(adata, arrFn);
    return true;
  }
  if (RO::EvalIsCompatibleClsMethType && isClsMethType(it.m_type)) {
    raiseClsMethToVecWarningHelper();
    adata = clsMethToVecHelper(it.m_data.pclsmeth).detach();
    if (adata) goto do_array_no_incref;
    return false;
  }
  if (it.m_type != KindOfObject) return false;
  auto odata = it.m_data.pobj;
  if (odata->isCollection()) {
    if (ArrayData::call_helper(preCollFn, odata)) return true;
    adata = collections::asArray(odata);
    if (adata) goto do_array;
    assertx(odata->collectionType() == CollectionType::Pair);
    auto tv = make_tv<KindOfInt64>(0);
    if (!ArrayData::call_helper(arrFn, *collections::at(odata, &tv))) {
      tv.m_data.num = 1;
      ArrayData::call_helper(arrFn, *collections::at(odata, &tv));
    }
    return true;
  }
  if (std::is_same<ObjFn, bool>::value) {
    return ArrayData::call_helper(objFn, nullptr);
  }
  bool isIterable;
  Object iterable = odata->iterableObject(isIterable);
  if (!isIterable) return false;
  for (ArrayIter iter(iterable.detach(), ArrayIter::noInc); iter; ++iter) {
    if (ArrayData::call_helper(objFn, iter.second().asTypedValue())) break;
  }
  return true;
}

template <typename ArrFn>
bool IterateV(const TypedValue& it, ArrFn arrFn) {
  return IterateV(it, false, arrFn, false, false);
}

/*
 * Iterate the keys and values of the iterable 'it'.
 *
 * The behavior is identical to that of IterateV, except the ArrFn and ObjFn
 * callbacks are called with both a key and a value.
 */

// Overload for the case where we already know we have an array
template <typename ArrFn>
void IterateKV(const ArrayData* adata, ArrFn arrFn) {
  if (adata->empty()) return;
  if (adata->isVanillaDict()) {
    MixedArray::IterateKV(MixedArray::asMixed(adata), arrFn);
  } else if (adata->isVanillaVec()) {
    PackedArray::IterateKV(adata, arrFn);
  } else if (adata->isVanillaKeyset()) {
    auto fun = [&](TypedValue v) { return arrFn(v, v); };
    SetArray::Iterate(SetArray::asSet(adata), fun);
  } else {
    for (ArrayIter iter(adata); iter; ++iter) {
      if (ArrayData::call_helper(arrFn, iter.nvFirst(), iter.secondVal())) {
        break;
      }
    }
  }
}

template <typename PreArrFn, typename ArrFn, typename PreCollFn, typename ObjFn>
bool IterateKV(const TypedValue& it,
               PreArrFn preArrFn,
               ArrFn arrFn,
               PreCollFn preCollFn,
               ObjFn objFn) {
  ArrayData* adata;
  if (LIKELY(isArrayLikeType(it.m_type))) {
    adata = it.m_data.parr;
   do_array:
    adata->incRefCount();
   do_array_no_incref:
    SCOPE_EXIT { decRefArr(adata); };
    if (ArrayData::call_helper(preArrFn, adata)) return true;
    IterateKV(adata, arrFn);
    return true;
  }
  if (RO::EvalIsCompatibleClsMethType && isClsMethType(it.m_type)) {
    raiseClsMethToVecWarningHelper();
    adata = clsMethToVecHelper(it.m_data.pclsmeth).detach();
    if (adata) goto do_array_no_incref;
    return false;
  }
  if (it.m_type != KindOfObject) return false;
  auto odata = it.m_data.pobj;
  if (odata->isCollection()) {
    if (ArrayData::call_helper(preCollFn, odata)) return true;
    adata = collections::asArray(odata);
    if (adata) goto do_array;
    assertx(odata->collectionType() == CollectionType::Pair);
    auto tv = make_tv<KindOfInt64>(0);
    if (!ArrayData::call_helper(arrFn, tv, *collections::at(odata, &tv))) {
      tv.m_data.num = 1;
      ArrayData::call_helper(arrFn, tv, *collections::at(odata, &tv));
    }
    return true;
  }
  if (std::is_same<ObjFn, bool>::value) {
    return ArrayData::call_helper(objFn, nullptr, nullptr);
  }
  bool isIterable;
  Object iterable = odata->iterableObject(isIterable);
  if (!isIterable) return false;
  for (ArrayIter iter(iterable.detach(), ArrayIter::noInc); iter; ++iter) {
    if (ArrayData::call_helper(objFn,
                               iter.first().asTypedValue(),
                               iter.second().asTypedValue())) {
      break;
    }
  }
  return true;
}

template <typename ArrFn>
bool IterateKV(const TypedValue& it, ArrFn arrFn) {
  return IterateKV(it, false, arrFn, false, false);
}

//////////////////////////////////////////////////////////////////////

}
