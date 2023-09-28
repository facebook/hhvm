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
#include "hphp/runtime/vm/jit/irgen-exit.h"

#include "hphp/runtime/vm/jit/irgen-inlining.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"

#include "hphp/runtime/vm/hhbc-codec.h"

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

bool branchesToItself(SrcKey sk) {
  if (sk.funcEntry()) return false;
  auto const op = sk.op();
  if (!instrIsControlFlow(op)) return false;
  if (isSwitch(op)) return false;
  auto const offset = sk.offset();
  auto const offsets = instrJumpTargets(sk.func()->entry(), offset);
  return std::find(offsets.begin(), offsets.end(), offset) != offsets.end();
}

/*
 * Helper to emit the appropriate service request to exit with a given target
 * bytecode offset.
 *
 * A ReqRetranslate is generated if we're still generating IR for the first
 * bytecode instruction in the region and the `target' offset matches the
 * current offset.  Note that this condition here always implies that the exit
 * corresponds to a guard failure (i.e., without advancing state), because
 * instructions that branch back to themselves are handled separately in
 * implMakeExit and never call into this function.
 *
 * In all other cases, a ReqBindJmp is generated.
 */
void exitRequest(IRGS& env, SrcKey target) {
  if (isInlining(env)) {
    sideExitFromInlined(env, target);
    return;
  }

  auto const irSP = spOffBCFromIRSP(env);
  auto const invSP = spOffBCFromStackBase(env);
  if (env.firstBcInst && target == curSrcKey(env)) {
    gen(
      env,
      ReqRetranslate,
      IRSPRelOffsetData { irSP },
      sp(env),
      fp(env)
    );
    return;
  }
  // FIXME: the following assert fails, because prepareInstruction() adds
  // illegal CheckTypes in the middle of translation exiting to the initial
  // SrcKey, which may be a func entry
  // assertx(!target.funcEntry());
  gen(
    env,
    ReqBindJmp,
    ReqBindJmpData { target, invSP, irSP, target.funcEntry() },
    sp(env),
    fp(env)
  );
}

Block* implMakeExit(IRGS& env, SrcKey targetSk) {
  auto const curSk = curSrcKey(env);

  // If the targetSk is to the same instruction, the instruction can also
  // branch back to itself (e.g. IterNext w/ offset=0), then we can't
  // distinguish whether the exit is due to a guard failure (i.e., no state
  // advanced) or an actual control-flow transfer (i.e., advancing state).
  // These are rare situations, and so we just punt to the interpreter.
  if (targetSk == curSk && branchesToItself(curSrcKey(env))) {
    PUNT(MakeExitAtBranchToItself);
  }

  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), exit);
  exitRequest(env, targetSk);
  return exit;
}

//////////////////////////////////////////////////////////////////////

}

Block* makeExit(IRGS& env) {
  return implMakeExit(env, curSrcKey(env));
}

Block* makeExit(IRGS& env, SrcKey targetSk) {
  return implMakeExit(env, targetSk);
}

Block* makeExitSlow(IRGS& env) {
  assertx(!curSrcKey(env).funcEntry());
  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), exit);
  interpOne(env);
  // If it changes the PC, InterpOneCF will get us to the new location.
  if (!opcodeChangesPC(curSrcKey(env).op())) {
    gen(env, Jmp, makeExit(env, nextSrcKey(env)));
  }
  return exit;
}

Block* makeExitSurprise(IRGS& env, SrcKey targetSk) {
  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), exit);
  gen(env, HandleRequestSurprise);
  exitRequest(env, targetSk);
  return exit;
}

Block* makeExitOpt(IRGS& env) {
  always_assert(!isInlining(env));
  auto const exit = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), exit);
  auto const data = IRSPRelOffsetData{spOffBCFromIRSP(env)};
  gen(env, ReqRetranslateOpt, data, sp(env), fp(env));
  return exit;
}

Block* makeUnreachable(IRGS& env, AssertReason reason) {
  auto const unreachable = defBlock(env, Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, curSrcKey(env)), unreachable);
  gen(env, Unreachable, reason);
  return unreachable;
}

//////////////////////////////////////////////////////////////////////

}
