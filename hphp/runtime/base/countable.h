/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::checkCount() const {
  return count >= 1 ||
         (CNT == Counted::Maybe &&
          (count == UncountedValue || count == StaticValue));
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::isRefCounted() const {
  return CNT == Counted::Always || count >= 0;
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::hasMultipleRefs() const {
  assert(checkCount());
  return uint32_t(count) > 1; // treat Static/Uncounted as large positive counts
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::hasExactlyOneRef() const {
  assert(checkCount());
  return count == 1;
}

template<class T, Counted CNT> ALWAYS_INLINE
void HeaderWord<T,CNT>::incRefCount() const {
  assert(checkCount() || count == 0 /* due to static init order */);
  if (isRefCounted()) ++count;
}

template<class T, Counted CNT> ALWAYS_INLINE
void HeaderWord<T,CNT>::rawIncRefCount() const {
  assert(isRefCounted());
  ++count;
}

template<class T, Counted CNT> ALWAYS_INLINE
void HeaderWord<T,CNT>::decRefCount() const {
  assert(checkCount());
  if (isRefCounted()) --count;
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::decWillRelease() const {
  return !hasMultipleRefs();
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::decReleaseCheck() {
  assert(checkCount());
  if (count == 1) return true;
  if (CNT == Counted::Always || count > 1) --count;
  return false;
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::isStatic() const {
  assert(checkCount());
  return CNT == Counted::Maybe && count == StaticValue;
}

template<class T, Counted CNT> ALWAYS_INLINE
bool HeaderWord<T,CNT>::isUncounted() const {
  assert(checkCount());
  return CNT == Counted::Maybe && count == UncountedValue;
}

/**
 * Ref-counted types have a HeaderWord field at HeaderOffset,
 * and define counting methods with these macros.
 */

#define IMPLEMENT_COUNTABLE_METHODS                                     \
  bool checkCount() const {                                             \
    assert(kindIsValid());                                              \
    return m_hdr.checkCount();                                          \
  }                                                                     \
  bool isRefCounted() const {                                           \
    assert(kindIsValid());                                              \
    return m_hdr.isRefCounted();                                        \
  }                                                                     \
  bool hasMultipleRefs() const {                                        \
    assert(kindIsValid());                                              \
    return m_hdr.hasMultipleRefs();                                     \
  }                                                                     \
  bool hasExactlyOneRef() const {                                       \
    assert(kindIsValid());                                              \
    return m_hdr.hasExactlyOneRef();                                    \
  }                                                                     \
  bool cowCheck() const {                                               \
    return m_hdr.hasMultipleRefs();                                     \
  }                                                                     \
  void incRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    m_hdr.incRefCount();                                                \
  }                                                                     \
  void rawIncRefCount() const {                                         \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    m_hdr.rawIncRefCount();                                             \
  }                                                                     \
  void decRefCount() const {                                            \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    m_hdr.decRefCount();                                                \
  }                                                                     \
  bool decWillRelease() const {                                         \
    assert(kindIsValid());                                              \
    return m_hdr.decWillRelease();                                      \
  }                                                                     \
  ALWAYS_INLINE bool decReleaseCheck() {                                \
    assert(!MemoryManager::sweeping());                                 \
    assert(kindIsValid());                                              \
    return m_hdr.decReleaseCheck();                                     \
  }                                                                     \
  ALWAYS_INLINE void decRefAndRelease() {                               \
    assert(kindIsValid());                                              \
    if (decReleaseCheck()) release();                                   \
  }                                                                     \
  bool isStatic() const {                                               \
    assert(kindIsValid());                                              \
    return m_hdr.isStatic();                                            \
  }                                                                     \
  bool isUncounted() const {                                            \
    assert(kindIsValid());                                              \
    return m_hdr.isUncounted();                                         \
  }

}
#endif // incl_HPHP_COUNTABLE_H_
