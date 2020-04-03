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
#include "hphp/runtime/base/array-data.h"

#include <vector>
#include <array>
#include <tbb/concurrent_unordered_set.h>

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/vm/interp-helpers.h"

#include "hphp/util/exception.h"

#include "hphp/zend/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_InvalidKeysetOperationMsg{"Invalid operation on keyset"},
  s_VecUnsetMsg{"Vecs do not support unsetting non-end elements"};
///////////////////////////////////////////////////////////////////////////////

__thread std::pair<const ArrayData*, size_t> s_cachedHash;

namespace {
static_assert(
  sizeof(ArrayData) == 16,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

struct ScalarHash {
  size_t operator()(const ArrayData* arr) const {
    return hash(arr);
  }
  size_t operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    return equal(ad1, ad2);
  }
  size_t hash(const ArrayData* arr) const {
    if (arr == s_cachedHash.first) return s_cachedHash.second;
    return raw_hash(arr);
  }
  size_t raw_hash(const ArrayData* arr, arrprov::Tag tag = {}) const {
    auto ret = uint64_t{
      arr->isHackArrayType()
      ? arr->kind()
      : ArrayData::ArrayKind::kMixedKind
    };
    ret |= (uint64_t{arr->dvArray()} << 32);
    ret |= (uint64_t{arr->isLegacyArray()} << 33);

    if (RuntimeOption::EvalArrayProvenance) {
      if (!tag.valid()) tag = arrprov::getTag(arr);
      if (tag.valid()) {
        ret = folly::hash::hash_combine(ret, static_cast<int>(tag.kind()));
        ret = folly::hash::hash_combine(ret, tag.line());
        ret = folly::hash::hash_combine(ret, tag.filename());
      }
    }
    IterateKV(
      arr,
      [&](TypedValue k, TypedValue v) {
        assertx(!isRefcountedType(k.m_type) ||
                (k.m_type == KindOfString && k.m_data.pstr->isStatic()));
        assertx(!isRefcountedType(v.m_type));
        ret = folly::hash::hash_combine(
          ret,
          static_cast<int>(k.m_type), k.m_data.num,
          static_cast<int>(v.m_type));
        switch (v.m_type) {
          case KindOfNull:
            break;
          case KindOfBoolean:
          case KindOfInt64:
          case KindOfDouble:
          case KindOfPersistentString:
          case KindOfPersistentDArray:
          case KindOfPersistentVArray:
          case KindOfPersistentArray:
          case KindOfPersistentVec:
          case KindOfPersistentDict:
          case KindOfPersistentKeyset:
            ret = folly::hash::hash_combine(ret, v.m_data.num);
            break;
          case KindOfUninit:
          case KindOfString:
          case KindOfDArray:
          case KindOfVArray:
          case KindOfArray:
          case KindOfVec:
          case KindOfDict:
          case KindOfKeyset:
          case KindOfObject:
          case KindOfResource:
          case KindOfFunc:
          case KindOfClass:
          case KindOfClsMeth:
          case KindOfRecord:
            always_assert(false);
        }
      }
    );
    return ret;
  }
  bool equal(const ArrayData* ad1, const ArrayData* ad2) const {
    if (ad1 == ad2) return true;
    if (ad1->size() != ad2->size()) return false;
    if (!ArrayData::dvArrayEqual(ad1, ad2)) return false;
    if (ad1->isHackArrayType()) {
      if (!ad2->isHackArrayType()) return false;
      if (ad1->kind() != ad2->kind()) return false;
    } else if (ad2->isHackArrayType()) {
      return false;
    }

    if (ad1->isLegacyArray() != ad2->isLegacyArray()) return false;

    if (UNLIKELY(RuntimeOption::EvalArrayProvenance) &&
        arrprov::getTag(ad1) != arrprov::getTag(ad2)) {
      return false;
    }

    auto check = [] (const TypedValue& tv1, const TypedValue& tv2) {
      if (tv1.m_type != tv2.m_type) {
        // String keys from arrays might be KindOfString, even when
        // the StringData is static.
        if (!isStringType(tv1.m_type) || !isStringType(tv2.m_type)) {
          return false;
        }
        assertx(tv1.m_data.pstr->isStatic());
      }
      if (isNullType(tv1.m_type)) return true;
      return tv1.m_data.num == tv2.m_data.num;
    };

    bool equal = true;
    ArrayIter iter2{ad2};
    IterateKV(
      ad1,
      [&](TypedValue k, TypedValue v) {
        if (!check(k, *iter2.first().asTypedValue()) ||
            !check(v, iter2.secondVal())) {
          equal = false;
          return true;
        }
        ++iter2;
        return false;
      }
    );
    return equal;
  }
};

using ArrayDataMap = tbb::concurrent_unordered_set<ArrayData*,
                                                   ScalarHash,
                                                   ScalarHash>;
ArrayDataMap s_arrayDataMap;

}
///////////////////////////////////////////////////////////////////////////////

void ArrayData::GetScalarArray(ArrayData** parr, arrprov::Tag tag) {
  auto const arr = *parr;
  auto const requested_tag = RuntimeOption::EvalArrayProvenance && tag.valid();

  if (arr->isStatic() && LIKELY(!requested_tag)) return;

  auto replace = [&] (ArrayData* rep) {
    *parr = rep;
    decRefArr(arr);
    s_cachedHash.first = nullptr;
  };

  if (arr->empty() && LIKELY(!requested_tag)) {
    if (arr->isKeysetType())   return replace(staticEmptyKeysetArray());
    if (arr->isVArray())       return replace(staticEmptyVArray());
    if (arr->isDArray())       return replace(staticEmptyDArray());
    if (arr->isVecArrayType()) return replace(staticEmptyVecArray());
    if (arr->isDictType())     return replace(staticEmptyDictArray());
    return replace(staticEmptyArray());
  }

  arr->onSetEvalScalar();

  s_cachedHash.first = arr;
  s_cachedHash.second = ScalarHash{}.raw_hash(arr, tag);

  // See documentation for `tl_tag_override`.
  auto it = s_arrayDataMap.find(arr);
  if (it != s_arrayDataMap.end()) return replace(*it);

  static std::array<std::mutex, 128> s_mutexes;

  std::lock_guard<std::mutex> g {
    s_mutexes[
      s_cachedHash.second % s_mutexes.size()
    ]
  };
  it = s_arrayDataMap.find(arr);
  if (it != s_arrayDataMap.end()) return replace(*it);

  ArrayData* ad;
  if (((arr->isMixedKind() && !arr->isDArray()) ||
       arr->isApcArrayKind() ||
       arr->isGlobalsArrayKind()) &&
      arr->isVectorData()) {
    ad = PackedArray::ConvertStatic(arr);
  } else {
    ad = arr->copyStatic();
  }

  assertx(ad->isStatic());
  s_cachedHash.first = ad;
  assertx(ScalarHash{}.raw_hash(ad) == s_cachedHash.second);
  auto const DEBUG_ONLY inserted = s_arrayDataMap.insert(ad).second;
  assertx(inserted);

  if (tag.valid()) arrprov::setTag<arrprov::Mode::Emplace>(ad, tag);
  return replace(ad);
}

ArrayData* ArrayData::GetScalarArray(Array&& arr) {
  auto a = arr.detach();
  GetScalarArray(&a);
  return a;
}

ArrayData* ArrayData::GetScalarArray(Variant&& arr) {
  assertx(arr.isArray());
  auto a = arr.detach().m_data.parr;
  GetScalarArray(&a);
  return a;
}

//////////////////////////////////////////////////////////////////////

static_assert(ArrayFunctions::NK == ArrayData::ArrayKind::kNumKinds,
              "add new kinds here");

#define DISPATCH(entry)                         \
  { PackedArray::entry,                         \
    MixedArray::entry,                          \
    EmptyArray::entry,                          \
    APCLocalArray::entry,                       \
    GlobalsArray::entry,                        \
    RecordArray::entry,                         \
    MixedArray::entry##Dict,   /* Dict */       \
    PackedArray::entry##Vec,   /* Vec */        \
    SetArray::entry,           /* Keyset */     \
  },

/*
 * "Copy/grow" semantics:
 *
 *   Many of the functions that mutate arrays return an ArrayData* and
 *   take a boolean parameter called 'copy'.  The semantics of these
 *   functions is the following:
 *
 *   If the `copy' argument is false, a COW is not required for
 *   language semantics.  The array may mutate itself in place and
 *   return the same pointer, or the array may allocate a new array
 *   and return that.  This is called "growing", since it may be used
 *   if an array is out of capacity for new elements---but it is also
 *   what happens if an array needs to change kinds and can't do that
 *   in-place.  If an array grows, the old array pointer may not be
 *   used for any more array operations except to eventually call
 *   Release---these grown-from arrays are sometimes described as
 *   being in "zombie" state.
 *
 *   If the `copy' argument is true, the returned array must be
 *   indistinguishable from an array that did a COW, in terms of the
 *   contents of the array.  Whether it is the same pointer value or
 *   not is not specified.  Note, for example, that this means an
 *   array doesn't have to copy if it was asked to remove an element
 *   that doesn't exist.
 *
 *   When a function with these semantics returns a new array, the new array is
 *   already incref'd. In a few cases, an existing array (different than the
 *   source array) may be returned. In this case, the array will already be
 *   incref'd.
 */

const ArrayFunctions g_array_funcs = {
  /*
   * void Release(ArrayData*)
   *
   *   Free memory associated with an array.  Generally called when
   *   the reference count on an array drops to zero.
   */
  DISPATCH(Release)

  /*
   * tv_rval NvGetInt(const ArrayData*, int64_t key)
   * tv_rval NvGetStr(const ArrayData*, const StringData*)
   *
   *   Lookup a value in an array using an int or string key. Returns a null
   *   tv_rval if the element is not present in the array.
   */
  DISPATCH(NvGetInt)
  DISPATCH(NvGetStr)

  /*
   * ssize_t NvGetIntPos(const ArrayData*, int64_t k)
   * ssize_t NvGetStrPos(const ArrayData*, const StringData* k)
   *
   *   Lookup the position of an int or string key in the array.  Returns the
   *   canonical invalid position if the key is not in the array.
   */
  DISPATCH(NvGetIntPos)
  DISPATCH(NvGetStrPos)

  /*
   * TypedValue GetPosKey(const ArrayData*, ssize_t pos)
   * TypedValue GetPosVal(const ArrayData*, ssize_t pos)
   *
   *   Look up the key or value at a valid iterator position in this array.
   *   Both of these methods return the result without inc-ref-ing it.
   */
  DISPATCH(GetPosKey)
  DISPATCH(GetPosVal)

  /*
   * ArrayData* SetInt(ArrayData*, int64_t key, TypedValue v)
   *
   *   Set a value in the array for an integer key, with copies / escalation.
   *   SetIntMove is equivalent to SetInt, followed by a dec-ref of the value,
   *   followed by a dec-ref of the old array (if it was copied or escalated).
   */
  DISPATCH(SetInt)
  DISPATCH(SetIntMove)

  /*
   * ArrayData* SetStr(ArrayData*, StringData*, TypedValue v)
   *
   *   Set a value in the array for a string key, with copies / escalation.
   *   SetStrMove is equivalent to SetStr, followed by a dec-ref of the value,
   *   followed by a dec-ref of the old array (if it was copied or escalated).
   */
  DISPATCH(SetStr)
  DISPATCH(SetStrMove)

  /*
   * size_t Vsize(const ArrayData*)
   *
   *   This entry point essentially is only for GlobalsArray;
   *   all the other cases are not_reached().
   *
   *   Because of particulars of how GlobalsArray works,
   *   determining the size of the array is an O(N) operation---we set
   *   the size field in the generic ArrayData header to -1 in that
   *   case and dispatch through this entry point.
   */
  DISPATCH(Vsize)

  /*
   * bool IsVectorData(const ArrayData*)
   *
   *   Return true if this array is empty, or if it has only contiguous integer
   *   keys and the first key is zero.  Determining this may be an O(N)
   *   operation.
   */
  DISPATCH(IsVectorData)

  /*
   * bool ExistsInt(const ArrayData*, int64_t key)
   *
   *   Return true iff this array contains an element with the supplied integer
   *   key.
   */
  DISPATCH(ExistsInt)

  /*
   * bool ExistsStr(const ArrayData*, const StringData*)
   *
   *   Return true iff this array contains an element with the supplied string
   *   key.  The string will not undergo intish-key cast.
   */
  DISPATCH(ExistsStr)

  /*
   * arr_lval LvalInt(ArrayData*, int64_t k, bool copy)
   * arr_lval LvalStr(ArrayData*, StringData* key, bool copy)
   *
   *   Look up a value in the array by the supplied key, throwing if it doesn't
   *   exist, and return a reference to it.  This function has copy/grow
   *   semantics.
   */
  DISPATCH(LvalInt)
  DISPATCH(LvalStr)

  /*
   * ArrayData* RemoveInt(ArrayData*, int64_t key)
   *
   *   Remove an array element with an integer key, copying or escalating
   *   as necessary. If there was no entry for that element, this function
   *   will not remove it, but it may still copy the array in that case.
   */
  DISPATCH(RemoveInt)

  /*
   * ArrayData* RemoveStr(ArrayData*, const StringData*)
   *
   *   Remove an array element with a string key, copying or escalating
   *   as necessary. If there was no entry for that element, this function
   *   will not remove it, but it may still copy the array in that case.
   */
  DISPATCH(RemoveStr)

  /*
   * ssize_t IterEnd(const ArrayData*)
   *
   *   Returns the canonical invalid position for this array.  Note
   *   that if elements are added or removed from the array, the value
   *   of the array's canonical invalid position may change.
   *
   * ssize_t IterBegin(const ArrayData*)
   *
   *   Returns the position of the first element, or the canonical
   *   invalid position if this array is empty.
   *
   * ssize_t IterLast(const ArrayData*)
   *
   *   Returns the position of the last element, or the canonical
   *   invalid position if this array is empty.
   */
  DISPATCH(IterBegin)
  DISPATCH(IterLast)
  DISPATCH(IterEnd)

  /*
   * ssize_t IterAdvance(const ArrayData*, size_t pos)
   *
   *   Returns the position of the element that comes after pos, or the
   *   canonical invalid position if there are no more elements after pos.
   *   If pos is the canonical invalid position, this method will return
   *   the canonical invalid position.
   */
  DISPATCH(IterAdvance)

  /*
   * ssize_t IterRewind(const ArrayData*, size_t pos)
   *
   *   Returns the position of the element that comes before pos, or the
   *   canonical invalid position if there are no elements before pos. If
   *   pos is the canonical invalid position, no guarantees are made about
   *   what this method returns.
   */
  DISPATCH(IterRewind)

  /*
   * ArrayData* EscalateForSort(ArrayData*, SortFunction)
   *
   *   Must be called before calling any of the sort routines on an
   *   array. This gives arrays a chance to change to a kind that
   *   supports sorting.
   */
  DISPATCH(EscalateForSort)

  /*
   * void Ksort(ArrayData*, int sort_flags, bool ascending)
   *
   *   Sort an array by its keys, keeping the values associated with
   *   their respective keys.
   */
  DISPATCH(Ksort)

  /*
   * void Sort(ArrayData*, int sort_flags, bool ascending)
   *
   *   Sort an array, by values, and then assign new keys to the
   *   elements in the resulting array.
   */
  DISPATCH(Sort)

  /*
   * void Asort(ArrayData*, int sort_flags, bool ascending)
   *
   *   Sort an array and maintain index association.  This means sort
   *   the array by values, but keep the keys associated with the
   *   values they used to be associated with.
   */
  DISPATCH(Asort)

  /*
   * bool Uksort(ArrayData*, const Variant&)
   *
   *   Sort on keys with a user-defined compare function (in the
   *   variant argument).  Returns false if the user comparison
   *   function modifies the array we are sorting.
   */
  DISPATCH(Uksort)

  /*
   * bool Usort(ArrayData*, const Variant&)
   *
   *   Sort the array by values with a user-defined comparison
   *   function (in the variant).  Returns false if the user-defined
   *   comparison function modifies the array we are sorting.
   */
  DISPATCH(Usort)

  /*
   * bool Uasort(ArrayData*, const Variant&)
   *
   *   Sort array by values with a user-defined comparison function
   *   (in the variant arg), keeping the original indexes associated
   *   with the values.  Returns false if the user-defined comparison
   *   function modifies the array we are sorting.
   */
  DISPATCH(Uasort)

  /*
   * ArrayData* Copy(const ArrayData*)
   *
   *   Explicitly request that an array be copied.  This API does
   *   /not/ actually guarantee a copy occurs.
   *
   *   (E.g. GlobalsArray doesn't copy here.)
   */
  DISPATCH(Copy)

  /*
   * ArrayData* CopyStatic(const ArrayData*)
   *
   *   Copy an array, allocating the new array with malloc() instead
   *   of from the request local allocator.  This function does
   *   guarantee the returned array is a new copy---but it may throw a
   *   fatal error if this cannot be accomplished (e.g. for $GLOBALS).
   */
  DISPATCH(CopyStatic)

  /*
   * ArrayData* Append(ArrayData*, TypedValue v);
   *
   *   Append a new value to the array, with the next available integer key,
   *   copying or escalating as necessary. If there is no available integer
   *   key, no value is appended, but this method may still copy the array.
   */
  DISPATCH(Append)

  /*
   * ArrayData* PlusEq(ArrayData*, const ArrayData* elems)
   *
   *    Performs array addition, logically mutating the first array.
   *    It may return a new array if the array needed to grow, or if
   *    it needed to COW because cowCheck() was true.
   */
  DISPATCH(PlusEq)

  /*
   * ArrayData* Merge(ArrayData*, const ArrayData* elems)
   *
   *   Perform part of the semantics of the php function array_merge.
   *   (Renumbering keys is not done by this routine currently.)
   */
  DISPATCH(Merge)

  /*
   * ArrayData* Pop(ArrayData*, Variant& value);
   *
   *   Remove the last element from the array and assign it to `value'.  This
   *   function may return a new array if it decided to COW due to
   *   cowCheck().
   */
  DISPATCH(Pop)

  /*
   * ArrayData* Dequeue(ArrayData*, Variant& value)
   *
   *   Remove the first element from the array and assign it to `value'.  This
   *   function may return a new array if it decided to COW due to
   *   cowCheck().
   */
  DISPATCH(Dequeue)

  /*
   * ArrayData* Prepend(ArrayData*, TypedValue v)
   *
   *   Insert `v' as the first element of the array.  Then renumber
   *   integer keys.  This function has copy/grow semantics.  `v' must
   *   not be KindOfUninit.
   */
  DISPATCH(Prepend)

  /*
   * void Renumber(ArrayData*)
   *
   *   Renumber integer keys on the array in place.
   */
  DISPATCH(Renumber)

  /*
   * void OnSetEvalScalar(ArrayData*)
   *
   *   Go through an array and call Variant::setEvalScalar on each
   *   value, and make all string keys into static strings.
   */
  DISPATCH(OnSetEvalScalar)

   /*
   * ArrayData* ToPHPArray(ArrayData*, bool)
   *
   *   Convert to a PHP array. If already a PHP array, it will be returned
   *   unchange (without copying). If copy is false, it may be converted in
   *   place.
   */
  DISPATCH(ToPHPArray)
  DISPATCH(ToPHPArrayIntishCast)

   /*
   * ArrayData* ToDict(ArrayData*, bool)
   *
   *   Convert to a dict. If already a dict, it will be returned unchange
   *   (without copying). If copy is false, it may be converted in place. If the
   *   input array contains references, an exception will be thrown.
   */
  DISPATCH(ToDict)

  /*
   * ArrayData* ToVec(ArrayData*, bool)
   *
   *   Convert to a vec. Keys will be discarded and the vec will contain the
   *   values in iteration order. If already a vec, it will be returned
   *   unchanged (without copying). If copy is false, it may be converted in
   *   place. If the input array contains references, an exception will be
   *   thrown.
   */
  DISPATCH(ToVec)

   /*
   * ArrayData* ToKeyset(ArrayData*, bool)
   *
   *   Convert to a keyset. Keys will be discarded and the keyset will contain
   *   just the values in iteration order. If already a keyset, it will be
   *   returned unchange (without copying). If copy is false, it may be
   *   converted in place. If the input array contains references, or if the
   *   input contains values that are neither integers or strings, an exception
   *   will be thrown.
   */
  DISPATCH(ToKeyset)

  /*
   * ArrayData* ToVArray(ArrayData*, bool)
   *
   * Convert to a varray (vector-like array). The array will be converted to a
   * packed array, discarding keys. If already a packed array, it will be
   * returned with the elements unchanged, but with the DVArray flag updated. If
   * copy is false, it may be converted in place.
   */
  DISPATCH(ToVArray)

  /*
   * ArrayData* ToDArray(ArrayData*, bool)
   *
   * Convert to a darray (dict-like array). The array will be converted to a
   * mixed array. If already a mixed array, it will be returned with the
   * elements unchanged, but with the DVArray flag updated. If copy is false, it
   * may be converted in place.
   */
  DISPATCH(ToDArray)
};

#undef DISPATCH

///////////////////////////////////////////////////////////////////////////////

namespace {

DEBUG_ONLY void assertForCreate(TypedValue name) {
  always_assert(isIntType(name.m_type) || isStringType(name.m_type));
}

}

// In general, arrays can contain int-valued-strings, even though plain array
// access converts them to integers.  non-int-string assertions should go
// upstream of the ArrayData api.

bool ArrayData::IsValidKey(TypedValue k) {
  return isIntType(k.m_type) ||
        (isStringType(k.m_type) && IsValidKey(k.m_data.pstr));
}

bool ArrayData::IsValidKey(const Variant& k) {
  return IsValidKey(*k.asTypedValue());
}

bool ArrayData::IsValidKey(const String& k) {
  return IsValidKey(k.get());
}

ArrayData* ArrayData::Create(TypedValue value) {
  PackedArrayInit pai(1);
  pai.append(value);
  return pai.create();
}

ArrayData* ArrayData::Create(TypedValue name, TypedValue value) {
  if (debug) assertForCreate(name);

  ArrayInit init(1, ArrayInit::Map{});
  init.setValidKey(name, value);
  return init.create();
}

///////////////////////////////////////////////////////////////////////////////
// reads

ALWAYS_INLINE
bool ArrayData::EqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                            bool strict) {
  assertx(ad1->isPHPArrayType());
  assertx(ad2->isPHPArrayType());

  if (ad1 == ad2) return true;

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatDVCmpNotices &&
               !ArrayData::dvArrayEqual(ad1, ad2))) {
    raiseHackArrCompatDVArrCmp(ad1, ad2, /* is relational */ false);
  }

  if (ad1->size() != ad2->size()) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  if (strict) {
    for (ArrayIter iter1{ad1}, iter2{ad2}; iter1; ++iter1, ++iter2) {
      assertx(iter2);
      if (!same(iter1.first(), iter2.first()) ||
          !tvSame(iter1.secondVal(), iter2.secondVal())) {
        return false;
      }
    }
    return true;
  } else {
    bool equal = true;
    IterateKV(
      ad1,
      [&](TypedValue k, TypedValue v) {
        if (!ad2->exists(k) || !tvEqual(v, ad2->get(k).tv())) {
          equal = false;
          return true;
        }
        return false;
      }
    );
    return equal;
  }
}

ALWAYS_INLINE
int64_t ArrayData::CompareHelper(const ArrayData* ad1, const ArrayData* ad2) {
  assertx(ad1->isPHPArrayType());
  assertx(ad2->isPHPArrayType());

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatDVCmpNotices)) {
    if (!ArrayData::dvArrayEqual(ad1, ad2)) {
      raiseHackArrCompatDVArrCmp(ad1, ad2, /* is relational */ true);
    } else if (ad1->isDArray()) {
      raise_hackarr_compat_notice("Comparing two darrays relationally");
    }
  }

  auto const size1 = ad1->size();
  auto const size2 = ad2->size();
  if (size1 < size2) return -1;
  if (size1 > size2) return 1;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  int result = 0;
  IterateKV(
    ad1,
    [&](TypedValue k, TypedValue v) {
      if (!ad2->exists(k)) {
        result = 1;
        return true;
      }
      auto const cmp = tvCompare(v, ad2->get(k).tv());
      if (cmp != 0) {
        result = cmp;
        return true;
      }
      return false;
    }
  );

  return result;
}

bool ArrayData::Equal(const ArrayData* ad1, const ArrayData* ad2) {
  return EqualHelper(ad1, ad2, false);
}

bool ArrayData::NotEqual(const ArrayData* ad1, const ArrayData* ad2) {
  return !EqualHelper(ad1, ad2, false);
}

bool ArrayData::Same(const ArrayData* ad1, const ArrayData* ad2) {
  return EqualHelper(ad1, ad2, true);
}

bool ArrayData::NotSame(const ArrayData* ad1, const ArrayData* ad2) {
  return !EqualHelper(ad1, ad2, true);
}

bool ArrayData::Lt(const ArrayData* ad1, const ArrayData* ad2) {
  return CompareHelper(ad1, ad2) < 0;
}

bool ArrayData::Lte(const ArrayData* ad1, const ArrayData* ad2) {
  return CompareHelper(ad1, ad2) <= 0;
}

bool ArrayData::Gt(const ArrayData* ad1, const ArrayData* ad2) {
  return 0 > CompareHelper(ad2, ad1); // Not symmetric; Order matters here.
}

bool ArrayData::Gte(const ArrayData* ad1, const ArrayData* ad2) {
  return 0 >= CompareHelper(ad2, ad1); // Not symmetric; Order matters here.
}

int64_t ArrayData::Compare(const ArrayData* ad1, const ArrayData* ad2) {
  return CompareHelper(ad1, ad2);
}

int ArrayData::compare(const ArrayData* v2) const {
  assertx(v2);

  if (isPHPArrayType()) {
    if (UNLIKELY(!v2->isPHPArrayType())) {
      if (UNLIKELY(checkHACCompare())) {
        raiseHackArrCompatArrHackArrCmp();
      }
      if (v2->isVecArrayType()) throw_vec_compare_exception();
      if (v2->isDictType()) throw_dict_compare_exception();
      if (v2->isKeysetType()) throw_keyset_compare_exception();
      not_reached();
    }
    return Compare(this, v2);
  }

  if (isVecArrayType()) {
    if (UNLIKELY(!v2->isVecArrayType())) {
      if (UNLIKELY(checkHACCompare() && v2->isPHPArrayType())) {
        raiseHackArrCompatArrHackArrCmp();
      }
      throw_vec_compare_exception();
    }
    assertx(isVecArrayKind() && v2->isVecArrayKind());
    return PackedArray::VecCmp(this, v2);
  }

  if (UNLIKELY(checkHACCompare() && v2->isPHPArrayType())) {
    raiseHackArrCompatArrHackArrCmp();
  }

  if (isDictType()) throw_dict_compare_exception();
  if (isKeysetType()) throw_keyset_compare_exception();

  not_reached();
}

bool ArrayData::equal(const ArrayData* v2, bool strict) const {
  assertx(v2);

  auto const mixed = [&]{
    if (UNLIKELY(checkHACCompare() && v2->isHackArrayType())) {
      raiseHackArrCompatArrHackArrCmp();
    }
    return false;
  };

  if (isPHPArrayType()) {
    if (UNLIKELY(!v2->isPHPArrayType())) return mixed();
    return strict ? Same(this, v2) : Equal(this, v2);
  }

  if (isVecArrayKind()) {
    if (UNLIKELY(!v2->isVecArrayKind())) return mixed();
    return strict
      ? PackedArray::VecSame(this, v2) : PackedArray::VecEqual(this, v2);
  }

  if (isDictKind()) {
    if (UNLIKELY(!v2->isDictKind())) return mixed();
    return strict
      ? MixedArray::DictSame(this, v2) : MixedArray::DictEqual(this, v2);
  }

  if (isKeysetKind()) {
    if (UNLIKELY(!v2->isKeysetKind())) return mixed();
    return strict ? SetArray::Same(this, v2) : SetArray::Equal(this, v2);
  }

  not_reached();
}

Variant ArrayData::reset() {
  setPosition(iter_begin());
  return m_pos != iter_end() ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::next() {
  // We call iter_advance() without checking if m_pos is the canonical invalid
  // position. This is okay, since all IterAdvance() impls handle this
  // correctly, but it means that EmptyArray::IterAdvance() is reachable.
  setPosition(iter_advance(m_pos));
  return m_pos != iter_end() ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::prev() {
  // We only call iter_rewind() if m_pos is not the canonical invalid position.
  // Thus, EmptyArray::IterRewind() is not reachable.
  auto pos_limit = iter_end();
  if (m_pos != pos_limit) {
    setPosition(iter_rewind(m_pos));
    if (m_pos != pos_limit) {
      return getValue(m_pos);
    }
  }
  return Variant(false);
}

Variant ArrayData::end() {
  setPosition(iter_last());
  return m_pos != iter_end() ? getValue(m_pos) : Variant(false);
}

Variant ArrayData::key() const {
  return m_pos != iter_end() ? getKey(m_pos) : uninit_null();
}

Variant ArrayData::value(int32_t pos) const {
  return pos != iter_end() ? getValue(pos) : Variant(false);
}

Variant ArrayData::current() const {
  return m_pos != iter_end() ? getValue(m_pos) : Variant(false);
}

const StaticString
  s_value("value"),
  s_key("key");

Variant ArrayData::each() {
  if (m_pos != iter_end()) {
    ArrayInit ret(4, ArrayInit::Mixed{});
    Variant key(getKey(m_pos));
    Variant value(getValue(m_pos));
    ret.set(1, value);
    ret.set(s_value, value);
    ret.set(0, key);
    ret.set(s_key, key);
    setPosition(iter_advance(m_pos));
    return ret.toVariant();
  }
  return Variant(false);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void ArrayData::getNotFound(int64_t k) const {
  assertx(kind() != kGlobalsKind);
  if (isHackArrayType()) throwOOBArrayKeyException(k, this);
  throwArrayIndexException(k, false);
}

void ArrayData::getNotFound(const StringData* k) const {
  assertx(kind() != kGlobalsKind);
  if (isVecArrayType()) throwInvalidArrayKeyException(k, this);
  if (isHackArrayType()) throwOOBArrayKeyException(k, this);
  throwArrayKeyException(k, false);
}

tv_rval ArrayData::getNotFound(int64_t k, bool error) const {
  if (error) getNotFound(k);
  return tv_rval::dummy();
}

tv_rval ArrayData::getNotFound(const StringData* k, bool error) const {
  if (error) getNotFound(k);
  return tv_rval::dummy();
}

const char* ArrayData::kindToString(ArrayKind kind) {
  std::array<const char*,9> names = {{
    "PackedKind",
    "MixedKind",
    "EmptyKind",
    "ApcKind",
    "GlobalsKind",
    "RecordKind",
    "DictKind",
    "VecKind",
    "KeysetKind"
  }};
  static_assert(names.size() == kNumKinds, "add new kinds here");
  return names[kind];
}

namespace {

////////////////////////////////////////////////////////////////////////////////

std::string describeKeyType(const TypedValue* tv) {
  switch (tv->m_type) {
  case KindOfUninit:
  case KindOfNull:             return "null";
  case KindOfBoolean:          return "bool";
  case KindOfInt64:            return "int";
  case KindOfDouble:           return "double";
  case KindOfPersistentString:
  case KindOfString:           return "string";
  case KindOfPersistentVec:
  case KindOfVec:              return "vec";
  case KindOfPersistentDict:
  case KindOfDict:             return "dict";
  case KindOfPersistentKeyset:
  case KindOfKeyset:           return "keyset";
  case KindOfPersistentDArray:
  case KindOfDArray:
    return UNLIKELY(RuntimeOption::EvalSpecializeDVArray) ? "darray" : "array";
  case KindOfPersistentVArray:
  case KindOfVArray:
    return UNLIKELY(RuntimeOption::EvalSpecializeDVArray) ? "varray" : "array";
  case KindOfPersistentArray:
  case KindOfArray:            return "array";
  case KindOfResource:
    return tv->m_data.pres->data()->o_getClassName().toCppString();

  case KindOfObject:
    return tv->m_data.pobj->getClassName().get()->toCppString();

  case KindOfRecord:
    return tv->m_data.prec->record()->name()->toCppString();

  case KindOfFunc:            return "func";
  case KindOfClass:           return "class";
  case KindOfClsMeth:         return "clsmeth";
  }
  not_reached();
}

std::string describeKeyValue(TypedValue tv) {
  switch (tv.m_type) {
  case KindOfPersistentString:
  case KindOfString:
    return folly::sformat("\"{}\"", tv.m_data.pstr->data());
  case KindOfInt64:
    return folly::to<std::string>(tv.m_data.num);
  case KindOfUninit:
  case KindOfNull:
  case KindOfBoolean:
  case KindOfDouble:
  case KindOfPersistentVec:
  case KindOfVec:
  case KindOfPersistentDict:
  case KindOfDict:
  case KindOfPersistentKeyset:
  case KindOfKeyset:
  case KindOfPersistentDArray:
  case KindOfDArray:
  case KindOfPersistentVArray:
  case KindOfVArray:
  case KindOfPersistentArray:
  case KindOfArray:
  case KindOfResource:
  case KindOfObject:
  case KindOfFunc:
  case KindOfClass:
  case KindOfClsMeth:
  case KindOfRecord:
    return "<invalid key type>";
  }
  not_reached();
}

////////////////////////////////////////////////////////////////////////////////

}

void throwInvalidArrayKeyException(const TypedValue* key, const ArrayData* ad) {
  std::pair<const char*, const char*> kind_type = [&]{
    if (ad->isVecArrayType()) return std::make_pair("vec", "int");
    if (ad->isDictType()) return std::make_pair("dict", "int or string");
    if (ad->isKeysetType()) return std::make_pair("keyset", "int or string");
    assertx(ad->isPHPArrayType());
    return std::make_pair("array", "int or string");
  }();
  SystemLib::throwInvalidArgumentExceptionObject(
    folly::sformat(
      "Invalid {} key: expected a key of type {}, {} given",
      kind_type.first, kind_type.second, describeKeyType(key)
    )
  );
}

void throwInvalidArrayKeyException(const StringData* key, const ArrayData* ad) {
  auto const tv = make_tv<KindOfString>(const_cast<StringData*>(key));
  throwInvalidArrayKeyException(&tv, ad);
}

void throwFalseyPromoteException(const char* type) {
  SystemLib::throwOutOfBoundsExceptionObject(
    folly::sformat("Promoting {} to array", type)
  );
}

void throwMissingElementException(const char* op) {
  SystemLib::throwOutOfBoundsExceptionObject(
    folly::sformat("{} on missing array element", op)
  );
}

void throwOOBArrayKeyException(TypedValue key, const ArrayData* ad) {
  const char* type = [&]{
    if (ad->isVecArrayType()) return "vec";
    if (ad->isDictType()) return "dict";
    if (ad->isKeysetType()) return "keyset";
    assertx(ad->isPHPArrayType());
    return "array";
  }();
  SystemLib::throwOutOfBoundsExceptionObject(
    folly::sformat(
      "Out of bounds {} access: invalid index {}",
      type, describeKeyValue(key)
    )
  );
}

void throwOOBArrayKeyException(int64_t key, const ArrayData* ad) {
  throwOOBArrayKeyException(make_tv<KindOfInt64>(key), ad);
}

void throwOOBArrayKeyException(const StringData* key, const ArrayData* ad) {
  throwOOBArrayKeyException(
    make_tv<KindOfString>(const_cast<StringData*>(key)),
    ad
  );
}

void throwInvalidKeysetOperation() {
  SystemLib::throwInvalidOperationExceptionObject(s_InvalidKeysetOperationMsg);
}

void throwInvalidAdditionException(const ArrayData* ad) {
  assertx(ad->isHackArrayType());
  const char* type = [&]{
    if (ad->isVecArrayType()) return "Vecs";
    if (ad->isDictType()) return "Dicts";
    if (ad->isKeysetType()) return "Keysets";
    not_reached();
  }();
  SystemLib::throwInvalidOperationExceptionObject(
    folly::sformat("{} do not support the + operator", type)
  );
}

void throwVecUnsetException() {
  SystemLib::throwInvalidOperationExceptionObject(s_VecUnsetMsg);
}

///////////////////////////////////////////////////////////////////////////////

void raiseHackArrCompatAdd() {
  raise_hac_array_plus_notice("Using + operator on arrays");
}

void raiseHackArrCompatArrHackArrCmp() {
  raise_hac_compare_notice(Strings::HACKARR_COMPAT_ARR_HACK_ARR_CMP);
}

void raiseHackArrCompatDVArrCmp(const ArrayData* ad1,
                                const ArrayData* ad2,
                                bool is_relational) {
  if (UNLIKELY(RID().getSuppressHACCompareNotices())) return;
  auto const type = [](const ArrayData* a) {
    if (a->isVArray()) return "varray";
    if (a->isDArray()) return "darray";
    return "array";
  };
  raise_hackarr_compat_notice(
    folly::sformat("Comparing {} and {}{}",
                   type(ad1),
                   type(ad2),
                   is_relational ? " relationally" : "")
  );
}

void raiseHackArrCompatHackArrBoolCmp() {
  if (!RuntimeOption::EvalHackArrCompatHackArrCmpNotices) return;
  raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_HACK_ARR_BOOL_CMP);
}

std::string makeHackArrCompatImplicitArrayKeyMsg(const TypedValue* key) {
  return folly::sformat(
    "Implicit conversion of {} to array key",
    describeKeyType(key)
  );
}

void raiseHackArrCompatImplicitArrayKey(const TypedValue* key) {
  raise_hac_array_key_cast_notice(makeHackArrCompatImplicitArrayKeyMsg(key));
}

///////////////////////////////////////////////////////////////////////////////

namespace arrprov_detail {

template<typename SrcArr>
ArrayData* tagArrProvImpl(ArrayData* ad, const SrcArr* src) {
  assertx(RuntimeOption::EvalArrayProvenance);
  assertx(ad->hasExactlyOneRef() || !ad->isRefCounted());

  if (!arrprov::arrayWantsTag(ad)) return ad;

  auto const do_tag = [] (ArrayData* ad, arrprov::Tag tag) {
    if (ad->isStatic()) return tagStaticArr(ad, tag);

    arrprov::setTag<arrprov::Mode::Emplace>(ad, tag);
    return ad;
  };

  if (src != nullptr) {
    if (auto const tag = arrprov::getTag(src)) return do_tag(ad, tag);
  }
  if (auto const tag = arrprov::tagFromPC()) return do_tag(ad, tag);

  return ad;
}

template ArrayData* tagArrProvImpl<ArrayData>(ArrayData*, const ArrayData*);
template ArrayData* tagArrProvImpl<APCArray>(ArrayData*, const APCArray*);

}

///////////////////////////////////////////////////////////////////////////////

}
