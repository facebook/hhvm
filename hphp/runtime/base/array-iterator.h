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
#include "hphp/runtime/base/type-variant.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct Iter;

enum class IterTypeOp { NonLocal, LocalBaseConst, LocalBaseMutable };

enum class IterNextIndex : uint8_t {
  ArrayPacked = 0,
  ArrayMixed,
  Array,
  Object,

  // JIT-only "pointer iteration", designed for good specialized code-gen.
  // In pointer iteration, the iterator has a pointer directly into the base.
  // We can only use it when the base is guaranteed unchanged during iteration.
  //
  // This condition is true for non-local iters, and for local iters with the
  // BaseUnchanged enum value. Almost all iters fall into these two cases.
  //
  // We additionally restrict pointer iteration based on the base layout:
  //   - For PackedArrays, we only do it for value iters. (For key-value iters,
  //     the position is also the key, so we must materialize it anyway.)
  //   - Fox MixedArrays, we only use it when the base is free of tombstones.
  ArrayPackedPointer,
  ArrayMixedPointer,
};

// For iterator specialization, we pack all the information we need to generate
// specialized code in a single byte so that we can check it in one comparison.
//
// This byte should be 0 for unspecialized iterators, as created by calling the
// normal ArrayIter constructor instead of using a specialized initializer.
struct IterSpecialization {
  enum BaseType : uint8_t { Packed = 0, Mixed, Vec, Dict, kNumBaseTypes };
  enum KeyTypes : uint8_t { ArrayKey = 0, Int, Str, StaticStr, kNumKeyTypes };

  // Returns a generic (unspecialized) IterSpecialization value.
  static IterSpecialization generic() {
    IterSpecialization result;
    result.as_byte = 0;
    assertx(!result.specialized);
    return result;
  }

  union {
    uint8_t as_byte;
    struct {
      // `base_type` and `key_types` are 2-bit encodings of the enums above.
      uint8_t base_type: 2;
      uint8_t key_types: 2;

      // When we JIT a specialized iterator, we set `specialized` to true,
      // We set `output_key` for key-value iters but not for value-only iters.
      // We set `base_const` if we know the base is const during iteration.
      bool specialized: 1;
      bool output_key: 1;
      bool base_const: 1;
    };
  };
};

// Debugging output.
std::string show(IterSpecialization type);
std::string show(IterSpecialization::BaseType type);
std::string show(IterSpecialization::KeyTypes type);

/*
 * Iterator over an array, a collection, or an object implementing the Hack
 * Iterator interface. This iterator is used by both C++ code and by the JIT,
 * but the JIT usage is mediated through the "Iter" wrapper below.
 *
 * Iteration in C++ normally looks like this, where "iter" invokes the "end"
 * method and "++iter" invokes the "next" method.
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 *
 * By default, iterators inc-ref their base to ensure that it won't be mutated
 * during the iteration. HHBBC can do an analysis that marks certain iterators
 * as "local" iterators, which means that their base only changes in certain
 * controlled ways during iteration. (Specifically: either the base does not
 * change at all, or the current key is assigned a new value in the loop.)
 *
 * For local iterators, the base is kept in a frame local and passed to the
 * iterator on each iteration. Local iterators are only used by the interpteter
 * and by the JIT - they're never used by C++ code. Local iterators are never
 * used for objects, since we can't constrain writes to them in this way.
 *
 * The purpose of the local iter optimization is to try to keep local bases at
 * a refcount of 1, so that they won't be COWed by the "set the current key"
 * type of mutating operations. Apparently, this pattern is somewhat common...
 */
struct ArrayIter {
  enum Type : uint8_t {
    TypeArray,
    TypeIterator  // for objects that implement Iterator or IteratorAggregate
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
  }
  ArrayIter(const ArrayData* data, Local) {
    setArrayData<true>(data);
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
    m_typeFields = iter.m_typeFields;
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

  // Pass a non-NULL ad to checkInvariants iff this iterator is local.
  // These invariants hold as long as the iterator hasn't yet reached the end.
  bool checkInvariants(const ArrayData* ad = nullptr) const;

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
    assertx(checkInvariants());
    if (UNLIKELY(!hasArrayData())) return nextHelper();
    m_pos = getArrayData()->iter_advance(m_pos);
  }
  void nextHelper();

  bool nextLocal(const ArrayData* ad) {
    assertx(checkInvariants(ad));
    m_pos = ad->iter_advance(m_pos);
    return m_pos == m_end;
  }

  // Return the key at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the key before returning it.
  Variant first() {
    if (UNLIKELY(!hasArrayData())) return firstHelper();
    return getArrayData()->getKey(m_pos);
  }
  Variant firstHelper();

  Variant firstLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
    return ad->getKey(m_pos);
  }

  // TypedValue versions of first. Used by the JIT iterator helpers.
  // These methods do NOT inc-ref the key before returning it.
  TypedValue nvFirst() const {
    return getArrayData()->nvGetKey(m_pos);
  }
  TypedValue nvFirstLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
    return ad->nvGetKey(m_pos);
  }

  // Return the value at the current position. firstHelper is used for Objects.
  // This method and its variants inc-ref the value before returning it.
  Variant second();
  Variant secondLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
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

  // TypedValue versions of second. Used by the JIT iterator helpers.
  // These methods do NOT inc-ref the value before returning it.
  tv_rval nvSecond() const {
    return getArrayData()->rvalPos(m_pos);
  }
  tv_rval nvSecondLocal(const ArrayData* ad) const {
    assertx(getArrayData() == nullptr);
    return ad->rvalPos(m_pos);
  }

  // This method returns null for local iterators, and for non-local iterators
  // with an empty array base. It must be checked in end() for this reason.
  bool hasArrayData() const {
    return !((intptr_t)m_data & objectBaseTag());
  }

  const ArrayData* getArrayData() const {
    assertx(hasArrayData());
    return m_data;
  }
  ssize_t getPos() const {
    return m_pos;
  }
  ssize_t getEnd() const {
    return m_end;
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

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill();

  IterNextIndex getHelperIndex() {
    return m_nextHelperIdx;
  }

  ObjectData* getObject() const {
    assertx(!hasArrayData());
    return (ObjectData*)((intptr_t)m_obj & ~objectBaseTag());
  }

  // Used by native code and by the JIT to pack the m_typeFields components.
  static uint32_t packTypeFields(
      Type type, IterNextIndex index,
      IterSpecialization spec = IterSpecialization::generic()) {
    return static_cast<uint32_t>(spec.as_byte) << 16 |
           static_cast<uint32_t>(index) << 8 |
           static_cast<uint32_t>(type);
  }

  // JIT helpers used for specializing iterators.
  static constexpr size_t baseOffset() {
    return offsetof(ArrayIter, m_data);
  }
  static constexpr size_t baseSize() {
    return sizeof(m_data);
  }
  static constexpr size_t typeOffset() {
    return offsetof(ArrayIter, m_typeFields);
  }
  static constexpr size_t typeSize() {
    return sizeof(m_typeFields);
  }
  static constexpr size_t posOffset() {
    return offsetof(ArrayIter, m_pos);
  }
  static constexpr size_t posSize() {
    return sizeof(m_pos);
  }
  static constexpr size_t endOffset() {
    return offsetof(ArrayIter, m_end);
  }
  static constexpr size_t endSize() {
    return sizeof(m_end);
  }

  // When we specialize an iterator, we must *set* all m_type components (so as
  // to be compatible with native helpers) but we only need to check this byte.
  static constexpr size_t specializationOffset() {
    return offsetof(ArrayIter, m_specialization);
  }

  // ObjectData bases have this additional bit set; ArrayData bases do not.
  static constexpr intptr_t objectBaseTag() {
    return 0b1;
  }

private:
  template<IterTypeOp Type>
  friend int64_t new_iter_array(Iter*, ArrayData*, TypedValue*);
  template<IterTypeOp Type>
  friend int64_t new_iter_array_key(Iter*, ArrayData*, TypedValue*,
                                    TypedValue*);
  template<bool Local>
  friend int64_t iter_next_packed_pointer(Iter*, TypedValue*, ArrayData*);
  template<bool HasKey, bool Local>
  friend int64_t iter_next_mixed_pointer(Iter*, TypedValue*, TypedValue*, ArrayData*);

  template <bool incRef = true>
  void arrInit(const ArrayData* arr);

  template <bool incRef>
  void objInit(ObjectData* obj);

  void tvInit(TypedValue);

  void destruct();

  // Set all ArrayIter fields for iteration over an array:
  //  - m_data is either the array, or null (for local iterators).
  //  - The type fields union is set based on the array type.
  //  - m_pos and m_end are set based on its virtual iter helpers.
  template <bool Local = false>
  void setArrayData(const ArrayData* ad) {
    assertx((intptr_t(ad) & objectBaseTag()) == 0);
    assertx(!Local || ad);
    m_data = Local ? nullptr : ad;
    setArrayNext(IterNextIndex::Array);
    if (ad != nullptr) {
      if (ad->hasPackedLayout()) {
        setArrayNext(IterNextIndex::ArrayPacked);
      } else if (ad->hasMixedLayout()) {
        setArrayNext(IterNextIndex::ArrayMixed);
      }
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
    m_typeFields = packTypeFields(TypeIterator, IterNextIndex::Object);
    assertx(m_itype == TypeIterator);
    assertx(m_nextHelperIdx == IterNextIndex::Object);
    assertx(!m_specialization.specialized);
  }

  // Set the type fields of an array. These fields are packed so that we
  // can set them with a single mov-immediate to the union.
  void setArrayNext(IterNextIndex index) {
    m_typeFields = packTypeFields(TypeArray, index);
    assertx(m_itype == TypeArray);
    assertx(m_nextHelperIdx == index);
    assertx(!m_specialization.specialized);
  }

  // The iterator base. Will be null for local iterators. We set the lowest
  // bit for object iterators to distinguish them from array iterators.
  union {
    const ArrayData* m_data;
    ObjectData* m_obj;
  };
  // This field is a union so new_iter_array can set it in one instruction.
  union {
    struct {
      Type m_itype;
      IterNextIndex m_nextHelperIdx;
      IterSpecialization m_specialization;
    };
    uint32_t m_typeFields;
  };
  // Current position. Beware that when m_data is null, m_pos is uninitialized.
  // For the pointer iteration types, we use the appropriate pointers instead.
  union {
    size_t m_pos;
    TypedValue* m_packed_elm;
    MixedArrayElm* m_mixed_elm;
  };
  union {
    size_t m_end;
    TypedValue* m_packed_end;
    MixedArrayElm* m_mixed_end;
  };

  // These elements are always referenced elsewhere, either in the m_data field
  // of this iterator or in a local. (If we weren't using pointer iteration, we
  // would track elements by index, not by pointer, but GC would still work.)
  TYPE_SCAN_IGNORE_FIELD(m_packed_end);
  TYPE_SCAN_IGNORE_FIELD(m_mixed_end);
  TYPE_SCAN_IGNORE_FIELD(m_packed_elm);
  TYPE_SCAN_IGNORE_FIELD(m_mixed_elm);
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

/*
 * The iterator API used by the interpreter and the JIT. This API is relatively
 * limited, because there are only two ways to interact with iterators in Hack:
 *  1. In a "foreach" loop, using the *IterInit* / *IterNext* bytecodes.
 *  2. As a delegated generator ("yield from").
 *
 * (*IterInit* here refers to {IterInit, IterInitK, LIterInit, LIterInitK}).
 *
 * The methods exposed here should be sufficient to implement both kinds of
 * iterator behavior. To speed up "foreach" loops, we also provide helpers
 * implementing *IterInit* / *IterNext* through helpers below.
 *
 * These helpers are faster than using the Iter class's methods directly
 * because they do one vtable lookup on the array type and then execute the
 * advance / bounds check / output key-value sequence based on that lookup,
 * rather than doing a separate vtable lookup for each step.
 *
 * NOTE: If you initialize an iterator using the faster init helpers, you MUST
 * use the faster next helpers for IterNext ops. That's because the helpers may
 * make iterators that use pointer iteration, which Iter::next doesn't handle.
 * doesn't handle. This invariant is checked in debug builds.
 *
 * In practice, this constraint shouldn't be a problem, because we always use
 * the helpers to do IterNext. That's true both in the interpreter and the JIT.
 */
struct alignas(16) Iter {
  Iter() = delete;
  ~Iter() = delete;

  // Returns true if the base is non-empty. Only used for non-local iterators.
  // For local iterators, use new_iter_array / new_iter_array_key below.
  bool init(TypedValue* base);

  // Returns true if there are more elems. Only used for non-local iterators.
  // For local iterators, use liter_next_ind / liter_next_key_ind below.
  bool next();

  // Returns true if the iterator is at its end.
  bool end() const { return m_iter.end(); }

  // Get the current key and value. Assumes that the iter is not at its end.
  // These methods will inc-ref the key and value before returning it.
  Variant key() { return m_iter.first(); }
  Variant val() { return m_iter.second(); };

  // It's valid to call end() on a killed iter, but the iter is otherwise dead.
  // In debug builds, this method will overwrite the iterator with garbage.
  void kill() { m_iter.kill(); }

  // Dec-refs the base, for non-local iters. Safe to call for local iters.
  void free();

  // Debug string, used when printing a frame.
  std::string toString() const;

private:
  // Used to implement the separate helper functions below. These functions
  // peek into the Iter and directly manipulate m_iter's fields.
  friend ArrayIter* unwrap(Iter*);

  ArrayIter m_iter;
};

// Native helpers for the interpreter + JIT used to implement *IterInit* ops.
// These helpers return 1 if the base has any elements and 0 otherwise.
// (They would return a bool, but native method calls from the JIT produce GP
// register outputs, so we extend the return type to an int64_t.)
//
// If these helpers return 1, they set `val` (and `key`, for key-value iters)
// from the first key-value pair of the base.
//
// For non-local iters, if these helpers return 0, they also dec-ref the base.
//
// For the array helpers, first provide an IterTypeOp to get an IterInit helper
// to call, then call it. This indirection lets us burn the appropriate helper
// into the JIT (where we know IterTypeOp statically). For objects, we don't
// need it because the type is always NonLocal.
using IterInitArr    = int64_t(*)(Iter*, ArrayData*, TypedValue*);
using IterInitArrKey = int64_t(*)(Iter*, ArrayData*, TypedValue*, TypedValue*);

IterInitArr    new_iter_array_helper(IterTypeOp type);
IterInitArrKey new_iter_array_key_helper(IterTypeOp type);

int64_t new_iter_object(Iter* dest, ObjectData* obj, Class* ctx,
                        TypedValue* val, TypedValue* key);


// Native helpers for the interpreter + JIT used to implement *IterInit* ops.
// These helpers return 1 if the base has more elements and 0 otherwise.
// (As above, they return a logical bool which we extend to a GP register.)
//
// If these helpers return 1, they set `val` (and `key`, for key-value iters)
// from the next key-value pair of the base.
//
// For non-local iters, if these helpers return 0, they also dec-ref the base.
NEVER_INLINE int64_t iter_next_ind(Iter* iter, TypedValue* valOut);
NEVER_INLINE int64_t iter_next_key_ind(Iter* iter, TypedValue* valOut, TypedValue* keyOut);
NEVER_INLINE int64_t liter_next_ind(Iter*, TypedValue*, ArrayData*);
NEVER_INLINE int64_t liter_next_key_ind(Iter*, TypedValue*, TypedValue*, ArrayData*);

//////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_ARRAY_ITERATOR_H_
