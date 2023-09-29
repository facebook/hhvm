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


#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/text-util.h"
#include "hphp/util/trace.h"

#include <folly/Random.h>

namespace HPHP::jit::irlower {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

void cgBeginCatch(IRLS& env, const IRInstruction* /*inst*/) {
  auto& v = vmain(env);

  v << landingpad{};
  emitIncStat(v, Stats::TC_CatchTrace);
}

void cgEndCatch(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  auto const data = inst->extra<EndCatch>();
  if (data->stublogue == EndCatchData::FrameMode::Stublogue) {
    assertx(data->teardown == EndCatchData::Teardown::NA);
    assertx(data->syncVMSP == EndCatchData::VMSPSyncMode::NA);
    assertx(inst->marker().prologue());

    // The caller is allowed to optimize away writes to the space reserved for
    // ActRec and inouts. In order to unwind the stack, we need to initialize
    // it to uninits. Do it in the catch blocks for inouts (which are uncommon)
    // and initialize the ActRec space in the helper (to reduce the code size).
    auto const spReg = srcLoc(env, inst, 1).reg();
    auto const defStackData = inst->src(1)->inst()->extra<DefStackData>();
    auto const numUninit = defStackData->irSPOff.offset;
    assertx(numUninit >= kNumActRecCells);
    for (auto i = kNumActRecCells; i < numUninit; ++i) {
      auto const offset = cellsToBytes(i) + TVOFF(m_type);
      v << storebi{static_cast<int8_t>(KindOfUninit), spReg[offset]};
    }

    v << syncvmsp{spReg};
    v << jmpi{tc::ustubs().endCatchStubloguePrologueHelper, vm_regs_with_sp()};
    return;
  }

  auto const helper = [&]() -> TCA {
    switch (data->teardown) {
      case EndCatchData::Teardown::None:
        assertx(data->syncVMSP == EndCatchData::VMSPSyncMode::DoNotSync);
        return tc::ustubs().endCatchSkipTeardownHelper;
      case EndCatchData::Teardown::OnlyThis:
        assertx(data->syncVMSP == EndCatchData::VMSPSyncMode::DoNotSync);
        return tc::ustubs().endCatchTeardownThisHelper;
      case EndCatchData::Teardown::Full:
        return data->syncVMSP == EndCatchData::VMSPSyncMode::Sync
          ? tc::ustubs().endCatchSyncVMSPHelper
          : tc::ustubs().endCatchHelper;
      case EndCatchData::Teardown::NA:
        always_assert(false && "Stublogue should not be emitting vasm");
    }
    not_reached();
  }();

  if (data->syncVMSP == EndCatchData::VMSPSyncMode::Sync) {
    auto const ssp = v.makeReg();
    auto const sp = srcLoc(env, inst, 1).reg();
    auto const offset = data->offset.offset;
    v << lea{sp[cellsToBytes(offset)], ssp};
    v << syncvmsp{ssp};
    v << jmpi{helper, vm_regs_with_sp()};
    return;
  }

  v << jmpi{helper, vm_regs_no_sp()};
}

void cgUnwindCheckSideExit(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);

  markRDSAccess(v, g_unwind_rds.handle());
  auto const sf = v.makeReg();
  v << cmpbim{0, rvmtl()[unwinderSideExitOff()], sf};
  fwdJcc(v, env, CC_E, sf, inst->taken());

  // doSideExit == true, so fall through to the side exit code.
  emitIncStat(v, Stats::TC_CatchSideExit);
}

void cgLdUnwinderValue(IRLS& env, const IRInstruction* inst) {
  auto& v = vmain(env);
  markRDSAccess(v, g_unwind_rds.handle());
  loadTV(v, inst->dst(), dstLoc(env, inst, 0), rvmtl()[unwinderTVOff()]);
}

void cgEnterTCUnwind(IRLS& env, const IRInstruction* inst) {
  auto const extra = inst->extra<EnterTCUnwind>();
  auto const exn = srcLoc(env, inst, 0).reg();
  auto& v = vmain(env);

  markRDSAccess(v, g_unwind_rds.handle());
  markRDSAccess(v, g_unwind_rds.handle());
  v << storebi{1, rvmtl()[unwinderSideEnterOff()]};
  v << store{exn, rvmtl()[unwinderExnOff()]};

  auto const target = [&] {
    if (extra->teardown) return tc::ustubs().endCatchHelper;
    if (inst->func()->hasThisInBody()) {
      return tc::ustubs().endCatchTeardownThisHelper;
    }
    return tc::ustubs().endCatchSkipTeardownHelper;
  }();

  v << jmpi{target, vm_regs_with_sp()};
}

IMPL_OPCODE_CALL(DebugBacktrace)

///////////////////////////////////////////////////////////////////////////////

static void raiseForbiddenDynCall(const Func* func) {
  auto dynCallable = func->isDynamicallyCallable();
  assertx(!dynCallable || RO::EvalForbidDynamicCallsWithAttr);
  auto level = func->isMethod()
    ? (func->isStatic()
        ? RO::EvalForbidDynamicCallsToClsMeth
        : RO::EvalForbidDynamicCallsToInstMeth)
    : RO::EvalForbidDynamicCallsToFunc;
  if (level <= 0) return;
  if (dynCallable && level < 2) return;

  if (auto const rate = func->dynCallSampleRate()) {
    if (folly::Random::rand32(*rate) != 0) return;
    level = 1;
  }

  auto error_msg = dynCallable ?
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITH_ATTRIBUTE :
    Strings::FUNCTION_CALLED_DYNAMICALLY_WITHOUT_ATTRIBUTE;
  if (level >= 3 || (level >= 2 && !dynCallable)) {
    std::string msg;
    string_printf(msg, error_msg, func->fullName()->data());
    throw_invalid_operation_exception(makeStaticString(msg));
  }
  raise_notice(error_msg, func->fullName()->data());
}

static void raiseForbiddenDynConstruct(const Class* cls) {
  auto level = RO::EvalForbidDynamicConstructs;
  assertx(level > 0);
  assertx(!cls->isDynamicallyConstructible());

  if (auto const rate = cls->dynConstructSampleRate()) {
    if (folly::Random::rand32(*rate) != 0) return;
    level = 1;
  }

  if (level >= 2) {
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

void cgRaiseCoeffectsCallViolation(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<FuncData>();
  cgCallHelper(vmain(env), env,
               CallSpec::direct(raiseCoeffectsCallViolationHelper),
               kVoidDest, SyncOptions::Sync,
               argGroup(env, inst).imm(data->func).ssa(0).ssa(1));
}

void cgRaiseModuleBoundaryViolation(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<OptClassAndFuncData>();
  auto const [target, args] = [&]() -> std::pair<CallSpec, ArgGroup> {
    if (inst->src(0)->isA(TFunc)) {
      using Fn = void(*)(const Class*, const Func* callee, const StringData*);
      return {
        CallSpec::direct(static_cast<Fn>(raiseModuleBoundaryViolation)),
        argGroup(env, inst).imm(data->cls).ssa(0).imm(data->func->moduleName())
      };
    };
    assertx(inst->src(0)->isA(TCls));
    using Fn = void(*)(const Class*, const StringData*);
    return {
      CallSpec::direct(static_cast<Fn>(raiseModuleBoundaryViolation)),
      argGroup(env, inst).ssa(0).imm(data->func->moduleName())
    };
  }();
  cgCallHelper(vmain(env), env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgRaiseModulePropertyViolation(IRLS& env, const IRInstruction* inst) {
  auto const data = inst->extra<ModulePropAccessData>();
  using Fn = void(*)(const Class*, const StringData* prop, const StringData*, bool);
  auto const target = CallSpec::direct(static_cast<Fn>(raiseModulePropertyViolation));
  auto const args = argGroup(env, inst)
    .imm(data->propCls)
    .imm(data->propName)
    .imm(data->caller->moduleName())
    .imm(data->is_static);
  cgCallHelper(vmain(env), env, target, kVoidDest, SyncOptions::Sync, args);
}

void cgRaiseDeploymentBoundaryViolation(IRLS& env, const IRInstruction* inst) {
  auto const target = [&]() -> CallSpec {
    if (inst->src(0)->isA(TFunc)) {
      using Fn = void(*)(const Func*);
      return CallSpec::direct(
        static_cast<Fn>(raiseDeploymentBoundaryViolation)
      );
    };
    assertx(inst->src(0)->isA(TCls));
    using Fn = void(*)(const Class*);
    return CallSpec::direct(
        static_cast<Fn>(raiseDeploymentBoundaryViolation)
      );
  }();
  cgCallHelper(
    vmain(env),
    env,
    target,
    kVoidDest,
    SyncOptions::Sync,
    argGroup(env, inst).ssa(0)
  );
}
///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(InitThrowableFileAndLine)
IMPL_OPCODE_CALL(ZeroErrorLevel)
IMPL_OPCODE_CALL(RestoreErrorLevel)

///////////////////////////////////////////////////////////////////////////////

IMPL_OPCODE_CALL(CheckClsMethFunc)
IMPL_OPCODE_CALL(CheckClsReifiedGenericMismatch)
IMPL_OPCODE_CALL(CheckClsRGSoft)
IMPL_OPCODE_CALL(CheckFunReifiedGenericMismatch)
IMPL_OPCODE_CALL(CheckInOutMismatch)
IMPL_OPCODE_CALL(CheckReadonlyMismatch)
IMPL_OPCODE_CALL(RaiseErrorOnInvalidIsAsExpressionType)
IMPL_OPCODE_CALL(RaiseCoeffectsFunParamCoeffectRulesViolation)
IMPL_OPCODE_CALL(RaiseCoeffectsFunParamTypeViolation)
IMPL_OPCODE_CALL(RaiseError)
IMPL_OPCODE_CALL(RaiseTooManyArg)
IMPL_OPCODE_CALL(RaiseNotice)
IMPL_OPCODE_CALL(ThrowUndefPropException)
IMPL_OPCODE_CALL(ThrowUninitLoc)
IMPL_OPCODE_CALL(RaiseWarning)
IMPL_OPCODE_CALL(ThrowArrayIndexException)
IMPL_OPCODE_CALL(ThrowArrayKeyException)
IMPL_OPCODE_CALL(ThrowAsTypeStructException)
IMPL_OPCODE_CALL(ThrowCallReifiedFunctionWithoutGenerics)
IMPL_OPCODE_CALL(ThrowDivisionByZeroException)
IMPL_OPCODE_CALL(ThrowHasThisNeedStatic)
IMPL_OPCODE_CALL(ThrowInOutMismatch)
IMPL_OPCODE_CALL(ThrowInvalidArrayKey)
IMPL_OPCODE_CALL(ThrowInvalidOperation)
IMPL_OPCODE_CALL(ThrowMissingArg)
IMPL_OPCODE_CALL(ThrowMissingThis)
IMPL_OPCODE_CALL(ThrowCannotModifyReadonlyCollection)
IMPL_OPCODE_CALL(ThrowLocalMustBeValueTypeException)
IMPL_OPCODE_CALL(ThrowMustBeEnclosedInReadonly)
IMPL_OPCODE_CALL(ThrowMustBeMutableException)
IMPL_OPCODE_CALL(ThrowMustBeReadonlyException)
IMPL_OPCODE_CALL(ThrowMustBeValueTypeException)
IMPL_OPCODE_CALL(ThrowOutOfBounds)
IMPL_OPCODE_CALL(ThrowParameterWrongType)
IMPL_OPCODE_CALL(ThrowReadonlyMismatch)
IMPL_OPCODE_CALL(RaiseImplicitContextStateInvalid)

///////////////////////////////////////////////////////////////////////////////

}
