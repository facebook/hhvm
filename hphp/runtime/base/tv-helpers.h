/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TV_HELPERS_H_
#define incl_HPHP_TV_HELPERS_H_

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/req-root.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Variant;

/*
 * Call a (ref-count related) function on a TypedValue data pointer where we
 * don't statically know the pointer's type. This is superior to the old idiom
 * of choosing one of the pointers, calling the method for that type, and
 * relying on the fact that all the method's have the same code, and therefore
 * will work correctly even if its actually a different type. This breaks strong
 * type aliasing and can cause GCC to miscompile code in hard to debug ways.
 * Instead use manual pointer arithmetic to index to where the ref-count is,
 * cast it, then call the appropriate generic function. Since we don't use any
 * of the pointer types, the compiler cannot use type aliasing assumptions to
 * miscompile.
 *
 * If we're in debug mode, switch on the type and call the appropriate function
 * directly. This is slightly slower, but gives us the benefit of extra
 * assertions.
 *
 * This assumes isRefcountedType() is true.
 */
#define TV_GENERIC_DISPATCH_FAST(exp, func)                     \
  reinterpret_cast<HPHP::HeaderWord<uint16_t,HPHP::Counted::Maybe>*>(\
      (exp).m_data.num + HPHP::HeaderOffset                     \
  )->func()

#define TV_GENERIC_DISPATCH_SLOW(exp, func) \
  [](HPHP::TypedValue tv) {                                     \
    switch (tv.m_type) {                                        \
      case HPHP::KindOfPersistentString:                        \
      case HPHP::KindOfString: return tv.m_data.pstr->func();   \
      case HPHP::KindOfPersistentArray:                         \
      case HPHP::KindOfArray: return tv.m_data.parr->func();    \
      case HPHP::KindOfObject: return tv.m_data.pobj->func();   \
      case HPHP::KindOfResource: return tv.m_data.pres->func(); \
      case HPHP::KindOfRef: return tv.m_data.pref->func();      \
      default:                                                  \
        assert_flog(false, "Bad KindOf: {}", (size_t)tv.m_type);\
    }                                                           \
  }(exp)

#ifdef DEBUG
#define TV_GENERIC_DISPATCH(exp, func) TV_GENERIC_DISPATCH_SLOW(exp, func)
#else
#define TV_GENERIC_DISPATCH(exp, func) TV_GENERIC_DISPATCH_FAST(exp, func)
#endif

/*
 * Assertions on Cells and TypedValues.  Should usually only happen
 * inside an assert().
 */
bool tvIsPlausible(TypedValue);
bool cellIsPlausible(Cell);
bool refIsPlausible(Ref);

/*
 * In a debug build, write garbage into a memory slot for a TypedValue
 * that should not be used anymore.
 */
ALWAYS_INLINE void tvDebugTrash(TypedValue* tv) {
  if (debug) memset(tv, kTVTrashFill, sizeof *tv);
}

/*
 * Returns: true if the supplied TypedValue is KindOfDouble or
 * KindOfInt64.  I.e. if it is a TypedNum.
 */
ALWAYS_INLINE bool isTypedNum(const TypedValue& tv) {
  return tv.m_type == KindOfInt64 || tv.m_type == KindOfDouble;
}

/*
 * Returns: true if the supplied TypedValue is a Cell, and either has
 * a non-reference counted type, or is a KindOfString that points to a
 * static string.
 */
ALWAYS_INLINE bool isUncounted(const TypedValue& tv) {
  auto const uncounted = !isRefcountedType(tv.m_type) ||
    (tv.m_type == KindOfString && tv.m_data.pstr->isStatic());
  if (uncounted) assert(cellIsPlausible(tv));
  return uncounted;
}

typedef void(*RawDestructor)(void*);
extern RawDestructor g_destructors[kDestrTableSize];

/*
 * Returns true if decreffing the specified TypedValue will free heap-allocated
 * data. Note that this function always returns false for non-refcounted types.
 */
bool tvDecRefWillRelease(TypedValue* tv);

/*
 * Returns true iff decreffing the specified TypedValue will cause any kind of
 * helper to be called. Note that there are cases where this function returns
 * true but tvDecRefWillRelease() will return false.
 */
ALWAYS_INLINE bool tvDecRefWillCallHelper(TypedValue* tv) {
  return isRefcountedType(tv->m_type) &&
         TV_GENERIC_DISPATCH(*tv, decWillRelease);
}

ALWAYS_INLINE void tvDecRefStr(TypedValue* tv) {
  assert(tv->m_type == KindOfString);
  decRefStr(tv->m_data.pstr);
}

ALWAYS_INLINE void tvDecRefArr(TypedValue* tv) {
  assert(tv->m_type == KindOfArray);
  decRefArr(tv->m_data.parr);
}

ALWAYS_INLINE void tvDecRefObj(TypedValue* tv) {
  assert(tv->m_type == KindOfObject);
  decRefObj(tv->m_data.pobj);
}

ALWAYS_INLINE void tvDecRefRes(TypedValue* tv) {
  assert(tv->m_type == KindOfResource);
  decRefRes(tv->m_data.pres);
}

// Assumes 'r' is live and points to a RefData
ALWAYS_INLINE void tvDecRefRefInternal(RefData* r) {
  assert(cellIsPlausible(*r->tv()));
  decRefRef(r);
}

// Assumes 'tv' is live
ALWAYS_INLINE void tvDecRefRef(TypedValue* tv) {
  assert(tv->m_type == KindOfRef);
  tvDecRefRefInternal(tv->m_data.pref);
}

// Assumes 'data' is live
// Assumes 'isRefcountedType(type)'
ALWAYS_INLINE void tvDecRefHelper(DataType type, uint64_t datum) noexcept {
  assert(type == KindOfString || type == KindOfArray ||
         type == KindOfObject || type == KindOfResource ||
         type == KindOfRef);
  TypedValue tmp;
  tmp.m_type = type;
  tmp.m_data.num = datum;
  if (TV_GENERIC_DISPATCH(tmp, decReleaseCheck)) {
    g_destructors[typeToDestrIdx(type)]((void*)datum);
  }
}

// Assumes 'tv' is live
ALWAYS_INLINE void tvRefcountedDecRefHelper(DataType type, uint64_t datum) {
  if (isRefcountedType(type)) {
    tvDecRefHelper(type, datum);
  }
}

// Assumes 'tv' is live and unlikely to be a refcounted type
ALWAYS_INLINE void tvUnlikelyRefcountedDecRef(TypedValue tv) {
  if (UNLIKELY(isRefcountedType(tv.m_type))) {
    tvDecRefHelper(tv.m_type, tv.m_data.num);
  }
}

ALWAYS_INLINE void tvRefcountedDecRef(TypedValue v) {
  return tvRefcountedDecRefHelper(v.m_type, v.m_data.num);
}

ALWAYS_INLINE void tvRefcountedDecRefNZ(TypedValue tv) {
  if (isRefcountedType(tv.m_type)) {
    TV_GENERIC_DISPATCH(tv, decRefCount);
  }
}

// Assumes 'tv' is live
// Assumes 'isRefcountedType(tv->m_type)'
ALWAYS_INLINE void tvDecRef(TypedValue* tv) {
  tvDecRefHelper(tv->m_type, tv->m_data.num);
}

// Assumes 'tv' is live
ALWAYS_INLINE void tvRefcountedDecRef(TypedValue* tv) {
  if (isRefcountedType(tv->m_type)) {
    tvDecRef(tv);
    // If we're in debug mode, turn the entry into null so that the GC doesn't
    // assert if it tries to follow it.
    if (debug) tv->m_type = KindOfNull;
  }
}

// decref when the count is known not to reach zero
ALWAYS_INLINE void tvDecRefOnly(TypedValue* tv) {
  assert(!tvDecRefWillCallHelper(tv));
  tvRefcountedDecRefNZ(*tv);
}

// Assumes 'tv' is live
ALWAYS_INLINE TypedValue* tvBox(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  assert(tv->m_type != KindOfRef);
  tv->m_data.pref = RefData::Make(*tv);
  tv->m_type = KindOfRef;
  return tv;
}

// Assumes 'tv' is live
// Assumes 'isRefcountedType(tv->m_type)'
ALWAYS_INLINE void tvIncRef(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  assert(isRefcountedType(tv->m_type));
  TV_GENERIC_DISPATCH(*tv, incRefCount);
}

ALWAYS_INLINE void tvRefcountedIncRef(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (isRefcountedType(tv->m_type)) {
    tvIncRef(tv);
  }
}

// Assumes 'tv' is live
// Assumes 'isRefcountedType(tv->m_type)'
// Assumes 'tv' is not shared (ie KindOfRef or KindOfObject)
ALWAYS_INLINE void tvIncRefNotShared(TypedValue* tv) {
  assert(tv->m_type == KindOfObject || tv->m_type == KindOfRef);
  tv->m_type == KindOfObject ?
    tv->m_data.pobj->incRefCount() :
    tv->m_data.pref->incRefCount();
}

// Assumes 'tv' is live
// Assumes 'tv.m_type == KindOfRef'
ALWAYS_INLINE void tvUnbox(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  assert(tv->m_type == KindOfRef);
  RefData* r = tv->m_data.pref;
  TypedValue* innerCell = r->tv();
  tv->m_data.num = innerCell->m_data.num;
  tv->m_type = innerCell->m_type;
  tvRefcountedIncRef(tv);
  tvDecRefRefInternal(r);
  assert(tvIsPlausible(*tv));
}

/*
 * Raw copy of a TypedValue from one location to another, without
 * doing any reference count manipulation.
 *
 * Copies the m_data and m_type fields, but not m_aux.  (For that you
 * need TypedValue::operator=.)
 */
ALWAYS_INLINE void tvCopy(const TypedValue& fr, TypedValue& to) {
  assert(tvIsPlausible(fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = fr.m_type;
}

/*
 * Equivalent of tvCopy for Cells and Vars.  These functions have the
 * same effects as tvCopy, but have some added assertions.
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
 * Duplicate a TypedValue to a new location. Copies the m_data and
 * m_type fields, and increments reference count. Does not perform a
 * decRef on to.
 */
ALWAYS_INLINE void tvDup(const TypedValue& fr, TypedValue& to) {
  tvCopy(fr, to);
  tvRefcountedIncRef(&to);
}

/*
 * Duplicate a Cell from one location to another. Copies the m_data and
 * m_type fields, and increments the reference count. Does not perform
 * a decRef on the value that was overwritten.
 */
ALWAYS_INLINE void cellDup(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
  tvRefcountedIncRef(&to);
}

/*
 * Duplicate a Ref from one location to another. Copies the m_data and m_type
 * fields and increments the reference count. Does not perform a decRef on the
 * value that was overwritten.
 */
ALWAYS_INLINE void refDup(const Ref fr, Ref& to) {
  assert(refIsPlausible(fr));
  to.m_type = KindOfRef;
  to.m_data.pref = fr.m_data.pref;
  to.m_data.pref->incRefCount();
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->m_aux
ALWAYS_INLINE void tvWriteNull(TypedValue* tv) {
  tv->m_type = KindOfNull;
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->m_aux
ALWAYS_INLINE void tvWriteUninit(TypedValue* tv) {
  tv->m_type = KindOfUninit;
}

// Assumes 'tv' is dead
ALWAYS_INLINE void tvWriteObject(ObjectData* pobj, TypedValue* tv) {
  tv->m_type = KindOfObject;
  tv->m_data.pobj = pobj;
  tv->m_data.pobj->incRefCount();
}

// Like tvWriteObject, but does not increment ref-count. Used for transferring
// object ownership.
ALWAYS_INLINE void tvMoveObject(ObjectData* pobj, TypedValue* tv) {
  tv->m_type = KindOfObject;
  tv->m_data.pobj = pobj;
}

// conditionally unbox tv
ALWAYS_INLINE Cell* tvToCell(TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}

// conditionally unbox tv, preserve constness.
ALWAYS_INLINE const Cell* tvToCell(const TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}

// assert that tv is cell
ALWAYS_INLINE Cell* tvAssertCell(TypedValue* tv) {
  assert(cellIsPlausible(*tv));
  return tv;
}
ALWAYS_INLINE const Cell* tvAssertCell(const TypedValue* tv) {
  assert(cellIsPlausible(*tv));
  return tv;
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate
 * reference count modifications.
 *
 * If `to' is KindOfRef, places the value of `fr' in the RefData
 * pointed to by `to'.
 *
 * `to' must contain a live php value; use cellDup when it doesn't.
 */
ALWAYS_INLINE void tvSet(const Cell fr, TypedValue& inTo) {
  assert(cellIsPlausible(fr));
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  cellDup(fr, *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// like tvSet, but RHS is bool
ALWAYS_INLINE void tvSetBool(bool val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  to->m_type = KindOfBoolean;
  to->m_data.num = val;
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// like tvSet, but RHS is int
ALWAYS_INLINE void tvSetInt(int64_t val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  to->m_type = KindOfInt64;
  to->m_data.num = val;
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// like tvSet, but RHS is double
ALWAYS_INLINE void tvSetDouble(double val, TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  to->m_type = KindOfDouble;
  to->m_data.dbl = val;
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assign the value of the Cell in `fr' to `to', decrementing the reference
 * count of 'to', but not incrementing 'to'.
 *
 * If `to' is KindOfRef, places the value of `fr' in the RefData
 * pointed to by `to'.
 *
 * `to' must contain a live php value; use cellCopy when it doesn't.
 */
ALWAYS_INLINE void tvMove(const Cell fr, TypedValue& inTo) {
  assert(cellIsPlausible(fr));
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  cellCopy(fr, *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assign null to `to', with appropriate reference count modifications.
 *
 * `to' must contain a live php value; use tvWriteNull when it doesn't.
 */
ALWAYS_INLINE void cellSetNull(Cell& to) {
  auto const oldType = to.m_type;
  auto const oldDatum = to.m_data.num;
  tvWriteNull(&to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assign null to `to', with appropriate reference count modifications.
 *
 * If `to' is KindOfRef, places the null in the RefData pointed to by `to'.
 *
 * `to' must contain a live php value; use tvWriteNull when it doesn't.
 */
ALWAYS_INLINE void tvSetNull(TypedValue& to) {
  cellSetNull(*tvToCell(&to));
}

/*
 * Assign the value of the Cell in `fr' to `to', with appropriate
 * reference count modifications.
 *
 * If `to' is KindOfRef, this function will decref the RefData and
 * replace it with the value in `fr', unlike tvSet.
 *
 * `to' must contain a live php value; use cellDup when it doesnt.
 *
 * Post: `to' is a Cell.
 */
ALWAYS_INLINE void tvSetIgnoreRef(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const oldType = to.m_type;
  auto const oldDatum = to.m_data.num;
  cellDup(fr, to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assigned the value of the Cell in `fr' to the Cell `to', with
 * appropriate reference count modifications.
 *
 * This function has the same effects as tvSetIgnoreRef, with stronger
 * assertions on `to'.
 */
ALWAYS_INLINE void cellSet(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(to));
  tvSetIgnoreRef(fr, to);
}

/*
 * Assign the value of the Cell in `fr' to `to', decrementing the reference
 * count of 'to', but not incrementing 'from'.
 *
 * If `to' is KindOfRef, this function will decref the RefData and
 * replace it with the value in `fr', unlike tvMove.
 *
 * `to' must contain a live php value; use cellCopy when it doesnt.
 *
 * Post: `to' is a Cell.
 */
ALWAYS_INLINE void tvMoveIgnoreRef(const Cell fr, TypedValue& to) {
  assert(cellIsPlausible(fr));
  auto const oldType = to.m_type;
  auto const oldDatum = to.m_data.num;
  cellCopy(fr, to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assign the value of the Cell in `fr' to the Cell `to', decrementing the
 * reference count of 'to', but not incrementing 'from'.
 *
 * This function has the same effects as tvMoveIgnoreRef, with stronger
 * assertions on `to'.
 */
ALWAYS_INLINE void cellMove(const Cell fr, Cell& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(to));
  tvMoveIgnoreRef(fr, to);
}

// Assumes 'to' and 'fr' are live
// Assumes that 'fr->m_type == KindOfRef'
ALWAYS_INLINE void tvBind(const TypedValue* fr, TypedValue* to) {
  assert(fr->m_type == KindOfRef);
  DataType oldType = to->m_type;
  uint64_t oldDatum = to->m_data.num;
  refDup(*fr, *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// Assumes 'to' and 'fr' are live
ALWAYS_INLINE void tvBindRef(RefData* fr, TypedValue* to) {
  auto const oldType  = to->m_type;
  auto const oldDatum = to->m_data.num;
  fr->incRefCount();
  tvCopy(make_tv<KindOfRef>(fr), *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Duplicate the value in `frIn' to `dst' in a reference-preserving
 * way.
 *
 * This means either the effects of tvDup(frIn, dst) or the effects of
 * cellDup(tvToCell(fIn), dst), depending on whether `frIn' is
 * "observably referenced"---i.e. if it KindOfRef and
 * RefData::isReferenced() is true.
 *
 * Pre: `frIn' is a live value, and `dst' is dead
 */
ALWAYS_INLINE
void tvDupWithRef(const TypedValue& frIn, TypedValue& dst) {
  assert(tvIsPlausible(frIn));
  auto fr = &frIn;
  if (UNLIKELY(fr->m_type == KindOfRef)) {
    if (!fr->m_data.pref->isReferenced()) {
      fr = fr->m_data.pref->tv();
    }
  }
  tvDup(*fr, dst);
}

// Assumes 'to' is live
ALWAYS_INLINE void tvUnset(TypedValue* to) {
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  tvWriteUninit(to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Returns true if this tv is not a ref-counted type, or if it is a
 * ref-counted type and the object pointed to is static.
 */
ALWAYS_INLINE bool tvIsStatic(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  return !isRefcountedType(tv->m_type) || TV_GENERIC_DISPATCH(*tv, isStatic);
}

/**
 * tvAsVariant and tvAsCVarRef serve as escape hatches that allow us to call
 * into the Variant machinery. Ideally we will use these as little as possible
 * in the long term.
 */

// Assumes 'tv' is live
ALWAYS_INLINE Variant& tvAsVariant(TypedValue* tv) {
  assert(tv != nullptr);
  assert(tvIsPlausible(*tv));
  return *(Variant*)(tv);
}

ALWAYS_INLINE Variant& tvAsUninitializedVariant(TypedValue* tv) {
  // A special case, for use when constructing a variant and we don't
  // assume initialization.
  return *(Variant*)(tv);
}

// Assumes 'tv' is live
ALWAYS_INLINE const Variant& tvAsCVarRef(const TypedValue* tv) {
  return *(const Variant*)(tv);
}

// Assumes 'tv' is live
ALWAYS_INLINE Variant& cellAsVariant(Cell& cell) {
  assert(cellIsPlausible(cell));
  return *(Variant*)(&cell);
}

// Assumes 'tv' is live
ALWAYS_INLINE const Variant& cellAsCVarRef(const Cell& cell) {
  assert(cellIsPlausible(cell));
  return *(const Variant*)(&cell);
}

// Assumes 'tv' is live
ALWAYS_INLINE Variant& refAsVariant(Ref& ref) {
  assert(refIsPlausible(ref));
  return *(Variant*)(&ref);
}

// Assumes 'tv' is live
ALWAYS_INLINE const Variant& refAsCVarRef(const Ref& ref) {
  assert(refIsPlausible(ref));
  return *(const Variant*)(&ref);
}

ALWAYS_INLINE bool tvIsStronglyBound(const TypedValue* tv) {
  return (tv->m_type == KindOfRef && tv->m_data.pref->isReferenced());
}

template<class Fun>
void tvDupFlattenImpl(const TypedValue* fr, TypedValue* to, Fun shouldFlatten) {
  tvCopy(*fr, *to);
  auto type = fr->m_type;
  if (!isRefcountedType(type)) return;
  if (type != KindOfRef) {
    tvIncRef(to);
    return;
  }
  auto ref = fr->m_data.pref;
  if (shouldFlatten(ref)) {
    cellDup(*ref->tv(), *to);
    return;
  }
  tvIncRefNotShared(to);
}

// Assumes 'fr' is live and 'to' is dead, and does not mutate to->m_aux
ALWAYS_INLINE void tvDupFlattenVars(const TypedValue* fr, TypedValue* to) {
  tvDupFlattenImpl(fr, to, [&](RefData* ref) {
    return !ref->isReferenced();
  });
}

// Assumes 'fr' is live and 'to' is dead, and does not mutate to->m_aux
ALWAYS_INLINE void tvDupFlattenVars(const TypedValue* fr, TypedValue* to,
                             const ArrayData* container) {
  assert(container);
  tvDupFlattenImpl(fr, to, [&](RefData* ref) {
    return !ref->isReferenced() && container != ref->tv()->m_data.parr;
  });
}

ALWAYS_INLINE bool cellIsNull(const Cell* tv) {
  assert(cellIsPlausible(*tv));
  return isNullType(tv->m_type);
}

ALWAYS_INLINE bool tvIsString(const TypedValue* tv) {
  return (tv->m_type & KindOfStringBit) != 0;
}

ALWAYS_INLINE void tvUnboxIfNeeded(TypedValue* tv) {
  if (tv->m_type == KindOfRef) tvUnbox(tv);
}

// Used when adding an array element.
ALWAYS_INLINE void initVal(TypedValue& tv, Cell v) {
  cellDup(v, tv);
  if (UNLIKELY(tv.m_type == KindOfUninit)) {
    tv.m_type = KindOfNull;
  }
}

// Used when changing an array element.
ALWAYS_INLINE void setVal(TypedValue& tv, Cell src) {
  auto const dst = tvToCell(&tv);
  if (UNLIKELY(src.m_type == KindOfUninit)) {
    src.m_type = KindOfNull;
  }
  cellSet(src, *dst);
}

/*
 * TypedValue conversions that update the tv in place (decrefing and
 * old value, if necessary).
 *
 * CastInPlace will forcibly change the value to the new type
 *   and will not fail. (Though the results may be silly)
 * CoerceInPlace will attempt to convert the type and
 *   return false on failure
 * CoerceOrThrow will attempt to convert the type and
 *   both raise a warning and throw a TVCoercionException on failure
 */
void cellCastToInt64InPlace(Cell*);
double tvCastToDouble(TypedValue* tv);
StringData* tvCastToString(const TypedValue* tv);
bool tvCanBeCoercedToNumber(TypedValue* tv);

/*
 * If the current function (func, a builtin) was called in a strict context then
 * verify that tv is the correct type for argNum or attempt to convert it to
 * the correct type, fataling on failure.
 *
 * If PHP7_ScalarType is false or EnableHipHopSyntax is true, this call does
 * nothing.
 */
void tvCoerceIfStrict(TypedValue& tv, int64_t argNum, const Func* func);

#define X(kind) \
void tvCastTo##kind##InPlace(TypedValue* tv); \
bool tvCoerceParamTo##kind##InPlace(TypedValue* tv); \
void tvCoerceParamTo##kind##OrThrow(TypedValue* tv, \
                                    const Func* callee, \
                                    unsigned int arg_num);
X(Boolean)
X(Int64)
X(Double)
X(String)
X(Array)
X(Object)
X(NullableObject)
X(Resource)
#undef X

ALWAYS_INLINE void tvCastInPlace(TypedValue *tv, DataType DType) {
#define X(kind) \
  if (DType == KindOf##kind) { tvCastTo##kind##InPlace(tv); return; }
  X(Boolean)
  X(Int64)
  X(Double)
  X(String)
  X(Array)
  X(Object)
  X(Resource)
#undef X
  not_reached();
}

ALWAYS_INLINE bool tvCoerceParamInPlace(TypedValue* tv, DataType DType) {
#define X(kind) \
  if (DType == KindOf##kind) return tvCoerceParamTo##kind##InPlace(tv);
  X(Boolean)
  X(Int64)
  X(Double)
  X(String)
  X(Array)
  X(Object)
  X(Resource)
#undef X
  not_reached();
}

/*
 * TVCoercionException is thrown to indicate that a parameter could not be
 * coerced when calling an HNI builtin function.
 */
struct TVCoercionException : std::runtime_error {
  TVCoercionException(const Func* func, int arg_num,
                      DataType actual, DataType expected);
  TypedValue tv() const { return m_tv; }
private:
  req::root<TypedValue> m_tv;
};

// for debugging & instrumentation only! otherwise, stick to the
// predicates defined in countable.h
ALWAYS_INLINE RefCount tvGetCount(const TypedValue* tv) {
  assert(isRefcountedType(tv->m_type));
  return reinterpret_cast<const HeaderWord<>*>(
    reinterpret_cast<const char*>(tv->m_data.parr) + HeaderOffset
  )->count;
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TV_HELPERS_H_
