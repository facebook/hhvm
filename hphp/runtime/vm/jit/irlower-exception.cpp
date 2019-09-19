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

#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit { namespace irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBeginCatch(IRLS& env, const IRInstruction* /*inst*/) {
  auto& v = vmain(env);

  v << landingpad{};
  emitIncStat(v, Stats::TC_CatchTrace);
}

void cgEndCatch(IRLS& env, const IRInstruction* /*inst*/) {
  // endCatchHelper only expects rvmtl() and rvmfp() to be live.
  vmain(env) << jmpi{tc::ustubs().endCatchHelper, rvmtl() | rvmfp()};
}

void cgUnwindCheckSideExit(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const sf = v.makeReg();
  v << cmpbim{0, rvmtl()[unwinderSideExitOff()], sf};
  fwdJcc(v, env, CC_E, sf, inst->taken());

  // doSideExit == true, so fall through to the side exit code.
  emitIncStat(v, Stats::TC_CatchSideExit);
}

void cgLdUnwinderValue(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), rvmtl()[unwinderTVOff()]);
}

IMPL_OPCODE_CALL(DebugBacktrace)
IMPL_OPCODE_CALL(DebugBacktraceFast)

///////////////////////////////////////////////////////////////////////////////

static void raiseHackArrCompatNotice(const StringData* msg) {
  raise_hackarr_compat_notice(msg->toCppString());
}

void cgRaiseHackArrCompatNotice(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(vmain(env), env, CallSpec::direct(raiseHackArrCompatNotice),
               kVoidDest, SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

static void raiseForbiddenDynCall(const Func* func) {
  assertx(!func->isDynamicallyCallable() ||
          RuntimeOption::EvalForbidDynamicCallsWithAttr);
  int dynCallErrorLevel = func->isMethod() ?
    (
      func->isStatic() ?
        RuntimeOption::EvalForbidDynamicCallsToClsMeth :
        RuntimeOption::EvalForbidDynamicCallsToInstMeth
    ) :
    RuntimeOption::EvalForbidDynamicCallsToFunc;
  if (dynCallErrorLevel <= 0) return;

  auto error_msg = func->isDynamicallyCallable() ?
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE :
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;
  if (dynCallErrorLevel >= 2) {
    std::string msg;
    string_printf(
      msg,
      error_msg,
      func->fullDisplayName()->data()
    );
    throw_invalid_operation_exception(makeStaticString(msg));
  } else {
    raise_notice(
      error_msg,
      func->fullDisplayName()->data()
    );
  }
}

static void raiseForbiddenDynConstruct(const Class* cls) {
  assertx(RuntimeOption::EvalForbidDynamicConstructs > 0);
  assertx(!cls->isDynamicallyConstructible());

  if (RuntimeOption::EvalForbidDynamicConstructs >= 2) {
    std::string msg;
    string_printf(
      msg,
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
    throw_invalid_operation_exception(makeStaticString(msg));
  } else {
    raise_notice(
      Strings::CLASS_CONSTRUCTED_DYNAMICALLY,
      cls->name()->data()
    );
  }
}

void cgRaiseForbiddenDynCall(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(vmain(env), env, CallSpec::direct(raiseForbiddenDynCall),
               kVoidDest, SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

void cgRaiseForbiddenDynConstruct(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(vmain(env), env, CallSpec::direct(raiseForbiddenDynConstruct),
               kVoidDest, SyncOptions::Sync, argGroup(env, inst).ssa(0));
}

void cgThrowLateInitPropError(IRLS& env, const IRInstruction* inst) {
  cgCallHelper(vmain(env), env, CallSpec::direct(throw_late_init_prop),
               kVoidDest, SyncOptions::Sync,
               argGroup(env, inst).ssa(0).ssa(1).ssa(2));
}

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(InitThrowableFileAndLine)
IMPL_OPCODE_CALL(ZeroErrorLevel)
IMPL_OPCODE_CALL(RestoreErrorLevel)

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(CheckClsReifiedGenericMismatch)
IMPL_OPCODE_CALL(CheckFunReifiedGenericMismatch)
IMPL_OPCODE_CALL(RaiseErrorOnInvalidIsAsExpressionType)
IMPL_OPCODE_CALL(RaiseError)
IMPL_OPCODE_CALL(RaiseMissingArg)
IMPL_OPCODE_CALL(RaiseTooManyArg)
IMPL_OPCODE_CALL(RaiseNotice)
IMPL_OPCODE_CALL(RaiseUndefProp)
IMPL_OPCODE_CALL(RaiseUninitLoc)
IMPL_OPCODE_CALL(RaiseWarning)
IMPL_OPCODE_CALL(RaiseRxCallViolation)
IMPL_OPCODE_CALL(ThrowArithmeticError)
IMPL_OPCODE_CALL(ThrowArrayIndexException)
IMPL_OPCODE_CALL(ThrowArrayKeyException)
IMPL_OPCODE_CALL(ThrowAsTypeStructException)
IMPL_OPCODE_CALL(ThrowCallReifiedFunctionWithoutGenerics)
IMPL_OPCODE_CALL(ThrowDivisionByZeroError)
IMPL_OPCODE_CALL(ThrowDivisionByZeroException)
IMPL_OPCODE_CALL(ThrowHasThisNeedStatic)
IMPL_OPCODE_CALL(ThrowInvalidArrayKey)
IMPL_OPCODE_CALL(ThrowInvalidOperation)
IMPL_OPCODE_CALL(ThrowMissingThis)
IMPL_OPCODE_CALL(ThrowOutOfBounds)
IMPL_OPCODE_CALL(ThrowParameterWrongType)
IMPL_OPCODE_CALL(ThrowParamRefMismatch)
IMPL_OPCODE_CALL(ThrowParamRefMismatchRange)

///////////////////////////////////////////////////////////////////////////////

}}}
