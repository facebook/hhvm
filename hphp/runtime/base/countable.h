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

#include "hphp/runtime/base/header-kind.h"
#include "hphp/util/assertions.h"

#include <cstdint>
#include <cstddef>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline RefCount& operator++(RefCount& count) {
  assertx(!one_bit_refcount);
  count = static_cast<RefCount>(count + 1);
  return count;
}

inline RefCount& operator--(RefCount& count) {
  assertx(!one_bit_refcount);
  count = static_cast<RefCount>(count - 1);
  return count;
}

constexpr int32_t RefCountMaxRealistic = (1 << 30) - 1;
auto constexpr FAST_REFCOUNT_OFFSET = HeapObject::count_offset();

/*
 * When true, IncRef operations on non-persistent objects in one-bit mode will
 * always store SharedValue. When false, they will only store SharedValue if
 * the current value is PrivateValue.
 */
auto constexpr unconditional_one_bit_incref = true;

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
  if (one_bit_refcount) {
    return m_count == PrivateValue || m_count == SharedValue ||
      m_count == UncountedValue || m_count == StaticValue;
  }

  return m_count >= 1 || m_count == UncountedValue || m_count == StaticValue;
}

ALWAYS_INLINE bool Countable::checkCount() const {
  if (one_bit_refcount) {
    return m_count == PrivateValue || m_count == SharedValue;
  }

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
  if (one_bit_refcount) return m_count != PrivateValue;

  return uint32_t(m_count) > 1; // treat Static/Uncounted as large counts
}

ALWAYS_INLINE bool Countable::hasMultipleRefs() const {
  assert(checkCount());
  if (one_bit_refcount) return m_count != PrivateValue;

  return m_count > 1;
}

ALWAYS_INLINE bool MaybeCountable::hasExactlyOneRef() const {
  assert(checkCount());
  if (one_bit_refcount) return m_count == PrivateValue;

  return m_count == 1;
}

ALWAYS_INLINE bool Countable::hasExactlyOneRef() const {
  assert(checkCount());
  if (one_bit_refcount) return m_count == PrivateValue;

  return m_count == 1;
}

ALWAYS_INLINE void MaybeCountable::incRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount() || m_count == 0 /* due to static init order */);
  if (one_bit_refcount) {
    if (m_count == PrivateValue) m_count = SharedValue;
    return;
  }

  if (isRefCounted()) ++m_count;
}

ALWAYS_INLINE void Countable::incRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount() || m_count == 0 /* due to static init order */);
  if (one_bit_refcount) {
    if (unconditional_one_bit_incref || m_count == PrivateValue) {
      m_count = SharedValue;
    }
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::rawIncRefCount() const {
  assert(!tl_sweeping);
  assert(isRefCounted());
  if (one_bit_refcount) {
    m_count = SharedValue;
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void Countable::rawIncRefCount() const {
  assert(!tl_sweeping);
  assert(isRefCounted());
  if (one_bit_refcount) {
    m_count = SharedValue;
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::decRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount());
  if (one_bit_refcount) return;

  if (isRefCounted()) --m_count;
}

ALWAYS_INLINE void Countable::decRefCount() const {
  assert(!tl_sweeping);
  assert(checkCount());
  if (one_bit_refcount) return;

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
  if (one_bit_refcount) return m_count == PrivateValue;

  if (m_count == 1) return true;
  if (m_count > 1) --m_count;
  return false;
}

ALWAYS_INLINE bool Countable::decReleaseCheck() {
  assert(!tl_sweeping);
  assert(checkCount());
  if (one_bit_refcount) return m_count == PrivateValue;

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
