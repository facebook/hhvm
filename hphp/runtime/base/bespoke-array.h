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

#ifndef HPHP_BESPOKE_ARRAY_H_
#define HPHP_BESPOKE_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

// Hide "BespokeLayout" and its implementations to the rest of the codebase.
namespace bespoke { struct Layout; }

/*
 * A bespoke array is an object satisfing the ArrayData interface but backed by
 * a variety of possible memory layouts.
 *
 * Bespoke arrays have an m_size that is the ones-complement of their bespoke
 * layout id. This means we will always call vsize() for these arrays.
 */
struct BespokeArray : ArrayData {
  const bespoke::Layout* layout() const;
  void setLayout(const bespoke::Layout*);

public:
  DataType toDataType() const;
  size_t heapSize() const;

  ArrayData* escalate() const;

  void scan(type_scan::Scanner& scan) const;

  static const BespokeArray* asBespoke(const ArrayData*);
  static       BespokeArray* asBespoke(      ArrayData*);

private:
  template <typename T, typename ... Args>
  [[noreturn]] static inline T UnsupportedOp(Args ... args) {
    always_assert(false);
  }
public:

  // ArrayData interface
  static void Release(ArrayData*);

  // accessors
  static TypedValue NvGetInt(const ArrayData* ad, int64_t key);
  static TypedValue NvGetStr(const ArrayData* ad, const StringData* key);
  static ssize_t NvGetIntPos(const ArrayData* ad, int64_t key);
  static ssize_t NvGetStrPos(const ArrayData* ad, const StringData* key);

  static TypedValue GetPosKey(const ArrayData* ad, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData* ad, ssize_t pos);

  // inspection
  static size_t Vsize(const ArrayData* ad);
  static bool IsVectorData(const ArrayData* ad);
  static bool ExistsInt(const ArrayData* ad, int64_t key);
  static bool ExistsStr(const ArrayData* ad, const StringData* key);

  // mutators
  static ArrayData* SetInt(ArrayData* ad, int64_t key, TypedValue v);
  static ArrayData* SetIntMove(ArrayData* ad, int64_t key, TypedValue v);
  static ArrayData* SetStr(ArrayData* ad, StringData* key, TypedValue v);
  static ArrayData* SetStrMove(ArrayData* ad, StringData* key, TypedValue v);

  // rw access
  static arr_lval LvalInt(ArrayData* ad, int64_t key);
  static arr_lval LvalStr(ArrayData* ad, StringData* key);

  // deletion
  static ArrayData* RemoveInt(ArrayData* ad, int64_t key);
  static ArrayData* RemoveStr(ArrayData* ad, const StringData* key);

  // iteration
  static ssize_t IterEnd(const ArrayData* ad);
  static ssize_t IterBegin(const ArrayData* ad);
  static ssize_t IterLast(const ArrayData* ad);
  static ssize_t IterAdvance(const ArrayData* ad, ssize_t pos);
  static ssize_t IterRewind(const ArrayData* ad, ssize_t pos);

  // garbage
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction sf);
  static auto constexpr Ksort = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Sort = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Asort = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Uksort = UnsupportedOp<bool, ArrayData*, const Variant&>;
  static auto constexpr Usort = UnsupportedOp<bool, ArrayData*, const Variant&>;
  static auto constexpr Uasort = UnsupportedOp<bool, ArrayData*, const Variant&>;

  // copies
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyStatic(const ArrayData* ad);

  // high level ops
  static ArrayData* Append(ArrayData* ad, TypedValue v);
  static ArrayData* Prepend(ArrayData* ad, TypedValue v);
  static ArrayData* PlusEq(ArrayData* ad, const ArrayData* other);
  static ArrayData* Merge(ArrayData* ad, const ArrayData* elems);
  static ArrayData* Pop(ArrayData* ad, Variant& out);
  static ArrayData* Dequeue(ArrayData* ad, Variant& out);
  static ArrayData* Renumber(ArrayData* ad);
  static void OnSetEvalScalar(ArrayData* ad);

  // conversions
  static ArrayData* ToPHPArray(ArrayData* ad, bool copy);
  static ArrayData* ToPHPArrayIntishCast(ArrayData* ad, bool copy);
  static ArrayData* ToDict(ArrayData* ad, bool copy);
  static ArrayData* ToVec(ArrayData* ad, bool copy);
  static ArrayData* ToKeyset(ArrayData* ad, bool copy);
  static ArrayData* ToVArray(ArrayData* ad, bool copy);
  static ArrayData* ToDArray(ArrayData* ad, bool copy);
};

} // namespace HPHP

#endif // HPHP_BESPOKE_ARRAY_H_
