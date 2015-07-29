/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COUNTABLE_H_
#define incl_HPHP_COUNTABLE_H_

#include <cstdint>
#include <cstddef>
#include "hphp/runtime/base/header-kind.h"
#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * The sign bit flags a reference count as static. If a reference count
 * is static, it means we should never increment or decrement it: the
 * object lives across requests and may be accessed by multiple threads.
 * Long lived objects can be static or uncounted; static objects have process
 * lifetime, while uncounted objects are freed using the treadmill.
 * Using 8-bit values generates shorter cmp instructions while still being
 * far enough from 0 to be safe.
 */

constexpr uint8_t StaticGCByte = 0x84; // 1000 0100, _static=mrb=1
constexpr uint8_t UncountedGCByte = 0xC4; // 1100 0100, _static=uncounted=mrb=1
constexpr uint8_t UnsharedGCByte = 0x0; // when refcount == 0 or 1
constexpr uint8_t SharedGCByte = 0x4; // when refcount > 1

/*
 * Ref-count manipulation functions, where static ref-counts are allowed. These
 * exist as stand-alone functions taking just the ref-count so that
 * TV_GENERIC_DISPATCH can manipulate ref-counts with raw pointer arithmetic
 * without having to know the object the ref-counts are contained within (this
 * is needed to avoid type aliasing errors).
 *
 * The member version of these functions will generally just forward to one of
 * these. Prefer the member versions whenever you know the actual object type
 * (and if you don't, use TV_GENERIC_DISPATCH).
 */
namespace CountableManip {

inline bool isRefCounted(bool _static) {
  return !_static;
}

inline bool hasMultipleRefs(bool mrb) {
  return mrb;
}

inline bool hasExactlyOneRef(bool mrb) {
  return !mrb;
}

inline bool maybeShared(bool mrb) {
  return mrb;
}

inline bool isStatic(bool _static, bool uncounted) {
  return _static && !uncounted;
}

ALWAYS_INLINE bool decReleaseCheck(bool mrb) {
  return !mrb;
}

}

/*
 * Same as above, but does not allow static ref-counts.
 */
namespace CountableManipNS {

inline bool isRefCounted(RefCount count) { return true; }

inline bool hasMultipleRefs(bool mrb) {
  return mrb;
}

inline bool hasExactlyOneRef(bool mrb) {
  return !mrb;
}

inline bool isStatic(bool mrb) { return false; }

ALWAYS_INLINE bool decReleaseCheck(bool mrb) {
  return !mrb;
}

}

/**
 * Ref-counted types have a count field at FAST_REFCOUNT_OFFSET
 * and define counting methods with these macros.
 */

#define IMPLEMENT_COUNTABLE_METHODS_WITH_STATIC                         \
  bool isRefCounted() const {                                           \
    return CountableManip::isRefCounted(m_hdr._static);                 \
  }                                                                     \
  bool hasMultipleRefs() const {                                        \
    return CountableManip::hasMultipleRefs(m_hdr.mrb);                  \
  }                                                                     \
  bool hasExactlyOneRef() const {                                       \
    return CountableManip::hasExactlyOneRef(m_hdr.mrb);                 \
  }                                                                     \
  bool maybeShared() const {                                            \
    return CountableManip::maybeShared(m_hdr.mrb);                      \
  }                                                                     \
  bool cowCheck() const {                                               \
    return maybeShared();                                               \
  }                                                                     \
  void incRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    m_hdr.mrb = true; /* can't pass a ref to a bitfield :( */           \
  }                                                                     \
  ALWAYS_INLINE bool decReleaseCheck() {                                \
    assert(!MemoryManager::sweeping());                                 \
    return CountableManip::decReleaseCheck(m_hdr.mrb);                  \
  }                                                                     \
  ALWAYS_INLINE void decRefAndRelease() {                               \
    if (decReleaseCheck()) release();                                   \
  }                                                                     \
  bool isStatic() const {                                               \
    return CountableManip::isStatic(m_hdr._static, m_hdr.uncounted);    \
  }                                                                     \
  bool isUncounted() const {                                            \
    return m_hdr._static && m_hdr.uncounted;                            \
  }

#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC                           \
  bool isRefCounted() const {                                           \
    return CountableManipNS::isRefCounted(m_hdr.count);                 \
  }                                                                     \
  bool hasMultipleRefs() const {                                        \
    return CountableManipNS::hasMultipleRefs(m_hdr.mrb);                \
  }                                                                     \
  bool hasExactlyOneRef() const {                                       \
    return CountableManipNS::hasExactlyOneRef(m_hdr.mrb);               \
  }                                                                     \
  bool isStatic() const {                                               \
    return CountableManipNS::isStatic(m_hdr.count);                     \
  }                                                                     \
  void incRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    m_hdr.mrb = true; /* can't pass a ref to a bitfield :( */           \
  }                                                                     \
  ALWAYS_INLINE bool decReleaseCheck() {                                \
    assert(!MemoryManager::sweeping());                                 \
    return CountableManipNS::decReleaseCheck(m_hdr.mrb);                \
  }                                                                     \
  ALWAYS_INLINE bool decRefAndRelease() {                               \
    assert(!MemoryManager::sweeping());                                 \
    if (!m_hdr.mrb) {                                                   \
      release();                                                        \
      return true;                                                      \
    }                                                                   \
    return false;                                                       \
  }

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_COUNTABLE_H_
