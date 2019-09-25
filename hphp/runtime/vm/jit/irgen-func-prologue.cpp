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
template <class Body>
void prologueDispatch(IRGS& env, const Func* func, Body body) {
  assertx(env.irb->curMarker().prologue());

  if (!func->mayHaveThis()) {
    body(false);
    return;
  }

  if (func->requiresThisInBody()) {
    body(true);
    return;
  }

  ifThenElse(
    env,
    [&] (Block* taken) {
      auto const ctx = gen(env, LdCtx, fp(env));
      gen(env, CheckCtxThis, taken, ctx);
    },
    [&] {
      body(true);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      body(false);
    }
  );
}

/*
 * Initialize parameters.
 *
 * Set un-passed parameters to Uninit (or the empty array, for the variadic
 * capture parameter) and set up the ExtraArgs on the ActRec as needed.
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
  if (func->hasReifiedGenerics() && argc <= nparams) {
    ifThenElse(
      env,
      [&] (Block* taken) {
        auto constexpr flag = 1 << CallFlags::Flags::HasGenerics;
        auto const hasGenerics = gen(env, AndInt, callFlags, cns(env, flag));
        gen(env, JmpNZero, taken, hasGenerics);
      },
      [&] {
        // Generics not given. We will fail later. Write uninit so that
        // the local is properly initialized.
        gen(env, StLoc, LocalId{func->numParams()}, fp(env), cns(env, TUninit));
      },
      [&] {
        // Already at the correct slot.
        if (argc == func->numParams()) return;

        auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
        gen(env, AssertLoc, type, LocalId{argc}, fp(env));
        auto const generics = gen(env, LdLoc, type, LocalId{argc}, fp(env));
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
    // Need to initialize `...$args'.
    gen(env, StLoc, LocalId{nparams}, fp(env),
        cns(env, ArrayData::CreateVArray()));
  }

  assertx(!isInlining(env) || argc <= nparams);
  if (!isInlining(env)) {
    // Null out or initialize the frame's ExtraArgs. Also takes care of moving
    // generics to the correct slot if there were too many args.
    env.irb->exceptionStackBoundary();
    gen(env, InitExtraArgs, FuncEntryData{func, argc}, fp(env), callFlags);
  }
}

/*
 * Set up the closure object and class context.
 *
 * We swap out the Closure object stored in m_this, and replace it with the
 * closure's bound Ctx, which may be either an object or a class context.  We
 * then teleport the object onto the stack as the first local after the params.
 */
SSATmp* juggle_closure_ctx(IRGS& env, const Func* func, SSATmp* closureOpt) {
  assertx(func->isClosureBody());

  auto const closure_type = Type::ExactObj(func->implCls());
  auto const closure = [&] {
    if (!closureOpt) {
      return gen(env, LdClosure, closure_type, fp(env));
    }
    if (closureOpt->hasConstVal() || closureOpt->isA(closure_type)) {
      return closureOpt;
    }
    return gen(env, AssertType, closure_type, closureOpt);
  }();

  auto const ctx = func->cls() ?
    gen(env, LdClosureCtx, closure) : cns(env, nullptr);

  gen(env, InitCtx, fp(env), ctx);
  // We can skip the incref for static closures, which have a Cctx.
  if (func->cls() && !func->isStatic()) {
    gen(env, IncRef, ctx);
  }

  // Teleport the closure to the next local.  There's no need to incref since
  // it came from m_this.
  gen(env, StLoc, LocalId{func->numParams()}, fp(env), closure);
  return closure;
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
  ptrdiff_t use_var_off = sizeof(ObjectData);

  for (auto i = 0; i < nuse; ++i, use_var_off += sizeof(Cell)) {
    auto const ty =
      typeFromRAT(cls->declPropRepoAuthType(i), func->cls()) & TCell;
    auto const addr = gen(
      env,
      LdPropAddr,
      ByteOffsetData { use_var_off },
      ty.lval(Ptr::Prop),
      closure
    );
    auto const prop = gen(env, LdMem, ty, addr);
    gen(env, StLoc, LocalId{nparams + 1 + i}, fp(env), prop);
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
    num_inited += 1 + nuse;
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
 * Emit raise-warnings for any missing or too many arguments.
 */
void warn_argument_arity(IRGS& env, uint32_t argc, SSATmp* numTooManyArgs) {
  auto const func = curFunc(env);
  auto const nparams = func->numNonVariadicParams();
  auto const& paramInfo = func->params();

  for (auto i = argc; i < nparams; ++i) {
    if (paramInfo[i].funcletOff == InvalidAbsoluteOffset) {
      env.irb->exceptionStackBoundary();
      gen(env, RaiseMissingArg, FuncArgData { func, argc });
      break;
    }
  }
  if (numTooManyArgs) {
    gen(env, RaiseTooManyArg, FuncData { func }, numTooManyArgs);
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
   * locals into an ExtraArgs structure.  The stack overflow code knows how to
   * handle the possibility of an ExtraArgs structure on the ActRec, and the
   * uninits are harmless as long as we know we aren't going to segfault while
   * we write them.
   *
   * There's always sSurprisePageSize extra space at the bottom (lowest
   * addresses) of the eval stack, so we just only do this optimization if
   * we're sure we're going to write few enough uninits that we would be
   * staying within that region if the locals are actually too deep.
   */
  auto const safeFromSEGV = Stack::sSurprisePageSize / sizeof(TypedValue);

  return func->numLocals() - argc < safeFromSEGV
    ? StackCheck::Combine
    : StackCheck::Early;
}

///////////////////////////////////////////////////////////////////////////////

void emitPrologueEntry(IRGS& env, uint32_t argc) {
  auto const func = curFunc(env);

  // Emit debug code.
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto msg = RBMsgData { Trace::RBTypeFuncPrologue, func->fullName() };
    gen(env, RBTraceMsg, msg);
  }

  gen(env, EnterFrame, fp(env));

  // Emit early stack overflow check if necessary.
  if (stack_check_kind(func, argc) == StackCheck::Early) {
    env.irb->exceptionStackBoundary();
    gen(env, CheckStackOverflow, fp(env));
  }
}

void emitPrologueBody(IRGS& env, uint32_t argc, TransID transID) {
  auto const func = curFunc(env);
  auto const callFlags = gen(env, DefCallFlags);

  // Increment profiling counter.
  if (isProfiling(env.context.kind)) {
    gen(env, IncProfCounter, TransIDData{transID});
    profData()->setProfiling(func->getFuncId());
  }

  // If too many arguments were passed, load the actual number of arguments
  // from ActRec before m_numArgs gets overwritten by emitPrologueLocals().
  // We can't use argc, as there is only one prologue for argc > nparams.
  auto const numTooManyArgs =
    !func->hasVariadicCaptureParam() && argc > func->numNonVariadicParams()
      ? gen(env, LdARNumParams, fp(env))
      : nullptr;

  // Initialize params, locals, and---if we have a closure---the closure's
  // bound class context and use vars.
  emitPrologueLocals(env, argc, callFlags, nullptr);

  warn_argument_arity(env, argc, numTooManyArgs);

  emitGenericsMismatchCheck(env, callFlags);

  // Check surprise flags in the same place as the interpreter: after setting
  // up the callee's frame but before executing any of its code.
  env.irb->exceptionStackBoundary();
  if (stack_check_kind(func, argc) == StackCheck::Combine) {
    gen(env, CheckSurpriseAndStack, FuncEntryData { func, argc }, fp(env));
  } else {
    gen(env, CheckSurpriseFlagsEnter, FuncEntryData { func, argc }, fp(env));
  }

  emitCalleeDynamicCallCheck(env);
  emitCallMCheck(env);

  prologueDispatch(
    env, func,
    [&] (bool hasThis) {
      // Emit the bindjmp for the function body.
      gen(
        env,
        ReqBindJmp,
        ReqBindJmpData {
          SrcKey { func, func->getEntryForNumArgs(argc), ResumeMode::None,
                   hasThis },
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

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitPrologueLocals(IRGS& env, uint32_t argc, SSATmp* callFlags,
                        SSATmp* closureOpt) {
  auto const func = curFunc(env);
  init_params(env, func, argc, callFlags);
  if (func->isClosureBody()) {
    auto const closure = juggle_closure_ctx(env, func, closureOpt);
    init_use_vars(env, func, closure);
  }
  init_locals(env, func);
}

void emitFuncPrologue(IRGS& env, uint32_t argc, TransID transID) {
  emitPrologueEntry(env, argc);
  emitPrologueBody(env, argc, transID);
}

void emitFuncBodyDispatch(IRGS& env, const DVFuncletsVec& dvs) {
  auto const func = curFunc(env);
  auto const num_args = gen(env, LdARNumParams, fp(env));

  if (isProfiling(env.context.kind)) {
    profData()->setProfiling(func->getFuncId());
  }

  prologueDispatch(
    env, func,
    [&] (bool hasThis) {
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
                SrcKey { func, dv.second, ResumeMode::None, hasThis },
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
          SrcKey { func, func->base(), ResumeMode::None, hasThis },
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

void emitGenericsMismatchCheck(IRGS& env, SSATmp* callFlags) {
  auto const func = curFunc(env);
  if (!func->hasReifiedGenerics()) return;

  // Fail if generics were not passed.
  ifThen(
    env,
    [&] (Block* taken) {
      auto constexpr flag = 1 << CallFlags::Flags::HasGenerics;
      auto const hasGenerics = gen(env, AndInt, callFlags, cns(env, flag));
      gen(env, JmpZero, taken, hasGenerics);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      updateMarker(env);
      env.irb->exceptionStackBoundary();
      gen(env, ThrowCallReifiedFunctionWithoutGenerics, cns(env, curFunc(env)));
    }
  );

  // Fail on generics count/wildcard mismatch.
  auto const type = RuntimeOption::EvalHackArrDVArrs ? TVec : TArr;
  auto const local = LocalId{func->numParams()};
  gen(env, AssertLoc, type, local, fp(env));
  auto const generics = gen(env, LdLoc, type, local, fp(env));

  // Generics may be known if we are inlining.
  if (generics->hasConstVal(type)) {
    auto const genericsArr = RuntimeOption::EvalHackArrDVArrs
      ? generics->vecVal() : generics->arrVal();
    auto const& genericsDef = func->getReifiedGenericsInfo().m_typeParamInfo;
    if (genericsArr->size() == genericsDef.size()) {
      bool match = true;
      IterateKV(genericsArr, [&](Cell k, TypedValue v) {
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
}

void emitCalleeDynamicCallCheck(IRGS& env) {
  auto const func = curFunc(env);

  if (!(RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin())) {
    return;
  }

  ifThen(
    env,
    [&] (Block* taken) {
      auto flags = gen(env, LdARNumArgsAndFlags, fp(env));
      auto test = gen(
        env, AndInt, flags,
        cns(env, static_cast<int32_t>(ActRec::Flags::DynamicCall))
      );
      gen(env, JmpNZero, taken, test);
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
        func->fullDisplayName()->data()
      );
      auto const msg = cns(env, makeStaticString(str));

      if (RuntimeOption::EvalNoticeOnBuiltinDynamicCalls && func->isBuiltin()) {
        gen(env, RaiseNotice, msg);
      }
    }
  );
}

const StaticString
  s_inoutError("In/out function called dynamically without inout annotations");

void emitCallMCheck(IRGS& env) {
  auto const func = curFunc(env);

  if (!func->takesInOutParams()) {
    return;
  }

  ifThen(
    env,
    [&] (Block* taken) {
      auto flags = gen(env, LdARNumArgsAndFlags, fp(env));
      auto test = gen(
        env, AndInt, flags,
        cns(env, static_cast<int32_t>(ActRec::Flags::MultiReturn))
      );
      gen(env, JmpZero, taken, test);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseError, cns(env, s_inoutError.get()));
    }
  );
}


///////////////////////////////////////////////////////////////////////////////

}}}
