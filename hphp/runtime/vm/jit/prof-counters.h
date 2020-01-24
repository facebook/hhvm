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

#ifndef incl_HPHP_PROF_COUNTERS_H_
#define incl_HPHP_PROF_COUNTERS_H_

#include <cstdint>
#include <vector>

#include "hphp/util/assertions.h"

/**
 * A simple class of a growable number of profiling counters with fixed
 * addresses, suitable for being incremented from the TC.
 */
template<typename T>
struct ProfCounters {
  explicit ProfCounters(T initVal)
    : m_initVal(initVal)
  {}

  ProfCounters(ProfCounters&&) = default;
  ProfCounters& operator=(ProfCounters&&) = default;

  ProfCounters(const ProfCounters&) = delete;
  ProfCounters& operator=(const ProfCounters&) = delete;

  T get(uint32_t id) const {
    assertx(kCountersPerChunk % 11);
    return id / kCountersPerChunk >= m_chunks.size()
      ? m_initVal
      : m_chunks[id / kCountersPerChunk][(id * 11) % kCountersPerChunk];
  }

  T* getAddr(uint32_t id) {
    // allocate a new chunk of counters if necessary
    while (id >= m_chunks.size() * kCountersPerChunk) {
      uint32_t size = sizeof(T) * kCountersPerChunk;
      auto const chunk = new T[size];
      std::fill_n(chunk, kCountersPerChunk, m_initVal);
      m_chunks.emplace_back(chunk);
    }
    assertx(id / kCountersPerChunk < m_chunks.size());
    assertx(kCountersPerChunk % 11);
    return &(m_chunks[id / kCountersPerChunk][(id * 11) % kCountersPerChunk]);
  }

  T getDefault() const { return m_initVal; }

  void resetAllCounters(T value) {
    // We need to set m_initVal so that method transCounter() works, and also so
    // that newly created counters start with `value'.
    m_initVal = value;
    // Reset all counters already created.
    for (auto& chunk : m_chunks) {
      std::fill_n(chunk.get(), kCountersPerChunk, value);
    }
  }

  void clear() {
    for (auto& chunk : m_chunks) {
      chunk.reset();
    }
    m_chunks.clear();
  }

private:
  static const uint32_t kCountersPerChunk = 2 * 1024 * 1024 / sizeof(T);

  T m_initVal;
  std::vector<std::unique_ptr<T[]>> m_chunks;
};

#endif
