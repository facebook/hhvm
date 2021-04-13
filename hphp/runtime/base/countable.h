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

#pragma once

#include "hphp/runtime/base/header-kind.h"
#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"

#include <cstdint>
#include <cstddef>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline RefCount& operator++(RefCount& count) {
  count = static_cast<RefCount>(count + 1);
  return count;
}

inline RefCount& operator--(RefCount& count) {
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
   * but if e.g. multiple threads store the same uncounted array in APC
   * at the same time, it might happen.
   *
   * uncountedIncRef() will increment this count, and uncountedDecRef()
   * will decrement this count. uncountedDecRef() will return true if
   * we've dec-ref-ed this value to "uncounted zero" - i.e. it's time
   * to release the memory. Even if uncountedDecRef() returns false,
   * the caller must not do anything further with the uncounted object,
   * because another thread may destroy it in the meantime.
   *
   * Note uncountedIncRef actually subtracts one from the refcount,
   * and uncountedDecRef adds one to it; but callers shouldn't care.
   */
  void uncountedIncRef() const;
  bool uncountedDecRef() const;
  /*
   * Like cowCheck(), but for uncounted values. Returns true if the count
   * for this value is anything other than "uncounted one".
   *
   * If a caller is going to use this check to mutate an uncounted value,
   * it must know that no other thread has a copy of this value and may
   * mutate the result. An example of when we can use this check is in APC:
   * if we're going to store a value there, and !uncountedCowCheck, then we
   * can modify the object freely before publishing it.
   */
  bool uncountedCowCheck() const;
  /*
   * In debug mode, we call this prior to releasing an uncounted array or
   * string in order to undo the last dec-ref and make the count valid again.
   */
  void uncountedFixCountForRelease() const;
  /*
   * Returns true if the value is persistent (static or uncounted). If it is
   * uncounted, does an uncountedIncRef() before returning.
   */
  bool persistentIncRef() const;
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

ALWAYS_INLINE void MaybeCountable::uncountedIncRef() const {
  assertx(isUncounted());
  auto& count = m_atomic_count;
  auto const DEBUG_ONLY val = count.fetch_sub(1, std::memory_order_relaxed);
  assertx(val <= UncountedValue);
}

ALWAYS_INLINE bool MaybeCountable::uncountedDecRef() const {
  assertx(isUncounted());
  auto const val = m_atomic_count.fetch_add(1, std::memory_order_relaxed);
  return val == UncountedValue;
}

ALWAYS_INLINE bool MaybeCountable::uncountedCowCheck() const {
  assertx(!isRefCounted());
  auto const val = m_atomic_count.load(std::memory_order_relaxed);
  return val != UncountedValue;
}

ALWAYS_INLINE void MaybeCountable::uncountedFixCountForRelease() const {
  assertx(m_count == UncountedZero);
  if (debug) m_atomic_count--;
}

ALWAYS_INLINE bool MaybeCountable::persistentIncRef() const {
  if (isRefCounted()) return false;
  if (isStatic()) return true;
  uncountedIncRef();
  return true;
}

ALWAYS_INLINE bool MaybeCountable::checkCount() const {
  // If this assertion fails, it indicates a double-free. Check it separately.
  assertx(m_count < RefCountMaxRealistic);
  return m_count >= 1 || m_count <= UncountedValue || m_count == StaticValue;
}

ALWAYS_INLINE bool MaybeCountable::checkCountZ() const {
  return m_count == 0 || checkCount();
}

ALWAYS_INLINE bool Countable::checkCount() const {
  // If this assertion fails, it indicates a double-free. Check it separately.
  assertx(m_count < RefCountMaxRealistic);
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
  return uint32_t(m_count) > 1; // treat Static/Uncounted as large counts
}

ALWAYS_INLINE bool Countable::hasMultipleRefs() const {
  assertx(checkCountZ());
  return m_count > 1;
}

ALWAYS_INLINE bool MaybeCountable::hasExactlyOneRef() const {
  assertx(checkCountZ());
  return m_count == OneReference;
}

ALWAYS_INLINE bool MaybeCountable::hasZeroRefs() const {
  assertx(checkCountZ());
  return m_count == 0;
}

ALWAYS_INLINE bool Countable::hasExactlyOneRef() const {
  assertx(checkCountZ());
  return m_count == OneReference;
}

ALWAYS_INLINE void MaybeCountable::incRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());

  if (isRefCounted()) ++m_count;
}

ALWAYS_INLINE void Countable::incRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::rawIncRefCount() const {
  assertx(!tl_sweeping);
  assertx(isRefCounted());
  ++m_count;
}

ALWAYS_INLINE void Countable::rawIncRefCount() const {
  assertx(!tl_sweeping);
  assertx(isRefCounted());
  ++m_count;
}

ALWAYS_INLINE void MaybeCountable::decRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  assertx(hasMultipleRefs());
  if (isRefCounted()) --m_count;
}

ALWAYS_INLINE void Countable::decRefCount() const {
  assertx(!tl_sweeping);
  assertx(checkCount());
  assertx(hasMultipleRefs());
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
  if (m_count == 1) return true;
  if (m_count > 1) --m_count;
  return false;
}

ALWAYS_INLINE void MaybeCountable::fixCountForRelease() {
  if (debug) {
    if (!m_count) ++m_count;
  }
}

ALWAYS_INLINE bool MaybeCountable::countedDecRefAndCheck() {
  assertx(!tl_sweeping);
  assertx(checkCount());
  if (noop_decref) return false;
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
  return m_count <= UncountedValue;
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
