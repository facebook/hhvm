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

#include <atomic>
#include <cstdlib>
#include <memory>
#include <utility>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A refcounted smart pointer that does deep copies if you ask for a
 * mutable copy when the ref-count is greater than one.
 */
template<class T>
struct copy_ptr {
  copy_ptr() noexcept {}
  copy_ptr(const copy_ptr& o) {
    inc_ref(m_p = o.m_p);
  }
  copy_ptr(copy_ptr&& o) noexcept {
    m_p = o.m_p;
    o.m_p = nullptr;
  }
  template <typename... Args> explicit copy_ptr(Args&&... args) {
    construct(nullptr, std::forward<Args>(args)...);
  }
  ~copy_ptr() { dec_ref(m_p); }
  copy_ptr& operator=(const copy_ptr& o) {
    auto const save = m_p;
    inc_ref(m_p = o.m_p);
    dec_ref(save);
    return *this;
  }
  copy_ptr& operator=(copy_ptr&& o) noexcept {
    std::swap(m_p, o.m_p);
    return *this;
  }

  explicit operator bool() const { return !isNull(); }
  bool isNull() const { return m_p == nullptr; }

  const T& operator*() const { return *m_p; }
  const T* operator->() const { return m_p; }
  const T* get() const { return m_p; }
  T* mutate() {
    if (m_p && get_ref(m_p) != 1) {
      emplace(*m_p);
    }
    return m_p;
  }

  void reset() {
    dec_ref(m_p);
    m_p = nullptr;
  }

  template <typename... Args> T* emplace(Args&&... args) {
    auto const save = m_p;
    construct(save, std::forward<Args>(args)...);
    dec_ref(save);
    return m_p;
  }

  friend bool
  operator==(const copy_ptr& a, const copy_ptr& b) { return a.m_p == b.m_p; }
  friend bool
  operator==(const copy_ptr& a, const T* b)        { return a.m_p == b; }
  friend bool
  operator==(const T* a, const copy_ptr& b)        { return a == b.m_p; }
  friend bool
  operator!=(const copy_ptr& a, const copy_ptr& b) { return a.m_p != b.m_p; }
  friend bool
  operator!=(const copy_ptr& a, const T* b)        { return a.m_p != b; }
  friend bool
  operator!=(const T* a, const copy_ptr& b)        { return a != b.m_p; }

private:
  template <typename... Args> void construct(T* save, Args&&... args) {
    auto const mem = std::malloc(data_offset() + sizeof(T));
    if (!mem) throw std::bad_alloc();
    new (mem) refcount_type{1};
    m_p = (T*)((char*)mem + data_offset());
    try {
      new (m_p) T(std::forward<Args>(args)...);
    } catch (...) {
      std::free(mem);
      m_p = save;
      throw;
    }
  }

  using refcount_type = std::atomic<uint32_t>;

  T* m_p{};

  static constexpr size_t data_offset() {
    return alignof(T) >= sizeof(refcount_type) ?
      alignof(T) : sizeof(refcount_type);
  }

  static refcount_type* get_ref_ptr(T* p) {
    return (refcount_type*)((char*)p - data_offset());
  }

  static uint32_t get_ref(T* p) {
    return get_ref_ptr(p)->load(std::memory_order_relaxed);
  }

  static void dec_ref(T* p) {
    if (!p) return;
    auto ref = get_ref_ptr(p);
    if (ref->fetch_sub(1, std::memory_order_relaxed) == 1) {
      p->~T();
      ref->~refcount_type();
      std::free(ref);
    }
  }

  static void inc_ref(T* p) {
    if (!p) return;
    get_ref_ptr(p)->fetch_add(1, std::memory_order_relaxed);
  }
};

//////////////////////////////////////////////////////////////////////

}
