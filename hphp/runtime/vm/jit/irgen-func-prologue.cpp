/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize parameters.
 *
 * Set un-passed parameters to Uninit (or the empty array, for the variadic
 * capture parameter) and set up the ExtraArgs on the ActRec as needed.
 */
void init_params(IRGS& env, uint32_t argc) {
  /*
   * Maximum number of default-value parameter initializations to unroll.
   */
  constexpr auto kMaxParamsInitUnroll = 5;

  auto const func = env.context.func;
  auto const nparams = func->numNonVariadicParams();

  if (argc < nparams) {
    // Too few arguments; set everything else to Uninit.
    if (nparams - argc <= kMaxParamsInitUnroll) {
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
        cns(env, staticEmptyArray()));
  }

  // Null out or initialize the frame's ExtraArgs.
  gen(env, InitExtraArgs, FuncEntryData{func, argc}, fp(env));
}

/*
 * Set locals to Uninit.
 */
void init_locals(IRGS& env) {
  /*
   * Maximum number of local initializations to unroll.
   *
   * The actual crossover point in terms of code size is 6 (just like for the
   * params init unroll limit); 9 was determined by experiment to be the
   * optimal point in certain benchmarks.
   */
  constexpr auto kMaxLocalsInitUnroll = 9;

  auto const func = env.context.func;
  auto const nparams = func->numParams();
  auto const nlocals = func->numLocals();

  if (nparams < nlocals) {
    if (nlocals - nparams <= kMaxLocalsInitUnroll) {
      for (auto i = nparams; i < nlocals; ++i) {
        gen(env, StLoc, LocalId{i}, fp(env), cns(env, TUninit));
      }
    } else {
      gen(env, StLocRange, LocalIdRange{nparams, (uint32_t)nlocals},
          fp(env), cns(env, TUninit));
    }
  }
}

/*
 * Emit raise-warnings for any missing arguments.
 */
void warn_missing_args(IRGS& env, uint32_t argc) {
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
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void emitFuncPrologue(IRGS& env, uint32_t argc, TransID transID) {
  auto const func = env.context.func;

  // Emit debug code.
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto msg = RBMsgData { Trace::RBTypeFuncPrologue, func->fullName() };
    gen(env, RBTraceMsg, msg);
  }
  if (RuntimeOption::EvalJitTransCounters) {
    gen(env, IncTransCounter);
  }

  gen(env, EnterFrame, fp(env));

  // Emit stack overflow check if necessary.
  if (!(func->attrs() & AttrPhpLeafFn) ||
      func->maxStackCells() >= kStackCheckLeafPadding) {
    env.irb->exceptionStackBoundary();
    gen(env, CheckStackOverflow, fp(env));
  }

  // Increment profiling counter.
  if (mcg->tx().mode() == TransKind::Proflogue) {
    assertx(shouldPGOFunc(*func));
    auto profData = mcg->tx().profData();

    gen(env, IncProfCounter, TransIDData{transID});
    profData->setProfiling(func->getFuncId());
  }

  // Initialize params and locals.
  init_params(env, argc);
  init_locals(env);
  warn_missing_args(env, argc);

  // Check surprise flags in the same place as the interpreter: after setting
  // up the callee's frame but before executing any of its code.
  env.irb->exceptionStackBoundary();
  gen(env, CheckSurpriseFlagsEnter, FuncEntryData{func, argc}, fp(env));

  // Emit the bindjmp for the function body.
  gen(
    env,
    ReqBindJmp,
    ReqBindJmpData {
      SrcKey { func, func->getEntryForNumArgs(argc), false },
      FPInvOffset{func->numSlotsInFrame()},
      offsetFromIRSP(env, BCSPOffset{0}),
      TransFlags{}
    },
    sp(env),
    fp(env)
  );
}

///////////////////////////////////////////////////////////////////////////////

}}}
