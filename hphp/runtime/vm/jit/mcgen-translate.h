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

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/trace.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit {

struct ProfTransRec;
struct TransArgs;

struct ProfDataSerializer;
struct ProfDataDeserializer;

namespace tc { struct TransMetaInfo; };

namespace mcgen {

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
 * Undo the effects of bumpTraceFunctions() (so that we don't trace
 * all the inlined functions we're because we're tracing the function
 * they're inlined into).
 */
CompactVector<Trace::BumpRelease> unbumpFunctions();

}}}

