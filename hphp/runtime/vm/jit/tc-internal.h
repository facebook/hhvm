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

#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/mutex.h"

namespace HPHP::jit::tc {

/*
 * Convenience class for creating TransLocs and TransRecs for new translations.
 *
 * Records the beginning and end of a translation and stores the size of the
 * cold and frozen regions in the first 4 bytes of their respective regions.
 */
struct TransLocMaker {
  explicit TransLocMaker(CodeCache::View c) : cache(c) {}

  /*
   * Record the start of a translation, and reserve space at the top of cold
   * and frozen (if they aren't the same) to record sizes.
   */
  void markStart() {
    mainStart = cache.main().frontier();
    coldStart = cache.cold().frontier();
    frozenStart = cache.frozen().frontier();
    dataStart = cache.data().frontier();
  }

  TcaRange dataRange() const {
    return TcaRange{dataStart, cache.data().frontier()};
  }

  bool empty() const {
    return !mainStart && !coldStart && !frozenStart && !dataStart;
  }

  /*
   * If loc contains a valid location, reset the frontiers of all code and data
   * blocks to the positions recorded by the last call to markStart().
   * Return the range being rolled back without writing to it.
   */
  TransRange rollback() {
    if (empty()) {
      return TransRange {
        {nullptr, nullptr},
        {nullptr, nullptr},
        {nullptr, nullptr},
        {nullptr, nullptr},
      };
    }

    // During a rollback we must be careful for cases where we failed to
    // reserve the dword in cold and frozen that is intended to store the size.
    // In those cases we must ensure the ranges are still valid (their end is
    // after their beginning).
    auto coldEnd = cache.cold().frontier();
    auto frozenEnd = cache.frozen().frontier();
    auto const range = TransRange{
      {mainStart, cache.main().frontier()},
      {coldStart, coldEnd},
      {frozenStart, frozenEnd},
      {dataStart, cache.data().frontier()}
    };
    cache.main().setFrontier(mainStart);
    cache.cold().setFrontier(coldStart);
    cache.frozen().setFrontier(frozenStart);
    cache.data().setFrontier(dataStart);
    return range;
  }

  TransRange range() const {
    assertx(!empty() && mainEnd && coldEnd && frozenEnd && dataEnd);
    return TransRange{
      {mainStart, mainEnd},
      {coldStart, coldEnd},
      {frozenStart, frozenEnd},
      {dataStart, dataEnd}
    };
  }

  /*
   * Record the end of a translation, storing the size of cold and frozen,
   * returns a TransRange representing the translation.
   */
  TransRange markEnd() {
    mainEnd = cache.main().frontier();
    coldEnd = cache.cold().frontier();
    frozenEnd = cache.frozen().frontier();
    dataEnd = cache.data().frontier();
    return range();
  }

  /*
   * Create a TransRec for the translation, markEnd() should be called prior to
   * calling rec().
   */
  TransRec rec(
      SrcKey                      sk,
      TransID                     transID,
      TransKind                   kind,
      Annotations&&               annot   = Annotations(),
      RegionDescPtr               region  = RegionDescPtr(),
      std::vector<TransBCMapping> bcmap   = std::vector<TransBCMapping>(),
      bool                        hasLoop = false) const {
    auto r = range();
    return TransRec(sk, transID, kind,
                    r.main.begin(), r.main.size(),
                    r.cold.begin(), r.cold.size(),
                    r.frozen.begin(), r.frozen.size(),
                    std::move(annot), std::move(region),
                    std::move(bcmap), hasLoop);
  }

  CodeCache::View view() const { return cache; }

private:
  CodeCache::View cache;
  TCA mainStart{nullptr};
  TCA mainEnd{nullptr};
  TCA coldStart{nullptr};
  TCA coldEnd{nullptr};
  TCA frozenStart{nullptr};
  TCA frozenEnd{nullptr};
  TCA dataStart{nullptr};
  TCA dataEnd{nullptr};
};

////////////////////////////////////////////////////////////////////////////////

/*
 * There are a number of different locks that protect data owned by or related
 * to the tc, described here in ascending Rank order (see hphp/util/rank.h).
 *
 * - Global write lease (GetWriteLease()). The write lease has a number of
 *   heuristics that are used to ensure we lay out Live translations in a good
 *   order. When Eval.JitConcurrently == 0, holding the write lease gives the
 *   owning thread exclusive permission to write to the Translation Cache (TC)
 *   and all associated metadata tables. When Eval.JitConcurrently > 0, the
 *   global write lease is only used to influence code layout and provides no
 *   protection against data races. In the latter case, the remaining locks are
 *   used to protect the integrity of the TC and its metadata:
 *
 * - Func-specific write leases (Func* argument to LeaseHolder). These locks
 *   give the owning thread exclusive permission to write to the SrcRec for
 *   translations of the corresponding Func, modify its prologue table, and
 *   read/write any ProfTransRecs for translations in the Func. Note that the
 *   Func-specific lease *must* be held to modify any Func-specific metadata
 *   when Eval.JitConcurrently > 0, even if the current thread holds the global
 *   write lease.
 *
 * - tc::lockCode() gives the owning thread exclusive permission to modify the
 *   contents and frontiers of all code/data blocks in m_code.
 *
 * - tc::lockMetadata() gives the owning thread exclusive permission to modify
 *   the metadata tables owned by the tc (FixupMap::recordFixup(), or
 *   tc::srcDB(), etc).
 */

/*
 * Acquire a lock on this object's code cache (the lock is deferred if
 * lock is false).
 *
 * Must be held even if the current thread owns the global write lease.
 */
std::unique_lock<SimpleMutex> lockCode(bool lock = true);

/*
 * Acquire a lock on this object's metadata tables (the lock is
 * deferred if lock is false).
 *
 * Must be held even if the current thread owns the global write lease.
 */
std::unique_lock<SimpleMutex> lockMetadata(bool lock = true);

struct CodeMetaLock {
  explicit CodeMetaLock(bool f);
  void lock();
  void unlock();
private:
  std::unique_lock<SimpleMutex> m_code;
  std::unique_lock<SimpleMutex> m_meta;
};

/*
 * Atomically bumps the translation counter and returns true iff emitting a new
 * translation will not exceed the global translation limit.
 */
bool newTranslation();

/*
 * Structure representing the various parts of the TC available to the JIT. The
 * code lock must be acquired before attempting to write into code.
 */
ALWAYS_INLINE CodeCache& code() {
  extern CodeCache* g_code;
  assertx(g_code);
  return *g_code;
}

/*
 * Structure containing records for each SrcKey present in the TC. The metadata
 * lock must be acquired before reading or writing the srcdb.
 */
ALWAYS_INLINE SrcDB& srcDB() {
  extern SrcDB g_srcDB;
  return g_srcDB;
}


/*
 * Called when we detect a change in the workload during JIT warmup, which makes
 * the JIT less effective. This affects the reported JIT maturity.
 */
void recordWorkloadChange();

/*
 * Get the current maturity of the JIT. It is an integer between 0 and 100 that
 * never decreases. When TC is filled up, the value is 100 or 99 (it will be
 * stuck at 99 when recordWorkloadChange() happened during warmup).
 */
int getJitMaturity();

/*
 * Whether JIT maturity is 100, or 99 due to workload changes.
 */
extern int g_maxJitMaturity;
ALWAYS_INLINE bool isMature() {
  return getJitMaturity() >= g_maxJitMaturity;
}

/*
 * Initialize the TC recycling mechanism. Does nothing if Eval.EnableReusableTC
 * is false.
 */
void recycleInit();

/*
 * Teardown TC recycling mechanism. Does nothing if Eval.EnableReusableTC is
 * false.
 */
void recycleStop();

}
