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

#ifndef incl_HPHP_ARRAY_ITERATOR_H_
#define incl_HPHP_ARRAY_ITERATOR_H_

#include <array>
#include <cstdint>

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/rds-local.h"
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/type-scan.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct TypedValue;
struct Iter;
struct MixedArray;

enum class IterNextIndex : uint16_t {
  ArrayPacked = 0,
  ArrayMixed,
  Array,
  Object,
};

/**
 * An iteration normally looks like this:
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 */

/**
 * Iterator for an immutable array.
 */
struct ArrayIter {
  enum Type : uint16_t {
    TypeUndefined = 0,
    TypeArray,
    TypeIterator  // for objects that implement Iterator or
                  // IteratorAggregate
  };

  enum NoInc { noInc = 0 };
  enum Local { local = 0 };

  /*
   * Constructors.  Note that sometimes ArrayIter objects are created
   * without running their C++ constructor.  (See new_iter_array.)
   */
  ArrayIter() {
    m_data = nullptr;
  }
  explicit ArrayIter(const ArrayData* data);
  ArrayIter(const ArrayData* data, NoInc) {
    setArrayData<false>(data);
    if (data) m_pos = data->iter_begin();
  }
  ArrayIter(const ArrayData* data, Local) {
    setArrayData<true>(data);
    if (data) m_pos = data->iter_begin();
  }
  explicit ArrayIter(const MixedArray*) = delete;
  explicit ArrayIter(const Array& array);
  explicit ArrayIter(ObjectData* obj);
  ArrayIter(ObjectData* obj, NoInc);
  explicit ArrayIter(const Object& obj);
  explicit ArrayIter(Cell);
  explicit ArrayIter(const Variant& v);

  // Copy ctor
  ArrayIter(const ArrayIter& iter);

  // Move ctor
  ArrayIter(ArrayIter&& iter) noexcept {
    m_data = iter.m_data;
    m_pos = iter.m_pos;
    m_itype = iter.m_itype;
    m_nextHelperIdx = iter.m_nextHelperIdx;
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
  bool end() const {
    if (LIKELY(hasArrayData())) {
      auto* ad = getArrayData();
      return !ad || m_pos == ad->iter_end();
    }
    return endHelper();
  }
  bool endHelper() const;

  void next() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assertx(ad);
      assertx(m_pos != ad->iter_end());
      m_pos = ad->iter_advance(m_pos);
      return;
    }
    nextHelper();
  }
  void nextHelper();

  bool nextLocal(const ArrayData* ad) {
    assertx(ad);
    assertx(!getArrayData());
    assertx(m_pos != ad->iter_end());
    m_pos = ad->iter_advance(m_pos);
    return m_pos == ad->iter_end();
  }

  Variant first() {
    if (LIKELY(hasArrayData())) {
      const ArrayData* ad = getArrayData();
      assertx(ad);
      assertx(m_pos != ad->iter_end());
      return ad->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant firstHelper();

  Variant firstLocal(const ArrayData* ad) const {
    assertx(!getArrayData());
    assertx(ad && m_pos != ad->iter_end());
    return ad->getKey(m_pos);
  }

  TypedValue nvFirst() const {
    auto const ad = getArrayData();
    assertx(ad && m_pos != ad->iter_end());
    return ad->nvGetKey(m_pos);
  }

  TypedValue nvFirstLocal(const ArrayData* ad) const {
    assertx(!getArrayData());
    assertx(ad && m_pos != ad->iter_end());
    return ad->nvGetKey(m_pos);
  }

  /*
   * Retrieve the value at the current position.
   */
  Variant second();

  Variant secondLocal(const ArrayData* ad) const {
    assertx(!getArrayData());
    assertx(ad && m_pos != ad->iter_end());
    return ad->getValue(m_pos);
  }

  /*
   * Get a tv_rval for the current iterator position.
   *
   * The difference between secondRval and secondRvalPlus is that, if called
   * when iterating an Iterable object the former will fatal and the latter
   * will throw (whereas second will invoke the current() method on the
   * Iterable object). Why this is has been lost in the mists of time.
   */
  tv_rval secondRval() const;
  tv_rval secondRvalPlus();

  TypedValue secondVal() const { return secondRval().tv(); }
  TypedValue secondValPlus() { return secondRvalPlus().tv(); }

  const_variant_ref secondRef() const {
    return const_variant_ref(secondRval());
  }

  // Inline version of secondRef.  Only for use in iterator helpers.
  tv_rval nvSecond() const {
    auto const ad = getArrayData();
    assertx(ad && m_pos != ad->iter_end());
    return ad->rvalPos(m_pos);
  }

  tv_rval nvSecondLocal(const ArrayData* ad) const {
    assertx(!getArrayData());
    assertx(ad && m_pos != ad->iter_end());
    return ad->rvalPos(m_pos);
  }

  bool hasArrayData() const {
    return !((intptr_t)m_data & 1);
  }

  const ArrayData* getArrayData() const {
    assertx(hasArrayData());
    return m_data;
  }
  ssize_t getPos() {
    return m_pos;
  }
  void setPos(ssize_t newPos) {
    m_pos = newPos;
  }
  void advance(ssize_t count) {
    while (!end() && count--) {
      next();
    }
  }
  void rewind();
  Type getIterType() const {
    return m_itype;
  }
  void setIterType(Type iterType) {
    m_itype = iterType;
  }

  IterNextIndex getHelperIndex() {
    return m_nextHelperIdx;
  }

  ObjectData* getObject() const {
    assertx(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~1);
  }

private:
  template<bool Local>
  friend int64_t new_iter_array(Iter*, ArrayData*, TypedValue*);
  template<bool Local>
  friend int64_t new_iter_array_key(Iter*, ArrayData*, TypedValue*,
                                    TypedValue*);

  template <bool incRef = true>
  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  void cellInit(Cell);

  void destruct();

  template <bool Local = false>
  void setArrayData(const ArrayData* ad) {
    assertx((intptr_t(ad) & 1) == 0);
    assertx(!Local || ad);
    m_data = Local ? nullptr : ad;
    m_nextHelperIdx = IterNextIndex::ArrayMixed;
    if (ad != nullptr) {
      if (ad->hasPackedLayout()) {
        m_nextHelperIdx = IterNextIndex::ArrayPacked;
      } else if (!ad->hasMixedLayout()) {
        m_nextHelperIdx = IterNextIndex::Array;
      }
    }
  }

  void setObject(ObjectData* obj) {
    assertx((intptr_t(obj) & 1) == 0);
    m_obj = (ObjectData*)((intptr_t)obj | 1);
    m_nextHelperIdx = IterNextIndex::Object;
  }

  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  // Current position. Beware that when m_data is null, m_pos is uninitialized.
  ssize_t m_pos;
  // This is unioned so new_iter_array can initialize it more
  // efficiently.
  union {
    struct {
      Type m_itype;
      IterNextIndex m_nextHelperIdx;
    };
    uint32_t m_itypeAndNextHelperIdx;
  };
};

///////////////////////////////////////////////////////////////////////////////

struct alignas(16) Iter {
  Iter() = delete;
  ~Iter() = delete;
  const ArrayIter& arr() const { return m_iter; }
        ArrayIter& arr()       { return m_iter; }
  template <bool Local> bool init(TypedValue* c1);
  bool next();
  bool nextLocal(const ArrayData*);
  void free();

private:
  ArrayIter m_iter;
};

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
 * There are also two supported shortcuts:
 * If ObjFn is a bool, and 'it' is not an array, and not a collection,
 * IterateV will do nothing, and return the value of objFn.
 *
 * If PreCollFn is a bool, and 'it' is not an array, IterateV will do nothing,
 * and return the value of preCollFn.
 *
 * There are overloads that take 4 and 3 arguments respectively, that pass
 * false for the trailing arguments as a convenience.
 */

// Overload for the case where we already know we have an array
template <typename ArrFn, bool IncRef = true>
bool IterateV(const ArrayData* adata, ArrFn arrFn) {
  if (adata->empty()) return true;
  if (adata->hasPackedLayout()) {
    PackedArray::IterateV<ArrFn, IncRef>(adata, arrFn);
  } else if (adata->hasMixedLayout()) {
    MixedArray::IterateV<ArrFn, IncRef>(MixedArray::asMixed(adata), arrFn);
  } else if (adata->isKeyset()) {
    SetArray::Iterate<ArrFn, IncRef>(SetArray::asSet(adata), arrFn);
  } else {
    for (ArrayIter iter(adata); iter; ++iter) {
      if (ArrayData::call_helper(arrFn, iter.secondVal())) {
        break;
      }
    }
  }
  return true;
}

template <typename ArrFn>
ALWAYS_INLINE bool IterateVNoInc(const ArrayData* adata, ArrFn arrFn) {
  return IterateV<ArrFn, false>(adata, std::move(arrFn));
}

template <typename PreArrFn, typename ArrFn, typename PreCollFn, typename ObjFn>
bool IterateV(const TypedValue& it,
              PreArrFn preArrFn,
              ArrFn arrFn,
              PreCollFn preCollFn,
              ObjFn objFn) {
  assertx(!isRefType(it.m_type));
  ArrayData* adata;
  if (LIKELY(isArrayLikeType(it.m_type))) {
    adata = it.m_data.parr;
   do_array:
    adata->incRefCount();
   do_array_no_incref:
    SCOPE_EXIT { decRefArr(adata); };
    if (ArrayData::call_helper(preArrFn, adata)) return true;
    return IterateV<ArrFn, false>(adata, arrFn);
  }
  if (std::is_same<PreCollFn, bool>::value) {
    return ArrayData::call_helper(preCollFn, nullptr);
  }
  if (isClsMethType(it.m_type)) {
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

template <typename PreArrFn, typename ArrFn, typename PreCollFn>
bool IterateV(const TypedValue& it,
              PreArrFn preArrFn,
              ArrFn arrFn,
              PreCollFn preCollFn) {
  return IterateV(it, preArrFn, arrFn, preCollFn, false);
}

template <typename PreArrFn, typename ArrFn>
bool IterateV(const TypedValue& it,
              PreArrFn preArrFn,
              ArrFn arrFn) {
  return IterateV(it, preArrFn, arrFn, false);
}

/*
 * Iterate the keys and values of the iterable 'it'.
 *
 * The behavior is identical to that of IterateV, except the ArrFn and ObjFn
 * callbacks are called with both a key and a value.
 */

// Overload for the case where we already know we have an array
template <typename ArrFn, bool IncRef = true>
bool IterateKV(const ArrayData* adata, ArrFn arrFn) {
  if (adata->empty()) return true;
  if (adata->hasMixedLayout()) {
    MixedArray::IterateKV<ArrFn, IncRef>(MixedArray::asMixed(adata), arrFn);
  } else if (adata->hasPackedLayout()) {
    PackedArray::IterateKV<ArrFn, IncRef>(adata, arrFn);
  } else if (adata->isKeyset()) {
    auto fun = [&](TypedValue v) { return arrFn(v, v); };
    SetArray::Iterate<decltype(fun), IncRef>(SetArray::asSet(adata), fun);
  } else {
    for (ArrayIter iter(adata); iter; ++iter) {
      if (ArrayData::call_helper(arrFn, iter.nvFirst(), iter.secondVal())) {
        break;
      }
    }
  }
  return true;
}

template <typename ArrFn>
ALWAYS_INLINE bool IterateKVNoInc(const ArrayData* adata, ArrFn arrFn) {
  return IterateKV<ArrFn, false>(adata, std::move(arrFn));
}

template <typename PreArrFn, typename ArrFn, typename PreCollFn, typename ObjFn>
bool IterateKV(const TypedValue& it,
               PreArrFn preArrFn,
               ArrFn arrFn,
               PreCollFn preCollFn,
               ObjFn objFn) {
  assertx(!isRefType(it.m_type));
  ArrayData* adata;
  if (LIKELY(isArrayLikeType(it.m_type))) {
    adata = it.m_data.parr;
   do_array:
    adata->incRefCount();
   do_array_no_incref:
    SCOPE_EXIT { decRefArr(adata); };
    if (preArrFn(adata)) return true;
    return IterateKV<ArrFn, false>(adata, arrFn);
  }
  if (std::is_same<PreCollFn, bool>::value) {
    return ArrayData::call_helper(preCollFn, nullptr);
  }
  if (isClsMethType(it.m_type)) {
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

template <typename PreArrFn, typename ArrFn, typename PreCollFn>
bool IterateKV(const TypedValue& it,
               PreArrFn preArrFn,
               ArrFn arrFn,
               PreCollFn preCollFn) {
  return IterateKV(it, preArrFn, arrFn, preCollFn, false);
}

template <typename PreArrFn, typename ArrFn>
bool IterateKV(const TypedValue& it,
               PreArrFn preArrFn,
               ArrFn arrFn) {
  return IterateKV(it, preArrFn, arrFn, false);
}

//////////////////////////////////////////////////////////////////////

template <bool Local>
int64_t new_iter_array(Iter* dest, ArrayData* arr, TypedValue* val);
template <bool Local>
int64_t new_iter_array_key(Iter* dest, ArrayData* arr, TypedValue* val,
                           TypedValue* key);
int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);


int64_t iter_next_ind(Iter* iter, TypedValue* valOut);
int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut);

int64_t liter_next_ind(Iter*, TypedValue*, ArrayData*);
int64_t liter_next_key_ind(Iter*, TypedValue*, TypedValue*, ArrayData*);

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_ARRAY_ITERATOR_H_
