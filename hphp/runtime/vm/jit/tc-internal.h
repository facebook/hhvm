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

#ifndef incl_HPHP_JIT_TC_INTERNAL_H_
#define incl_HPHP_JIT_TC_INTERNAL_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/mutex.h"

namespace HPHP { namespace jit { namespace tc {

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
    loc.setMainStart(cache.main().frontier());
    loc.setColdStart(cache.cold().frontier());
    loc.setFrozenStart(cache.frozen().frontier());
    dataStart = cache.data().frontier();

    cache.cold().dword(0);
    if (&cache.cold() != &cache.frozen()) cache.frozen().dword(0);
  }

  /*
   * If loc contains a valid location, reset the frontiers of all code and data
   * blocks to the positions recorded by the last call to markStart().
   */
  void rollback() {
    if (loc.empty()) return;

    cache.main().setFrontier(loc.mainStart());
    cache.cold().setFrontier(loc.coldStart());
    cache.frozen().setFrontier(loc.frozenStart());
    cache.data().setFrontier(dataStart);
  }

  /*
   * Record the end of a translation, storing the size of cold and frozen,
   * returns a TransLoc representing the translation.
   */
  TransLoc markEnd() {
    uint32_t* coldSize   = (uint32_t*)loc.coldStart();
    uint32_t* frozenSize = (uint32_t*)loc.frozenStart();
    *coldSize   = cache  .cold().frontier() - loc.coldStart();
    *frozenSize = cache.frozen().frontier() - loc.frozenStart();
    loc.setMainSize(cache.main().frontier() - loc.mainStart());

    return loc;
  }

  /*
   * Create a TransRec for the translation, markEnd() should be called prior to
   * calling rec().
   */
  TransRec rec(
      SrcKey                      sk,
      TransID                     transID,
      TransKind                   kind,
      RegionDescPtr               region  = RegionDescPtr(),
      std::vector<TransBCMapping> bcmap   = std::vector<TransBCMapping>(),
      Annotations&&               annot   = Annotations(),
      bool                        hasLoop = false) const {
    return TransRec(sk, transID, kind,
                    loc.mainStart(), loc.mainSize(),
                    loc.coldCodeStart(), loc.coldCodeSize(),
                    loc.frozenCodeStart(), loc.frozenCodeSize(),
                    std::move(region), std::move(bcmap),
                    std::move(annot), hasLoop);
  }

private:
  CodeCache::View cache;
  TransLoc loc;
  Address dataStart;
};

/*
 * Structure representing the various parts of the TC available to the JIT. The
 * code lock must be acquired before attempting to write into code.
 */
CodeCache& code();

/*
 * Structure containing records for each SrcKey present in the TC. The metadata
 * lock must be acquired before reading or writing the srcdb.
 */
SrcDB& srcDB();

/*
 * Acquire a lock on this object's code cache.
 *
 * Must be held even if the current thread owns the global write lease.
 */
std::unique_lock<SimpleMutex> lockCode();

/*
 * Acquire a lock on this object's metadata tables.
 *
 * Must be held even if the current thread owns the global write lease.
 */
std::unique_lock<SimpleMutex> lockMetadata();

/*
 * Atomically bumps the translation counter and returns true iff emitting a new
 * translation will not exceed the global translation limit.
 */
bool newTranslation();

}}}

#endif
