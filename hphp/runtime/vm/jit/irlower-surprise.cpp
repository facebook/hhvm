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

#include "hphp/runtime/vm/jit/irlower-internal.h"

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-overflow.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void emitCheckSurpriseFlags(Vout& v, Vreg fp, Vlabel handleSurprise) {
  auto const done = v.makeBlock();
  auto const sf = v.makeReg();
  v << cmpqm{fp, rvmtl()[rds::kSurpriseFlagsOff], sf};
  v << jcc{CC_NBE, sf, {done, handleSurprise}};
  v = done;
}

void cgCheckSurpriseFlags(IRLS& env, const IRInstruction* inst) {
  // This is not a correctness assertion, but we want to know if we get it
  // wrong because it'll be a subtle perf bug:
  if (inst->marker().resumeMode() != ResumeMode::None) {
    assertx(inst->src(0)->isA(TStkPtr));
  } else {
    assertx(inst->src(0)->isA(TFramePtr));
  }
  auto const fp_or_sp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  emitCheckSurpriseFlags(v, fp_or_sp, label(env, inst->taken()));
}

static void handleSurpriseCheck() {
  size_t flags = handle_request_surprise();
  // Memory Threhsold callback should also be fired here
  if (flags & MemThresholdFlag) {
    EventHook::DoMemoryThresholdCallback();
  }
  if (flags & TimedOutFlag) {
    RID().invokeUserTimeoutCallback();
  }
  if (flags & DebuggerSignalFlag) {
    VMRegAnchor _;
    markFunctionWithDebuggerIntr(vmfp()->func());
  }
}

void cgHandleRequestSurprise(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  cgCallHelper(v, env, CallSpec::direct(handleSurpriseCheck), kVoidDest,
    SyncOptions::Sync, argGroup(env, inst));
}

void cgCheckStackOverflow(IRLS& env, const IRInstruction* inst) {
  auto const calleeFP = srcLoc(env, inst, 0).reg();
  auto const callee = inst->marker().func();
  auto& v = vmain(env);

  auto const stackMask = int32_t{
    cellsToBytes(RuntimeOption::EvalVMStackElms) - 1
  };
  auto const depth = cellsToBytes(callee->maxStackCells()) +
                     cellsToBytes(stackCheckPadding()) +
                     Stack::sSurprisePageSize;

  auto const r = v.makeReg();
  auto const sf = v.makeReg();
  v << andqi{stackMask, calleeFP, r, v.makeReg()};
  v << subqi{safe_cast<int32_t>(depth), r, v.makeReg(), sf};

  unlikelyIfThen(v, vcold(env), CC_L, sf, [&] (Vout& v) {
    cgCallHelper(v, env, CallSpec::direct(throw_stack_overflow), kVoidDest,
                 SyncOptions::Sync, argGroup(env, inst));
  });
}

void cgCheckSurpriseFlagsEnter(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const& extra = inst->extra<CheckSurpriseFlagsEnter>();
  auto& v = vmain(env);

  // Check for surprise. Check also for stack overflow if requested.
  auto const sf = v.makeReg();
  auto const neededTop = extra->checkStackOverflow ? v.makeReg() : fp;
  if (extra->checkStackOverflow) {
    v << lea{fp[-cellsToBytes(extra->func->maxStackCells())], neededTop};
  }
  v << cmpqm{neededTop, rvmtl()[rds::kSurpriseFlagsOff], sf};

  auto const done = v.makeBlock();
  auto const handleSurprise = label(env, inst->taken());
  if (extra->func->isInterceptable()) {
    v << interceptjcc{CC_AE, sf, {done, handleSurprise}};
  } else {
    v << jcc{CC_AE, sf, {done, handleSurprise}};
  }
  v = done;
}

void cgHandleSurpriseEnter(IRLS& env, const IRInstruction* inst) {
  auto const& extra = inst->extra<HandleSurpriseEnter>();
  auto& v = vmain(env);

  // Call the surprise / stack overflow handler.
  auto const stub = extra->checkStackOverflow
    ? tc::ustubs().functionSurprisedOrStackOverflow
    : tc::ustubs().functionSurprised;
  auto const done = v.makeBlock();
  auto const catchBlock = label(env, inst->taken());
  auto const fixup = makeFixup(inst->marker(), SyncOptions::Sync);
  v << vinvoke{CallSpec::stub(stub), v.makeVcallArgs({}), v.makeTuple({}),
               {done, catchBlock}, fixup};
  v = done;

  // If the function is not interceptable, the handler returned nullptr.
  if (!extra->func->isInterceptable()) return;

  auto const sf = v.makeReg();
  auto const rIntercept = rarg(3); // NB: must match emitFunctionSurprised
  assertx(!php_return_regs().contains(rIntercept));

  v << testq{rIntercept, rIntercept, sf};
  ifThen(v, CC_NZ, sf, [&] (Vout& v) {
    // We are intercepting. Return to the caller.
    v << unrecordbasenativesp{};
    v << jmpr{rIntercept, php_return_regs()};
  });
}

IMPL_OPCODE_CALL(SuspendHookAwaitEF)
IMPL_OPCODE_CALL(SuspendHookAwaitEG)
IMPL_OPCODE_CALL(SuspendHookAwaitR)
IMPL_OPCODE_CALL(SuspendHookCreateCont)
IMPL_OPCODE_CALL(SuspendHookYield)
IMPL_OPCODE_CALL(ReturnHook)

///////////////////////////////////////////////////////////////////////////////

}
