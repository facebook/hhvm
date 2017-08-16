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

#ifndef incl_HPHP_TV_MUTATE_H_
#define incl_HPHP_TV_MUTATE_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
/*
 * Boxing and unboxing.
 *
 * All of these functions assume that `tv' is live (i.e., contains a valid,
 * refcount-supported value).
 */

/*
 * Box `tv' in place.
 */
ALWAYS_INLINE TypedValue* tvBox(TypedValue* tv) {
  assert(cellIsPlausible(*tv));
  tv->m_data.pref = RefData::Make(*tv);
  tv->m_type = KindOfRef;
  return tv;
}

/*
 * Box `tv' in place, if it's not already boxed.
 */
ALWAYS_INLINE void tvBoxIfNeeded(TypedValue* tv) {
  if (tv->m_type != KindOfRef) tvBox(tv);
}

/*
 * Unbox `tv' in place.
 *
 * @requires: tv->m_type == KindOfRef
 */
ALWAYS_INLINE void tvUnbox(TypedValue* tv) {
  assert(refIsPlausible(*tv));

  auto const r = tv->m_data.pref;
  auto const inner = r->tv();
  assert(cellIsPlausible(*inner));

  tv->m_data.num = inner->m_data.num;
  tv->m_type = inner->m_type;
  tvIncRefGen(tv);
  decRefRef(r);

  assert(tvIsPlausible(*tv));
}

/*
 * Unbox `tv' in place, if it's a ref.
 */
ALWAYS_INLINE void tvUnboxIfNeeded(TypedValue* tv) {
  if (tv->m_type == KindOfRef) tvUnbox(tv);
}

/*
 * Return a reference to an unboxed `tv'.
 */
ALWAYS_INLINE Cell tvToCell(TypedValue tv) {
  return LIKELY(tv.m_type != KindOfRef) ? tv : *tv.m_data.pref->tv();
}
ALWAYS_INLINE Cell* tvToCell(TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}
ALWAYS_INLINE const Cell* tvToCell(const TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}
ALWAYS_INLINE member_rval tvToCell(member_rval tv) {
  return LIKELY(tv.type() != KindOfRef)
    ? tv
    : member_rval { tv.val().pref, tv.val().pref->tv() };
}

/*
 * Return an unboxed and initialized `tv'.
 *
 * This function:
 *  - is *tvToCell() if `tv' is KindOfRef.
 *  - returns a KindOfNull when `tv' is KindOfUninit.
 *  - is the identity otherwise.
 */
ALWAYS_INLINE Cell tvToInitCell(TypedValue tv) {
  if (UNLIKELY(tv.m_type == KindOfRef)) {
    assertx(tv.m_data.pref->tv()->m_type != KindOfUninit);
    return *tv.m_data.pref->tv();
  }
  if (tv.m_type == KindOfUninit) return make_tv<KindOfNull>();
  return tv;
}
ALWAYS_INLINE Cell tvToInitCell(const TypedValue* tv) {
  return tvToInitCell(*tv);
}

/*
 * Assert that `tv' is a Cell; then just return it.
 */
ALWAYS_INLINE Cell tvAssertCell(TypedValue tv) {
  assert(cellIsPlausible(tv));
  return tv;
}
ALWAYS_INLINE Cell* tvAssertCell(TypedValue* tv) {
  assert(cellIsPlausible(*tv));
  return tv;
}
ALWAYS_INLINE const Cell* tvAssertCell(const TypedValue* tv) {
  assert(cellIsPlausible(*tv));
  return tv;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Copy and duplicate operations.
 *
 * These operations each copy a `fr' TypedValue rval to a `to' TypedValue lval.
 * All of them assume that `to' is dead (i.e., decref'd elsewhere, and possibly
 * invalid), but:
 *
 *  - Copy doesn't touch refcounts at all.
 *  - Dup increfs `fr' after copying it to `to'.
 */

/*
 * Raw copy of a TypedValue from one location to another, without doing any
 * reference count manipulation.
 *
 * Copies the m_data and m_type fields, but not m_aux.  (For that you need
 * TypedValue::operator=.)
 */
ALWAYS_INLINE void tvCopy(const TypedValue& fr, TypedValue& to) {
  assert(tvIsPlausible(fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = fr.m_type;
}

/*
 * tvCopy() with added assertions, for Cells and Refs.
 */
ALWAYS_INLINE void cellCopy(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
}
ALWAYS_INLINE void refCopy(const Ref fr, Ref& to) {
  assert(refIsPlausible(fr));
  tvCopy(fr, to);
}

/*
 * Duplicate a TypedValue from one location to another.
 *
 * Copies the m_data and m_type fields, and increfs `fr', but does not decref
 * `to' (i.e., `to' is assumed to be dead, or decref'd elsewhere).
 */
ALWAYS_INLINE void tvDup(const TypedValue& fr, TypedValue& to) {
  tvCopy(fr, to);
  tvIncRefGen(&to);
}

/*
 * tvDup() with added assertions, for Cells and Refs.
 */
ALWAYS_INLINE void cellDup(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
  tvIncRefGen(&to);
}
ALWAYS_INLINE void refDup(const Ref fr, Ref& to) {
  assert(refIsPlausible(fr));
  to.m_type = KindOfRef;
  to.m_data.pref = fr.m_data.pref;
  to.m_data.pref->incRefCount();
}

namespace detail {

/*
 * Duplicate `fr' to `to' with custom reference demotion semantics.
 */
template<class Fn>
void tvDupWithRef(const TypedValue& fr, TypedValue& to, Fn should_demote) {
  assert(tvIsPlausible(fr));

  tvCopy(fr, to);
  if (!isRefcountedType(fr.m_type)) return;
  if (fr.m_type != KindOfRef) {
    tvIncRefCountable(&to);
    return;
  }
  auto ref = fr.m_data.pref;
  if (should_demote(ref)) {
    cellDup(*ref->tv(), to);
    return;
  }
  assert(to.m_type == KindOfRef);
  to.m_data.pref->incRefCount();
}

}

/*
 * Duplicate `fr' to `to' with reference demotion semantics.
 *
 * If `fr' has type KindOfRef but is not "observably referenced"---i.e.,
 * RefData::isReferenced() is false---this does cellDup(tvToCell(fr), to).
 * Otherwise, it's the same as tvDup().
 *
 * In other words, this is a tvDup() that unboxes single-reference refs.
 */
ALWAYS_INLINE
void tvDupWithRef(const TypedValue& fr, TypedValue& to) {
  detail::tvDupWithRef(fr, to, [] (RefData* ref) {
    return !ref->isReferenced();
  });
}

/*
 * Duplicate `fr' to `to' with reference demotion semantics.
 *
 * Just like tvDupWithRef(fr, to), except we won't demote if the ref's
 * inner value is `container'.
 */
ALWAYS_INLINE
void tvDupWithRef(const TypedValue& fr, TypedValue& to,
                  const ArrayData* container) {
  assert(container);
  detail::tvDupWithRef(fr, to, [&] (RefData* ref) {
    return !ref->isReferenced() &&
           (!isArrayType(ref->tv()->m_type) ||
            container != ref->tv()->m_data.parr);
  });
}

/*
 * Write a value to `to', with Dup semantics.
 */
ALWAYS_INLINE void tvWriteUninit(TypedValue* to) {
  to->m_type = KindOfUninit;
}
ALWAYS_INLINE void tvWriteNull(TypedValue* to) {
  to->m_type = KindOfNull;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Move and set operations.
 *
 * These operations each copy a `fr' TypedValue rval to a `to' TypedValue lval.
 * However, unlike the copy and duplicate operations above, these assume that
 * `to' is live, and will thus decref it as appropriate.
 *
 * Additionally, unlike Copy and Dup, Move and Set will transfer `fr' into the
 * inner type of `to' when `to' is a KindOfRef.  (Meanwhile, MoveIgnoreRef and
 * SetIgnoreRef are exactly Copy and Dup which decref `to' after assignment.)
 *
 * Specifically:
 *
 *  - Move represents ownership transfer, and doesn't incref the moved value.
 *  - Set increfs the assigned value.
 *
 * Move corresponds to Copy, and Set corresponds to Dup.
 */

/*
 * Move the value of the Cell in `fr' to `to'.
 *
 * To represent move semantics, we decref the original value of `to', but we do
 * not increment its new value (i.e., `fr').
 *
 * If `to' is KindOfRef, places the value of `fr' in the RefData pointed to by
 * `to' instead.
 */
ALWAYS_INLINE void tvMove(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const cell = tvToCell(&to);
  auto const old = *cell;
  cellCopy(fr, *cell);
  tvDecRefGen(old);
}

/*
 * Move the value of the Cell in `fr' to `to'.
 *
 * Just like tvMove(), except we always overwrite `to' itself rather than its
 * inner value if it has type KindOfRef.
 */
ALWAYS_INLINE void tvMoveIgnoreRef(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const old = to;
  cellCopy(fr, to);
  tvDecRefGen(old);
  assert(cellIsPlausible(to));
}

/*
 * Move the value of the Cell in `fr' to the Cell `to'.
 *
 * Just like tvMove() or tvMoveIgnoreRef() with added assertions for `to'.
 */
ALWAYS_INLINE void cellMove(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(to));
  tvMoveIgnoreRef(fr, to);
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * If `to' has type KindOfRef, places the value of `fr' in the RefData pointed
 * to by `to'.
 */
ALWAYS_INLINE void tvSet(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const cell = tvToCell(&to);
  auto const old = *cell;
  cellDup(fr, *cell);
  tvDecRefGen(old);
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * Just like tvSet(), except we always overwrite `to' itself rather than its
 * inner value if it has type KindOfRef.
 */
ALWAYS_INLINE void tvSetIgnoreRef(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const old = to;
  cellDup(fr, to);
  tvDecRefGen(old);
  assert(cellIsPlausible(to));
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * Just like tvSet() or tvSetIgnoreRef() with added assertions for `to'.
 */
ALWAYS_INLINE void cellSet(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(to));
  tvSetIgnoreRef(fr, to);
}

/*
 * Set `fr' to `to' with reference demotion semantics.
 *
 * This is just like tvDupWithRef(), except it decrefs the old value of `to'.
 * Unlike the other Set functions, this accepts any TypedValue as `fr', since
 * it is ref-preserving (modulo demotion).
 */
ALWAYS_INLINE void tvSetWithRef(const TypedValue fr, TypedValue& to) {
  assert(tvIsPlausible(to));
  auto const old = to;
  tvDupWithRef(fr, to);
  tvDecRefGen(old);
  assert(tvIsPlausible(to));
}

/*
 * Binding assignment from `fr' to `to'.
 *
 * This behaves just like tvSetIgnoreRef(), in that we always overwrite `to'
 * directly, rather than its inner value.  Notably, this causes `to' to refer
 * to the RefData of `fr'---it does not set the inner value of `to' to be that
 * of `fr'.
 *
 * @requires: fr->m_type == KindOfRef
 */
ALWAYS_INLINE void tvBind(const TypedValue* fr, TypedValue* to) {
  assert(fr->m_type == KindOfRef);
  auto const old = *to;
  refDup(*fr, *to);
  tvDecRefGen(old);
}

/*
 * Binding assignment from `fr' to `to'.
 *
 * Like tvBind(), except with a raw RefData* instead of a TypedValue.
 */
ALWAYS_INLINE void tvBindRef(RefData* fr, TypedValue* to) {
  auto const old = *to;
  fr->incRefCount();
  tvCopy(make_tv<KindOfRef>(fr), *to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfUninit to `to' directly, ignoring refs.
 *
 * Equivalent to tvSetIgnoreRef(make_tv<KindOfUninit>(), to).
 */
ALWAYS_INLINE void tvUnset(TypedValue& to) {
  auto const old = to;
  tvWriteUninit(&to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfNull to `to' directly, ignoring refs.
 *
 * Equivalent to tvSetIgnoreRef(make_tv<KindOfNull>(), to).
 */
ALWAYS_INLINE void cellSetNull(Cell& to) {
  auto const old = to;
  tvWriteNull(&to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfNull to `to'.
 *
 * If `to' has type KindOfRef, places the value of `fr' in the RefData pointed
 * to by `to'.
 *
 * Equivalent to tvSet(make_tv<KindOfNull>(), to).
 */
ALWAYS_INLINE void tvSetNull(TypedValue& to) {
  cellSetNull(*tvToCell(&to));
}

/*
 * tvSet() analogues for raw data elements.
 */
ALWAYS_INLINE void tvSetBool(bool val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const old = *to;
  to->m_type = KindOfBoolean;
  to->m_data.num = val;
  tvDecRefGen(old);
}
ALWAYS_INLINE void tvSetInt(int64_t val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const old = *to;
  to->m_type = KindOfInt64;
  to->m_data.num = val;
  tvDecRefGen(old);
}
ALWAYS_INLINE void tvSetDouble(double val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const old = *to;
  to->m_type = KindOfDouble;
  to->m_data.dbl = val;
  tvDecRefGen(old);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Write garbage into a memory slot for a TypedValue that should not be used
 * anymore.
 *
 * Does nothing outside of debug builds.
 */
ALWAYS_INLINE void tvDebugTrash(TypedValue* tv) {
  if (debug) memset(tv, kTVTrashFill, sizeof *tv);
}

/*
 * Like make_tv(), but determine the appropriate Datatype from an ArrayData.
 */
ALWAYS_INLINE TypedValue make_array_like_tv(ArrayData* a) {
  TypedValue ret;
  ret.m_data.parr = a;
  ret.m_type = a->toDataType();
  assert(cellIsPlausible(ret));
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
