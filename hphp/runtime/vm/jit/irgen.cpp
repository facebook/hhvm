/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/dce.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

Block* create_catch_block(IRGS& env) {
  auto const catchBlock = env.irb->unit().defBlock(Block::Hint::Unused);
  BlockPusher bp(*env.irb, env.irb->curMarker(), catchBlock);

  // Install the exception stack state so that spillStack and other stuff
  // behaves appropriately.  We've already asserted that sp(env) was the same
  // as the exception state version.
  auto const& exnState = env.irb->exceptionStackState();
  env.irb->evalStack() = exnState.evalStack;
  env.irb->setStackDeficit(exnState.stackDeficit);

  gen(env, BeginCatch);
  spillStack(env);
  gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    fp(env), sp(env));
  return catchBlock;
}

void check_catch_stack_state(IRGS& env, const IRInstruction* inst) {
  auto const& exnStack = env.irb->exceptionStackState();
  always_assert_flog(
    exnStack.sp == sp(env) &&
      exnStack.syncedSpLevel == env.irb->syncedSpLevel() &&
      exnStack.spOffset == env.irb->spOffset(),
    "catch block would have had a mismatched stack pointer:\n"
    "     inst: {}\n"
    "       sp: {}\n"
    " expected: {}\n"
    "  spLevel: {}\n"
    " expected: {}\n",
    inst->toString(),
    sp(env)->toString(),
    exnStack.sp->toString(),
    exnStack.syncedSpLevel.offset,
    env.irb->syncedSpLevel().offset
  );
}

//////////////////////////////////////////////////////////////////////

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
    FTRACE(1, "{}: creating {}catch block\n",
      inst->toString(),
      env.catchCreator ? "custom " : "");
    /*
     * If you hit this assertion, you're gen'ing an IR instruction that can
     * throw after gen'ing one that could write to the evaluation stack.  This
     * is usually not what HHBC opcodes do, and could be a bug.  See the
     * documentation for exceptionStackBoundary in the header for more
     * information.
     */
    check_catch_stack_state(env, inst);
    inst->setTaken(
      env.catchCreator
        ? env.catchCreator()
        : create_catch_block(env)
    );
  }

  if (inst->mayRaiseError()) {
    assertx(inst->taken() && inst->taken()->isCatch());
  }

  return env.irb->optimizeInst(inst, IRBuilder::CloneFlag::Yes, nullptr);
}
}

//////////////////////////////////////////////////////////////////////

void incTransCounter(IRGS& env) { gen(env, IncTransCounter); }

void incProfCounter(IRGS& env, TransID transId) {
  gen(env, IncProfCounter, TransIDData(transId));
}

void checkCold(IRGS& env, TransID transId) {
  gen(env, CheckCold, makeExitOpt(env, transId), TransIDData(transId));
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
   * If assertions are on, before we do anything, each region makes a call to a
   * C++ function that checks the state of everything.
   */
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const data = IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) };
    gen(env, DbgTraceCall, data, fp(env), sp(env));
  }

  /*
   * We automatically hoist a load of the context to the beginning of every
   * region.  The reason is that it's trivially CSEable, so we might as well
   * make it available everywhere.  If nothing uses it, it'll just be DCE'd.
   */
  ldCtx(env);
}

void prepareForSideExit(IRGS& env) { spillStack(env); }

void endRegion(IRGS& env) {
  auto const curSk  = curSrcKey(env);
  if (!instrAllowsFallThru(curSk.op())) return; // nothing to do here

  auto const nextSk = curSk.advanced(curUnit(env));
  endRegion(env, nextSk);
}

void endRegion(IRGS& env, SrcKey nextSk) {
  FTRACE(1, "------------------- endRegion ---------------------------\n");
  if (!fp(env)) {
    // The function already returned.  There's no reason to generate code to
    // try to go to the next part of it.
    return;
  }
  spillStack(env);
  auto const data = ReqBindJmpData {
    nextSk,
    invSPOff(env),
    offsetFromIRSP(env, BCSPOffset{0}),
    TransFlags{}
  };
  gen(env, ReqBindJmp, data, sp(env), fp(env));
}

void sealUnit(IRGS& env) {
  mandatoryDCE(env.unit);
}

Type predictedTypeFromLocal(const IRGS& env, uint32_t locId) {
  return env.irb->predictedLocalType(locId);
}

Type predictedTypeFromStack(const IRGS& env, BCSPOffset offset) {
  if (offset < env.irb->evalStack().size()) {
    return env.irb->evalStack().topPredictedType(offset.offset);
  }
  return env.irb->predictedStackType(offsetFromIRSP(env, offset));
}

// All accesses to the stack and locals in this function use DataTypeGeneric so
// this function should only be used for inspecting state; when the values are
// actually used they must be constrained further.
Type predictedTypeFromLocation(const IRGS& env, const Location& loc) {
  switch (loc.space) {
    case Location::Stack:
      return predictedTypeFromStack(env, loc.bcRelOffset);
    case Location::Local:
      return predictedTypeFromLocal(env, loc.offset);
    case Location::Litstr:
      return Type::cns(curUnit(env)->lookupLitstrId(loc.offset));
    case Location::Litint:
      return Type::cns(loc.offset);
    case Location::This:
      // Don't specialize $this for cloned closures which may have been re-bound
      if (curFunc(env)->hasForeignThis()) return TObj;
      if (auto const cls = curFunc(env)->cls()) {
        return Type::SubObj(cls);
      }
      return TObj;

    case Location::Iter:
    case Location::Invalid:
      break;
  }
  not_reached();
}

Type provenTypeFromLocal(const IRGS& env, uint32_t locId) {
  return env.irb->localType(locId, DataTypeGeneric);
}

Type provenTypeFromStack(const IRGS& env, BCSPOffset offset) {
  if (offset < env.irb->evalStack().size()) {
    return env.irb->evalStack().top(offset.offset)->type();
  }
  return env.irb->stackType(offsetFromIRSP(env, offset), DataTypeGeneric);
}

Type provenTypeFromLocation(const IRGS& env, const Location& loc) {
  switch (loc.space) {
  case Location::Stack:
    return provenTypeFromStack(env, loc.bcRelOffset);
  case Location::Local:
    return provenTypeFromLocal(env, loc.offset);
  case Location::Litstr:
    return Type::cns(curUnit(env)->lookupLitstrId(loc.offset));
  case Location::Litint:
    return Type::cns(loc.offset);
  case Location::This:
    // Don't specialize $this for cloned closures which may have been re-bound
    if (curFunc(env)->hasForeignThis()) return TObj;
    if (auto const cls = curFunc(env)->cls()) {
      return Type::SubObj(cls);
    }
    return TObj;

  case Location::Iter:
  case Location::Invalid:
    break;
  }
  not_reached();
}

void endBlock(IRGS& env, Offset next, bool nextIsMerge) {
  if (!fp(env)) {
    // If there's no fp, we've already executed a RetCtrl or similar, so
    // there's no reason to try to jump anywhere now.
    return;
  }
  // Don't emit the jump if it would be unreachable.  This avoids
  // unreachable blocks appearing to be reachable, which would cause
  // translateRegion to process them.
  if (auto const curBlock = env.irb->curBlock()) {
    if (!curBlock->empty() && curBlock->back().isTerminal()) return;
  }
  jmpImpl(env, next, nextIsMerge ? JmpFlagNextIsMerge : JmpFlagNone);
}

void prepareForNextHHBC(IRGS& env,
                        const NormalizedInstruction* ni,
                        SrcKey newSk,
                        bool lastBcInst) {
  FTRACE(1, "------------------- prepareForNextHHBC ------------------\n");
  env.currentNormalizedInstruction = ni;

  always_assert_flog(
    IMPLIES(isInlining(env), !env.lastBcInst),
    "Tried to end trace while inlining."
  );

  always_assert_flog(
    IMPLIES(isInlining(env), !env.firstBcInst),
    "Inlining while still at the first region instruction."
  );

  always_assert(env.bcStateStack.size() >= env.inlineLevel + 1);
  auto pops = env.bcStateStack.size() - 1 - env.inlineLevel;
  while (pops--) env.bcStateStack.pop_back();

  always_assert_flog(env.bcStateStack.back().func() == newSk.func(),
                     "Tried to update current SrcKey with a different func");

  env.bcStateStack.back().setOffset(newSk.offset());
  updateMarker(env);
  env.lastBcInst = lastBcInst;
  env.catchCreator = nullptr;
  env.irb->exceptionStackBoundary();
}

void finishHHBC(IRGS& env) {
  env.firstBcInst = false;
}

void prepareForHHBCMergePoint(IRGS& env) {
  spillStack(env);
}

FPInvOffset logicalStackDepth(const IRGS& env) {
  // Negate the offsetFromIRSP because it is an offset from the actual StkPtr
  // (so negative values go deeper on the stack), but this function deals with
  // logical stack depths (where more positive values are deeper).
  return env.irb->spOffset() - offsetFromIRSP(env, BCSPOffset{0});
}

Type publicTopType(const IRGS& env, BCSPOffset idx) {
  // It's logically const, because we're using DataTypeGeneric.
  return topType(const_cast<IRGS&>(env), idx, DataTypeGeneric);
}

//////////////////////////////////////////////////////////////////////

}}}
