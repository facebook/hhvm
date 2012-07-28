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
#include <util/trace.h>
#include <util/atomic.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * StringData and Variant do not formally derive from Countable, but they
 * have a _count field and define all of the methods from Countable. These
 * macros are provided to avoid code duplication.
 */
#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC                            \
  int32_t getCount() const { return _count; }                            \
  bool isRefCounted() const { return _count != Countable::STATIC_FLAG; } \
  void incRefCount() const { if (isRefCounted()) { ++_count; } }         \
  int32_t decRefCount() const {                                          \
    ASSERT(_count > 0);                                                  \
    return isRefCounted() ? --_count : _count;                           \
  }

#define IMPLEMENT_COUNTABLE_METHODS                                      \
  /* setStatic() is used by StaticString and StaticArray to make  */     \
  /* sure ref count is "never" going to reach 0, even if multiple */     \
  /* threads modify it in a non-thread-safe fashion.              */     \
  void setStatic() const { _count = Countable::STATIC_FLAG; }            \
  bool isStatic() const { return _count == STATIC_FLAG; }                \
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

const int32_t RefCountStaticValue = (1 << 30);

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

/**
 * This is a special value for _count used to indicate objects that
 * are already deallocated. (See smart_allocator.h.)
 */
const int32_t RefCountTombstoneValue = 0xde1ee7ed;

#define IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC                               \
  int32_t getCount() const { return _count; }                                 \
  bool isRefCounted() const { return true; }                                  \
  void incRefCount() const { ++_count; }                                      \
  int32_t decRefCount() const { ASSERT(_count > 0); return --_count; }

/**
 * CountableNF : countable no flags
 */
class CountableNF : public Countable {
 public:
  void setStatic() const { ASSERT(false); }
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
