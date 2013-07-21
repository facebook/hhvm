/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_TV_HELPERS_H_
#define incl_HPHP_TV_HELPERS_H_

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'tv_helpers.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Variant;

/*
 * Assertions on Cells and TypedValues.  Should usually only happen
 * inside an assert().
 */
bool tvIsPlausible(const TypedValue*);
bool cellIsPlausible(const Cell*);
bool refIsPlausible(const Ref*);

/*
 * Returns: true if the supplied TypedValue is KindOfDouble or
 * KindOfInt64.  I.e. if it is a TypedNum.
 */
inline bool isTypedNum(const TypedValue& tv) {
  return tv.m_type == KindOfInt64 || tv.m_type == KindOfDouble;
}

// Assumes 'data' is live
// Assumes 'IS_REFCOUNTED_TYPE(type)'
void tvDecRefHelper(DataType type, uint64_t datum);

inline bool tvWillBeReleased(TypedValue* tv) {
  return IS_REFCOUNTED_TYPE(tv->m_type) &&
         tv->m_data.pstr->getCount() <= 1;
}

// Assumes 'tv' is live
inline void tvRefcountedDecRefCell(TypedValue* tv) {
  assert(tvIsPlausible(tv));
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

// Assumes 'r' is live and points to a RefData
inline void tvDecRefRefInternal(RefData* r) {
  assert(tvIsPlausible(r->tv()));
  assert(r->tv()->m_type != KindOfRef);
  assert(r->_count > 0);
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

// Assumes 'tv' is live
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
inline void tvDecRef(TypedValue* tv) {
  tvDecRefHelper(tv->m_type, tv->m_data.num);
}

// Assumes 'tv' is live
ALWAYS_INLINE inline void tvRefcountedDecRef(TypedValue* tv) {
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tvDecRef(tv);
  }
}

// decref when the count is known not to reach zero
ALWAYS_INLINE inline void tvDecRefOnly(TypedValue* tv) {
  assert(!tvWillBeReleased(tv));
  if (IS_REFCOUNTED_TYPE(tv->m_type)) {
    tv->m_data.pstr->decRefCount();
  }
}

// tvBoxHelper sets the refcount of the newly allocated inner cell to 1
inline RefData* tvBoxHelper(DataType type, uint64_t datum) {
  return NEW(RefData)(type, datum);
}

// Assumes 'tv' is live
inline TypedValue* tvBox(TypedValue* tv) {
  assert(tvIsPlausible(tv));
  assert(tv->m_type != KindOfRef);
  tv->m_data.pref = tvBoxHelper(tv->m_type, tv->m_data.num);
  tv->m_type = KindOfRef;
  return tv;
}

// Assumes 'tv' is live
//
// Assumes 'IS_REFCOUNTED_TYPE(tv->m_type)'
inline void tvIncRef(TypedValue* tv) {
  assert(tvIsPlausible(tv));
  assert(IS_REFCOUNTED_TYPE(tv->m_type));
  tv->m_data.pstr->incRefCount();
}

ALWAYS_INLINE inline void tvRefcountedIncRef(TypedValue* tv) {
  assert(tvIsPlausible(tv));
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
  assert(tvIsPlausible(tv));
  assert(tv->m_type == KindOfRef);
  RefData* r = tv->m_data.pref;
  TypedValue* innerCell = r->tv();
  tv->m_data.num = innerCell->m_data.num;
  tv->m_type = innerCell->m_type;
  tvRefcountedIncRef(tv);
  tvDecRefRefInternal(r);
  assert(tvIsPlausible(tv));
}

// Assumes 'fr' is live and 'to' is dead. Store a reference to 'fr',
// as a Cell, into 'to'.
inline void tvReadCell(const TypedValue* fr, TypedValue* to) {
  assert(tvIsPlausible(fr));
  if (fr->m_type != KindOfRef) {
    memcpy(to, fr, sizeof(TypedValue));
  } else {
    TypedValue* fr2 = fr->m_data.pref->tv();
    to->m_data.num = fr2->m_data.num;
    to->m_type = fr2->m_type;
  }
  tvRefcountedIncRef(to);
}

/*
 * Raw copy of a TypedValue from one location to another, without
 * doing any reference count manipulation.
 *
 * Copies the m_data and m_type fields, but not m_aux.  (For that you
 * need TypedValue::operator=.)
 */
inline void tvCopy(const TypedValue& fr, TypedValue& to) {
  assert(tvIsPlausible(&fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = fr.m_type;
}

/*
 * Equivalent of tvCopy for Cells and Vars.  These functions have the
 * same effects as tvCopy, but have some added assertions.
 */
inline void cellCopy(const Cell& fr, Cell& to) {
  assert(cellIsPlausible(&fr));
  tvCopy(fr, to);
}
inline void refCopy(const Ref& fr, Ref& to) {
  assert(refIsPlausible(&fr));
  tvCopy(fr, to);
}

/*
 * Duplicate a TypedValue to a new location.  Copies the m_data and
 * m_type fields, and increments reference count.  Does not perform a
 * decRef on to.
 */
inline void tvDup(const TypedValue& fr, TypedValue& to) {
  tvCopy(fr, to);
  tvRefcountedIncRef(&to);
}

/*
 * Duplicate a Cell from one location to another.  Is equivalent to
 * tvDup, with some added assertions.
 */
inline void cellDup(const Cell& fr, Cell& to) {
  assert(cellIsPlausible(&fr));
  tvDup(fr, to);
}

/*
 * Duplicate a Ref from one location to another.
 *
 * This has the same effects as tvDup(fr, to), but is slightly more
 * efficient because we don't need to check the type tag.
 */
inline void refDup(const Ref& fr, Ref& to) {
  assert(refIsPlausible(&fr));
  to.m_data.num = fr.m_data.num;
  to.m_type = KindOfRef;
  tvIncRefNotShared(&to);
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->_count
inline void tvWriteNull(TypedValue* tv) {
  tv->m_type = KindOfNull;
}

// Assumes 'tv' is dead
// NOTE: this helper does not modify tv->_count
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
  assert(cellIsPlausible(tv));
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
  assert(cellIsPlausible(&fr));
  Cell* to = tvToCell(&inTo);
  auto const oldType = to->m_type;
  auto const oldDatum = to->m_data.num;
  cellDup(fr, *to);
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
  assert(cellIsPlausible(&fr));
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
  assert(cellIsPlausible(&fr));
  assert(cellIsPlausible(&to));
  tvSetIgnoreRef(fr, to);
}

// Assumes 'to' and 'fr' are live
// Assumes that 'fr->m_type == KindOfRef'
inline void tvBind(TypedValue* fr, TypedValue* to) {
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
  tvCopy(make_tv<KindOfRef>(fr), *to);
  fr->incRefCount();
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

// Assumes 'to' is live
inline void tvUnset(TypedValue * to) {
  tvRefcountedDecRef(to);
  tvWriteUninit(to);
}

// Assumes `fr' is dead and binds it using KindOfIndirect to `to'.
inline void tvBindIndirect(TypedValue* fr, TypedValue* to) {
  assert(tvIsPlausible(to));
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
  assert(tvIsPlausible(tv));
  return !IS_REFCOUNTED_TYPE(tv->m_type) ||
    tv->m_data.pref->_count == RefCountStaticValue;
}

/**
 * tvAsVariant and tvAsCVarRef serve as escape hatches that allow us to call
 * into the Variant machinery. Ideally we will use these as little as possible
 * in the long term.
 */

// Assumes 'tv' is live
inline Variant& tvAsVariant(TypedValue* tv) {
  // Avoid treating uninitialized TV's as variants. We have some slightly
  // perverse, but defensible uses where we pass in NULL (and later check
  // a Variant* against NULL) so tolerate it.
  assert(nullptr == tv || tvIsPlausible(tv));
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
  assert(cellIsPlausible(&cell));
  return *(Variant*)(&cell);
}

// Assumes 'tv' is live
inline const Variant& cellAsCVarRef(const Cell& cell) {
  assert(cellIsPlausible(&cell));
  return *(const Variant*)(&cell);
}

// Assumes 'tv' is live
inline Variant& refAsVariant(Ref& ref) {
  assert(refIsPlausible(&ref));
  return *(Variant*)(&ref);
}

// Assumes 'tv' is live
inline const Variant& refAsCVarRef(const Ref& ref) {
  assert(refIsPlausible(&ref));
  return *(const Variant*)(&ref);
}

inline bool tvIsStronglyBound(const TypedValue* tv) {
  return (tv->m_type == KindOfRef && tv->m_data.pref->_count > 1);
}

// Assumes 'fr' is live and 'to' is dead, and does not mutate to->_count
inline void tvDupFlattenVars(const TypedValue* fr, TypedValue* to,
                             const ArrayData* container) {
  if (LIKELY(fr->m_type != KindOfRef)) {
    cellDup(*fr, *to);
  } else if (fr->m_data.pref->_count <= 1 &&
             (!container || fr->m_data.pref->tv()->m_data.parr != container)) {
    fr = fr->m_data.pref->tv();
    cellDup(*fr, *to);
  } else {
    refDup(*fr, *to);
  }
}

inline bool tvIsString(const TypedValue* tv) {
  return (tv->m_type & KindOfStringBit) != 0;
}

void tvUnboxIfNeeded(TypedValue* tv);

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
StringData* tvCastToString(TypedValue* tv);
void tvCastToArrayInPlace(TypedValue* tv);
void tvCastToObjectInPlace(TypedValue* tv);

bool tvCanBeCoercedToNumber(TypedValue* tv);
bool tvCoerceParamToBooleanInPlace(TypedValue* tv);
bool tvCoerceParamToInt64InPlace(TypedValue* tv);
bool tvCoerceParamToDoubleInPlace(TypedValue* tv);
bool tvCoerceParamToStringInPlace(TypedValue* tv);
bool tvCoerceParamToArrayInPlace(TypedValue* tv);
bool tvCoerceParamToObjectInPlace(TypedValue* tv);

typedef void(*RawDestructor)(void*);
extern const RawDestructor g_destructors[kDestrTableSize];

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TV_HELPERS_H_
