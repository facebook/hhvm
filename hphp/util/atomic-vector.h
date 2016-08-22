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

#ifndef incl_HPHP_ATOMIC_VECTOR_H
#define incl_HPHP_ATOMIC_VECTOR_H

#include <algorithm>
#include <atomic>
#include <functional>

#include <folly/String.h>

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
struct AtomicVector {
  AtomicVector(size_t size, const Value& def);
  ~AtomicVector();

  void ensureSize(size_t size);
  Value exchange(size_t i, const Value& val);
  std::atomic<Value>& operator[](size_t i);
  const std::atomic<Value>& operator[](size_t i) const;

  size_t size() const;
  Value get(size_t i) const;
  template <typename F> void foreach(F fun) const;

  Value defaultValue() const;

 private:
  static std::string typeName();

  const size_t m_size;
  std::atomic<AtomicVector*> m_next;
  const Value m_default;
  std::unique_ptr<std::atomic<Value>[]> m_vals;
  TRACE_SET_MOD(atomicvector);
};

/*
 * A number of AtomicVectors have sizes that we'd like to control with runtime
 * options, but these options are parsed after the relevant AtomicVectors are
 * constructed. AtomicVectorInit is used to register a list of these
 * AtomicVectors, allowing us to reconstruct them once the options have been
 * parsed.
 */
struct AtomicVectorInit {
  template<typename Vec>
  AtomicVectorInit(Vec& vector, const uint64_t& size);

  /*
   * Run any reinitialization code registered by AtomicVectorInit structs.
   *
   * This does not preserve the existing contents of any AtomicVectors that are
   * reconstructed.
   */
  static void runAll();

private:
  static std::vector<std::function<void()>> s_funcs;
};

}

#include "hphp/util/atomic-vector-inl.h"

#endif
