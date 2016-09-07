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

#include "hphp/runtime/vm/jit/mcgen-emit.h"

#include "hphp/runtime/vm/jit/mcgen.h"

#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/func-guard.h"
#include "hphp/runtime/vm/jit/func-prologue.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/smashable-instr.h"
#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/tc-info.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/trans-rec.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vtune-jit.h"

#include "hphp/runtime/server/http-server.h"

#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(mcg);

namespace HPHP { namespace jit { namespace mcgen { namespace {
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

////////////////////////////////////////////////////////////////////////////////

/*
 * Attempt to emit code for the given IRUnit to `code'. Returns true on
 * success, false if codegen failed.
 */
bool mcGenUnit(TransEnv& env, CodeCache::View code, CGMeta& fixups) {
  auto const& unit = *env.unit;
  try {
    emitVunit(*env.vunit, unit, code, fixups,
              dumpTCAnnotation(*env.args.sk.func(), env.args.kind)
              ? &env.annotations
              : nullptr);
  } catch (const DataBlockFull& dbFull) {
    if (dbFull.name == "hot") {
      mcg->code().disableHot();
      return false;
    } else {
      always_assert_flog(0, "data block = {}\nmessage: {}\n",
                         dbFull.name, dbFull.what());
    }
  }

  auto const startSk = unit.context().srcKey();
  if (unit.context().kind == TransKind::Profile) {
    profData()->setProfiling(startSk.func()->getFuncId());
  }

  return true;
}

/*
 * If TC reuse is enabled, attempt to relocate the newly-emitted translation to
 * a hole reclaimed from dead code. Returns true if the translation was
 * relocated and false otherwise.
 */
void tryRelocateNewTranslation(SrcKey sk, TransLoc& loc,
                               CodeCache::View code, CGMeta& fixups) {
  if (!RuntimeOption::EvalEnableReusableTC) return;

  TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
             cs = loc.coldStart(), ce = loc.coldEnd(),
             fs = loc.frozenStart(), fe = loc.frozenEnd();

  auto const did_relocate = relocateNewTranslation(loc, code, fixups);

  if (did_relocate) {
    FTRACE_MOD(Trace::reusetc, 1,
               "Relocated translation for func {} (id = {})  @ sk({}) "
               "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
               "C[{}, {}] F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe, loc.mainStart(),
               loc.mainEnd(), loc.coldStart(), loc.coldEnd(),
               loc.frozenStart(), loc.frozenEnd());
  } else {
    FTRACE_MOD(Trace::reusetc, 1,
               "Created translation for func {} (id = {}) "
               " @ sk({}) at M[{}, {}], C[{}, {}], F[{}, {}]\n",
               sk.func()->fullName()->data(), sk.func()->getFuncId(),
               sk.offset(), ms, me, cs, ce, fs, fe);
  }
}

////////////////////////////////////////////////////////////////////////////////

/*
 * If live code relocation is enabled, record metadata for the current
 * translation.
 */
void recordRelocationMetaData(SrcKey sk, SrcRec& srcRec, const TransLoc& loc,
                              CGMeta& fixups) {
  if (!RuntimeOption::EvalPerfRelocate) return;

  recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                     loc.coldCodeStart(), loc.coldEnd(),
                     sk, -1,
                     srcRec.tailFallbackJumps(),
                     fixups);
}

Debug::TCRange rangeFrom(const CodeBlock& cb, const TCA addr, bool isAcold) {
  assertx(cb.contains(addr));
  return Debug::TCRange(addr, cb.frontier(), isAcold);
}

void recordGdbTranslation(SrcKey sk, const Func* srcFunc, const CodeBlock& cb,
                          const TCA start, bool exit, bool inPrologue) {
  if (start != cb.frontier()) {
    mcg->assertOwnsCodeLock();
    if (!RuntimeOption::EvalJitNoGdb) {
      mcg->debugInfo()->recordTracelet(
        rangeFrom(cb, start, &cb == &mcg->code().cold()),
        srcFunc,
        srcFunc->unit() ? srcFunc->unit()->at(sk.offset()) : nullptr,
        exit, inPrologue
      );
    }
    if (RuntimeOption::EvalPerfPidMap) {
      mcg->debugInfo()->recordPerfMap(
        rangeFrom(cb, start, &cb == &mcg->code().cold()),
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
    mcg->debugInfo()->recordBCInstr(Debug::TCRange(addr, end, cold), op);
  }
}

////////////////////////////////////////////////////////////////////////////////

std::atomic<bool> s_loggedJitMature{false};

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

// send stats about this translation to StructuredLog
void logTranslation(const TransEnv& env) {
  auto nanos = HPHP::Timer::GetThreadCPUTimeNanos() - env.unit->startNanos();
  StructuredLogEntry cols;
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
  // finish & log
  StructuredLog::log("hhvm_jit", cols);
}

////////////////////////////////////////////////////////////////////////////////

std::atomic<uint64_t> s_numTrans;

////////////////////////////////////////////////////////////////////////////////
}

bool canTranslate() {
  return s_numTrans.load(std::memory_order_relaxed) <
    RuntimeOption::EvalJitGlobalTranslationLimit;
}

void invalidateSrcKey(SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative || RuntimeOption::EvalJitPGO);
  /*
   * Reroute existing translations for SrcKey to an as-yet indeterminate
   * new one.
   */
  auto const sr = mcg->srcDB().find(sk);
  always_assert(sr);
  /*
   * Since previous translations aren't reachable from here, we know we
   * just created some garbage in the TC. We currently have no mechanism
   * to reclaim this.
   */
  FTRACE_MOD(Trace::reusetc, 1, "Replacing translations from func {} (id = {}) "
             "@ sk({}) in SrcRec addr={}\n",
             sk.func()->fullName()->data(), sk.func()->getFuncId(), sk.offset(),
             (void*)sr);
  Trace::Indent _i;
  sr->replaceOldTranslations();
}

void invalidateFuncProfSrcKeys(const Func* func) {
  assertx(profData());
  auto const funcId = func->getFuncId();
  auto codeLock = mcg->lockCode();
  for (auto tid : profData()->funcProfTransIDs(funcId)) {
    invalidateSrcKey(profData()->transRec(tid)->srcKey());
  }
}

TCA emitFuncPrologue(Func* func, int argc, TransKind kind) {
  if (s_numTrans.fetch_add(1, std::memory_order_relaxed) >=
      RuntimeOption::EvalJitGlobalTranslationLimit) {
    return nullptr;
  }

  const int nparams = func->numNonVariadicParams();
  const int paramIndex = argc <= nparams ? argc : nparams + 1;

  auto const funcBody = SrcKey{func, func->getEntryForNumArgs(argc), false};

  profileSetHotFuncAttr();
  auto codeLock = mcg->lockCode();
  auto code = mcg->code().view(kind);
  TCA mainOrig = code.main().frontier();
  CGMeta fixups;

  // If we're close to a cache line boundary, just burn some space to
  // try to keep the func and its body on fewer total lines.
  align(code.main(), &fixups, Alignment::CacheLineRoundUp, AlignContext::Dead);

  TransLocMaker maker(code);
  maker.markStart();

  // Careful: this isn't necessarily the real entry point. For funcIsMagic
  // prologues, this is just a possible prologue.
  TCA aStart = code.main().frontier();

  // Give the prologue a TransID if we have profiling data.
  auto const transID = [&]{
    if (kind == TransKind::ProfPrologue) {
      auto const profData = jit::profData();
      auto const id = profData->allocTransID();
      profData->addTransProfPrologue(id, funcBody, paramIndex);
      return id;
    }
    if (profData() && Translator::isTransDBEnabled()) {
      return profData()->allocTransID();
    }
    return kInvalidTransID;
  }();

  TCA start = genFuncPrologue(transID, kind, func, argc, code, fixups);

  auto loc = maker.markEnd();
  auto metaLock = mcg->lockMetadata();

  if (RuntimeOption::EvalEnableReusableTC) {
    TCA UNUSED ms = loc.mainStart(), me = loc.mainEnd(),
               cs = loc.coldStart(), ce = loc.coldEnd(),
               fs = loc.frozenStart(), fe = loc.frozenEnd(),
               oldStart = start;

    auto const did_relocate = relocateNewTranslation(loc, code, fixups, &start);

    if (did_relocate) {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Relocated prologue for func {} (id = {}) "
                 "from M[{}, {}], C[{}, {}], F[{}, {}] to M[{}, {}] "
                 "C[{}, {}] F[{}, {}] orig start @ {} new start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, loc.mainStart(), loc.mainEnd(),
                 loc.coldStart(), loc.coldEnd(), loc.frozenStart(),
                 loc.frozenEnd(), oldStart, start);
    } else {
      FTRACE_MOD(Trace::reusetc, 1,
                 "Created prologue for func {} (id = {}) at "
                 "M[{}, {}], C[{}, {}], F[{}, {}] start @ {}\n",
                 func->fullName()->data(), func->getFuncId(),
                 ms, me, cs, ce, fs, fe, oldStart);
    }

    recordFuncPrologue(func, loc);
    if (loc.mainStart() != aStart) {
      code.main().setFrontier(mainOrig); // we may have shifted to align
    }
  }
  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> incomingBranches;
    recordPerfRelocMap(loc.mainStart(), loc.mainEnd(),
                       loc.coldCodeStart(), loc.coldEnd(),
                       funcBody, paramIndex,
                       incomingBranches,
                       fixups);
  }
  fixups.process(nullptr);

  assertx(funcGuardMatches(funcGuardFromPrologue(start, func), func));
  assertx(mcg->code().isValidCodeAddress(start));

  TRACE(2, "funcPrologue mcg %p %s(%d) setting prologue %p\n",
        mcg, func->fullName()->data(), argc, start);
  func->setPrologue(paramIndex, start);

  assertx(kind == TransKind::LivePrologue ||
          kind == TransKind::ProfPrologue ||
          kind == TransKind::OptPrologue);

  auto tr = maker.rec(funcBody, transID, kind);
  mcg->tx().addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(func->unit(), func, tr);
  }


  recordGdbTranslation(funcBody, func, code.main(), loc.mainStart(),
                       false, true);
  recordBCInstr(OpFuncPrologue, loc.mainStart(), loc.mainEnd(), false);

  return start;
}

////////////////////////////////////////////////////////////////////////////////

TCA emitTranslation(TransEnv env) {
  Timer timer(Timer::mcg_finishTranslation);

  auto& args = env.args;
  auto const sk = args.sk;

  profileSetHotFuncAttr();

  auto codeLock = mcg->lockCode();
  auto code = mcg->code().view(args.kind);

  CGMeta fixups;
  mcgen::TransLocMaker maker{code};
  maker.markStart();

  // mcGenUnit emits machine code from vasm
  if (env.vunit && !mcGenUnit(env, code, fixups)) {
    // mcGenUnit() failed. Roll back, drop the unit and region, and clear
    // fixups.
    maker.rollback();
    maker.markStart();
    env.unit.reset();
    env.vunit.reset();
    args.region.reset();
    fixups.clear();
  }

  if (env.vunit) {
    if (s_numTrans.fetch_add(1, std::memory_order_relaxed) >=
        RuntimeOption::EvalJitGlobalTranslationLimit) {
      return nullptr;
    }
  } else {
    args.kind = TransKind::Interp;
    FTRACE(1, "emitting dispatchBB interp request for failed "
           "translation (spOff = {})\n", env.initSpOffset.offset);
    vwrap(code.main(), code.data(), fixups,
          [&] (Vout& v) { emitInterpReq(v, sk, env.initSpOffset); },
          CodeKind::Helper);
  }

  Timer metaTimer(Timer::mcg_finishTranslation_metadata);
  auto metaLock = mcg->lockMetadata();
  auto loc = maker.markEnd();

  tryRelocateNewTranslation(sk, loc, code, fixups);

  // Finally, record various metadata about the translation and add it to the
  // SrcRec.
  if (RuntimeOption::EvalProfileBC) {
    auto const vmUnit = sk.unit();
    TransBCMapping prev{};
    for (auto& cur : fixups.bcMap) {
      if (!cur.aStart) continue;
      if (prev.aStart) {
        if (prev.bcStart < vmUnit->bclen()) {
          recordBCInstr(uint32_t(vmUnit->getOp(prev.bcStart)),
                        prev.aStart, cur.aStart, false);
        }
      } else {
        recordBCInstr(OpTraceletGuard, loc.mainStart(), cur.aStart, false);
      }
      prev = cur;
    }
  }

  auto const srcRec = mcg->srcDB().find(args.sk);
  always_assert(srcRec);
  recordRelocationMetaData(sk, *srcRec, loc, fixups);
  recordGdbTranslation(sk, sk.func(), code.main(), loc.mainStart(),
                       false, false);
  recordGdbTranslation(sk, sk.func(), code.cold(), loc.coldStart(),
                       false, false);
  if (args.kind == TransKind::Profile) {
    always_assert(args.region);
    profData()->addTransProfile(env.transID, args.region, env.pconds);
  }

  auto tr = maker.rec(sk, env.transID, args.kind, args.region, fixups.bcMap,
                      std::move(env.annotations),
                      env.unit && cfgHasLoop(*env.unit));
  mcg->tx().addTranslation(tr);
  if (RuntimeOption::EvalJitUseVtuneAPI) {
    reportTraceletToVtune(sk.unit(), sk.func(), tr);
  }

  GrowableVector<IncomingBranch> inProgressTailBranches;
  fixups.process(&inProgressTailBranches);

  // SrcRec::newTranslation() makes this code reachable. Do this last;
  // otherwise there's some chance of hitting in the reader threads whose
  // metadata is not yet visible.
  TRACE(1, "newTranslation: %p  sk: (func %d, bcOff %d)\n",
        loc.mainStart(), sk.funcID(), sk.offset());
  srcRec->newTranslation(loc, inProgressTailBranches);

  TRACE(1, "mcg: %zd-byte tracelet\n", (ssize_t)loc.mainSize());
  if (Trace::moduleEnabledRelease(Trace::tcspace, 1)) {
    Trace::traceRelease("%s", getTCSpace().c_str());
  }

  reportJitMaturity(mcg->code());

  if (env.unit) {
    auto func = env.unit->context().func;
    auto enable = func ? func->shouldSampleJit() :
                  StructuredLog::coinflip(RuntimeOption::EvalJitSampleRate);
    if (enable) logTranslation(env);
  }

  return loc.mainStart();
}

}}}
