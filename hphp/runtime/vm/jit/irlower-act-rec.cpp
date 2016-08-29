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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/call-spec.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/trace.h"
#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

namespace {

int iterOffset(const BCMarker& marker, uint32_t id) {
  auto const func = marker.func();
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

}

void cgSpillFrame(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<SpillFrame>();
  auto const funcTmp = inst->src(1);
  auto const ctxTmp = inst->src(2);
  auto& v = vmain(env);

  auto const ar = sp[cellsToBytes(extra->spOffset.offset)];

  // Set m_this/m_cls.
  if (ctxTmp->isA(TCls)) {
    // Store the Class* as a Cctx.
    if (ctxTmp->hasConstVal()) {
      emitImmStoreq(v,
                    uintptr_t(ctxTmp->clsVal()) | ActRec::kHasClassBit,
                    ar + AROFF(m_thisUnsafe));
    } else {
      auto const cls = srcLoc(env, inst, 2).reg();
      auto const cctx = v.makeReg();
      v << orqi{ActRec::kHasClassBit, cls, cctx, v.makeReg()};
      v << store{cctx, ar + AROFF(m_thisUnsafe)};
    }
  } else if (ctxTmp->isA(TCtx)) {
    // We don't have to incref here;
    auto const ctx = srcLoc(env, inst, 2).reg();
    v << store{ctx, ar + AROFF(m_thisUnsafe)};
  } else {
    always_assert(ctxTmp->isA(TNullptr));
    // No $this or class; this happens in FPushFunc.
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitImmStoreq(v, ActRec::kTrashedThisSlot, ar + AROFF(m_thisUnsafe));
    }
  }

  // Set m_invName.
  if (extra->invName) {
    auto const invName = reinterpret_cast<uintptr_t>(extra->invName);
    emitImmStoreq(v, invName, ar + AROFF(m_invName));
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, ar + AROFF(m_invName));
  }

  // Set m_func.
  if (!funcTmp->isA(TNullptr)) {
    auto const func = srcLoc(env, inst, 1).reg(0);
    v << store{func, ar + AROFF(m_func)};
  }

  auto const caller = inst->marker().func();
  auto flags = !caller->isBuiltin() && !caller->unit()->useStrictTypes()
    ? ActRec::Flags::UseWeakTypes
    : ActRec::Flags::None;
  if (extra->invName) {
    flags = static_cast<ActRec::Flags>(flags | ActRec::Flags::MagicDispatch);
  }
  auto const naaf = static_cast<int32_t>(
    ActRec::encodeNumArgsAndFlags(extra->numArgs, flags)
  );
  v << storeli{naaf, ar + AROFF(m_numArgsAndFlags)};
}

void cgCufIterSpillFrame(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 1).reg();
  auto const extra = inst->extra<CufIterSpillFrame>();
  auto const iterOff = iterOffset(inst->marker(), extra->iterId);

  auto& v = vmain(env);

  auto const ar = sp[cellsToBytes(extra->spOffset.offset)];

  auto const func = v.makeReg();
  v << load{fp[iterOff + CufIter::funcOff()], func};
  v << store{func, ar + AROFF(m_func)};

  auto const ctx = v.makeReg();
  v << load{fp[iterOff + CufIter::ctxOff()], ctx};
  v << store{ctx, ar + AROFF(m_thisUnsafe)};

  { // Incref m_this, if it's indeed a $this (rather than a class context).
    auto const sf = v.makeReg();
    auto const shifted = v.makeReg();
    v << shrqi{1, ctx, shifted, sf};
    ifThen(v, CC_NBE, sf, [&](Vout& v) {
      auto const rthis = v.makeReg();
      v << shlqi{1, shifted, rthis, v.makeReg()};
      emitIncRef(v, rthis);
    });
  }

  auto const caller = inst->marker().func();
  auto const flags = !caller->isBuiltin() && !caller->unit()->useStrictTypes()
    ? ActRec::Flags::UseWeakTypes
    : ActRec::Flags::None;

  auto const name = v.makeReg();
  auto const sf = v.makeReg();
  v << load{fp[iterOff + CufIter::nameOff()], name};
  v << store{name, ar + AROFF(m_invName)};
  v << testq{name, name, sf};

  ifThenElse(v, CC_NZ, sf,
    [&] (Vout& v) {
      static_assert(UncountedValue < 0 && StaticValue < 0, "");

      // Incref m_invName if it's non-persistent.
      auto const sf = v.makeReg();
      v << cmplim{0, name[FAST_REFCOUNT_OFFSET], sf};
      ifThen(v, CC_GE, sf, [&] (Vout& v) { emitIncRef(v, name); });

      auto const naaf = static_cast<int32_t>(
        ActRec::encodeNumArgsAndFlags(
          safe_cast<int32_t>(extra->args),
          static_cast<ActRec::Flags>(ActRec::Flags::MagicDispatch | flags)
        )
      );
      v << storeli{naaf, ar + AROFF(m_numArgsAndFlags)};
    },
    [&] (Vout& v) {
      auto const naaf = static_cast<int32_t>(
        ActRec::encodeNumArgsAndFlags(
          safe_cast<int32_t>(extra->args),
          flags
        )
      );
      v << storeli{naaf, ar + AROFF(m_numArgsAndFlags)};
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

void cgLdARFuncPtr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdARFuncPtr>()->offset.offset);
  vmain(env) << load{sp[off + AROFF(m_func)], dst};
}

void cgLdARNumArgsAndFlags(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{fp[AROFF(m_numArgsAndFlags)], dst};
}

void cgStARNumArgsAndFlags(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const val = srcLoc(env, inst, 1).reg();
  auto &v = vmain(env);

  auto const tmp = v.makeReg();
  v << movtql{val, tmp};
  v << storel{tmp, fp[AROFF(m_numArgsAndFlags)]};
}

void cgLdARNumParams(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const naaf = v.makeReg();
  v << loadzlq{fp[AROFF(m_numArgsAndFlags)], naaf};
  v << andqi{ActRec::kNumArgsMask, naaf, dst, v.makeReg()};
}

void cgCheckARMagicFlag(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const sf = v.makeReg();

  auto const mask = static_cast<int32_t>(ActRec::Flags::MagicDispatch);

  if (mask & (mask - 1)) {
    auto const tmp = v.makeReg();
    auto const naaf = v.makeReg();
    // We need to test multiple bits.
    v << loadl{fp[AROFF(m_numArgsAndFlags)], naaf};
    v << andli{mask, naaf, tmp, v.makeReg()};
    v << cmpli{mask, tmp, sf};
    v << jcc{CC_NZ, sf, {label(env, inst->next()), label(env, inst->taken())}};
  } else {
    v << testlim{mask, fp[AROFF(m_numArgsAndFlags)], sf};
    v << jcc{CC_Z, sf, {label(env, inst->next()), label(env, inst->taken())}};
  }
}

void cgLdCtx(IRLS& env, const IRInstruction* inst) {
  assertx(!inst->func() || inst->ctx());
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << load{fp[AROFF(m_thisUnsafe)], dst};
}

void cgLdCctx(IRLS& env, const IRInstruction* inst) {
  return cgLdCtx(env, inst);
}

void cgInitCtx(IRLS& env, const IRInstruction* inst) {
  assertx(!inst->func() || inst->func()->isClosureBody());
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const ctx = srcLoc(env, inst, 1).reg();
  vmain(env) << store{ctx, fp[AROFF(m_thisUnsafe)]};
}

void cgLdARInvName(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << load{fp[AROFF(m_invName)], dst};
}

void cgStARInvName(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const val = srcLoc(env, inst, 1).reg();
  vmain(env) << store{val, fp[AROFF(m_invName)]};
}

void cgInitExtraArgs(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<InitExtraArgs>();
  auto const func = extra->func;
  auto const argc = extra->argc;

  using Action = ExtraArgsAction;

  auto& v = vmain(env);
  void (*handler)(ActRec*) = nullptr;

  switch (extra_args_action(func, argc)) {
    case Action::None:
      if (func->attrs() & AttrMayUseVV) {
        v << storeqi{0, fp[AROFF(m_invName)]};
      }
      return;

    case Action::Discard:
      handler = trimExtraArgs;
      break;
    case Action::Variadic:
      handler = shuffleExtraArgsVariadic;
      break;
    case Action::MayUseVV:
      handler = shuffleExtraArgsMayUseVV;
      break;
    case Action::VarAndVV:
      handler = shuffleExtraArgsVariadicAndVV;
      break;
  }

  v << vcall{
    CallSpec::direct(handler),
    v.makeVcallArgs({{fp}}),
    v.makeTuple({})
  };
}

void cgPackMagicArgs(IRLS& env, const IRInstruction* inst) {
  auto const fp = srcLoc(env, inst, 0).reg();

  auto& v = vmain(env);
  auto const naaf = v.makeReg();
  auto const num_args = v.makeReg();

  v << loadl{fp[AROFF(m_numArgsAndFlags)], naaf};
  v << andli{ActRec::kNumArgsMask, naaf, num_args, v.makeReg()};

  auto const offset = v.makeReg();
  auto const offsetq = v.makeReg();
  auto const values = v.makeReg();

  static_assert(sizeof(Cell) == 16, "");
  v << shlli{4, num_args, offset, v.makeReg()};
  v << movzlq{offset, offsetq};
  v << subq{offsetq, fp, values, v.makeReg()};

  auto const args = argGroup(env, inst)
    .reg(num_args)
    .reg(values);

  cgCallHelper(v, env, CallSpec::direct(PackedArray::MakePacked),
               callDest(env, inst), SyncOptions::Sync, args);
}

///////////////////////////////////////////////////////////////////////////////

}}}
