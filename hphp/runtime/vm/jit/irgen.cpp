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

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

Block* create_catch_block(HTS& env) {
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
  gen(env, EndCatch, StackOffset { offsetFromSP(env, 0) }, fp(env), sp(env));
  return catchBlock;
}

void check_catch_stack_state(HTS& env, const IRInstruction* inst) {
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
    exnStack.syncedSpLevel,
    env.irb->syncedSpLevel()
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
SSATmp* genInstruction(HTS& env, IRInstruction* inst) {
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
    assert(inst->taken() && inst->taken()->isCatch());
  }

  return env.irb->optimizeInst(inst, IRBuilder::CloneFlag::Yes, nullptr,
    folly::none);
}
}

//////////////////////////////////////////////////////////////////////

void incTransCounter(HTS& env) { gen(env, IncTransCounter); }

void incProfCounter(HTS& env, TransID transId) {
  gen(env, IncProfCounter, TransIDData(transId));
}

void checkCold(HTS& env, TransID transId) {
  gen(env, CheckCold, makeExitOpt(env, transId), TransIDData(transId));
}

void ringbuffer(HTS& env, Trace::RingBufferType t, SrcKey sk, int level) {
  if (!Trace::moduleEnabledRelease(Trace::ringbuffer, level)) return;
  gen(env, RBTrace, RBTraceData(t, sk));
}

void prepareEntry(HTS& env) {
  /*
   * We automatically hoist a load of the context to the beginning of every
   * region.  The reason is that it's trivially CSEable, so we might as well
   * make it available everywhere.  If nothing uses it, it'll just be DCE'd.
   */
  ldCtx(env);
}

void prepareForSideExit(HTS& env) { spillStack(env); }

void endRegion(HTS& env) {
  auto const nextSk = curSrcKey(env).advanced(curUnit(env));
  endRegion(env, nextSk.offset());
}

void endRegion(HTS& env, Offset nextPc) {
  FTRACE(1, "------------------- endRegion ---------------------------\n");
  if (!fp(env)) {
    // The function already returned.  There's no reason to generate code to
    // try to go to the next part of it.
    return;
  }
  if (nextPc >= curFunc(env)->past()) {
    // We have fallen off the end of the func's bytecodes. This happens
    // when the function's bytecodes end with an unconditional
    // backwards jump so that nextPc is out of bounds and causes an
    // assertion failure in unit.cpp. The common case for this comes
    // from the default value funclets, which are placed after the end
    // of the function, with an unconditional branch back to the start
    // of the function. So you should see this in any function with
    // default params.
    return;
  }
  prepareForNextHHBC(env, nullptr, nextPc, true);
  spillStack(env);
  auto dest = SrcKey{curSrcKey(env), nextPc};
  gen(env, AdjustSP, StackOffset { offsetFromSP(env, 0) }, sp(env));
  gen(env, ReqBindJmp, ReqBindJmpData { dest }, sp(env));
}

// All accesses to the stack and locals in this function use DataTypeGeneric so
// this function should only be used for inspecting state; when the values are
// actually used they must be constrained further.
Type predictedTypeFromLocation(HTS& env, const Location& loc) {
  switch (loc.space) {
    case Location::Stack: {
      auto i = loc.offset;
      assert(i >= 0);
      if (i < env.irb->evalStack().size()) {
        return topType(env, i, DataTypeGeneric);
      } else {
        auto stackTy = env.irb->stackType(
          offsetFromSP(env, i),
          DataTypeGeneric
        );
        if (stackTy.isBoxed()) {
          return env.irb->stackInnerTypePrediction(offsetFromSP(env, i)).box();
        }
        return stackTy;
      }
    } break;
    case Location::Local:
      return env.irb->predictedLocalType(loc.offset);
    case Location::Litstr:
      return Type::cns(curUnit(env)->lookupLitstrId(loc.offset));
    case Location::Litint:
      return Type::cns(loc.offset);
    case Location::This:
      // Don't specialize $this for cloned closures which may have been re-bound
      if (curFunc(env)->hasForeignThis()) return Type::Obj;
      if (auto const cls = curFunc(env)->cls()) {
        return Type::SubObj(cls);
      }
      return Type::Obj;

    case Location::Iter:
    case Location::Invalid:
      break;
  }
  not_reached();
}

void endBlock(HTS& env, Offset next, bool nextIsMerge) {
  if (!fp(env)) {
    // If there's no fp, we've already executed a RetCtrl or similar, so
    // there's no reason to try to jump anywhere now.
    return;
  }
  jmpImpl(env, next, nextIsMerge ? JmpFlagNextIsMerge : JmpFlagNone);
}

void prepareForNextHHBC(HTS& env,
                        const NormalizedInstruction* ni,
                        Offset newOff,
                        bool lastBcOff) {
  FTRACE(1, "------------------- prepareForNextHHBC ------------------\n");
  env.currentNormalizedInstruction = ni;

  always_assert_flog(
    IMPLIES(isInlining(env), !env.lastBcOff),
    "Tried to end trace while inlining."
  );

  env.bcStateStack.back().setOffset(newOff);
  updateMarker(env);
  env.lastBcOff = lastBcOff;
  env.catchCreator = nullptr;
  env.irb->prepareForNextHHBC();
}

void prepareForHHBCMergePoint(HTS& env) {
  spillStack(env);

  // For EvalHHIRBytecodeControlFlow we need to make sure the spOffset
  // is the same on all incoming edges going to a merge point.  This
  // would "just happen" if we didn't still have instructions that
  // redefine StkPtrs, but calls still need to do that for now, so we
  // need this hack.
  auto spOff = StackOffset{-(env.irb->syncedSpLevel() - env.irb->spOffset())};
  gen(env, AdjustSP, spOff, sp(env));
}

size_t logicalStackDepth(const HTS& env) {
  // Negate the offsetFromSP because it is an offset from the actual StkPtr (so
  // negative values go deeper on the stack), but this function deals with
  // logical stack depths (where more positive values are deeper).
  return env.irb->spOffset() + -offsetFromSP(env, 0);
}

Type publicTopType(const HTS& env, int32_t idx) {
  // It's logically const, because we're using DataTypeGeneric.
  return topType(const_cast<HTS&>(env), idx, DataTypeGeneric);
}

//////////////////////////////////////////////////////////////////////

}}}
