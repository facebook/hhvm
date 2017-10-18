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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvBox(T&& tv) {
  assert(cellIsPlausible(as_tv(tv)));
  val(tv).pref = RefData::Make(as_tv(tv));
  type(tv) = KindOfRef;
}

/*
 * Box `tv' in place, if it's not already boxed.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvBoxIfNeeded(T&& tv) {
  if (type(tv) != KindOfRef) tvBox(tv);
}

/*
 * Unbox `tv' in place.
 *
 * @requires: tv->m_type == KindOfRef
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvUnbox(T&& tv) {
  assert(refIsPlausible(as_tv(tv)));

  auto const r = val(tv).pref;
  auto const inner = r->tv();
  assert(cellIsPlausible(*inner));

  val(tv).num = inner->m_data.num;
  type(tv) = inner->m_type;
  tvIncRefGen(as_tv(tv));
  decRefRef(r);

  assert(tvIsPlausible(as_tv(tv)));
}

/*
 * Unbox `tv' in place, if it's a ref.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvUnboxIfNeeded(T&& tv) {
  if (type(tv) == KindOfRef) tvUnbox(tv);
}

/*
 * Return a reference to an unboxed `tv'.
 */
ALWAYS_INLINE Cell& tvToCell(TypedValue& tv) {
  return LIKELY(tv.m_type != KindOfRef) ? tv : *tv.m_data.pref->tv();
}
ALWAYS_INLINE Cell tvToCell(const TypedValue& tv) {
  return LIKELY(tv.m_type != KindOfRef) ? tv : *tv.m_data.pref->tv();
}
ALWAYS_INLINE Cell* tvToCell(TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}
ALWAYS_INLINE const Cell* tvToCell(const TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}
ALWAYS_INLINE member_lval tvToCell(member_lval lval) {
  return LIKELY(lval.type() != KindOfRef)
    ? lval
    : member_lval { lval.val().pref, lval.val().pref->tv() };
}
inline member_lval member_lval::unboxed() const {
  return tvToCell(*this);
}
inline member_rval member_rval::unboxed() const {
  return LIKELY(type() != KindOfRef)
    ? *this
    : member_rval { val().pref, val().pref->tv() };
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvCopy(const TypedValue& fr, T&& to) {
  assert(tvIsPlausible(fr));
  val(to).num = fr.m_data.num;
  type(to) = fr.m_type;
}

/*
 * tvCopy() with added assertions, for Cells and Refs.
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> cellCopy(const Cell fr, C&& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
}
template<typename R> ALWAYS_INLINE
enable_if_lval_t<R&&, void> refCopy(const Ref fr, R&& to) {
  assert(refIsPlausible(fr));
  tvCopy(fr, to);
}

/*
 * Duplicate a TypedValue from one location to another.
 *
 * Copies the m_data and m_type fields, and increfs `fr', but does not decref
 * `to' (i.e., `to' is assumed to be dead, or decref'd elsewhere).
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvDup(const TypedValue& fr, T&& to) {
  tvCopy(fr, to);
  tvIncRefGen(as_tv(to));
}

/*
 * tvDup() with added assertions, for Cells and Refs.
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> cellDup(const Cell fr, C&& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
  tvIncRefGen(as_tv(to));
}
template<typename R> ALWAYS_INLINE
enable_if_lval_t<R&&, void> refDup(const Ref fr, R&& to) {
  assert(refIsPlausible(fr));
  type(to) = KindOfRef;
  val(to).pref = fr.m_data.pref;
  val(to).pref->incRefCount();
}

namespace detail {

/*
 * Duplicate `fr' to `to' with custom reference demotion semantics.
 */
template<typename T, typename Fn>
enable_if_lval_t<T&&, void>
tvDupWithRef(const TypedValue& fr, T&& to, Fn should_demote) {
  assert(tvIsPlausible(fr));

  tvCopy(fr, to);
  if (!isRefcountedType(fr.m_type)) return;
  if (fr.m_type != KindOfRef) {
    tvIncRefCountable(as_tv(to));
    return;
  }
  auto ref = fr.m_data.pref;
  if (should_demote(ref)) {
    cellDup(*ref->tv(), to);
    return;
  }
  assert(type(to) == KindOfRef);
  val(to).pref->incRefCount();
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void>
tvDupWithRef(const TypedValue& fr, T&& to) {
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void>
tvDupWithRef(const TypedValue& fr, T&& to, const ArrayData* container) {
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvWriteUninit(T&& to) {
  type(to) = KindOfUninit;
}
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvWriteNull(T&& to) {
  type(to) = KindOfNull;
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvMove(const Cell fr, T&& to) {
  assert(cellIsPlausible(fr));
  auto&& cell = tvToCell(to);
  auto const old = as_tv(cell);
  cellCopy(fr, cell);
  tvDecRefGen(old);
}

/*
 * Move the value of the Cell in `fr' to `to'.
 *
 * Just like tvMove(), except we always overwrite `to' itself rather than its
 * inner value if it has type KindOfRef.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvMoveIgnoreRef(const Cell fr, T&& to) {
  assert(cellIsPlausible(fr));
  auto const old = as_tv(to);
  cellCopy(fr, to);
  tvDecRefGen(old);
  // NB: If we're in a pseudo-main, the dec-ref on "old" may trigger a
  // destructor which can bind or change the value in "to", so there's no
  // guarantee that "from" and "to" contain the same value at this point (or
  // that "to" is even still a cell).
  assert(tvIsPlausible(as_tv(to)));
}

/*
 * Move the value of the Cell in `fr' to the Cell `to'.
 *
 * Just like tvMove() or tvMoveIgnoreRef() with added assertions for `to'.
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> cellMove(const Cell fr, C&& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(as_tv(to)));
  tvMoveIgnoreRef(fr, to);
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * If `to' has type KindOfRef, places the value of `fr' in the RefData pointed
 * to by `to'.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSet(const Cell fr, T&& to) {
  assert(cellIsPlausible(fr));
  auto&& cell = tvToCell(to);
  auto const old = as_tv(cell);
  cellDup(fr, cell);
  tvDecRefGen(old);
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * Just like tvSet(), except we always overwrite `to' itself rather than its
 * inner value if it has type KindOfRef.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetIgnoreRef(const Cell fr, T&& to) {
  assert(cellIsPlausible(fr));
  auto const old = as_tv(to);
  cellDup(fr, to);
  tvDecRefGen(old);
  // NB: If we're in a pseudo-main, the dec-ref on "old" may trigger a
  // destructor which can bind or change the value in "to", so there's no
  // guarantee that "from" and "to" contain the same value at this point (or
  // that "to" is even still a cell).
  assert(tvIsPlausible(as_tv(to)));
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate reference
 * count modifications.
 *
 * Just like tvSet() or tvSetIgnoreRef() with added assertions for `to'.
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> cellSet(const Cell fr, C&& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(as_tv(to)));
  tvSetIgnoreRef(fr, to);
}

/*
 * Set `fr' to `to' with reference demotion semantics.
 *
 * This is just like tvDupWithRef(), except it decrefs the old value of `to'.
 * Unlike the other Set functions, this accepts any TypedValue as `fr', since
 * it is ref-preserving (modulo demotion).
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetWithRef(const TypedValue fr, T&& to) {
  assert(tvIsPlausible(as_tv(to)));
  auto const old = as_tv(to);
  tvDupWithRef(fr, to);
  tvDecRefGen(old);
  assert(tvIsPlausible(as_tv(to)));
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvBind(const TypedValue fr, T&& to) {
  assert(fr.m_type == KindOfRef);
  auto const old = as_tv(to);
  refDup(fr, to);
  tvDecRefGen(old);
}

/*
 * Binding assignment from `fr' to `to'.
 *
 * Like tvBind(), except with a raw RefData* instead of a TypedValue.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvBindRef(RefData* fr, T&& to) {
  auto const old = as_tv(to);
  fr->incRefCount();
  tvCopy(make_tv<KindOfRef>(fr), to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfUninit to `to' directly, ignoring refs.
 *
 * Equivalent to tvSetIgnoreRef(make_tv<KindOfUninit>(), to).
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvUnset(T&& to) {
  auto const old = as_tv(to);
  tvWriteUninit(to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfNull to `to' directly, ignoring refs.
 *
 * Equivalent to tvSetIgnoreRef(make_tv<KindOfNull>(), to).
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> cellSetNull(C&& to) {
  auto const old = as_tv(to);
  tvWriteNull(to);
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
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetNull(T&& to) {
  cellSetNull(tvToCell(to));
}

/*
 * tvSet() analogues for raw data elements.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetBool(bool v, T&& inTo) {
  auto&& to = tvToCell(inTo);
  auto const old = as_tv(to);
  type(to) = KindOfBoolean;
  val(to).num = v;
  tvDecRefGen(old);
}
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetInt(int64_t v, T&& inTo) {
  auto&& to = tvToCell(inTo);
  auto const old = as_tv(to);
  type(to) = KindOfInt64;
  val(to).num = v;
  tvDecRefGen(old);
}
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetDouble(double v, T&& inTo) {
  auto&& to = tvToCell(inTo);
  auto const old = as_tv(to);
  type(to) = KindOfDouble;
  val(to).dbl = v;
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
