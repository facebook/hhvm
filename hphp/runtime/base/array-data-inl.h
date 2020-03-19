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

#include "hphp/runtime/base/array-provenance.h"
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

ALWAYS_INLINE ArrayData* staticEmptyKeysetArray() {
  void* vp = &s_theEmptySetArray;
  return static_cast<ArrayData*>(vp);
}

///////////////////////////////////////////////////////////////////////////////
// Creation and destruction.

ALWAYS_INLINE ArrayData* ArrayData::Create() {
  return staticEmptyArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVArray(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenanceEmpty &&
         RO::EvalArrProvDVArrays
    ? arrprov::tagStaticArr(staticEmptyVArray(), tag)
    : staticEmptyVArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVec(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenanceEmpty &&
         RO::EvalArrProvHackArrays
    ? arrprov::tagStaticArr(staticEmptyVecArray(), tag)
    : staticEmptyVecArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDArray(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenanceEmpty &&
         RO::EvalArrProvDVArrays
    ? arrprov::tagStaticArr(staticEmptyDArray(), tag)
    : staticEmptyDArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDict(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenanceEmpty &&
         RO::EvalArrProvHackArrays
    ? arrprov::tagStaticArr(staticEmptyDictArray(), tag)
    : staticEmptyDictArray();
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
  if (UNLIKELY((int)m_size < 0)) return vsize();
  return m_size;
}

inline size_t ArrayData::getSize() const {
  return m_size;
}

inline bool ArrayData::empty() const {
  return size() == 0;
}

inline bool ArrayData::kindIsValid() const {
  assertx(isVanilla());
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

inline bool ArrayData::isPackedKind() const { return kind() == kPackedKind; }
inline bool ArrayData::isMixedKind() const { return kind() == kMixedKind; }
inline bool ArrayData::isApcArrayKind() const { return kind() == kApcKind; }
inline bool ArrayData::isGlobalsArrayKind() const { return kind() == kGlobalsKind; }
inline bool ArrayData::isEmptyArrayKind() const { return kind() == kEmptyKind; }
inline bool ArrayData::isDictKind() const { return kind() == kDictKind; }

inline bool ArrayData::isVecArrayKind() const { return kind() == kVecKind; }
inline bool ArrayData::isKeysetKind() const { return kind() == kKeysetKind; }
inline bool ArrayData::isRecordArrayKind() const { return kind() == kRecordKind; }

inline bool ArrayData::isPHPArrayType() const {
  return ::HPHP::isArrayType(toDataType());
}
inline bool ArrayData::isHackArrayType() const {
  return ::HPHP::isHackArrayType(toDataType());
}

inline bool ArrayData::isDictType() const {
  return ::HPHP::isDictType(toDataType());
}
inline bool ArrayData::isVecArrayType() const {
  return ::HPHP::isVecType(toDataType());
}
inline bool ArrayData::isKeysetType() const {
  return ::HPHP::isKeysetType(toDataType());
}

inline bool ArrayData::hasVanillaPackedLayout() const {
  return isPackedKind() || isVecArrayKind();
}
inline bool ArrayData::hasVanillaMixedLayout() const {
  return isMixedKind() || isDictKind();
}

inline bool ArrayData::isPHPArrayKind() const {
  return kind() <= kRecordKind;
}

inline bool ArrayData::isHackArrayKind() const {
  return kind() >= kDictKind;
}

inline bool ArrayData::isVanilla() const {
  return !(m_aux16 & kIsBespoke);
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

inline bool ArrayData::isDVArray() const { return dvArray(); }
inline bool ArrayData::isNotDVArray() const { return dvArray() == kNotDVArray; }
inline bool ArrayData::isVecOrVArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isVecArrayType() : isVArray();
}
inline bool ArrayData::isDictOrDArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isDictType() : isDArray();
}

// gcc doesn't optimize (a & 3) == (b & 3) very well; help it a little.
inline bool ArrayData::dvArrayEqual(const ArrayData* a, const ArrayData* b) {
  return ((a->m_aux16 ^ b->m_aux16) & kDVArrayMask) == 0;
}

inline bool ArrayData::dvArraySanityCheck() const {
  auto const dv = dvArray();
  if (!RuntimeOption::EvalHackArrDVArrs) {
    if (isPackedKind()) return !(dv & kDArray);
    if (isMixedKind())  return !(dv & kVArray);
  }
  return dv == kNotDVArray;
}

inline bool ArrayData::hasApcTv() const { return m_aux16 & kHasApcTv; }

inline bool ArrayData::isLegacyArray() const { return m_aux16 & kLegacyArray; }

inline void ArrayData::setLegacyArray(bool legacy) {
  assertx(hasExactlyOneRef());
  assertx(!legacy
          || isDictType()
          || isVecArrayType()
          || (!RO::EvalHackArrDVArrs && isDVArray()));
  /* TODO(jgriego) we should be asserting that the
   * mark-ee should have provenance here but it's not
   * safe and sane yet */
  if (legacy && !isLegacyArray() && hasProvenanceData()) {
    arrprov::clearTag(this);
    setHasProvenanceData(false);
  }
  m_aux16 = (m_aux16 & ~kLegacyArray) | (legacy ? kLegacyArray : 0);
}

inline uint8_t ArrayData::auxBits() const {
  return dvArray() | (isLegacyArray() ? kLegacyArray : 0);
}

inline bool ArrayData::useWeakKeys() const {
  return isPHPArrayType();
}

inline DataType ArrayData::toDataType() const {
  if (UNLIKELY(RuntimeOption::EvalEmitDVArray)) {
    if (isVArray()) {
      assertx(isPackedKind());
      return KindOfVArray;
    } else if (isDArray()) {
      assertx(isMixedKind());
      return KindOfDArray;
    }
  }
  switch (kind()) {
    case kPackedKind:
    case kMixedKind:
    case kEmptyKind:
    case kApcKind:
    case kGlobalsKind:
    case kRecordKind:
      return KindOfArray;

    case kDictKind:   return KindOfDict;
    case kVecKind:    return KindOfVec;
    case kKeysetKind: return KindOfKeyset;
    case kNumKinds:   not_reached();
  }
  not_reached();
}

inline DataType ArrayData::toPersistentDataType() const {
  if (UNLIKELY(RuntimeOption::EvalEmitDVArray)) {
    if (isVArray()) {
      assertx(isPackedKind());
      return KindOfPersistentVArray;
    } else if (isDArray()) {
      assertx(isMixedKind());
      return KindOfPersistentDArray;
    }
  }
  switch (kind()) {
    case kPackedKind:
    case kMixedKind:
    case kEmptyKind:
    case kApcKind:
    case kGlobalsKind:
    case kRecordKind:
      return KindOfPersistentArray;

    case kDictKind:   return KindOfPersistentDict;
    case kVecKind:    return KindOfPersistentVec;
    case kKeysetKind: return KindOfPersistentKeyset;
    case kNumKinds:   not_reached();
  }
  not_reached();
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

ALWAYS_INLINE
bool ArrayData::hasProvenanceData() const {
  return m_aux16 & kHasProvenanceData;
}

ALWAYS_INLINE
void ArrayData::setHasProvenanceData(bool value) {
  m_aux16 = (m_aux16 & ~kHasProvenanceData) |
    (value ? kHasProvenanceData : 0);
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE void decRefArr(ArrayData* arr) {
  arr->decRefAndRelease();
}

///////////////////////////////////////////////////////////////////////////////

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

namespace arrprov_detail {
template<typename SrcArray>
ArrayData* tagArrProvImpl(ArrayData*, const SrcArray*);
}

ALWAYS_INLINE ArrayData* tagArrProv(ArrayData* ad, const ArrayData* src) {
  return RO::EvalArrayProvenance
    ? arrprov_detail::tagArrProvImpl(ad, src)
    : ad;
}
ALWAYS_INLINE ArrayData* tagArrProv(ArrayData* ad, const APCArray* src) {
  return RO::EvalArrayProvenance
    ? arrprov_detail::tagArrProvImpl(ad, src)
    : ad;
}

///////////////////////////////////////////////////////////////////////////////

}
