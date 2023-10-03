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

#include "hphp/runtime/vm/jit/irgen-func-prologue.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/ext/asio/ext_resumable-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/reified-generics-info.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-control.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/irgen-types.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/text-util.h"

namespace HPHP::jit::irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * How to perform our stack overflow check.
 */
enum class StackCheck {
  None,   // not needed
  Early,  // must occur before setting up locals
  Combine // can be delayed and combined with surprise flags check
};

StackCheck stack_check_kind(const Func* func) {
  if (func->isPhpLeafFn() &&
      func->maxStackCells() < RO::EvalStackCheckLeafPadding) {
    return StackCheck::None;
  }

  /*
   * Determine how many stack slots we're going to write that the caller hasn't
   * already checked we have space for.
   *
   * We don't need to worry about any of the passed parameter locals, because
   * the caller must have checked for that in its maxStackCells().  However,
   * we'd like to delay our stack overflow check until after we've entered our
   * frame, so we can combine it with the surprise flag check (which must run
   * after we've created the callee).
   *
   * The only things we are going to do is write uninits to the non-passed
   * params and to the non-parameter locals, and possibly shuffle some of the
   * locals into the variadic capture param.  The uninits are harmless to the
   * stack overflow code as long as we know we aren't going to segfault while
   * we write them.
   *
   * There's always sSurprisePageSize extra space at the bottom (lowest
   * addresses) of the eval stack, so we just only do this optimization if
   * we're sure we're going to write few enough uninits that we would be
   * staying within that region if the locals are actually too deep.
   */
  auto const safeFromSEGV = Stack::sSurprisePageSize / sizeof(TypedValue);

  return func->numLocals() < safeFromSEGV + func->numRequiredParams()
    ? StackCheck::Combine
    : StackCheck::Early;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

void emitCalleeGenericsChecks(IRGS& env, const Func* callee,
                              SSATmp* prologueFlags, bool pushed) {
  if (!callee->hasReifiedGenerics()) {
    // FIXME: leaks memory if generics were given but not expected nor pushed.
    if (pushed) {
      popDecRef(env);
      updateMarker(env);
      env.irb->exceptionStackBoundary();
    }
    return;
  }

  // Fail if generics were not passed.
  ifThenElse(
    env,
    [&] (Block* taken) {
      if (pushed) return;
      auto constexpr flag = 1 << PrologueFlags::Flags::HasGenerics;
      auto const hasGenerics = gen(env, AndInt, prologueFlags, cns(env, flag));
      gen(env, JmpZero, taken, hasGenerics);
    },
    [&] {
      // Generics were passed. Make them visible on the stack.
      auto const generics = pushed ? topC(env) : apparate(env, TVec);
      updateMarker(env);
      env.irb->exceptionStackBoundary();

      // Generics may be known if we are inlining.
      if (generics->hasConstVal(TVec)) {
        auto const genericsArr = generics->arrLikeVal();
        auto const& genericsDef =
          callee->getReifiedGenericsInfo().m_typeParamInfo;
        if (genericsArr->size() == genericsDef.size()) {
          bool match = true;
          IterateKV(genericsArr, [&](TypedValue k, TypedValue v) {
            assertx(tvIsInt(k) && tvIsArrayLike(v));
            auto const idx = k.m_data.num;
            auto const ts = v.m_data.parr;
            if (isWildCard(ts) && genericsDef[idx].m_isReified) {
              match = false;
              return true;
            }
            return false;
          });
          if (match) return;
        }
      }

      // Fail on generics count/wildcard mismatch.
      ifThen(
        env,
        [&] (Block* taken) {
          auto const match = gen(
            env, IsFunReifiedGenericsMatched, FuncData{callee}, prologueFlags);
          gen(env, JmpZero, taken, match);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          gen(env, CheckFunReifiedGenericMismatch, cns(env, callee), generics);
        }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      // FIXME: ifThenElse() doesn't save/restore marker and stack boundary.
      updateMarker(env);
      env.irb->exceptionStackBoundary();

      if (pushed) {
        gen(env, Unreachable, ASSERT_REASON);
        return;
      }

      // Generics not given. We will either fail or raise a warning.
      if (!areAllGenericsSoft(callee->getReifiedGenericsInfo())) {
        gen(env, ThrowCallReifiedFunctionWithoutGenerics, cns(env, callee));
        return;
      }

      auto const errMsg = makeStaticString(folly::sformat(
        "Generic at index 0 to Function {} must be reified, erased given",
        callee->fullName()->data()));
      gen(env, RaiseWarning, cns(env, errMsg));

      // Push an empty array, as the remainder of the prologue assumes generics
      // are on the stack.
      push(env, cns(env, ArrayData::CreateVec()));
      updateMarker(env);
      env.irb->exceptionStackBoundary();
    }
  );

  // FIXME: ifThenElse() doesn't save/restore marker and stack boundary.
  updateMarker(env);
  env.irb->exceptionStackBoundary();
}

/*
 * Check for too few or too many arguments and trim extra args.
 */
void emitCalleeArgumentArityChecks(IRGS& env, const Func* callee,
                                   uint32_t& argc) {
  if (argc < callee->numRequiredParams()) {
    gen(env, ThrowMissingArg, FuncArgData { callee, argc });
  }

  if (argc > callee->numParams()) {
    assertx(!callee->hasVariadicCaptureParam());
    assertx(argc == callee->numNonVariadicParams() + 1);
    --argc;

    // Pop unpack args, skipping generics (we already know their type).
    auto const generics = callee->hasReifiedGenerics()
      ? popC(env, DataTypeGeneric) : nullptr;
    auto const unpackArgs = pop(env, DataTypeGeneric);
    if (generics != nullptr) push(env, generics);

    // We have updated the stack.
    updateMarker(env);
    env.irb->exceptionStackBoundary();

    // Pass unpack args to the raiseTooManyArgumentsPrologue() helper, which
    // will use them to report the correct number and also take care of decref.
    auto const unpackArgsArr = gen(env, AssertType, TVec, unpackArgs);
    gen(env, RaiseTooManyArg, FuncData { callee }, unpackArgsArr);
  }
}

void emitCalleeArgumentTypeChecks(IRGS& env, const Func* callee,
                                  uint32_t argc, SSATmp* prologueCtx) {
  // Builtins use a separate non-standard mechanism.
  if (callee->isCPPBuiltin()) return;

  auto const numArgs = std::min(argc, callee->numNonVariadicParams());
  auto const firstArgIdx = argc - 1 + (callee->hasReifiedGenerics() ? 1 : 0);
  for (auto i = 0; i < numArgs; ++i) {
    auto const offset = BCSPRelOffset { safe_cast<int32_t>(firstArgIdx - i) };
    auto const irsproData = IRSPRelOffsetData { offsetFromIRSP(env, offset ) };
    gen(env, AssertStk, TInitCell, irsproData, sp(env));
    verifyParamType(env, callee, i, offset, prologueCtx);
  }
}

void emitCalleeDynamicCallChecks(IRGS& env, const Func* callee,
                                 SSATmp* prologueFlags) {
  if (!RuntimeOption::EvalNoticeOnBuiltinDynamicCalls || !callee->isBuiltin()) {
    return;
  }

  ifThen(
    env,
    [&] (Block* taken) {
      auto constexpr flag = 1 << PrologueFlags::Flags::IsDynamicCall;
      auto const isDynCall = gen(env, AndInt, prologueFlags, cns(env, flag));
      gen(env, JmpNZero, taken, isDynCall);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      std::string errMsg;
      auto const fmtString = callee->isDynamicallyCallable()
        ? Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE
        : Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;
      string_printf(errMsg, fmtString, callee->fullName()->data());
      gen(env, RaiseNotice, SampleRateData {}, cns(env, makeStaticString(errMsg)));
    }
  );
}

void emitCalleeCoeffectChecks(IRGS& env, const Func* callee,
                              SSATmp* prologueFlags, SSATmp* providedCoeffects,
                              bool skipCoeffectsCheck,
                              uint32_t argc, SSATmp* prologueCtx) {
  assertx(callee);
  assertx(prologueFlags);

  if (!CoeffectsConfig::enabled()) {
    if (callee->hasCoeffectsLocal()) {
      push(env, cns(env, RuntimeCoeffects::none().value()));
      updateMarker(env);
      env.irb->exceptionStackBoundary();
    }
    return;
  }

  // If ambient coeffects are directly provided use them, otherwise extract
  // them from prologueFlags
  if (!providedCoeffects) {
    providedCoeffects =
      gen(env, Lshr, prologueFlags, cns(env, PrologueFlags::CoeffectsStart));
  }

  if (skipCoeffectsCheck) {
    if (callee->hasCoeffectsLocal()) {
      push(env, providedCoeffects);
      updateMarker(env);
      env.irb->exceptionStackBoundary();
    }
    return;
  }

  auto const requiredCoeffects = [&] {
    auto required = cns(env, callee->requiredCoeffects().value());
    if (!callee->hasCoeffectRules()) return required;
    for (auto const& rule : callee->getCoeffectRules()) {
      if (auto const coeffect = rule.emitJit(env, callee, argc, prologueCtx,
                                             providedCoeffects)) {
        required = gen(env, OrInt, required, coeffect);
      }
    }
    if (callee->hasCoeffectsLocal()) {
      push(env, required);
      updateMarker(env);
      env.irb->exceptionStackBoundary();
    }
    return required;
  }();

  ifThen(
    env,
    [&] (Block* taken) {
      // (~providedCoeffects) & requiredCoeffects == 0

      // The unused higher order bits of providedCoeffects will be 0
      // We want to flip the used lower order bits
      auto const mask = (1 << CoeffectsConfig::numUsedBits()) - 1;
      auto const providedCoeffectsFlipped =
        gen(env, XorInt, providedCoeffects, cns(env, mask));

      auto const cond =
        gen(env, AndInt, providedCoeffectsFlipped, requiredCoeffects);
      gen(env, JmpNZero, taken, cond);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseCoeffectsCallViolation, FuncData{callee},
          providedCoeffects, requiredCoeffects);
    }
  );
}

void emitCalleeRecordFuncCoverage(IRGS& env, const Func* callee) {
  if (RO::RepoAuthoritative || !RO::EvalEnableFuncCoverage) return;
  if (callee->isNoInjection() || callee->isMethCaller()) return;

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckFuncNeedsCoverage, FuncData{callee}, taken);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RecordFuncCall, FuncData{callee});
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void emitPrologueEntry(IRGS& env, const Func* callee, uint32_t argc,
                       TransID transID) {
  gen(env, EnterPrologue);

  // Update marker with the stublogue bit.
  updateMarker(env);

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Make sure we are at the right function.
    auto const calleeSSA = gen(env, DefFuncPrologueCallee);
    auto const calleeOK = gen(env, EqFunc, calleeSSA, cns(env, callee));
    gen(env, JmpZero, makeUnreachable(env, ASSERT_REASON), calleeOK);

    // Make sure we are at the right prologue.
    auto const numArgs = gen(env, DefFuncPrologueNumArgs);
    auto const numArgsOK = gen(env, EqInt, numArgs, cns(env, argc));
    gen(env, JmpZero, makeUnreachable(env, ASSERT_REASON), numArgsOK);
  }

  // Emit debug code.
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto msg = RBMsgData { Trace::RBTypeFuncPrologue, callee->fullName() };
    gen(env, RBTraceMsg, msg);
  }

  // Increment profiling counter.
  if (isProfiling(env.context.kind)) {
    gen(env, IncProfCounter, TransIDData{transID});
    profData()->setProfiling(callee);
  }
}

void emitCalleeChecks(IRGS& env, const Func* callee, uint32_t& argc,
                      SSATmp* prologueFlags, SSATmp* prologueCtx) {
  // Generics are special and need to be checked first, as they may or may not
  // be on the stack. This check makes sure they materialize on the stack
  // if we expect them.
  emitCalleeGenericsChecks(env, callee, prologueFlags, false);
  emitCalleeArgumentArityChecks(env, callee, argc);
  emitCalleeArgumentTypeChecks(env, callee, argc, prologueCtx);
  emitCalleeDynamicCallChecks(env, callee, prologueFlags);
  emitCalleeCoeffectChecks(env, callee, prologueFlags, nullptr, false,
                           argc, prologueCtx);
  emitCalleeRecordFuncCoverage(env, callee);

  // Emit early stack overflow check if necessary.
  if (stack_check_kind(callee) == StackCheck::Early) {
    gen(env, CheckStackOverflow, sp(env));
  }
}

} // namespace

void emitInitFuncInputs(IRGS& env, const Func* callee, uint32_t argc) {
  assertx(argc <= callee->numParams());
  if (argc == callee->numParams()) return;

  // Generics and coeffects are already initialized
  auto const coeffects = callee->hasCoeffectsLocal()
    ? popC(env, DataTypeGeneric) : nullptr;
  auto const generics = callee->hasReifiedGenerics()
    ? popC(env, DataTypeGeneric) : nullptr;

  // Push Uninit for un-passed arguments.
  auto const numParams = callee->numNonVariadicParams();
  if (argc < numParams) {
    auto const kMaxArgsInitUnroll = 10;
    auto const count = numParams - argc;
    if (count <= kMaxArgsInitUnroll) {
      while (argc < numParams) {
        push(env, cns(env, TUninit));
        ++argc;
      }
    } else {
      pushMany(env, cns(env, TUninit), count);
      argc = numParams;
    }
  }

  if (argc < callee->numParams()) {
    // Push an empty array for `...$args'.
    assertx(callee->hasVariadicCaptureParam());
    push(env, cns(env, ArrayData::CreateVec()));
    ++argc;
  }

  assertx(argc == callee->numParams());

  // Place generics and coeffects in the correct position.
  if (generics != nullptr) push(env, generics);
  if (coeffects != nullptr) push(env, coeffects);

  updateMarker(env);
  env.irb->exceptionStackBoundary();
}

namespace {

std::tuple<SSATmp*, SSATmp*> emitPrologueExit(IRGS& env, const Func* callee,
                                              SSATmp* prologueFlags) {
  auto const arFlags = gen(env, ConvFuncPrologueFlagsToARFlags, prologueFlags);
  auto const calleeId = cns(env, callee->getFuncId().toInt());
  gen(env, ExitPrologue);
  return std::make_tuple(arFlags, calleeId);
}

void emitJmpFuncBody(IRGS& env, const Func* callee, uint32_t argc,
                     SSATmp* callerFP, SSATmp* arFlags, SSATmp* calleeId,
                     SSATmp* ctx) {

  // Emit the bindjmp for the function body.
  auto const numArgs = std::min(argc, callee->numNonVariadicParams());
  gen(
    env,
    ReqBindJmp,
    ReqBindJmpData {
      SrcKey { callee, numArgs, SrcKey::FuncEntryTag {} },
      SBInvOffset { 0 },
      spOffBCFromIRSP(env),
      false /* popFrame */
    },
    sp(env),
    callerFP,
    arFlags,
    calleeId,
    ctx
  );
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

namespace {

void definePrologueFrameAndStack(IRGS& env, const Func* callee, uint32_t argc) {
  // Define caller's frame. It is unknown if/where it lives on the stack.
  gen(env, DefFP, DefFPData { std::nullopt });
  updateMarker(env);

  // The stack base of prologues points to the stack without the potentially
  // uninitialized space reserved for ActRec and inouts. The rvmsp() register
  // points to the future ActRec. The stack contains additional `argc' inputs
  // below the ActRec.
  auto const cells = callee->numInOutParamsForArgs(argc) + kNumActRecCells;
  auto const irSPOff = SBInvOffset { safe_cast<int32_t>(cells) };
  auto const bcSPOff = SBInvOffset { safe_cast<int32_t>(cells + argc) };
  gen(env, DefRegSP, DefStackData { irSPOff, bcSPOff });

  // Now that the stack is initialized, update the BC marker and perform
  // initial sync of the exception stack boundary.
  updateMarker(env);
  env.irb->exceptionStackBoundary();
}

} // namespace

void emitFuncPrologue(IRGS& env, const Func* callee, uint32_t argc,
                      TransID transID) {
  assertx(argc <= callee->numNonVariadicParams() + 1);

  definePrologueFrameAndStack(env, callee, argc);

  // Define register inputs before doing anything else that may clobber them.
  auto const prologueFlags = gen(env, DefFuncPrologueFlags);
  auto const prologueCtx = (callee->isClosureBody() || callee->cls())
    ? gen(env, DefFuncPrologueCtx, callCtxType(callee))
    : cns(env, nullptr);

  emitPrologueEntry(env, callee, argc, transID);
  emitCalleeChecks(env, callee, argc, prologueFlags, prologueCtx);
  emitInitFuncInputs(env, callee, argc);
  auto [arFlags, calleeId] = emitPrologueExit(env, callee, prologueFlags);
  emitJmpFuncBody(env, callee, argc, fp(env), arFlags, calleeId, prologueCtx);
}

namespace {

void emitInitScalarDefaultParamLocal(IRGS& env, const Func* callee,
                                     uint32_t param) {
  auto const dv = callee->params()[param].defaultValue;
  gen(env, StLoc, LocalId{param}, fp(env), cns(env, dv));
}

void emitInitLocalRange(IRGS& env, const Func* callee,
                        uint32_t from, uint32_t to) {
  assertx(from <= to);
  /*
   * Maximum number of local initializations to unroll.
   *
   * The actual crossover point in terms of code size is 6 (just like for the
   * params init unroll limit); 9 was determined by experiment to be the
   * optimal point in certain benchmarks.
   *
   * We don't limit the count in optimized translations, as we expect these
   * stores to be elided.
   */
  auto constexpr kMaxLocalsInitUnroll = 9;

  assertx(from <= to);

  // Set all remaining uninitialized locals to Uninit.
  if (env.context.kind == TransKind::Optimize ||
      to - from <= kMaxLocalsInitUnroll) {
    for (auto i = from; i < to; ++i) {
      gen(env, StLoc, LocalId{i}, fp(env), cns(env, TUninit));
    }
  } else {
    auto const range = LocalIdRange{from, to};
    gen(env, StLocRange, range, fp(env), cns(env, TUninit));
  }
}

void emitInitDefaultParamLocals(IRGS& env, const Func* callee, uint32_t argc) {
  assertx(argc <= callee->numNonVariadicParams());

  // We are done. If there were variadics, they were already initialized.
  if (argc == callee->numNonVariadicParams()) return;

  // Set locals of parameters with default values to Uninit. In optimized
  // translations, these locals will be overwritten later, so these stores
  // will be optimized away.
  emitInitLocalRange(env, callee, argc, callee->numNonVariadicParams());
}

/*
 * Unpack closure use variables into locals.
 */
void emitInitClosureLocals(IRGS& env, const Func* callee) {
  if (!callee->isClosureBody()) return;

  auto const closureTy = Type::ExactObj(callee->implCls());
  auto const closure = gen(env, LdFrameThis, closureTy, fp(env));

  auto const ctx = [&] {
    if (!callee->cls()) return cns(env, nullptr);
    if (callee->isStatic()) {
      return gen(env, LdClosureCls, Type::SubCls(callee->cls()), closure);
    }
    auto const closureThis =
      gen(env, LdClosureThis, Type::SubObj(callee->cls()), closure);
    gen(env, IncRef, closureThis);
    return closureThis;
  }();
  if (!(ctx->type() <= TNullptr)) gen(env, StFrameCtx, fp(env), ctx);

  // Push the closure's use variables (stored in closure object properties).
  auto const firstClosureUseLocal = callee->firstClosureUseLocalId();
  auto const numUses = callee->numClosureUseLocals();
  auto const cls = callee->implCls();

  auto const getProp = [&](auto index) {
    assertx(index < numUses);
    auto const slot = index + (cls->hasClosureCoeffectsProp() ? 1 : 0);
    auto const type =
      typeFromRAT(cls->declPropRepoAuthType(slot), callee->cls()) & TCell;
    auto const addr = ldPropAddr(env, closure, nullptr, cls, slot, type);
    return gen(env, LdMem, type, addr);
  };

  // Move props and skip incref when closure refcount is 1 and going to be
  // released.
  ifThenElse(
    env,
    [&](Block* taken){
      gen(env, DecReleaseCheck, taken, closure);
    },
    [&] { // Next: closure RC goes to 0
      for (auto i = 0; i < numUses; ++i) {
        auto const prop = getProp(i);
        gen(env, StLoc, LocalId{firstClosureUseLocal + i}, fp(env), prop);
      }
      gen(env, ReleaseShallow, closure);
    },
    [&] { // Taken: closure RC != 0
      for (auto i = 0; i < numUses; ++i) {
        auto const prop = getProp(i);
        gen(env, IncRef, prop);
        gen(env, StLoc, LocalId{firstClosureUseLocal + i}, fp(env), prop);
      }
    }
  );
}

/*
 * Set non-input locals to Uninit.
 */
void emitInitRegularLocals(IRGS& env, const Func* callee) {
  emitInitLocalRange(
    env, callee, callee->firstRegularLocalId(), callee->numLocals());
}

void emitSurpriseCheck(IRGS& env, const Func* callee) {
  if (isInlining(env)) return;

  // Check surprise flags in the same place as the interpreter: after func entry
  // and DV initializers, right before entering the main body.
  auto const checkStackOverflow =
    stack_check_kind(callee) == StackCheck::Combine;
  auto const data = CheckHandleSurpriseEnterData { callee, checkStackOverflow };

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckSurpriseFlagsEnter, data, taken, fp(env));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, HandleSurpriseEnter, data, fp(env));
    }
  );
}

}

void emitEnter(IRGS& env, Offset relOffset) {
  emitSurpriseCheck(env, curFunc(env));

  jmpImpl(env, bcOff(env) + relOffset);
}

void emitFuncEntry(IRGS& env) {
  assertx(curSrcKey(env).funcEntry());
  auto const callee = curFunc(env);

  if (curSrcKey(env).trivialDVFuncEntry()) {
    assertx(!isInlining(env));
    auto const param = curSrcKey(env).numEntryArgs();
    emitInitScalarDefaultParamLocal(env, callee, param);
    emitJmpFuncBody(env, callee, param + 1, env.funcEntryPrevFP,
                    env.funcEntryArFlags, env.funcEntryCalleeId,
                    env.funcEntryCtx);
    return;
  }

  emitInitDefaultParamLocals(env, callee, curSrcKey(env).numEntryArgs());
  emitInitClosureLocals(env, callee);
  emitInitRegularLocals(env, callee);

  // DV initializers delay the function call event hook until the Enter opcode.
  if (curSrcKey(env).numEntryArgs() < callee->numNonVariadicParams()) return;

  emitSurpriseCheck(env, callee);
}

///////////////////////////////////////////////////////////////////////////////

}
