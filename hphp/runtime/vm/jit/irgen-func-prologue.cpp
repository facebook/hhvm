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
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
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
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/text-util.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize parameters.
 *
 * Set un-passed parameters to Uninit and set up the variadic capture parameter
 * as neeeded.
 */
void init_params(IRGS& env, const Func* func, uint32_t argc,
                 SSATmp* callFlags) {
  // Reified generics are not supported on closures yet.
  assertx(!func->hasReifiedGenerics() || !func->isClosureBody());

  // Maximum number of default-value parameter initializations to unroll.
  auto constexpr kMaxParamsInitUnroll = 5;
  auto const nparams = func->numNonVariadicParams();

  // If generics were expected and given, but not enough args were provided,
  // move generics to the correct slot ($0ReifiedGenerics is the first
  // non-parameter local).
  // FIXME: leaks memory if generics were given but not expected.
  if (func->hasReifiedGenerics()) {
    ifThenElse(
      env,
      [&] (Block* taken) {
        auto constexpr flag = 1 << CallFlags::Flags::HasGenerics;
        auto const hasGenerics = gen(env, AndInt, callFlags, cns(env, flag));
        gen(env, JmpNZero, taken, hasGenerics);
      },
      [&] {
        arrprov::TagOverride ap_override{arrprov::tagFromSK(env.bcState)};
        // Generics not given. We will either fail later or raise a warning.
        // Write empty array so that the local is properly initialized.
        auto const emptyArr =
          RuntimeOption::EvalHackArrDVArrs ? ArrayData::CreateVec()
                                           : ArrayData::CreateVArray();
        gen(
          env,
          StLoc,
          LocalId{func->numParams()},
          fp(env),
          cns(env, emptyArr));
      },
      [&] {
        // Already at the correct slot.
        if (argc == func->numParams()) return;

        auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
        auto const generics = [&] {
          if (argc < func->numParams()) {
            gen(env, AssertLoc, type, LocalId{argc}, fp(env));
            return gen(env, LdLoc, type, LocalId{argc}, fp(env));
          } else {
            assertx(!isInlining(env));
            auto const genericsOff = IRSPRelOffsetData(offsetFromIRSP(
              env, BCSPRelOffset{func->numSlotsInFrame() - int32_t(argc) - 1}));
            gen(env, AssertStk, type, genericsOff, sp(env));
            return gen(env, LdStk, type, genericsOff, sp(env));
          }
        }();
        gen(env, StLoc, LocalId{func->numParams()}, fp(env), generics);
      }
    );
  }

  if (argc < nparams) {
    // Too few arguments; set everything else to Uninit.
    if (nparams - argc <= kMaxParamsInitUnroll || isInlining(env)) {
      for (auto i = argc; i < nparams; ++i) {
        gen(env, StLoc, LocalId{i}, fp(env), cns(env, TUninit));
      }
    } else {
      gen(env, StLocRange, LocalIdRange{argc, nparams},
          fp(env), cns(env, TUninit));
    }
  }

  if (argc <= nparams && func->hasVariadicCaptureParam()) {
    ARRPROV_USE_RUNTIME_LOCATION();
    // Need to initialize `...$args'.
    gen(env, StLoc, LocalId{nparams}, fp(env),
        cns(env, ArrayData::CreateVArray()));
  }
}

/*
 * Copy the closure's use variables from the closure object's properties onto
 * the stack.
 */
void init_use_vars(IRGS& env, const Func* func, SSATmp* closure) {
  auto const cls = func->implCls();
  auto const nparams = func->numParams();

  assertx(func->isClosureBody());

  // Closure object properties are the use vars.
  auto const nuse = cls->numDeclProperties();

  for (auto i = 0; i < nuse; ++i) {
    auto const ty =
      typeFromRAT(cls->declPropRepoAuthType(i), func->cls()) & TCell;
    auto const addr = gen(
      env,
      LdPropAddr,
      IndexData { cls->propSlotToIndex(i) },
      ty.lval(Ptr::Prop),
      closure
    );
    auto const prop = gen(env, LdMem, ty, addr);
    gen(env, StLoc, LocalId{nparams + i}, fp(env), prop);
    gen(env, IncRef, prop);
  }
}

/*
 * Set locals to Uninit.
 */
void init_locals(IRGS& env, const Func* func) {
  /*
   * Maximum number of local initializations to unroll.
   *
   * The actual crossover point in terms of code size is 6 (just like for the
   * params init unroll limit); 9 was determined by experiment to be the
   * optimal point in certain benchmarks.
   */
  constexpr auto kMaxLocalsInitUnroll = 9;

  auto const nlocals = func->numLocals();

  auto num_inited = func->numParams();

  if (func->isClosureBody()) {
    auto const nuse = func->implCls()->numDeclProperties();
    num_inited += nuse;
  }

  if (func->hasReifiedGenerics()) num_inited++;

  // We set to Uninit all locals beyond any params and any closure use vars.
  if (num_inited < nlocals) {
    if (nlocals - num_inited <= kMaxLocalsInitUnroll) {
      for (auto i = num_inited; i < nlocals; ++i) {
        gen(env, StLoc, LocalId{i}, fp(env), cns(env, TUninit));
      }
    } else {
      gen(env, StLocRange, LocalIdRange{num_inited, (uint32_t)nlocals},
          fp(env), cns(env, TUninit));
    }
  }
}

/*
 * Emit raise-warnings for any missing arguments.
 */
void warnOnMissingArgs(IRGS& env, uint32_t argc) {
  auto const func = curFunc(env);
  if (argc < func->numRequiredParams()) {
    env.irb->exceptionStackBoundary();
    gen(env, ThrowMissingArg, FuncArgData { func, argc });
  }
}

///////////////////////////////////////////////////////////////////////////////

/*
 * How to perform our stack overflow check.
 */
enum class StackCheck {
  None,   // not needed
  Early,  // must occur before setting up locals
  Combine // can be delayed and combined with surprise flags check
};

StackCheck stack_check_kind(const Func* func, uint32_t argc) {
  if (func->isPhpLeafFn() &&
      func->maxStackCells() < kStackCheckLeafPadding) {
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

  return func->numLocals() < safeFromSEGV + argc
    ? StackCheck::Combine
    : StackCheck::Early;
}

///////////////////////////////////////////////////////////////////////////////

Type prologueCtxType(const Func* func) {
  assertx(func->isClosureBody() || func->cls());
  if (func->isClosureBody()) return Type::ExactObj(func->implCls());
  if (func->isStatic()) return Type::SubCls(func->cls());
  return thisTypeFromFunc(func);
}

void emitPrologueEntry(IRGS& env, uint32_t argc) {
  auto const func = curFunc(env);

  // Emit debug code.
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto msg = RBMsgData { Trace::RBTypeFuncPrologue, func->fullName() };
    gen(env, RBTraceMsg, msg);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    // Make sure we are at the right function.
    auto const callFunc = gen(env, DefCallFunc);
    auto const callFuncOK = gen(env, EqFunc, callFunc, cns(env, func));
    gen(env, JmpZero, makeUnreachable(env, ASSERT_REASON), callFuncOK);

    // Make sure we are at the right prologue.
    auto const numArgs = gen(env, DefCallNumArgs);
    auto const numArgsOK = gen(env, EqInt, numArgs, cns(env, argc));
    gen(env, JmpZero, makeUnreachable(env, ASSERT_REASON), numArgsOK);
  }

  gen(env, EnterPrologue);

  // Emit early stack overflow check if necessary.
  if (stack_check_kind(func, argc) == StackCheck::Early) {
    env.irb->exceptionStackBoundary();
    gen(env, CheckStackOverflow, sp(env));
  }
}

void emitSpillFrame(IRGS& env, uint32_t argc, SSATmp* callFlags,
                    SSATmp* prologueCtx) {
  auto const func = curFunc(env);
  auto const ctx = [&] {
    if (!func->isClosureBody()) return prologueCtx;

    if (!func->cls()) return cns(env, nullptr);
    if (func->isStatic()) {
      return gen(env, LdClosureCls, Type::SubCls(func->cls()), prologueCtx);
    }
    auto const closureThis =
      gen(env, LdClosureThis, Type::SubObj(func->cls()), prologueCtx);
    gen(env, IncRef, closureThis);
    return closureThis;
  }();

  // If we don't have variadics, unpack arg will be dropped.
  auto const arNumArgs = std::min(argc, func->numParams());

  gen(env, DefFuncEntryFP, FuncData { func },
      fp(env), sp(env), callFlags, cns(env, arNumArgs), ctx);
  auto const spOffset = FPInvOffset { func->numSlotsInFrame() };
  gen(env, DefFrameRelSP, FPInvOffsetData { spOffset }, fp(env));
}

void emitPrologueBody(IRGS& env, uint32_t argc, TransID transID,
                      SSATmp* callFlags, SSATmp* closure) {
  auto const func = curFunc(env);

  // Increment profiling counter.
  if (isProfiling(env.context.kind)) {
    gen(env, IncProfCounter, TransIDData{transID});
    profData()->setProfiling(func->getFuncId());
  }

  auto const unpackArgsForTooManyArgs = [&]() -> SSATmp* {
    if (argc <= func->numParams()) return nullptr;

    // If too many arguments were passed, load the array containing the unpack
    // args, as it is about to get overridden by emitPrologueLocals(). Need to
    // use LdStk instead of LdLoc, as there may be no such local.
    assertx(!func->hasVariadicCaptureParam());
    assertx(argc == func->numNonVariadicParams() + 1);
    auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
    auto const unpackOff = IRSPRelOffsetData(offsetFromIRSP(
      env, BCSPRelOffset{func->numSlotsInFrame() - int32_t(argc)}));
    gen(env, AssertStk, type, unpackOff, sp(env));
    return gen(env, LdStk, type, unpackOff, sp(env));
  }();

  // Initialize params, locals, and---if we have a closure---the closure's
  // bound class context and use vars.
  emitPrologueLocals(env, argc, callFlags, closure);

  env.irb->exceptionStackBoundary();

  warnOnMissingArgs(env, argc);
  if (unpackArgsForTooManyArgs != nullptr) {
    // RaiseTooManyArg will free unpackArgsForTooManyArgs and also use it to
    // report the correct numbers.
    gen(env, RaiseTooManyArg, FuncData { func }, unpackArgsForTooManyArgs);
  }

  emitGenericsMismatchCheck(env, callFlags);
  emitCalleeDynamicCallCheck(env, callFlags);

  // Check surprise flags in the same place as the interpreter: after setting
  // up the callee's frame but before executing any of its code.
  env.irb->exceptionStackBoundary();
  if (stack_check_kind(func, argc) == StackCheck::Combine) {
    gen(env, CheckSurpriseAndStack, FuncEntryData { func, argc }, fp(env));
  } else {
    gen(env, CheckSurpriseFlagsEnter, FuncEntryData { func, argc }, fp(env));
  }


  // Emit the bindjmp for the function body.
  gen(
    env,
    ReqBindJmp,
    ReqBindJmpData {
      SrcKey { func, func->getEntryForNumArgs(argc), ResumeMode::None },
      FPInvOffset { func->numSlotsInFrame() },
      spOffBCFromIRSP(env),
      TransFlags{}
    },
    sp(env),
    fp(env)
  );
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitPrologueLocals(IRGS& env, uint32_t argc, SSATmp* callFlags,
                        SSATmp* closure) {
  auto const func = curFunc(env);
  init_params(env, func, argc, callFlags);

  assertx(func->isClosureBody() == (closure != nullptr));
  if (func->isClosureBody()) {
    init_use_vars(env, func, closure);
    decRef(env, closure);
  }

  init_locals(env, func);
}

void emitFuncPrologue(IRGS& env, uint32_t argc, TransID transID) {
  auto const func = curFunc(env);
  assertx(argc <= func->numNonVariadicParams() + 1);

  // Define register inputs before doing anything else that may clobber them.
  auto const callFlags = gen(env, DefCallFlags);
  auto const prologueCtx = (func->isClosureBody() || func->cls())
    ? gen(env, DefCallCtx, prologueCtxType(func))
    : cns(env, nullptr);
  auto const closure = func->isClosureBody() ? prologueCtx : nullptr;

  emitPrologueEntry(env, argc);
  emitSpillFrame(env, argc, callFlags, prologueCtx);
  emitPrologueBody(env, argc, transID, callFlags, closure);
}

void emitFuncBodyDispatch(IRGS& env, const DVFuncletsVec& dvs) {
  auto const func = curFunc(env);
  auto const num_args = gen(env, LdARNumParams, fp(env));

  if (isProfiling(env.context.kind)) {
    profData()->setProfiling(func->getFuncId());
  }

  for (auto const& dv : dvs) {
    ifThen(
      env,
      [&] (Block* taken) {
        auto const lte = gen(env, LteInt, num_args, cns(env, dv.first));
        gen(env, JmpNZero, taken, lte);
      },
      [&] {
        gen(
          env,
          ReqBindJmp,
          ReqBindJmpData {
            SrcKey { func, dv.second, ResumeMode::None },
            FPInvOffset { func->numSlotsInFrame() },
            spOffBCFromIRSP(env),
            TransFlags{}
          },
          sp(env),
          fp(env)
        );
      }
    );
  }

  gen(
    env,
    ReqBindJmp,
    ReqBindJmpData {
      SrcKey { func, func->base(), ResumeMode::None },
      FPInvOffset { func->numSlotsInFrame() },
      spOffBCFromIRSP(env),
      TransFlags{}
    },
    sp(env),
    fp(env)
  );
}

void emitGenericsMismatchCheck(IRGS& env, SSATmp* callFlags) {
  auto const func = curFunc(env);
  if (!func->hasReifiedGenerics()) return;

  // Fail if generics were not passed.
  ifThenElse(
    env,
    [&] (Block* taken) {
      auto constexpr flag = 1 << CallFlags::Flags::HasGenerics;
      auto const hasGenerics = gen(env, AndInt, callFlags, cns(env, flag));
      gen(env, JmpZero, taken, hasGenerics);
    },
    [&] {
      // Fail on generics count/wildcard mismatch.
      auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
      auto const local = LocalId{func->numParams()};
      gen(env, AssertLoc, type, local, fp(env));
      auto const generics = gen(env, LdLoc, type, local, fp(env));

      // Generics may be known if we are inlining.
      if (generics->hasConstVal(type)) {
        auto const genericsArr = generics->arrLikeVal();
        auto const& genericsDef =
          func->getReifiedGenericsInfo().m_typeParamInfo;
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

      ifThen(
        env,
        [&] (Block* taken) {
          auto const fd = FuncData { func };
          auto const match =
            gen(env, IsFunReifiedGenericsMatched, fd, callFlags);
          gen(env, JmpZero, taken, match);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          updateMarker(env);
          env.irb->exceptionStackBoundary();
          gen(env, CheckFunReifiedGenericMismatch, FuncData{func}, generics);
        }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      updateMarker(env);
      env.irb->exceptionStackBoundary();
      if (areAllGenericsSoft(func->getReifiedGenericsInfo())) {
        gen(
          env,
          RaiseWarning,
          cns(env, makeStaticString(folly::sformat(
            "Generic at index 0 to Function {} must be reified, erased given",
            func->fullName()->data()))));
        return;
      }
      gen(env, ThrowCallReifiedFunctionWithoutGenerics, cns(env, func));
    }
  );
}

void emitCalleeDynamicCallCheck(IRGS& env, SSATmp* callFlags) {
  auto const func = curFunc(env);

  if (!(RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin())) {
    return;
  }

  ifThen(
    env,
    [&] (Block* taken) {
      auto constexpr flag = 1 << CallFlags::Flags::IsDynamicCall;
      auto const isDynamicCall = gen(env, AndInt, callFlags, cns(env, flag));
      gen(env, JmpNZero, taken, isDynamicCall);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);

      std::string str;
      auto error_msg = func->isDynamicallyCallable() ?
        Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE :
        Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;
      string_printf(
        str,
        error_msg,
        func->fullName()->data()
      );
      auto const msg = cns(env, makeStaticString(str));

      if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin()) {
        gen(env, RaiseNotice, msg);
      }
    }
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
