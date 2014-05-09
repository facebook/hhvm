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

#ifndef incl_HPHP_LOW_PTR_H_
#define incl_HPHP_LOW_PTR_H_

#include "hphp/util/assertions.h"

#include "folly/CPortability.h" // FOLLY_SANITIZE_ADDRESS

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <utility>

#ifdef FOLLY_SANITIZE_ADDRESS
#undef USE_LOWPTR
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Low memory pointer template.
 *
 * Defaults to a 4-byte pointer, but can be configured.
 */
template <typename T, typename S = uint32_t>
class LowPtr {
public:
  /**
   * Constructors.
   */
  LowPtr() {}

  /* implicit */ LowPtr(T* px) : m_raw(toLow(px)) {}

  /* implicit */ LowPtr(std::nullptr_t px) : m_raw(0) {}

  LowPtr(const LowPtr<T, S>& r) : m_raw(r.raw()) {}

  LowPtr(LowPtr<T>&& r) : m_raw(r.raw()) {
    r.m_raw = 0;
  }

  /**
   * Assignments.
   */
  LowPtr& operator=(T* px) {
    return operator=(toLow(px));
  }

  LowPtr& operator=(std::nullptr_t px) {
    return operator=((S)0);
  }

  LowPtr& operator=(const LowPtr<T, S>& r) {
    return operator=(r.raw());
  }

  LowPtr& operator=(LowPtr<T, S>&& r) {
    assert(this != &r);
    m_raw = std::move(r.m_raw);
    return *this;
  }

  /**
   * Observers.
   */
  T* get() const {
    return reinterpret_cast<T*>(m_raw);
  }

  T& operator*() const {
    return *get();
  }

  T* operator->() const {
    return get();
  }

  /* implicit */ operator T*() const {
    return get();
  }

  explicit operator bool() const {
    return get();
  }

  /**
   * Modifiers.
   */
  void reset() {
    operator=(nullptr);
  }

  void swap(LowPtr& r) {
    std::swap(m_raw, r.m_raw);
  }

private:
  /**
   * Raw value manipulation.
   */
  S raw() const {
    return m_raw;
  }

  LowPtr& operator=(S raw) {
    m_raw = raw;
    return *this;
  }

  static bool isLow(T* px) {
    S ones = ~0;
    auto ptr = reinterpret_cast<uintptr_t>(px);
    return (ptr & ones) == ptr;
  }

  static S toLow(T* px) {
    always_assert(isLow(px));
    return (S)((uintptr_t)(px));
  }

protected:
  S m_raw;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_LOW_PTR_H_
