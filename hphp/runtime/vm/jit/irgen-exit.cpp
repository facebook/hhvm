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

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

//////////////////////////////////////////////////////////////////////

Block* makeExit(HTS& env, Offset targetBcOff /* = -1 */) {
  auto spillValues = peekSpillValues(env);
  return makeExit(env, targetBcOff, spillValues);
}

Block* makeExit(HTS& env, TransFlags trflags) {
  auto spillValues = peekSpillValues(env);
  return makeExit(env, -1, spillValues, trflags);
}

Block* makeExit(HTS& env,
                Offset targetBcOff,
                std::vector<SSATmp*>& spillValues,
                TransFlags trflags) {
  if (targetBcOff == -1) targetBcOff = bcOff(env);
  return makeExitImpl(env, targetBcOff, ExitFlag::JIT,
    spillValues, CustomExit{}, trflags);
}

Block* makePseudoMainExit(HTS& env, Offset targetBcOff /* = -1 */) {
  return curFunc(env)->isPseudoMain()
    ? makeExit(env, targetBcOff)
    : nullptr;
}

Block* makeExitError(HTS& env, SSATmp* msg, Block* catchBlock) {
  auto const exit = env.irb->makeExit();
  BlockPusher bp(*env.irb, env.irb->nextMarker(), exit);
  gen(env, RaiseError, catchBlock, msg);
  return exit;
}

Block* makeExitNullThis(HTS& env) {
  return makeExitError(
    env,
    cns(env, makeStaticString(Strings::FATAL_NULL_THIS)),
    makeCatch(env)
  );
}

Block* makeExitSlow(HTS& env) {
  auto spillValues = peekSpillValues(env);
  return makeExitImpl(
    env,
    bcOff(env),
    ExitFlag::Interp,
    spillValues,
    CustomExit{}
  );
}

Block* makeExitOpt(HTS& env, TransID transId) {
  Offset targetBcOff = bcOff(env);
  auto const exit = env.irb->makeExit();

  BCMarker exitMarker {
    SrcKey { curFunc(env), targetBcOff, resumed(env) },
    static_cast<int32_t>(
      env.irb->spOffset() + env.irb->evalStack().size() -
        env.irb->stackDeficit()),
    env.profTransID
  };

  BlockPusher blockPusher(*env.irb, exitMarker, exit);

  SSATmp* stack = nullptr;
  if (env.irb->stackDeficit() != 0 || !env.irb->evalStack().empty()) {
    stack = spillStack(env);
  } else {
    stack = sp(env);
  }

  gen(env, SyncABIRegs, fp(env), stack);
  gen(env, ReqRetranslateOpt, ReqRetransOptData(transId, targetBcOff));

  return exit;
}

Block* makeExitImpl(HTS& env,
                    Offset targetBcOff,
                    ExitFlag flag,
                    std::vector<SSATmp*>& stackValues,
                    const CustomExit& customFn,
                    TransFlags trflags) {
  auto const curBcOff = bcOff(env);
  env.irb->evalStack().swap(stackValues);
  SCOPE_EXIT {
    env.bcStateStack.back().setOffset(curBcOff);
    env.irb->evalStack().swap(stackValues);
  };

  auto exitMarker = makeMarker(env, targetBcOff);

  auto const exit = env.irb->makeExit();
  BlockPusher tp(*env.irb, exitMarker, exit);

  env.bcStateStack.back().setOffset(targetBcOff);

  auto stack = spillStack(env);

  if (customFn) {
    stack = gen(env, ExceptionBarrier, stack);
    auto const customTmp = customFn();
    if (customTmp) {
      SSATmp* spill2[] = { stack, cns(env, 0), customTmp };
      stack = gen(
        env,
        SpillStack,
        std::make_pair(sizeof spill2 / sizeof spill2[0], spill2)
      );
      exitMarker.setSpOff(exitMarker.spOff() + 1);
    }
  }

  if (flag == ExitFlag::Interp) {
    auto interpSk = SrcKey { curFunc(env), targetBcOff, resumed(env) };
    auto pc = curUnit(env)->at(targetBcOff);
    auto changesPC = opcodeChangesPC(*reinterpret_cast<const Op*>(pc));
    auto interpOp = changesPC ? InterpOneCF : InterpOne;

    InterpOneData idata;
    idata.bcOff = targetBcOff;
    idata.cellsPopped = getStackPopped(pc);
    idata.cellsPushed = getStackPushed(pc);
    idata.opcode = *reinterpret_cast<const Op*>(pc);

    stack = gen(env, interpOp, idata, makeCatchNoSpill(env), stack, fp(env));

    if (!changesPC) {
      // If the op changes PC, InterpOneCF handles getting to the right place
      gen(env, SyncABIRegs, fp(env), stack);
      gen(env, ReqBindJmp, ReqBindJmpData(interpSk.advanced().offset()));
    }
    return exit;
  }

  gen(env, SyncABIRegs, fp(env), stack);

  if (!isInlining(env) &&
      curBcOff == env.context.initBcOffset &&
      targetBcOff == env.context.initBcOffset) {
    // Note that if we're inlining, then targetBcOff is in the inlined
    // func, while context.initBcOffset is in the outer func, so
    // bindJmp will always work (and there's no guarantee that there
    // is an anchor translation, so we must not use ReqRetranslate).
    gen(env, ReqRetranslate, ReqRetranslateData(trflags));
  } else {
    gen(env, ReqBindJmp, ReqBindJmpData(targetBcOff, trflags));
  }
  return exit;
}

//////////////////////////////////////////////////////////////////////

Block* makeCatchNoSpill(HTS& env) {
  return makeCatchImpl(env, [&] { return sp(env); });
}

Block* makeCatch(HTS& env) {
  return makeCatchImpl(env, [&] {
    auto spills = peekSpillValues(env);
    return implSpillStack(env, sp(env), spills, 0);
  });
}

//////////////////////////////////////////////////////////////////////

}}}
