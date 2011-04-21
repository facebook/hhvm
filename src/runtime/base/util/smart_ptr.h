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

#ifndef __HPHP_SMART_PTR_H__
#define __HPHP_SMART_PTR_H__

#include <util/base.h>
#include <runtime/base/util/exceptions.h>
#include <runtime/base/util/countable.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Work with Countable to implement reference counting. For example,
 *
 *   class MyClassData : public Countable {
 *     // now MyClassData has a counter to keep track of references
 *   };
 *
 *   class MyClass : public SmartPtr<MyClassData> {
 *     // now MyClassData becomes an inner pointer that's managed by SmartPtr
 *   };
 */
template<typename T>
class SmartPtr {
public:
  /**
   * C++ code is always generated with 2-phase object creations by declaring
   * a smart pointer first, then assigning with a new-ed object raw pointer.
   */
  SmartPtr() : m_px(NULL) {}

  SmartPtr(T *px) : m_px(px) { if (m_px) m_px->incRefCount();}

  /**
   * Copy constructor.
   */
  SmartPtr(const SmartPtr<T> &src) : m_px(src.m_px) {
    if (m_px) m_px->incRefCount();
  }
  template<class Y>
  SmartPtr(const SmartPtr<Y> &src) : m_px(src.get()) {
    if (m_px) m_px->incRefCount();
  }

  /**
   * If I'm the last one who holds a reference to the object, destroy it.
   */
  ~SmartPtr() {
    if (m_px && m_px->decRefCount() == 0) {
      m_px->release();
    }
  }

  /**
   * Assignments.
   */
  SmartPtr &operator=(const SmartPtr<T> &src) {
    return operator=(src.m_px);
  }
  template<class Y>
  SmartPtr &operator=(const SmartPtr<Y> &src) {
    return operator=(src.get());
  }

  SmartPtr &operator=(T *px) {
    if (m_px != px) {
      if (m_px && m_px->decRefCount() == 0) {
        m_px->release();
      }
      m_px = px;
      if (m_px) m_px->incRefCount();
    }
    return *this;
  }

  /**
   * Magic delegation.
   */
  T *operator->() const {
    if (!m_px) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Get the raw pointer.
   */
  T *get() const {
    return m_px;
  }

  /**
   * Reset the raw pointer.
   */
  void reset() {
    operator=((T*)NULL);
  }

protected:
  T *m_px;  // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
// AtomicSmartPtr

/**
 * Thread-safe ref-counting smart pointer.
 */
template<typename T>
class AtomicSmartPtr {
public:
  AtomicSmartPtr() : m_px(NULL) {}

  AtomicSmartPtr(T *px) : m_px(px) {
    if (m_px) {
      m_px->setAtomic();
      m_px->incAtomicCount();
    }
  }
  template<class Y>
  AtomicSmartPtr(Y *px) : m_px(px) {
    if (m_px) {
      m_px->setAtomic();
      m_px->incAtomicCount();
    }
  }

  /**
   * Copy constructors.
   */
  AtomicSmartPtr(const AtomicSmartPtr<T> &src) : m_px(NULL) {
    operator=(src.m_px);
  }
  template<class Y>
  AtomicSmartPtr(const AtomicSmartPtr<Y> &src) : m_px(NULL) {
    operator=(src.get());
  }

  ~AtomicSmartPtr() {
    if (m_px && m_px->decAtomicCount() == 0) {
      delete m_px;
    }
  }

  /**
   * Assignments.
   */
  AtomicSmartPtr &operator=(const AtomicSmartPtr<T> &src) {
    return operator=(src.m_px);
  }
  template<class Y>
  AtomicSmartPtr &operator=(const AtomicSmartPtr<Y> &src) {
    return operator=(src.get());
  }
  AtomicSmartPtr &operator=(T *px) {
    if (m_px != px) {
      if (m_px && m_px->decAtomicCount() == 0) {
        delete m_px;
      }
      m_px = px;
      if (m_px) {
        m_px->setAtomic();
        m_px->incAtomicCount();
      }
    }
    return *this;
  }
  template<class Y>
  AtomicSmartPtr &operator=(Y *px) {
    T *npx = dynamic_cast<T *>(px);
    if (m_px != npx) {
      if (m_px && m_px->decAtomicCount() == 0) {
        delete m_px;
      }
      m_px = npx;
      if (m_px) {
        m_px->setAtomic();
        m_px->incAtomicCount();
      }
    }
    return *this;
  }

  /**
   * Magic delegation.
   */
  T *operator->() const {
    if (!m_px) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Get the raw pointer.
   */
  T *get() const {
    return m_px;
  }

  /**
   * Reset the raw pointer.
   */
  void reset() {
    operator=((T*)NULL);
  }

protected:
  T *m_px; // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_PTR_H__
