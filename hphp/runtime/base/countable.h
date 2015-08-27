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
constexpr size_t UncountedBitPos = 31;
constexpr int32_t UncountedValue = -128;
constexpr int32_t StaticValue = -127; // implies UncountedValue
constexpr int32_t RefCountMaxRealistic = (1 << 30) - 1;

static_assert((uint32_t)UncountedValue & (1uL << UncountedBitPos),
              "Check UncountedValue and UncountedBitPos");
static_assert((uint32_t)StaticValue & (1uL << UncountedBitPos),
              "Check StaticValue and UncountedBitPos");

/*
 * Check that the refcount is realistic, and not the static flag
 */
inline bool check_refcount_ns(int32_t count) {
  return (uint32_t)count <= (uint32_t)RefCountMaxRealistic;
}

/*
 * Real count values should always be less than or equal to
 * RefCountMaxRealistic, and asserting this will also catch
 * common malloc freed-memory patterns (e.g. 0x5a5a5a5a and
 * MemoryManager's 0x6a6a6a6a).
 */
inline bool check_refcount(int32_t count) {
  return count <= StaticValue || check_refcount_ns(count);
}

/*
 * As above, but additionally check for non-zero
 */
inline bool check_refcount_nz(int32_t count) {
  return count <= StaticValue || check_refcount_ns(count - 1);
}

/*
 * As above, but additionally check for greater-than-zero
 */
inline bool check_refcount_ns_nz(int32_t count) {
  return check_refcount_ns(count - 1);
}

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

// Clowny... but needed so we can handle the functions generically
inline RefCount getCount(RefCount count) {
  assert(check_refcount(count));
  return count;
}

inline bool isRefCounted(RefCount count) {
  assert(check_refcount(count));
  return count >= 0;
}

inline bool hasMultipleRefs(RefCount count) {
  assert(check_refcount(count));
  return (uint32_t)count > 1;
}

inline bool hasExactlyOneRef(RefCount count) {
  assert(check_refcount(count));
  return (uint32_t)count == 1;
}

inline bool isStatic(RefCount count) {
  return count == StaticValue;
}

inline void incRefCount(RefCount& count) {
  assert(check_refcount(count));
  if (isRefCounted(count)) { ++count; }
}

inline RefCount decRefCount(RefCount& count) {
  assert(check_refcount_nz(count));
  return (isRefCounted(count)) ? --count : count;
}

ALWAYS_INLINE bool decReleaseCheck(RefCount& count) {
  assert(check_refcount_nz(count));
  if (count == 1) return true;
  if (count > 1) --count;
  return false;
}

}

/*
 * Same as above, but does not allow static ref-counts.
 */
namespace CountableManipNS {

inline RefCount getCount(RefCount count) {
  assert(check_refcount_ns(count));
  return count;
}

inline bool isRefCounted(RefCount count) { return true; }

inline bool hasMultipleRefs(RefCount count) {
  assert(check_refcount_ns(count));
  return count > 1;
}

inline bool hasExactlyOneRef(RefCount count) {
  assert(check_refcount(count));
  return count == 1;
}

inline bool isStatic(RefCount count) { return false; }

inline void incRefCount(RefCount& count) {
  assert(check_refcount_ns(count));
  ++count;
}

inline RefCount decRefCount(RefCount& count) {
  assert(check_refcount_ns_nz(count));
  return --count;
}

ALWAYS_INLINE bool decReleaseCheck(RefCount& count) {
  assert(check_refcount_nz(count));
  if (count == 1) return true;
  if (count > 1) --count;
  return false;
}

}

/**
 * Ref-counted types have a count field at FAST_REFCOUNT_OFFSET
 * and define counting methods with these macros.
 */

#define IMPLEMENT_COUNTABLE_METHODS_WITH_STATIC                         \
  RefCount getCount() const {                                           \
    assert(kindIsValid());                                              \
    return CountableManip::getCount(m_hdr.count);                       \
  }                                                                     \
  bool isRefCounted() const {                                           \
    assert(kindIsValid());                                              \
    return CountableManip::isRefCounted(m_hdr.count);                   \
  }                                                                     \
  bool hasMultipleRefs() const {                                        \
    assert(kindIsValid());                                              \
    return CountableManip::hasMultipleRefs(m_hdr.count);                \
  }                                                                     \
  bool hasExactlyOneRef() const {                                       \
    assert(kindIsValid());                                              \
    return CountableManip::hasExactlyOneRef(m_hdr.count);               \
  }                                                                     \
  void incRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    CountableManip::incRefCount(m_hdr.count);                           \
  }                                                                     \
  RefCount decRefCount() const {                                        \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    return CountableManip::decRefCount(m_hdr.count);                    \
  }                                                                     \
  ALWAYS_INLINE bool decReleaseCheck() {                                \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    return CountableManip::decReleaseCheck(m_hdr.count);                \
  }                                                                     \
  ALWAYS_INLINE void decRefAndRelease() {                               \
    assert(kindIsValid());                                              \
    if (decReleaseCheck()) release();                                   \
  }                                                                     \
  bool isStatic() const {                                               \
    assert(kindIsValid());                                              \
    return CountableManip::isStatic(m_hdr.count);                       \
  }                                                                     \
  bool isUncounted() const {                                            \
    assert(kindIsValid());                                              \
    return m_hdr.count == UncountedValue;                               \
  }

#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC                           \
  RefCount getCount() const {                                           \
    assert(kindIsValid());                                              \
    return CountableManipNS::getCount(m_hdr.count);                     \
  }                                                                     \
  bool isRefCounted() const {                                           \
    assert(kindIsValid());                                              \
    return CountableManipNS::isRefCounted(m_hdr.count);                 \
  }                                                                     \
  bool hasMultipleRefs() const {                                        \
    assert(kindIsValid());                                              \
    return CountableManipNS::hasMultipleRefs(m_hdr.count);              \
  }                                                                     \
  bool hasExactlyOneRef() const {                                       \
    assert(kindIsValid());                                              \
    return CountableManipNS::hasExactlyOneRef(m_hdr.count);             \
  }                                                                     \
  bool isStatic() const {                                               \
    assert(kindIsValid());                                              \
    return CountableManipNS::isStatic(m_hdr.count);                     \
  }                                                                     \
  void incRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    CountableManipNS::incRefCount(m_hdr.count);                         \
  }                                                                     \
  RefCount decRefCount() const {                                        \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    return CountableManipNS::decRefCount(m_hdr.count);                  \
  }                                                                     \
  ALWAYS_INLINE bool decReleaseCheck() {                                \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    return CountableManipNS::decReleaseCheck(m_hdr.count);              \
  }                                                                     \
  ALWAYS_INLINE bool decRefAndRelease() {                               \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    assert(check_refcount_ns_nz(m_hdr.count));                          \
    if (!--m_hdr.count) {                                               \
      release();                                                        \
      return true;                                                      \
    }                                                                   \
    return false;                                                       \
  }

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_COUNTABLE_H_
