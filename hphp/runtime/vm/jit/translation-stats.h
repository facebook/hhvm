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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/prof-counters.h"
#include "hphp/runtime/vm/srckey.h"

#include <folly/SharedMutex.h>

namespace HPHP::jit {

struct TransStats;

/*
 * Perform any process-global initialization required for TransStats.
 */
void processInitTransStats();
TransStats* globalTransStats();

void logGlobalTransStats();

struct TransStatsRec {
  TransStatsRec(TransKind m_kind, SrcKey sk);

  TransKind kind() const { return m_kind; }
  SrcKey srcKey() const { return m_sk; }
private:
  TransKind m_kind;
  SrcKey m_sk;
};

/**
 * TransStats encapsulates the translation stats kept by the runtime.
 */
struct TransStats {
  TransStats();


  TransStats(const TransStats&) = delete;
  TransStats& operator=(const TransStats&) = delete;

  TransStatsRec* transStats(TransID id) {
    std::shared_lock lock{m_transLock};
    return m_transStatsRecs.at(id).get();
  }

  size_t numTransStats() {
    std::shared_lock lock{m_transLock};
    return m_transStatsRecs.size();
  }

  void logCallCounts() const;

  /**
   * Return the TransID for `sk` for `kind` if one exists, kInvalidTransID otherwise.
   */
  TransID getTransID(TransKind kind, SrcKey sk) const;

  /**
   * Allocates a new TransID for `sk` and returns it. If two translations
   * for the same SrcKey call this function concurrently, only one TransID
   * will be generated.
   */
  TransID initTransStats(TransKind, SrcKey);

  /*
   * The absolute number of times that a translation executed.
   */
  int64_t transStatsCounter(TransID id) const {
    assertx(id < m_transStatsRecs.size());
    auto const counter = m_counters.get(id);
    auto const initVal = m_counters.getDefault();
    assert_flog(initVal >= counter,
                "transStatsCounter({}) = {}, initVal = {}\n",
                id, counter, initVal);
    return initVal - counter;
  }

  int64_t* transStatsCounterAddr(TransID id) {
    // getAddr() can grow the slab list, so grab a write lock.
    std::unique_lock lock{m_transLock};
    return m_counters.getAddr(id);
  }

  /*
   * m_transLock is used to protect m_transStatsRecs, and m_counters, which are
   * modified in the process of creating a new translation. It must be held
   * even by threads with the global write lease, to synchronize with threads
   * that don't have the write lease.
  */
  mutable folly::SharedMutex m_transLock;
  std::vector<std::unique_ptr<TransStatsRec>> m_transStatsRecs;
  ProfCounters<int64_t> m_counters;
  /*
   * Map from SrcKey.toAtomicInt() to TransID.
   */
  folly::AtomicHashMap<uint64_t, TransID> m_srcKeyToTransID;

};

} // namespace HPHP::jit
