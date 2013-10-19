/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/base.h"
#include "hphp/util/util.h"
#include "hphp/util/trace.h"
#include "hphp/util/atomic.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * This bit flags a reference count as "static".  If a reference count
 * is static, it means we should never increment or decrement it: the
 * object lives across requests and may be accessed by multiple
 * threads.
 */
const int32_t RefCountStaticValue = (1 << 30);

/*
 * RefCount type for m_count field in refcounted objects
 */
typedef int32_t RefCount;

/*
 * Used for assertions.  Real count values should always be less than
 * or equal to RefCountStaticValue, and asserting this will also catch
 * common malloc freed-memory patterns (e.g. 0x5a5a5a5a and smart
 * allocator's 0x6a6a6a6a).
 */
inline DEBUG_ONLY bool is_refcount_realistic(int32_t count) {
  return count <= RefCountStaticValue && count >= 0;
}

/**
 * Ref-counted types have a m_count field at FAST_REFCOUNT_OFFSET
 * and define counting methods with these macros.
 */
#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC           \
  RefCount getCount() const {                           \
    assert(is_refcount_realistic(m_count));             \
    return m_count;                                     \
  }                                                     \
                                                        \
  bool isRefCounted() const {                           \
    assert(is_refcount_realistic(m_count));             \
    return m_count != RefCountStaticValue;              \
  }                                                     \
                                                        \
  bool hasMultipleRefs() const {                        \
    assert(is_refcount_realistic(m_count));             \
    return (uint32_t)m_count > 1;                       \
  }                                                     \
                                                        \
  void incRefCount() const {                            \
    assert(!MemoryManager::sweeping());                 \
    assert(is_refcount_realistic(m_count));             \
    if (isRefCounted()) { ++m_count; }                  \
  }                                                     \
                                                        \
  RefCount decRefCount() const {                        \
    assert(!MemoryManager::sweeping());                 \
    assert(m_count > 0);                                \
    assert(is_refcount_realistic(m_count));             \
    return isRefCounted() ? --m_count : m_count;        \
  }

#define IMPLEMENT_COUNTABLE_METHODS             \
  void setStatic() const {                      \
    assert(is_refcount_realistic(m_count));     \
    m_count = RefCountStaticValue;              \
  }                                             \
  bool isStatic() const {                       \
    assert(is_refcount_realistic(m_count));     \
    return m_count == RefCountStaticValue;      \
  }                                             \
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

#define IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC \
  RefCount getCount() const {                   \
    assert(is_refcount_realistic(m_count));     \
    return m_count;                             \
  }                                             \
                                                \
  bool isRefCounted() const { return true; }    \
                                                \
  bool hasMultipleRefs() const {                \
    assert(m_count >= 0);                       \
    assert(is_refcount_realistic(m_count));     \
    return m_count > 1;                         \
  }                                             \
                                                \
  void incRefCount() const {                    \
    assert(!MemoryManager::sweeping());         \
    assert(is_refcount_realistic(m_count));     \
    ++m_count;                                  \
  }                                             \
                                                \
  RefCount decRefCount() const {                \
    assert(m_count > 0);                        \
    assert(is_refcount_realistic(m_count));     \
    return --m_count;                           \
  }

class ObjectData;

/* We only use this to hold objects */
class CountableHelper : private boost::noncopyable {
public:
  explicit CountableHelper(ObjectData* object);
  ~CountableHelper();
private:
  ObjectData *m_object;
};

/**
 * If an object may be shared by multiple threads but we want to reclaim it
 * when all threads are finished using it, we need to allocate it with the C++
 * new operator (instead of SmartAllocator) and we need to use AtomicSmartPtr
 * instead of SmartPtr.
 */
class AtomicCountable {
 public:
  AtomicCountable() : m_count(0) {}
  RefCount getCount() const { return m_count; }
  void incAtomicCount() const { ++m_count; }
  RefCount decAtomicCount() const { return --m_count; }
 protected:
  mutable std::atomic<RefCount> m_count;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_COUNTABLE_H_
