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
#include <tbb/concurrent_hash_map.h>

#include "hphp/util/exception.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/empty-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/zend/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static_assert(
  sizeof(ArrayData) == 16,
  "Performance is sensitive to sizeof(ArrayData)."
  " Make sure you changed it with good reason and then update this assert.");

using ArrayDataMap = tbb::concurrent_hash_map<ArrayData::ScalarArrayKey,
                                              ArrayData*,
                                              ArrayData::ScalarHash>;
static ArrayDataMap s_arrayDataMap;

const StaticString s_InvalidKeysetOperationMsg("Invalid operation on keyset");
const StaticString s_VecUnsetMsg("Vecs do not support unsetting non-end elements");

ArrayData::ScalarArrayKey ArrayData::GetScalarArrayKey(const char* str,
                                                       size_t sz) {
  return MD5(string_md5(folly::StringPiece{str, sz}));
}

ArrayData::ScalarArrayKey ArrayData::GetScalarArrayKey(ArrayData* arr) {
  // FIXME: this function uses a lot of temporary memory, reimplement it without
  //        serialization; do not count its memory towards OOM in meanwhile
  MemoryManager::SuppressOOM so(MM());

  VariableSerializer vs(VariableSerializer::Type::Serialize);
  auto s = vs.serializeValue(VarNR(arr), false /* limit */);
  return GetScalarArrayKey(s.data(), s.size());
}

ArrayData* ArrayData::GetScalarArray(ArrayData* arr) {
  if (arr->empty()) {
    if (arr->isVecArray()) return staticEmptyVecArray();
    if (arr->isDict()) return staticEmptyDictArray();
    if (arr->isKeyset()) return staticEmptyKeysetArray();
    return staticEmptyArray();
  }
  auto key = GetScalarArrayKey(arr);
  return GetScalarArray(arr, key);
}

ArrayData* ArrayData::GetScalarArray(ArrayData* arr,
                                     const ScalarArrayKey& key) {
  if (arr->empty()) {
    if (arr->isVecArray()) return staticEmptyVecArray();
    if (arr->isDict()) return staticEmptyDictArray();
    if (arr->isKeyset()) return staticEmptyKeysetArray();
    return staticEmptyArray();
  }
  assert(key == GetScalarArrayKey(arr));

  ArrayDataMap::accessor acc;
  if (s_arrayDataMap.insert(acc, key)) {
    ArrayData* ad;
    if (arr->isVectorData() && !arr->hasPackedLayout() && !arr->isDict() &&
        !arr->isKeyset()) {
      ad = PackedArray::ConvertStatic(arr);
    } else {
      ad = arr->copyStatic();
    }
    assert(ad->isStatic());
    ad->onSetEvalScalar();
    acc->second = ad;
  }
  return acc->second;
}

//////////////////////////////////////////////////////////////////////

static ArrayData*
ZSetIntThrow(ArrayData* /*ad*/, int64_t /*k*/, RefData* /*v*/) {
  raise_fatal_error("Unimplemented ArrayData::ZSetInt");
}

static ArrayData*
ZSetStrThrow(ArrayData* /*ad*/, StringData* /*k*/, RefData* /*v*/) {
  raise_fatal_error("Unimplemented ArrayData::ZSetStr");
}

static ArrayData*
ZAppendThrow(ArrayData* /*ad*/, RefData* /*v*/, int64_t* /*key_ptr*/) {
  raise_fatal_error("Unimplemented ArrayData::ZAppend");
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
    ProxyArray::entry,                          \
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
   * member_rval::ptr_u NvGetInt(const ArrayData*, int64_t key)
   *
   *   Lookup a value in an array using an integer key.  Returns nullptr if the
   *   key is not in the array.  Must not throw if key isn't present.
   */
  DISPATCH(NvGetInt)

  /*
   * member_rval::ptr_u NvTryGetInt(const ArrayData*, int64_t key)
   *
   *   Lookup a value in an array using an integer key.  Either throws or
   *   returns nullptr if the key is not in the array.
   */
  DISPATCH(NvTryGetInt)

  /*
   * member_rval::ptr_u NvGetStr(const ArrayData*, const StringData*)
   *
   *   Lookup a value in an array using a string key.  The string key must not
   *   be an integer-like string.  Returns nullptr if the key is not in the
   *   array.
   */
  DISPATCH(NvGetStr)

  /*
   * member_rval::ptr_u NvTryGetStr(const ArrayData*, const StringData*)
   *
   *   Lookup a value in an array using a string key.  Either throws or returns
   *   nullptr if the key is not in the array.
   */
  DISPATCH(NvTryGetStr)

  /*
   * Cell NvGetKey(const ArrayData*, ssize_t pos)
   *
   *   Look up the key for an array position.  `pos' must be a valid
   *   position for this array.
   */
  DISPATCH(NvGetKey)

  /*
   * ArrayData* SetInt(ArrayData*, int64_t key, Cell v, bool copy)
   *
   *   Set a value in the array for an integer key.  This function has
   *   copy/grow semantics.
   */
  DISPATCH(SetInt)

  /*
   * ArrayData* SetStr(ArrayData*, StringData*, Cell v, bool copy)
   *
   *   Set a value in the array for a string key.  The string must not
   *   be an integer-like string.  This function has copy/grow
   *   semantics.
   */
  DISPATCH(SetStr)

  /*
   * size_t Vsize(const ArrayData*)
   *
   *   This entry point essentially is only for GlobalsArray and ProxyArray;
   *   all the other cases are not_reached().
   *
   *   Because of particulars of how GlobalsArray works,
   *   determining the size of the array is an O(N) operation---we set
   *   the size field in the generic ArrayData header to -1 in that
   *   case and dispatch through this entry point.  ProxyArray also
   *   always involves virtual size, because of the possibility that
   *   it could be proxying a GlobalsArray.
   */
  DISPATCH(Vsize)

  /*
   * member_rval::ptr_u GetValueRef(const ArrayData*, ssize_t pos)
   *
   *   Return a reference to the value at an iterator position.  `pos' must be
   *   a valid position for this array.
   */
  DISPATCH(GetValueRef)

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
   * member_lval LvalInt(ArrayData*, int64_t k, bool copy)
   * member_lval LvalIntRef(ArrayData*, int64_t k, bool copy)
   *
   *   Look up a value in the array by the supplied integer key, creating it as
   *   a KindOfNull if it doesn't exist, and return a reference to it.  Use the
   *   ref variant if the retrieved value will be boxed.  This function has
   *   copy/grow semantics.
   */
  DISPATCH(LvalInt)
  DISPATCH(LvalIntRef)

  /*
   * member_lval LvalStr(ArrayData*, StringData* key, bool copy)
   * member_lval LvalStrRef(ArrayData*, StringData* key, bool copy)
   *
   *   Look up a value in the array by the supplied string key, creating it as
   *   a KindOfNull if it doesn't exist, and return a reference to it.  The
   *   string `key' may not be an integer-like string.  Use the ref variant if
   *   the retrieved value will be boxed.  This function has copy/grow
   *   semantics.
   */
  DISPATCH(LvalStr)
  DISPATCH(LvalStrRef)

  /*
   * member_lval LvalNew(ArrayData*, bool copy)
   * member_lval LvalNewRef(ArrayData*, bool copy)
   *
   *   Insert a new null value in the array at the next available integer key,
   *   and return a reference to it.  In the case that there is no next
   *   available integer key, this function returns a reference to the
   *   lvalBlackHole.  Use the ref variant if the retrieved value will be
   *   boxed.  This function has copy/grow semantics.
   */
  DISPATCH(LvalNew)
  DISPATCH(LvalNewRef)

  /*
   * ArrayData* SetRefInt(ArrayData*, int64_t key, Variant& v, bool copy)
   *
   *   Binding set with an integer key.  Box `v' if it is not already
   *   boxed, and then insert a KindOfRef that points to v's RefData.
   *   This function has copy/grow semantics.
   */
  DISPATCH(SetRefInt)

  /*
   * ArrayData* SetRefStr(ArrayData*, StringData* key, Variant& v, bool copy)
   *
   *  Binding set with a string key.  The string `key' must not be an
   *  integer-like string.  Box `v' if it is not already boxed, and
   *  then insert a KindOfRef that points to v's RefData.  This
   *  function has copy/grow semantics.
   */
  DISPATCH(SetRefStr)

  /*
   * ArrayData* AddInt(ArrayData*, int64_t key, Cell, bool copy)
   * ArrayData* AddStr(ArrayData*, StringData* key, Cell, bool copy)
   *
   *   These functions have the same effects as SetInt and SetStr,
   *   respectively, except that the array may assume that it does not
   *   already contain a value for the key `key' if it can make the
   *   operation more efficient.
   */
  DISPATCH(SetInt)
  DISPATCH(SetStr)

  /*
   * ArrayData* RemoveInt(ArrayData*, int64_t key, bool copy)
   *
   *   Remove an array element with an integer key.  If there was no
   *   entry for that element, this function does not remove it, and
   *   may or may not cow.  This function has copy/grow semantics.
   */
  DISPATCH(RemoveInt)

  /*
   * ArrayData* RemoveStr(ArrayData*, const StringData*, bool copy)
   *
   *   Remove an array element with a string key.  If there was no
   *   entry for that element, this function does not remove it, and
   *   may or may not cow.  This function has copy/grow semantics.
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
   * bool ValidMArrayIter(const ArrayData*, const MArrayIter& fp)
   *
   *    Returns whether a given MArrayIter is pointing at a valid
   *    position for this array.  This should return false if the
   *    MArrayIter is in the reset flag state.
   *
   *    This function may not be called without first calling
   *    Escalate.
   *
   *    Pre: fp.getContainer() == ad
   */
  DISPATCH(ValidMArrayIter)

  /*
   * bool AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp)
   *
   *   Advance a mutable array iterator to the next position.
   *
   *   This function may not be called without first calling Escalate.
   *
   *   Pre: fp.getContainer() == ad
   */
  DISPATCH(AdvanceMArrayIter)

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
   * ArrayData* Append(ArrayData*, Cell v, bool copy)
   *
   *   Append a new value to the array, with the next available
   *   integer key.  If there is no next available integer key, no
   *   value is appended.  This function has copy/grow semantics.  The
   *   value must not be KindOfUninit.
   */
  DISPATCH(Append)

  /*
   * ArrayData* AppendRef(ArrayData*, Variant& v, bool copy)
   *
   *   Binding append.  This function appends a new KindOfRef to the
   *   array with the next available integer key, boxes v if it is not
   *   already boxed, and points the new value to the same RefData.
   *   If there is no next available integer key, this function does
   *   not append a value.  This function has copy/grow semantics.
   */
  DISPATCH(AppendRef)

  /*
   * ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy)
   *
   *   "With ref" append.  This function appends a new value to the
   *   array with the next available integer key, if there is a next
   *   available integer key.  It either sets the value to `v', or
   *   binds the value to `v', depending on whether `v' is "observably
   *   referenced"---i.e. if `v' is already KindOfRef and
   *   RefData::isReferenced is true.
   */
  DISPATCH(AppendWithRef)

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
   * ArrayData* Prepend(ArrayData*, Cell v, bool copy)
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
   * ArrayData* Escalate(const ArrayData*)
   *
   *   Arrays must be given a chance to 'escalate' to more general
   *   kinds prior to some unusual operations.  The operations that
   *   are only legal after a call to Escalate are:
   *
   *      - ValidMArrayIter
   *      - AdvanceMArrayIter
   */
  DISPATCH(Escalate)

  /*
   * ArrayData* ZSet{Int,Str}
   * ArrayData* ZAppend
   *
   *   These functions are part of the zend compat layer but their
   *   effects currently aren't documented.
   */
  {
    &PackedArray::ZSetInt,
    &MixedArray::ZSetInt,
    &ZSetIntThrow,
    &ZSetIntThrow,
    &ZSetIntThrow,
    &ProxyArray::ZSetInt,
    &MixedArray::ZSetInt,
    &ZSetIntThrow,
    &ZSetIntThrow,
  },

  {
    &PackedArray::ZSetStr,
    &MixedArray::ZSetStr,
    &ZSetStrThrow,
    &ZSetStrThrow,
    &ZSetStrThrow,
    &ProxyArray::ZSetStr,
    &MixedArray::ZSetStr,
    &ZSetStrThrow,
    &ZSetStrThrow,
  },

  {
    &PackedArray::ZAppend,
    &MixedArray::ZAppend,
    &ZAppendThrow,
    &ZAppendThrow,
    &ZAppendThrow,
    &ProxyArray::ZAppend,
    &MixedArray::ZAppend,
    &ZAppendThrow,
    &ZAppendThrow,
  },

   /*
   * ArrayData* ToPHPArray(ArrayData*, bool)
   *
   *   Convert to a PHP array. If already a PHP array, it will be returned
   *   unchange (without copying). If copy is false, it may be converted in
   *   place.
   */
  DISPATCH(ToPHPArray)

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
   * packed (or empty) array, discarding keys. If already a packed or empty
   * array, it will be returned unchanged (without copying). If copy is false,
   * it may be converted in place.
   */
  DISPATCH(ToVArray)
};

#undef DISPATCH

///////////////////////////////////////////////////////////////////////////////

// In general, arrays can contain int-valued-strings, even though plain array
// access converts them to integers.  non-int-string assertions should go
// upstream of the ArrayData api.

bool ArrayData::IsValidKey(Cell k) {
  return isIntType(k.m_type) ||
        (isStringType(k.m_type) && IsValidKey(k.m_data.pstr));
}

bool ArrayData::IsValidKey(const Variant& k) {
  return IsValidKey(*k.asCell());
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
  if (debug) {
    DEBUG_ONLY auto const k = tvToCell(name);
    DEBUG_ONLY int64_t unused;
    assertx(isStringType(k.m_type)
              ? !k.m_data.pstr->isStrictlyInteger(unused)
              : isIntType(k.m_type));
  }

  ArrayInit init(1, ArrayInit::Map{});
  init.setValidKey(name, value);
  return init.create();
}

ArrayData* ArrayData::CreateRef(Variant& value) {
  PackedArrayInit pai(1);
  pai.appendRef(value);
  return pai.create();
}

ArrayData* ArrayData::CreateRef(TypedValue name, Variant& value) {
  if (debug) {
    DEBUG_ONLY auto const k = tvToCell(name);
    DEBUG_ONLY int64_t unused;
    assertx(isStringType(k.m_type)
              ? !k.m_data.pstr->isStrictlyInteger(unused)
              : isIntType(k.m_type));
  }

  ArrayInit init(1, ArrayInit::Map{});
  init.setRef(name, value, true);
  return init.create();
}

///////////////////////////////////////////////////////////////////////////////
// reads

ALWAYS_INLINE
bool ArrayData::EqualHelper(const ArrayData* ad1, const ArrayData* ad2,
                            bool strict) {
  assert(ad1->isPHPArray());
  assert(ad2->isPHPArray());

  if (ad1 == ad2) return true;
  if (ad1->size() != ad2->size()) return false;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  if (strict) {
    for (ArrayIter iter1{ad1}, iter2{ad2}; iter1; ++iter1, ++iter2) {
      assert(iter2);
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
      [&](Cell k, TypedValue v) {
        if (!ad2->exists(k) || !tvEqual(v, *ad2->get(k).asTypedValue())) {
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
  assert(ad1->isPHPArray());
  assert(ad2->isPHPArray());

  auto const size1 = ad1->size();
  auto const size2 = ad2->size();
  if (size1 < size2) return -1;
  if (size1 > size2) return 1;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  int result = 0;
  IterateKV(
    ad1,
    [&](Cell k, TypedValue v) {
      if (!ad2->exists(k)) {
        result = 1;
        return true;
      }
      auto const cmp = tvCompare(v, *ad2->get(k).asTypedValue());
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
  assert(v2);

  if (isPHPArray()) {
    if (UNLIKELY(!v2->isPHPArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices)) {
        raiseHackArrCompatArrMixedCmp();
      }
      if (v2->isVecArray()) throw_vec_compare_exception();
      if (v2->isDict()) throw_dict_compare_exception();
      if (v2->isKeyset()) throw_keyset_compare_exception();
      not_reached();
    }
    return Compare(this, v2);
  }

  if (isVecArray()) {
    if (UNLIKELY(!v2->isVecArray())) {
      if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                   v2->isPHPArray())) {
        raiseHackArrCompatArrMixedCmp();
      }
      throw_vec_compare_exception();
    }
    return PackedArray::VecCmp(this, v2);
  }

  if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
               v2->isPHPArray())) {
    raiseHackArrCompatArrMixedCmp();
  }

  if (isDict()) throw_dict_compare_exception();
  if (isKeyset()) throw_keyset_compare_exception();

  not_reached();
}

bool ArrayData::equal(const ArrayData* v2, bool strict) const {
  assert(v2);

  auto const mixed = [&]{
    if (UNLIKELY(RuntimeOption::EvalHackArrCompatNotices &&
                 v2->isHackArray())) {
      raiseHackArrCompatArrMixedCmp();
    }
    return false;
  };

  if (isPHPArray()) {
    if (UNLIKELY(!v2->isPHPArray())) return mixed();
    return strict ? Same(this, v2) : Equal(this, v2);
  }

  if (isVecArray()) {
    if (UNLIKELY(!v2->isVecArray())) return mixed();
    return strict
      ? PackedArray::VecSame(this, v2) : PackedArray::VecEqual(this, v2);
  }

  if (isDict()) {
    if (UNLIKELY(!v2->isDict())) return mixed();
    return strict
      ? MixedArray::DictSame(this, v2) : MixedArray::DictEqual(this, v2);
  }

  if (isKeyset()) {
    if (UNLIKELY(!v2->isKeyset())) return mixed();
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

Variant ArrayData::value(int32_t &pos) const {
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

const Variant& ArrayData::getNotFound(int64_t k) {
  raise_notice("Undefined index: %" PRId64, k);
  return uninit_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  return uninit_variant;
}

const Variant& ArrayData::getNotFound(int64_t k, bool error) const {
  return error && kind() != kGlobalsKind ? getNotFound(k) :
         uninit_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k, bool error) const {
  return error && kind() != kGlobalsKind ? getNotFound(k) :
         uninit_variant;
}

const char* ArrayData::kindToString(ArrayKind kind) {
  std::array<const char*,9> names = {{
    "PackedKind",
    "MixedKind",
    "EmptyKind",
    "ApcKind",
    "GlobalsKind",
    "ProxyKind",
    "DictKind",
    "VecKind",
    "KeysetKind"
  }};
  static_assert(names.size() == kNumKinds, "add new kinds here");
  return names[kind];
}

namespace {

////////////////////////////////////////////////////////////////////////////////

const char* describeKeyType(const TypedValue* tv) {
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
  case KindOfPersistentArray:
  case KindOfArray:            return "array";
  case KindOfResource:
    return tv->m_data.pres->data()->o_getClassName().c_str();

  case KindOfObject:
    return tv->m_data.pobj->getClassName().c_str();

  case KindOfRef:
    return describeKeyType(tv->m_data.pref->var()->asTypedValue());
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
  case KindOfRef:
    return describeKeyValue(*tv.m_data.pref->var()->asTypedValue());
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
  case KindOfPersistentArray:
  case KindOfArray:
  case KindOfResource:
  case KindOfObject:
    return "<invalid key type>";
  }
  not_reached();
}

////////////////////////////////////////////////////////////////////////////////

}

void throwInvalidArrayKeyException(const TypedValue* key, const ArrayData* ad) {
  std::pair<const char*, const char*> kind_type = [&]{
    if (ad->isVecArray()) return std::make_pair("vec", "int");
    if (ad->isDict()) return std::make_pair("dict", "int or string");
    if (ad->isKeyset()) return std::make_pair("keyset", "int or string");
    assertx(ad->isPHPArray());
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

void throwOOBArrayKeyException(TypedValue key, const ArrayData* ad) {
  const char* type = [&]{
    if (ad->isVecArray()) return "vec";
    if (ad->isDict()) return "dict";
    if (ad->isKeyset()) return "keyset";
    assertx(ad->isPHPArray());
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

void throwRefInvalidArrayValueException(const ArrayData* ad) {
  assertx(ad->isHackArray());
  const char* type = [&]{
    if (ad->isVecArray()) return "Vecs";
    if (ad->isDict()) return "Dicts";
    if (ad->isKeyset()) return "Keysets";
    not_reached();
  }();
  SystemLib::throwInvalidArgumentExceptionObject(
    folly::sformat("{} cannot contain references", type)
  );
}

void throwRefInvalidArrayValueException(const Array& arr) {
  throwRefInvalidArrayValueException(arr.get());
}

void throwInvalidKeysetOperation() {
  SystemLib::throwInvalidOperationExceptionObject(s_InvalidKeysetOperationMsg);
}

void throwInvalidAdditionException(const ArrayData* ad) {
  assertx(ad->isHackArray());
  const char* type = [&]{
    if (ad->isVecArray()) return "Vecs";
    if (ad->isDict()) return "Dicts";
    if (ad->isKeyset()) return "Keysets";
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

void raiseHackArrCompatRefBind(int64_t k) {
  raise_hackarr_compat_notice(
    folly::sformat("Binding ref in array with key {}", k)
  );
}

void raiseHackArrCompatRefBind(const StringData* k) {
  raise_hackarr_compat_notice(
    folly::sformat("Binding ref in array with key \"{}\"", k)
  );
}

void raiseHackArrCompatRefNew() {
  raise_hackarr_compat_notice("Binding new-element ref in array");
}

void raiseHackArrCompatRefIter() {
  raise_hackarr_compat_notice("Ref binding iteration on array");
}

void raiseHackArrCompatAdd() {
  raise_hackarr_compat_notice("Using + operator on arrays");
}

void raiseHackArrCompatArrMixedCmp() {
  raise_hackarr_compat_notice(Strings::HACKARR_COMPAT_ARR_MIXEDCMP);
}

void raiseHackArrCompatMissingIncDec() {
  raise_hackarr_compat_notice("Inc/dec on missing array element");
}

void raiseHackArrCompatMissingSetOp() {
  raise_hackarr_compat_notice("Set-op on missing array element");
}

std::string makeHackArrCompatImplicitArrayKeyMsg(const TypedValue* key) {
  return folly::sformat(
    "Implicit conversion of {} to array key",
    describeKeyType(key)
  );
}

void raiseHackArrCompatImplicitArrayKey(const TypedValue* key) {
  raise_hackarr_compat_notice(makeHackArrCompatImplicitArrayKeyMsg(key));
}

///////////////////////////////////////////////////////////////////////////////

}
