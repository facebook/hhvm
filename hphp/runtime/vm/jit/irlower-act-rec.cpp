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

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/countable.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/tv-mutate.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

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

void cgSpillFrame(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<SpillFrame>();
  auto const funcTmp = inst->src(1);
  auto const ctxTmp = inst->src(2);
  auto const invNameTmp = inst->src(3);
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
  } else if (ctxTmp->isA(TNullptr)) {
    // No $this or class; this happens in FPushFunc.
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitImmStoreq(v, ActRec::kTrashedThisSlot, ar + AROFF(m_thisUnsafe));
    }
  } else {
    // It could be TCls | TCtx | TNullptr, but we can't distinguish TCls and
    // TCtx so assert it doesn't happen. We don't generate SpillFrames with such
    // input types.
    assertx(ctxTmp->isA(TCtx | TNullptr));

    // We don't have to incref here
    auto const ctx = srcLoc(env, inst, 2).reg();
    v << store{ctx, ar + AROFF(m_thisUnsafe)};
    if (RuntimeOption::EvalHHIRGenerateAsserts &&
        ctxTmp->type().maybe(TNullptr)) {
      auto const sf = v.makeReg();
      v << testq{ctx, ctx, sf};
      ifThen(
        v,
        CC_Z,
        sf,
        [&] (Vout& v) {
          emitImmStoreq(v, ActRec::kTrashedThisSlot, ar + AROFF(m_thisUnsafe));
        }
      );
    }
  }

  auto const caller = inst->marker().func();
  auto const baseFlags =
    caller->isBuiltin() || !caller->unit()->useStrictTypes()
      ? ActRec::Flags::UseWeakTypes
      : ActRec::Flags::None;
  auto const naaf = static_cast<int32_t>(
    ActRec::encodeNumArgsAndFlags(extra->numArgs, baseFlags)
  );
  auto const naafMagic = static_cast<int32_t>(
    ActRec::encodeNumArgsAndFlags(
      extra->numArgs,
      static_cast<ActRec::Flags>(baseFlags | ActRec::Flags::MagicDispatch)
    )
  );

  // Set m_invName.
  if (invNameTmp->isA(TNullptr)) {
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, ar + AROFF(m_invName));
    }
    v << storeli{naaf, ar + AROFF(m_numArgsAndFlags)};
  } else {
    assertx(invNameTmp->isA(TStr | TNullptr));

    // We don't have to incref here
    auto const invName = srcLoc(env, inst, 3).reg();
    v << store{invName, ar + AROFF(m_invName)};
    if (!invNameTmp->type().maybe(TNullptr)) {
      v << storeli{naafMagic, ar + AROFF(m_numArgsAndFlags)};
    } else {
      auto const sf = v.makeReg();
      auto const naafReg = v.makeReg();
      v << testq{invName, invName, sf};
      v << cmovl{
        CC_Z,
        sf,
        v.cns(static_cast<uint32_t>(naafMagic)),
        v.cns(static_cast<uint32_t>(naaf)),
        naafReg
      };
      v << storel{naafReg, ar + AROFF(m_numArgsAndFlags)};
      if (RuntimeOption::EvalHHIRGenerateAsserts) {
        ifThen(
          v,
          CC_Z,
          sf,
          [&] (Vout& v) {
            emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, ar + AROFF(m_invName));
          }
        );
      }
    }
  }

  // Set m_func.
  if (funcTmp->isA(TNullptr)) {
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      emitImmStoreq(v, ActRec::kTrashedFuncSlot, ar + AROFF(m_func));
    }
  } else {
    assertx(funcTmp->isA(TFunc | TNullptr));
    auto const func = srcLoc(env, inst, 1).reg();
    v << store{func, ar + AROFF(m_func)};
    if (RuntimeOption::EvalHHIRGenerateAsserts &&
        funcTmp->type().maybe(TNullptr)) {
      auto const sf = v.makeReg();
      v << testq{func, func, sf};
      ifThen(
        v,
        CC_Z,
        sf,
        [&] (Vout& v) {
          emitImmStoreq(v, ActRec::kTrashedFuncSlot, ar + AROFF(m_func));
        }
      );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void cgLdARFuncPtr(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdARFuncPtr>()->offset.offset);
  vmain(env) << load{sp[off + AROFF(m_func)], dst};
}

void cgLdARCtx(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdARCtx>()->offset.offset);
  vmain(env) << load{sp[off + AROFF(m_thisUnsafe)], dst};
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

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * The standard VMRegAnchor treatment won't work for some cases called during
 * function prologues.
 *
 * The fp sync machinery is fundamentally based on the notion that instruction
 * pointers in the TC are uniquely associated with source HHBC instructions,
 * and that source HHBC instructions are in turn uniquely associated with
 * SP->FP deltas.
 *
 * trimExtraArgs() is called from the prologue of the callee.  The prologue is
 * (a) still in the caller frame for now, and (b) shared across multiple call
 * sites.  (a) means that we have the fp from the caller's frame, and (b) means
 * that this fp is not enough to figure out sp.
 *
 * However, the prologue passes us the callee ActRec, whose predecessor has to
 * be the caller.  So we can sync sp and fp by ourselves here.  Geronimo!
 */
static void sync_regstate_to_caller(ActRec* preLive) {
  assertx(tl_regState == VMRegState::DIRTY);
  auto const ec = g_context.getNoCheck();
  auto& regs = vmRegsUnsafe();

  regs.stack.top() = reinterpret_cast<TypedValue*>(preLive)
                     - preLive->numArgs();
  auto fp = preLive == vmFirstAR()
    ? ec->m_nestedVMs.back().fp
    : preLive->m_sfp;
  regs.fp = fp;
  regs.pc = fp->func()->unit()->at(fp->func()->base() + preLive->m_soff);

  tl_regState = VMRegState::CLEAN;
}

NEVER_INLINE
static void trimExtraArgsMayReenter(ActRec* ar,
                                    TypedValue* tvArgs,
                                    TypedValue* limit) {
  sync_regstate_to_caller(ar);
  do {
    tvDecRefGen(tvArgs); // may reenter for __destruct
    ++tvArgs;
  } while (tvArgs != limit);
  ar->setNumArgs(ar->m_func->numParams());

  // Go back to dirty (see the comments of sync_regstate_to_caller()).
  tl_regState = VMRegState::DIRTY;
}

}

#define SHUFFLE_EXTRA_ARGS_PRELUDE()                                    \
  auto const f = ar->func();                                            \
  auto const numParams = f->numNonVariadicParams();                     \
  auto const numArgs = ar->numArgs();                                   \
  assertx(numArgs > numParams);                                         \
  auto const numExtra = numArgs - numParams;                            \
  TRACE(1, "extra args: %d args, function %s takes only %d, ar %p\n",   \
        numArgs, f->name()->data(), numParams, ar);                     \
  auto tvArgs = reinterpret_cast<TypedValue*>(ar) - numArgs;            \
  /* end SHUFFLE_EXTRA_ARGS_PRELUDE */

void trimExtraArgs(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  auto limit = tvArgs + numExtra;
  do {
    if (UNLIKELY(tvDecRefWillCallHelper(*tvArgs))) {
      trimExtraArgsMayReenter(ar, tvArgs, limit);
      return;
    }
    tvDecRefGenNZ(tvArgs);
    ++tvArgs;
  } while (tvArgs != limit);

  assertx(f->numParams() == (numArgs - numExtra));
  assertx(f->numParams() == numParams);
  ar->setNumArgs(numParams);
}

void shuffleExtraArgsMayUseVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));
}

void shuffleExtraArgsVariadic(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  auto varArgsArray = Array::attach(PackedArray::MakePacked(numExtra, tvArgs));
  // Write into the last (variadic) param.
  auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
  tv->m_type = KindOfArray;
  tv->m_data.parr = varArgsArray.detach();
  assertx(tv->m_data.parr->hasExactlyOneRef());

  // No incref is needed, since extra values are being transferred from the
  // stack to the last local.
  assertx(f->numParams() == (numArgs - numExtra + 1));
  assertx(f->numParams() == (numParams + 1));
  ar->setNumArgs(numParams + 1);
}

void shuffleExtraArgsVariadicAndVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));

  auto varArgsArray = Array::attach(PackedArray::MakePacked(numExtra, tvArgs));
  auto tvIncr = tvArgs;
  // An incref is needed to compensate for discarding from the stack.
  for (uint32_t i = 0; i < numExtra; ++i, ++tvIncr) {
    tvIncRefGen(*tvIncr);
  }
  // Write into the last (variadic) param.
  auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
  tv->m_type = KindOfArray;
  tv->m_data.parr = varArgsArray.detach();
  assertx(tv->m_data.parr->hasExactlyOneRef());
  // Before, for each arg: refcount = n + 1 (stack).
  // After, for each arg: refcount = n + 2 (ExtraArgs, varArgsArray).
}

#undef SHUFFLE_EXTRA_ARGS_PRELUDE

///////////////////////////////////////////////////////////////////////////////

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
