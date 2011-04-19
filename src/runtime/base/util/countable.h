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
#include <util/atomic.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * If a Countable object may be shared by multiple threads, and cannot be set
 * static, we need to create it with the C++ new operator, i.e., not through
 * the SmartAllocator, and call setAtomic() on it.
 * It is possible to wrap an atomic Countable with a SmartPtr, but
 * incRefCount() and decRefCount() are no-ops. Instead, incAtomicCount() and
 * decAtomicCount() are used to maintain its (atomic) reference count.
 */
#define IMPLEMENT_ATOMIC_COUNTABLE_METHODS                              \
  bool isRefCounted() const { return _count < Countable::ATOMIC_FLAG; } \
  void setAtomic() const {                                              \
    if (!isAtomic()) _count = Countable::ATOMIC_FLAG;                   \
  }                                                                     \
  bool isAtomic() const { return _count & Countable::ATOMIC_FLAG; }     \
  void incAtomicCount() const {                                         \
    ASSERT(!isRefCounted());                                            \
    if (isAtomic()) atomic_inc(_count);                                 \
  }                                                                     \
  int decAtomicCount() const {                                          \
    ASSERT(_count > Countable::ATOMIC_FLAG);                            \
    return isAtomic() ? atomic_dec(_count) - Countable::ATOMIC_FLAG     \
                      : _count;                                         \
  }                                                                     \

/**
 * StringData and Variant do not formally derived from Countable, but they
 * have a _count field and define all of the methods from Countable. These
 * macros are provided to avoid code duplication.
 */
#define IMPLEMENT_COUNTABLE_METHODS_NO_STATIC                           \
  void incRefCount() const { if (isRefCounted()) ++_count; }            \
  int decRefCount() const {                                             \
    ASSERT(_count > 0);                                                 \
    return isRefCounted() ? --_count : _count;                          \
  }                                                                     \
  int getCount() const { return _count; }                               \
  IMPLEMENT_ATOMIC_COUNTABLE_METHODS                                    \

#define IMPLEMENT_COUNTABLE_METHODS                                     \
  /* setStatic() is used by StaticString and StaticArray to make  */    \
  /* sure ref count is "never" going to reach 0, even if multiple */    \
  /* threads modify it in a non-thread-safe fashion.              */    \
  void setStatic() const { _count = Countable::STATIC_FLAG; }           \
  bool isStatic() const { return _count == STATIC_FLAG; }               \
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC                                 \

/**
 * Implements reference counting. We could have used boost::shared_ptr<T> for
 * reference counting, but deriving our classes from Countable is more
 * efficient, both in time and space. This is because _count is not separately
 * allocated from the object, and it's an int than a 64-bit pointer. We
 * achieved this because we ask our classes to derive from Countable,
 * something boost::shared_ptr<T> doesn't have the luxury to.
 */

class Countable {
 public:
  static const int STATIC_FLAG = (1 << 30);
  static const int ATOMIC_FLAG = (1 << 29);

  Countable() : _count(0) {}
  IMPLEMENT_COUNTABLE_METHODS
 protected:
  mutable int _count;
};

/**
 * CountableNF : countable no flags
 */

#define IMPLEMENT_COUNTABLENF_METHODS_NO_STATIC                               \
  void setAtomic() const { ASSERT(false); }                                   \
  bool isAtomic() const { return false;   }                                   \
  bool isRefCounted() const { return true; }                                  \
  void incAtomicCount() const { ASSERT(false); }                              \
  int decAtomicCount() const { ASSERT(false); return _count; }                \
  void incRefCount() const { ++_count; }                                      \
  int decRefCount() const { ASSERT(_count > 0); return --_count; }            \
  int getCount() const { return _count; }                                     \

class CountableNF : public Countable {
 public:
  void setStatic() const { ASSERT(false); }
  bool isStatic() const { return false;   }
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_COUNTABLE_H__
