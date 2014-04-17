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
#ifndef incl_HPHP_PACKED_ARRAY_H_
#define incl_HPHP_PACKED_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct RefData;
struct ArrayData;
struct StringData;
struct TypedValue;
struct MArrayIter;
struct APCHandle;
struct MixedArray;

//////////////////////////////////////////////////////////////////////

/*
 * Packed arrays are a specialized array layout for vector-like data.
 * That is, php arrays with zero-based contiguous integer keys, and
 * values of mixed types.
 *
 * Currently the layout of this array kind is set up to match
 * MixedArray, with some of the fields uninitialized.  I.e., packed
 * arrays allocate space for a hashtable that they don't use, in order
 * to make the code path that transitions from packed to mixed
 * cheaper.  (This is a transitional thing---we'd like to further
 * specialize the layout.)  See MixedArray::checkInvariants for
 * details.
 */
struct PackedArray {
  static void Release(ArrayData*);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static constexpr auto NvGetStr =
    reinterpret_cast<const TypedValue* (*)(const ArrayData*,
                                           const StringData*)>(
      ArrayCommon::ReturnNull
    );
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v,
    bool copy);
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static constexpr auto IsVectorData =
    reinterpret_cast<bool (*)(const ArrayData*)>(
      ArrayCommon::ReturnTrue
    );
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static constexpr auto ExistsStr =
    reinterpret_cast<bool (*)(const ArrayData*, const StringData*)>(
      ArrayCommon::ReturnFalse
    );
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, Variant& v,
    bool copy);
  static constexpr auto AddInt = &SetInt;
  static constexpr auto AddStr = &SetStr;
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*);
  static void Ksort(ArrayData*, int, bool);
  static void Sort(ArrayData*, int, bool);
  static void Asort(ArrayData*, int, bool);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* ZSetInt(ArrayData*, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData*, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData*, RefData* v);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static constexpr auto Renumber =
    reinterpret_cast<void (*)(ArrayData*)>(
      ArrayCommon::NoOp
    );
  static void OnSetEvalScalar(ArrayData*);
  static constexpr auto Escalate =
    reinterpret_cast<ArrayData* (*)(const ArrayData*)>(
      ArrayCommon::ReturnFirstArg
    );
  static constexpr auto GetAPCHandle =
    reinterpret_cast<APCHandle* (*)(const ArrayData*)>(
      ArrayCommon::ReturnNull
    );

  //////////////////////////////////////////////////////////////////////

  static bool checkInvariants(const ArrayData*);

private:
  static ArrayData* Grow(ArrayData*);
  static MixedArray* ToMixedHeader(const ArrayData*, size_t);
  static MixedArray* ToMixed(ArrayData*);
  static MixedArray* ToMixedCopy(const ArrayData*);
  static MixedArray* ToMixedCopyReserve(const ArrayData*, size_t);
  static ArrayData* CopyAndResizeIfNeededSlow(const ArrayData*);
  static ArrayData* CopyAndResizeIfNeeded(const ArrayData*);
  static ArrayData* ResizeIfNeeded(ArrayData*);
};

//////////////////////////////////////////////////////////////////////

}

#endif
