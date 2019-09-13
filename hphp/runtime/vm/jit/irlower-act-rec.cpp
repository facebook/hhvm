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

#include "hphp/runtime/base/array-init.h"
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
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/reified-generics.h"
#include "hphp/runtime/vm/runtime.h"
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

namespace {

uint32_t getTSBitsImpl(const ArrayData* tsList) {
  if (!tsList || tsList->size() > 15) return 0;
  auto bitmap = 1;
  IterateV(
    tsList,
    [&](TypedValue v) {
      assertx(isArrayLikeType(v.m_type));
      bitmap = (bitmap << 1) | !isWildCard(v.m_data.parr);
    }
  );
  return bitmap;
}

folly::Optional<Vreg> getTSBits(
  IRLS& env,
  const IRInstruction* inst,
  SSATmp* tsList,
  Vreg tsListReg
) {
  auto& v = vmain(env);
  auto const tsListType = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
  if (tsList->isA(TNullptr) || !tsList->type().maybe(tsListType)) {
    return folly::none;
  }
  if (tsList->hasConstVal(tsListType)) {
    auto const bitmap =
      getTSBitsImpl(RuntimeOption::EvalHackArrDVArrs ? tsList->vecVal()
                                                     : tsList->arrVal());
    return v.cns(bitmap);
  }

  auto const args = argGroup(env, inst).reg(tsListReg);
  auto const dest = v.makeReg();
  cgCallHelper(v, env, CallSpec::direct(getTSBitsImpl),
               callDest(dest), SyncOptions::None, args);
  return dest;
}

}

void cgSpillFrame(IRLS& env, const IRInstruction* inst) {
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const extra = inst->extra<SpillFrame>();
  auto const ctxTmp = inst->src(2);
  auto const tsListTmp = inst->src(3);
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
    // No $this or class; this happens in FCallFunc*.
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

  // Trash m_varEnv.
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedVarEnvSlot, ar + AROFF(m_varEnv));
  }

  // Set m_func.
  assertx(inst->src(1)->isA(TFunc));
  auto const func = srcLoc(env, inst, 1).reg();
  v << store{func, ar + AROFF(m_func)};

  // Set flags
  auto flags = ActRec::Flags::None;
  if (extra->dynamicCall) {
    flags = static_cast<ActRec::Flags>(flags | ActRec::Flags::DynamicCall);
  }

  auto const createCompactReifiedPtr = [&] (Vout& v) {
    auto const tsListReg = srcLoc(env, inst, 3).reg();
    auto const bits = getTSBits(env, inst, tsListTmp, tsListReg);
    if (!bits) return tsListReg;
    auto const bits_shifted = v.makeReg();
    auto const result = v.makeReg();
    v << shlqi{
      static_cast<int32_t>(ReifiedGenericsPtr::kShiftAmount),
      *bits,
      bits_shifted,
      v.makeReg()
    };
    v << orq{bits_shifted, tsListReg, result, v.makeReg()};
    return result;
  };

  bool reifiedCheck = false;
  if (!tsListTmp->type().maybe(TNullptr)) {
    auto const tsList = createCompactReifiedPtr(v);
    v << store{tsList, ar + AROFF(m_reifiedGenerics)};
    flags =
      static_cast<ActRec::Flags>(flags | ActRec::Flags::HasReifiedGenerics);
  } else if (!tsListTmp->isA(TNullptr)) {
    reifiedCheck = true;
  }

  auto naaf = v.cns(
    static_cast<int32_t>(ActRec::encodeNumArgsAndFlags(extra->numArgs, flags))
  );

  if (reifiedCheck) {
    auto const tsList = createCompactReifiedPtr(v);
    v << store{tsList, ar + AROFF(m_reifiedGenerics)};
    auto const sf = v.makeReg();
    auto const naaf2 = v.makeReg();
    auto const dst = v.makeReg();
    v << orli{
      static_cast<int32_t>(ActRec::Flags::HasReifiedGenerics),
      naaf,
      dst,
      v.makeReg()
    };
    v << testq{tsList, tsList, sf};
    v << cmovl{CC_NZ, sf, naaf, dst, naaf2};
    naaf = naaf2;
  }

  v << storel{naaf, ar + AROFF(m_numArgsAndFlags)};
}

///////////////////////////////////////////////////////////////////////////////

void cgLdARCtx(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const off = cellsToBytes(inst->extra<LdARCtx>()->offset.offset);
  vmain(env) << load{sp[off + AROFF(m_thisUnsafe)], dst};
}

void cgLdARReifiedGenerics(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const compactTSList = v.makeReg();
  auto const imm = v.cns(-1ull >> ReifiedGenericsPtr::kMaxTagSize);
  v << load{fp[AROFF(m_reifiedGenerics)], compactTSList};
  v << andq{imm, compactTSList, dst, v.makeReg()};
}

void cgIsFunReifiedGenericsMatched(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto const func = inst->extra<FuncData>()->func;
  auto& v = vmain(env);

  auto const info = func->getReifiedGenericsInfo();
  if (!func->hasReifiedGenerics() ||
      info.m_hasSoftGenerics ||
      info.m_typeParamInfo.size() > 15) {
    v << copy{v.cns(0), dst};
    return;
  }
  auto const sf = v.makeReg();
  // Higher order 16 bits contain the tag in a compact tagged pointer
  // Tag contains ((1 << number-of-parameters) | bitmap)
  auto const tagAddress = fp[AROFF(m_reifiedGenerics) + 6];
  auto const bitmapImmed = static_cast<int16_t>(info.m_bitmap);
  auto const topBit = 1u << info.m_typeParamInfo.size();

  if (bitmapImmed == topBit - 1) {
    v << cmpwim{static_cast<int16_t>(bitmapImmed | topBit), tagAddress, sf};
  } else {
    auto const tag = v.makeReg();
    auto const anded = v.makeReg();
    v << loadw{tagAddress, tag};
    v << andwi{static_cast<int16_t>(bitmapImmed | -topBit),
               tag, anded, v.makeReg()};
    v << cmpwi{static_cast<int16_t>(bitmapImmed | topBit), anded, sf};
  }
  v << setcc{CC_Z, sf, dst};
}

void cgKillARReifiedGenerics(IRLS& env, const IRInstruction* inst) {
  if (!debug) return;
  auto const fp = srcLoc(env, inst, 0).reg();
  emitImmStoreq(vmain(env), ActRec::kTrashedReifiedGenericsSlot,
    fp[AROFF(m_reifiedGenerics)]);
}

void cgLdARNumArgsAndFlags(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  vmain(env) << loadzlq{fp[AROFF(m_numArgsAndFlags)], dst};
}

void cgLdARNumParams(IRLS& env, const IRInstruction* inst) {
  auto const dst = dstLoc(env, inst, 0).reg();
  auto const fp = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  auto const naaf = v.makeReg();
  v << loadzlq{fp[AROFF(m_numArgsAndFlags)], naaf};
  v << andqi{ActRec::kNumArgsMask, naaf, dst, v.makeReg()};
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
  regs.pc = fp->func()->unit()->at(fp->func()->base() + preLive->m_callOff);
  regs.jitReturnAddr = (TCA)preLive->m_savedRip;

  tl_regState = VMRegState::CLEAN;
}

/*
 * Perform the action specified by 'action1' on the range of TypedValues
 * represented by 'tv' and 'limit'. If 'pred' ever returns true, sync the
 * register register state and then start calling 'action2' instead.
 */
template <typename Iter, typename Pred, typename Action1, typename Action2>
NEVER_INLINE
static void actionMayReenter(ActRec* ar,
                             Iter tv,
                             Iter limit,
                             Pred pred,
                             Action1 action1,
                             Action2 action2) {
  do {
    if (pred(*tv)) {
      sync_regstate_to_caller(ar);
      // Go back to dirty (see the comments of sync_regstate_to_caller()).
      SCOPE_EXIT { tl_regState = VMRegState::DIRTY; };
      do {
        action2(*tv);
        ++tv;
      } while (tv != limit);
      break;
    }
    action1(*tv);
    ++tv;
  } while (tv != limit);
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
  auto tsList = !ar->hasReifiedGenerics() ? nullptr                     \
                                          : ar->getReifiedGenerics();   \
  auto reifiedLocal = frame_local(ar, f->numParams()); // regular + var \
  /* end SHUFFLE_EXTRA_ARGS_PRELUDE */

void trimExtraArgs(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  actionMayReenter(
    ar,
    tvArgs,
    tvArgs + numExtra,
    [](TypedValue v){ return tvDecRefWillCallHelper(v); },
    [](TypedValue v){ tvDecRefGenNZ(v); },
    [](TypedValue v){ tvDecRefGen(v); }
  );

  assertx(f->numParams() == (numArgs - numExtra));
  assertx(f->numParams() == numParams);
  ar->setNumArgs(numParams);
  if (tsList) *reifiedLocal = make_array_like_tv(tsList);
}

void shuffleExtraArgsMayUseVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(!f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));
  if (tsList) *reifiedLocal = make_array_like_tv(tsList);
}

void shuffleExtraArgsVariadic(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(!(f->attrs() & AttrMayUseVV));

  VArrayInit ai{numExtra};
  actionMayReenter(
    ar,
    std::reverse_iterator<TypedValue*>(tvArgs + numExtra),
    std::reverse_iterator<TypedValue*>(tvArgs),
    [](TypedValue v)  { return isRefType(v.m_type); },
    [&](TypedValue v) { ai.appendWithRef(v); },
    [&](TypedValue v) { ai.appendWithRef(v); }
  );
  actionMayReenter(
    ar,
    tvArgs,
    tvArgs + numExtra,
    /* If the value wasn't a ref, we'll have definitely inc-reffed it, so we
     * won't re-enter. */
    [](TypedValue v){ return isRefType(v.m_type); },
    [](TypedValue v){ tvDecRefGenNZ(v); },
    [](TypedValue v){ tvDecRefGen(v); }
  );

  // Write into the last (variadic) param.
  auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
  *tv = make_array_like_tv(ai.create());
  assertx(tv->m_data.parr->hasExactlyOneRef());

  // No incref is needed, since extra values are being transferred from the
  // stack to the last local.
  assertx(f->numParams() == (numArgs - numExtra + 1));
  assertx(f->numParams() == (numParams + 1));
  ar->setNumArgs(numParams + 1);
  if (tsList) *reifiedLocal = make_array_like_tv(tsList);
}

void shuffleExtraArgsVariadicAndVV(ActRec* ar) {
  SHUFFLE_EXTRA_ARGS_PRELUDE()
  assertx(f->hasVariadicCaptureParam());
  assertx(f->attrs() & AttrMayUseVV);

  ar->setExtraArgs(ExtraArgs::allocateCopy(tvArgs, numExtra));
  try {
    VArrayInit ai{numExtra};
    actionMayReenter(
      ar,
      std::reverse_iterator<TypedValue*>(tvArgs + numExtra),
      std::reverse_iterator<TypedValue*>(tvArgs),
      [](TypedValue v)  { return isRefType(v.m_type); },
      [&](TypedValue v) { ai.appendWithRef(v); },
      [&](TypedValue v) { ai.appendWithRef(v); }
    );
    // Write into the last (variadic) param.
    auto tv = reinterpret_cast<TypedValue*>(ar) - numParams - 1;
    *tv = make_array_like_tv(ai.create());
    assertx(tv->m_data.parr->hasExactlyOneRef());
    // Before, for each arg: refcount = n + 1 (stack).
    // After, for each arg: refcount = n + 2 (ExtraArgs, varArgsArray).
  } catch (...) {
    ExtraArgs::deallocateRaw(ar->getExtraArgs());
    ar->resetExtraArgs();
    throw;
  }
  if (tsList) *reifiedLocal = make_array_like_tv(tsList);
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

  cgCallHelper(
    v,
    env,
    CallSpec::direct(handler),
    callDest(env, inst),
    SyncOptions::Sync,
    argGroup(env, inst).reg(fp)
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
