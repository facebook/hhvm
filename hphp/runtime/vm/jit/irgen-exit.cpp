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
#include "hphp/runtime/vm/jit/irgen-exit.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

bool branchesToItself(SrcKey sk) {
  auto op = (Op*)sk.pc();
  if (!instrIsControlFlow(*op)) return false;
  if (isSwitch(*op)) return false;
  auto const branchOffsetPtr = instrJumpOffset(op);
  return branchOffsetPtr != nullptr && *branchOffsetPtr == 0;
}

/*
 * Helper to emit the appropriate service request to exit with a given target
 * bytecode offset.
 *
 * A ReqRetranslate is generated if we're still generating IR for the
 * first bytecode instruction in the region and the `target' offset
 * matches the current offset.  Note that this condition here always
 * implies that the exit corresponds to a guard failure (i.e., without
 * advancing state), because instructions that branch back to
 * themselves are handled separately in implMakeExit and never call
 * into this function.
 *
 * In all other cases, a ReqBindJmp is generated.
 *
 */
void exitRequest(IRGS& env, TransFlags flags, SrcKey target) {
  auto const curBcOff = bcOff(env);
  if (env.firstBcInst && target.offset() == curBcOff) {
    // The case where the instruction may branch back to itself is
    // handled in implMakeExit.
    assertx(!branchesToItself(curSrcKey(env)));
    gen(env, ReqRetranslate, ReqRetranslateData { flags }, sp(env));
  } else {
    gen(env, ReqBindJmp, ReqBindJmpData { target, flags }, sp(env));
  }
}

Block* implMakeExit(IRGS& env, TransFlags trflags, Offset targetBcOff) {
  auto const curBcOff = bcOff(env);
  if (targetBcOff == -1) targetBcOff = curBcOff;

  // If the targetBcOff is to the same instruction, and the
  // instruction can also branch back to itself (e.g. IterNext w/
  // offset=0), then we can't distinguish whether the exit is due to a
  // guard failure (i.e., no state advanced) or an actual control-flow
  // transfer (i.e., advancing state).  These are rare situations, and
  // so we just punt to the interpreter.
  if (targetBcOff == curBcOff && branchesToItself(curSrcKey(env))) {
    PUNT(MakeExitAtBranchToItself);
  }

  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, targetBcOff), exit);
  spillStack(env);
  auto const offset = offsetFromIRSP(env, BCSPOffset{0});
  gen(env, AdjustSP, IRSPOffsetData { offset }, sp(env));
  exitRequest(env, trflags, SrcKey{curSrcKey(env), targetBcOff});
  return exit;
}

//////////////////////////////////////////////////////////////////////

}

Block* makeExit(IRGS& env, Offset targetBcOff /* = -1 */) {
  return implMakeExit(env, TransFlags{}, targetBcOff);
}

Block* makeExit(IRGS& env, TransFlags flags) {
  return implMakeExit(env, flags, -1);
}

Block* makeExitSlow(IRGS& env) {
  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
  interpOne(env, *env.currentNormalizedInstruction);
  // If it changes the PC, InterpOneCF will get us to the new location.
  if (!opcodeChangesPC(env.currentNormalizedInstruction->op())) {
    gen(env, Jmp, makeExit(env, nextBcOff(env)));
  }
  return exit;
}

Block* makePseudoMainExit(IRGS& env) {
  return curFunc(env)->isPseudoMain()
    ? makeExit(env)
    : nullptr;
}

Block* makeExitOpt(IRGS& env, TransID transId) {
  assertx(!isInlining(env));
  auto const targetBcOff = bcOff(env);
  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher blockPusher(*env.irb, makeMarker(env, targetBcOff), exit);
  spillStack(env);
  auto const offset = offsetFromIRSP(env, BCSPOffset{0});
  gen(env, AdjustSP, IRSPOffsetData { offset }, sp(env));
  gen(env,
      ReqRetranslateOpt,
      ReqRetransOptData{transId, SrcKey{curSrcKey(env), targetBcOff}},
      sp(env));
  return exit;
}

//////////////////////////////////////////////////////////////////////

}}}
