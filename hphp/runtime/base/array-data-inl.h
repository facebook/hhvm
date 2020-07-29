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
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE ArrayData* staticEmptyArray() {
  return staticEmptyDArray();
}

ALWAYS_INLINE ArrayData* staticEmptyVArray() {
  void* vp1 = &s_theEmptyVArray;
  void* vp2 = &s_theEmptyVec;
  return static_cast<ArrayData*>(RuntimeOption::EvalHackArrDVArrs ? vp2 : vp1);
}

ALWAYS_INLINE ArrayData* staticEmptyVec() {
  void* vp = &s_theEmptyVec;
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
  return ArrayData::CreateDArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVArray(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenance
    ? arrprov::tagStaticArr(staticEmptyVArray(), tag)
    : staticEmptyVArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDArray(arrprov::Tag tag /* = {} */) {
  return RO::EvalArrayProvenance
    ? arrprov::tagStaticArr(staticEmptyDArray(), tag)
    : staticEmptyDArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateVec(arrprov::Tag tag /* = {} */) {
  return staticEmptyVec();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDict(arrprov::Tag tag /* = {} */) {
  return staticEmptyDictArray();
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

inline void ArrayData::release() noexcept {
  assertx(!hasMultipleRefs());
  g_array_funcs.release[kind()](this);
  AARCH64_WALKABLE_FRAME();
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
  return isArrayKind(m_kind);
}

inline ArrayData::ArrayKind ArrayData::kind() const {
  assertx(kindIsValid());
  return static_cast<ArrayKind>(m_kind);
}

inline bool ArrayData::isPackedKind() const { return kind() == kPackedKind; }
inline bool ArrayData::isMixedKind() const { return kind() == kMixedKind; }
inline bool ArrayData::isVecKind() const { return kind() == kVecKind; }
inline bool ArrayData::isDictKind() const { return kind() == kDictKind; }
inline bool ArrayData::isKeysetKind() const { return kind() == kKeysetKind; }

inline bool ArrayData::isPHPArrayType() const {
  return kind() < kDictKind;
}
inline bool ArrayData::isHackArrayType() const {
  return kind() >= kDictKind;
}

inline bool ArrayData::isVecType() const {
  return (kind() & ~kBespokeKindMask) == kVecKind;
}
inline bool ArrayData::isDictType() const {
  return (kind() & ~kBespokeKindMask) == kDictKind;
}
inline bool ArrayData::isKeysetType() const {
  return (kind() & ~kBespokeKindMask) == kKeysetKind;
}

inline bool ArrayData::hasVanillaPackedLayout() const {
  return isPackedKind() || isVecKind();
}
inline bool ArrayData::hasVanillaMixedLayout() const {
  return isMixedKind() || isDictKind();
}

inline bool ArrayData::isVanilla() const {
  return !(kind() & kBespokeKindMask);
}

inline bool ArrayData::bothVanilla(const ArrayData* ad1, const ArrayData* ad2) {
  return !((ad1->kind() | ad2->kind()) & kBespokeKindMask);
}

inline bool ArrayData::isVArray() const {
  return (kind() & ~kBespokeKindMask) == kPackedKind;
}

inline bool ArrayData::isDArray() const {
  static_assert(kMixedKind == 0);
  static_assert(kBespokeDArrayKind == 1);
  return kind() <= kBespokeDArrayKind;
}

inline bool ArrayData::isDVArray() const {
  static_assert(kMixedKind == 0);
  static_assert(kBespokeDArrayKind == 1);
  static_assert(kPackedKind == 2);
  static_assert(kBespokeVArrayKind == 3);
  return kind() <= kBespokeVArrayKind;
}

inline bool ArrayData::isNotDVArray() const { return !isDVArray(); }

inline bool ArrayData::isHAMSafeVArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isVecType() : isVArray();
}
inline bool ArrayData::isHAMSafeDArray() const {
  return RuntimeOption::EvalHackArrDVArrs ? isDictType() : isDArray();
}

inline bool ArrayData::dvArrayEqual(const ArrayData* a, const ArrayData* b) {
  static_assert(kMixedKind == 0);
  static_assert(kBespokeDArrayKind == 1);
  static_assert(kPackedKind == 2);
  static_assert(kBespokeVArrayKind == 3);
  return std::min(uint8_t(a->kind() & ~kBespokeKindMask), uint8_t{4}) ==
         std::min(uint8_t(b->kind() & ~kBespokeKindMask), uint8_t{4});
}

inline bool ArrayData::hasApcTv() const { return m_aux16 & kHasApcTv; }

inline bool ArrayData::isLegacyArray() const { return m_aux16 & kLegacyArray; }

inline void ArrayData::setLegacyArray(bool legacy) {
  assertx(hasExactlyOneRef());
  assertx(!legacy
          || isDictType()
          || isVecType()
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

inline bool ArrayData::hasStrKeyTable() const {
  return m_aux16 & kHasStrKeyTable;
}

inline uint8_t ArrayData::auxBits() const {
  return isLegacyArray() ? kLegacyArray : 0;
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
DataType ArrayData::toDataType() const {
  switch (kind()) {
    case kPackedKind:
    case kBespokeVArrayKind:
      return KindOfVArray;

    case kMixedKind:
    case kBespokeDArrayKind:
      return KindOfDArray;

    case kVecKind:
    case kBespokeVecKind:
      return KindOfVec;

    case kDictKind:
    case kBespokeDictKind:
      return KindOfDict;

    case kKeysetKind:
    case kBespokeKeysetKind:
      return KindOfKeyset;

    case kNumKinds:   not_reached();
  }
  not_reached();
}

ALWAYS_INLINE
DataType ArrayData::toPersistentDataType() const {
  switch (kind()) {
    case kPackedKind:
    case kBespokeVArrayKind:
      return KindOfPersistentVArray;

    case kMixedKind:
    case kBespokeDArrayKind:
      return KindOfPersistentDArray;

    case kVecKind:
    case kBespokeVecKind:
      return KindOfPersistentVec;

    case kDictKind:
    case kBespokeDictKind:
      return KindOfPersistentDict;

    case kKeysetKind:
    case kBespokeKeysetKind:
      return KindOfPersistentKeyset;

    case kNumKinds:   not_reached();
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// Iteration.

inline int32_t ArrayData::getPosition() const {
  return isVanilla() ? m_pos : iter_begin();
}

inline void ArrayData::setPosition(int32_t p) {
  auto const cur_pos = getPosition();
  assertx(cur_pos == p || (!isStatic() && isVanilla()));
  if (cur_pos != p) m_pos = p;
}

inline bool ArrayData::isHead() const {
  return getPosition() == iter_begin();
}

inline bool ArrayData::isTail() const {
  return getPosition() == iter_last();
}

inline bool ArrayData::isInvalid() const {
  return getPosition() == iter_end();
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
