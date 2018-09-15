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

#ifndef incl_HPHP_TV_REFCOUNT_H_
#define incl_HPHP_TV_REFCOUNT_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/assertions.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

using RawDestructor = void(*)(void*);
extern RawDestructor g_destructors[kDestrTableSize];

///////////////////////////////////////////////////////////////////////////////

/*
 * Return true iff decreffing `tv' will cause any helper function to be called.
 *
 * Note that there are cases (specifically, for RefData) where this function
 * returns true but tvDecRefWillRelease() will return false.
 */
ALWAYS_INLINE bool tvDecRefWillCallHelper(const TypedValue tv) {
  if (noop_decref) return false;
  return isRefcountedType(tv.m_type) && tv.m_data.pcnt->decWillRelease();
}

/*
 * Return true iff decreffing `tv' will free heap-allocated data.
 *
 * Always returns false for non-refcounted types.
 */
ALWAYS_INLINE bool tvDecRefWillRelease(TypedValue tv) {
  if (noop_decref) return false;
  if (!isRefcountedType(tv.m_type)) {
    return false;
  }
  return tv.m_data.pcnt->decWillRelease();
}

/*
 * Get the reference count of `tv'.  Intended for debugging and instrumentation
 * purposes only.
 *
 * @requires: isRefcountedType(tv->m_type)
 */
ALWAYS_INLINE RefCount tvGetCount(TypedValue tv) {
  assertx(isRefcountedType(tv.m_type));
  return tv.m_data.pcnt->count();
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE RawDestructor& destructorForType(DataType dt) {
  // We want g_destructors[(dt - kMinRefCountedDataType) >> 1]. Unfortunately,
  // gcc and clang can't quite figure out the optimal way to emit this, so we
  // do the address arithmetic manually. This results in smaller code on both
  // x86 and ARM.
  auto const elem_sz = int{sizeof(g_destructors[0])} / 2;
  auto const table = reinterpret_cast<char*>(g_destructors) -
    kMinRefCountedDataType * elem_sz;
  auto const addr = table + static_cast<int64_t>(dt) * elem_sz;
  return *reinterpret_cast<RawDestructor*>(addr);
}

/*
 * Decref `tv'.
 *
 * @requires: isRefcountedType(tv->m_type)
 */
ALWAYS_INLINE void tvDecRefCountable(TypedValue tv) {
  assertx(isRefcountedType(tv.m_type));

  if (noop_decref) return;

  if (tv.m_data.pcnt->decReleaseCheck()) {
    destructorForType(tv.m_type)(tv.m_data.pcnt);
  }
}

ALWAYS_INLINE void tvDecRefCountable(const TypedValue* tv) {
  tvDecRefCountable(*tv);
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
ALWAYS_INLINE void tvDecRefGen(TypedValue* tv) {
  if (noop_decref) return;

  if (isRefcountedType(tv->m_type)) {
    tvDecRefCountable(tv);
    // If we're in debug mode, turn the entry into null so that the GC doesn't
    // assert if it tries to follow it.
    if (debug) tv->m_type = KindOfNull;
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
ALWAYS_INLINE void tvDecRefStr(const TypedValue* tv) {
  assertx(tv->m_type == KindOfString);
  decRefStr(tv->m_data.pstr);
}

ALWAYS_INLINE void tvDecRefArr(const TypedValue* tv) {
  assertx(tv->m_type == KindOfArray ||
         tv->m_type == KindOfVec ||
         tv->m_type == KindOfDict ||
         tv->m_type == KindOfShape ||
         tv->m_type == KindOfKeyset);
  decRefArr(tv->m_data.parr);
}

ALWAYS_INLINE void tvDecRefArr(TypedValue tv) {
  tvDecRefArr(&tv);
}

ALWAYS_INLINE void tvDecRefObj(const TypedValue* tv) {
  assertx(tv->m_type == KindOfObject);
  decRefObj(tv->m_data.pobj);
}

ALWAYS_INLINE void tvDecRefRes(const TypedValue* tv) {
  assertx(tv->m_type == KindOfResource);
  decRefRes(tv->m_data.pres);
}

ALWAYS_INLINE void tvDecRefRef(const TypedValue* tv) {
  assertx(isRefType(tv->m_type));
  decRefRef(tv->m_data.pref);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Incref `tv'.
 *
 * @requires: isRefcountedType(tv->m_type)
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

///////////////////////////////////////////////////////////////////////////////

}

#endif
