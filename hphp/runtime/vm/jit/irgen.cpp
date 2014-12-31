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
  gen(env, EndCatch, fp(env), sp(env));
  return catchBlock;
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
    FTRACE(1, "{}: asserting about catch block\n",
      inst->toString());
    /*
     * This assertion means you manually created a catch block, but didn't put
     * an exceptionStackBoundary after an update to the stack.  Even if you're
     * manually creating catches we require this just to make sure you're not
     * doing it on accident.
     */
    always_assert_flog(
      env.irb->exceptionStackState().sp == sp(env),
      "catch block would have had a mismatched stack pointer:\n"
      "     inst: {}\n"
      "       sp: {}\n"
      " expected: {}\n",
      inst->toString(),
      sp(env)->toString(),
      env.irb->exceptionStackState().sp->toString()
    );
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
    always_assert_flog(
      env.irb->exceptionStackState().sp == sp(env),
      "catch block would have had a mismatched stack pointer:\n"
      "     inst: {}\n"
      "       sp: {}\n"
      " expected: {}\n",
      inst->toString(),
      sp(env)->toString(),
      env.irb->exceptionStackState().sp->toString()
    );
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
  auto const stack = spillStack(env);
  gen(env, SyncABIRegs, fp(env), stack);
  gen(env, ReqBindJmp, ReqBindJmpData(nextPc));
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
        return top(env, i, DataTypeGeneric)->type();
      } else {
        auto stackVal =
          getStackValue(
            env.irb->sp(),
            i - env.irb->evalStack().size() + env.irb->stackDeficit()
          );
        if (stackVal.knownType.isBoxed() &&
            !(stackVal.predictedInner <= Type::Bottom)) {
          return ldRefReturn(stackVal.predictedInner.unbox()).box();
        }
        return stackVal.knownType;
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
        return Type::Obj.specialize(cls);
      }
      return Type::Obj;

    case Location::Iter:
    case Location::Invalid:
      break;
  }
  not_reached();
}

void endBlock(HTS& env, Offset next, bool nextIsMerge) {
  jmpImpl(env, next, nextIsMerge ? JmpFlagNextIsMerge : JmpFlagNone);
}

void prepareForNextHHBC(HTS& env,
                        const NormalizedInstruction* ni,
                        Offset newOff,
                        bool lastBcOff) {
  FTRACE(1, "------------------- prepareForNextHHBC ------------------\n");
  env.currentNormalizedInstruction = ni;

  always_assert_log(
    IMPLIES(isInlining(env), !env.lastBcOff),
    [&] {
      return folly::format("Tried to end trace while inlining:\n{}",
                           env.unit).str();
    }
  );

  env.bcStateStack.back().setOffset(newOff);
  updateMarker(env);
  env.lastBcOff = lastBcOff;
  env.catchCreator = nullptr;
  env.irb->prepareForNextHHBC();
}

size_t spOffset(const HTS& env) {
  return env.irb->spOffset() + env.irb->evalStack().size() -
    env.irb->stackDeficit();
}

Type publicTopType(const HTS& env, int32_t idx) {
  // It's logically const, because we're using DataTypeGeneric.
  return topType(const_cast<HTS&>(env), idx, DataTypeGeneric);
}

//////////////////////////////////////////////////////////////////////

}}}
