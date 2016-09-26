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


#include "hphp/runtime/vm/jit/tc-record.h"

#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/tc-relocate.h"

#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/vm/debug/debug.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/trans-db.h"
#include "hphp/runtime/vm/jit/trans-rec.h"

#include "hphp/util/data-block.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace tc {

void recordPerfRelocMap(
    TCA start, TCA end,
    TCA coldStart, TCA coldEnd,
    SrcKey sk, int argNum,
    const GrowableVector<IncomingBranch> &incomingBranchesIn,
    CGMeta& fixups) {
  auto info = perfRelocMapInfo(start, end,
                               coldStart, coldEnd,
                               sk, argNum,
                               incomingBranchesIn,
                               fixups);
  Debug::DebugInfo::Get()->recordRelocMap(start, end, info);
}

void recordRelocationMetaData(SrcKey sk, SrcRec& srcRec, const TransLoc& loc,
                              CGMeta& fixups) {
  if (!RuntimeOption::EvalPerfRelocate) return;

  recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                     loc.coldCodeStart(), loc.coldEnd(),
                     sk, -1,
                     srcRec.tailFallbackJumps(),
                     fixups);
}

static Debug::TCRange rangeFrom(const CodeBlock& cb, const TCA addr,
                                bool isAcold) {
  assertx(cb.contains(addr));
  return Debug::TCRange(addr, cb.frontier(), isAcold);
}

void recordGdbTranslation(SrcKey sk, const Func* srcFunc, const CodeBlock& cb,
                          const TCA start, bool exit, bool inPrologue) {
  if (start != cb.frontier()) {
    assertOwnsCodeLock();
    if (!RuntimeOption::EvalJitNoGdb) {
      Debug::DebugInfo::Get()->recordTracelet(
        rangeFrom(cb, start, &cb == &code().cold()),
        srcFunc,
        srcFunc->unit() ? srcFunc->unit()->at(sk.offset()) : nullptr,
        exit, inPrologue
      );
    }
    if (RuntimeOption::EvalPerfPidMap) {
      Debug::DebugInfo::Get()->recordPerfMap(
        rangeFrom(cb, start, &cb == &code().cold()),
        sk,
        srcFunc,
        exit,
        inPrologue
      );
    }
  }
}

void recordBCInstr(uint32_t op, const TCA addr, const TCA end, bool cold) {
  if (addr != end) {
    Debug::DebugInfo::Get()->recordBCInstr(Debug::TCRange(addr, end, cold), op);
  }
}

////////////////////////////////////////////////////////////////////////////////

static std::atomic<bool> s_loggedJitMature{false};

/*
 * If the jit maturity counter is enabled, update it with the current amount of
 * emitted code.
 */
void reportJitMaturity(const CodeCache& code) {
  auto static jitMaturityCounter = ServiceData::createCounter("jit.maturity");

  // Optimized translations are faster than profiling translations, which are
  // faster than the interpreter.  But when optimized translations are
  // generated, some profiling translations will become dead.  We assume the
  // incremental value of an optimized translation over the corresponding
  // profiling translations is comparable to the incremental value of a
  // profiling translation of similar size; thus we don't have to apply
  // different weights to code in different regions.
  auto const codeSize =
    code.hot().used() + code.main().used() + code.prof().used();
  if (jitMaturityCounter) {
    // EvalJitMatureSize is supposed to to be set to approximately 20% of the
    // code that will give us full performance, so recover the "fully mature"
    // size with some math.
    auto const fullSize = RuntimeOption::EvalJitMatureSize * 5;
    auto const after = codeSize >= fullSize ? 100
                                            : (codeSize * 100 / fullSize);
    auto const before = jitMaturityCounter->getValue();
    if (after > before) jitMaturityCounter->setValue(after);
  }

  if (!s_loggedJitMature.load(std::memory_order_relaxed) &&
      StructuredLog::enabled() &&
      codeSize >= RuntimeOption::EvalJitMatureSize &&
      !s_loggedJitMature.exchange(true, std::memory_order_relaxed)) {
    StructuredLogEntry cols;
    cols.setInt("jit_mature_sec", time(nullptr) - HttpServer::StartTime);
    StructuredLog::log("hhvm_warmup", cols);
  }
}

void logTranslation(const TransEnv& env) {
  auto nanos = HPHP::Timer::GetThreadCPUTimeNanos() - env.unit->startNanos();
  auto& cols = *env.unit->logEntry();
  auto& context = env.unit->context();
  auto kind = show(context.kind);
  cols.setStr("trans_kind", !debug ? kind : kind + "_debug");
  if (context.func) {
    cols.setStr("func", context.func->fullName()->data());
  }
  cols.setInt("jit_sample_rate", RuntimeOption::EvalJitSampleRate);
  // timing info
  cols.setInt("jit_micros", nanos / 1000);
  // hhir stats
  cols.setInt("max_tmps", env.unit->numTmps());
  cols.setInt("max_blocks", env.unit->numBlocks());
  cols.setInt("max_insts", env.unit->numInsts());
  auto hhir_blocks = rpoSortCfg(*env.unit);
  cols.setInt("num_blocks", hhir_blocks.size());
  size_t num_insts = 0;
  for (auto b : hhir_blocks) num_insts += b->instrs().size();
  cols.setInt("num_insts", num_insts);
  // vasm stats
  if (env.vunit) {
    cols.setInt("max_vreg", env.vunit->next_vr);
    cols.setInt("max_vblocks", env.vunit->blocks.size());
    cols.setInt("max_vcalls", env.vunit->vcallArgs.size());
    size_t max_vinstr = 0;
    for (auto& blk : env.vunit->blocks) max_vinstr += blk.code.size();
    cols.setInt("max_vinstr", max_vinstr);
    cols.setInt("num_vconst", env.vunit->constToReg.size());
    auto vblocks = sortBlocks(*env.vunit);
    size_t num_vinstr[kNumAreas] = {0, 0, 0};
    size_t num_vblocks[kNumAreas] = {0, 0, 0};
    for (auto b : vblocks) {
      const auto& block = env.vunit->blocks[b];
      num_vinstr[(int)block.area_idx] += block.code.size();
      num_vblocks[(int)block.area_idx]++;
    }
    cols.setInt("num_vinstr_main", num_vinstr[(int)AreaIndex::Main]);
    cols.setInt("num_vinstr_cold", num_vinstr[(int)AreaIndex::Cold]);
    cols.setInt("num_vinstr_frozen", num_vinstr[(int)AreaIndex::Frozen]);
    cols.setInt("num_vblocks_main", num_vblocks[(int)AreaIndex::Main]);
    cols.setInt("num_vblocks_cold", num_vblocks[(int)AreaIndex::Cold]);
    cols.setInt("num_vblocks_frozen", num_vblocks[(int)AreaIndex::Frozen]);
  }
  // finish & log
  StructuredLog::log("hhvm_jit", cols);
}

}}}
