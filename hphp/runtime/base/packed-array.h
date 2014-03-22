/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct ArrayData;
struct RefData;
struct StringData;
struct TypedValue;
struct MArrayIter;
struct APCHandle;

//////////////////////////////////////////////////////////////////////

/*
 * Packed arrays are a specialized array layout for vector-like data.
 * That is, php arrays with zero-based contiguous integer keys, and
 * values of mixed types.
 *
 * Currently the layout of this array kind is set up to match
 * HphpArray, with some of the fields uninitialized.  I.e., packed
 * arrays allocate space for a hashtable that they don't use, in order
 * to make the code path that transitions from packed to mixed
 * cheaper.  (This is a transitional thing---we'd like to further
 * specialize the layout.)  See HphpArray::checkInvariants for
 * details.
 */
struct PackedArray {
  static void Release(ArrayData*);
  static TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v,
    bool copy);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, const Variant& v,
    bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k,
    const Variant& v, bool copy);
  static ArrayData* AddInt(ArrayData* ad, int64_t k, const Variant& v,
    bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);
  static bool ValidMArrayIter(const ArrayData*, const MArrayIter& fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);
  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void OnSetEvalScalar(ArrayData*);
};

//////////////////////////////////////////////////////////////////////

}

#endif
