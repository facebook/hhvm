/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/util/atomic-countable.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/ptr.h"

#include <type_traits>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// AtomicSharedPtr

/**
 * Thread-safe ref-counting smart pointer.
 */
template<typename T, typename S>
struct AtomicSharedPtrImpl {
  explicit AtomicSharedPtrImpl(T* px = nullptr) noexcept : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }

  template<class Y>
  explicit AtomicSharedPtrImpl(Y* px) noexcept : m_px(px) {
    if (m_px) m_px->incAtomicCount();
  }

  AtomicSharedPtrImpl(const AtomicSharedPtrImpl& src) noexcept
    : m_px(nullptr) {
    operator=(src.get());
  }

  template<class Y>
  AtomicSharedPtrImpl(const AtomicSharedPtrImpl<Y, S>& src) noexcept
    : m_px(nullptr) {
    operator=(src.get());
  }

  AtomicSharedPtrImpl(AtomicSharedPtrImpl&& src) noexcept
    : m_px(std::exchange(src.m_px, nullptr)) {}

  ~AtomicSharedPtrImpl() {
    if (m_px && m_px->decAtomicCount() == 0) {
      m_px->atomicRelease();
    }
  }

  /**
   * Assignments.
   */

  AtomicSharedPtrImpl& operator=(const AtomicSharedPtrImpl& src) noexcept {
    return operator=(src.m_px);
  }

  AtomicSharedPtrImpl& operator=(AtomicSharedPtrImpl&& src) noexcept {
    if (m_px && m_px->decAtomicCount() == 0) {
      m_px->atomicRelease();
    }
    m_px = nullptr; // guard against self-assignment!
    m_px = std::exchange(src.m_px, nullptr);
    return *this;
  }

  template<class Y>
  AtomicSharedPtrImpl& operator=(const AtomicSharedPtrImpl<Y, S>& src) noexcept {
    return operator=(src.get());
  }

  AtomicSharedPtrImpl& operator=(T* px) noexcept {
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
  AtomicSharedPtrImpl& operator=(Y* px) noexcept {
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
  explicit operator bool() const noexcept { return m_px != nullptr; }

  /**
   * Magic delegation.
   */
  T* operator->() const noexcept {
    return m_px;
  }

  /**
   * Get the raw pointer.
   */
  T* get() const noexcept {
    return m_px;
  }

  /**
   * Reset the raw pointer.
   */
  void reset(T* p = nullptr) noexcept {
    operator=(p);
  }

protected:
  void overwrite_unsafe(T* ptr) noexcept {
    assertx(!m_px);
    m_px = ptr;
  }

private:
  S m_px;
};

template<typename T>
using AtomicSharedPtr = AtomicSharedPtrImpl<T, T*>;

template<typename T>
using AtomicSharedLowPtr = AtomicSharedPtrImpl<T, LowPtr<T>>;

template<typename T>
using AtomicSharedPackedPtr = AtomicSharedPtrImpl<T, PackedPtr<T>>;

}
