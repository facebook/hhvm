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

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void emitCheckSurpriseFlagsEnter(Vout& v, Vout& vcold, Vreg fp,
                                 Fixup fixup, Vlabel catchBlock) {
  auto const cold = vcold.makeBlock();
  auto const done = v.makeBlock();

  auto const sf = v.makeReg();
  v << cmpqm{fp, rvmtl()[rds::kSurpriseFlagsOff], sf};
  v << jcc{CC_NBE, sf, {done, cold}};
  v = done;

  vcold = cold;
  auto const call = CallSpec::stub(tc::ustubs().functionEnterHelper);
  auto const args = v.makeVcallArgs({});
  vcold << vinvoke{call, args, v.makeTuple({}), {done, catchBlock}, fixup};
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

  auto const sf = v.makeReg();
  v << cmpqm{fp_or_sp, rvmtl()[rds::kSurpriseFlagsOff], sf};
  v << jcc{CC_NBE, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgCheckStackOverflow(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const func = inst->marker().func();
  auto& v = vmain(env);

  auto const stackMask = int32_t{
    cellsToBytes(RuntimeOption::EvalVMStackElms) - 1
  };
  auto const depth = cellsToBytes(func->maxStackCells()) +
                     cellsToBytes(kStackCheckPadding) +
                     Stack::sSurprisePageSize;

  auto const r = v.makeReg();
  auto const sf = v.makeReg();
  v << andqi{stackMask, fp, r, v.makeReg()};
  v << subqi{safe_cast<int32_t>(depth), r, v.makeReg(), sf};

  unlikelyIfThen(v, vcold(env), CC_L, sf, [&] (Vout& v) {
    cgCallHelper(v, env, CallSpec::direct(handleStackOverflow), kVoidDest,
                 SyncOptions::Sync, argGroup(env, inst).reg(fp));
  });
}

void cgCheckSurpriseFlagsEnter(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<CheckSurpriseFlagsEnter>();
  auto const func = extra->func;

  auto const off = func->getEntryForNumArgs(extra->argc) - func->base();
  auto const fixup = Fixup(off, func->numSlotsInFrame());

  auto const catchBlock = label(env, inst->taken());
  emitCheckSurpriseFlagsEnter(vmain(env), vcold(env), fp, fixup, catchBlock);
}

void cgCheckSurpriseAndStack(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<CheckSurpriseAndStack>();
  auto const func = extra->func;

  auto const off = func->getEntryForNumArgs(extra->argc) - func->base();
  auto const fixup = Fixup(off, func->numSlotsInFrame());
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  auto const needed_top = v.makeReg();
  v << lea{fp[-cellsToBytes(func->maxStackCells())], needed_top};
  v << cmpqm{needed_top, rvmtl()[rds::kSurpriseFlagsOff], sf};

  unlikelyIfThen(v, vcold(env), CC_AE, sf, [&] (Vout& v) {
    auto const stub = tc::ustubs().functionSurprisedOrStackOverflow;
    auto const done = v.makeBlock();
    v << vinvoke{CallSpec::stub(stub), v.makeVcallArgs({}), v.makeTuple({}),
                 {done, label(env, inst->taken())}, fixup };
    v = done;
  });
}

IMPL_OPCODE_CALL(SuspendHookE)
IMPL_OPCODE_CALL(SuspendHookR)
IMPL_OPCODE_CALL(ReturnHook)

///////////////////////////////////////////////////////////////////////////////

}}}
