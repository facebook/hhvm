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
#include "hphp/runtime/base/tv-array-like.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
/*
 * Return an initialized `tv'.
 *
 * This function:
 *  - returns a KindOfNull when `tv' is KindOfUninit.
 *  - is the identity otherwise.
 */
ALWAYS_INLINE TypedValue tvToInit(TypedValue tv) {
  if (tv.m_type == KindOfUninit) return make_tv<KindOfNull>();
  return tv;
}

/*
 * Assert that `tv' is a TypedValue; then just return it.
 */
ALWAYS_INLINE TypedValue tvAssertPlausible(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  return tv;
}
ALWAYS_INLINE TypedValue* tvAssertPlausible(TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  return tv;
}
ALWAYS_INLINE const TypedValue* tvAssertPlausible(const TypedValue* tv) {
  assertx(tvIsPlausible(*tv));
  return tv;
}
template<bool is_const>
ALWAYS_INLINE tv_val<is_const> tvAssertPlausible(tv_val<is_const> tv) {
  assertx(tvIsPlausible(*tv));
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
  assertx(tvIsPlausible(fr));
  val(to).num = fr.m_data.num;
  type(to) = fr.m_type;
}

/*
 * Duplicate a TypedValue from one location to another.
 *
 * Copies the m_data and m_type fields, and increfs `fr', but does not decref
 * `to' (i.e., `to' is assumed to be dead, or decref'd elsewhere).
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvDup(const TypedValue& fr, T&& to) {
  assertx(tvIsPlausible(fr));
  tvCopy(fr, to);
  tvIncRefGen(as_tv(to));
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
 * Specifically:
 *
 *  - Move represents ownership transfer, and doesn't incref the moved value.
 *  - Set increfs the assigned value.
 *
 * Move corresponds to Copy, and Set corresponds to Dup.
 */

/*
 * Move the value of the TypedValue in `fr' to `to'.
 *
 * To represent move semantics, we decref the original value of `to', but we do
 * not increment its new value (i.e., `fr').
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvMove(const TypedValue fr, T&& to) {
  assertx(tvIsPlausible(fr));
  assertx(tvIsPlausible(as_tv(to)));
  auto const old = as_tv(to);
  tvCopy(fr, to);
  tvDecRefGen(old);
}

/*
 * Assign the value of the TypedValue in `fr' to `to', with appropriate reference
 * count modifications.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSet(const TypedValue fr, T&& to) {
  assertx(tvIsPlausible(fr));
  assertx(tvIsPlausible(as_tv(to)));
  auto const old = as_tv(to);
  tvDup(fr, to);
  tvDecRefGen(old);
}

/*
 * Like tvSet(), but preserves m_aux
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> tvSetWithAux(const TypedValue fr, C&& to) {
  assertx(tvIsPlausible(fr));
  auto const old = as_tv(to);
  to = fr;
  tvIncRefGen(to);
  tvDecRefGen(old);
}

/*
 * Assign KindOfUninit to `to' directly, ignoring refs.
 *
 * Equivalent to tvSet(make_tv<KindOfUninit>(), to).
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
 * Equivalent to tvSet(make_tv<KindOfNull>(), to).
 */
template<typename C> ALWAYS_INLINE
enable_if_lval_t<C&&, void> tvSetNull(C&& to) {
  auto const old = as_tv(to);
  tvWriteNull(to);
  tvDecRefGen(old);
}

/*
 * tvSet() analogues for raw data elements.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetBool(bool v, T&& to) {
  auto const old = as_tv(to);
  type(to) = KindOfBoolean;
  val(to).num = v;
  tvDecRefGen(old);
}
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetInt(int64_t v, T&& to) {
  auto const old = as_tv(to);
  type(to) = KindOfInt64;
  val(to).num = v;
  tvDecRefGen(old);
}
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T&&, void> tvSetDouble(double v, T&& to) {
  auto const old = as_tv(to);
  type(to) = KindOfDouble;
  val(to).dbl = v;
  tvDecRefGen(old);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Swap operation. Since both TypedValues are touched by a swap, both must be
 * valid to call this method, and neither one has its refcount updated.
 */

template<typename T, typename U, typename Ret = void>
using enable_if_both_lvals_t = typename std::enable_if<
  conjunction<
    std::is_same<enable_if_lval_t<T, void>, void>,
    std::is_same<enable_if_lval_t<U, void>, void>
  >::value,
  Ret
>::type;

template<typename T, typename U> ALWAYS_INLINE
enable_if_both_lvals_t<T&&, U&&> tvSwap(T&& a, U&& b) {
  assertx(tvIsPlausible(as_tv(a)));
  assertx(tvIsPlausible(as_tv(b)));
  using std::swap;
  swap(type(a), type(b));
  swap(val(a), val(b));
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

///////////////////////////////////////////////////////////////////////////////

}

#endif
