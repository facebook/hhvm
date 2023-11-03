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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-data.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void maybe_syncsp(Vout& v, const BCMarker& marker, Vreg sp, IRSPRelOffset off) {
  if (marker.resumeMode() == ResumeMode::None) {
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      v << syncvmsp{v.cns(0x42)};
    }
    return;
  }
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(off.offset)], sync_sp};
  v << syncvmsp{sync_sp};
}

RegSet cross_trace_args(const BCMarker& marker, SrcKey target) {
  if (target.valid() && target.funcEntry()) {
    auto const func = target.func();
    auto const withCtx =
      func->isClosureBody() || func->cls() ||
      RuntimeOption::EvalHHIRGenerateAsserts;
    return func_entry_regs(withCtx);
  }

  return marker.resumeMode() != ResumeMode::None
    ? cross_trace_regs_resumed() : cross_trace_regs();
}

void popFrameToFuncEntryRegs(Vout& v, const Func* func) {
  v << unrecordbasenativesp{};
  if (func->isClosureBody() || func->cls()) {
    v << load{Vreg(rvmfp()) + AROFF(m_thisUnsafe), r_func_entry_ctx()};
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << copy{v.cns(ActRec::kTrashedThisSlot), r_func_entry_ctx()};
  }
  v << loadl{Vreg(rvmfp()) + AROFF(m_callOffAndFlags), r_func_entry_ar_flags()};
  v << loadl{Vreg(rvmfp()) + AROFF(m_funcId), r_func_entry_callee_id()};
  v << copy{rvmfp(), rvmsp()};
  v << restoreripm{Vreg(rvmfp()) + AROFF(m_savedRip)};
  v << load{Vreg(rvmsp()) + AROFF(m_sfp), rvmfp()};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void cgDefLabel(IRLS& env, const IRInstruction* inst) {
  auto const arity = inst->numDsts();
  if (arity == 0) return;

  auto& v = vmain(env);

  VregList args;
  for (unsigned i = 0; i < arity; i++) {
    auto const dloc = dstLoc(env, inst, i);
    args.push_back(dloc.reg(0));
    if (dloc.numAllocated() == 2) {
      args.push_back(dloc.reg(1));
    } else {
      always_assert(dloc.numAllocated() == 1);
    }
  }
  v << phidef{v.makeTuple(std::move(args))};
}

void cgJmp(IRLS& env, const IRInstruction* inst) {
  auto target = label(env, inst->taken());
  auto& v = vmain(env);

  auto const arity = inst->numSrcs();
  if (arity == 0) {
    v << jmp{target};
    return;
  }

  auto const& def = inst->taken()->front();
  always_assert(arity == def.numDsts());

  VregList args;
  for (unsigned i = 0; i < arity; i++) {
    auto const src = inst->src(i);
    auto const sloc = srcLoc(env, inst, i);
    auto const dloc = dstLoc(env, &def, i);

    always_assert(sloc.numAllocated() <= dloc.numAllocated());
    always_assert(dloc.numAllocated() >= 1);

    // Handle phi for the value.
    auto val = sloc.reg(0);
    if (src->type().nonTrivialSubtypeOf(TBool) && !def.dst(i)->isA(TBool)) {
      val = v.makeReg();
      v << movzbq{sloc.reg(0), val};
    }
    args.push_back(val);

    // Handle phi for the type.
    if (dloc.numAllocated() == 2) {
      auto const type = sloc.numAllocated() == 2
        ? sloc.reg(1)
        : v.cns(src->type().toDataType());
      args.push_back(type);
    }
  }
  v << phijmp{target, v.makeTuple(std::move(args))};
}

void cgSelect(IRLS& env, const IRInstruction* inst) {
  auto const condReg  = srcLoc(env, inst, 0).reg();
  auto const trueTy   = inst->src(1)->type();
  auto const falseTy  = inst->src(2)->type();
  auto const tloc     = srcLoc(env, inst, 1);
  auto const floc     = srcLoc(env, inst, 2);
  auto const dloc     = dstLoc(env, inst, 0);
  auto& v             = vmain(env);

  auto const sf = v.makeReg();
  if (inst->src(0)->isA(TBool)) {
    v << testb{condReg, condReg, sf};
  } else {
    v << testq{condReg, condReg, sf};
  }

  // First copy the type if the destination needs one. This should only apply to
  // types <= TCell.
  if (dloc.hasReg(1)) {
    assertx(trueTy.isKnownDataType() || tloc.hasReg(1));
    assertx(falseTy.isKnownDataType() || floc.hasReg(1));

    auto const trueTyReg = trueTy.isKnownDataType()
      ? v.cns(trueTy.toDataType())
      : tloc.reg(1);
    auto const falseTyReg = falseTy.isKnownDataType()
      ? v.cns(falseTy.toDataType())
      : floc.reg(1);

    v << cmovb{CC_NZ, sf, falseTyReg, trueTyReg, dloc.reg(1)};
  }

  // If the value is statically known (IE, its one of the types with a singleton
  // value), don't bother copying it (this also applies to Bottom).
  if (trueTy <= TBool && falseTy <= TBool) {
    v << cmovb{CC_NZ, sf, floc.reg(0), tloc.reg(0), dloc.reg(0)};
  } else {
    auto const t = zeroExtendIfBool(v, trueTy, tloc.reg(0));
    auto const f = zeroExtendIfBool(v, falseTy, floc.reg(0));
    v << cmovq{CC_NZ, sf, f, t, dloc.reg(0)};
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void implJmpZ(IRLS& env, const IRInstruction* inst, ConditionCode cc) {
  auto const val = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  if (inst->src(0)->isA(TBool)) {
    v << testb{val, val, sf};
  } else {
    v << testq{val, val, sf};
  }
  v << jcc{cc, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

}

void cgJmpZero(IRLS& env, const IRInstruction* inst) {
  implJmpZ(env, inst, CC_Z);
}
void cgJmpNZero(IRLS& env, const IRInstruction* inst) {
  implJmpZ(env, inst, CC_NZ);
}

void cgCheckNullptr(IRLS& env, const IRInstruction* inst) {
  if (!inst->taken()) return;

  auto src = srcLoc(env, inst, 0).reg(0);
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << testq{src, src, sf};
  v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgCheckNonNull(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  assertx(inst->taken());

  auto const sf = v.makeReg();
  v << testq{src, src, sf};
  fwdJcc(v, env, CC_Z, sf, inst->taken());
  v << copy{src, dst};
  auto const regs = inst->dst()->numWords();
  assertx(regs == 1 || (inst->dst()->isA(TLval) && regs == 2));
  if (regs == 2) {
    v << copy{srcLoc(env, inst, 0).reg(1), dstLoc(env, inst, 0).reg(1)};
  }
}

void cgAssertNonNull(IRLS& env, const IRInstruction* inst) {
  auto dst = dstLoc(env, inst, 0).reg();
  auto src = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const sf = v.makeReg();
    v << testq{src, src, sf};
    ifThen(v, CC_Z, sf, [&](Vout& v) { v << trap{TRAP_REASON}; });
  }
  v << copy{src, dst};
}

void cgCheckInit(IRLS& env, const IRInstruction* inst) {
  assertx(inst->taken());

  auto const src = inst->src(0);
  if (!src->type().maybe(TUninit)) return;

  auto const type = srcLoc(env, inst, 0).reg(1);
  assertx(type != InvalidReg);
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpbi{static_cast<data_type_t>(KindOfUninit), type, sf};
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

void cgCheckInitMem(IRLS& env, const IRInstruction* inst) {
  assertx(inst->taken());

  auto const src = inst->src(0);

  auto const ptrLoc = srcLoc(env, inst, 0);
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, memTVTypePtr(src, ptrLoc));

  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

///////////////////////////////////////////////////////////////////////////////

void cgProfileSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileSwitchDest>();
  assertx(!rds::isPersistentHandle(extra->handle));
  auto const idx = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const rcase = v.makeReg();
  auto const sf = v.makeReg();
  v << subq{v.cns(extra->base), idx, rcase, v.makeReg()};
  v << cmpqi{extra->cases - 2, rcase, sf};

  ifThenElse(
    v, CC_AE, sf,
    [&] (Vout& v) {
      // Last vector element is the default case.
      v << inclm{rvmtl()[extra->handle + (extra->cases - 1) * sizeof(int32_t)],
                 v.makeReg()};
    },
    [&] (Vout& v) {
      v << inclm{Vreg{rvmtl()}[rcase * 4 + extra->handle], v.makeReg()};
    }
  );
}

void cgLdSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdSwitchDest>();
  auto const idx = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const marker = inst->marker();
  auto& v = vmain(env);

  auto const table = v.allocData<TCA>(extra->cases);
  for (int i = 0; i < extra->cases; i++) {
    v << bindaddr{&table[i], extra->targets[i], extra->spOffBCFromStackBase};
  }

  auto const t = v.makeReg();
  v << lead{table, t};
  v << load{t[idx * 8], dst};
}

///////////////////////////////////////////////////////////////////////////////

namespace {

using SSwitchMap = FixedStringMap<TCA>;

TCA sswitchHelper(const StringData* val,
                      const SSwitchMap* table,
                      TCA* def) {
  auto const dest = table->find(val);
  return dest ? *dest : *def;
}

}

void cgLdSSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdSSwitchDest>();
  auto& v = vmain(env);

  auto const table = v.allocData<SSwitchMap>();
  // TODO(t10347945): This causes our data section to own a pointer to heap
  // memory, and we're putting bindaddrs in said heap memory.
  new (table) SSwitchMap(extra->numCases);

  std::unordered_map<const StringData*,TCA> map;
  for (int64_t i = 0; i < extra->numCases; ++i) {
    map[extra->cases[i].str] = nullptr;
  }
  table->addFrom(map.begin(), map.end());

  for (int64_t i = 0; i < extra->numCases; ++i) {
    auto const addr = table->find(extra->cases[i].str);

    // The addresses we're passing to bindaddr{} here live in SSwitchMap's heap
    // buffer (see comment above).  They don't need to be relocated like normal
    // VdataPtrs, so bind them here.
    VdataPtr<TCA> dataPtr{nullptr};
    dataPtr.bind(addr);
    v << bindaddr{dataPtr, extra->cases[i].dest, extra->bcSPOff};
  }

  // Bind the default case target.
  auto const def = v.allocData<TCA>();
  v << bindaddr{def, extra->defaultSk, extra->bcSPOff};

  auto const args = argGroup(env, inst)
    .ssa(0)
    .dataPtr(table)
    .dataPtr(def);

  cgCallHelper(v, env, CallSpec::direct(sswitchHelper),
               callDest(env, inst), SyncOptions::None, args);
}


void cgJmpExit(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<JmpExit>();
  auto const marker = inst->marker();
  auto& v = vmain(env);

  maybe_syncsp(v, marker, srcLoc(env, inst, 1).reg(), extra->offset);
  v << jmpr{srcLoc(env, inst, 0).reg(), cross_trace_args(marker, SrcKey{})};
}

///////////////////////////////////////////////////////////////////////////////

void cgReqBindJmp(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ReqBindJmp>();
  auto const target = extra->target;
  auto& v = vmain(env);

  if (target.funcEntry()) {
    if (extra->popFrame) {
      assertx(inst->numSrcs() == 2);
      popFrameToFuncEntryRegs(v, target.func());
    } else {
      assertx(inst->numSrcs() == 5);
      auto const arFlags = srcLoc(env, inst, 2).reg();
      auto const calleeId = srcLoc(env, inst, 3).reg();
      auto const ctx = srcLoc(env, inst, 4).reg();
      v << copy{arFlags, r_func_entry_ar_flags()};
      v << copy{calleeId, r_func_entry_callee_id()};
      if (target.func()->isClosureBody() || target.func()->cls()) {
        v << copy{ctx, r_func_entry_ctx()};
      } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
        v << copy{v.cns(ActRec::kTrashedThisSlot), r_func_entry_ctx()};
      }
    }
  } else {
    assertx(inst->numSrcs() == 2);
    maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->irSPOff);
  }

  v << bindjmp{
    target,
    extra->invSPOff,
    cross_trace_args(inst->marker(), target)
  };
}

void cgReqRetranslate(IRLS& env, const IRInstruction* inst) {
  auto const destSK = env.unit.initSrcKey();
  auto const extra  = inst->extra<ReqRetranslate>();
  auto& v = vmain(env);

  if (destSK.funcEntry()) {
    popFrameToFuncEntryRegs(v, destSK.func());
  } else {
    maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->offset);
  }

  v << fallback{
    destSK,
    inst->marker().bcSPOff(),
    cross_trace_args(inst->marker(), destSK)
  };
}

void cgReqRetranslateOpt(IRLS& env, const IRInstruction* inst) {
  assertx(inst->marker().sk().funcEntry());
  assertx(inst->marker().bcSPOff() == SBInvOffset{0});
  auto& v = vmain(env);
  v << copy{v.cns(inst->marker().sk().numEntryArgs()), rarg(0)};
  // This jmp is not smashable, so the ABI does not need to be compatible with
  // TC targets. This allows the stub to accept pushed frame.
  v << jmpi{
    tc::ustubs().handleRetranslateOpt,
    vm_regs_no_sp() | rarg(0)
  };
}

void cgReqInterpBBNoTranslate(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ReqInterpBBNoTranslate>();
  auto& v = vmain(env);

  if (extra->target.funcEntry()) {
    assertx(extra->popFrame);
    popFrameToFuncEntryRegs(v, extra->target.func());
  } else {
    maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->irSPOff);
  }

  emitInterpReqNoTranslate(v, extra->target, extra->invSPOff);
}

void cgLdBindAddr(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdBindAddr>();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  // Emit service request to smash address of SrcKey into 'addr'.
  v << ldbindaddr{extra->sk, extra->bcSPOff, dst};
}

///////////////////////////////////////////////////////////////////////////////

}
