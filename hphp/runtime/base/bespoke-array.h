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
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/bespoke-layout.h"

namespace HPHP {

inline bool allowBespokeArrayLikes() {
  return RO::EvalBespokeArrayLikeMode > 0;
}

inline bool shouldTestBespokeArrayLikes() {
  return RO::EvalBespokeArrayLikeMode == 1;
}

namespace bespoke {

// Hide Layout and its implementations to the rest of the codebase.
struct Layout;
struct LayoutFunctions;

// Maybe wrap this array in a LoggingArray, based on runtime options.
ArrayData* maybeMakeLoggingArray(ArrayData*);
const ArrayData* maybeMakeLoggingArray(const ArrayData*);
ArrayData* makeBespokeForTesting(ArrayData*);
void setLoggingEnabled(bool);
void exportProfiles();
void waitOnExportProfiles();

// Type-safe layout index, so that we can't mix it up with other ints.
struct LayoutIndex {
  bool operator==(LayoutIndex o) const { return raw == o.raw; }
  bool operator!=(LayoutIndex o) const { return raw != o.raw; }
  uint16_t raw;
};

}

/*
 * A bespoke array is an array satisfing the ArrayData interface but backed by
 * a variety of possible memory layouts. Eventually, our goal is to generate
 * these layouts at runtime, based on profiling information.
 *
 * Bespoke arrays store their bespoke layout in the ArrayData's m_extra_hi16
 * field.
 *
 * Individual bespoke layouts may choose to use m_extra_lo16 for whatever they
 * like.
 */
struct BespokeArray : ArrayData {
  /*
   * We set the MSB of m_extra_hi16 when we store the bespoke layout there. This
   * is so (on little-endian systems) we can do a single comparison to test both
   * (size >= some constant) and bespoke-ness together.
   */
  static constexpr bespoke::LayoutIndex kExtraMagicBit = {1 << 15};

  static BespokeArray* asBespoke(ArrayData*);
  static const BespokeArray* asBespoke(const ArrayData*);

  BespokeLayout layout() const;

protected:
  bespoke::LayoutIndex layoutIndex() const;
  const bespoke::LayoutFunctions* vtable() const;
  void setLayoutIndex(bespoke::LayoutIndex index);

public:
  size_t heapSize() const;
  void scan(type_scan::Scanner& scan) const;

  bool checkInvariants() const;

  // Escalate the given bespoke array-like to a vanilla array-like.
  // The provided `reason` may be logged.
  static ArrayData* ToVanilla(const ArrayData* ad, const char* reason);

  // Bespoke arrays can be converted to uncounted values for APC.
  static ArrayData* MakeUncounted(ArrayData* array, bool hasApcTv,
                                  DataWalker::PointerMap* seen);
  static void ReleaseUncounted(ArrayData*);

  // Log that a bespoke array made it to a particular profiling translation.
  void logReachEvent(TransID transId, uint32_t guardIdx);

private:
  template <typename T, typename ... Args>
  [[noreturn]] static inline T UnsupportedOp(Args ... args) {
    always_assert(false);
  }

public:
  // ArrayData interface
  static void Release(ArrayData*);
  static bool IsVectorData(const ArrayData* ad);

  // RO access
  static TypedValue NvGetInt(const ArrayData* ad, int64_t key);
  static TypedValue NvGetStr(const ArrayData* ad, const StringData* key);
  static TypedValue GetPosKey(const ArrayData* ad, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData* ad, ssize_t pos);
  static ssize_t NvGetIntPos(const ArrayData* ad, int64_t key);
  static ssize_t NvGetStrPos(const ArrayData* ad, const StringData* key);
  static bool ExistsInt(const ArrayData* ad, int64_t key);
  static bool ExistsStr(const ArrayData* ad, const StringData* key);

  // iteration
  static ssize_t IterBegin(const ArrayData* ad);
  static ssize_t IterLast(const ArrayData* ad);
  static ssize_t IterEnd(const ArrayData* ad);
  static ssize_t IterAdvance(const ArrayData* ad, ssize_t pos);
  static ssize_t IterRewind(const ArrayData* ad, ssize_t pos);

  // RW access
  //
  // The "Elem" methods are variants on the "Lval" methods that allow us to
  // avoid unnecessary escalation. It restricts both the callee and caller:
  //
  //  * The callee implementing the method may *never* return a DataType*
  //    pointing to a persistent counterpart of a maybe-countable DataType
  //    (i.e. KindOfPersistentString, or any of the persistent array-likes).
  //
  //  * The caller using this method may *never* change the value of the type
  //    the resulting lval points to. (It may store to it with the same type.)
  //
  static arr_lval LvalInt(ArrayData* ad, int64_t key);
  static arr_lval LvalStr(ArrayData* ad, StringData* key);
  static arr_lval ElemInt(ArrayData* ad, int64_t key);
  static arr_lval ElemStr(ArrayData* ad, StringData* key);

  // insertion
  static ArrayData* SetInt(ArrayData* ad, int64_t key, TypedValue v);
  static ArrayData* SetStr(ArrayData* ad, StringData* key, TypedValue v);
  static ArrayData* SetIntMove(ArrayData* ad, int64_t key, TypedValue v);
  static ArrayData* SetStrMove(ArrayData* ad, StringData* key, TypedValue v);

  // deletion
  static ArrayData* RemoveInt(ArrayData* ad, int64_t key);
  static ArrayData* RemoveStr(ArrayData* ad, const StringData* key);

  // sorting
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction sf);
  static auto constexpr Sort   = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Asort  = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Ksort  = UnsupportedOp<void, ArrayData*, int, bool>;
  static auto constexpr Usort  = UnsupportedOp<bool, ArrayData*, const Variant&>;
  static auto constexpr Uasort = UnsupportedOp<bool, ArrayData*, const Variant&>;
  static auto constexpr Uksort = UnsupportedOp<bool, ArrayData*, const Variant&>;

  // high-level ops
  static ArrayData* Append(ArrayData* ad, TypedValue v);
  static ArrayData* Pop(ArrayData* ad, Variant& out);
  static void OnSetEvalScalar(ArrayData* ad);

  // copies and conversions
  static ArrayData* CopyStatic(const ArrayData* ad);
  static ArrayData* ToDVArray(ArrayData* ad, bool copy);
  static ArrayData* ToHackArr(ArrayData* ad, bool copy);
  static ArrayData* SetLegacyArray(ArrayData* ad, bool copy, bool legacy);
};

} // namespace HPHP

#endif // HPHP_BESPOKE_ARRAY_H_
