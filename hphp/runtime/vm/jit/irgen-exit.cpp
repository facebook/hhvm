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

/*
 * Helper to emit the appropriate service request to exit with a given target
 * bytecode offset.
 *
 * Note that if we're inlining, then targetBcOff is in the inlined func, while
 * context.initBcOffset is in the outer func, so bindJmp will always work (and
 * there's no guarantee that there is an anchor translation, so we must not use
 * ReqRetranslate).  In the non-inlined situation, however, we want to use
 * ReqRetranslate if and only if the target bc offset is the initial bc offset.
 */
void exitRequest(HTS& env, TransFlags flags, Offset targetBcOff) {
  auto const curBcOff = bcOff(env);
  if (!isInlining(env) &&
      curBcOff == env.context.initBcOffset &&
      targetBcOff == curBcOff) {
    gen(env, ReqRetranslate, ReqRetranslateData { flags });
  } else {
    gen(env, ReqBindJmp, ReqBindJmpData { targetBcOff, flags });
  }
}

Block* implMakeExit(HTS& env, TransFlags trflags, Offset targetBcOff) {
  if (targetBcOff == -1) targetBcOff = bcOff(env);
  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, targetBcOff), exit);
  auto const stack = spillStack(env);
  gen(env, SyncABIRegs, fp(env), stack);
  exitRequest(env, trflags, targetBcOff);
  return exit;
}

//////////////////////////////////////////////////////////////////////

}

Block* makeExit(HTS& env, Offset targetBcOff /* = -1 */) {
  return implMakeExit(env, TransFlags{}, targetBcOff);
}

Block* makeExit(HTS& env, TransFlags flags) {
  return implMakeExit(env, flags, -1);
}

Block* makeExitSlow(HTS& env) {
  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), exit);
  interpOne(env, *env.currentNormalizedInstruction);
  // If it changes the PC, InterpOneCF will get us to the new location.
  if (!opcodeChangesPC(env.currentNormalizedInstruction->op())) {
    gen(env, Jmp, makeExit(env, nextBcOff(env)));
  }
  return exit;
}

Block* makePseudoMainExit(HTS& env) {
  return curFunc(env)->isPseudoMain()
    ? makeExit(env)
    : nullptr;
}

Block* makeExitOpt(HTS& env, TransID transId) {
  assert(!isInlining(env));
  auto const targetBcOff = bcOff(env);
  auto const exit = env.unit.defBlock(Block::Hint::Unlikely);
  BlockPusher blockPusher(*env.irb, makeMarker(env, targetBcOff), exit);
  auto const stack = spillStack(env);
  gen(env, SyncABIRegs, fp(env), stack);
  gen(env, ReqRetranslateOpt, ReqRetransOptData(transId, targetBcOff));
  return exit;
}

//////////////////////////////////////////////////////////////////////

}}}
