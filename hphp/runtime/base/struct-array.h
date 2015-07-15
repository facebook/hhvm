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
#ifndef incl_HPHP_STRUCT_ARRAY_H_
#define incl_HPHP_STRUCT_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-common.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct RefData;
struct ArrayData;
struct StringData;
struct MArrayIter;
struct MixedArray;
class Shape;

//////////////////////////////////////////////////////////////////////

/*
 * Struct arrays are a specialized array layout for record-like data.
 * That is, php arrays with static string keys and values of mixed types.
 *
 * Currently the layout of this array kind is set up to match MixedArray,
 * with some of the fields uninitialized to make the code path that
 * transitions from struct to mixed cheaper. See MixedArray::checkInvariants
 * for details.
 */
struct StructArray : public ArrayData {
  static ArrayData* MakeUncounted(ArrayData*);
  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static const TypedValue* NvGetStr(const ArrayData*, const StringData*);
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*);
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData*);
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
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*, SortFunction);
  static void Ksort(ArrayData*, int, bool);
  static void Sort(ArrayData*, int, bool);
  static void Asort(ArrayData*, int, bool);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* ZSetInt(ArrayData*, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData*, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData*, RefData* v, int64_t* key_ptr);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad);

  //////////////////////////////////////////////////////////////////////

  ArrayData* asArrayData() { return this; }
  const ArrayData* asArrayData() const { return this; }
  static bool checkInvariants(const ArrayData*);

  static StructArray* asStructArray(ArrayData*);
  static const StructArray* asStructArray(const ArrayData*);

  static StructArray* create(Shape*, const TypedValue*, size_t);
  static StructArray* createReversedValues(Shape* shape,
    const TypedValue* values, size_t length);
  static StructArray* createNoCopy(Shape*, size_t);
  static StructArray* createStatic(Shape*, size_t);
  static StructArray* createUncounted(Shape*, size_t);
  static size_t bytesForCapacity(size_t capacity);

  Shape* shape();
  Shape* shape() const;
  void setShape(Shape*);
  TypedValue* data();
  const TypedValue* data() const;
  size_t size() const;
  size_t capacity() const;
  static size_t heapSize(const ArrayData*);

  MixedArray* toMixedArray();

  static const uint32_t MaxMakeSize = 64;

  static constexpr ptrdiff_t dataOffset() {
    return sizeof(StructArray);
  }

  static constexpr ptrdiff_t shapeOffset() {
    return offsetof(StructArray, m_shape);
  }

  template<class F> void scan(F&) const;

private:
  template <typename F> friend void scan(const StructArray&, F&);

  StructArray(uint32_t size, uint32_t pos, Shape* shape);

  static MixedArray* ToMixedHeader(size_t);
  static MixedArray* ToMixed(StructArray*);
  static MixedArray* ToMixedCopy(const StructArray*);
  static MixedArray* ToMixedCopyReserve(const StructArray*, size_t);

  static StructArray* Grow(StructArray*, Shape*);

  static StructArray* CopyAndResizeIfNeeded(const StructArray*, Shape*);
  static StructArray* ResizeIfNeeded(StructArray*, Shape*);

  Shape* m_shape;
};

inline size_t StructArray::bytesForCapacity(size_t capacity) {
  return sizeof(StructArray) + sizeof(TypedValue) * capacity;
}

}

#endif
