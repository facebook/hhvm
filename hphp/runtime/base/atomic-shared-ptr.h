/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ATOMIC_SHARED_PTR_H_
#define incl_HPHP_ATOMIC_SHARED_PTR_H_

#include "hphp/util/low-ptr.h"

#include <cassert>
#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// AtomicSharedPtr

/**
 * Thread-safe ref-counting smart pointer.
 */
template<typename T, bool isLow>
struct AtomicSharedPtrImpl {
  explicit AtomicSharedPtrImpl(T* px = nullptr) : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }

  template<class Y>
  explicit AtomicSharedPtrImpl(Y* px) : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }

  AtomicSharedPtrImpl(const AtomicSharedPtrImpl& src)
    : m_px(nullptr) {
    operator=(src.get());
  }

  template<class Y>
  AtomicSharedPtrImpl(const AtomicSharedPtrImpl<Y, isLow>& src)
    : m_px(nullptr) {
    operator=(src.get());
  }

  ~AtomicSharedPtrImpl() {
    if (m_px && m_px->decAtomicCount() == 0) {
      m_px->atomicRelease();
    }
  }

  /**
   * Assignments.
   */

  AtomicSharedPtrImpl& operator=(const AtomicSharedPtrImpl& src) {
    return operator=(src.m_px);
  }

  template<class Y>
  AtomicSharedPtrImpl& operator=(const AtomicSharedPtrImpl<Y, isLow>& src) {
    return operator=(src.get());
  }

  AtomicSharedPtrImpl& operator=(T* px) {
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
  AtomicSharedPtrImpl& operator=(Y* px) {
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
   * Safe bool cast.
   */
  explicit operator bool() const { return m_px != nullptr; }

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
  void reset(T* p = nullptr) {
    operator=(p);
  }

protected:
  void overwrite_unsafe(T* ptr) {
    assert(!m_px);
    m_px = ptr;
  }

private:
  typename std::conditional<isLow, LowPtr<T>, T*>::type m_px;
};

template<typename T>
using AtomicSharedPtr = AtomicSharedPtrImpl<T, false>;

template<typename T>
using AtomicSharedLowPtr = AtomicSharedPtrImpl<T, true>;

}

#endif
