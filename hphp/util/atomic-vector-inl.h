/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

template<typename Value>
std::string AtomicVector<Value>::typeName() {
  auto name = folly::demangle(typeid(Value));
  return folly::format("AtomicVector<{}>", name).str();
}

template<typename Value>
AtomicVector<Value>::AtomicVector(size_t size, const Value& def)
  : m_size(size)
  , m_next(nullptr)
  , m_default(def)
  , m_vals(new std::atomic<Value>[size])
{
  FTRACE(1, "{} {} constructing with size {}, default {}\n",
         typeName(), this, size, def);

  for (size_t i = 0; i < size; ++i) {
    new (&m_vals[i]) std::atomic<Value>(def);
  }
}

template<typename Value>
AtomicVector<Value>::~AtomicVector() {
  FTRACE(1, "{} {} destructing\n",
         typeName(), this);

  delete m_next.load(std::memory_order_relaxed);
}

template<typename Value>
void AtomicVector<Value>::ensureSize(size_t size) {
  FTRACE(2, "{}::ensureSize({}), m_size = {}\n",
         typeName(), size, m_size);
  if (m_size >= size) return;

  auto next = m_next.load(std::memory_order_acquire);
  if (!next) {
    next = new AtomicVector(std::max(m_size * 2, size_t{1}), m_default);
    AtomicVector* expected = nullptr;
    FTRACE(2, "Attempting to use {}...", next);
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

template<typename Value>
Value AtomicVector<Value>::exchange(size_t i, const Value& val) {
  FTRACE(3, "{}::exchange({}, {}), m_size = {}\n",
         typeName(), i, val, m_size);
  if (i < m_size) {
    auto oldVal = m_vals[i].exchange(val, std::memory_order_acq_rel);
    FTRACE(3, "{}::exchange returning {}\n", typeName(), oldVal);
    return oldVal;
  }

  assert(m_next.load());
  return m_next.load(std::memory_order_acquire)->exchange(i - m_size, val);
}

template<typename Value>
std::atomic<Value>& AtomicVector<Value>::operator[](size_t i) {
  if (i < m_size) return m_vals[i];

  return (*m_next.load(std::memory_order_acquire))[i - m_size];
}

template<typename Value>
const std::atomic<Value>& AtomicVector<Value>::operator[](size_t i) const {
  return const_cast<AtomicVector&>(*this)[i];
}

template<typename Value>
Value AtomicVector<Value>::get(size_t i) const {
  FTRACE(4, "{}::get({}), m_size = {}\n", typeName(), i, m_size);
  if (i < m_size) {
    auto val = m_vals[i].load(std::memory_order_acquire);
    FTRACE(5, "{}::get returning {}\n", typeName(), m_vals[i].load());
    return val;
  }

  assert(m_next.load());
  return m_next.load(std::memory_order_acquire)->get(i - m_size);
}

template<typename Value>
size_t AtomicVector<Value>::size() const {
  auto next = m_next.load(std::memory_order_acquire);
  return m_size + (next ? next->size() : 0);
}

template<typename Value>
template<typename F>
void AtomicVector<Value>::foreach(F fun) const {
  auto size = m_size;

  for (auto i = 0; i < size; i++) {
    fun(m_vals[i].load(std::memory_order_acquire));
  }
  auto next = m_next.load(std::memory_order_acquire);
  if (next) next->foreach(fun);
}

template<typename Value>
Value AtomicVector<Value>::defaultValue() const {
  return m_default;
}

template<typename Value>
void UnsafeReinitEmptyAtomicVector(AtomicVector<Value>& vec, size_t size) {
  always_assert(vec.size() == 0);

  auto const defVal = vec.defaultValue();
  vec.~AtomicVector<Value>();
  new (&vec) AtomicVector<Value>(size, defVal);
}

}
