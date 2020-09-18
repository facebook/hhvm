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
#include <memory>

#include "hphp/util/low-ptr.h"
#include "hphp/util/trace.h"

namespace HPHP {

/*
 * A simple atomically-growable vector.
 *
 * The size given to the constructor determines how many elements the
 * AtomicGrowableVector will initially hold, and each one will be initialized
 * to the given default value.  References to elements are guaranteed to remain
 * valid through the lifetime of the vector itself.
 *
 * The only way to increase the size of an AtomicGrowableVector is with the
 * ensureSize() method.  It does not reallocate the internal storage to grow;
 * instead it allocates a new AtomicGrowableVector and chains to that for
 * increased capacity.  This means that if the initial size is too low, reading
 * and modifying elements at high indexes will be increasingly slower as the
 * chain of AtomicGrowableVectors is walked to find the right element.
 *
 * An AtomicGrowableVector cannot shrink, and will only reclaim memory when
 * destructed.
 */
template<typename T, typename D = T>
struct AtomicGrowableVector {
  AtomicGrowableVector(size_t size, const D& def);
  ~AtomicGrowableVector();

  /*
   * Get the size of the vector.
   *
   * Note that unlike a more traditional vector like std::vector<>, users of
   * AtomicGrowableVector have little control over its size.  The vector's size
   * may be much larger than the highest index accessed, but all untouched
   * elements are guaranteed to be set to the chosen default value.
   */
  size_t size() const;

  /*
   * Grow the vector such that it can fit at least `size` elements.
   */
  void ensureSize(size_t size);

  /*
   * Element accessors.
   *
   * The vector grows atomically, but element accesses are not protected.
   */
  T& operator[](size_t i);
  const T& operator[](size_t i) const;

  /*
   * Call `fun` on each element.
   */
  template <typename F> void foreach(F fun) const;

protected:
  const size_t m_size;
  std::atomic<AtomicGrowableVector*> m_next;
  const D m_default;
  std::unique_ptr<T[]> m_vals;

private:
  AtomicGrowableVector<T,D>* next() const;

  TRACE_SET_MOD(atomicvector);
};

/*
 * AtomicVector is a atomically-growable vector intended for use by many
 * concurrent readers and writers.
 *
 * This is a convenience extension of AtomicGrowableVector with atomic
 * elements.  Elements may be retrieved and exchanged with any valid index by
 * many readers and writers concurrently, though the operations may be very
 * slow if std::atomic<T>::is_lock_free() == false.
 */
template<typename T>
struct AtomicVector : AtomicGrowableVector<std::atomic<T>, T> {
  AtomicVector(size_t size, const T& def);

  /*
   * Convenience accessors for std::atomic<T> elements.
   *
   * All reads/writes in this API use acquire/release std::memory_order.
   */
  T get(size_t i) const;
  T exchange(size_t i, const T& val);

  /*
   * Call `fun` on each atomically-loaded element (including default-
   * initialized ones conjured via ensureSize().
   */
  template <typename F> void foreach(F fun) const;

  /*
   * Reconstruct a currently empty vector with new initial size. Thread-unsafe.
   *
   * A number of AtomicVectors have sizes that we'd like to control with runtime
   * options, but these options are parsed after the relevant AtomicVectors are
   * constructed. This method  allows us to reconstruct them once the options
   * have been parsed.
   */
  template<typename V>
  friend void UnsafeReinitEmptyAtomicVector(AtomicVector<V>& vec, size_t size);
};

template<typename T>
struct AtomicLowPtrVector
  : AtomicGrowableVector<AtomicLowPtr<T,
                                      std::memory_order_acquire,
                                      std::memory_order_release>,
                         T*>
{
  AtomicLowPtrVector(size_t size, const T* def);

  /*
   * Accessors
   */
  T* get(size_t i) const;
  void set(size_t i, const T* val);

  /*
   * Reconstruct a currently empty vector with new initial size. Thread-unsafe.
   *
   * A number of AtomicVectors have sizes that we'd like to control with runtime
   * options, but these options are parsed after the relevant AtomicVectors are
   * constructed. This method  allows us to reconstruct them once the options
   * have been parsed.
   */
  template<typename V>
  friend void UnsafeReinitEmptyAtomicLowPtrVector(AtomicLowPtrVector<V>& vec,
                                                  size_t size);
};

}

#include "hphp/util/atomic-vector-inl.h"

