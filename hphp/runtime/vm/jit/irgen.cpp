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
#include "hphp/runtime/vm/jit/irgen.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/prof-data.h"

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void check_catch_stack_state(IRGS& env, const IRInstruction* inst) {
  always_assert_flog(
    !env.irb->fs().stackModified(),
    "catch block used after writing to stack\n"
    "     inst: {}\n",
    inst->toString()
  );
}

//////////////////////////////////////////////////////////////////////

}

uint64_t curProfCount(const IRGS& env) {
  auto const& tids = env.profTransIDs;
  assertx(tids.empty() || (env.region != nullptr && profData() != nullptr));
  if (tids.empty()) return env.profFactor;
  uint64_t totalProfCount = 0;
  for (auto tid : tids) {
    if (env.region->hasBlock(tid)) {
      totalProfCount += env.region->blockProfCount(tid);
    }
  }
  return env.profFactor * totalProfCount;
}

uint64_t calleeProfCount(const IRGS& env, const RegionDesc& calleeRegion) {
  auto const tid = calleeRegion.entry()->id();
  if (tid == kInvalidTransID) return 0;
  return env.profFactor * calleeRegion.blockProfCount(tid);
}

//////////////////////////////////////////////////////////////////////

void eagerVMSync(IRGS& env, IRSPRelOffset spOff) {
  auto const bcSP = gen(env, LoadBCSP, IRSPRelOffsetData { spOff }, sp(env));
  gen(env, StVMFP, fixupFP(env));
  gen(env, StVMSP, bcSP);
  gen(env, StVMPC, cns(env, uintptr_t(env.irb->curMarker().fixupSk().pc())));
  genStVMReturnAddr(env);
  gen(env, StVMRegState, cns(env, eagerlyCleanState()));
}

//////////////////////////////////////////////////////////////////////

/*
 * This is called when each instruction is generated during initial IR
 * creation.
 *
 * This function inspects the instruction and prepares it for potentially being
 * inserted to the instruction stream.  It then calls IRBuilder optimizeInst,
 * which may or may not insert it depending on a variety of factors.
 */
namespace detail {
SSATmp* genInstruction(IRGS& env, IRInstruction* inst) {
  if (env.irb->inUnreachableState()) {
    FTRACE(1, "Skipping unreachable instruction: {}\n", inst->toString());
    return inst->hasDst() ? cns(env, TBottom) : nullptr;
  }

  /*
   * Now that we've verified we're in a reachable state (to rule out any Bottom
   * types in scope), verify the types are correct.
   */
  assertx(checkOperandTypes(inst));

  if (inst->mayRaiseError() && inst->taken()) {
    FTRACE(1, "{}: asserting about catch block\n", inst->toString());
    /*
     * This assertion means you manually created a catch block, but didn't put
     * an exceptionStackBoundary after an update to the stack.  Even if you're
     * manually creating catches we require this just to make sure you're not
     * doing it on accident.
     */
    check_catch_stack_state(env, inst);
  }

  if (inst->mayRaiseError() && !inst->taken()) {
    FTRACE(1, "{}: creating catch block\n", inst->toString());
    /*
     * If you hit this assertion, you're gen'ing an IR instruction that can
     * throw after gen'ing one that could write to the evaluation stack.  This
     * is usually not what HHBC opcodes do, and could be a bug.  See the
     * documentation for exceptionStackBoundary in the header for more
     * information.
     */
    check_catch_stack_state(env, inst);
    auto const offsetToAdjustSPForCall = [&]() -> int32_t {
      if (inst->is(Call)) {
        auto const extra = inst->extra<Call>();
        return extra->numInputs() + kNumActRecCells + extra->numOut;
      }
      if (inst->is(CallFuncEntry)) {
        auto const callee = inst->extra<CallFuncEntry>()->target.func();
        return
          callee->numFuncEntryInputs() +
          kNumActRecCells +
          callee->numInOutParams();
      }
      return 0;
    }();
    auto const catchMode = [&]() {
      if (inst->is(ReturnHook,
                   SuspendHookAwaitEF,
                   SuspendHookAwaitEG,
                   SuspendHookCreateCont,
                   CheckSurpriseFlagsEnter)) {
        return EndCatchData::CatchMode::LocalsDecRefd;
      }
      return EndCatchData::CatchMode::UnwindOnly;
    }();
    inst->setTaken(create_catch_block(env, []{}, catchMode,
                                      offsetToAdjustSPForCall));
  }

  if (inst->mayRaiseError()) {
    assertx(inst->taken() && inst->taken()->isCatch());
  }

  auto const outputInst = [&] {
    return env.irb->optimizeInst(inst, IRBuilder::CloneFlag::Yes, nullptr);
  };

  /*
   * In debug mode, emit eager syncs with high frequency to ensure that
   * store and load elimination optimizations are correct. The correctness of
   * the VMRegs is verified in VMRegAnchor.
   */
  auto const shouldStressEagerSync =
    debug &&
    inst->maySyncVMRegsWithSources() &&
    !inst->marker().prologue() &&
    !inst->marker().sk().funcEntry() &&
    !inst->is(Call, CallFuncEntry, ContEnter) &&
    (env.unit.numInsts() % 3 == 0);

  if (shouldStressEagerSync) {
    auto const spOff = offsetFromIRSP(env, inst->marker().bcSPOff());
    eagerVMSync(env, spOff);
    auto const res = outputInst();
    gen(env, StVMRegState, cns(env, VMRegState::DIRTY));
    return res;
  }

  return outputInst();
}
} // detail

//////////////////////////////////////////////////////////////////////

void incProfCounter(IRGS& env, TransID transId) {
  gen(env, IncProfCounter, TransIDData(transId));
}

void checkCold(IRGS& env, TransID transId) {
  gen(env, CheckCold, makeExitOpt(env), TransIDData(transId));
}

void checkCoverage(IRGS& env) {
  assertx(!isInlining(env));
  auto const handle = RDSHandleData { curUnit(env)->coverageDataHandle() };
  ifElse(
    env,
    [&] (Block* next) { gen(env, CheckRDSInitialized, next, handle); },
    [&] {
      // Exit to the interpreter at the current SrcKey location.
      hint(env, Block::Hint::Unlikely);
      auto const irSP = spOffBCFromIRSP(env);
      auto const invSP = spOffBCFromStackBase(env);
      auto const rbjData = ReqBindJmpData {
        curSrcKey(env), invSP, irSP, curSrcKey(env).funcEntry() /* popFrame */
      };
      gen(env, ReqInterpBBNoTranslate, rbjData, sp(env), fp(env));
    }
  );
}

void checkDebuggerIntr(IRGS& env, SrcKey sk) {
  assertx(!RuntimeOption::RepoAuthoritative);
  assertx(RuntimeOption::EnableVSDebugger);
  assertx(RuntimeOption::EvalEmitDebuggerIntrCheck);
  assertx(curFunc(env) == sk.func());
  if (sk.func()->isBuiltin()) return;
  assertx(!isInlining(env));
  auto const handle = RDSHandleData { curFunc(env)->debuggerIntrSetHandle() };
  ifElse(
    env,
    [&] (Block* next) { gen(env, CheckRDSInitialized, next, handle); },
    [&] {
      // Exit to the interpreter at the given SrcKey location.
      hint(env, Block::Hint::Unlikely);
      auto const irSP = spOffBCFromIRSP(env);
      auto const invSP = spOffBCFromStackBase(env);
      auto const rbjData = ReqBindJmpData {
        sk, invSP, irSP, sk.funcEntry() /* popFrame */
      };
      gen(env, ReqInterpBBNoTranslate, rbjData, sp(env), fp(env));
    }
  );
}

void ringbufferEntry(IRGS& env, Trace::RingBufferType t, SrcKey sk, int level) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, level)) return;
  gen(env, RBTraceEntry, RBEntryData(t, sk));
}

void ringbufferMsg(IRGS& env,
                   Trace::RingBufferType t,
                   const StringData* msg,
                   int level) {
  if (!Trace::moduleEnabled(Trace::ringbuffer, level)) return;
  gen(env, RBTraceMsg, RBMsgData(t, msg));
}

void prepareEntry(IRGS& env) {
  /*
   * Trivial DV Func Entries don't have the frame setup yet, so we
   * can't validate the frame or load the context from it.
   */
  if (curSrcKey(env).trivialDVFuncEntry()) return;

  /*
   * If assertions are on, before we do anything, each region makes a call to a
   * C++ function that checks the state of everything.
   */
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const data = IRSPRelOffsetData { spOffBCFromIRSP(env) };
    gen(env, DbgTraceCall, data, fp(env), sp(env));
  }

  /*
   * We automatically hoist a load of the context to the beginning of every
   * region.  The reason is that it's trivially CSEable, so we might as well
   * make it available everywhere.  If nothing uses it, it'll just be DCE'd.
   *
   * Don't do it in function entries, as we already have SSATmp of ctx received
   * via register. Furthermore, ldCtx is not yet operational in case of closure
   * bodies, as they were not unpacked yet.
   */
  if (!curSrcKey(env).funcEntry()) ldCtx(env);
}

void endRegion(IRGS& env) {
  auto const curSk  = curSrcKey(env);
  if (!curSk.funcEntry() && !instrAllowsFallThru(curSk.op())) {
    return; // nothing to do here
  }

  auto const nextSk = curSk.advanced(curFunc(env));
  endRegion(env, nextSk);
}

void endRegion(IRGS& env, SrcKey nextSk) {
  FTRACE(1, "------------------- endRegion ---------------------------\n");
  if (env.irb->inUnreachableState()) {
    // This location is unreachable.  There's no reason to generate code to
    // try to go to the next part of it.
    return;
  }

  spillInlinedFrames(env);

  assertx(!nextSk.funcEntry());
  auto const data = ReqBindJmpData {
    nextSk,
    spOffBCFromStackBase(env),
    spOffBCFromIRSP(env),
    false /* popFrame */
  };
  gen(env, ReqBindJmp, data, sp(env), fp(env));
}

void sealUnit(IRGS& env) {
  mandatoryDCE(env.unit);
}

///////////////////////////////////////////////////////////////////////////////

Type publicTopType(const IRGS& env, BCSPRelOffset idx) {
  // It's logically const, because we're using DataTypeGeneric.
  return topType(const_cast<IRGS&>(env), idx, DataTypeGeneric);
}

Type provenType(const IRGS& env, const Location& loc) {
  auto& fs = env.irb->fs();

  switch (loc.tag()) {
    case LTag::Stack:
      return fs.stack(offsetFromIRSP(env, loc.stackIdx())).type;
    case LTag::Local:
      return fs.local(loc.localId()).type;
    case LTag::MBase:
      return fs.mbase().type;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

void endBlock(IRGS& env, SrcKey nextSk) {
  // If there's no fp, we've already executed a RetCtrl or similar, so there's
  // no reason to try to jump anywhere now. We can probably drop the fp check
  // here and rely on the unreachable check alone.
  if (!fp(env) || env.irb->inUnreachableState()) return;
  jmpImpl(env, nextSk);
}

void prepareForNextHHBC(IRGS& env, SrcKey newSk) {
  FTRACE(1, "------------------- prepareForNextHHBC ------------------\n");
  always_assert_flog(
    IMPLIES(isInlining(env), !env.firstBcInst),
    "Inlining while still at the first region instruction."
  );

  always_assert(env.inlineState.bcStateStack.size() == inlineDepth(env));
  always_assert_flog(curFunc(env) == newSk.func(),
                     "Tried to update current SrcKey with a different func");

  FTRACE(1, "Next instruction: {}: {}\n", newSk.printableOffset(),
         NormalizedInstruction(newSk, curUnit(env)).toString());

  env.bcState = newSk;
  updateMarker(env);
  env.irb->exceptionStackBoundary();
  env.irb->resetCurIROff();
}

void finishHHBC(IRGS& env) {
  env.firstBcInst = false;
}

//////////////////////////////////////////////////////////////////////

}
