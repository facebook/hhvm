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

#ifndef incl_HPHP_JIT_MCGEN_TRANSLATE_H_
#define incl_HPHP_JIT_MCGEN_TRANSLATE_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit {

struct FPInvOffset;
struct ProfTransRec;
struct TransArgs;

namespace tc { struct TransMetaInfo; };

namespace mcgen {

/*
 * Create a new translation based on args.
 *
 * The SrcKey and kind of this translation must be specified in args. The
 * TransID and region may optionally be specified as well. If the kind of region
 * requested is TransOptimize a TransID must be specified.
 *
 * Should the region be absent, an appropriate region for the designated kind
 * will be selected.
 */
folly::Optional<tc::TransMetaInfo> translate(
  TransArgs args,
  FPInvOffset spOff,
  folly::Optional<CodeCache::View> optView = folly::none
);

/*
 * True iff retranslateAll is enabled and supported by the current server
 * execution mode.
 */
bool retranslateAllEnabled();
/*
 * Is still a pending call to retranslateAll()
 */
bool retranslateAllPending();
/*
 * Whether retranslateAll has been scheduled (and possibly already completed).
 */
bool retranslateAllScheduled();

/*
 * Whether retranslateAll has been scheduled but has not complete.
 */
bool pendingRetranslateAllScheduled();

/*
 * Is retranslateAll() finished.
 */
bool retranslateAllComplete();

/*
 * Get the ordering of optimized translations using hfsort on the call graph.
 * Also returns the average profile count across all profiled blocks.
 */
std::pair<std::vector<FuncId>, uint64_t> hfsortFuncs();

/*
 * Undo the effects of bumpTraceFunctions() (so that we don't trace
 * all the inlined functions we're because we're tracing the function
 * they're inlined into).
 */
CompactVector<Trace::BumpRelease> unbumpFunctions();

}}}

#endif
