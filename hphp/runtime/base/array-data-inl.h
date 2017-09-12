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
  assert(m_size == -1);
  assert(m_pos == 0);
  assert(m_kind == static_cast<HeaderKind>(kind));
}

///////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE ArrayData* staticEmptyArray() {
  void* vp = &s_theEmptyArray;
  return static_cast<ArrayData*>(vp);
}

ALWAYS_INLINE ArrayData* staticEmptyVecArray() {
  void* vp = &s_theEmptyVecArray;
  return static_cast<ArrayData*>(vp);
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

ALWAYS_INLINE ArrayData* ArrayData::CreateVec() {
  return staticEmptyVecArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateDict() {
  return staticEmptyDictArray();
}

ALWAYS_INLINE ArrayData* ArrayData::CreateKeyset() {
  return staticEmptyKeysetArray();
}

ALWAYS_INLINE void ArrayData::decRefAndRelease() {
  assert(kindIsValid());
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
  assert(kindIsValid());
  return static_cast<ArrayKind>(m_kind);
}

inline bool ArrayData::noCopyOnWrite() const {
  // GlobalsArray doesn't support COW.
  return kind() == kGlobalsKind;
}

inline bool ArrayData::isPacked() const { return kind() == kPackedKind; }
inline bool ArrayData::isMixed() const { return kind() == kMixedKind; }
inline bool ArrayData::isApcArray() const { return kind() == kApcKind; }
inline bool ArrayData::isGlobalsArray() const { return kind() == kGlobalsKind; }
inline bool ArrayData::isProxyArray() const { return kind() == kProxyKind; }
inline bool ArrayData::isEmptyArray() const { return kind() == kEmptyKind; }
inline bool ArrayData::isDict() const { return kind() == kDictKind; }
inline bool ArrayData::isVecArray() const { return kind() == kVecKind; }
inline bool ArrayData::isKeyset() const { return kind() == kKeysetKind; }

inline bool ArrayData::hasPackedLayout() const {
  return isPacked() || isVecArray();
}
inline bool ArrayData::hasMixedLayout() const {
  return isMixed() || isDict();
}

inline bool ArrayData::isPHPArray() const {
  return kind() < kDictKind;
}
inline bool ArrayData::isHackArray() const {
  return kind() >= kDictKind;
}

inline ArrayData::DVArray ArrayData::dvArray() const {
  // The darray/varray state is stored in the lower 8-bits of m_aux16. The
  // array is free to store whatever it wants in the upper 8-bits.
  return static_cast<DVArray>(m_aux16);
}

inline bool ArrayData::isVArray() const {
  return isPacked() || isEmptyArray();
}

inline bool ArrayData::useWeakKeys() const { return isPHPArray(); }

inline DataType ArrayData::toDataType() const {
  auto const k = kind();
  if (k < kDictKind) return KindOfArray;
  if (k == kVecKind) return KindOfVec;
  if (k == kDictKind) return KindOfDict;
  assert(k == kKeysetKind);
  return KindOfKeyset;
}

inline DataType ArrayData::toPersistentDataType() const {
  auto const k = kind();
  if (k < kDictKind) return KindOfPersistentArray;
  if (k == kVecKind) return KindOfPersistentVec;
  if (k == kDictKind) return KindOfPersistentDict;
  assert(k == kKeysetKind);
  return KindOfPersistentKeyset;
}

///////////////////////////////////////////////////////////////////////////////
// Iteration.

inline int32_t ArrayData::getPosition() const {
  return m_pos;
}

inline void ArrayData::setPosition(int32_t p) {
  assert(m_pos == p || !isStatic());
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

}
