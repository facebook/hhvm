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
#include "hphp/util/safe-cast.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE ArrayData* staticEmptyVec() {
  void* vp = &s_theEmptyVec;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyMarkedVec() {
  void* vp = &s_theEmptyMarkedVec;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyDictArray() {
  void* vp = s_theEmptyDictArrayPtr;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyMarkedDictArray() {
  void* vp = s_theEmptyMarkedDictArrayPtr;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyKeysetArray() {
  void* vp = &s_theEmptySetArray;
  return static_cast<ArrayData*>(vp);
}

///////////////////////////////////////////////////////////////////////////////
// Creation and destruction.

ALWAYS_INLINE ArrayData* ArrayData::CreateVec(bool legacy) {
  return legacy ? staticEmptyMarkedVec() : staticEmptyVec();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDict(bool legacy) {
  return legacy ? staticEmptyMarkedDictArray() : staticEmptyDictArray();
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

NO_PROFILING
inline void ArrayData::release() DEBUG_NOEXCEPT {
  assertx(!hasMultipleRefs());
  g_array_funcs.release[kind()](this);
  AARCH64_WALKABLE_FRAME();
}

///////////////////////////////////////////////////////////////////////////////
// Introspection.

inline size_t ArrayData::size() const {
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

inline bool ArrayData::isVecType() const {
  return (kind() & ~kBespokeKindMask) == kVecKind;
}
inline bool ArrayData::isDictType() const {
  return (kind() & ~kBespokeKindMask) == kDictKind;
}
inline bool ArrayData::isKeysetType() const {
  return (kind() & ~kBespokeKindMask) == kKeysetKind;
}

inline bool ArrayData::isVanillaVec() const { return kind() == kVecKind; }
inline bool ArrayData::isVanillaDict() const { return kind() == kDictKind; }
inline bool ArrayData::isVanillaKeyset() const { return kind() == kKeysetKind; }

inline bool ArrayData::isVanilla() const {
  return !(kind() & kBespokeKindMask);
}

inline bool ArrayData::bothVanilla(const ArrayData* ad1, const ArrayData* ad2) {
  return !((ad1->kind() | ad2->kind()) & kBespokeKindMask);
}

inline bool ArrayData::hasApcTv() const { return m_aux16 & kHasApcTv; }

inline bool ArrayData::isLegacyArray() const { return m_aux16 & kLegacyArray; }

inline bool ArrayData::hasStrKeyTable() const {
  return m_aux16 & kHasStrKeyTable;
}

inline uint8_t ArrayData::auxBits() const {
  return safe_cast<uint8_t>(m_aux16 & (kLegacyArray | kSampledArray));
}

inline bool ArrayData::isSampledArray() const {
  return m_aux16 & kSampledArray;
}

inline void ArrayData::setSampledArrayInPlace() {
  assertx(hasExactlyOneRef());
  m_aux16 |= ArrayData::kSampledArray;
}

inline ArrayData* ArrayData::makeSampledStaticArray() const {
  assertx(isStatic());
  auto const result = copyStatic();
  result->m_aux16 |= ArrayData::kSampledArray;
  return result;
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
DataType ArrayData::toDataType() const {
  switch (kind()) {
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

inline bool ArrayData::IsValidKey(const StringData* k) {
  return k;
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

///////////////////////////////////////////////////////////////////////////////

}
