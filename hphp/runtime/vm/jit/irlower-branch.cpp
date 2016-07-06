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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/tv-helpers.h"

#include "hphp/runtime/vm/jit/types.h"
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
#include "hphp/runtime/vm/jit/vasm-data.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

void maybe_syncsp(Vout& v, BCMarker marker, Vreg sp, IRSPRelOffset off) {
  if (!marker.resumed()) {
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      v << syncvmsp{v.cns(0x42)};
    }
    return;
  }
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(off.offset)], sync_sp};
  v << syncvmsp{sync_sp};
}

RegSet cross_trace_args(BCMarker marker) {
  return marker.resumed() ? cross_trace_regs_resumed() : cross_trace_regs();
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
    if (src->isA(TBool) && !def.dst(i)->isA(TBool)) {
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

///////////////////////////////////////////////////////////////////////////////

void cgProfileSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ProfileSwitchDest>();
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

void cgJmpSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<JmpSwitchDest>();
  auto const idx = srcLoc(env, inst, 0).reg();
  auto const marker = inst->marker();
  auto& v = vmain(env);

  maybe_syncsp(v, marker, srcLoc(env, inst, 1).reg(), extra->irSPOff);

  auto const table = v.allocData<TCA>(extra->cases);
  for (int i = 0; i < extra->cases; i++) {
    v << bindaddr{&table[i], extra->targets[i], extra->invSPOff};
  }

  auto const t = v.makeReg();
  v << lead{table, t};
  v << jmpm{t[idx * 8], cross_trace_args(marker)};
}

///////////////////////////////////////////////////////////////////////////////

namespace {

using SSwitchMap = FixedStringMap<TCA,true>;

TCA sswitchHelperFast(const StringData* val,
                      const SSwitchMap* table,
                      TCA* def) {
  auto const dest = table->find(val);
  return dest ? *dest : *def;
}

TCA sswitchHelperSlow(TypedValue tv, const StringData** strs,
                      int numCases, TCA* jmptab) {
  auto const cell = tvToCell(&tv);
  for (int i = 0; i < numCases; ++i) {
    if (cellEqual(*cell, strs[i])) return jmptab[i];
  }
  return jmptab[numCases]; // default case
}

}

void cgLdSSwitchDestFast(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdSSwitchDestFast>();
  auto& v = vmain(env);

  auto const table = v.allocData<SSwitchMap>();
  // TODO(t10347945): This causes our data section to own a pointer to heap
  // memory, and we're putting bindaddrs in said heap memory.
  new (table) SSwitchMap(extra->numCases);

  for (int64_t i = 0; i < extra->numCases; ++i) {
    table->add(extra->cases[i].str, nullptr);
    auto const addr = table->find(extra->cases[i].str);

    // The addresses we're passing to bindaddr{} here live in SSwitchMap's heap
    // buffer (see comment above).  They don't need to be relocated like normal
    // VdataPtrs, so bind them here.
    VdataPtr<TCA> dataPtr{nullptr};
    dataPtr.bind(addr);
    v << bindaddr{dataPtr, extra->cases[i].dest, extra->spOff};
  }

  // Bind the default case target.
  auto const def = v.allocData<TCA>();
  v << bindaddr{def, extra->defaultSk, extra->spOff};

  auto const args = argGroup(env, inst)
    .ssa(0)
    .dataPtr(table)
    .dataPtr(def);

  cgCallHelper(v, env, CallSpec::direct(sswitchHelperFast),
               callDest(env, inst), SyncOptions::None, args);
}

void cgLdSSwitchDestSlow(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<LdSSwitchDestSlow>();
  auto& v = vmain(env);

  auto strtab = v.allocData<const StringData*>(extra->numCases);
  auto jmptab = v.allocData<TCA>(extra->numCases + 1);

  for (int64_t i = 0; i < extra->numCases; ++i) {
    strtab[i] = extra->cases[i].str;
    v << bindaddr{&jmptab[i], extra->cases[i].dest, extra->spOff};
  }
  v << bindaddr{&jmptab[extra->numCases], extra->defaultSk, extra->spOff};

  auto const args = argGroup(env, inst)
    .typedValue(0)
    .dataPtr(strtab)
    .imm(extra->numCases)
    .dataPtr(jmptab);

  cgCallHelper(v, env, CallSpec::direct(sswitchHelperSlow),
               callDest(env, inst), SyncOptions::Sync, args);
}

void cgJmpSSwitchDest(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<JmpSSwitchDest>();
  auto const marker = inst->marker();
  auto& v = vmain(env);

  maybe_syncsp(v, marker, srcLoc(env, inst, 1).reg(), extra->offset);
  v << jmpr{srcLoc(env, inst, 0).reg(), cross_trace_args(marker)};
}

///////////////////////////////////////////////////////////////////////////////

void cgReqBindJmp(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ReqBindJmp>();
  auto& v = vmain(env);
  maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->irSPOff);
  v << bindjmp{
    extra->target,
    extra->invSPOff,
    extra->trflags,
    cross_trace_args(inst->marker())
  };
}

void cgReqRetranslate(IRLS& env, const IRInstruction* inst) {
  auto const destSK = env.unit.initSrcKey();
  auto const extra  = inst->extra<ReqRetranslate>();
  auto& v = vmain(env);

  maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->irSPOff);
  v << fallback{
    destSK,
    inst->marker().spOff(),
    extra->trflags,
    cross_trace_args(inst->marker())
  };
}

void cgReqRetranslateOpt(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<ReqRetranslateOpt>();
  auto& v = vmain(env);
  maybe_syncsp(v, inst->marker(), srcLoc(env, inst, 0).reg(), extra->irSPOff);
  v << retransopt{
    extra->transID,
    extra->target,
    inst->marker().spOff(),
    cross_trace_args(inst->marker())
  };
}

///////////////////////////////////////////////////////////////////////////////

}}}
