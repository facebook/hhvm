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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBeginCatch(IRLS& env, const IRInstruction* inst) {
  auto const callType = env.catch_calls[inst->block()];
  always_assert(callType != CatchCall::Uninit &&
                "Tried to emit BeginCatch with Uninit call type.\n"
                "Catch blocks must be emitted after their predecessors.");
  auto& v = vmain(env);

  v << landingpad{callType == CatchCall::PHP};
  emitIncStat(v, Stats::TC_CatchTrace);
}

void cgEndCatch(IRLS& env, const IRInstruction* inst) {
  // endCatchHelper only expects rvmtl() and rvmfp() to be live.
  vmain(env) << jmpi{tc::ustubs().endCatchHelper, rvmtl() | rvmfp()};
}

void cgUnwindCheckSideExit(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpbim{0, rvmtl()[unwinderSideExitOff()], sf};
  fwdJcc(v, env, CC_E, sf, inst->taken());

  // doSideExit == true, so fall through to the side exit code.
  emitIncStat(v, Stats::TC_CatchSideExit);
}

void cgLdUnwinderValue(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), rvmtl()[unwinderTVOff()]);
}

IMPL_OPCODE_CALL(DebugBacktrace)
IMPL_OPCODE_CALL(InitThrowableFileAndLine)
IMPL_OPCODE_CALL(ZeroErrorLevel)
IMPL_OPCODE_CALL(RestoreErrorLevel)

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(RaiseArrayIndexNotice)
IMPL_OPCODE_CALL(RaiseArrayKeyNotice)
IMPL_OPCODE_CALL(RaiseError)
IMPL_OPCODE_CALL(RaiseMissingArg)
IMPL_OPCODE_CALL(RaiseNotice)
IMPL_OPCODE_CALL(RaiseUndefProp)
IMPL_OPCODE_CALL(RaiseUninitLoc)
IMPL_OPCODE_CALL(RaiseWarning)
IMPL_OPCODE_CALL(RaiseMissingThis)
IMPL_OPCODE_CALL(FatalMissingThis)
IMPL_OPCODE_CALL(ThrowArithmeticError)
IMPL_OPCODE_CALL(ThrowDivisionByZeroError)
IMPL_OPCODE_CALL(ThrowInvalidArrayKey)
IMPL_OPCODE_CALL(ThrowInvalidOperation)
IMPL_OPCODE_CALL(ThrowOutOfBounds)

///////////////////////////////////////////////////////////////////////////////

}}}
