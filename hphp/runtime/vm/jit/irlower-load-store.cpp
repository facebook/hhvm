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

#include "hphp/runtime/base/memory-manager.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/dce.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/runtime.h"

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

  auto const typePtr = v.makeReg();
  auto const dataPtr = v.makeReg();
  auto const endPtr = v.makeReg();
  v << lea{ptrToLocalType(fp, range->start), typePtr};
  v << lea{ptrToLocalData(fp, range->start), dataPtr};
  v << lea{ptrToLocalType(fp, range->end), endPtr};

  doWhile(v, CC_NE, {typePtr, dataPtr},
    [&] (const VregList& in, const VregList& out) {
      auto const typeIn = in[0];
      auto const dataIn = in[1];
      auto const typeOut = out[0];
      auto const dataOut = out[1];
      auto const sf = v.makeReg();

      storeTV(v, val->type(), loc, *typeIn, *dataIn);
      nextLocal(v, typeIn, dataIn, typeOut, dataOut);
      v << cmpq{typeOut, endPtr, sf};
      return sf;
    },
    range->end - range->start
  );
}

void cgDbgTrashFrame(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<DbgTrashFrame>()->offset.offset);
  for (auto i = 0; i < kNumActRecCells; ++i) {
    trashFullTV(vmain(env), fp[off + cellsToBytes(i)], kTVTrashJITFrame);
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

  auto const type = inst->hasTypeParam()
    ? inst->typeParam()
    : inst->src(1)->type();

  storeTV(vmain(env), sp[off], srcLoc(env, inst, 1), inst->src(1), type);
}

void cgStOutValue(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(
    inst->extra<StOutValue>()->index + kNumActRecCells
  );
  storeTV(vmain(env), fp[off], srcLoc(env, inst, 1), inst->src(1));
}

void cgLdOutAddr(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(
    inst->extra<LdOutAddr>()->index + kNumActRecCells
  );
  vmain(env) << lea{fp[off], dstLoc(env, inst, 0).reg()};
}

void cgDbgTrashStk(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<DbgTrashStk>()->offset.offset);
  trashFullTV(vmain(env), sp[off],  kTVTrashJITStk);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

const Func* funcFromFp(const SSATmp* fp) {
  fp = canonical(fp);
  auto inst = fp->inst();
  if (UNLIKELY(inst->is(DefLabel))) {
    inst = resolveFpDefLabel(fp);
    assertx(inst);
  }
  assertx(inst->is(DefFP, DefFuncEntryFP, DefInlineFP));
  if (inst->is(DefFP)) return inst->marker().func();
  if (inst->is(DefFuncEntryFP)) return inst->extra<DefFuncEntryFP>()->func;
  if (inst->is(DefInlineFP)) return inst->extra<DefInlineFP>()->target;
  always_assert(false);
}

}

///////////////////////////////////////////////////////////////////////////////

void cgLdMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr    = inst->src(0);
  auto const ptrLoc = tmpLoc(env, ptr);
  auto const dstLoc = tmpLoc(env, inst->dst());

  loadTV(vmain(env), inst->dst()->type(), dstLoc,
         memTVTypePtr(ptr, ptrLoc), memTVValPtr(ptr, ptrLoc));
}

void cgStMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr    = inst->src(0);
  auto const ptrLoc = tmpLoc(env, ptr);
  auto const src    = inst->src(1);
  auto const srcLoc = tmpLoc(env, src);

  auto const type   = inst->hasTypeParam() ? inst->typeParam() : src->type();

  storeTV(vmain(env), type, srcLoc,
          memTVTypePtr(ptr, ptrLoc), memTVValPtr(ptr, ptrLoc));
}

void cgDbgTrashMem(IRLS& env, const IRInstruction* inst) {
  auto const ptr    = inst->src(0);
  auto const ptrLoc = tmpLoc(env, ptr);
  trashTV(vmain(env), memTVTypePtr(ptr, ptrLoc), memTVValPtr(ptr, ptrLoc),
          kTVTrashJITHeap);
}

///////////////////////////////////////////////////////////////////////////////

void cgLdClsInitElem(IRLS& env, const IRInstruction* inst) {
  auto const base = srcLoc(env, inst, 0).reg();
  auto const idx = inst->extra<IndexData>()->index;
  auto& v = vmain(env);

  auto const lval_offset = ObjectProps::offsetOf(idx);

  loadTV(v, inst->dst()->type(), dstLoc(env, inst, 0),
         base[lval_offset.typeOffset()],
         base[lval_offset.dataOffset()]);
}

void cgStClsInitElem(IRLS& env, const IRInstruction* inst) {
  auto const base = srcLoc(env, inst, 0).reg();
  auto const idx = inst->extra<IndexData>()->index;
  auto& v = vmain(env);

  auto const lval_offset = ObjectProps::offsetOf(idx);

  storeTV(v, inst->src(1)->type(),
          srcLoc(env, inst, 1),
          base[lval_offset.typeOffset()],
          base[lval_offset.dataOffset()]);
}

///////////////////////////////////////////////////////////////////////////////

void cgLdMIStateAddr(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + inst->src(0)->intVal();
  vmain(env) << lea{rvmtl()[off], dstLoc(env, inst, 0).reg()};
}

void cgLdMIPropStateAddr(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, propState);
  vmain(env) << lea{rvmtl()[off], dstLoc(env, inst, 0).reg()};
}

void cgLdMBase(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, base);
  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  vmain(env) << load{rvmtl()[off], dstLoc.reg(0)};
  if (wide_tv_val) {
    vmain(env) << load{rvmtl()[off + sizeof(intptr_t)], dstLoc.reg(1)};
  }
}

void cgStMBase(IRLS& env, const IRInstruction* inst) {
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, base);
  auto const srcLoc = irlower::srcLoc(env, inst, 0);
  vmain(env) << store{srcLoc.reg(0), rvmtl()[off]};
  if (wide_tv_val) {
    vmain(env) << store{srcLoc.reg(1), rvmtl()[off + sizeof(intptr_t)]};
  }
}

void cgStMIPropState(IRLS& env, const IRInstruction* inst) {
  auto const cls = srcLoc(env, inst, 0).reg();
  auto const slot = srcLoc(env, inst, 1).reg();
  auto const isStatic = inst->src(2)->boolVal();
  auto const off = rds::kVmMInstrStateOff + offsetof(MInstrState, propState);
  auto& v = vmain(env);

  using M = MInstrPropState;

  static_assert(sizeof(M::slotSize() == 4), "");
  static_assert(sizeof(M::clsSize()) == 4 || sizeof(M::clsSize()) == 8, "");

  if (inst->src(0)->isA(TNullptr)) {
    // If the Class* field is null, none of the other fields matter.
    emitStLowPtr(v, v.cns(0), rvmtl()[off + M::clsOff()], M::clsSize());
    return;
  }

  if (inst->src(0)->hasConstVal(TCls) &&
      inst->src(1)->hasConstVal(TInt)) {
    // If everything is a constant, and this is a LowPtr build, we can store the
    // values with a single 64-bit immediate.
    if (M::clsOff() + M::clsSize() == M::slotOff() && M::clsSize() == 4) {
      auto const clsVal = inst->src(0)->clsVal();
      auto const slotVal = inst->src(1)->intVal();
      auto raw = reinterpret_cast<uint64_t>(clsVal);
      raw |= (static_cast<uint64_t>(slotVal) << 32);
      if (isStatic) raw |= 0x1;
      emitImmStoreq(v, raw, rvmtl()[off + M::clsOff()]);
      return;
    }
  }

  auto markedCls = cls;
  if (isStatic) {
    markedCls = v.makeReg();
    v << orqi{0x1, cls, markedCls, v.makeReg()};
  }
  emitStLowPtr(v, markedCls, rvmtl()[off + M::clsOff()], M::clsSize());
  v << storel{slot, rvmtl()[off + M::slotOff()]};
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
  auto& v = vmain(env);
  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  auto const valReg = dstLoc.reg(tv_lval::val_idx);

  auto const src = srcLoc(env, inst, 0).reg();
  auto const offs = ObjectProps::offsetOf(inst->extra<LdPropAddr>()->index)
    .shift(sizeof(ObjectData));

  v << lea{src[offs.dataOffset()], valReg};
  if (wide_tv_val) {
    static_assert(TVOFF(m_data) == 0, "");
    v << lea{src[offs.typeOffset()], dstLoc.reg(tv_lval::type_idx)};
  }
}

void cgLdInitPropAddr(IRLS& env, const IRInstruction* inst) {
  auto const dstLoc = irlower::dstLoc(env, inst, 0);
  auto const obj = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const offs = ObjectProps::offsetOf(inst->extra<LdInitPropAddr>()->index)
    .shift(sizeof(ObjectData));

  if (dstLoc.hasReg(tv_lval::val_idx)) {
    v << lea{obj[offs.dataOffset()], dstLoc.reg(tv_lval::val_idx)};
  }
  if (wide_tv_val && dstLoc.hasReg(tv_lval::type_idx)) {
    static_assert(TVOFF(m_data) == 0, "");
    v << lea{obj[offs.typeOffset()], dstLoc.reg(tv_lval::type_idx)};
  }

  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, obj[offs.typeOffset()]);
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

IMPL_OPCODE_CALL(LdClsPropAddrOrNull)
IMPL_OPCODE_CALL(LdClsPropAddrOrRaise)

///////////////////////////////////////////////////////////////////////////////

void cgCheckRDSInitialized(IRLS& env, const IRInstruction* inst) {
  auto const handle = inst->extra<CheckRDSInitialized>()->handle;
  auto& v = vmain(env);

  if (rds::isNormalHandle(handle)) {
    auto const sf = checkRDSHandleInitialized(v, handle);
    v << jcc{CC_NE, sf, {label(env, inst->next()), label(env, inst->taken())}};
  } else {
    // Always initialized; just fall through to inst->next().
    assertx(rds::isPersistentHandle(handle));
    DEBUG_ONLY bool initialized =
      rds::handleToRef<bool, rds::Mode::Persistent>(handle);
    assertx(initialized);
  }
}

void cgMarkRDSInitialized(IRLS& env, const IRInstruction* inst) {
  auto const handle = inst->extra<MarkRDSInitialized>()->handle;
  if (rds::isNormalHandle(handle)) markRDSHandleInitialized(vmain(env), handle);
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void ldRDSAddrImpl(Vout& v, rds::Handle handle, Vreg dst) {
  if (rds::isPersistentHandle(handle)) {
    // Persistent RDS handles are not relative to RDS base.
    auto const addr = rds::handleToPtr<void, rds::Mode::Persistent>(handle);
    v << copy{v.cns(addr), dst};
  } else {
    v << lea{rvmtl()[handle], dst};
  }
}

}

void cgLdRDSAddr(IRLS& env, const IRInstruction* inst) {
  ldRDSAddrImpl(
    vmain(env),
    inst->extra<LdRDSAddr>()->handle,
    dstLoc(env, inst, 0).reg()
  );
}

void cgLdInitRDSAddr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  ldRDSAddrImpl(v, inst->extra<LdInitRDSAddr>()->handle, dst);

  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, dst[TVOFF(m_type)]);
  v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
}

///////////////////////////////////////////////////////////////////////////////

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
        v << trap{TRAP_REASON};
      });
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

}}}
