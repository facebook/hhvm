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

#ifndef incl_HPHP_ATOMIC_VECTOR_H
#define incl_HPHP_ATOMIC_VECTOR_H

#include <atomic>

#include "folly/String.h"

#include "hphp/util/trace.h"

namespace HPHP {

/*
 * AtomicVector is a simple vector intended for use by many concurrent readers
 * and writers. The size given to the constructor determines how many elements
 * the AtomicVector will initially hold, and each one will be initialized to
 * the given default value. Elements may be retrieved and exchanged with any
 * valid index by many readers and writers concurrently, though the operations
 * may be very slow if std::atomic<Value>::is_lock_free() == false.
 *
 * The only way to increase the size of an AtomicVector is with the ensureSize
 * method. It does not reallocate the internal storage to grow; it allocates a
 * new AtomicVector and chains to that for increased capacity. This means that
 * if the initial size is too low, reading and modifying elements at high
 * indexes will be increasingly slower as the chain of AtomicVectors is walked
 * to find the right element.
 *
 * An AtomicVector cannot shrink, and will only reclaim memory when destructed.
 */

template<typename Value>
class AtomicVector {
 public:
  AtomicVector(size_t size, const Value& def);
  ~AtomicVector();

  void ensureSize(size_t size);
  Value exchange(size_t i, const Value& val);
  bool compare_exchange(size_t i, Value expect, const Value& val);

  Value get(size_t i) const;

 private:
  static std::string typeName();

  const size_t m_size;
  std::atomic<AtomicVector*> m_next;
  const Value m_default;
  std::unique_ptr<std::atomic<Value>[]> m_vals;
  TRACE_SET_MOD(atomicvector);
};

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
  assert(size > 0 && "size must be nonzero");

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
    next = new AtomicVector(m_size * 2, m_default);
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

  assert(m_next);
  return m_next.load(std::memory_order_acquire)->exchange(i - m_size, val);
}

template<typename Value>
bool AtomicVector<Value>::compare_exchange(size_t i,
                                           Value expect,
                                           const Value& val) {
  FTRACE(3, "{}::compare_exchange({}, {}), m_size = {}\n",
         typeName(), i, val, m_size);
  if (i < m_size) {
    auto oldVal = m_vals[i].compare_exchange_strong(expect, val,
        std::memory_order_release, std::memory_order_acquire);

    FTRACE(3, "{}::compare_exchange returning {}\n", typeName(), oldVal);
    return oldVal;
  }

  assert(m_next);
  return m_next.load(std::memory_order_acquire)->
                     compare_exchange(i - m_size, expect,  val);
}

template<typename Value>
Value AtomicVector<Value>::get(size_t i) const {
  FTRACE(4, "{}::get({}), m_size = {}\n", typeName(), i, m_size);
  if (i < m_size) {
    auto val = m_vals[i].load(std::memory_order_acquire);
    FTRACE(5, "{}::get returning {}\n", typeName(), m_vals[i].load());
    return val;
  }

  assert(m_next);
  return m_next.load(std::memory_order_acquire)->get(i - m_size);
}

}

#endif
