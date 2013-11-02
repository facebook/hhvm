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

#ifndef incl_HPHP_SMART_PTR_H_
#define incl_HPHP_SMART_PTR_H_

#include <boost/static_assert.hpp>
#include "hphp/util/base.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/countable.h"
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Because of type punning with the *NR classes (ArrNR, StrNR, etc),
 * we want to assert that offsetof(T, m_px) is a specific value in all
 * these classes.
 */
const std::size_t kExpectedMPxOffset = 0;

template<typename T>
class SmartPtr {
public:
  SmartPtr() : m_px(nullptr) {}
  explicit SmartPtr(T* px) : m_px(px) { if (m_px) m_px->incRefCount(); }
  SmartPtr(const SmartPtr<T>& src) : m_px(src.get()) {
    if (m_px) m_px->incRefCount();
  }

  enum class NoIncRef {};
  explicit SmartPtr(T* px, NoIncRef) : m_px(px) {}

  enum class NonNull { Tag };
  explicit SmartPtr(T* px, NonNull) : m_px(px) {
    assert(px);
    m_px->incRefCount();
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
    decRefPtr(m_px);
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
    decRefPtr(goner);
    return *this;
  }
  SmartPtr& operator=(T* px) {
    // Works with self-assignment because incRefCount is
    // called before decRefCount.
    if (px) px->incRefCount();
    auto goner = m_px;
    m_px = px;
    decRefPtr(goner);
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

private:
  static ALWAYS_INLINE void decRefPtr(T* ptr) {
    if (ptr) ptr->decRefAndRelease();
  }
  static void compileTimeAssertions() {
    static_assert(offsetof(SmartPtr, m_px) == kExpectedMPxOffset, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
// AtomicSmartPtr

/**
 * Thread-safe ref-counting smart pointer.
 */
template<typename T>
struct AtomicSmartPtr {
  explicit AtomicSmartPtr(T* px = nullptr) : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }

  template<class Y>
  explicit AtomicSmartPtr(Y* px) : m_px(px) {
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
        m_px->atomicRelease();
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
        m_px->atomicRelease();
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
  void overwrite_unsafe(T* ptr) {
    assert(!m_px);
    m_px = ptr;
  }

private:
  T* m_px;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SMART_PTR_H_
