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

#pragma once

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/vm/class-meth-data-ref.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

using RawDestructor = void(*)(void*);
using RawDestructors = std::array<RawDestructor, kDestrTableSize>;

extern RawDestructors g_destructors;

///////////////////////////////////////////////////////////////////////////////

/*
 * Return true iff decreffing `tv' will cause any helper function to be called.
 */
ALWAYS_INLINE bool tvDecRefWillCallHelper(const TypedValue tv) {
  if (noop_decref) return false;
  return isRefcountedType(tv.m_type) && tv.m_data.pcnt->decWillRelease();
}

/*
 * Get the reference count of `tv'.  Intended for debugging and instrumentation
 * purposes only.
 *
 * @requires: isRefcountedType(type(tv))
 */
ALWAYS_INLINE RefCount tvGetCount(TypedValue tv) {
  assertx(isRefcountedType(tv.m_type));
  return tv.m_data.pcnt->count();
}

///////////////////////////////////////////////////////////////////////////////

// Optimize array-like destructors, given the fact that all array-likes are
// "vanilla" - that is, that they have their standard layouts. Vanilla Hack
// arrays have a single known layout.
void specializeVanillaDestructors();

ALWAYS_INLINE RawDestructor destructorForType(DataType dt) {
  // We want g_destructors[(dt - kMinRefCountedDataType) >> 1]. Unfortunately,
  // gcc and clang can't quite figure out the optimal way to emit this, so we
  // do the address arithmetic manually. This results in smaller code on both
  // x86 and ARM.
  assertx(isRefcountedType(dt));
  auto const elem_sz = int{sizeof(g_destructors[0])} / 2;
  auto const table = reinterpret_cast<char*>(&g_destructors[0]) -
    kMinRefCountedDataType * elem_sz;
  auto const addr = table + static_cast<int64_t>(dt) * elem_sz;
  auto result = *reinterpret_cast<RawDestructor*>(addr);
  assertx(result != nullptr);
  return result;
}

/*
 * Decref `tv'.
 *
 * @requires: isRefcountedType(type(tv))
 */
NO_PROFILING
ALWAYS_INLINE void tvDecRefCountable(TypedValue tv) {
  assertx(isRefcountedType(tv.m_type));

  if (noop_decref) return;

  if (tv.m_data.pcnt->decReleaseCheck()) {
    destructorForType(tv.m_type)(tv.m_data.pcnt);
  }
}

template<typename T>
NO_PROFILING ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefCountable(T tv) {
  assertx(isRefcountedType(type(tv)));

  if (noop_decref) return;

  if (val(tv).pcnt->decReleaseCheck()) {
    destructorForType(type(tv))(val(tv).pcnt);
  }
}

/*
 * Decref `tv', or do nothing if it's not refcounted.
 */
ALWAYS_INLINE void tvDecRefGen(TypedValue tv) {
  if (noop_decref) return;

  if (isRefcountedType(tv.m_type)) {
    tvDecRefCountable(tv);
  }
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefGen(T tv) {
  if (noop_decref) return;

  if (isRefcountedType(type(tv))) {
    tvDecRefCountable(tv);
    // If we're in debug mode, turn the entry into null so that the GC doesn't
    // assert if it tries to follow it.
    if (debug) type(tv) = KindOfNull;
  }
}

/*
 * Same as tvDecRefGen(), except the decref branch is marked unlikely.
 */
ALWAYS_INLINE void tvDecRefGenUnlikely(TypedValue tv) {
  if (noop_decref) return;

  if (UNLIKELY(isRefcountedType(tv.m_type))) {
    tvDecRefCountable(tv);
  }
}

/*
 * Decref `tv' without releasing it, if it's refcounted.
 */
ALWAYS_INLINE void tvDecRefGenNZ(TypedValue tv) {
  assertx(!tvDecRefWillCallHelper(tv));
  if (noop_decref) return;
  if (isRefcountedType(tv.m_type)) {
    tv.m_data.pcnt->decRefCount();
  }
}
ALWAYS_INLINE void tvDecRefGenNZ(const TypedValue* tv) {
  tvDecRefGenNZ(*tv);
}

/*
 * DecRefs for TypedValues of known type.
 */
template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefStr(T tv) {
  assertx(type(tv) == KindOfString);
  decRefStr(val(tv).pstr);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefArr(T tv) {
  assertx(isArrayLikeType(type(tv)));
  assertx(isRefcountedType(type(tv)));
  decRefArr(val(tv).parr);
}

ALWAYS_INLINE void tvDecRefArr(TypedValue tv) {
  tvDecRefArr(&tv);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefObj(T tv) {
  assertx(type(tv) == KindOfObject);
  decRefObj(val(tv).pobj);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefRes(T tv) {
  assertx(type(tv) == KindOfResource);
  decRefRes(val(tv).pres);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefClsMeth(T tv) {
  assertx(isClsMethType(type(tv)));
  decRefClsMeth(val(tv).pclsmeth);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefRClsMeth(T tv) {
  assertx(isRClsMethType(type(tv)));
  decRefRClsMeth(val(tv).prclsmeth);
}

template<typename T> ALWAYS_INLINE
enable_if_lval_t<T, void> tvDecRefRFunc(T tv) {
  assertx(isRFuncType(type(tv)));
  decRefRFunc(val(tv).prfunc);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Incref `tv'.
 *
 * @requires: isRefcountedType(type(tv))
 */
ALWAYS_INLINE void tvIncRefCountable(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  assertx(isRefcountedType(tv.m_type));
  tv.m_data.pcnt->incRefCount();
}

/*
 * Incref `tv', or do nothing if it's not refcounted.
 */
ALWAYS_INLINE void tvIncRefGen(TypedValue tv) {
  assertx(tvIsPlausible(tv));
  if (isRefcountedType(tv.m_type)) {
    tvIncRefCountable(tv);
  }
}

/*
 * Incref `tv', or do nothing if it's not refcounted. This method allows tv to
 * have kInvalidDataType, which is never refcounted; that means that we can use
 * it for MixedArray values (which may be tombstones with kInvalidDataType).
 */
ALWAYS_INLINE void tvIncRefGenUnsafe(TypedValue tv) {
  assertx(tv.m_type == kInvalidDataType || tvIsPlausible(tv));
  if (isRefcountedType(tv.m_type)) {
    tvIncRefCountable(tv);
  }
}

///////////////////////////////////////////////////////////////////////////////

}

