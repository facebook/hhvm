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

#ifndef incl_HPHP_JIT_TC_RECORD_H_
#define incl_HPHP_JIT_TC_RECORD_H_

#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/data-block.h"
#include "hphp/util/growable-vector.h"
#include "hphp/util/service-data.h"

namespace HPHP {

struct Func;
struct SrcKey;

namespace jit {

struct CGMeta;
struct CodeCache;
struct ProfTransRec;
struct TransEnv;

namespace tc {

/*
 * If live code relocation is enabled, record metadata for the current
 * translation.
 */
void recordRelocationMetaData(SrcKey sk, SrcRec& srcRec, const TransLoc& loc,
                              CGMeta& fixups);

/*
 * Record translation for gdb debugging of the tc.
 */
void recordGdbTranslation(SrcKey sk, const Func* srcFunc, const CodeBlock& cb,
                          const TCA start, const TCA end, bool exit,
                          bool inPrologue);

/*
 * Record BC instruction ranges in the tc for the perf map.
 */
void recordBCInstr(uint32_t op, const TCA addr, const TCA end, bool cold);

/*
 * Report jit warmup statistics to scribe via StructuredLog.
 */
void reportJitMaturity(const CodeCache& code);

/*
 * Get a code size counter for the named code block ("main", "cold", etc.)
 */
ServiceData::ExportedTimeSeries* getCodeSizeCounter(const std::string& name);
/*
 * Log statistics about a translation to scribe via StructuredLog.
 */
void logTranslation(const TransEnv& env, const TransRange& range);

/*
 * Log inlined frames in unit via StructuredLog.
 */
void logFrames(const Vunit& unit);

/*
 * Record smashed calls in the TC that may need to be re-smashed in the event
 * that a prologue is reused-- additionally any information in ProfData will
 * need to be erased before a translation with a call to a Proflogue is
 * reclaimed.
 */
void recordFuncCaller(const Func* func, TCA toSmash, bool immutable,
                      ProfTransRec* rec);

/*
 * When a function is treadmilled its bytecode may no longer be available,
 * keep a table of associated SrcRecs to be reclaimed as it will be impossible
 * to walk the bytecode stream to search the SrcDB.
 */
void recordFuncSrcRec(const Func* func, SrcRec* rec);

/*
 * Record a prologue associated with a function so that it may be reclaimed
 * when the function is treadmilled.
 */
void recordFuncPrologue(const Func* func, TransLoc loc);

/*
 * This function is like a request-agnostic version of
 * server_warmup_status().
 * Three conditions necessary for the jit to qualify as "warmed-up":
 * 1. Has HHVM evaluated enough requests?
 * 2. Has retranslateAll happened yet?
 * 3. Has code size plateaued? Is the rate of new code emission flat?
 * If the jit is warmed up, this function returns the empty string.
 */
std::string warmupStatusString();

}}}

#endif
