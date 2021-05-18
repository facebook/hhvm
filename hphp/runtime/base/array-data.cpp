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

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/server/memory-stats.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/reverse-data-map.h"

#include "hphp/util/exception.h"

#include "hphp/zend/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_InvalidKeysetOperationMsg{"Invalid operation on keyset"},
  s_ReadOnlyCollectionMutationMsg{"Read-Only collections cannot be mutated."},
  s_VarrayUnsetMsg{"varrays do not support unsetting non-end elements"},
  s_VecUnsetMsg{"Vecs do not support unsetting non-end elements"};
///////////////////////////////////////////////////////////////////////////////

__thread std::pair<const ArrayData*, size_t> s_cachedHash;

namespace {
static_assert(
  sizeof(ArrayData) == 16,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

size_t hashArrayPortion(const ArrayData* arr) {
  assertx(arr->isVanilla());

  auto hash = folly::hash::hash_128_to_64(arr->kind(), arr->isLegacyArray());

  IterateKV(
    arr,
    [&](TypedValue k, TypedValue v) {
      assertx(tvIsString(k) || tvIsInt(k));
      assertx(IMPLIES(tvIsString(k), val(k).pstr->isStatic()));
      hash = folly::hash::hash_combine(
        hash,
        dt_with_persistence(k.m_type),
        k.m_data.num,
        v.m_type
      );
      switch (v.m_type) {
        case KindOfNull:
          break;
        case KindOfBoolean:
        case KindOfInt64:
        case KindOfDouble:
        case KindOfPersistentString:
        case KindOfPersistentVec:
        case KindOfPersistentDict:
        case KindOfPersistentKeyset:
          hash = folly::hash::hash_combine(hash, v.m_data.num);
          break;
        case KindOfLazyClass:
          assertx(v.m_data.plazyclass.name()->isStatic());
          hash = folly::hash::hash_combine(
            hash,
            (uintptr_t)v.m_data.plazyclass.name()
          );
          break;
        case KindOfUninit:
        case KindOfString:
        case KindOfVec:
        case KindOfDict:
        case KindOfKeyset:
        case KindOfObject:
        case KindOfResource:
        case KindOfRFunc:
        case KindOfFunc:
        case KindOfClass:
        case KindOfClsMeth:
        case KindOfRClsMeth:
        case KindOfRecord:
          always_assert(false);
      }
    }
  );
  return hash;
}

bool compareArrayPortion(const ArrayData* ad1, const ArrayData* ad2) {
  assertx(ad1->isVanilla());
  assertx(ad2->isVanilla());

  if (ad1 == ad2) return true;
  if (ad1->kind() != ad2->kind()) return false;
  if (ad1->size() != ad2->size()) return false;
  if (ad1->isLegacyArray() != ad2->isLegacyArray()) return false;

  auto const check = [] (const TypedValue& tv1, const TypedValue& tv2) {
    if (tv1.m_type != tv2.m_type) {
      // String keys from arrays might be KindOfString, even when
      // the StringData is static.
      if (!isStringType(tv1.m_type) || !isStringType(tv2.m_type)) {
        return false;
      }
      assertx(tv1.m_data.pstr->isStatic());
      assertx(tv2.m_data.pstr->isStatic());
    }
    if (isNullType(tv1.m_type)) return true;
    return tv1.m_data.num == tv2.m_data.num;
  };

  auto equal = true;
  ArrayIter iter2{ad2};
  IterateKV(
    ad1,
    [&](TypedValue k, TypedValue v) {
      if (!check(k, iter2.nvFirst()) || !check(v, iter2.secondVal())) {
        equal = false;
        return true;
      }
      ++iter2;
      return false;
    }
  );
  return equal;
}

struct ScalarHash {
  size_t operator()(ArrayData* arr) const {
    return s_cachedHash.first == arr
      ? s_cachedHash.second
      : hashArrayPortion(arr);
  }
  bool operator()(ArrayData* arr1, ArrayData* arr2) const {
    return compareArrayPortion(arr1, arr2);
  }
};

using ArrayDataMap =
  tbb::concurrent_unordered_set<ArrayData*, ScalarHash, ScalarHash>;
ArrayDataMap s_arrayDataMap;

}

size_t loadedStaticArrayCount() {
  return s_arrayDataMap.size();
}

///////////////////////////////////////////////////////////////////////////////

void ArrayData::GetScalarArray(ArrayData** parr) {
  auto const base = *parr;
  if (base->isStatic()) return;

  auto const arr = [&]{
    if (base->isVanilla()) return base;
    *parr = BespokeArray::ToVanilla(base, "ArrayData::GetScalarArray");
    decRefArr(base);
    return *parr;
  }();

  auto const replace = [&] (ArrayData* rep) {
    *parr = rep;
    decRefArr(arr);
    s_cachedHash.first = nullptr;
  };

  if (arr->empty()) {
    return replace([&]{
      auto const legacy = arr->isLegacyArray();
      switch (arr->toDataType()) {
        case KindOfVec:    return ArrayData::CreateVec(legacy);
        case KindOfDict:   return ArrayData::CreateDict(legacy);
        case KindOfKeyset: return ArrayData::CreateKeyset();
        default: always_assert(false);
      }
    }());
  }

  arr->onSetEvalScalar();

  s_cachedHash.first = arr;
  s_cachedHash.second = hashArrayPortion(arr);

  auto const lookup = [&] (ArrayData* a) -> ArrayData* {
    if (auto const it = s_arrayDataMap.find(a);
        it != s_arrayDataMap.end()) {
      return *it;
    }
    return nullptr;
  };

  if (auto const a = lookup(arr)) return replace(a);

  static std::array<std::mutex, 128> s_mutexes;
  std::lock_guard<std::mutex> g{
    s_mutexes[s_cachedHash.second % s_mutexes.size()]
  };

  if (auto const a = lookup(arr)) return replace(a);

  // We should clear the sampled bit in the new static array regardless of
  // whether the input array was sampled, because specializing the input is
  // not sufficient to specialize this new static array.
  auto const ad = arr->copyStatic();
  ad->m_aux16 &= ~kSampledArray;
  assertx(ad->isStatic());

  // TODO(T68458896): allocSize rounds up to size class, which we shouldn't do.
  MemoryStats::LogAlloc(AllocKind::StaticArray, allocSize(ad));
  if (RuntimeOption::EvalEnableReverseDataMap) {
    data_map::register_start(ad);
  }

  s_cachedHash.first = ad;
  assertx(hashArrayPortion(ad) == s_cachedHash.second);

  auto const DEBUG_ONLY inserted = s_arrayDataMap.insert(ad).second;
  assertx(inserted);
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

#define DISPATCH(entry)                           \
  { PackedArray::entry,      /* vanilla vec */    \
    BespokeArray::entry,     /* bespoke vec */    \
    MixedArray::entry,       /* vanilla dict */   \
    BespokeArray::entry,     /* bespoke dict */   \
    SetArray::entry,         /* vanilla keyset */ \
    BespokeArray::entry,     /* bespoke keyset */ \
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
   *   Free the memory associated with a refcounted array. Generally called
   *   when the reference count on the array drops to zero.
   */
  DISPATCH(Release)

  /*
   * void ReleaseUncounted(ArrayData*)
   *
   *   Free the memory associated with an uncounted array. Generally called
   *   when the reference count on the array drops to "uncounted zero".
   */
  DISPATCH(ReleaseUncounted)

  /*
   * TypedValue NvGetInt(const ArrayData*, int64_t key)
   * TypedValue NvGetStr(const ArrayData*, const StringData*)
   *
   *   Lookup a value in an array using an int or string key. Returns Uninit if
   *   the key is not in the array. This method does not inc-ref the result.
   */
  DISPATCH(NvGetInt)
  DISPATCH(NvGetStr)

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
  DISPATCH(SetIntMove)

  /*
   * ArrayData* SetStr(ArrayData*, StringData*, TypedValue v)
   *
   *   Set a value in the array for a string key, with copies / escalation.
   *   SetStrMove is equivalent to SetStr, followed by a dec-ref of the value,
   *   followed by a dec-ref of the old array (if it was copied or escalated).
   */
  DISPATCH(SetStrMove)

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
   * arr_lval LvalInt(ArrayData*, int64_t k)
   * arr_lval LvalStr(ArrayData*, StringData* key)
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
   * ArrayData* CopyStatic(const ArrayData*)
   *
   *   Copy an array, allocating the new array with malloc() instead
   *   of from the request local allocator.  This function does
   *   guarantee the returned array is a new copy, but it may throw a
   *   fatal error if this cannot be accomplished.
   */
  DISPATCH(CopyStatic)

  /*
   * ArrayData* Append(ArrayData*, TypedValue v);
   *
   *   Append a new value to the array, with the next available integer key,
   *   copying or escalating as necessary. If there is no available integer
   *   key, no value is appended, but this method may still copy the array.
   *
   *   AppendMove is equivalent to calling Append, dec-ref-ing the value,
   *   and (if copy or escalation was needed), dec-ref-ing the old array.
   */
  DISPATCH(AppendMove)

  /*
   * ArrayData* Pop(ArrayData*, Variant& value);
   *
   *   Remove the last element from the array and assign it to `value'.  This
   *   function may return a new array if it decided to COW due to
   *   cowCheck().
   */
  DISPATCH(Pop)

  /*
   * void OnSetEvalScalar(ArrayData*)
   *
   *   Go through an array and call Variant::setEvalScalar on each
   *   value, and make all string keys into static strings.
   */
  DISPATCH(OnSetEvalScalar)

  /*
   * void MakeUncounted(ArrayData*, DataWalker::PointerMap*, bool hasApcTv)
   *
   *   Return a copy of the array where all its contents are converted to
   *   uncounted values. The caller must confirm that its contents can all be
   *   converted, e.g. by using DataWalker. If hasApcTv is true, this method
   *   will colocate an ApcTypedValue right before the array.
   */
  DISPATCH(MakeUncounted)
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

///////////////////////////////////////////////////////////////////////////////
// reads

ALWAYS_INLINE
bool ArrayData::EqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                            bool strict) {
  if (ad1 == ad2) return true;
  if (ad1->size() != ad2->size()) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  if (strict) {
    for (ArrayIter iter1{ad1}, iter2{ad2}; iter1; ++iter1, ++iter2) {
      assertx(iter2);
      if (!HPHP::same(iter1.first(), iter2.first()) ||
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
        auto const v2 = ad2->get(k);
        if (!v2.is_init() || !tvEqual(v, v2)) {
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
      auto const v2 = ad2->get(k);
      if (!v2.is_init()) {
        result = 1;
        return true;
      }
      auto const cmp = tvCompare(v, v2);
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
  return CompareHelper(ad1, ad2) > 0;
}

bool ArrayData::Gte(const ArrayData* ad1, const ArrayData* ad2) {
  return CompareHelper(ad1, ad2) >= 0;
}

int64_t ArrayData::Compare(const ArrayData* ad1, const ArrayData* ad2) {
  return CompareHelper(ad1, ad2);
}

bool ArrayData::same(const ArrayData* v2) const {
  assertx(v2);

  if (toDataType() != v2->toDataType()) {
    if (UNLIKELY(checkHACCompare())) {
      raiseHackArrCompatArrHackArrCmp();
    }
    return false;
  }

  if (!bothVanilla(this, v2)) return Same(this, v2);
  if (isVanillaVec())  return PackedArray::VecSame(this, v2);
  if (isVanillaDict()) return MixedArray::DictSame(this, v2);
  return SetArray::Same(this, v2);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

void ArrayData::getNotFound(int64_t k) const {
  throwOOBArrayKeyException(k, this);
}

void ArrayData::getNotFound(const StringData* k) const {
  if (isVecType()) throwInvalidArrayKeyException(k, this);
  throwOOBArrayKeyException(k, this);
}

const char* ArrayData::kindToString(ArrayKind kind) {
  std::array<const char*, 6> names = {{
    "VecKind",
    "BespokeVecKind",
    "DictKind",
    "BespokeDictKind",
    "KeysetKind",
    "BespokeKeysetKind",
  }};
  static_assert(names.size() == kNumKinds, "add new kinds here");
  return names[kind];
}

namespace {

////////////////////////////////////////////////////////////////////////////////

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
  case KindOfResource:
  case KindOfObject:
  case KindOfRFunc:
  case KindOfFunc:
  case KindOfClass:
  case KindOfLazyClass:
  case KindOfClsMeth:
  case KindOfRClsMeth:
  case KindOfRecord:
    return "<invalid key type>";
  }
  not_reached();
}

////////////////////////////////////////////////////////////////////////////////

}

void throwInvalidArrayKeyException(const TypedValue* key, const ArrayData* ad) {
  std::pair<const char*, const char*> kind_type = [&]{
    if (ad->isVecType()) return std::make_pair("vec", "int");
    if (ad->isDictType()) return std::make_pair("dict", "int or string");
    assertx(ad->isKeysetType());
    return std::make_pair("keyset", "int or string");
  }();
  SystemLib::throwInvalidArgumentExceptionObject(
    folly::sformat(
      "Invalid {} key: expected a key of type {}, {} given",
      kind_type.first, kind_type.second, describe_actual_type(key)
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

void throwOOBArrayKeyException(TypedValue key, const ArrayData* ad) {
  SystemLib::throwOutOfBoundsExceptionObject(
    folly::sformat(
      "Out of bounds {} access: invalid index {}",
      getDataTypeString(ad->toDataType(), ad->isLegacyArray()).data(),
      describeKeyValue(key)
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

void throwReadOnlyCollectionMutation() {
  SystemLib::throwInvalidOperationExceptionObject(s_ReadOnlyCollectionMutationMsg);
}

void throwVarrayUnsetException() {
  SystemLib::throwInvalidOperationExceptionObject(s_VarrayUnsetMsg);
}

void throwVecUnsetException() {
  SystemLib::throwInvalidOperationExceptionObject(s_VecUnsetMsg);
}

///////////////////////////////////////////////////////////////////////////////

void raiseHackArrCompatArrHackArrCmp() {
  raise_hac_compare_notice(Strings::HACKARR_COMPAT_ARR_HACK_ARR_CMP);
}

///////////////////////////////////////////////////////////////////////////////

ArrayData* ArrayData::toDictIntishCast(bool copy) {
  auto const base = toDict(copy);
  if (isVecType()) return base;

  // Check if we need to intish-cast any string keys.
  int64_t i;
  auto intish = false;
  IterateKV(base, [&](TypedValue k, TypedValue) {
    return intish |= tvIsString(k) && val(k).pstr->isStrictlyInteger(i);
  });
  if (!intish) return base;

  if (RO::EvalHackArrCompatIntishCastNotices) {
    raise_hackarr_compat_notice("triggered IntishCast for toDictIntishCast");
  }

  // Create a new, plain PHP array with the casted keys.
  auto result = MixedArrayInit(base->size());
  IterateKV(base, [&](TypedValue k, TypedValue v) {
    result.setUnknownKey<IntishCast::Cast>(tvAsCVarRef(&k), tvAsCVarRef(&v));
  });
  if (base != this) decRefArr(base);
  auto ad = result.create();
  if (!isKeysetType()) return ad;

  // When keysets are intish-casted, we cast the values, too. Do so in place.
  // This case is rare, so we keep the extra checks out of the loop above.
  assertx(ad->hasExactlyOneRef());
  auto elm = MixedArray::asMixed(ad)->data();
  for (auto const end = elm + ad->size(); elm != end; elm++) {
    if (tvIsString(elm->data) && elm->hasIntKey()) {
      decRefStr(val(elm->data).pstr);
      tvCopy(make_tv<KindOfInt64>(elm->intKey()), elm->data);
    }
  }
  return ad;
}

bool ArrayData::IntishCastKey(const StringData* key, int64_t& i) {
  if (key->isStrictlyInteger(i)) {
    if (RO::EvalHackArrCompatIntishCastNotices) {
      raise_hackarr_compat_notice("triggered IntishCast for set");
    }
    return true;
  }
  return false;
}

ArrayData* ArrayData::setLegacyArray(bool copy, bool legacy) {
  assertx(IMPLIES(cowCheck(), copy));
  assertx(isDictType() || isVecType());

  if (legacy == isLegacyArray()) return this;

  if (!isVanilla()) return BespokeArray::SetLegacyArray(this, copy, legacy);

  auto const ad = [&]{
    if (!copy) return this;
    return isVanillaVec() ? PackedArray::Copy(this) : MixedArray::Copy(this);
  }();
  ad->setLegacyArrayInPlace(legacy);
  return ad;
}

void ArrayData::setLegacyArrayInPlace(bool legacy) {
  assertx(hasExactlyOneRef());
  if (legacy) {
    m_aux16 |= kLegacyArray;
  } else {
    m_aux16 &= ~kLegacyArray;
  }
}

///////////////////////////////////////////////////////////////////////////////

ArrayData* ArrayData::toVec(bool copy) {
  assertx(IMPLIES(cowCheck(), copy));
  if (isVecType()) return this;

  if (empty()) return ArrayData::CreateVec();
  VecInit init{size()};
  IterateV(this, [&](auto v) { init.append(v); });
  return init.create();
}

ArrayData* ArrayData::toDict(bool copy) {
  assertx(IMPLIES(cowCheck(), copy));
  if (isDictType()) return this;

  if (empty()) return ArrayData::CreateDict();
  DictInit init{size()};
  IterateKV(this, [&](auto k, auto v) { init.setValidKey(k, v); });
  return init.create();
}

ArrayData* ArrayData::toKeyset(bool copy) {
  assertx(IMPLIES(cowCheck(), copy));
  if (isKeysetType()) return this;
  if (empty()) return ArrayData::CreateKeyset();
  KeysetInit init{size()};
  IterateV(this, [&](auto v) { init.add(v); });
  return init.create();
}

///////////////////////////////////////////////////////////////////////////////

}
