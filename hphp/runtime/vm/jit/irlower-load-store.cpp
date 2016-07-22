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

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/ref-data.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgLdLoc(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = localOffset(inst->extra<LdLoc>()->locId);
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), fp[off]);
}

void cgLdLocAddr(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = localOffset(inst->extra<LdLocAddr>()->locId);
  if (dstLoc(env, inst, 0).hasReg()) {
    vmain(env) << lea{fp[off], dstLoc(env, inst, 0).reg()};
  }
}

void cgStLoc(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = localOffset(inst->extra<StLoc>()->locId);
  storeTV(vmain(env), fp[off], srcLoc(env, inst, 1), inst->src(1));
}

void cgStLocRange(IRLS& env, const IRInstruction* inst) {
  auto const range = inst->extra<StLocRange>();

  if (range->start >= range->end) return;

  auto const fp = srcLoc(env, inst, 0).reg();
  auto const loc = srcLoc(env, inst, 1);
  auto const val = inst->src(1);
  auto& v = vmain(env);

  auto ireg = v.makeReg();
  auto nreg = v.makeReg();

  v << lea{fp[localOffset(range->start)], ireg};
  v << lea{fp[localOffset(range->end)], nreg};

  doWhile(v, CC_NE, {ireg},
    [&] (const VregList& in, const VregList& out) {
      auto const i = in[0];
      auto const res = out[0];
      auto const sf = v.makeReg();

      storeTV(v, i[0], loc, val);
      v << subqi{int32_t{sizeof(Cell)}, i, res, v.makeReg()};
      v << cmpq{res, nreg, sf};
      return sf;
    }
  );
}

void cgDbgTrashFrame(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<DbgTrashFrame>()->offset.offset);
  for (auto i = 0; i < kNumActRecCells; ++i) {
    trashTV(vmain(env), fp, off + cellsToBytes(i), kTVTrashJITFrame);
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgLdLocPseudoMain(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = localOffset(inst->extra<LdLocPseudoMain>()->locId);
  auto& v = vmain(env);

  irlower::emitTypeCheck(v, env, inst->typeParam(), fp[off + TVOFF(m_type)],
                         fp[off + TVOFF(m_data)], inst->taken());
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), fp[off]);
}

void cgStLocPseudoMain(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = localOffset(inst->extra<StLocPseudoMain>()->locId);
  storeTV(vmain(env), fp[off], srcLoc(env, inst, 1), inst->src(1));
}

///////////////////////////////////////////////////////////////////////////////

void cgLdStk(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdStk>()->offset.offset);
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), sp[off]);
}

void cgLdStkAddr(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdStkAddr>()->offset.offset);
  vmain(env) << lea{sp[off], dstLoc(env, inst, 0).reg()};
}

void cgStStk(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<StStk>()->offset.offset);
  storeTV(vmain(env), sp[off], srcLoc(env, inst, 1), inst->src(1));
}

void cgDbgTrashStk(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<DbgTrashStk>()->offset.offset);
  trashTV(vmain(env), sp, off, kTVTrashJITStk);
}

///////////////////////////////////////////////////////////////////////////////

void cgLdMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0), *ptr);
}

void cgStMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();
  storeTV(vmain(env), *ptr, srcLoc(env, inst, 1), inst->src(1));
}

void cgDbgTrashMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();
  trashTV(vmain(env), ptr, 0, kTVTrashJITHeap);
}

///////////////////////////////////////////////////////////////////////////////

void cgLdRef(IRLS& env, const IRInstruction* inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();
  loadTV(vmain(env), inst->dst(), dstLoc(env, inst, 0),
         ptr[RefData::tvOffset()]);
}

void cgStRef(IRLS& env, const IRInstruction* inst) {
  auto const ptr = srcLoc(env, inst, 0).reg();
  auto const valLoc = srcLoc(env, inst, 1);
  always_assert(!srcLoc(env, inst, 1).isFullSIMD());

  storeTV(vmain(env), ptr[RefData::tvOffset()], valLoc, inst->src(1));
}

///////////////////////////////////////////////////////////////////////////////

void cgLdElem(IRLS& env, const IRInstruction* inst) {
  auto const rbase = srcLoc(env, inst, 0).reg();
  auto const ridx = srcLoc(env, inst, 1).reg();
  auto const idx = inst->src(1);
  auto& v = vmain(env);

  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    loadTV(v, inst->dst(), dstLoc(env, inst, 0), rbase[idx->intVal()]);
  } else {
    loadTV(v, inst->dst(), dstLoc(env, inst, 0), rbase[ridx]);
  }
}

void cgStElem(IRLS& env, const IRInstruction* inst) {
  auto const rbase = srcLoc(env, inst, 0).reg();
  auto const ridx = srcLoc(env, inst, 1).reg();
  auto const idx = inst->src(1);
  auto& v = vmain(env);

  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    storeTV(v, rbase[idx->intVal()], srcLoc(env, inst, 2), inst->src(2));
  } else {
    storeTV(v, rbase[ridx], srcLoc(env, inst, 2), inst->src(2));
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgLdMIStateAddr(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + inst->src(0)->intVal();
  vmain(env) << lea{rvmtl()[off], dstLoc(env, inst, 0).reg()};
}

void cgLdMBase(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, base);
  vmain(env) << load{rvmtl()[off], dstLoc(env, inst, 0).reg()};
}

void cgStMBase(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, base);
  vmain(env) << store{srcLoc(env, inst, 0).reg(), rvmtl()[off]};
}

///////////////////////////////////////////////////////////////////////////////

static TypedValue* ldGblAddrHelper(const StringData* name) {
  return g_context->m_globalVarEnv->lookup(name);
}

void cgLdGblAddr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  cgCallHelper(v, env, CallSpec::direct(ldGblAddrHelper), callDest(dst),
               SyncOptions::None, argGroup(env, inst).ssa(0));

  auto const sf = v.makeReg();
  v << testq{dst, dst, sf};
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

IMPL_OPCODE_CALL(LdGblAddrDef)

void cgLdPropAddr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const obj = srcLoc(env, inst, 0).reg();
  vmain(env) << lea{obj[inst->extra<LdPropAddr>()->offsetBytes], dst};
}

IMPL_OPCODE_CALL(LdClsPropAddrOrNull)
IMPL_OPCODE_CALL(LdClsPropAddrOrRaise)

///////////////////////////////////////////////////////////////////////////////

void cgLdRDSAddr(IRLS& env, const IRInstruction* inst) {
  auto const handle = inst->extra<LdRDSAddr>()->handle;
  vmain(env) << lea{rvmtl()[handle], dstLoc(env, inst, 0).reg()};
}

void cgLdTVAux(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();

  auto const tv = srcLoc(env, inst, 0);
  assertx(tv.hasReg(1));
  auto const type = tv.reg(1);

  auto& v = vmain(env);
  v << shrqi{32, type, dst, v.makeReg()};

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const extra = inst->extra<LdTVAux>();
    auto const mask = -extra->valid - 1;

    if (mask) {
      auto const sf = v.makeReg();
      v << testqi{mask, dst, sf};
      ifThen(v, CC_NZ, sf, [](Vout& v) {
        v << ud2{};
      });
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
