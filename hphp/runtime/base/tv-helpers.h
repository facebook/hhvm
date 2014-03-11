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

#ifndef incl_HPHP_TV_HELPERS_H_
#define incl_HPHP_TV_HELPERS_H_

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/ref-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Variant;

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
inline void tvDebugTrash(TypedValue* tv) {
  if (debug) memset(tv, kTVTrashFill, sizeof *tv);
}

/*
 * Returns: true if the supplied TypedValue is KindOfDouble or
 * KindOfInt64.  I.e. if it is a TypedNum.
 */
inline bool isTypedNum(const TypedValue& tv) {
  return tv.m_type == KindOfInt64 || tv.m_type == KindOfDouble;
}

/*
 * Returns: true if the supplied TypedValue is a Cell, and either has
 * a non-reference counted type, or is a KindOfString that points to a
 * static string.
 */
inline bool isUncounted(const TypedValue& tv) {
  auto const uncounted = !IS_REFCOUNTED_TYPE(tv.m_type) ||
    (tv.m_type == KindOfString && tv.m_data.pstr->isStatic());
  if (uncounted) assert(cellIsPlausible(tv));
  return uncounted;
}

// Assumes 'data' is live
// Assumes 'IS_REFCOUNTED_TYPE(type)'
void tvDecRefHelper(DataType type, uint64_t datum);

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
inline bool tvDecRefWillCallHelper(TypedValue* tv) {
  return IS_REFCOUNTED_TYPE(tv->m_type) &&
    !tv->m_data.pstr->hasMultipleRefs();
}

// Assumes 'tv' is live
inline void tvRefcountedDecRefCell(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tvDecRefHelper(tv->m_type, tv->m_data.num);
  }
}

inline void tvDecRefStr(TypedValue* tv) {
  assert(tv->m_type == KindOfString);
  decRefStr(tv->m_data.pstr);
}

inline void tvDecRefArr(TypedValue* tv) {
  assert(tv->m_type == KindOfArray);
  decRefArr(tv->m_data.parr);
}

inline void tvDecRefObj(TypedValue* tv) {
  assert(tv->m_type == KindOfObject);
  decRefObj(tv->m_data.pobj);
}

inline void tvDecRefRes(TypedValue* tv) {
  assert(tv->m_type == KindOfResource);
  decRefRes(tv->m_data.pres);
}

// Assumes 'r' is live and points to a RefData
inline void tvDecRefRefInternal(RefData* r) {
  assert(tvIsPlausible(*r->tv()));
  assert(r->tv()->m_type != KindOfRef);
  assert(r->getRealCount() > 0);
  decRefRef(r);
}

// Assumes 'tv' is live
inline void tvDecRefRef(TypedValue* tv) {
  assert(tv->m_type == KindOfRef);
  tvDecRefRefInternal(tv->m_data.pref);
}

// Assumes 'tv' is live
inline void tvRefcountedDecRefHelper(DataType type, uint64_t datum) {
  if (IS_REFCOUNTED_TYPE(type)) {
    tvDecRefHelper(type, datum);
  }
}

inline void tvRefcountedDecRef(TypedValue v) {
  return tvRefcountedDecRefHelper(v.m_type, v.m_data.num);
}

// Assumes the value is live, and has a count of at least 2 coming in.
inline void tvRefcountedDecRefHelperNZ(DataType type, uint64_t datum) {
  if (IS_REFCOUNTED_TYPE(type)) {
    auto* asStr = reinterpret_cast<StringData*>(datum);
    auto const DEBUG_ONLY newCount = asStr->decRefCount();
    assert(newCount != 0);
  }
}

inline void tvRefcountedDecRefNZ(TypedValue v) {
  return tvRefcountedDecRefHelperNZ(v.m_type, v.m_data.num);
}

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
inline void tvDecRef(TypedValue* tv) {
  tvDecRefHelper(tv->m_type, tv->m_data.num);
}

// Assumes 'tv' is live
ALWAYS_INLINE void tvRefcountedDecRef(TypedValue* tv) {
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tvDecRef(tv);
  }
}

// decref when the count is known not to reach zero
ALWAYS_INLINE void tvDecRefOnly(TypedValue* tv) {
  assert(!tvDecRefWillCallHelper(tv));
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tv->m_data.pstr->decRefCount();
  }
}

// Assumes 'tv' is live
inline TypedValue* tvBox(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  assert(tv->m_type != KindOfRef);
  tv->m_data.pref = RefData::Make(*tv);
  tv->m_type = KindOfRef;
  return tv;
}

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
inline void tvIncRef(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  assert(IS_REFCOUNTED_TYPE(tv->m_type));
  tv->m_data.pstr->incRefCount();
}

ALWAYS_INLINE void tvRefcountedIncRef(TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tvIncRef(tv);
  }
}

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
// Assumes 'tv' is not shared (ie KindOfRef or KindOfObject)
inline void tvIncRefNotShared(TypedValue* tv) {
  assert(tv->m_type == KindOfObject || tv->m_type == KindOfRef);
  tv->m_data.pobj->incRefCount();
}

// Assumes 'tv' is live
// Assumes 'tv.m_type == KindOfRef'
inline void tvUnbox(TypedValue* tv) {
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
inline void tvCopy(const TypedValue& fr, TypedValue& to) {
  assert(tvIsPlausible(fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = fr.m_type;
}

/*
 * Equivalent of tvCopy for Cells and Vars.  These functions have the
 * same effects as tvCopy, but have some added assertions.
 */
inline void cellCopy(const Cell& fr, Cell& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
}
inline void refCopy(const Ref& fr, Ref& to) {
  assert(refIsPlausible(fr));
  tvCopy(fr, to);
}

/*
 * Duplicate a TypedValue to a new location. Copies the m_data and
 * m_type fields, and increments reference count. Does not perform a
 * decRef on to.
 */
inline void tvDup(const TypedValue& fr, TypedValue& to) {
  tvCopy(fr, to);
  tvRefcountedIncRef(&to);
}

/*
 * Duplicate a Cell from one location to another. Copies the m_data and
 * m_type fields, and increments the reference count. Does not perform
 * a decRef on the value that was overwritten.
 */
inline void cellDup(const Cell& fr, Cell& to) {
  assert(cellIsPlausible(fr));
  tvCopy(fr, to);
  tvRefcountedIncRef(&to);
}

/*
 * Duplicate a Ref from one location to another. Copies the m_data and
 * m_type fields and increments the reference count. Does not perform
 * as decRef on the value that was overwritten.
 */
inline void refDup(const Ref& fr, Ref& to) {
  assert(refIsPlausible(fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = KindOfRef;
  tvIncRefNotShared(&to);
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->m_aux
inline void tvWriteNull(TypedValue* tv) {
  tv->m_type = KindOfNull;
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->m_aux
inline void tvWriteUninit(TypedValue* tv) {
  tv->m_type = KindOfUninit;
}

// Assumes 'tv' is dead
inline void tvWriteObject(ObjectData* pobj, TypedValue* tv) {
  tv->m_type = KindOfObject;
  tv->m_data.pobj = pobj;
  tvIncRef(tv);
}

// conditionally unbox tv
inline Cell* tvToCell(TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}

// conditionally unbox tv, preserve constness.
inline const Cell* tvToCell(const TypedValue* tv) {
  return LIKELY(tv->m_type != KindOfRef) ? tv : tv->m_data.pref->tv();
}

// assert that tv is cell
inline Cell* tvAssertCell(TypedValue* tv) {
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
inline void tvSet(const Cell& fr, TypedValue& inTo) {
  assert(cellIsPlausible(fr));
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  cellDup(fr, *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

/*
 * Assign null to `to', with appropriate reference count modifications.
 *
 * If `to' is KindOfRef, places the null in the RefData pointed to by `to'.
 *
 * `to' must contain a live php value; use tvWriteNull when it doesn't.
 */
inline void tvSetNull(TypedValue& inTo) {
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  tvWriteNull(to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
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
inline void tvSetIgnoreRef(const Cell& fr, TypedValue& to) {
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
inline void cellSet(const Cell& fr, Cell& to) {
  assert(cellIsPlausible(fr));
  assert(cellIsPlausible(to));
  tvSetIgnoreRef(fr, to);
}

// Assumes 'to' and 'fr' are live
// Assumes that 'fr->m_type == KindOfRef'
inline void tvBind(const TypedValue* fr, TypedValue* to) {
  assert(fr->m_type == KindOfRef);
  DataType oldType = to->m_type;
  uint64_t oldDatum = to->m_data.num;
  refDup(*fr, *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// Assumes 'to' and 'fr' are live
inline void tvBindRef(RefData* fr, TypedValue* to) {
  auto const oldType  = to->m_type;
  auto const oldDatum = to->m_data.num;
  fr->incRefCount();
  tvCopy(make_tv<KindOfRef>(fr), *to);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// Assumes 'to' is live
inline void tvUnset(TypedValue * to) {
  tvRefcountedDecRef(to);
  tvWriteUninit(to);
}

// Assumes `fr' is dead and binds it using KindOfIndirect to `to'.
inline void tvBindIndirect(TypedValue* fr, TypedValue* to) {
  assert(tvIsPlausible(*to));
  fr->m_type = KindOfIndirect;
  fr->m_data.pind = to;
}

// If a TypedValue is KindOfIndirect, dereference to the inner
// TypedValue.
inline TypedValue* tvDerefIndirect(TypedValue* tv) {
  return tv->m_type == KindOfIndirect ? tv->m_data.pind : tv;
}
inline const TypedValue* tvDerefIndirect(const TypedValue* tv) {
  return tvDerefIndirect(const_cast<TypedValue*>(tv));
}

/*
 * Returns true if this tv is not a ref-counted type, or if it is a
 * ref-counted type and the object pointed to is static.
 */
inline bool tvIsStatic(const TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  return !IS_REFCOUNTED_TYPE(tv->m_type) ||
    tv->m_data.pref->m_count == StaticValue;
}

/**
 * tvAsVariant and tvAsconst Variant& serve as escape hatches that allow us to call
 * into the Variant machinery. Ideally we will use these as little as possible
 * in the long term.
 */

// Assumes 'tv' is live
inline Variant& tvAsVariant(TypedValue* tv) {
  // Avoid treating uninitialized TV's as variants. We have some slightly
  // perverse, but defensible uses where we pass in NULL (and later check
  // a Variant* against NULL) so tolerate it.
  assert(nullptr == tv || tvIsPlausible(*tv));
  return *(Variant*)(tv);
}

inline Variant& tvAsUninitializedVariant(TypedValue* tv) {
  // A special case, for use when constructing a variant and we don't
  // assume initialization.
  return *(Variant*)(tv);
}

// Assumes 'tv' is live
inline const Variant& tvAsCVarRef(const TypedValue* tv) {
  return *(const Variant*)(tv);
}

// Assumes 'tv' is live
inline Variant& cellAsVariant(Cell& cell) {
  assert(cellIsPlausible(cell));
  return *(Variant*)(&cell);
}

// Assumes 'tv' is live
inline const Variant& cellAsCVarRef(const Cell& cell) {
  assert(cellIsPlausible(cell));
  return *(const Variant*)(&cell);
}

// Assumes 'tv' is live
inline Variant& refAsVariant(Ref& ref) {
  assert(refIsPlausible(ref));
  return *(Variant*)(&ref);
}

// Assumes 'tv' is live
inline const Variant& refAsCVarRef(const Ref& ref) {
  assert(refIsPlausible(ref));
  return *(const Variant*)(&ref);
}

inline bool tvIsStronglyBound(const TypedValue* tv) {
  return (tv->m_type == KindOfRef && tv->m_data.pref->isReferenced());
}

template<class Fun>
void tvDupFlattenImpl(const TypedValue* fr, TypedValue* to, Fun shouldFlatten) {
  tvCopy(*fr, *to);
  auto type = fr->m_type;
  if (!IS_REFCOUNTED_TYPE(type)) return;
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
inline void tvDupFlattenVars(const TypedValue* fr, TypedValue* to) {
  tvDupFlattenImpl(fr, to, [&](RefData* ref) {
    return !ref->isReferenced();
  });
}

// Assumes 'fr' is live and 'to' is dead, and does not mutate to->m_aux
inline void tvDupFlattenVars(const TypedValue* fr, TypedValue* to,
                             const ArrayData* container) {
  assert(container);
  tvDupFlattenImpl(fr, to, [&](RefData* ref) {
    return !ref->isReferenced() && container != ref->tv()->m_data.parr;
  });
}

inline bool cellIsNull(const Cell* tv) {
  assert(cellIsPlausible(*tv));
  return IS_NULL_TYPE(tv->m_type);
}

inline bool tvIsString(const TypedValue* tv) {
  return (tv->m_type & KindOfStringBit) != 0;
}

inline void tvUnboxIfNeeded(TypedValue* tv) {
  if (tv->m_type == KindOfRef) tvUnbox(tv);
}

/*
 * TypedValue conversions that update the tv in place (decrefing and
 * old value, if necessary).
 */
void tvCastToBooleanInPlace(TypedValue* tv);
void tvCastToInt64InPlace(TypedValue* tv);
void cellCastToInt64InPlace(Cell*);
void tvCastToDoubleInPlace(TypedValue* tv);
double tvCastToDouble(TypedValue* tv);
void tvCastToStringInPlace(TypedValue* tv);
StringData* tvCastToString(const TypedValue* tv);
void tvCastToArrayInPlace(TypedValue* tv);
void tvCastToObjectInPlace(TypedValue* tv);
void tvCastToResourceInPlace(TypedValue* tv);

bool tvCanBeCoercedToNumber(TypedValue* tv);
bool tvCoerceParamToBooleanInPlace(TypedValue* tv);
bool tvCoerceParamToInt64InPlace(TypedValue* tv);
bool tvCoerceParamToDoubleInPlace(TypedValue* tv);
bool tvCoerceParamToStringInPlace(TypedValue* tv);
bool tvCoerceParamToArrayInPlace(TypedValue* tv);
bool tvCoerceParamToObjectInPlace(TypedValue* tv);
bool tvCoerceParamToResourceInPlace(TypedValue* tv);

typedef void(*RawDestructor)(void*);
extern const RawDestructor g_destructors[kDestrTableSize];

inline void tvCastInPlace(TypedValue *tv, DataType DType) {
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

inline bool tvCoerceParamInPlace(TypedValue* tv, DataType DType) {
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TV_HELPERS_H_
