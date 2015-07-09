/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/struct-array.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/vm/globals-array.h"
#include "hphp/runtime/base/proxy-array.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/mixed-array.h"
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

ArrayData::ScalarArrayKey ArrayData::GetScalarArrayKey(const char* str,
                                                       size_t sz) {
  return MD5(string_md5(str, sz).c_str());
}

ArrayData::ScalarArrayKey ArrayData::GetScalarArrayKey(ArrayData* arr) {
  VariableSerializer vs(VariableSerializer::Type::Serialize);
  auto s = vs.serialize(VarNR(arr), true);
  return GetScalarArrayKey(s.data(), s.size());
}

ArrayData* ArrayData::GetScalarArray(ArrayData* arr) {
  if (arr->empty()) return staticEmptyArray();
  auto key = GetScalarArrayKey(arr);
  return GetScalarArray(arr, key);
}

ArrayData* ArrayData::GetScalarArray(ArrayData* arr,
                                     const ScalarArrayKey& key) {
  if (arr->empty()) return staticEmptyArray();
  assert(key == GetScalarArrayKey(arr));

  ArrayDataMap::accessor acc;
  if (s_arrayDataMap.insert(acc, key)) {
    ArrayData* ad;

    if (arr->isVectorData() && !arr->isPacked()) {
      ad = PackedArray::ConvertStatic(arr);
    } else {
      ad = arr->copyStatic();
    }
    assert(ad->hasExactlyOneRef());
    ad->setStatic();
    ad->onSetEvalScalar();
    acc->second = ad;
  }
  return acc->second;
}

//////////////////////////////////////////////////////////////////////

static ArrayData* ZSetIntThrow(ArrayData* ad, int64_t k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetInt");
}

static ArrayData* ZSetStrThrow(ArrayData* ad, StringData* k, RefData* v) {
  throw FatalErrorException("Unimplemented ArrayData::ZSetStr");
}

static ArrayData* ZAppendThrow(ArrayData* ad, RefData* v, int64_t* key_ptr) {
  throw FatalErrorException("Unimplemented ArrayData::ZAppend");
}

//////////////////////////////////////////////////////////////////////

#define DISPATCH(entry)                         \
  { PackedArray::entry,                         \
    StructArray::entry,                         \
    MixedArray::entry,                          \
    EmptyArray::entry,                          \
    APCLocalArray::entry,                       \
    GlobalsArray::entry,                        \
    ProxyArray::entry                           \
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

extern const ArrayFunctions g_array_funcs_unmodified = {
  /*
   * void Release(ArrayData*)
   *
   *   Free memory associated with an array.  Generally called when
   *   the reference count on an array drops to zero.
   */
  DISPATCH(Release)

  /*
   * const TypedValue* NvGetInt(const ArrayData*, int64_t key)
   *
   *   Lookup a value in an array using an integer key.  Returns
   *   nullptr if the key is not in the array.
   */
  DISPATCH(NvGetInt)

  /*
   * const TypedValue* NvGetStr(const ArrayData*, const StringData*)
   *
   *   Lookup a value in an array using a string key.  The string key
   *   must not be an integer-like string.  Returns nullptr if the key
   *   is not in the array.
   */
  DISPATCH(NvGetStr)

  /*
   * void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos)
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
   * const Variant& GetValueRef(const ArrayData*, ssize_t pos)
   *
   *   Return a reference to the value at an iterator position.  `pos'
   *   must be a valid position for this array.
   */
  DISPATCH(GetValueRef)

  /*
   * bool IsVectorData(const ArrayData*)
   *
   *   Returns true if this array is empty, or if it has only
   *   contiguous integer keys and the first key is zero.  Determining
   *   this may be an O(N) operation.
   */
  DISPATCH(IsVectorData)

  /*
   * bool ExistsInt(const ArrayData*, int64_t key)
   *
   *   Returns true iff this array contains an element with the
   *   supplied integer key.
   */
  DISPATCH(ExistsInt)

  /*
   * bool ExistsStr(const ArrayData*, const StringData*)
   *
   *   Return true iff this array contains an element with the
   *   supplied string key.  The string must not be an integer-like
   *   string.
   */
  DISPATCH(ExistsStr)

  /*
   * ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& out, bool copy)
   *
   *   Looks up a value in the array by the supplied integer key,
   *   creating it as a KindOfNull if it doesn't exist, and sets `out'
   *   to point to it.  This function has copy/grow semantics.
   */
  DISPATCH(LvalInt)

  /*
   * ArrayData* LvalStr(ArrayData*, StringData* key, Variant*& out, bool copy)
   *
   *   Looks up a value in the array by the supplied string key,
   *   creating it as a KindOfNull if it doesn't exist, and sets `out'
   *   to point to it.  The string `key' may not be an integer-like
   *   string.  This function has copy/grow semantics.
   */
  DISPATCH(LvalStr)

  /*
   * ArrayData* LvalNew(ArrayData*, Variant*& out, bool copy)
   *
   *   This function inserts a new null value in the array at the next
   *   available integer key, and then sets `out' to point to it.  In
   *   the case that there is no next available integer key, this
   *   function sets out to point to the lvalBlackHole.  This function
   *   has copy/grow semantics.
   */
  DISPATCH(LvalNew)

  /*
   * ArrayData* LvalNewRef(ArrayData*, Variant*& out, bool copy)
   */
  {
    PackedArray::LvalNewRef,
    StructArray::LvalNew,
    MixedArray::LvalNew,
    EmptyArray::LvalNew,
    APCLocalArray::LvalNew,
    GlobalsArray::LvalNew,
    ProxyArray::LvalNew,
  },

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
   * void Ksort(int sort_flags, bool ascending)
   *
   *   Sort an array by its keys, keeping the values associated with
   *   their respective keys.
   */
  DISPATCH(Ksort)

  /*
   * void Sort(int sort_flags, bool ascending)
   *
   *   Sort an array, by values, and then assign new keys to the
   *   elements in the resulting array.
   */
  DISPATCH(Sort)

  /*
   * void Asort(int sort_flags, bool ascending)
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
   * ArrayData* CopyWithStrongIterators(const ArrayData*)
   *
   *   Explicitly request an array be copied, and that any associated
   *   strong iterators are moved to the new array.  This API does
   *   /not/ actually guarantee a copy occurs, but if it does any
   *   assoicated strong iterators must be moved.
   */
  DISPATCH(CopyWithStrongIterators)

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
   * ArrayData* Append(ArrayData*, const Variant& v, bool copy)
   *
   *   Append a new value to the array, with the next available
   *   integer key.  If there is no next available integer key, no
   *   value is appended.  This function has copy/grow semantics.
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
   * ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy)
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
   *    it needed to COW because hasMultipleRefs was true.
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
   *   hasMultipleRefs().
   */
  DISPATCH(Pop)

  /*
   * ArrayData* Dequeue(ArrayData*, Variant& value)
   *
   *   Remove the first element from the array and assign it to `value'.  This
   *   function may return a new array if it decided to COW due to
   *   hasMultipleRefs().
   */
  DISPATCH(Dequeue)

  /*
   * ArrayData* Prepend(ArrayData*, const Variant& `v', bool copy)
   *
   *   Insert `v' as the first element of the array.  Then renumber
   *   integer keys.  This function has copy/grow semantics.
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
    &StructArray::ZSetInt,
    &MixedArray::ZSetInt,
    &ZSetIntThrow,
    &ZSetIntThrow,
    &ZSetIntThrow,
    &ProxyArray::ZSetInt,
  },

  {
    &PackedArray::ZSetStr,
    &StructArray::ZSetStr,
    &MixedArray::ZSetStr,
    &ZSetStrThrow,
    &ZSetStrThrow,
    &ZSetStrThrow,
    &ProxyArray::ZSetStr,
  },

  {
    &PackedArray::ZAppend,
    &StructArray::ZAppend,
    &MixedArray::ZAppend,
    &ZAppendThrow,
    &ZAppendThrow,
    &ZAppendThrow,
    &ProxyArray::ZAppend,
  },
};

// We create a copy so that we can install instrumentation shim-functions
// instrument g_array_funcs at runtime.
ArrayFunctions g_array_funcs = g_array_funcs_unmodified;

#undef DISPATCH

///////////////////////////////////////////////////////////////////////////////

// In general, arrays can contain int-valued-strings, even though
// plain array access converts them to integers.  non-int-string
// assersions should go upstream of the ArrayData api.

bool ArrayData::IsValidKey(const String& k) {
  return IsValidKey(k.get());
}

bool ArrayData::IsValidKey(const Variant& k) {
  return k.isInteger() ||
         (k.isString() && IsValidKey(k.getStringData()));
}

ArrayData *ArrayData::Create() {
  return staticEmptyArray();
}

ArrayData *ArrayData::Create(const Variant& value) {
  PackedArrayInit pai(1);
  pai.append(value);
  return pai.create();
}

ArrayData *ArrayData::Create(const Variant& name, const Variant& value) {
  ArrayInit init(1, ArrayInit::Map{});
  // There is no toKey() call on name.
  init.set(name, value);
  return init.create();
}

ArrayData *ArrayData::CreateRef(Variant& value) {
  PackedArrayInit pai(1);
  pai.appendRef(value);
  return pai.create();
}

ArrayData *ArrayData::CreateRef(const Variant& name, Variant& value) {
  ArrayInit init(1, ArrayInit::Map{});
  // There is no toKey() call on name.
  init.setRef(name, value, true);
  return init.create();
}

///////////////////////////////////////////////////////////////////////////////
// reads

int ArrayData::compare(const ArrayData *v2) const {
  assert(v2);

  auto const count1 = size();
  auto const count2 = v2->size();
  if (count1 < count2) return -1;
  if (count1 > count2) return 1;
  if (count1 == 0) return 0;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  for (ArrayIter iter(this); iter; ++iter) {
    auto key = iter.first();
    if (!v2->exists(key)) return 1;
    auto value1 = iter.second();
    auto value2 = v2->get(key);
    if (HPHP::more(value1, value2)) return 1;
    if (HPHP::less(value1, value2)) return -1;
  }

  return 0;
}

bool ArrayData::equal(const ArrayData *v2, bool strict) const {
  assert(v2);

  if (this == v2) return true;
  auto const count1 = size();
  auto const count2 = v2->size();
  if (count1 != count2) return false;
  if (count1 == 0) return true;

  // Prevent circular referenced objects/arrays or deep ones.
  check_recursion_error();

  if (strict) {
    for (ArrayIter iter1(this), iter2(v2); iter1; ++iter1, ++iter2) {
      assert(iter2);
      if (!same(iter1.first(), iter2.first())
          || !same(iter1.second(), iter2.secondRef())) return false;
    }
  } else {
    for (ArrayIter iter(this); iter; ++iter) {
      Variant key(iter.first());
      if (!v2->exists(key)) return false;
      if (!tvEqual(*iter.second().asTypedValue(),
                   *v2->get(key).asTypedValue())) {
        return false;
      }
    }
  }

  return true;
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

void ArrayData::serializeImpl(VariableSerializer *serializer) const {
  serializer->writeArrayHeader(size(), isVectorData());
  for (ArrayIter iter(this); iter; ++iter) {
    serializer->writeArrayKey(iter.first());
    serializer->writeArrayValue(iter.secondRef());
  }
  serializer->writeArrayFooter();
}

void ArrayData::serialize(VariableSerializer *serializer,
                          bool skipNestCheck /* = false */) const {
  if (size() == 0) {
    serializer->writeArrayHeader(0, isVectorData());
    serializer->writeArrayFooter();
    return;
  }
  if (!skipNestCheck) {
    if (serializer->incNestedLevel((void*)this)) {
      serializer->writeOverflow((void*)this);
    } else {
      serializeImpl(serializer);
    }
    serializer->decNestedLevel((void*)this);
  } else {
    // If isObject, the array is temporary and we should not check or save
    // its pointer.
    serializeImpl(serializer);
  }
}

const Variant& ArrayData::get(const Variant& k, bool error) const {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? get(getIntKey(cell), error)
                        : get(getStringKey(cell), error);
}

const Variant& ArrayData::getNotFound(int64_t k) {
  raise_notice("Undefined index: %" PRId64, k);
  return null_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  return null_variant;
}

const Variant& ArrayData::getNotFound(int64_t k, bool error) const {
  return error && kind() != kGlobalsKind ? getNotFound(k) :
         null_variant;
}

const Variant& ArrayData::getNotFound(const StringData* k, bool error) const {
  return error && kind() != kGlobalsKind ? getNotFound(k) :
         null_variant;
}

const Variant& ArrayData::getNotFound(const String& k) {
  raise_notice("Undefined index: %s", k.data());
  return null_variant;
}

const Variant& ArrayData::getNotFound(const Variant& k) {
  raise_notice("Undefined index: %s", k.toString().data());
  return null_variant;
}

const char* ArrayData::kindToString(ArrayKind kind) {
  std::array<const char*,7> names = {{
    "PackedKind",
    "StructKind",
    "MixedKind",
    "EmptyKind",
    "ApcKind",
    "GlobalsKind",
    "ProxyKind",
  }};
  static_assert(names.size() == kNumKinds, "add new kinds here");
  return names[kind];
}

///////////////////////////////////////////////////////////////////////////////
}
