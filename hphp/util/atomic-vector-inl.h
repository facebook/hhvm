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

#include <folly/Demangle.h>
#include <folly/String.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {

template<typename T>
std::string type_name() {
  auto const name = folly::demangle(typeid(T));
  return folly::format("AtomicGrowableVector<{}>", name).str();
}

}

template<typename T, typename D>
AtomicGrowableVector<T,D>::AtomicGrowableVector(size_t size, const D& def)
  : m_size(size)
  , m_next(nullptr)
  , m_default(def)
  , m_vals(new T[size])
{
  FTRACE(1, "{} {} constructing with size {}\n",
         type_name<T>(), this, size);

  for (size_t i = 0; i < size; ++i) {
    new (&m_vals[i]) T(def);
  }
}

template<typename T, typename D>
AtomicGrowableVector<T,D>::~AtomicGrowableVector() {
  FTRACE(1, "{} {} destructing\n",
         type_name<T>(), this);

  delete m_next.load(std::memory_order_relaxed);
}

template<typename T, typename D>
size_t AtomicGrowableVector<T,D>::size() const {
  auto const next = this->next();
  return m_size + (next ? next->size() : 0);
}

template<typename T, typename D>
void AtomicGrowableVector<T,D>::ensureSize(size_t size) {
  FTRACE(2, "{}::ensureSize({}), m_size = {}\n",
         type_name<T>(), size, m_size);

  if (m_size >= size) return;

  auto next = this->next();
  if (!next) {
    next = new AtomicGrowableVector(
      std::max(m_size * 2, size_t{1}),
      m_default
    );
    AtomicGrowableVector* expected = nullptr;
    FTRACE(2, "attempting to use {}...", next);
    if (!m_next.compare_exchange_strong(expected, next,
                                        std::memory_order_acq_rel)) {
      FTRACE(2, "lost race to {}\n", expected);
      delete next;
      next = expected;
    } else {
      FTRACE(2, "success\n");
    }
  }
  next->ensureSize(size - m_size);
}

template<typename T, typename D>
T& AtomicGrowableVector<T,D>::operator[](size_t i) {
  return i < m_size ? m_vals[i] : (*next())[i - m_size];
}

template<typename T, typename D>
const T& AtomicGrowableVector<T,D>::operator[](size_t i) const {
  return i < m_size ? m_vals[i] : (*next())[i - m_size];
}

template<typename T, typename D>
template<typename F>
void AtomicGrowableVector<T,D>::foreach(F fun) const {
  auto const size = m_size;

  for (auto i = 0; i < size; i++) fun(m_vals[i]);

  auto const next = this->next();
  if (next) next->foreach(fun);
}

template<typename T, typename D>
AtomicGrowableVector<T,D>* AtomicGrowableVector<T,D>::next() const {
  return m_next.load(std::memory_order_acquire);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AtomicVector<T>::AtomicVector(size_t size, const T& def)
  : AtomicGrowableVector<std::atomic<T>, T>(size, def)
{}

template<typename T>
T AtomicVector<T>::get(size_t i) const {
  return (*this)[i].load(std::memory_order_acquire);
}

template<typename T>
T AtomicVector<T>::exchange(size_t i, const T& val) {
  return (*this)[i].exchange(val, std::memory_order_acq_rel);
}

template<typename T>
template<typename F>
void AtomicVector<T>::foreach(F fun) const {
  AtomicGrowableVector<std::atomic<T>,T>::foreach(
    [&] (const std::atomic<T>& elm) {
      fun(elm.load(std::memory_order_acquire));
    });
}

template<typename T>
void UnsafeReinitEmptyAtomicVector(AtomicVector<T>& vec, size_t size) {
  always_assert(vec.size() == 0);

  auto const defVal = vec.m_default;
  vec.~AtomicVector<T>();
  new (&vec) AtomicVector<T>(size, defVal);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
AtomicLowPtrVector<T>::AtomicLowPtrVector(size_t size, const T* def)
  : AtomicGrowableVector<AtomicLowPtr<T,
                                      std::memory_order_acquire,
                                      std::memory_order_release>,
                         T*>(size, def)
{}

template<typename T>
T* AtomicLowPtrVector<T>::get(size_t i) const {
  return (*this)[i];
}

template<typename T>
void AtomicLowPtrVector<T>::set(size_t i, const T* val) {
  (*this)[i] = val;
}

template<typename T>
void UnsafeReinitEmptyAtomicLowPtrVector(AtomicLowPtrVector<T>& vec,
                                         size_t size) {
  always_assert(vec.size() == 0);

  auto const defVal = vec.m_default;
  vec.~AtomicLowPtrVector<T>();
  new (&vec) AtomicLowPtrVector<T>(size, defVal);
}

///////////////////////////////////////////////////////////////////////////////

}
