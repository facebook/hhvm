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
constexpr int32_t UncountedValue = -128;
constexpr int32_t StaticValue = -127; // implies UncountedValue
constexpr int32_t RefCountMaxRealistic = (1 << 30) - 1;
constexpr auto FAST_REFCOUNT_OFFSET = HeapObject::count_offset();
extern __thread bool tl_sweeping;

/*
 * refcounted objects that have count == Uncounted/StaticValue when persistent
 */
struct MaybeCountable : HeapObject {
  RefCount count() const { return m_count; } // only for debugging & profiling
  bool checkCount() const;
  bool isRefCounted() const;
  bool hasMultipleRefs() const;
  bool hasExactlyOneRef() const;
  bool isStatic() const;
  bool isUncounted() const;
  void incRefCount() const;
  void rawIncRefCount() const;
  void decRefCount() const;
  bool decWillRelease() const;
  bool decReleaseCheck();
  bool cowCheck() const;
};

/*
 * refcounted objects that are never persistent
 */
struct Countable : MaybeCountable {
  bool checkCount() const;
  bool isRefCounted() const;
  bool hasMultipleRefs() const;
  bool hasExactlyOneRef() const;
  bool isStatic() const;
  bool isUncounted() const;
  void incRefCount() const;
  void rawIncRefCount() const;
  void decRefCount() const;
  bool decWillRelease() const;
  bool decReleaseCheck();
  bool cowCheck() const;
};

ALWAYS_INLINE bool MaybeCountable::checkCount() const {
  return m_count >= 1 || m_count == UncountedValue || m_count == StaticValue;
}

ALWAYS_INLINE bool Countable::checkCount() const {
  return m_count >= 1;
}

ALWAYS_INLINE bool MaybeCountable::isRefCounted() const {
  return m_count >= 0;
}

ALWAYS_INLINE bool Countable::isRefCounted() const {
  return true;
}

ALWAYS_INLINE bool MaybeCountable::hasMultipleRefs() const {
  assert(checkCount());
  return uint32_t(m_count) > 1; // treat Static/Uncounted as large counts
}

ALWAYS_INLINE bool Countable::hasMultipleRefs() const {
  assert(checkCount());
  return m_count > 1;
}

ALWAYS_INLINE bool MaybeCountable::hasExactlyOneRef() const {
  assert(checkCount());
  return m_count == 1;
}

ALWAYS_INLINE bool Countable::hasExactlyOneRef() const {
  assert(checkCount());
  return m_count == 1;
}

ALWAYS_INLINE void MaybeCountable::incRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount() || m_count == 0 /* due to static init order */);
  if (isRefCounted()) ++m_count;
}

ALWAYS_INLINE void Countable::incRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount() || m_count == 0 /* due to static init order */);
  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::rawIncRefCount() const {
  assert(!tl_sweeping);
  assert(isRefCounted());
  ++m_count;
}

ALWAYS_INLINE void Countable::rawIncRefCount() const {
  assert(!tl_sweeping);
  assert(isRefCounted());
  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::decRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount());
  if (isRefCounted()) --m_count;
}

ALWAYS_INLINE void Countable::decRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount());
  --m_count;
}

ALWAYS_INLINE bool MaybeCountable::decWillRelease() const {
  return !hasMultipleRefs();
}

ALWAYS_INLINE bool Countable::decWillRelease() const {
  return !hasMultipleRefs();
}

ALWAYS_INLINE bool MaybeCountable::decReleaseCheck() {
  assert(!tl_sweeping);
  assert(checkCount());
  if (m_count == 1) return true;
  if (m_count > 1) --m_count;
  return false;
}

ALWAYS_INLINE bool Countable::decReleaseCheck() {
  assert(!tl_sweeping);
  assert(checkCount());
  if (m_count == 1) return true;
  --m_count;
  return false;
}

ALWAYS_INLINE bool MaybeCountable::isStatic() const {
  assert(checkCount());
  return m_count == StaticValue;
}

ALWAYS_INLINE bool Countable::isStatic() const {
  assert(checkCount());
  return false;
}

ALWAYS_INLINE bool MaybeCountable::isUncounted() const {
  assert(checkCount());
  return m_count == UncountedValue;
}

ALWAYS_INLINE bool Countable::isUncounted() const {
  assert(checkCount());
  return false;
}

ALWAYS_INLINE bool MaybeCountable::cowCheck() const {
  return hasMultipleRefs();
}

ALWAYS_INLINE bool Countable::cowCheck() const {
  return hasMultipleRefs();
}

}
#endif // incl_HPHP_COUNTABLE_H_
