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

#include <boost/static_assert.hpp>
#include <util/base.h>
#include <runtime/base/util/exceptions.h>
#include <runtime/base/util/countable.h>
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Because of type punning with the *NR classes (ArrNR, StrNR, etc),
 * we want to assert that offsetof(T, m_px) is a specific value in all
 * these classes.
 */
static const std::size_t kExpectedMPxOffset = 0;

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
  SmartPtr() : m_px(nullptr) {}
  SmartPtr(T* px) : m_px(px) { if (m_px) m_px->incRefCount(); }
  SmartPtr(const SmartPtr<T>& src) : m_px(src.get()) {
    if (m_px) m_px->incRefCount();
  }

  // Move ctor
  SmartPtr(SmartPtr&& src) : m_px(src.get()) {
    src.m_px = nullptr;
  }

  template<class Y>
  SmartPtr(const SmartPtr<Y>& src) : m_px(src.get()) {
    if (m_px) m_px->incRefCount();
  }
  // Move ctor for derived types
  template<class Y>
  SmartPtr(SmartPtr<Y>&& src) : m_px(src.get()) {
    src.m_px = nullptr;
  }

  ~SmartPtr() {
    if (m_px && m_px->decRefCount() == 0) {
      m_px->release();
    }
  }

  /**
   * Assignments.
   */
  SmartPtr& operator=(const SmartPtr<T>& src) {
    return operator=(src.m_px);
  }
  // Move assignment
  SmartPtr& operator=(SmartPtr&& src) {
    // a = std::move(a), ILLEGAL per C++11 17.6.4.9
    assert(this != &src);
    std::swap(m_px, src.m_px);
    return *this;
  }
  template<class Y>
  SmartPtr& operator=(const SmartPtr<Y>& src) {
    return operator=(src.get());
  }
  // Move assignment for derived types
  template<class Y>
  SmartPtr& operator=(SmartPtr<Y>&& src) {
    assert(this != &src);
    // Update m_px before releasing the goner
    auto goner = m_px;
    m_px = src.m_px;
    src.m_px = nullptr;
    if (goner && !goner->decRefCount()) goner->release();
    return *this;
  }
  SmartPtr& operator=(T* px) {
    // Incidentally works with self-assignment because incRefCount is
    // called before decRefCount.
    if (px) px->incRefCount();
    auto goner = m_px;
    m_px = px;
    if (goner && !goner->decRefCount()) goner->release();
    return *this;
  }

  /**
   * Swap two smart pointers
   */
  void swap(SmartPtr & rhs) {
    std::swap(m_px, rhs.m_px);
  }

  /**
   * Magic delegation.
   */
  T* operator->() const {
    if (UNLIKELY(!m_px)) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Get the raw pointer.
   */
  T* get() const {
    return m_px;
  }

  /**
   * Reset the raw pointer.
   */
  void reset() {
    operator=((T*)nullptr);
  }

protected:
  T* m_px;  // raw pointer

  static void compileTimeAssertions() {
    BOOST_STATIC_ASSERT((offsetof(SmartPtr, m_px) == kExpectedMPxOffset));
  }
};

///////////////////////////////////////////////////////////////////////////////
// AtomicSmartPtr

/**
 * Thread-safe ref-counting smart pointer.
 */
template<typename T>
class AtomicSmartPtr {
public:
  AtomicSmartPtr() : m_px(nullptr) {}
  AtomicSmartPtr(T* px) : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }
  template<class Y>
  AtomicSmartPtr(Y* px) : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }
  AtomicSmartPtr(const AtomicSmartPtr<T>& src) : m_px(nullptr) {
    operator=(src.get());
  }
  template<class Y>
  AtomicSmartPtr(const AtomicSmartPtr<Y>& src) : m_px(nullptr) {
    operator=(src.get());
  }
  ~AtomicSmartPtr() {
    if (m_px && m_px->decAtomicCount() == 0) {
      m_px->atomicRelease();
    }
  }

  /**
   * Assignments.
   */
  AtomicSmartPtr& operator=(const AtomicSmartPtr<T>& src) {
    return operator=(src.m_px);
  }
  template<class Y>
  AtomicSmartPtr& operator=(const AtomicSmartPtr<Y>& src) {
    return operator=(src.get());
  }
  AtomicSmartPtr& operator=(T* px) {
    if (m_px != px) {
      if (m_px && m_px->decAtomicCount() == 0) {
        delete m_px;
      }
      m_px = px;
      if (m_px) {
        m_px->incAtomicCount();
      }
    }
    return *this;
  }
  template<class Y>
  AtomicSmartPtr& operator=(Y* px) {
    T* npx = dynamic_cast<T*>(px);
    if (m_px != npx) {
      if (m_px && m_px->decAtomicCount() == 0) {
        delete m_px;
      }
      m_px = npx;
      if (m_px) {
        m_px->incAtomicCount();
      }
    }
    return *this;
  }

  /**
   * Magic delegation.
   */
  T* operator->() const {
    if (!m_px) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Get the raw pointer.
   */
  T* get() const {
    return m_px;
  }

  /**
   * Reset the raw pointer.
   */
  void reset() {
    operator=((T*)nullptr);
  }

protected:
  T* m_px; // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_PTR_H__
