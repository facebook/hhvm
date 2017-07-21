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

#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/write-lease.h"

#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgNop(IRLS&, const IRInstruction*) {}
void cgDefConst(IRLS&, const IRInstruction*) {}
void cgEndGuards(IRLS&, const IRInstruction*) {}
void cgExitPlaceholder(IRLS&, const IRInstruction*) {}

///////////////////////////////////////////////////////////////////////////////

void cgFuncGuard(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<FuncGuard>();
  vmain(env) << funcguard{extra->func, extra->prologueAddrPtr};
}

void cgDefFP(IRLS&, const IRInstruction*) {}

void cgDefSP(IRLS& env, const IRInstruction* inst) {
  auto const sp = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  if (inst->marker().resumed()) {
    v << defvmsp{sp};
    return;
  }

  auto const fp = srcLoc(env, inst, 0).reg();
  v << lea{fp[-cellsToBytes(inst->extra<DefSP>()->offset.offset)], sp};
}

void cgEagerSyncVMRegs(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<EagerSyncVMRegs>();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 1).reg();
  auto& v = vmain(env);

  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(extra->offset.offset)], sync_sp};
  emitEagerSyncPoint(v, inst->marker().fixupSk().pc(), rvmtl(), fp, sync_sp);
}

void cgMov(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0);
  auto const src = srcLoc(env, inst, 0);
  always_assert(inst->src(0)->numWords() == inst->dst(0)->numWords());
  copyTV(vmain(env), src, dst, inst->dst()->type());
}

void cgUnreachable(IRLS& env, const IRInstruction* /*inst*/) {
  vmain(env) << ud2{};
}

void cgEndBlock(IRLS& env, const IRInstruction* /*inst*/) {
  vmain(env) << ud2{};
}

///////////////////////////////////////////////////////////////////////////////

void cgInterpOne(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<InterpOne>();
  auto const sp = srcLoc(env, inst, 0).reg();

  auto const helper = interpOneEntryPoints[size_t(extra->opcode)];
  auto const args = argGroup(env, inst)
    .ssa(1)
    .addr(sp, cellsToBytes(extra->spOffset.offset))
    .imm(extra->bcOff);

  // Call the interpOne##Op() routine, which syncs VM regs manually.
  cgCallHelper(vmain(env), env, CallSpec::direct(helper),
               kVoidDest, SyncOptions::None, args);
}

void cgInterpOneCF(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<InterpOneCF>();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(extra->spOffset.offset)], sync_sp};
  v << syncvmsp{sync_sp};

  assertx(tc::ustubs().interpOneCFHelpers.count(extra->opcode));

  // We pass the Offset in the third argument register.
  v << ldimml{extra->bcOff, rarg(2)};
  v << jmpi{tc::ustubs().interpOneCFHelpers.at(extra->opcode),
            interp_one_cf_regs()};
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(GetTime);

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(PrintBool)
IMPL_OPCODE_CALL(PrintInt)
IMPL_OPCODE_CALL(PrintStr)

IMPL_OPCODE_CALL(GetMemoKey)

void cgRBTraceEntry(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RBTraceEntry>();

  auto const args = argGroup(env, inst)
    .imm(extra->type)
    .imm(extra->sk.toAtomicInt());

  cgCallHelper(vmain(env), env, CallSpec::direct(Trace::ringbufferEntryRip),
               kVoidDest, SyncOptions::None, args);
}

void cgRBTraceMsg(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<RBTraceMsg>();
  assertx(extra->msg->isStatic());

  auto const args = argGroup(env, inst)
    .immPtr(extra->msg->data())
    .imm(extra->msg->size())
    .imm(extra->type);

  cgCallHelper(vmain(env), env, CallSpec::direct(Trace::ringbufferMsg),
               kVoidDest, SyncOptions::None, args);
}

///////////////////////////////////////////////////////////////////////////////

void cgIncStat(IRLS& env, const IRInstruction *inst) {
  auto const stat = Stats::StatCounter(inst->src(0)->intVal());
  auto const n = inst->src(1)->intVal();
  auto const force = inst->src(2)->boolVal();
  emitIncStat(vmain(env), stat, n, force);
}

IMPL_OPCODE_CALL(IncStatGrouped)

void cgIncProfCounter(IRLS& env, const IRInstruction* inst) {
  auto const transID = inst->extra<TransIDData>()->transId;
  auto const counterAddr = profData()->transCounterAddr(transID);
  auto& v = vmain(env);

  v << decqmlock{v.cns(counterAddr)[0], v.makeReg()};
}

void cgCheckCold(IRLS& env, const IRInstruction* inst) {
  auto const transID = inst->extra<CheckCold>()->transId;
  auto const counterAddr = profData()->transCounterAddr(transID);
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << decqmlock{v.cns(counterAddr)[0], sf};
  if (RuntimeOption::EvalJitFilterLease) {
    auto filter = v.makeBlock();
    v << jcc{CC_LE, sf, {label(env, inst->next()), filter}};
    v = filter;
    auto const res = v.makeReg();
    cgCallHelper(v, env, CallSpec::direct(couldAcquireOptimizeLease),
                 callDest(res), SyncOptions::None,
                 argGroup(env, inst).immPtr(inst->func()));
    auto const sf2 = v.makeReg();
    v << testb{res, res, sf2};
    v << jcc{CC_NZ, sf2, {label(env, inst->next()), label(env, inst->taken())}};
  } else {
    v << jcc{CC_LE, sf, {label(env, inst->next()), label(env, inst->taken())}};
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
