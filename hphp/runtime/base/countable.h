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
#include "hphp/util/alloc.h"
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

auto constexpr FAST_REFCOUNT_OFFSET = HeapObject::count_offset();

/*
 * When true, IncRef operations on non-persistent objects in one-bit mode will
 * always store MultiReference. When false, they will only store MultiReference
 * if the current value is OneReference.
 */
auto constexpr unconditional_one_bit_incref = true;

/*
 * When true, all DecRef operations will be no-ops. Enabling this when
 * one_bit_refcount == false will visibly affect program behavior; use with
 * caution.
 */
auto constexpr noop_decref = false;

extern __thread bool tl_sweeping;

/*
 * refcounted objects that have count == Uncounted/StaticValue when persistent
 */
struct MaybeCountable : HeapObject {
  RefCount count() const { return m_count; } // only for debugging & profiling
  bool checkCount() const;
  bool checkCountZ() const; // allows zero
  bool isRefCounted() const;
  bool hasMultipleRefs() const;
  bool hasExactlyOneRef() const;
  bool hasZeroRefs() const;
  bool isStatic() const;
  bool isUncounted() const;
  void incRefCount() const;
  void rawIncRefCount() const;
  void decRefCount() const;
  bool decWillRelease() const;
  bool countedDecRefAndCheck();
  bool decReleaseCheck();
  void fixCountForRelease();            // set count to 1 if it was 0
  bool cowCheck() const;
  /*
   * Uncounted types still record how many references there are to
   * them from apc, or other uncounted types. Generally, there will
   * only be one thing manipulating their refcounts at any given time,
   * but if eg multiple threads store the same uncounted array in apc
   * at the same time, it might happen.
   *
   * uncountedIncRef() will attempt to increment the refcount (in
   * one_bit_refcount builds there are only 127 ref counts available,
   * so it can fail) and return true on success. uncountedDecRef()
   * will decrement the refcount, and return true if the caller is
   * responsible for freeing it. When it returns false, the caller
   * cannot do anything further with the uncounted object, since
   * another thread might already be destroying it.
   *
   * Note that in non one_bit_refcount builds, uncountedIncRef
   * actually subtracts one from the refcount, and uncountedDecRef
   * adds one to it; but callers shouldn't care.
   */
  bool uncountedIncRef() const;
  bool uncountedDecRef() const;
};

/*
 * refcounted objects that are never persistent
 */
struct Countable : MaybeCountable {
  bool checkCount() const;
  bool checkCountZ() const; // allows zero
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

ALWAYS_INLINE bool MaybeCountable::uncountedIncRef() const {
  assertx(isUncounted());
  auto& count = m_atomic_count;
  if (one_bit_refcount) {
    auto val = count.load(std::memory_order_relaxed);
    while (val != -1) {
      assertx(val < 0);
      if (count.compare_exchange_weak(val, val + 1,
                                      std::memory_order_relaxed)) {
        return true;
      }
    }
    return false;
  }

  auto const DEBUG_ONLY val = count.fetch_sub(1, std::memory_order_relaxed);
  assertx(val <= UncountedValue);
  return true;
}

ALWAYS_INLINE bool MaybeCountable::uncountedDecRef() const {
  assertx(isUncounted());
  auto const val = m_atomic_count.fetch_add(one_bit_refcount ? -1 : 1,
                                            std::memory_order_relaxed);
  return val == UncountedValue;
}

ALWAYS_INLINE bool MaybeCountable::checkCount() const {
  if (one_bit_refcount) {
    return m_count == OneReference || m_count == MultiReference || m_count < 0;
  }

  // If this assertion fails, it indicates a double-free. Check it separately.
  assertx(m_count < RefCountMaxRealistic);
  return m_count >= 1 || m_count <= UncountedValue || m_count == StaticValue;
}

ALWAYS_INLINE bool MaybeCountable::checkCountZ() const {
  return m_count == 0 || checkCount();
}

ALWAYS_INLINE bool Countable::checkCount() const {
  if (one_bit_refcount) {
    return m_count == OneReference || m_count == MultiReference;
  }

  return m_count >= 1;
}

ALWAYS_INLINE bool Countable::checkCountZ() const {
  return m_count == 0 || checkCount();
}

ALWAYS_INLINE bool MaybeCountable::isRefCounted() const {
  return m_count >= 0;
}

ALWAYS_INLINE bool Countable::isRefCounted() const {
  return true;
}

ALWAYS_INLINE bool MaybeCountable::hasMultipleRefs() const {
  if (one_bit_refcount) return m_count != OneReference;

  return uint32_t(m_count) > 1; // treat Static/Uncounted as large counts
}

ALWAYS_INLINE bool Countable::hasMultipleRefs() const {
  assertx(checkCountZ());
  if (one_bit_refcount) return m_count != OneReference;

  return m_count > 1;
}

ALWAYS_INLINE bool MaybeCountable::hasExactlyOneRef() const {
  assertx(checkCountZ());
  return m_count == OneReference;
}

ALWAYS_INLINE bool MaybeCountable::hasZeroRefs() const {
  assertx(checkCountZ());
  if (one_bit_refcount) return false;
  return m_count == 0;
}

ALWAYS_INLINE bool Countable::hasExactlyOneRef() const {
  assertx(checkCountZ());
  return m_count == OneReference;
}

ALWAYS_INLINE void MaybeCountable::incRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (one_bit_refcount) {
    if (m_count == OneReference) m_count = MultiReference;
    return;
  }

  if (isRefCounted()) ++m_count;
}

ALWAYS_INLINE void Countable::incRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (one_bit_refcount) {
    if (unconditional_one_bit_incref || m_count == OneReference) {
      m_count = MultiReference;
    }
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::rawIncRefCount() const {
  assertx(!tl_sweeping);
  assertx(isRefCounted());
  if (one_bit_refcount) {
    if (unconditional_one_bit_incref || m_count == OneReference) {
      m_count = MultiReference;
    }
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void Countable::rawIncRefCount() const {
  assertx(!tl_sweeping);
  assertx(isRefCounted());
  if (one_bit_refcount) {
    if (unconditional_one_bit_incref || m_count == OneReference) {
      m_count = MultiReference;
    }
    return;
  }

  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::decRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (one_bit_refcount) return;

  if (isRefCounted()) --m_count;
}

ALWAYS_INLINE void Countable::decRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (one_bit_refcount) return;

  --m_count;
}

ALWAYS_INLINE bool MaybeCountable::decWillRelease() const {
  return !noop_decref && !hasMultipleRefs();
}

ALWAYS_INLINE bool Countable::decWillRelease() const {
  return !noop_decref && !hasMultipleRefs();
}

ALWAYS_INLINE bool MaybeCountable::decReleaseCheck() {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (noop_decref) return false;
  if (one_bit_refcount) return m_count == OneReference;

  if (m_count == 1) return true;
  if (m_count > 1) --m_count;
  return false;
}

ALWAYS_INLINE void MaybeCountable::fixCountForRelease() {
  if (debug && !one_bit_refcount) {
    if (!m_count) ++m_count;
  }
}

ALWAYS_INLINE bool MaybeCountable::countedDecRefAndCheck() {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (noop_decref) return false;
  if (one_bit_refcount) return m_count == OneReference;
  assertx(m_count > 0);
  return !(--m_count);
}

ALWAYS_INLINE bool Countable::decReleaseCheck() {
  return countedDecRefAndCheck();
}

ALWAYS_INLINE bool MaybeCountable::isStatic() const {
  assertx(checkCount());
  return m_count == StaticValue;
}

ALWAYS_INLINE bool Countable::isStatic() const {
  assertx(checkCount());
  return false;
}

ALWAYS_INLINE bool MaybeCountable::isUncounted() const {
  assertx(checkCount());
  return one_bit_refcount ?
    m_count < 0 && m_count != StaticValue : m_count <= UncountedValue;
}

ALWAYS_INLINE bool Countable::isUncounted() const {
  assertx(checkCount());
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
