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

#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/portability.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * NOTE: MixedArray no longer calls this constructor.  If you change it, change
 * the MixedArray::Make functions as appropriate.
 */
inline ArrayData::ArrayData(ArrayKind kind, RefCount initial_count)
  : m_sizeAndPos(uint32_t(-1))
{
  initHeader(static_cast<HeaderKind>(kind), initial_count);
  assertx(m_size == -1);
  assertx(m_pos == 0);
  assertx(m_kind == static_cast<HeaderKind>(kind));
  assertx(dvArraySanityCheck());
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE ArrayData* staticEmptyArray() {
  void* vp = &s_theEmptyArray;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyVArray() {
  void* vp1 = &s_theEmptyVArray;
  void* vp2 = &s_theEmptyVecArray;
  return static_cast<ArrayData*>(RuntimeOption::EvalHackArrDVArrs ? vp2 : vp1);
}

ALWAYS_INLINE ArrayData* staticEmptyVecArray() {
  void* vp = &s_theEmptyVecArray;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyDArray() {
  void* vp1 = &s_theEmptyDArray;
  void* vp2 = &s_theEmptyDictArray;
  return static_cast<ArrayData*>(RuntimeOption::EvalHackArrDVArrs ? vp2 : vp1);
}

ALWAYS_INLINE ArrayData* staticEmptyDictArray() {
  void* vp = &s_theEmptyDictArray;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyShapeArray() {
  void* vp = RuntimeOption::EvalHackArrDVArrs
    ? &s_theEmptyShapeDict : &s_theEmptyShapeDArray;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyKeysetArray() {
  void* vp = &s_theEmptySetArray;
  return static_cast<ArrayData*>(vp);
}

///////////////////////////////////////////////////////////////////////////////
// Creation and destruction.

ALWAYS_INLINE ArrayData* ArrayData::Create() {
  return staticEmptyArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVArray() {
  return staticEmptyVArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVec() {
  return staticEmptyVecArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDArray() {
  return staticEmptyDArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDict() {
  return staticEmptyDictArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateShape() {
  return staticEmptyShapeArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateKeyset() {
  return staticEmptyKeysetArray();
}

ALWAYS_INLINE void ArrayData::decRefAndRelease() {
  assertx(kindIsValid());
  if (decReleaseCheck()) release();
}

///////////////////////////////////////////////////////////////////////////////
// ArrayFunction dispatch.

inline size_t ArrayData::vsize() const {
  return g_array_funcs.vsize[kind()](this);
}

///////////////////////////////////////////////////////////////////////////////
// Introspection.

inline size_t ArrayData::size() const {
  if (UNLIKELY((int)m_size) < 0) return vsize();
  return m_size;
}

inline size_t ArrayData::getSize() const {
  return m_size;
}

inline bool ArrayData::empty() const {
  return size() == 0;
}

inline bool ArrayData::kindIsValid() const {
  return isArrayKind(m_kind);
}

inline ArrayData::ArrayKind ArrayData::kind() const {
  assertx(kindIsValid());
  return static_cast<ArrayKind>(m_kind);
}

inline bool ArrayData::noCopyOnWrite() const {
  // GlobalsArray doesn't support COW.
  return kind() == kGlobalsKind;
}

inline bool ArrayData::isPacked() const { return kind() == kPackedKind; }
inline bool ArrayData::isMixed() const { return kind() == kMixedKind; }

/*
 * isMixedOrShape checks whether the ArrayData is kMixedKind or a Shape that
 * behaves like a mixed PHP Array. This is important because this check is
 * often used to check that a piece of code is only operating on
 * mixed PHP array-like objects and not dict-like objects.
 */
inline bool ArrayData::isMixedOrShape() const {
  return kind() == kMixedKind ||
    (!RuntimeOption::EvalHackArrDVArrs && kind() == kShapeKind);
}
inline bool ArrayData::isApcArray() const { return kind() == kApcKind; }
inline bool ArrayData::isGlobalsArray() const { return kind() == kGlobalsKind; }
inline bool ArrayData::isEmptyArray() const { return kind() == kEmptyKind; }
inline bool ArrayData::isDict() const { return kind() == kDictKind; }

/*
 * isDictOrShape checks whether the ArrayData is a dict or a Shape that
 * behaves like a dict. This is important because this check is often used
 * to check that a piece of code is only operating on dict-like objects and
 * not array-like objects.
 */
inline bool ArrayData::isDictOrShape() const {
  return kind() == kDictKind ||
    (RuntimeOption::EvalHackArrDVArrs && kind() == kShapeKind);
}
inline bool ArrayData::isVecArray() const { return kind() == kVecKind; }
inline bool ArrayData::isKeyset() const { return kind() == kKeysetKind; }
inline bool ArrayData::isShape() const { return kind() == kShapeKind; }

inline bool ArrayData::hasPackedLayout() const {
  return isPacked() || isVecArray();
}
inline bool ArrayData::hasMixedLayout() const {
  return isMixed() || isDict() || isShape();
}

inline bool ArrayData::isPHPArray() const {
  return RuntimeOption::EvalHackArrDVArrs
    ? kind() < kShapeKind
    : kind() <= kShapeKind;
}

inline bool ArrayData::isHackArray() const {
  return RuntimeOption::EvalHackArrDVArrs
    ? kind() >= kShapeKind
    : kind() >= kDictKind;
}

inline ArrayData::DVArray ArrayData::dvArray() const {
  // The darray/varray state is stored in the lower 2-bits of m_aux16. The
  // array is free to store whatever it wants in the upper 14-bits.
  return static_cast<DVArray>(m_aux16 & kDVArrayMask);
}

inline void ArrayData::setDVArray(DVArray d) {
  assertx(!RuntimeOption::EvalHackArrDVArrs || d == kNotDVArray);
  assertx(!(d & ~kDVArrayMask));
  m_aux16 = (m_aux16 & ~kDVArrayMask) | d;
}

inline bool ArrayData::isVArray() const { return dvArray() & kVArray; }
inline bool ArrayData::isDArray() const { return dvArray() & kDArray; }

inline bool ArrayData::isNotDVArray() const { return dvArray() == kNotDVArray; }
inline bool ArrayData::isVecOrVArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isVecArray() : isVArray();
}
inline bool ArrayData::isDictOrDArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isDict() : isDArray();
}

inline bool ArrayData::isDictOrDArrayOrShape() const {
  return isShape() || isDictOrDArray();
}

// gcc doesn't optimize (a & 3) == (b & 3) very well; help it a little.
inline bool ArrayData::dvArrayEqual(const ArrayData* a, const ArrayData* b) {
  return ((a->m_aux16 ^ b->m_aux16) & kDVArrayMask) == 0;
}

inline bool ArrayData::dvArraySanityCheck() const {
  auto const dv = dvArray();
  if (!RuntimeOption::EvalHackArrDVArrs) {
    if (isPacked()) return !(dv & kDArray);
    if (isMixed())  return !(dv & kVArray);
    if (isShape())  return dv == kDArray;
  }
  return dv == kNotDVArray;
}

inline bool ArrayData::hasApcTv() const { return m_aux16 & kHasApcTv; }

inline bool ArrayData::isLegacyArray() const { return m_aux16 & kLegacyArray; }

inline void ArrayData::setLegacyArray(bool legacy) {
  assert(!legacy || kind() == kDictKind || kind() == kVecKind);
  m_aux16 = (m_aux16 & ~kLegacyArray) | (legacy ? kLegacyArray : 0);
}

inline uint8_t ArrayData::auxBits() const {
  return dvArray() | (isLegacyArray() ? kLegacyArray : 0);
}

inline bool ArrayData::useWeakKeys() const { return isPHPArray(); }

inline DataType ArrayData::toDataType() const {
  auto const k = kind();
  if (k < kShapeKind) return KindOfArray;
  if (k == kVecKind) return KindOfVec;
  if (k == kDictKind) return KindOfDict;
  if (k == kShapeKind) return KindOfShape;
  assertx(k == kKeysetKind);
  return KindOfKeyset;
}

inline DataType ArrayData::toPersistentDataType() const {
  auto const k = kind();
  if (k < kShapeKind) return KindOfPersistentArray;
  if (k == kVecKind) return KindOfPersistentVec;
  if (k == kDictKind) return KindOfPersistentDict;
  if (k == kShapeKind) return KindOfPersistentShape;
  assertx(k == kKeysetKind);
  return KindOfPersistentKeyset;
}

///////////////////////////////////////////////////////////////////////////////
// Iteration.

inline int32_t ArrayData::getPosition() const {
  return m_pos;
}

inline void ArrayData::setPosition(int32_t p) {
  assertx(m_pos == p || !isStatic());
  m_pos = p;
}

inline bool ArrayData::isHead() const {
  return m_pos == iter_begin();
}

inline bool ArrayData::isTail() const {
  return m_pos == iter_last();
}

inline bool ArrayData::isInvalid() const {
  return m_pos == iter_end();
}

///////////////////////////////////////////////////////////////////////////////

inline bool ArrayData::IsValidKey(const StringData* k) {
  return k;
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void decRefArr(ArrayData* arr) {
  arr->decRefAndRelease();
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE bool checkHACRefBind() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckRefBind;
}
ALWAYS_INLINE bool checkHACFalseyPromote() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckFalseyPromote;
}
ALWAYS_INLINE bool checkHACEmptyStringPromote() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckEmptyStringPromote;
}
ALWAYS_INLINE bool checkHACCompare() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckCompare;
}
ALWAYS_INLINE bool checkHACArrayPlus() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckArrayPlus;
}
ALWAYS_INLINE bool checkHACArrayKeyCast() {
  return RuntimeOption::EvalHackArrCompatNotices &&
         RuntimeOption::EvalHackArrCompatCheckArrayKeyCast;
}

///////////////////////////////////////////////////////////////////////////////

}
