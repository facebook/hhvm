/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifdef DEBUG_RACE_CONDITION
#include <util/lock.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

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
  Countable() : _count(0) {
  }

  void incRefCount() const {
#ifdef DEBUG_RACE_CONDITION
    Lock lock(m_mutex);
#endif
    ++_count;
  }

  int decRefCount() const {
#ifdef DEBUG_RACE_CONDITION
    Lock lock(m_mutex);
#endif
    ASSERT(_count > 0);
    return --_count;
  }

  int getCount() const {
#ifdef DEBUG_RACE_CONDITION
    Lock lock(m_mutex);
#endif
    return _count;
  }

  /**
   * Used by StaticString and StaticArray to make sure ref count is "never"
   * going to reach 0, even if multiple threads modify it in a non-thread-safe
   * fashion.
   */
  void setStatic() const {
    _count = (1 << 30); // INT_MAX / 2
  }

  /**
   * For debugging purpose mostly.
   */
  bool isStatic() const {
    return _count > (1 << 29);
  }

 protected:
#ifdef DEBUG_RACE_CONDITION
  mutable Mutex m_mutex;
#endif
  mutable int _count;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_COUNTABLE_H__
