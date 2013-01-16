/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_COUNTABLE_H__
#define __HPHP_COUNTABLE_H__

#include <util/base.h>
#include <util/util.h>
#include <util/trace.h>
#include <util/atomic.h>

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
 * Used for assertions.  Real count values should always be less than
 * or equal to RefCountStaticValue, and asserting this will also catch
 * common malloc freed-memory patterns (e.g. 0x5a5a5a5a and smart
 * allocator's 0x6a6a6a6a).
 */
inline DEBUG_ONLY bool is_refcount_realistic(int32_t count) {
  return count <= RefCountStaticValue && count >= 0;
}

/**
 * StringData and Variant do not formally derive from Countable, but they
 * have a _count field and define all of the methods from Countable. These
 * macros are provided to avoid code duplication.
 */
#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC   \
  int32_t getCount() const {                    \
    assert(is_refcount_realistic(_count));      \
    return _count;                              \
  }                                             \
                                                \
  bool isRefCounted() const {                   \
    assert(is_refcount_realistic(_count));      \
    return _count != RefCountStaticValue;       \
  }                                             \
                                                \
  void incRefCount() const {                    \
    assert(is_refcount_realistic(_count));      \
    if (isRefCounted()) { ++_count; }           \
  }                                             \
                                                \
  int32_t decRefCount() const {                 \
    assert(_count > 0);                         \
    assert(is_refcount_realistic(_count));      \
    return isRefCounted() ? --_count : _count;  \
  }

#define IMPLEMENT_COUNTABLE_METHODS                                   \
  void setStatic() const {                                            \
    assert(is_refcount_realistic(_count));                            \
    _count = RefCountStaticValue;                                     \
  }                                                                   \
  bool isStatic() const {                                             \
    assert(is_refcount_realistic(_count));                            \
    return _count == STATIC_FLAG;                                     \
  }                                                                   \
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

/**
 * Implements reference counting. We chose to roll our own SmartPtr type
 * instead of using boost::shared_ptr<T> so that we can achieve better
 * performance by directly embedding the reference count in the object.
 */
class Countable {
 public:
  static const int32_t STATIC_FLAG = RefCountStaticValue;
  Countable() : _count(0) {}
  IMPLEMENT_COUNTABLE_METHODS
 protected:
  mutable int32_t _count;
};

#define IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC \
  int32_t getCount() const {                    \
    assert(is_refcount_realistic(_count));      \
    return _count;                              \
  }                                             \
                                                \
  bool isRefCounted() const { return true; }    \
                                                \
  void incRefCount() const {                    \
    assert(is_refcount_realistic(_count));      \
    ++_count;                                   \
  }                                             \
                                                \
  int32_t decRefCount() const {                 \
    assert(_count > 0);                         \
    assert(is_refcount_realistic(_count));      \
    return --_count;                            \
  }

/**
 * CountableNF : countable no flags
 */
class CountableNF : public Countable {
 public:
  void setStatic() const { assert(false); }
  bool isStatic() const { return false; }
  IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC;
};

/* We only use this to hold objects */
class CountableHelper {
public:
  CountableHelper(CountableNF *countable) : m_countable(countable) {
    m_countable->incRefCount();
  }
  ~CountableHelper() {
    m_countable->decRefCount();
  }
private:
  CountableNF *m_countable;
};

/**
 * If an object may be shared by multiple threads but we want to reclaim it
 * when all threads are finished using it, we need to allocate it with the C++
 * new operator (instead of SmartAllocator) and we need to use AtomicSmartPtr
 * instead of SmartPtr.
 */
class AtomicCountable {
 public:
  AtomicCountable() : _count(0) {}
  int32_t getCount() const { return _count; }
  void incAtomicCount() const { atomic_inc(_count); }
  int decAtomicCount() const { return atomic_dec(_count); }
 protected:
  mutable int32_t _count;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_COUNTABLE_H__
