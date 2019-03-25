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
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
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

// Check if HasReifiedGenerics flag is set
SSATmp* emitARHasReifiedGenericsTest(IRGS& env) {
  auto const flags = gen(env, LdARNumArgsAndFlags, fp(env));
  auto const test = gen(
    env, AndInt, flags,
    cns(env, static_cast<int32_t>(ActRec::Flags::HasReifiedGenerics))
  );
  return test;
}

// Load the reified generics from ActRec if the bit is set otherwise set
// it uninit. We will error at the end of the prologue
SSATmp* emitLdARReifiedGenericsSafe(IRGS& env) {
  return cond(
    env,
    [&] (Block* taken) {
      auto const test = emitARHasReifiedGenericsTest(env);
      gen(env, JmpZero, taken, test);
    },
    [&] { return gen(env, LdARReifiedGenerics, fp(env)); },
    // taken
    [&] { return cns(env, TUninit); }
  );
}

const StaticString s_reified_generics_not_given(
  Strings::REIFIED_GENERICS_NOT_GIVEN
);

// Check whether HasReifiedGenerics is set on the ActRec
void emitARHasReifiedGenericsCheck(IRGS& env) {
  auto const func = curFunc(env);
  // It is possible to create a reified function and call it with reified
  // parameters but then, on sandboxes, make this function unreified yet
  // still call it with reified parameters. We need to catch this, hence
  // the following check has to happen not only on reified functions but
  // on all functions when we are not in repo mode.
  if (!func->hasReifiedGenerics() &&
      (func->cls() ? func->cls()->attrs() & AttrUnique : func->isUnique()) &&
      RuntimeOption::RepoAuthoritative) {
    return;
  }
  if (!func->hasReifiedGenerics()) return;
  ifThen(
    env,
    [&] (Block* taken) {
      auto const test = emitARHasReifiedGenericsTest(env);
      gen(env, JmpZero, taken, test);
    },
    [&] {
      if (!func->hasReifiedGenerics()) return;
      // null out VarEnv before raising error
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseError, cns(env, s_reified_generics_not_given.get()));
    }
  );
  // Now that we know that first local is not Tuninit,
  // lets tell that to the JIT
  gen(
    env,
    AssertLoc,
    RuntimeOption::EvalHackArrDVArrs ? TVec : TArr,
    LocalId{func->numParams()},
    fp(env)
  );
}

// Checks whether the reified generics matches the one we expect
void emitCorrectNumOfReifiedGenericsCheck(IRGS& env) {
  auto const func = curFunc(env);
  if (!func->hasReifiedGenerics()) return;
  // First local contains the reified generics
  auto const reified_generics =
    gen(
      env,
      LdLoc,
      RuntimeOption::EvalHackArrDVArrs ? TVec : TArr,
      LocalId{func->numParams()},
      fp(env)
    );
  gen(env, CheckFunReifiedGenericMismatch, FuncData{func}, reified_generics);
}

/*
 * Initialize parameters.
 *
 * Set un-passed parameters to Uninit (or the empty array, for the variadic
 * capture parameter) and set up the ExtraArgs on the ActRec as needed.
 */
void init_params(IRGS& env, const Func* func, uint32_t argc) {
  /*
   * Maximum number of default-value parameter initializations to unroll.
   */
  constexpr auto kMaxParamsInitUnroll = 5;

  auto const nparams = func->numNonVariadicParams();

  if (func->hasReifiedGenerics()) {
    // Currently does not work with closures
    assertx(!func->isClosureBody());
    auto const reified_generics = emitLdARReifiedGenericsSafe(env);
    gen(env, KillARReifiedGenerics, fp(env));
    // $0ReifiedGenerics is the first local
    gen(env, StLoc, LocalId{func->numParams()}, fp(env), reified_generics);
  }

  if (argc < nparams) {
    // Too few arguments; set everything else to Uninit.
    if (nparams - argc <= kMaxParamsInitUnroll || env.inlineLevel) {
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
        cns(env, staticEmptyVArray()));
  }

  if (!env.inlineLevel) {
    // Null out or initialize the frame's ExtraArgs.
    env.irb->exceptionStackBoundary();
    gen(env, InitExtraArgs, FuncEntryData{func, argc}, fp(env));
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

  // Closure object properties are the use vars followed by the static locals
  // (which are per-instance).
  auto const nuse = cls->numDeclProperties() - func->numStaticLocals();
  ptrdiff_t use_var_off = sizeof(ObjectData);

  for (auto i = 0; i < nuse; ++i, use_var_off += sizeof(Cell)) {
    auto const ty = typeFromRAT(cls->declPropRepoAuthType(i), func->cls());
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
    auto const nuse = func->implCls()->numDeclProperties() -
                      func->numStaticLocals();
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
void warn_argument_arity(IRGS& env, uint32_t argc) {
  auto const func = env.context.func;
  auto const nparams = func->numNonVariadicParams();

  if (!func->isCPPBuiltin()) {
    auto const& paramInfo = func->params();

    for (auto i = argc; i < nparams; ++i) {
      if (paramInfo[i].funcletOff == InvalidAbsoluteOffset) {
        env.irb->exceptionStackBoundary();
        gen(env, RaiseMissingArg, FuncArgData { func, argc });
        break;
      }
    }
  }
  if (!func->hasVariadicCaptureParam() && argc > nparams) {
    gen(env, RaiseTooManyArg, FuncArgData { func, argc });
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
  auto const func = env.context.func;

  // Emit debug code.
  if (Trace::moduleEnabled(Trace::ringbuffer) && !func->isMagic()) {
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
  auto const func = env.context.func;

  // Increment profiling counter.
  if (isProfiling(env.context.kind)) {
    gen(env, IncProfCounter, TransIDData{transID});
    profData()->setProfiling(func->getFuncId());
  }

  // Initialize params, locals, and---if we have a closure---the closure's
  // bound class context and use vars.
  emitPrologueLocals(env, argc, func, nullptr);
  // "Kill" all the class-ref slots initially. This normally won't do anything
  // (the class-ref slots should be unoccupied at this point), but in debugging
  // builds it will write poison values to them.
  for (uint32_t slot = 0; slot < func->numClsRefSlots(); ++slot) {
    killClsRef(env, slot);
  }
  warn_argument_arity(env, argc);

  // Check surprise flags in the same place as the interpreter: after setting
  // up the callee's frame but before executing any of its code.
  env.irb->exceptionStackBoundary();
  if (stack_check_kind(func, argc) == StackCheck::Combine) {
    gen(env, CheckSurpriseAndStack, FuncEntryData { func, argc }, fp(env));
  } else {
    gen(env, CheckSurpriseFlagsEnter, FuncEntryData { func, argc }, fp(env));
  }

  emitARHasReifiedGenericsCheck(env);
  emitCorrectNumOfReifiedGenericsCheck(env);
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

void emitMagicFuncPrologue(IRGS& env, uint32_t argc, TransID transID) {
  DEBUG_ONLY auto const func = env.context.func;
  assertx(func->isMagic());
  assertx(func->numParams() == 2);
  assertx(!func->hasVariadicCaptureParam());

  Block* two_arg_prologue = nullptr;

  emitPrologueEntry(env, argc);

  // If someone just called __call() or __callStatic() directly, branch to a
  // normal non-magic prologue.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckARMagicFlag, taken, fp(env));
      if (argc == 2) two_arg_prologue = taken;
    },
    [&] {
      emitPrologueBody(env, argc, transID);
    }
  );

  // Pack the passed args into an array, then store it as the second param.
  // This has to happen before we write the first param.
  auto const args_arr = (argc == 0)
    ? cns(env, staticEmptyVArray())
    : gen(env, PackMagicArgs, fp(env));
  gen(env, StLoc, LocalId{1}, fp(env), args_arr);

  // Store the name of the called function to the first param, then null it out
  // on the ActRec.
  auto const inv_name = gen(env, LdARInvName, fp(env));
  gen(env, StLoc, LocalId{0}, fp(env), inv_name);
  gen(env, StARInvName, fp(env), cns(env, nullptr));

  // Reset all the flags except for the dynamic call flag and set the argument
  // count to 2.
  auto const flag = gen(
    env,
    AndInt,
    gen(env, LdARNumArgsAndFlags, fp(env)),
    cns(env, static_cast<int32_t>(ActRec::Flags::DynamicCall))
  );
  auto combined = gen(
    env,
    OrInt,
    flag,
    cns(env, ActRec::encodeNumArgsAndFlags(2, ActRec::Flags::None))
  );
  gen(env, StARNumArgsAndFlags, fp(env), combined);

  // Jmp to the two-argument prologue, or emit it if it doesn't exist yet.
  if (two_arg_prologue) {
    gen(env, Jmp, two_arg_prologue);
  } else {
    emitPrologueBody(env, 2, transID);
  }
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitPrologueLocals(IRGS& env, uint32_t argc,
                        const Func* func, SSATmp* closureOpt) {
  init_params(env, func, argc);
  if (func->isClosureBody()) {
    auto const closure = juggle_closure_ctx(env, func, closureOpt);
    init_use_vars(env, func, closure);
  }
  init_locals(env, func);
}

void emitFuncPrologue(IRGS& env, uint32_t argc, TransID transID) {
  if (env.context.func->isMagic()) {
    return emitMagicFuncPrologue(env, argc, transID);
  }
  emitPrologueEntry(env, argc);
  emitPrologueBody(env, argc, transID);
}

void emitFuncBodyDispatch(IRGS& env, const DVFuncletsVec& dvs) {
  auto const func = env.context.func;
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
      string_printf(
        str,
        Strings::FUNCTION_CALLED_DYNAMICALLY,
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
