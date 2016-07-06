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

#include "hphp/runtime/vm/jit/code-gen-x64.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <vector>
#ifndef _MSC_VER
#include <unwind.h>
#endif

#include <folly/ScopeGuard.h>
#include <folly/Format.h>
#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/util/abi-cxx.h"

#include "hphp/runtime/base/apc-local-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
//#include "hphp/runtime/base/typed-value.h"
//#include "hphp/runtime/base/struct-array.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/irlower-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stack-overflow.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_wait-handle.h"
#include "hphp/runtime/ext/std/ext_std_closure.h"
#include "hphp/runtime/ext/collections/hash-collection.h"
#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#define rvmsp() DontUseRVmSpInThisFile

namespace HPHP { namespace jit { namespace irlower {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * mcg that hardcode these registers.)
 */
using namespace jit::reg;

///////////////////////////////////////////////////////////////////////////////

const char* getContextName(const Class* ctx) {
  return ctx ? ctx->name()->data() : ":anonymous:";
}

///////////////////////////////////////////////////////////////////////////////

template<class Then>
void ifNonPersistent(Vout& v, Type ty, Vloc loc, Then then) {
  if (!ty.maybe(TPersistent)) {
    then(v); // non-persistent check below will always succeed
    return;
  }

  auto const sf = v.makeReg();
  v << cmplim{0, loc.reg()[FAST_REFCOUNT_OFFSET], sf};
  static_assert(UncountedValue < 0 && StaticValue < 0, "");
  ifThen(v, CC_GE, sf, then);
}

template<class Then>
void ifRefCountedType(Vout& v, Vout& vtaken, Type ty, Vloc loc, Then then) {
  if (!ty.maybe(TCounted)) return;
  if (ty.isKnownDataType()) {
    if (isRefcountedType(ty.toDataType())) then(v);
    return;
  }
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfRefCountThreshold, loc.reg(1));
  unlikelyIfThen(v, vtaken, CC_NLE, sf, then);
}

template<class Then>
void ifRefCountedNonPersistent(Vout& v, Type ty, Vloc loc, Then then) {
  ifRefCountedType(v, v, ty, loc, [&] (Vout& v) {
    ifNonPersistent(v, ty, loc, then);
  });
}

///////////////////////////////////////////////////////////////////////////////

void debug_trashsp(Vout& v) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << syncvmsp{v.cns(0x42)};
  }
}

void maybe_syncsp(Vout& v, BCMarker marker, Vreg irSP, IRSPRelOffset off) {
  if (!marker.resumed()) {
    debug_trashsp(v);
    return;
  }
  auto const sp = v.makeReg();
  v << lea{irSP[cellsToBytes(off.offset)], sp};
  v << syncvmsp{sp};
}

RegSet cross_trace_args(BCMarker marker) {
  return marker.resumed() ? cross_trace_regs_resumed() : cross_trace_regs();
}

void prepare_return_regs(Vout& v, SSATmp* retVal, Vloc retLoc,
                         folly::Optional<AuxUnion> aux) {
  auto const type = [&] {
    auto const mask = [&] { return uint64_t{(*aux).u_raw} << 32; };

    if (!retLoc.hasReg(1)) {
      auto const dt = retVal->type().toDataType();
      return aux ? v.cns(dt | mask()) : v.cns(dt);
    }
    auto const type = retLoc.reg(1);

    if (!aux) {
      auto const ret = v.makeReg();
      v << copy{type, ret};
      return ret;
    }

    auto const extended = v.makeReg();
    auto const result = v.makeReg();

    v << movzbq{type, extended};
    v << orq{extended, v.cns(mask()), result, v.makeReg()};
    return result;
  }();
  auto const data = zeroExtendIfBool(v, retVal, retLoc.reg(0));

  v << syncvmret{data, type};
}

///////////////////////////////////////////////////////////////////////////////

static void emitCheckSurpriseFlagsEnter(Vout& v, Vout& vcold,
                                        Vreg fp, Vreg rds,
                                        Fixup fixup, Vlabel catchBlock) {
  auto cold = vcold.makeBlock();
  auto done = v.makeBlock();

  auto const sf = v.makeReg();
  v << cmpqm{fp, rds[rds::kSurpriseFlagsOff], sf};
  v << jcc{CC_NBE, sf, {done, cold}};

  v = done;
  vcold = cold;

  auto const call = CallSpec::stub(mcg->ustubs().functionEnterHelper);
  auto const args = v.makeVcallArgs({});
  vcold << vinvoke{call, args, v.makeTuple({}), {done, catchBlock}, fixup};
}

ptrdiff_t genOffset(bool isAsync) {
  return isAsync ? AsyncGenerator::objectOff() : Generator::objectOff();
}

//////////////////////////////////////////////////////////////////////

} // unnamed namespace

//////////////////////////////////////////////////////////////////////

Vloc CodeGenerator::srcLoc(const IRInstruction* inst, unsigned i) const {
  return irlower::srcLoc(m_state, inst, i);
}

Vloc CodeGenerator::dstLoc(const IRInstruction* inst, unsigned i) const {
  return irlower::dstLoc(m_state, inst, i);
}

ArgGroup CodeGenerator::argGroup(const IRInstruction* inst) const {
  return irlower::argGroup(m_state, inst);
}

void CodeGenerator::cgInst(IRInstruction* inst) {
  SCOPE_ASSERT_DETAIL("cgInst") { return inst->toString(); };

  switch (inst->op()) {
#define O(name, dsts, srcs, flags)                                \
    case name: FTRACE(7, "cg" #name "\n");                        \
      cg ## name (inst);                                          \
      break;
    IR_OPCODES
#undef O
    default:
      always_assert(false);
  }
  auto& v = vmain();
  if (inst->isBlockEnd() && !v.closed()) {
    if (auto next = inst->next()) {
      v << jmp{m_state.labels[next]};
    } else {
      v << ud2{}; // or end?
    }
  }
}

#define NOOP_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction*) {}

#define CALL_OPCODE(opcode)                           \
  void CodeGenerator::cg##opcode(IRInstruction* i) {  \
    cgCallNative(vmain(), i);                         \
  }

#define DELEGATE_OPCODE(opcode)                       \
  void CodeGenerator::cg##opcode(IRInstruction* i) {  \
    irlower::cg##opcode(m_state, i);                  \
  }

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(Nop)
NOOP_OPCODE(EndGuards)
NOOP_OPCODE(ExitPlaceholder);
NOOP_OPCODE(HintLocInner)
NOOP_OPCODE(HintStkInner)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(FinishMemberOp)
NOOP_OPCODE(BeginInlining)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(MapAddElemC)
CALL_OPCODE(ColAddNewElemC)

CALL_OPCODE(CoerceCellToBool);
CALL_OPCODE(CoerceCellToInt);
CALL_OPCODE(CoerceCellToDbl);
CALL_OPCODE(CoerceStrToDbl);
CALL_OPCODE(CoerceStrToInt);

CALL_OPCODE(ConvBoolToArr);
CALL_OPCODE(ConvDblToArr);
CALL_OPCODE(ConvIntToArr);
CALL_OPCODE(ConvObjToArr);
CALL_OPCODE(ConvStrToArr);
CALL_OPCODE(ConvVecToArr);
CALL_OPCODE(ConvDictToArr);
CALL_OPCODE(ConvCellToArr);

CALL_OPCODE(ConvCellToBool);

CALL_OPCODE(ConvArrToDbl);
CALL_OPCODE(ConvObjToDbl);
CALL_OPCODE(ConvStrToDbl);
CALL_OPCODE(ConvResToDbl);
CALL_OPCODE(ConvCellToDbl);

CALL_OPCODE(ConvArrToInt);
CALL_OPCODE(ConvObjToInt);
CALL_OPCODE(ConvStrToInt);
CALL_OPCODE(ConvResToInt);
CALL_OPCODE(ConvCellToInt);

CALL_OPCODE(ConvCellToObj);

CALL_OPCODE(ConvDblToStr);
CALL_OPCODE(ConvIntToStr);
CALL_OPCODE(ConvObjToStr);
CALL_OPCODE(ConvResToStr);
CALL_OPCODE(ConvCellToStr);

CALL_OPCODE(ConcatStrStr);
CALL_OPCODE(ConcatStrInt);
CALL_OPCODE(ConcatIntStr);
CALL_OPCODE(ConcatStr3);
CALL_OPCODE(ConcatStr4);

CALL_OPCODE(GtStr);
CALL_OPCODE(GteStr);
CALL_OPCODE(LtStr);
CALL_OPCODE(LteStr);
CALL_OPCODE(EqStr);
CALL_OPCODE(NeqStr);
CALL_OPCODE(SameStr);
CALL_OPCODE(NSameStr);
CALL_OPCODE(CmpStr);
CALL_OPCODE(GtStrInt);
CALL_OPCODE(GteStrInt);
CALL_OPCODE(LtStrInt);
CALL_OPCODE(LteStrInt);
CALL_OPCODE(EqStrInt);
CALL_OPCODE(NeqStrInt);
CALL_OPCODE(CmpStrInt);
CALL_OPCODE(GtObj);
CALL_OPCODE(GteObj);
CALL_OPCODE(LtObj);
CALL_OPCODE(LteObj);
CALL_OPCODE(EqObj);
CALL_OPCODE(NeqObj);
CALL_OPCODE(CmpObj);
CALL_OPCODE(GtArr);
CALL_OPCODE(GteArr);
CALL_OPCODE(LtArr);
CALL_OPCODE(LteArr);
CALL_OPCODE(EqArr);
CALL_OPCODE(NeqArr);
CALL_OPCODE(SameArr);
CALL_OPCODE(NSameArr);
CALL_OPCODE(CmpArr);
CALL_OPCODE(GtRes);
CALL_OPCODE(GteRes);
CALL_OPCODE(LtRes);
CALL_OPCODE(LteRes);
CALL_OPCODE(CmpRes);

CALL_OPCODE(ThrowInvalidOperation);
CALL_OPCODE(HasToString);

CALL_OPCODE(CreateCont)
CALL_OPCODE(CreateAFWH)
CALL_OPCODE(CreateAFWHNoVV)
CALL_OPCODE(CreateSSWH)
CALL_OPCODE(AFWHPrepareChild)
CALL_OPCODE(ABCUnblock)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewDictArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(AllocPackedArray)
CALL_OPCODE(AllocVecArray)
CALL_OPCODE(Clone)
CALL_OPCODE(AllocObj)
CALL_OPCODE(InitProps)
CALL_OPCODE(InitSProps)
CALL_OPCODE(DebugBacktrace)
CALL_OPCODE(InitThrowableFileAndLine)
CALL_OPCODE(RegisterLiveObj)
CALL_OPCODE(LdClsCtor)
CALL_OPCODE(LookupClsRDS)
CALL_OPCODE(PrintStr)
CALL_OPCODE(PrintInt)
CALL_OPCODE(PrintBool)
CALL_OPCODE(DbgAssertPtr)
CALL_OPCODE(LdSwitchDblIndex)
CALL_OPCODE(LdSwitchStrIndex)
CALL_OPCODE(LdSwitchObjIndex)
CALL_OPCODE(VerifyParamCallable)
CALL_OPCODE(VerifyParamFail)
CALL_OPCODE(VerifyParamFailHard)
CALL_OPCODE(VerifyRetCallable)
CALL_OPCODE(VerifyRetFail)
CALL_OPCODE(RaiseUninitLoc)
CALL_OPCODE(RaiseUndefProp)
CALL_OPCODE(RaiseMissingArg)
CALL_OPCODE(RaiseError)
CALL_OPCODE(RaiseWarning)
CALL_OPCODE(RaiseNotice)
CALL_OPCODE(RaiseArrayIndexNotice)
CALL_OPCODE(RaiseArrayKeyNotice)
CALL_OPCODE(IncStatGrouped)
CALL_OPCODE(LdClosureStaticLoc)
CALL_OPCODE(MapIdx)
CALL_OPCODE(LdClsPropAddrOrNull)
CALL_OPCODE(LdClsPropAddrOrRaise)
CALL_OPCODE(LdGblAddrDef)

CALL_OPCODE(MethodExists)

// Vector instruction helpers
CALL_OPCODE(StringGet)
CALL_OPCODE(BindElem)
CALL_OPCODE(SetWithRefElem)
CALL_OPCODE(SetOpElem)
CALL_OPCODE(IncDecElem)
CALL_OPCODE(SetNewElem)
CALL_OPCODE(SetNewElemArray)
CALL_OPCODE(BindNewElem)
CALL_OPCODE(VectorIsset)
CALL_OPCODE(PairIsset)
CALL_OPCODE(ThrowOutOfBounds)
CALL_OPCODE(ThrowArithmeticError)
CALL_OPCODE(ThrowDivisionByZeroError)

CALL_OPCODE(ZeroErrorLevel)
CALL_OPCODE(RestoreErrorLevel)

CALL_OPCODE(Count)

CALL_OPCODE(SuspendHookE)
CALL_OPCODE(SuspendHookR)
CALL_OPCODE(ReturnHook)

CALL_OPCODE(OODeclExists)

CALL_OPCODE(GetMemoKey)

DELEGATE_OPCODE(Box)
DELEGATE_OPCODE(BoxPtr)
DELEGATE_OPCODE(UnboxPtr)

DELEGATE_OPCODE(AddInt)
DELEGATE_OPCODE(SubInt)
DELEGATE_OPCODE(MulInt)
DELEGATE_OPCODE(AddIntO)
DELEGATE_OPCODE(SubIntO)
DELEGATE_OPCODE(MulIntO)
DELEGATE_OPCODE(AddDbl)
DELEGATE_OPCODE(SubDbl)
DELEGATE_OPCODE(MulDbl)
DELEGATE_OPCODE(DivDbl)
DELEGATE_OPCODE(DivInt)
DELEGATE_OPCODE(Mod)
DELEGATE_OPCODE(Floor)
DELEGATE_OPCODE(Ceil)
DELEGATE_OPCODE(AbsDbl)
DELEGATE_OPCODE(Sqrt)
DELEGATE_OPCODE(AndInt)
DELEGATE_OPCODE(OrInt)
DELEGATE_OPCODE(XorInt)
DELEGATE_OPCODE(Shl)
DELEGATE_OPCODE(Shr)
DELEGATE_OPCODE(XorBool)

DELEGATE_OPCODE(GtInt)
DELEGATE_OPCODE(GteInt)
DELEGATE_OPCODE(LtInt)
DELEGATE_OPCODE(LteInt)
DELEGATE_OPCODE(EqInt)
DELEGATE_OPCODE(NeqInt)
DELEGATE_OPCODE(CmpInt)
DELEGATE_OPCODE(GtBool)
DELEGATE_OPCODE(GteBool)
DELEGATE_OPCODE(LtBool)
DELEGATE_OPCODE(LteBool)
DELEGATE_OPCODE(EqBool)
DELEGATE_OPCODE(NeqBool)
DELEGATE_OPCODE(CmpBool)
DELEGATE_OPCODE(GtDbl)
DELEGATE_OPCODE(GteDbl)
DELEGATE_OPCODE(LtDbl)
DELEGATE_OPCODE(LteDbl)
DELEGATE_OPCODE(EqDbl)
DELEGATE_OPCODE(NeqDbl)
DELEGATE_OPCODE(CmpDbl)
DELEGATE_OPCODE(SameObj)
DELEGATE_OPCODE(NSameObj)
DELEGATE_OPCODE(EqRes)
DELEGATE_OPCODE(NeqRes)

DELEGATE_OPCODE(EqCls)
DELEGATE_OPCODE(InstanceOf)
DELEGATE_OPCODE(InstanceOfIface)
DELEGATE_OPCODE(InstanceOfIfaceVtable)
DELEGATE_OPCODE(ExtendsClass)
DELEGATE_OPCODE(InstanceOfBitmask)
DELEGATE_OPCODE(NInstanceOfBitmask)
DELEGATE_OPCODE(InterfaceSupportsArr)
DELEGATE_OPCODE(InterfaceSupportsStr)
DELEGATE_OPCODE(InterfaceSupportsInt)
DELEGATE_OPCODE(InterfaceSupportsDbl)

DELEGATE_OPCODE(CheckType)
DELEGATE_OPCODE(CheckTypeMem)
DELEGATE_OPCODE(CheckLoc)
DELEGATE_OPCODE(CheckStk)
DELEGATE_OPCODE(CheckRefInner)
DELEGATE_OPCODE(IsType)
DELEGATE_OPCODE(IsNType)
DELEGATE_OPCODE(IsTypeMem)
DELEGATE_OPCODE(IsNTypeMem)
DELEGATE_OPCODE(IsScalarType)

DELEGATE_OPCODE(BaseG)
DELEGATE_OPCODE(PropX)
DELEGATE_OPCODE(PropDX)
DELEGATE_OPCODE(PropQ)
DELEGATE_OPCODE(CGetProp)
DELEGATE_OPCODE(CGetPropQ)
DELEGATE_OPCODE(VGetProp)
DELEGATE_OPCODE(BindProp)
DELEGATE_OPCODE(SetProp)
DELEGATE_OPCODE(UnsetProp)
DELEGATE_OPCODE(SetOpProp)
DELEGATE_OPCODE(IncDecProp)
DELEGATE_OPCODE(IssetProp)
DELEGATE_OPCODE(EmptyProp)
DELEGATE_OPCODE(ElemX)
DELEGATE_OPCODE(ElemDX)
DELEGATE_OPCODE(ElemUX)
DELEGATE_OPCODE(CGetElem)
DELEGATE_OPCODE(VGetElem)
DELEGATE_OPCODE(SetElem)
DELEGATE_OPCODE(UnsetElem)
DELEGATE_OPCODE(IssetElem)
DELEGATE_OPCODE(EmptyElem)
DELEGATE_OPCODE(ProfileMixedArrayOffset)
DELEGATE_OPCODE(CheckMixedArrayOffset)
DELEGATE_OPCODE(CheckArrayCOW)
DELEGATE_OPCODE(ElemArray)
DELEGATE_OPCODE(ElemArrayW)
DELEGATE_OPCODE(ElemArrayD)
DELEGATE_OPCODE(ElemArrayU)
DELEGATE_OPCODE(ElemMixedArrayK)
DELEGATE_OPCODE(ArrayGet)
DELEGATE_OPCODE(MixedArrayGetK)
DELEGATE_OPCODE(ArraySet)
DELEGATE_OPCODE(ArraySetRef)
DELEGATE_OPCODE(ArrayIsset)
DELEGATE_OPCODE(ArrayIdx)
DELEGATE_OPCODE(MapGet)
DELEGATE_OPCODE(MapSet)
DELEGATE_OPCODE(MapIsset)

DELEGATE_OPCODE(LdCns)
DELEGATE_OPCODE(LookupCns)
DELEGATE_OPCODE(LookupCnsE)
DELEGATE_OPCODE(LookupCnsU)
DELEGATE_OPCODE(LdClsCns)
DELEGATE_OPCODE(InitClsCns)

DELEGATE_OPCODE(IterInit)
DELEGATE_OPCODE(IterInitK)
DELEGATE_OPCODE(WIterInit)
DELEGATE_OPCODE(WIterInitK)
DELEGATE_OPCODE(MIterInit)
DELEGATE_OPCODE(MIterInitK)
DELEGATE_OPCODE(IterNext)
DELEGATE_OPCODE(IterNextK)
DELEGATE_OPCODE(WIterNext)
DELEGATE_OPCODE(WIterNextK)
DELEGATE_OPCODE(MIterNext)
DELEGATE_OPCODE(MIterNextK)
DELEGATE_OPCODE(IterFree)
DELEGATE_OPCODE(MIterFree)
DELEGATE_OPCODE(CIterFree)
DELEGATE_OPCODE(DecodeCufIter)

#undef NOOP_OPCODE
#undef DELEGATE_OPCODE

///////////////////////////////////////////////////////////////////////////////

Vlabel CodeGenerator::label(Block* b) {
  return irlower::label(m_state, b);
}

void CodeGenerator::emitFwdJcc(Vout& v, ConditionCode cc, Vreg sf,
                               Block* target) {
  irlower::fwdJcc(v, m_state, cc, sf, target);
}

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgDefSP(IRInstruction* inst) {
  auto const sp = dstLoc(inst, 0).reg();
  auto& v = vmain();

  if (inst->marker().resumed()) {
    v << defvmsp{sp};
    return;
  }

  auto const fp = srcLoc(inst, 0).reg();
  v << lea{fp[-cellsToBytes(inst->extra<DefSP>()->offset.offset)], sp};
}

void CodeGenerator::cgCheckNullptr(IRInstruction* inst) {
  if (!inst->taken()) return;
  auto reg = srcLoc(inst, 0).reg(0);
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testq{reg, reg, sf};
  v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckNonNull(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  auto taken  = inst->taken();
  assertx(taken);

  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testq{srcReg, srcReg, sf};
  emitFwdJcc(v, CC_Z, sf, taken);
  v << copy{srcReg, dstReg};
}

void CodeGenerator::cgAssertNonNull(IRInstruction* inst) {
  auto& v = vmain();
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const sf = v.makeReg();
    v << testq{srcReg, srcReg, sf};
    ifThen(v, CC_Z, sf, [&](Vout& v) {
      v << ud2{};
    });
  }
  v << copy{srcReg, dstReg};
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  copyTV(vmain(), srcLoc(inst, 0), dstLoc(inst, 0), inst->dst()->type());
}

void CodeGenerator::cgLdUnwinderValue(IRInstruction* inst) {
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), rvmtl()[unwinderTVOff()]);
}

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  auto& v = vmain();
  auto const callType = m_state.catch_calls[inst->block()];
  always_assert(callType != CatchCall::Uninit &&
                "Tried to emit BeginCatch with Uninit call type. "
                "Catch blocks must be emitted after their predecessors.");

  v << landingpad{callType == CatchCall::PHP};
  emitIncStat(v, Stats::TC_CatchTrace);
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  auto& v = vmain();
  // endCatchHelper only expects rvmtl() and rvmfp() to be live.
  v << jmpi{mcg->ustubs().endCatchHelper, rvmtl() | rvmfp()};
}

void CodeGenerator::cgUnwindCheckSideExit(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmpbim{0, rvmtl()[unwinderSideExitOff()], sf};

  auto done = v.makeBlock();
  v << jcc{CC_E, sf, {done, label(inst->taken())}};
  v = done;

  // doSideExit == true, so fall through to the side exit code
  emitIncStat(v, Stats::TC_CatchSideExit);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgHalt(IRInstruction* inst) {
  vmain() << ud2{};
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgCallNative(Vout& v, IRInstruction* inst) {
  return irlower::cgCallNative(v, m_state, inst);
}

CallDest CodeGenerator::callDest(Vreg reg0) const {
  return irlower::callDest(reg0);
}

CallDest CodeGenerator::callDest(Vreg reg0, Vreg reg1) const {
  return irlower::callDest(reg0, reg1);
}

CallDest CodeGenerator::callDest(const IRInstruction* inst) const {
  return irlower::callDest(m_state, inst);
}

CallDest CodeGenerator::callDestTV(const IRInstruction* inst) const {
  return irlower::callDestTV(m_state, inst);
}

CallDest CodeGenerator::callDestDbl(const IRInstruction* inst) const {
  return irlower::callDestDbl(m_state, inst);
}

void CodeGenerator::cgCallHelper(Vout& v, CallSpec call,
                                 const CallDest& dstInfo,
                                 SyncOptions sync,
                                 const ArgGroup& args) {
  irlower::cgCallHelper(v, m_state, call, dstInfo, sync, args);
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  always_assert(inst->src(0)->numWords() == inst->dst(0)->numWords());
  copyTV(vmain(), srcLoc(inst, 0), dstLoc(inst, 0), inst->dst()->type());
}

///////////////////////////////////////////////////////////////////////////////

void CodeGenerator::cgEqFunc(IRInstruction* inst) {
  auto const dst  = dstLoc(inst, 0).reg();
  auto const src1 = srcLoc(inst, 0).reg();
  auto const src2 = srcLoc(inst, 1).reg();

  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpLowPtr<Func>(v, sf, src2, src1);
  v << setcc{CC_E, sf, dst};
}

void CodeGenerator::cgConvDblToInt(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  constexpr uint64_t maxULongAsDouble  = 0x43F0000000000000LL;
  constexpr uint64_t maxLongAsDouble   = 0x43E0000000000000LL;

  auto rIndef = v.cns(0x8000000000000000L);
  auto dst1 = v.makeReg();
  v << cvttsd2siq{srcReg, dst1};
  auto const sf = v.makeReg();
  v << cmpq{rIndef, dst1, sf};
  unlikelyCond(v, vcold(), CC_E, sf, dstReg, [&](Vout& v) {
    // result > max signed int or unordered
    auto const sf = v.makeReg();
    v << ucomisd{v.cns(0), srcReg, sf};
    return cond(v, CC_NB, sf, v.makeReg(), [&](Vout& v) {
      return dst1;
    }, [&](Vout& v) {
      // src0 > 0 (CF = 1 -> less than 0 or unordered)
      return cond(v, CC_P, sf, v.makeReg(), [&](Vout& v) {
        // PF = 1 -> unordered, i.e., we are doing an int cast of NaN. PHP5
        // didn't formally define this, but observationally returns the
        // truncated value (i.e., what dst1 currently holds). PHP7 formally
        // defines this case to return 0.
        if (RuntimeOption::PHP7_IntSemantics) {
          return v.cns(0);
        } else {
          return dst1;
        }
      }, [&](Vout& v) {
        auto const sf = v.makeReg();
        v << ucomisd{v.cns(maxULongAsDouble), srcReg, sf};
        return cond(v, CC_B, sf, v.makeReg(), [&](Vout& v) { // src0 > ULONG_MAX
            return v.cns(0);
        }, [&](Vout& v) {
          // 0 < src0 <= ULONG_MAX
          // we know that LONG_MAX < src0 <= UINT_MAX, therefore,
          // 0 < src0 - ULONG_MAX <= LONG_MAX
          auto tmp_sub = v.makeReg();
          auto tmp_int = v.makeReg();
          auto dst5 = v.makeReg();
          v << subsd{v.cns(maxLongAsDouble), srcReg, tmp_sub};
          v << cvttsd2siq{tmp_sub, tmp_int};

          // We want to simulate integer overflow so we take the resulting
          // integer and flip its sign bit (NB: we don't use orq here
          // because it's possible that src0 == LONG_MAX in which case
          // cvttsd2siq will yield an indefiniteInteger, which we would
          // like to make zero)
          v << xorq{rIndef, tmp_int, dst5, v.makeReg()};
          return dst5;
        });
      });
    });
  }, [&](Vout& v) {
    return dst1;
  });
}

void CodeGenerator::cgConvDblToBool(IRInstruction* inst) {
  auto dst = dstLoc(inst, 0).reg();
  auto src = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto t1 = v.makeReg();
  auto const sf = v.makeReg();
  v << shlqi{1, src, t1, sf}; // 0.0 stays zero and -0.0 is now 0.0
  v << setcc{CC_NE, sf, dst}; // lower byte becomes 1 if dstReg != 0
}

void CodeGenerator::cgConvIntToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testq{srcReg, srcReg, sf};
  v << setcc{CC_NE, sf, dstReg};
}

void CodeGenerator::cgConvArrToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  auto const sf = v.makeReg();
  v << cmplim{0, srcReg[ArrayData::offsetofSize()], sf};
  unlikelyCond(v, vcold(), CC_S, sf, dstReg,
    [&](Vout& v) {
      auto vsize = v.makeReg();
      auto dst1 = v.makeReg();
      cgCallHelper(v, CallSpec::method(&ArrayData::vsize),
                   callDest(vsize), SyncOptions::None,
                   argGroup(inst).ssa(0));
      auto const sf = v.makeReg();
      v << testl{vsize, vsize, sf};
      v << setcc{CC_NZ, sf, dst1};
      return dst1;
    },
    [&](Vout& v) {
      auto dst2 = v.makeReg();
      v << setcc{CC_NZ, sf, dst2};
      return dst2;
    }
  );
}

void CodeGenerator::cgIsCol(IRInstruction* inst) {
  assertx(inst->src(0)->type() <= TObj);
  auto const rdst = dstLoc(inst, 0).reg();
  auto const rsrc = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testwim{ObjectData::IsCollection, rsrc[ObjectData::attributeOff()], sf};
  v << setcc{CC_NE, sf, rdst};
}

void CodeGenerator::cgColIsEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assertx(ty < TObj &&
         ty.clsSpec().cls() &&
         ty.clsSpec().cls()->isCollectionClass());
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmplim{0, srcLoc(inst, 0).reg()[collections::FAST_SIZE_OFFSET], sf};
  v << setcc{CC_E, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgColIsNEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assertx(ty < TObj &&
         ty.clsSpec().cls() &&
         ty.clsSpec().cls()->isCollectionClass());
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmplim{0, srcLoc(inst, 0).reg()[collections::FAST_SIZE_OFFSET], sf};
  v << setcc{CC_NE, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgConvObjToBool(IRInstruction* inst) {
  auto const rdst = dstLoc(inst, 0).reg();
  auto const rsrc = srcLoc(inst, 0).reg();
  auto& v = vmain();

  auto const sf = v.makeReg();
  v << testwim{ObjectData::CallToImpl, rsrc[ObjectData::attributeOff()], sf};
  unlikelyCond(v, vcold(), CC_NZ, sf, rdst,
    [&] (Vout& v) {
      auto const sf = v.makeReg();
      v << testwim{ObjectData::IsCollection, rsrc[ObjectData::attributeOff()],
                   sf};
      return cond(v, CC_NZ, sf, v.makeReg(),
        [&] (Vout& v) { // rsrc points to native collection
          auto dst2 = v.makeReg();
          auto const sf = v.makeReg();
          v << cmplim{0, rsrc[collections::FAST_SIZE_OFFSET], sf};
          v << setcc{CC_NE, sf, dst2}; // true iff size not zero
          return dst2;
        }, [&] (Vout& v) { // rsrc is not a native collection
          auto dst3 = v.makeReg();
          cgCallHelper(v,
            CallSpec::method(&ObjectData::toBoolean),
            CallDest{DestType::Byte, dst3},
            SyncOptions::Sync,
            argGroup(inst).ssa(0));
          return dst3;
        });
    }, [&] (Vout& v) {
      return v.cns(true);
    }
  );
}

void CodeGenerator::cgConvStrToBool(IRInstruction* inst) {
  auto const dst = dstLoc(inst, 0).reg();
  auto const src = srcLoc(inst, 0).reg();
  auto& v = vmain();

  auto const sf = v.makeReg();

  v << cmplim{1, src[StringData::sizeOff()], sf};
  unlikelyCond(v, vcold(), CC_E, sf, dst, [&] (Vout& v) {
    // Unlikely case is we end up having to check whether the first byte of the
    // string is equal to '0'.
    auto const dst = v.makeReg();
    auto const sd  = v.makeReg();
    auto const sf  = v.makeReg();
    v << load{src[StringData::dataOff()], sd};
    v << cmpbim{'0', sd[0], sf};
    v << setcc{CC_NE, sf, dst};
    return dst;
  }, [&] (Vout& v) {
    // Common case is we have an empty string or a string with size bigger than
    // one.
    auto const dst = v.makeReg();
    v << setcc{CC_G, sf, dst};
    return dst;
  });
}

void CodeGenerator::emitConvBoolOrIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  assertx(src->isA(TBool) || src->isA(TInt));
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  // cvtsi2sd doesn't modify the high bits of its target, which can
  // cause false dependencies to prevent register renaming from kicking
  // in. Break the dependency chain by zeroing out the XMM reg.
  auto& v = vmain();
  auto s2 = zeroExtendIfBool(v, src, srcReg);
  v << cvtsi2sd{s2, dstReg};
}

void CodeGenerator::cgConvBoolToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvIntToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvBoolToInt(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  vmain() << movzbq{srcReg, dstReg};
}

void CodeGenerator::cgOrdStr(IRInstruction* inst) {
  auto& v = vmain();
  auto const sd = v.makeReg();
  // sd = StringData->m_data;
  v << load{srcLoc(inst, 0).reg()[StringData::dataOff()], sd};
  // dst = (unsigned char)sd[0];
  v << loadzbq{sd[0], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgOrdStrIdx(IRInstruction* inst) {
  auto& v = vmain();
  auto const strReg = srcLoc(inst, 0).reg();
  auto const sd = v.makeReg();
  auto const strLen = v.makeReg();
  auto const srcOff = srcLoc(inst, 1).reg();
  auto const sf = v.makeReg();

  v << loadzlq{strReg[StringData::sizeOff()], strLen};
  v << cmpq{srcOff, strLen, sf};
  unlikelyCond(v, vcold(), CC_B, sf, dstLoc(inst, 0).reg(),
               [&] (Vout& v) {
                 cgCallHelper(
                   v,
                   CallSpec::direct(MInstrHelpers::stringGetI),
                   kVoidDest,
                   SyncOptions::Sync,
                   argGroup(inst).ssa(0).ssa(1)
                 );
                 return v.cns(0);
               },
               [&] (Vout& v) {
                 auto const dst = v.makeReg();
                 // sd = StringData->m_data;
                 v << load{strReg[StringData::dataOff()], sd};
                 // dst = (unsigned char)sd[0];
                 v << loadzbq{sd[srcOff], dst};
                 return dst;
               });
}

static const StaticString s_1("1");

void CodeGenerator::cgConvBoolToStr(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto f = v.cns(staticEmptyString());
  auto t = v.cns(s_1.get());
  auto const sf = v.makeReg();
  v << testb{srcReg, srcReg, sf};
  v << cmovq{CC_NZ, sf, f, t, dstReg};
}

void CodeGenerator::cgConvClsToCctx(IRInstruction* inst) {
  auto const sreg = srcLoc(inst, 0).reg();
  auto const dreg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << orqi{1, sreg, dreg, v.makeReg()};
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  auto const dst = dstLoc(inst, 0).reg();
  auto const extra = inst->extra<LdFuncCached>();
  auto const ch = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain();
  auto const sf = v.makeReg();

  auto const call_helper = [&] (Vout& v) {
    auto const ptr = v.makeReg();
    const Func* (*const func)(const StringData*) = lookupUnknownFunc;
    cgCallHelper(
      v,
      CallSpec::direct(func),
      callDest(ptr),
      SyncOptions::Sync,
      argGroup(inst).immPtr(extra->name)
    );
    return ptr;
  };

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(), CC_NE, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) {
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));
        return ptr;
      }
    );
  } else {
    auto const ptr = v.makeReg();
    emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));
    v << testq{ptr, ptr, sf};
    unlikelyCond(
      v, vcold(), CC_Z, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) { return ptr; }
    );
  }
}

void CodeGenerator::cgLdFuncCachedU(IRInstruction* inst) {
  auto const dst = dstLoc(inst, 0).reg();
  auto const extra = inst->extra<LdFuncCachedU>();
  auto const ch = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain();

  auto const call_helper = [&] (Vout& v) {
    // If we get here, things are going to be slow anyway, so do all the
    // autoloading logic in lookupFallbackFunc instead of ASM
    auto const ptr = v.makeReg();
    const Func* (*const func)(const StringData*, const StringData*) =
        lookupFallbackFunc;
    cgCallHelper(v, CallSpec::direct(func), callDest(ptr),
      SyncOptions::Sync,
      argGroup(inst)
        .immPtr(extra->name)
        .immPtr(extra->fallback)
    );
    return ptr;
  };

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(), CC_NE, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) {
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));
        return ptr;
      }
    );
  } else {
    auto const ptr = v.makeReg();
    auto const sf = v.makeReg();
    emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Func>));
    v << testq{ptr, ptr, sf};
    unlikelyCond(
      v, vcold(), CC_Z, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) { return ptr; }
    );
  }
}

void CodeGenerator::cgLdFuncCachedSafe(IRInstruction* inst) {
  auto const dst = dstLoc(inst, 0).reg();
  auto const ch = NamedEntity::get(
    inst->extra<LdFuncCachedSafe>()->name
  )->getFuncHandle();
  auto& v = vmain();

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    emitFwdJcc(v, CC_NE, sf, inst->taken());
  }
  emitLdLowPtr(v, rvmtl()[ch], dst, sizeof(LowPtr<Func>));
}

void CodeGenerator::cgLdFunc(IRInstruction* inst) {
  auto const ch = FuncCache::alloc();
  rds::recordRds(ch, sizeof(FuncCache),
                 "FuncCache", getFunc(inst->marker())->fullName()->data());

  // raises an error if function not found
  cgCallHelper(vmain(),
               CallSpec::direct(FuncCache::lookup),
               callDest(dstLoc(inst, 0).reg()),
               SyncOptions::Sync,
               argGroup(inst).imm(ch).ssa(0/*methodName*/));
}

void CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto objReg = srcLoc(inst, 0).reg();
  emitLdObjClass(vmain(), objReg, dstReg);
}

void CodeGenerator::cgLdArrFuncCtx(IRInstruction* inst) {
  cgCallHelper(
    vmain(),
    CallSpec::direct(loadArrayFunctionContext),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .addr(srcLoc(inst, 1).reg(),
            cellsToBytes(inst->extra<LdArrFuncCtx>()->offset.offset))
      .ssa(2)
  );
}

void CodeGenerator::cgLdArrFPushCuf(IRInstruction* inst) {
  cgCallHelper(
    vmain(),
    CallSpec::direct(fpushCufHelperArray),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .addr(srcLoc(inst, 1).reg(),
            cellsToBytes(inst->extra<LdArrFPushCuf>()->offset.offset))
      .ssa(2)
  );
}

void CodeGenerator::cgLdStrFPushCuf(IRInstruction* inst) {
  cgCallHelper(
    vmain(),
    CallSpec::direct(fpushCufHelperString),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .addr(srcLoc(inst, 1).reg(),
            cellsToBytes(inst->extra<LdStrFPushCuf>()->offset.offset))
      .ssa(2)
  );
}

void CodeGenerator::cgLookupClsMethod(IRInstruction* inst) {
  cgCallHelper(
    vmain(),
    CallSpec::direct(lookupClsMethodHelper),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
      .addr(srcLoc(inst, 2).reg(),
            cellsToBytes(inst->extra<LookupClsMethod>()->offset.offset))
      .ssa(3)
  );
}

void CodeGenerator::cgLdObjMethod(IRInstruction* inst) {
  assertx(inst->taken() && inst->taken()->isCatch()); // must have catch block
  using namespace MethodCache;

  auto const clsReg    = srcLoc(inst, 0).reg();
  auto const actRecReg = srcLoc(inst, 1).reg();
  auto const extra     = inst->extra<LdObjMethodData>();
  auto& v = vmain();

  // Allocate the request-local one-way method cache for this lookup.
  auto const handle = rds::alloc<Entry, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    rds::recordRds(
      handle,
      sizeof(TypedValue),
      "MethodCache",
      getFunc(inst->marker())->fullName()->toCppString()
    );
  }

  auto const mcHandler = extra->fatal ? handlePrimeCacheInit<true>
                                      : handlePrimeCacheInit<false>;

  auto fast_path = v.makeBlock();
  auto slow_path = v.makeBlock();
  auto done = v.makeBlock();

  /*
   * The `mcprep' instruction here creates a smashable move, which serves as
   * the inline cache, or "prime cache" for the method lookup.
   *
   * On our first time through this codepath in the TC, we "prime" this cache
   * (which holds across /all/ requests) by smashing the mov immediate to hold
   * a Func* in the upper 32 bits, and a Class* in the lower 32 bits.  This is
   * not always possible (see handlePrimeCacheInit() for details), in which
   * case we smash an immediate with some low bits set, so that we always miss
   * on the inline cache when comparing against our live Class*.
   *
   * The inline cache is set up so that we always miss initially, and take the
   * slow path to initialize it.  After initialization, we also smash the slow
   * path call to point instead to a lookup routine for the out-of-line method
   * cache (allocated above).  The inline cache is guaranteed to be set only
   * once, but the one-way request-local method cache is updated on each miss.
   */
  auto func_class = v.makeReg();
  auto classptr = v.makeReg();
  v << mcprep{func_class};

  // Get the Class* part of the cache line.
  auto tmp = v.makeReg();
  v << movtql{func_class, tmp};
  v << movzlq{tmp, classptr};

  // Check the inline cache.
  auto const sf = v.makeReg();
  v << cmpq{classptr, clsReg, sf};
  v << jcc{CC_NE, sf, {fast_path, slow_path}};

  // Inline cache hit; store the value in the AR.
  v = fast_path;
  auto funcptr = v.makeReg();
  v << shrqi{32, func_class, funcptr, v.makeReg()};
  v << store{funcptr,
             actRecReg[cellsToBytes(extra->offset.offset) + AROFF(m_func)]};
  v << jmp{done};

  // Initialize the inline cache, or do a lookup in the out-of-line cache if
  // we've finished initialization and have smashed this call.
  v = slow_path;
  cgCallHelper(v,
    CallSpec::smashable(mcHandler),
    kVoidDest,
    SyncOptions::Sync,
    argGroup(inst)
      .imm(safe_cast<int32_t>(handle))
      .addr(srcLoc(inst, 1).reg(), cellsToBytes(extra->offset.offset))
      .immPtr(extra->method)
      .ssa(0/*cls*/)
      .immPtr(getClass(inst->marker()))
      .reg(func_class)
  );
  v << jmp{done};
  v = done;
}

void CodeGenerator::cgLdObjInvoke(IRInstruction* inst) {
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto& v = vmain();
  emitLdLowPtr(v, rsrc[Class::invokeOff()], rdst, sizeof(LowPtr<Func>));
  auto const sf = v.makeReg();
  v << testq{rdst, rdst, sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgLdRetVal(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), fp[AROFF(m_r)], true);
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == mcg->ustubs().callToExit) {
    return;
  }
  checkFrame(fp, sp, /*fullCheck*/ false, 0);
  assertx(sp <= (Cell*)fp || fp->resumed());
}

static Vreg adjustSPForReturn(IRLS& env, const IRInstruction* inst) {
  auto const adjust = inst->extra<RetCtrlData>()->spOffset.offset;

  auto& v = vmain(env);
  auto const sp = srcLoc(env, inst, 0).reg();
  auto const sync_sp = v.makeReg();

  v << lea{sp[cellsToBytes(adjust)], sync_sp};
  v << syncvmsp{sync_sp};

  return sync_sp;
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 1).reg();
  auto const sync_sp = adjustSPForReturn(m_state, inst);

  auto& v = vmain();

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto ripReg = v.makeReg();
    v << load{fp[AROFF(m_savedRip)], ripReg};
    auto prev_fp = v.makeReg();
    v << load{fp[AROFF(m_sfp)], prev_fp};
    v << vcall{CallSpec::direct(traceRet),
               v.makeVcallArgs({{prev_fp, sync_sp, ripReg}}), v.makeTuple({})};
  }

  prepare_return_regs(v, inst->src(2), srcLoc(inst, 2),
                      inst->extra<RetCtrl>()->aux);
  v << phpret{fp, rvmfp(), php_return_regs()};
}

void CodeGenerator::cgAsyncRetCtrl(IRInstruction* inst) {
  auto& v = vmain();
  adjustSPForReturn(m_state, inst);
  prepare_return_regs(v, inst->src(2), srcLoc(inst, 2),
                      inst->extra<AsyncRetCtrl>()->aux);
  v << leavetc{php_return_regs()};
}

void CodeGenerator::cgLdBindAddr(IRInstruction* inst) {
  auto const extra  = inst->extra<LdBindAddr>();
  auto const dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();

  // Emit service request to smash address of SrcKey into 'addr'.
  auto const addrPtr = v.allocData<TCA>();
  v << bindaddr{addrPtr, extra->sk, extra->spOff};
  v << loadqd{reinterpret_cast<uint64_t*>(addrPtr), dstReg};
}

void CodeGenerator::cgProfileSwitchDest(IRInstruction* inst) {
  auto& v = vmain();
  auto const idxReg = srcLoc(inst, 0).reg();
  auto const sf = v.makeReg();
  auto const& extra = *inst->extra<ProfileSwitchDest>();
  auto const vmtl = Vreg{rvmtl()};

  auto caseReg = v.makeReg();
  v << subq{v.cns(extra.base), idxReg, caseReg, v.makeReg()};
  v << cmpqi{extra.cases - 2, caseReg, sf};
  ifThenElse(
    v, CC_AE, sf,
    [&](Vout& v) {
      // Last vector element is the default case
      v << inclm{vmtl[extra.handle + (extra.cases - 1) * sizeof(int32_t)],
                 v.makeReg()};
    },
    [&](Vout& v) {
      v << inclm{vmtl[caseReg * 4 + extra.handle], v.makeReg()};
    }
  );
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  auto const extra    = inst->extra<JmpSwitchDest>();
  auto const marker   = inst->marker();
  auto const indexReg = srcLoc(inst, 0).reg();
  auto const invSPOff = extra->invSPOff;
  auto& v = vmain();

  maybe_syncsp(v, marker, srcLoc(inst, 1).reg(), extra->irSPOff);

  auto const table = v.allocData<TCA>(extra->cases);
  auto const t = v.makeReg();
  for (int i = 0; i < extra->cases; i++) {
    v << bindaddr{&table[i], extra->targets[i], invSPOff};
  }
  v << lead{table, t};
  v << jmpm{t[indexReg * 8], cross_trace_args(marker)};
}

void CodeGenerator::cgLdSSwitchDestFast(IRInstruction* inst) {
  auto const extra = inst->extra<LdSSwitchDestFast>();
  auto const spOff = extra->spOff;

  auto& v = vmain();
  auto const table = v.allocData<SSwitchMap>();
  // XXX(t10347945): This causes our data section to own a pointer to heap
  // memory, and we're putting bindaddrs in said heap memory.
  new (table) SSwitchMap(extra->numCases);

  for (int64_t i = 0; i < extra->numCases; ++i) {
    table->add(extra->cases[i].str, nullptr);
    auto const addr = table->find(extra->cases[i].str);
    // The addresses we're passing to bindaddr{} here live in SSwitchMap's heap
    // buffer (see comment above). They don't need to be relocated like normal
    // VdataPtrs, so bind them here.
    VdataPtr<TCA> dataPtr{nullptr};
    dataPtr.bind(addr);
    v << bindaddr{dataPtr, extra->cases[i].dest, spOff};
  }
  auto const def = v.allocData<TCA>();
  v << bindaddr{def, extra->defaultSk, spOff};
  cgCallHelper(v,
               CallSpec::direct(sswitchHelperFast),
               callDest(inst),
               SyncOptions::None,
               argGroup(inst)
                 .ssa(0)
                 .dataPtr(table)
                 .dataPtr(def));
}

static TCA sswitchHelperSlow(TypedValue typedVal,
                             const StringData** strs,
                             int numStrs,
                             TCA* jmptab) {
  auto const cell = tvToCell(&typedVal);
  for (int i = 0; i < numStrs; ++i) {
    if (cellEqual(*cell, strs[i])) return jmptab[i];
  }
  return jmptab[numStrs]; // default case
}

void CodeGenerator::cgLdSSwitchDestSlow(IRInstruction* inst) {
  auto const extra = inst->extra<LdSSwitchDestSlow>();
  auto const spOff = extra->spOff;

  auto& v = vmain();
  auto strtab = v.allocData<const StringData*>(extra->numCases);
  auto jmptab = v.allocData<TCA>(extra->numCases + 1);

  for (int i = 0; i < extra->numCases; ++i) {
    strtab[i] = extra->cases[i].str;
    v << bindaddr{&jmptab[i], extra->cases[i].dest, spOff};
  }
  v << bindaddr{&jmptab[extra->numCases], extra->defaultSk, spOff};
  cgCallHelper(v,
               CallSpec::direct(sswitchHelperSlow),
               callDest(inst),
               SyncOptions::Sync,
               argGroup(inst)
                 .typedValue(0)
                 .dataPtr(strtab)
                 .imm(extra->numCases)
                 .dataPtr(jmptab));
}

/*
 * It'd be nice not to have the cgMov here (and just copy propagate
 * the source or something), but for now we're keeping it allocated to
 * rvmfp() so inlined calls to C++ helpers that use the rbp chain to
 * find the caller's ActRec will work correctly.
 *
 * This instruction primarily exists to assist in optimizing away
 * unused activation records, so it's usually not going to happen
 * anyway.
 */
void CodeGenerator::cgDefInlineFP(IRInstruction* inst) {
  auto const callerSP = srcLoc(inst, 0).reg();
  auto const callerFP = srcLoc(inst, 1).reg();
  auto const fakeRet  = mcg->ustubs().retInlHelper;
  auto const extra    = inst->extra<DefInlineFP>();
  auto const retBCOff = extra->retBCOff;
  auto const offset   = cellsToBytes(extra->spOffset.offset);
  auto& v = vmain();
  v << store{callerFP, callerSP[offset + AROFF(m_sfp)]};
  emitImmStoreq(v, intptr_t(fakeRet), callerSP[offset + AROFF(m_savedRip)]);
  v << storeli{retBCOff, callerSP[offset + AROFF(m_soff)]};
  if (extra->target->attrs() & AttrMayUseVV) {
    v << storeqi{0, callerSP[offset + AROFF(m_invName)]};
  }
  v << lea{callerSP[offset], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgInlineReturn(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 0).reg();
  assertx(fpReg == rvmfp());
  vmain() << load{fpReg[AROFF(m_sfp)], rvmfp()};
}

void CodeGenerator::cgInlineReturnNoFrame(IRInstruction* inst) {
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const offset = cellsToBytes(
      inst->extra<InlineReturnNoFrame>()->frameOffset.offset);
    for (auto i = 0; i < kNumActRecCells; ++i) {
      emitTrashTV(rvmfp(), offset - cellsToBytes(i), kTVTrashJITFrame);
    }
  }
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = AROFF(m_sfp);
  auto dst = dstLoc(inst, 0).reg();
  vmain() << load{ptr[off], dst};
}

void CodeGenerator::cgStMem(IRInstruction* inst) {
  auto const ptr = srcLoc(inst, 0).reg();
  storeTV(vmain(), ptr[0], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgStRef(IRInstruction* inst) {
  always_assert(!srcLoc(inst, 1).isFullSIMD());
  auto ptr = srcLoc(inst, 0).reg();
  auto off = RefData::tvOffset();
  storeTV(vmain(), ptr[off], srcLoc(inst, 1), inst->src(1));
}

int CodeGenerator::iterOffset(const BCMarker& marker, uint32_t id) {
  const Func* func = getFunc(marker);
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

void CodeGenerator::cgStLoc(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = localOffset(inst->extra<StLoc>()->locId);
  storeTV(vmain(), ptr[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgStLocRange(IRInstruction* inst) {
  auto const range = inst->extra<StLocRange>();

  if (range->start >= range->end) return;

  auto const fp = srcLoc(inst, 0).reg();
  auto const loc = srcLoc(inst, 1);
  auto const val = inst->src(1);
  auto& v = vmain();

  auto ireg = v.makeReg();
  auto nreg = v.makeReg();

  v << lea{fp[localOffset(range->start)], ireg};
  v << lea{fp[localOffset(range->end)], nreg};

  doWhile(v, CC_NE, {ireg},
    [&] (const VregList& in, const VregList& out) {
      auto const i = in[0];
      auto const res = out[0];
      auto const sf = v.makeReg();

      storeTV(v, i[0], loc, val);
      v << subqi{int32_t{sizeof(Cell)}, i, res, v.makeReg()};
      v << cmpq{res, nreg, sf};
      return sf;
    }
  );
}

void CodeGenerator::cgEagerSyncVMRegs(IRInstruction* inst) {
  auto const spOff = inst->extra<EagerSyncVMRegs>()->offset;
  auto& v = vmain();
  auto const sync_sp = v.makeReg();
  v << lea{srcLoc(inst, 1).reg()[cellsToBytes(spOff.offset)], sync_sp};
  emitEagerSyncPoint(v, inst->marker().fixupSk().pc(),
                     rvmtl(), srcLoc(inst, 0).reg(), sync_sp);
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  auto const extra = inst->extra<ReqBindJmp>();
  auto& v = vmain();
  maybe_syncsp(v, inst->marker(), srcLoc(inst, 0).reg(), extra->irSPOff);
  v << bindjmp{
    extra->target,
    extra->invSPOff,
    extra->trflags,
    cross_trace_args(inst->marker())
  };
}

void CodeGenerator::cgReqRetranslateOpt(IRInstruction* inst) {
  auto const extra = inst->extra<ReqRetranslateOpt>();
  auto& v = vmain();
  maybe_syncsp(v, inst->marker(), srcLoc(inst, 0).reg(), extra->irSPOff);
  v << retransopt{
    extra->transID,
    extra->target,
    inst->marker().spOff(),
    cross_trace_args(inst->marker())
  };
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  auto const destSK = m_state.unit.initSrcKey();
  auto const extra  = inst->extra<ReqRetranslate>();
  auto& v = vmain();

  maybe_syncsp(v, inst->marker(), srcLoc(inst, 0).reg(), extra->irSPOff);
  v << fallback{
    destSK,
    inst->marker().spOff(),
    extra->trflags,
    cross_trace_args(inst->marker())
  };
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  // This is redundant with a check in ifRefCountedNonPersistent, but we check
  // earlier to avoid emitting profiling code in this case.
  auto const ty = inst->src(0)->type();
  if (!ty.maybe(TCounted)) return;

  folly::Optional<rds::Handle> profHandle;
  auto vtaken = &vmain();
  // We profile generic IncRefs to see which ones are unlikely to see
  // refcounted values.
  if (RuntimeOption::EvalHHIROutlineGenericIncDecRef && !ty.isKnownDataType()) {
    auto const profileKey =
      makeStaticString(folly::to<std::string>("IncRefProfile-",
                                              ty.toString()));
    TargetProfile<IncRefProfile> profile{
      m_state.unit.context(), inst->marker(), profileKey
    };
    if (profile.profiling()) {
      profHandle = profile.handle();
    } else if (profile.optimizing()) {
      auto const data = profile.data(IncRefProfile::reduce);
      if (data.tryinc == 0) {
        FTRACE(3, "Emitting cold IncRef for {}, {}\n", data, *inst);
        vtaken = &vcold();
      }
    }
  }

  auto& v = vmain();
  auto const loc = srcLoc(inst, 0);
  ifRefCountedType(v, *vtaken, ty, loc, [&](Vout& v) {
    if (profHandle) {
      v << incwm{rvmtl()[*profHandle + offsetof(IncRefProfile, tryinc)],
                 v.makeReg()};
    }
    ifNonPersistent(v, ty, loc, [&](Vout& v) {
      emitIncRef(v, loc.reg());
    });
  });
}

void CodeGenerator::cgIncRefCtx(IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (ty <= TObj) return cgIncRef(inst);
  if (ty <= TCctx || ty <= TNullptr) return;

  auto const src = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();

  if (ty.maybe(TNullptr)) {
    auto const shifted = v.makeReg();
    v << shrqi{1, src, shifted, sf};

    ifThen(v, CC_NBE, sf, [&] (Vout& v) {
      auto const unshifted = v.makeReg();
      v << shlqi{1, shifted, unshifted, v.makeReg()};
      emitIncRef(v, unshifted);
    });
  } else {
    v << testqi{0x1, src, sf};
    ifThen(v, CC_Z, sf, [&] (Vout& v) { emitIncRef(v, src); });
  }
}

void CodeGenerator::cgGenericRetDecRefs(IRInstruction* inst) {
  auto const rFp       = srcLoc(inst, 0).reg();
  auto const numLocals = getFunc(inst->marker())->numLocals();
  auto& v = vmain();

  assertx(rFp == rvmfp() &&
         "free locals helper assumes the frame pointer is rvmfp()");

  if (numLocals == 0) return;

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? mcg->ustubs().freeManyLocalsHelper
    : mcg->ustubs().freeLocalsHelpers[numLocals - 1];

  auto const iterReg = v.makeReg();
  v << lea{rFp[localOffset(numLocals - 1)], iterReg};

  auto const& marker = inst->marker();
  auto const fix = Fixup{
    marker.bcOff() - marker.func()->base(),
    marker.spOff().offset
  };
  // The stub uses arg reg 0 as scratch and to pass arguments to destructors,
  // so it expects the iter argument in arg reg 1.
  auto const args = v.makeVcallArgs({{v.cns(Vconst::Quad), iterReg}});
  v << vcall{CallSpec::stub(target),
             args, v.makeTuple({}), fix, DestType::None, false};
}

/*
 * Depending on the current translation kind, do nothing, profile, or collect
 * profiling data for the current DecRef* instruction
 *
 * Returns true iff the release path for this DecRef should be put in cold
 * code.
 */
float CodeGenerator::decRefDestroyRate(const IRInstruction* inst,
                                       OptDecRefProfile& profile,
                                       Type type) {
  auto const kind = m_state.unit.context().kind;
  // Without profiling data, we assume destroy is unlikely.
  if (kind != TransKind::Profile && kind != TransKind::Optimize) return 0.0;

  auto const locId = inst->extra<DecRef>()->locId;
  auto const profileKey =
    makeStaticString(folly::to<std::string>("DecRefProfile-",
                                            opcodeName(inst->op()),
                                            '-', type.toString(),
                                            '-', locId));
  profile.emplace(m_state.unit.context(), inst->marker(), profileKey);

  auto& v = vmain();
  if (profile->profiling()) {
    v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, hits)],
               v.makeReg()};
  } else if (profile->optimizing()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.destroyRate() != 0.0 && data.destroyRate() != 1.0) {
      // These are the only interesting cases where we could be doing better.
      FTRACE(5, "DecRefProfile: {}: {} {}\n",
             data, inst->marker().show(), profileKey->data());
    }
    if (data.destroyRate() == 0.0) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_0);
    } else if (data.destroyRate() == 1.0) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_100);
    }
    return data.destroyRate();
  }

  return 0.0;
}

static CallSpec getDtorCallSpec(DataType type) {
  switch (type) {
    case KindOfString:
      return CallSpec::method(&StringData::release);
    case KindOfArray:
      return CallSpec::method(&ArrayData::release);
    case KindOfObject:
      return CallSpec::method(
        RuntimeOption::EnableObjDestructCall
          ? &ObjectData::release
          : &ObjectData::releaseNoObjDestructCheck
      );
    case KindOfResource:
      return CallSpec::method(&ResourceHdr::release);
    case KindOfRef:
      return CallSpec::method(&RefData::release);
    DT_UNCOUNTED_CASE:
    case KindOfClass:
      break;
  }
  not_reached();
}

static CallSpec makeDtorCall(Type ty, Vloc loc, ArgGroup& args) {
  static auto const TPackedArr = Type::Array(ArrayData::kPackedKind);
  static auto const TVecArr = Type::Array(ArrayData::kVecKind);
  static auto const TDictArr = Type::Array(ArrayData::kDictKind);
  static auto const TMixedArr = Type::Array(ArrayData::kMixedKind);
  static auto const TApcArr = Type::Array(ArrayData::kApcKind);

  if (ty <= TPackedArr) return CallSpec::direct(PackedArray::Release);
  if (ty <= TVecArr) return CallSpec::direct(PackedArray::Release);
  if (ty <= TDictArr) return CallSpec::direct(MixedArray::Release);
  if (ty <= TMixedArr) return CallSpec::direct(MixedArray::Release);
  if (ty <= TApcArr) return CallSpec::direct(APCLocalArray::Release);
  if (ty <= TArr) return  CallSpec::array(&g_array_funcs.release);

  if (ty <= TObj && ty.clsSpec().cls()) {
    auto cls = ty.clsSpec().cls();

    // These conditions must match what causes us to call cls->instanceDtor()
    // in ObjectData::release().
    if ((cls->attrs() & AttrNoOverride) &&
        !cls->getDtor() && cls->instanceDtor()) {
      args.immPtr(cls);
      return CallSpec::direct(cls->instanceDtor().get());
    }
  }

  return ty.isKnownDataType() ? getDtorCallSpec(ty.toDataType())
                              : CallSpec::destruct(loc.reg(1));
}

/*
 * We've tried a variety of tweaks to this and found the current state of
 * things optimal, at least when measurements of the following factors were
 * made:
 *
 * - whether to load the count into a register
 *
 * - whether to use if (!--count) release(); if we don't need a static check
 *
 * - whether to skip using the register and just emit --count if we know
 *   its not static, and can't hit zero.
 *
 * The current scheme generates if (!--count) release() for types that cannot
 * possibly be static.  For types that might be static, it generates a compare
 * of the m_count field against 1, followed by two conditional branches on the
 * same flags.  We make use of the invariant that count fields are never zero,
 * and use a code sequence that looks like this:
 *
 *    cmpl $1, $FAST_REFCOUNT_OFFSET(%base)
 *    je do_release  // call the destructor, usually in acold
 *    jl skip_dec    // count < 1 implies it's static
 *    decl $FAST_REFCOUNT_OFFSET(%base)
 *  skip_dec:
 *    // ....
 */
void CodeGenerator::decRefImpl(Vout& v, const IRInstruction* inst,
                               const OptDecRefProfile& profile,
                               bool unlikelyDestroy) {
  auto const ty   = inst->src(0)->type();
  auto const base = srcLoc(inst, 0).reg(0);

  auto destroy = [&] (Vout& v) {
    emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Destroy
                                   : Stats::TC_DecRef_Likely_Destroy);
    if (profile && profile->profiling()) {
      v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, destroy)],
                 v.makeReg()};
    }
    auto args = argGroup(inst).reg(base);
    auto const dtor = makeDtorCall(ty, srcLoc(inst, 0), args);
    cgCallHelper(v, dtor, kVoidDest, SyncOptions::Sync, args);
  };

  emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Decl
                                 : Stats::TC_DecRef_Likely_Decl);

  if (profile && profile->profiling()) {
    v << incwm{rvmtl()[profile->handle() + offsetof(DecRefProfile, trydec)],
               v.makeReg()};
  }

  if (!ty.maybe(TPersistent)) {
    auto const sf = emitDecRef(v, base);
    ifThen(v, vcold(), CC_E, sf, destroy, unlikelyDestroy);
    return;
  }

  emitDecRefWork(v, vcold(), base, destroy, unlikelyDestroy);
}

void CodeGenerator::emitDecRefTypeStat(Vout& v, const IRInstruction* inst) {
  if (!Trace::moduleEnabled(Trace::decreftype)) return;

  auto category = makeStaticString(inst->is(DecRef) ? "DecRef" : "DecRefNZ");
  auto key = makeStaticString(inst->src(0)->type().unspecialize().toString());
  cgCallHelper(
    v,
    CallSpec::direct(Stats::incStatGrouped),
    kVoidDest,
    SyncOptions::None,
    argGroup(inst)
      .immPtr(category)
      .immPtr(key)
      .imm(1)
  );
}

void CodeGenerator::cgDecRef(IRInstruction *inst) {
  // This is redundant with a check in ifRefCounted, but we check earlier to
  // avoid emitting profiling code in this case.
  auto const ty = inst->src(0)->type();
  if (!ty.maybe(TCounted)) return;

  auto& v = vmain();
  emitDecRefTypeStat(v, inst);
  OptDecRefProfile profile;
  auto const destroyRate = decRefDestroyRate(inst, profile, ty);
  FTRACE(3, "destroyPercent {:.2%} for {}\n", destroyRate, *inst);

  auto const rData = srcLoc(inst, 0).reg(0);
  auto const rType = srcLoc(inst, 0).reg(1);
  if (RuntimeOption::EvalHHIROutlineGenericIncDecRef &&
      profile && profile->optimizing() && !ty.isKnownDataType()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.trydec == 0) {
      // This DecRef never saw a refcounted type during profiling, so call the
      // stub in cold, keeping only the type check in main.
      FTRACE(3, "Emitting partially outlined DecRef for {}, {}\n", data, *inst);
      auto const sf = v.makeReg();
      emitCmpTVType(v, sf, KindOfRefCountThreshold, rType);
      unlikelyIfThen(v, vcold(), CC_NLE, sf, [&](Vout& v) {
        auto const stub = mcg->ustubs().decRefGeneric;
        v << copy2{rData, rType, rarg(0), rarg(1)};
        v << callfaststub{stub, makeFixup(inst->marker()), arg_regs(2)};
      });
      return;
    }
  }

  ifRefCountedType(
    v, v, ty, srcLoc(inst, 0),
    [&] (Vout& v) {
      decRefImpl(
        v, inst, profile,
        destroyRate * 100 < RuntimeOption::EvalJitUnlikelyDecRefPercent
      );
    }
  );
}

void CodeGenerator::cgDecRefNZ(IRInstruction* inst) {
  emitIncStat(vmain(), Stats::TC_DecRef_NZ);
  emitDecRefTypeStat(vmain(), inst);
  auto const ty = inst->src(0)->type();
  ifRefCountedNonPersistent(
    vmain(), ty, srcLoc(inst, 0),
    [&] (Vout& v) {
      auto const base = srcLoc(inst, 0).reg();
      emitDecRef(v, base);
    }
  );
}

void CodeGenerator::cgCufIterSpillFrame(IRInstruction* inst) {
  auto const extra = inst->extra<CufIterSpillFrame>();
  auto const nArgs = extra->args;
  auto const iterId = extra->iterId;
  auto const itOff = iterOffset(inst->marker(), iterId);

  auto const spOffset = cellsToBytes(extra->spOffset.offset);
  auto spReg = srcLoc(inst, 0).reg();
  auto fpReg = srcLoc(inst, 1).reg();
  auto& v = vmain();

  auto func = v.makeReg();
  v << load{fpReg[itOff + CufIter::funcOff()], func};
  v << store{func, spReg[spOffset + int(AROFF(m_func))]};

  auto ctx = v.makeReg();
  v << load{fpReg[itOff + CufIter::ctxOff()], ctx};
  v << store{ctx, spReg[spOffset + int(AROFF(m_this))]};

  auto ctx2 = v.makeReg();
  {
    auto const sf = v.makeReg();
    v << shrqi{1, ctx, ctx2, sf};
    ifThen(v, CC_NBE, sf, [&](Vout& v) {
      auto ctx3 = v.makeReg();
      v << shlqi{1, ctx2, ctx3, v.makeReg()};
      emitIncRef(v, ctx3);
    });
  }

  auto callerFunc = inst->marker().func();
  auto unit = callerFunc->unit();
  auto weakTypesFlag = !callerFunc->isBuiltin() && !unit->useStrictTypes()
      ? ActRec::Flags::UseWeakTypes
      : ActRec::Flags::None;

  auto name = v.makeReg();
  v << load{fpReg[itOff + CufIter::nameOff()], name};
  auto const sf = v.makeReg();
  v << testq{name, name, sf};
  ifThenElse(v, CC_NZ, sf, [&](Vout& v) {
    auto const sf = v.makeReg();
    v << cmplim{0, name[FAST_REFCOUNT_OFFSET], sf};
    static_assert(UncountedValue < 0 && StaticValue < 0, "");
    ifThen(v, CC_NS, sf, [&](Vout& v) { emitIncRef(v, name); });
    v << store{name, spReg[spOffset + int(AROFF(m_invName))]};
    auto const encoded = ActRec::encodeNumArgsAndFlags(
      safe_cast<int32_t>(nArgs),
      static_cast<ActRec::Flags>(ActRec::Flags::MagicDispatch | weakTypesFlag)
    );
    v << storeli{static_cast<int32_t>(encoded),
                 spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
  }, [&](Vout& v) {
    auto const encoded = ActRec::encodeNumArgsAndFlags(
      safe_cast<int32_t>(nArgs),
      weakTypesFlag
    );
    v << store{name, spReg[spOffset + int(AROFF(m_invName))]};
    v << storeli{safe_cast<int32_t>(encoded),
                 spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
  });
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func      = inst->src(1);
  auto const objOrCls  = inst->src(2);
  auto const extra     = inst->extra<SpillFrame>();
  auto const magicName = extra->invName;
  auto const nArgs     = extra->numArgs;
  auto& v              = vmain();

  auto const spOffset = cellsToBytes(extra->spOffset.offset);

  auto spReg = srcLoc(inst, 0).reg();
  // actRec->m_this
  if (objOrCls->isA(TCls)) {
    // store class
    if (objOrCls->hasConstVal()) {
      emitImmStoreq(v, uintptr_t(objOrCls->clsVal()) | 1,
                    spReg[spOffset + int(AROFF(m_this))]);
    } else {
      auto clsPtrReg = srcLoc(inst, 2/*objOrCls*/).reg();
      auto thisptr = v.makeReg();
      v << orqi{1, clsPtrReg, thisptr, v.makeReg()};
      v << store{thisptr, spReg[spOffset + int(AROFF(m_this))]};
    }
  } else if (objOrCls->isA(TObj)) {
    // store this pointer
    v << store{srcLoc(inst, 2/*objOrCls*/).reg(),
               spReg[spOffset + int(AROFF(m_this))]};
  } else if (objOrCls->isA(TCtx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    auto objOrClsPtrReg = srcLoc(inst, 2/*objOrCls*/).reg();
    v << store{objOrClsPtrReg, spReg[spOffset + int(AROFF(m_this))]};
  } else {
    always_assert(objOrCls->isA(TNullptr));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + int(AROFF(m_this));
    v << storeqi{0, spReg[offset_m_this]};
  }

  // actRec->m_invName
  if (magicName) {
    auto const invName = reinterpret_cast<uintptr_t>(magicName);
    emitImmStoreq(v, invName, spReg[spOffset] + int{AROFF(m_invName)});
  } else if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, ActRec::kTrashedVarEnvSlot,
                  spReg[spOffset] + int{AROFF(m_invName)});
  }

  // actRec->m_func
  if (!func->isA(TNullptr)) {
    v << store{srcLoc(inst, 1).reg(0), spReg[spOffset + int(AROFF(m_func))]};
  }

  auto callerFunc = inst->marker().func();
  auto unit = callerFunc->unit();
  auto flags = !callerFunc->isBuiltin() && !unit->useStrictTypes()
      ? ActRec::Flags::UseWeakTypes
      : ActRec::Flags::None;
  if (magicName) {
    flags = static_cast<ActRec::Flags>(flags | ActRec::Flags::MagicDispatch);
  }
  auto const encoded = static_cast<int32_t>(ActRec::encodeNumArgsAndFlags(
    nArgs,
    flags
  ));
  v << storeli{encoded, spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
}

void CodeGenerator::cgSyncReturnBC(IRInstruction* inst) {
  auto const extra = inst->extra<SyncReturnBC>();
  auto const spOffset = cellsToBytes(extra->spOffset.offset);
  auto const bcOffset = extra->bcOffset;
  auto const spReg = srcLoc(inst, 0).reg();
  auto const fpReg = srcLoc(inst, 1).reg();

  auto& v = vmain();
  v << storeli{safe_cast<int32_t>(bcOffset), spReg[spOffset + AROFF(m_soff)]};
  v << store{fpReg, spReg[spOffset + AROFF(m_sfp)]};
}

void CodeGenerator::cgStClosureArg(IRInstruction* inst) {
  auto const ptr = srcLoc(inst, 0).reg();
  auto const off = inst->extra<StClosureArg>()->offsetBytes;
  storeTV(vmain(), ptr[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgLdClosureCtx(IRInstruction* inst) {
  auto const obj = srcLoc(inst, 0).reg();
  auto const ctx = dstLoc(inst, 0).reg();
  vmain() << load{obj[c_Closure::ctxOffset()], ctx};
}

void CodeGenerator::cgStClosureCtx(IRInstruction* inst) {
  auto const obj = srcLoc(inst, 0).reg();
  auto& v = vmain();
  if (inst->src(1)->isA(TNullptr)) {
    v << storeqi{0, obj[c_Closure::ctxOffset()]};
  } else {
    auto const ctx = srcLoc(inst, 1).reg();
    v << store{ctx, obj[c_Closure::ctxOffset()]};
  }
}

void CodeGenerator::emitInitObjProps(const IRInstruction* inst, Vreg dstReg,
                                     const Class* cls, size_t nProps) {
  // If the object has a small number of properties, just emit stores
  // inline.
  auto& v = vmain();
  if (nProps < 8) {
    for (int i = 0; i < nProps; ++i) {
      auto propOffset =
        sizeof(ObjectData) + cls->builtinODTailSize() + sizeof(TypedValue) * i;
      auto propDataOffset = propOffset + TVOFF(m_data);
      auto propTypeOffset = propOffset + TVOFF(m_type);
      if (!isNullType(cls->declPropInit()[i].m_type)) {
        emitImmStoreq(v, cls->declPropInit()[i].m_data.num,
                      dstReg[propDataOffset]);
      }
      v << storebi{cls->declPropInit()[i].m_type, dstReg[propTypeOffset]};
    }
    return;
  }

  // Use memcpy for large numbers of properties.
  auto args = argGroup(inst)
    .addr(dstReg,
          safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
    .imm(int64_t(&cls->declPropInit()[0]))
    .imm(cellsToBytes(nProps));
  cgCallHelper(v,
               CallSpec::direct(memcpy),
               kVoidDest,
               SyncOptions::None,
               args);
}

void CodeGenerator::cgConstructInstance(IRInstruction* inst) {
  auto const cls    = inst->extra<ConstructInstance>()->cls;
  auto const dstReg = dstLoc(inst, 0).reg();
  cgCallHelper(vmain(),
               CallSpec::direct(cls->instanceCtor().get()),
               callDest(dstReg),
               SyncOptions::Sync,
               argGroup(inst).immPtr(cls));
}

void CodeGenerator::cgCheckInitProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitProps>()->cls;
  auto& v = vmain();
  auto const sf = checkRDSHandleInitialized(v, cls->propHandle());
  v << jcc{CC_NE, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckInitSProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitSProps>()->cls;
  auto& v = vmain();
  auto const handle = cls->sPropInitHandle();
  if (rds::isNormalHandle(handle)) {
    auto const sf = checkRDSHandleInitialized(v, handle);
    v << jcc{CC_NE, sf, {label(inst->next()), label(inst->taken())}};
  } else {
    // Always initialized; just fall through to next.
    assert(rds::isPersistentHandle(handle));
    assert(rds::handleToRef<bool>(handle));
  }
}

void CodeGenerator::cgNewInstanceRaw(IRInstruction* inst) {
  auto const cls    = inst->extra<NewInstanceRaw>()->cls;
  auto const dstReg = dstLoc(inst, 0).reg();
  size_t size = ObjectData::sizeForNProps(cls->numDeclProperties());
  cgCallHelper(vmain(),
               size <= kMaxSmallSize
               ? CallSpec::direct(ObjectData::newInstanceRaw)
               : CallSpec::direct(ObjectData::newInstanceRawBig),
               callDest(dstReg),
               SyncOptions::Sync,
               argGroup(inst).imm((uint64_t)cls).imm(size));
}

void CodeGenerator::cgInitObjProps(IRInstruction* inst) {
  auto const cls    = inst->extra<InitObjProps>()->cls;
  auto const srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  // Set the attributes, if any
  int odAttrs = cls->getODAttrs();
  if (odAttrs) {
    static_assert(sizeof(ObjectData::Attribute) == 2,
                  "Codegen expects 2-byte ObjectData attributes");
    assertx(!(odAttrs & 0xffff0000));
    v << orwim{odAttrs, srcReg[ObjectData::attributeOff()], v.makeReg()};
  }

  // Initialize the properties
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() == 0) {
      // Fast case: copy from a known address in the Class
      emitInitObjProps(inst, srcReg, cls, nProps);
    } else {
      // Slower case: we have to load the src address from the targetcache
      auto propInitVec = v.makeReg();
      // Load the Class's propInitVec from the targetcache. We
      // know its already been initialized as a pre-condition
      // on this op.
      auto const propHandle = cls->propHandle();
      assertx(rds::isNormalHandle(propHandle));
      v << load{rvmtl()[propHandle], propInitVec};
      // We want &(*propData)[0]
      auto rPropData = v.makeReg();
      v << load{propInitVec[Class::PropInitVec::dataOff()], rPropData};
      if (!cls->hasDeepInitProps()) {
        auto args = argGroup(inst)
          .addr(srcReg,
              safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
          .reg(rPropData)
          .imm(cellsToBytes(nProps));
        cgCallHelper(v,
                     CallSpec::direct(memcpy),
                     kVoidDest,
                     SyncOptions::None,
                     args);
      } else {
        auto args = argGroup(inst)
          .addr(srcReg,
              safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
          .reg(rPropData)
          .imm(nProps);
        cgCallHelper(v,
                     CallSpec::direct(deepInitHelper),
                     kVoidDest,
                     SyncOptions::None,
                     args);
      }
    }
  }
}

void CodeGenerator::cgCallArray(IRInstruction* inst) {
  auto& v = vmain();
  auto const extra  = inst->extra<CallArray>();
  auto const pc     = v.cns(extra->pc);
  auto const after  = v.cns(extra->after);
  auto const target = extra->numParams == 0 ?
    mcg->ustubs().fcallArrayHelper :
    mcg->ustubs().fcallUnpackHelper;
  auto const rSP    = srcLoc(inst, 0 /* sp */).reg();
  auto const syncSP = v.makeReg();
  v << lea{rSP[cellsToBytes(extra->spOffset.offset)], syncSP};
  v << syncvmsp{syncSP};

  auto done = v.makeBlock();
  auto args = extra->numParams == 0 ?
    v.makeTuple({pc, after}) :
    v.makeTuple({pc, after, v.cns(extra->numParams)});
  v << vcallarray{target, fcall_array_regs(), args,
                  {done, m_state.labels[inst->taken()]}};
  m_state.catch_calls[inst->taken()] = CatchCall::PHP;
  v = done;

  auto const dst = dstLoc(inst, 0);
  v << defvmret{dst.reg(0), dst.reg(1)};
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  auto const rSP    = srcLoc(inst, 0).reg();
  auto const rFP    = srcLoc(inst, 1).reg();
  auto const extra  = inst->extra<Call>();
  auto const callee = extra->callee;
  auto const argc = extra->numParams;
  auto const rds = rvmtl();
  auto& v = vmain();
  auto& vc = vcold();

  // An intentionally funny-looking-in-core-dumps constant for uninitialized
  // instruction pointers.
  constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

  auto const arOff =
    cellsToBytes(extra->spOffset.offset) + argc * sizeof(TypedValue);

  v << store{rFP, rSP[arOff + AROFF(m_sfp)]};
  v << storeli{safe_cast<int32_t>(extra->after), rSP[arOff + AROFF(m_soff)]};
  if (extra->fcallAwait) {
    auto imm = static_cast<int32_t>(
      ActRec::encodeNumArgsAndFlags(argc, ActRec::Flags::IsFCallAwait));
    v << storeli{imm, rSP[arOff + AROFF(m_numArgsAndFlags)]};
  }
  // The sync_sp temporary will be eliminated by vasm-copy.
  auto const sync_sp = v.makeReg();
  v << lea{rSP[cellsToBytes(extra->spOffset.offset)], sync_sp};

  auto catchBlock = m_state.labels[inst->taken()];
  if (isNativeImplCall(callee, argc)) {
    // The assumption here is that for builtins, the generated func contains
    // only a single opcode (NativeImpl), and there are no non-argument locals.
    if (do_assert) {
      assertx(argc == callee->numLocals() && callee->numIterators() == 0);
      PC addr = callee->getEntry();
      while (peek_op(addr) == Op::AssertRATL) {
        addr += instrLen(addr);
      }
      assertx(peek_op(addr) == Op::NativeImpl);
      assertx(addr + instrLen(addr) ==
              callee->unit()->entry() + callee->past());
    }
    auto retAddr = (int64_t)mcg->ustubs().retHelper;
    v << store{v.cns(retAddr),
               sync_sp[cellsToBytes(argc) + AROFF(m_savedRip)]};
    if (callee->attrs() & AttrMayUseVV) {
      v << storeqi{0, sync_sp[cellsToBytes(argc) + AROFF(m_invName)]};
    }
    v << lea{sync_sp[cellsToBytes(argc)], rvmfp()};
    emitCheckSurpriseFlagsEnter(v, vc, rFP, rds, Fixup(0, argc), catchBlock);
    BuiltinFunction builtinFuncPtr = callee->builtinFuncPtr();
    TRACE(2, "calling builtin preClass %p func %p\n", callee->preClass(),
          builtinFuncPtr);
    // We sometimes call this while curFunc() isn't really the builtin, so
    // make sure to record the sync point as if we are inside the builtin.
    if (FixupMap::eagerRecord(callee)) {
      emitEagerSyncPoint(v, callee->getEntry(), rds, rvmfp(), sync_sp);
    }
    // Call the native implementation. This will free the locals for us in the
    // normal case. In the case where an exception is thrown, the VM unwinder
    // will handle it for us.
    auto next = v.makeBlock();
    v << vinvoke{CallSpec::direct(builtinFuncPtr), v.makeVcallArgs({{rvmfp()}}),
                 v.makeTuple({}), {next, catchBlock}, Fixup(0, argc)};
    m_state.catch_calls[inst->taken()] = CatchCall::CPP;
    v = next;
    // The native implementation already put the return value on the stack for
    // us, and handled cleaning up the arguments.  We have to update the frame
    // pointer and the stack pointer, and load the return value into the return
    // register so the trace we are returning to has it where it expects.
    // TODO(#1273094): we should probably modify the actual builtins to return
    // values via registers using the C ABI and do a reg-to-reg move.
    loadTV(v, inst->dst(), dstLoc(inst, 0), rvmfp()[AROFF(m_r)], true);
    v << load{rvmfp()[AROFF(m_sfp)], rvmfp()};
    emitRB(v, Trace::RBTypeFuncExit, callee->fullName()->data());
    return;
  }

  // Emit a smashable call that initially calls a recyclable service request
  // stub.  The stub and the eventual targets take rvmfp() as an argument,
  // pointing to the callee ActRec.
  auto& us = mcg->ustubs();
  auto addr = callee ? us.immutableBindCallStub : us.bindCallStub;
  debug_trashsp(v);
  v << lea{sync_sp[cellsToBytes(argc)], rvmfp()};
  if (debug && RuntimeOption::EvalHHIRGenerateAsserts) {
    emitImmStoreq(v, kUninitializedRIP, rvmfp()[AROFF(m_savedRip)]);
  }
  auto next = v.makeBlock();
  v << callphp{addr, php_call_regs(), {{next, catchBlock}}};
  m_state.catch_calls[inst->taken()] = CatchCall::PHP;
  v = next;

  auto const dst = dstLoc(inst, 0);
  v << defvmret{dst.reg(0), dst.reg(1)};
}

void CodeGenerator::cgCastHelper(IRInstruction *inst,
                                 Vreg base, int offset) {
  Type type       = inst->typeParam();
  bool nullable = false;
  if (!type.isKnownDataType()) {
    assertx(TNull <= type);
    type -= TNull;
    assertx(type.isKnownDataType());
    nullable = true;
  }

  auto args = argGroup(inst);
  args.addr(base, offset);

  TCA tvCastHelper;
  if (type <= TBool) {
    tvCastHelper = (TCA)tvCastToBooleanInPlace;
  } else if (type <= TInt) {
    tvCastHelper = (TCA)tvCastToInt64InPlace;
  } else if (type <= TDbl) {
    tvCastHelper = (TCA)tvCastToDoubleInPlace;
  } else if (type <= TArr) {
    tvCastHelper = (TCA)tvCastToArrayInPlace;
  } else if (type <= TStr) {
    tvCastHelper = (TCA)tvCastToStringInPlace;
  } else if (type <= TObj) {
    tvCastHelper = nullable ?
      (TCA)tvCastToNullableObjectInPlace :
      (TCA)tvCastToObjectInPlace;
    nullable = false;
  } else if (type <= TRes) {
    tvCastHelper = (TCA)tvCastToResourceInPlace;
  } else {
    not_reached();
  }
  assert(!nullable);
  cgCallHelper(vmain(),
               CallSpec::direct(reinterpret_cast<void (*)()>(tvCastHelper)),
               kVoidDest,
               SyncOptions::Sync,
               args);
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  auto offset     = inst->extra<CastStk>()->offset;
  auto spReg      = srcLoc(inst, 0).reg();

  cgCastHelper(inst, spReg, cellsToBytes(offset.offset));
}

void CodeGenerator::cgCastMem(IRInstruction *inst) {
  auto ptr      = srcLoc(inst, 0).reg();

  cgCastHelper(inst, ptr, 0);
}

void CodeGenerator::cgCoerceHelper(IRInstruction* inst,
                                   Vreg base, int offset,
                                   Func const* callee, int argNum) {
  auto& v = vmain();

  Type type = inst->typeParam();
  assertx(type.isKnownDataType());

  // If the type-specific test(s) failed,
  // fallback on actually calling the tvCoerceParamTo*() helper
  auto args = argGroup(inst);
  args.addr(base, offset);
  args.imm(callee);
  args.imm(argNum);

  TCA tvCoerceHelper;
  if (type <= TBool) {
    tvCoerceHelper = (TCA)tvCoerceParamToBooleanOrThrow;
  } else if (type <= TInt) {
    tvCoerceHelper = (TCA)tvCoerceParamToInt64OrThrow;
  } else if (type <= TDbl) {
    tvCoerceHelper = (TCA)tvCoerceParamToDoubleOrThrow;
  } else if (type <= TArr) {
    tvCoerceHelper = (TCA)tvCoerceParamToArrayOrThrow;
  } else if (type <= TStr) {
    tvCoerceHelper = (TCA)tvCoerceParamToStringOrThrow;
  } else if (type <= TObj) {
    tvCoerceHelper = (TCA)tvCoerceParamToObjectOrThrow;
  } else if (type <= TRes) {
    tvCoerceHelper = (TCA)tvCoerceParamToResourceOrThrow;
  } else {
    not_reached();
  }

  cgCallHelper(v,
    CallSpec::direct(reinterpret_cast<void (*)()>(tvCoerceHelper)),
    kVoidDest,
    SyncOptions::Sync,
    args
  );
}

void CodeGenerator::cgCoerceStk(IRInstruction *inst) {
  auto extra      = inst->extra<CoerceStk>();
  auto spReg      = srcLoc(inst, 0).reg();
  auto offset     = cellsToBytes(extra->offset.offset);

  cgCoerceHelper(inst, spReg, offset, extra->callee, extra->argNum);
}

void CodeGenerator::cgCoerceMem(IRInstruction *inst) {
  auto extra      = inst->extra<CoerceMem>();
  auto ptr        = srcLoc(inst, 0).reg();

  cgCoerceHelper(inst, ptr, 0, extra->callee, extra->argNum);
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const dst            = dstLoc(inst, 0);
  auto const dstReg         = dst.reg(0);
  auto const dstType        = dst.reg(1);
  auto const callee         = inst->extra<CallBuiltin>()->callee;
  auto const numArgs        = callee->numParams();
  auto const numNonDefault  = inst->extra<CallBuiltin>()->numNonDefault;
  auto const returnType     = inst->typeParam();
  auto const funcReturnType = callee->returnType();
  auto const returnByValue  = callee->isReturnByValue();
  auto& v = vmain();

  int returnOffset = rds::kVmMInstrStateOff +
    offsetof(MInstrState, tvBuiltinReturn);

  if (FixupMap::eagerRecord(callee)) {
    auto const rSP = srcLoc(inst, 1).reg();
    auto const spOffset = cellsToBytes(
      inst->extra<CallBuiltin>()->spOffset.offset);
    auto const& marker = inst->marker();
    auto const pc = marker.fixupSk().unit()->entry() + marker.fixupBcOff();
    auto const synced_sp = v.makeReg();
    v << lea{rSP[spOffset], synced_sp};
    emitEagerSyncPoint(
      v,
      pc,
      rvmtl(),
      srcLoc(inst, 0).reg(),
      synced_sp
    );
  }

  // The MInstrState we need to use is at a constant offset from the base of
  // the RDS header.
  PhysReg rdsReg(rvmtl());

  auto callArgs = argGroup(inst);
  if (!returnByValue) {
    if (isBuiltinByRef(funcReturnType)) {
      // First arg is pointer to storage for that return value
      if (isReqPtrRef(funcReturnType)) {
        returnOffset += TVOFF(m_data);
      }
      // Pass the address of tvBuiltinReturn to the native function as the
      // location it can construct the return Array, String, Object, or Variant.
      callArgs.addr(rdsReg, returnOffset); // &rdsReg[returnOffset]
    }
  }

  // Non-pointer and nativeArg args, are passed by value.  String,
  // Array, Object, and Variant are passed by const&, ie a pointer to stack
  // memory holding the value, so expect PtrToT types for these.
  // Pointers to req::ptr types (String, Array, Object) need adjusting to
  // point to &ptr->m_data.
  auto srcNum = uint32_t{2};
  if (callee->isMethod()) {
    if (callee->isStatic()) {
      if (callee->isNative()) {
        callArgs.ssa(srcNum);
        ++srcNum;
      }
    } else {
      // Note, we don't support objects with vtables here (if they may
      // need a this pointer adjustment).  This should be filtered out
      // earlier right now.
      callArgs.ssa(srcNum);
      ++srcNum;
    }
  }

  if (callee->attrs() & AttrNumArgs) {
    if (numNonDefault >= 0) {
      callArgs.imm((int64_t)numNonDefault);
    } else {
      callArgs.ssa(srcNum);
      ++srcNum;
    }
  }

  for (uint32_t i = 0; i < numArgs; ++i, ++srcNum) {
    auto const& pi = callee->params()[i];
    if (TVOFF(m_data) &&
        !pi.nativeArg &&
        isReqPtrRef(pi.builtinType)) {
      assertx(inst->src(srcNum)->type() <= TPtrToGen);
      callArgs.addr(srcLoc(inst, srcNum).reg(), TVOFF(m_data));
    } else if (pi.nativeArg && !pi.builtinType && !callee->byRef(i)) {
      callArgs.typedValue(srcNum);
    } else {
      callArgs.ssa(srcNum, pi.builtinType == KindOfDouble);
    }
  }

  auto dest = [&] () -> CallDest {
    if (isBuiltinByRef(funcReturnType)) {
      if (returnByValue) {
        if (!funcReturnType) {
          return callDest(dstReg, dstType);
        }
        return callDest(dstReg);
      }
      return kVoidDest;
    }
    if (funcReturnType == KindOfDouble) {
      return callDestDbl(inst);
    }
    return callDest(inst);
  }();

  cgCallHelper(v, CallSpec::direct(callee->nativeFuncPtr()),
               dest, SyncOptions::Sync, callArgs);

  // For primitive return types (int, bool, double), and returnByValue,
  // the return value is already in dstReg/dstType
  if (returnType.isSimpleType() || returnByValue) {
    return;
  }

  // For return by reference (String, Object, Array, Variant),
  // the builtin writes the return value into MInstrState::tvBuiltinReturn
  // TV, from where it has to be tested and copied.
  if (returnType.isReferenceType()) {
    assertx(isBuiltinByRef(funcReturnType) && isReqPtrRef(funcReturnType));
    // return type is String, Array, or Object; fold nullptr to KindOfNull
    auto rtype = v.cns(returnType.toDataType());
    auto nulltype = v.cns(KindOfNull);
    v << load{rdsReg[returnOffset], dstReg};
    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      v << testq{dstReg, dstReg, sf};
      v << cmovb{CC_Z, sf, rtype, nulltype, dstType};
    }
    return;
  }
  if (returnType <= TCell || returnType <= TBoxedCell) {
    // return type is Variant; fold KindOfUninit to KindOfNull
    assertx(isBuiltinByRef(funcReturnType) && !isReqPtrRef(funcReturnType));
    auto nulltype = v.cns(KindOfNull);
    auto tmp_type = v.makeReg();
    v << loadb{rdsReg[returnOffset + TVOFF(m_type)], tmp_type};
    v << load{rdsReg[returnOffset + TVOFF(m_data)], dstReg};
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      v << testb{tmp_type, tmp_type, sf};
      v << cmovb{CC_Z, sf, tmp_type, nulltype, dstType};
    }
    return;
  }
  not_reached();
}

void CodeGenerator::cgStStk(IRInstruction* inst) {
  auto const spReg = srcLoc(inst, 0).reg();
  auto const offset = cellsToBytes(inst->extra<StStk>()->offset.offset);
  storeTV(vmain(), spReg[offset], srcLoc(inst, 1), inst->src(1));
}

// Fill the entire 16-byte space for a TypedValue with trash.  Note: it will
// clobber the Aux area of a TypedValueAux.
void CodeGenerator::emitTrashTV(Vreg ptr, int32_t offset, char fillByte) {
  auto& v = vmain();
  int32_t trash32;
  memset(&trash32, fillByte, sizeof trash32);
  static_assert(sizeof(TypedValue) == 16, "");
  v << storeli{trash32, ptr[offset + 0x0]};
  v << storeli{trash32, ptr[offset + 0x4]};
  v << storeli{trash32, ptr[offset + 0x8]};
  v << storeli{trash32, ptr[offset + 0xc]};
}

void CodeGenerator::cgDbgTrashStk(IRInstruction* inst) {
  emitTrashTV(
    srcLoc(inst, 0).reg(),
    cellsToBytes(inst->extra<DbgTrashStk>()->offset.offset),
    kTVTrashJITStk
  );
}

void CodeGenerator::cgDbgTrashFrame(IRInstruction* inst) {
  auto const reg = srcLoc(inst, 0).reg();
  auto const offset = cellsToBytes(inst->extra<DbgTrashFrame>()->offset.offset);
  for (auto i = 0; i < kNumActRecCells; ++i) {
    emitTrashTV(reg, offset + cellsToBytes(i), kTVTrashJITFrame);
  }
}

void CodeGenerator::cgDbgTrashMem(IRInstruction* inst) {
  emitTrashTV(srcLoc(inst, 0).reg(), 0, kTVTrashJITHeap);
}

void CodeGenerator::cgDbgTrashRetVal(IRInstruction* inst) {
  emitTrashTV(srcLoc(inst, 0).reg(), AROFF(m_r), kTVTrashJITRetVal);
}

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  auto const func = getFunc(inst->marker());
  auto const builtinFuncPtr = func->builtinFuncPtr();
  auto& v = vmain();
  auto fp = srcLoc(inst, 0).reg();
  auto sp = srcLoc(inst, 1).reg();

  if (FixupMap::eagerRecord(func)) {
    emitEagerSyncPoint(v, func->getEntry(), rvmtl(), fp, sp);
  }
  v << vinvoke{
    CallSpec::direct(builtinFuncPtr),
    v.makeVcallArgs({{fp}}),
    v.makeTuple({}),
    {m_state.labels[inst->next()], m_state.labels[inst->taken()]},
    makeFixup(inst->marker(), SyncOptions::Sync)
  };
  m_state.catch_calls[inst->taken()] = CatchCall::CPP;
}

void CodeGenerator::cgCastCtxThis(IRInstruction* inst) {
  vmain() << copy{srcLoc(inst, 0).reg(), dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgCheckCtxThis(IRInstruction* inst) {
  auto const rctx = srcLoc(inst, 0).reg();
  auto& v = vmain();

  auto const func = getFunc(inst->marker());
  if (func->isPseudoMain() || !func->mayHaveThis()) {
    // Check for a null $this pointer first.
    auto const sf = v.makeReg();
    v << testq{rctx, rctx, sf};
    emitFwdJcc(v, CC_Z, sf, inst->taken());
  }

  auto const sf = v.makeReg();
  v << testqi{1, rctx, sf};
  v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  // Context could be either a this object or a class ptr
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testqi{1, srcReg, sf};
  cond(v, CC_NZ, sf, dstReg,
    [&](Vout& v) { // ctx is a class
      return emitLdClsCctx(v, srcReg, v.makeReg());
    }, [&](Vout& v) { // ctx is this ptr
      return emitLdObjClass(v, srcReg, v.makeReg());
    });
}

void CodeGenerator::cgLdClsCctx(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  emitLdClsCctx(vmain(), srcReg, dstReg);
}

void CodeGenerator::cgLdCtx(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const srcReg = srcLoc(inst, 0).reg();
  vmain() << load{srcReg[AROFF(m_this)], dstReg};
}

void CodeGenerator::cgLdCctx(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdClosure(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdClsName(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto preclass = v.makeReg();
  v << load{srcReg[Class::preClassOff()], preclass};
  emitLdLowPtr(v, preclass[PreClass::nameOffset()],
               dstReg, sizeof(LowStringPtr));
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  auto const off = cellsToBytes(inst->extra<LdARFuncPtr>()->offset.offset);
  auto dstReg = dstLoc(inst, 0).reg();
  auto baseReg = srcLoc(inst, 0).reg();
  vmain() << load{baseReg[off + AROFF(m_func)], dstReg};
}

void CodeGenerator::cgLdARNumParams(IRInstruction* inst) {
  auto& v = vmain();
  auto dstReg = dstLoc(inst, 0).reg();
  auto baseReg = srcLoc(inst, 0).reg();
  auto tmp = v.makeReg();
  v << loadzlq{baseReg[AROFF(m_numArgsAndFlags)], tmp};
  v << andqi{ActRec::kNumArgsMask, tmp, dstReg, v.makeReg()};
}

void CodeGenerator::cgCheckClosureStaticLocInit(IRInstruction* inst) {
  auto const src = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, src[RefData::tvOffset() + TVOFF(m_type)]);
  v << jcc{CC_E, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgInitClosureStaticLoc(IRInstruction* inst) {
  always_assert(!srcLoc(inst, 1).isFullSIMD());
  auto ptr = srcLoc(inst, 0).reg();
  auto off = RefData::tvOffset();
  storeTV(vmain(), ptr[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgLdStaticLoc(IRInstruction* inst) {
  auto const extra = inst->extra<LdStaticLoc>();
  auto const link  = rds::bindStaticLocal(extra->func, extra->name);
  auto const dst   = dstLoc(inst, 0).reg();
  auto& v          = vmain();
  auto const sf    = checkRDSHandleInitialized(v, link.handle());
  emitFwdJcc(v, CC_NE, sf, inst->taken());
  v << lea{rvmtl()[link.handle()], dst};
}

void CodeGenerator::cgInitStaticLoc(IRInstruction* inst) {
  auto const extra = inst->extra<InitStaticLoc>();
  auto const link  = rds::bindStaticLocal(extra->func, extra->name);
  auto const dst   = dstLoc(inst, 0).reg();
  auto& v          = vmain();

  // If we're here, the target-cache-local RefData is all zeros, so we
  // can initialize it by storing the new value into it's TypedValue
  // and incrementing the RefData reference count (which will set it
  // to 1).
  //
  // We are storing the rdSrc value into the static, but we don't need
  // to inc ref it because it's a bytecode invariant that it's not a
  // reference counted type.

  v << lea{rvmtl()[link.handle()], dst};
  storeTV(v, dst[RefData::tvOffset()], srcLoc(inst, 0), inst->src(0));
  v << storeli{1, dst[FAST_REFCOUNT_OFFSET]};
  v << storebi{0, dst[RefData::cowZOffset()]};
  v << storebi{uint8_t(HeaderKind::Ref), dst[HeaderKindOffset]};
  markRDSHandleInitialized(v, link.handle());

  static_assert(sizeof(HeaderKind) == 1, "");
}

void CodeGenerator::cgLdContField(IRInstruction* inst) {
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0),
         srcLoc(inst, 0).reg()[inst->src(1)->intVal()]);
}

void CodeGenerator::cgLdMem(IRInstruction* inst) {
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), srcLoc(inst, 0).reg()[0]);
}

void CodeGenerator::cgLdRef(IRInstruction* inst) {
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0),
         srcLoc(inst, 0).reg()[RefData::tvOffset()]);
}

void CodeGenerator::cgStringIsset(IRInstruction* inst) {
  auto strReg = srcLoc(inst, 0).reg();
  auto idxReg = srcLoc(inst, 1).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto const strLen = v.makeReg();
  auto const sf = v.makeReg();
  v << loadzlq{strReg[StringData::sizeOff()], strLen};
  v << cmpq{idxReg, strLen, sf};
  v << setcc{CC_NBE, sf, dstReg};
}

void CodeGenerator::cgProfileArrayKind(IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  auto& v = vmain();
  auto const profile = v.makeReg();
  v << lea{rvmtl()[extra->handle], profile};
  cgCallHelper(v, CallSpec::direct(profileArrayKindHelper), kVoidDest,
               SyncOptions::None, argGroup(inst).reg(profile).ssa(0));
}

void CodeGenerator::cgProfileStructArray(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto handle  = inst->extra<ProfileStructArray>()->handle;
  auto& v = vmain();

  auto isStruct = v.makeBlock();
  auto shapeIsDifferent = v.makeBlock();
  auto notStruct = v.makeBlock();
  auto done = v.makeBlock();

  auto const sf0 = v.makeReg();
  static_assert(sizeof(HeaderKind) == 1, "");
  v << cmpbim{ArrayData::kStructKind, baseReg[HeaderKindOffset], sf0};
  v << jcc{CC_E, sf0, {notStruct, isStruct}};

  auto const shape = v.makeReg();
  auto const sf1 = v.makeReg();
  v = isStruct;
  v << load{baseReg[StructArray::shapeOffset()], shape};
  v << cmpqm{shape, rvmtl()[handle + offsetof(StructArrayProfile, shape)], sf1};
  v << jcc{CC_E, sf1, {shapeIsDifferent, done}};

  v = shapeIsDifferent;
  v << addlm{v.cns(uint32_t{1}),
             rvmtl()[handle + offsetof(StructArrayProfile, numShapesSeen)],
             v.makeReg()};
  v << store{shape, rvmtl()[handle + offsetof(StructArrayProfile, shape)]};
  v << jmp{done};

  v = notStruct;
  v << addlm{v.cns(uint32_t{1}),
             rvmtl()[handle + offsetof(StructArrayProfile, nonStructCount)],
             v.makeReg()};
  v << jmp{done};

  v = done;
}

void CodeGenerator::cgCheckPackedArrayBounds(IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");
  // We may check packed array bounds on profiled arrays for which
  // we do not statically know that they are of kPackedKind.
  assertx(inst->taken());
  auto arrReg = srcLoc(inst, 0).reg();
  auto idxReg = srcLoc(inst, 1).reg();
  auto& v = vmain();
  // ArrayData::m_size is a uint32_t but we need to do a 64-bit comparison
  // since idx is KindOfInt64.
  auto tmp_size = v.makeReg();
  v << loadzlq{arrReg[ArrayData::offsetofSize()], tmp_size};
  auto const sf = v.makeReg();
  v << cmpq{idxReg, tmp_size, sf};
  v << jcc{CC_BE, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgLdPackedArrayElemAddr(IRInstruction* inst) {
  auto const idx = inst->src(1);
  auto const rArr = srcLoc(inst, 0).reg();
  auto const rIdx = srcLoc(inst, 1).reg();
  auto const dst = dstLoc(inst, 0).reg();
  auto& v = vmain();

  if (idx->hasConstVal()) {
    auto const offset = sizeof(ArrayData) + idx->intVal() * sizeof(TypedValue);
    if (deltaFits(offset, sz::dword)) {
      v << lea{rArr[offset], dst};
      return;
    }
  }

  static_assert(sizeof(TypedValue) == 16 && sizeof(ArrayData) == 16, "");
  /*
   * This computes `rArr + rIdx * sizeof(TypedValue) + sizeof(ArrayData)`. The
   * logic of `scaledIdx * 16` is split in the following two instructions, in
   * order to save a byte in the shl instruction.
   *
   * TODO(#7728856): We should really move this into vasm-x64.cpp...
   */
  auto idxl = v.makeReg();
  auto scaled_idxl = v.makeReg();
  auto scaled_idx = v.makeReg();
  v << movtql{rIdx, idxl};
  v << shlli{1, idxl, scaled_idxl, v.makeReg()};
  v << movzlq{scaled_idxl, scaled_idx};
  v << lea{rArr[scaled_idx * int(sizeof(TypedValue) / 2) +
                int(sizeof(ArrayData))], dst};
}

void CodeGenerator::cgCheckRange(IRInstruction* inst) {
  auto val = inst->src(0);
  auto valReg = srcLoc(inst, 0).reg();
  auto limitReg  = srcLoc(inst, 1).reg();

  auto& v = vmain();
  ConditionCode cc;
  auto const sf = v.makeReg();
  if (val->hasConstVal()) {
    // Try to put the constant in a position that can get imm-folded. A
    // suffiently smart imm-folder could handle this for us.
    v << cmpq{valReg, limitReg, sf};
    cc = CC_A;
  } else {
    v << cmpq{limitReg, valReg, sf};
    cc = CC_B;
  }

  v << setcc{cc, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLdVectorSize(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  assertx(vec->type() < TObj);
  assertx(collections::isType(vec->type().clsSpec().cls(),
                              CollectionType::Vector,
                              CollectionType::ImmVector));
  vmain() << loadzlq{vecReg[BaseVector::sizeOffset()], dstReg};
}

void CodeGenerator::cgLdVectorBase(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  assertx(vec->type() < TObj);
  assertx(collections::isType(vec->type().clsSpec().cls(),
                              CollectionType::Vector,
                              CollectionType::ImmVector));
  auto& v = vmain();
  auto arr = v.makeReg();
  v << load{vecReg[BaseVector::arrOffset()], arr};
  v << lea{arr[PackedArray::entriesOffset()], dstReg};
}

void CodeGenerator::cgLdColArray(IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const cls = src->type().clsSpec().cls();
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto& v = vmain();

  always_assert_flog(
    collections::isType(cls, CollectionType::Vector, CollectionType::ImmVector,
                        CollectionType::Map, CollectionType::ImmMap,
                        CollectionType::Set, CollectionType::ImmSet),
    "LdColArray received an unsupported type: {}\n",
    src->type().toString()
  );
  auto offset = collections::isType(cls, CollectionType::Vector,
                                    CollectionType::ImmVector) ?
    BaseVector::arrOffset() : HashCollection::arrOffset();
  v << load{rsrc[offset], rdst};
}

void CodeGenerator::cgVectorHasImmCopy(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  assertx(vec->type() < TObj);
  assertx(collections::isType(vec->type().clsSpec().cls(),
                              CollectionType::Vector));

  auto arr = v.makeReg();
  v << load{vecReg[BaseVector::arrOffset()], arr};
  auto const sf = v.makeReg();
  v << cmplim{1, arr[FAST_REFCOUNT_OFFSET], sf};
  v << jcc{CC_NE, sf, {label(inst->next()), label(inst->taken())}};
}

/**
 * Given the base of a vector object, pass it to a helper
 * which is responsible for triggering COW.
 */
void CodeGenerator::cgVectorDoCow(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  assertx(vec->type() < TObj);
  assertx(collections::isType(vec->type().clsSpec().cls(),
                              CollectionType::Vector));
  auto args = argGroup(inst);
  args.ssa(0); // vec
  cgCallHelper(vmain(), CallSpec::direct(triggerCow),
               kVoidDest, SyncOptions::Sync, args);
}

void CodeGenerator::cgLdPairBase(IRInstruction* inst) {
  DEBUG_ONLY auto pair = inst->src(0);
  auto pairReg = srcLoc(inst, 0).reg();
  assertx(pair->type() < TObj);
  assertx(collections::isType(pair->type().clsSpec().cls(),
                              CollectionType::Pair));
  vmain() << lea{pairReg[c_Pair::dataOffset()], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLdElem(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto idx = inst->src(1);
  auto idxReg = srcLoc(inst, 1).reg();
  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    loadTV(vmain(), inst->dst(), dstLoc(inst, 0), baseReg[idx->intVal()]);
  } else {
    loadTV(vmain(), inst->dst(), dstLoc(inst, 0), baseReg[idxReg]);
  }
}

void CodeGenerator::cgStElem(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto idxReg = srcLoc(inst, 1).reg();
  auto idx = inst->src(1);
  auto val = inst->src(2);

  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    storeTV(vmain(), baseReg[idx->intVal()], srcLoc(inst, 2), val);
  } else {
    storeTV(vmain(), baseReg[idxReg], srcLoc(inst, 2), val);
  }
}

void CodeGenerator::cgLdMIStateAddr(IRInstruction* inst) {
  auto const base = rvmtl();
  int64_t offset = inst->src(0)->intVal();
  vmain() << lea{base[rds::kVmMInstrStateOff + offset], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0),
         srcLoc(inst, 0).reg()[localOffset(inst->extra<LdLoc>()->locId)]);
}

void CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  auto const fpReg  = srcLoc(inst, 0).reg();
  auto const offset = localOffset(inst->extra<LdLocAddr>()->locId);
  if (dstLoc(inst, 0).hasReg()) {
    vmain() << lea{fpReg[offset], dstLoc(inst, 0).reg()};
  }
}

void CodeGenerator::cgLdLocPseudoMain(IRInstruction* inst) {
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const lmem = rsrc[localOffset(inst->extra<LdLocPseudoMain>()->locId)];
  irlower::emitTypeCheck(
    vmain(), m_state,
    inst->typeParam(),
    lmem + TVOFF(m_type),
    lmem + TVOFF(m_data),
    inst->taken()
  );
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), lmem);
}

void CodeGenerator::cgStLocPseudoMain(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = localOffset(inst->extra<StLocPseudoMain>()->locId);
  storeTV(vmain(), ptr[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgLdStkAddr(IRInstruction* inst) {
  auto const base   = srcLoc(inst, 0).reg();
  auto const offset = cellsToBytes(inst->extra<LdStkAddr>()->offset.offset);
  auto const dst    = dstLoc(inst, 0).reg();
  vmain() << lea{base[offset], dst};
}

void CodeGenerator::cgLdStk(IRInstruction* inst) {
  auto const offset = cellsToBytes(inst->extra<LdStk>()->offset.offset);
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), srcLoc(inst, 0).reg()[offset]);
}

void CodeGenerator::cgLdMBase(IRInstruction* inst) {
  vmain() << load{rvmtl()[rds::kVmMInstrStateOff + offsetof(MInstrState, base)],
                  dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgStMBase(IRInstruction* inst) {
  vmain() << store{
    srcLoc(inst, 0).reg(),
    rvmtl()[rds::kVmMInstrStateOff + offsetof(MInstrState, base)]
  };
}

template <class JmpFn>
void CodeGenerator::emitReffinessTest(IRInstruction* inst, Vreg sf,
                                      JmpFn doJcc) {
  DEBUG_ONLY SSATmp* nParamsTmp = inst->src(1);
  SSATmp* firstBitNumTmp = inst->src(2);
  SSATmp* mask64Tmp  = inst->src(3);
  SSATmp* vals64Tmp  = inst->src(4);

  auto funcPtrReg = srcLoc(inst, 0).reg();
  auto nParamsReg = srcLoc(inst, 1).reg();
  auto mask64Reg = srcLoc(inst, 3).reg();
  auto vals64Reg = srcLoc(inst, 4).reg();

  // Get values in place
  assertx(firstBitNumTmp->hasConstVal(TInt));
  auto firstBitNum = safe_cast<int32_t>(firstBitNumTmp->intVal());

  uint64_t mask64 = mask64Tmp->intVal();
  assertx(mask64);

  uint64_t vals64 = vals64Tmp->intVal();
  assertx((vals64 & mask64) == vals64);

  auto& v = vmain();

  auto thenBody = [&](Vout& v) {
    auto bitsOff = sizeof(uint64_t) * (firstBitNum / 64);
    auto cond = CC_NE;
    auto bitsPtrReg = v.makeReg();
    if (firstBitNum == 0) {
      bitsOff = Func::refBitValOff();
      bitsPtrReg = funcPtrReg;
    } else {
      v << load{funcPtrReg[Func::sharedOff()], bitsPtrReg};
      bitsOff -= sizeof(uint64_t);
    }

    if (vals64 == 0 || (mask64 & (mask64 - 1)) == 0) {
      // If vals64 is zero, or we're testing a single bit, we can get away with
      // a single test, rather than mask-and-compare. The use of testbim and
      // testlim here is little-endian specific but it's "ok" for now as long
      // as nothing else is read or written using the same pointer.
      if (mask64 <= 0xff) {
        v << testbim{(int8_t)mask64, bitsPtrReg[bitsOff], sf};
      } else if (mask64 <= 0xffffffff) {
        v << testlim{(int32_t)mask64, bitsPtrReg[bitsOff], sf};
      } else {
        v << testqm{mask64Reg, bitsPtrReg[bitsOff], sf};
      }
      if (vals64) cond = CC_E;
    } else {
      auto bitsValReg = v.makeReg();
      v << load{bitsPtrReg[bitsOff], bitsValReg};

      auto truncBits = v.makeReg();
      auto maskedBits = v.makeReg();
      if (mask64 <= 0xff && vals64 <= 0xff) {
        v << movtqb{bitsValReg, truncBits};
        v << andbi{(int8_t)mask64, truncBits, maskedBits, v.makeReg()};
        v << cmpbi{(int8_t)vals64, maskedBits, sf};
      } else if (mask64 <= 0xffffffff && vals64 <= 0xffffffff) {
        v << movtql{bitsValReg, truncBits};
        v << andli{(int32_t)mask64, truncBits, maskedBits, v.makeReg()};
        v << cmpli{(int32_t)vals64, maskedBits, sf};
      } else {
        v << andq{mask64Reg, bitsValReg, maskedBits, v.makeReg()};
        v << cmpq{vals64Reg, maskedBits, sf};
      }
    }
    doJcc(v, cond, sf);
  };

  if (firstBitNum == 0) {
    assertx(nParamsTmp->hasConstVal());
    // This is the first 64 bits. No need to check
    // nParams.
    thenBody(v);
  } else {
    // Check number of args...
    auto const sf2 = v.makeReg();
    v << cmpqi{firstBitNum, nParamsReg, sf2};

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nParams, then either all params
      // are refs, or all params are non-refs, so if vals64
      // isn't 0 and isnt mask64, there's no possibility of
      // a match
      doJcc(v, CC_LE, sf2);
      thenBody(v);
    } else {
      ifThenElse(v, CC_NLE, sf2, thenBody,
                 /* else */ [&](Vout& v) {
          //   If not special builtin...
          auto const sf = v.makeReg();
          v << testlim{AttrVariadicByRef, funcPtrReg[Func::attrsOff()], sf};
          doJcc(v, vals64 ? CC_Z : CC_NZ, sf);
        });
    }
  }
}

void CodeGenerator::cgCheckRefs(IRInstruction* inst)  {
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitReffinessTest(inst, sf,
    [&](Vout& v, ConditionCode cc, Vreg sfTaken) {
      emitFwdJcc(v, cc, sfTaken, inst->taken());
    });
}

void CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const objReg = srcLoc(inst, 0).reg();
  vmain() << lea{objReg[inst->extra<LdPropAddr>()->offsetBytes], dstReg};
}

void CodeGenerator::cgLdFuncVecLen(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto clsReg = srcLoc(inst, 0).reg();

  // A Cctx is a Cls with the bottom bit set; subtract one from the
  // offset to handle that case
  auto off = Class::funcVecLenOff() - (inst->src(0)->isA(TCctx) ? 1 : 0);
  vmain() << loadzlq{clsReg[off], dstReg};
}

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto clsReg = srcLoc(inst, 0).reg();
  int32_t mSlotVal = inst->src(1)->rawVal();
  // We could have a Cls or a Cctx. The Cctx has the low bit set, so
  // we need to subtract one in that case.
  auto methOff = int32_t(mSlotVal * sizeof(LowPtr<Func>)) -
    (inst->src(0)->isA(TCctx) ? 1 : 0);
  auto& v = vmain();
  emitLdLowPtr(v, clsReg[methOff], dstReg, sizeof(LowPtr<Func>));
}

void CodeGenerator::cgLdIfaceMethod(IRInstruction* inst) {
  auto& extra = *inst->extra<LdIfaceMethod>();
  auto& v = vmain();
  auto const clsReg = srcLoc(inst, 0).reg();
  auto const vtableVecReg = v.makeReg();
  auto const vtableReg = v.makeReg();
  auto const funcReg = dstLoc(inst, 0).reg();

  emitLdLowPtr(v, clsReg[Class::vtableVecOff()],
               vtableVecReg, sizeof(LowPtr<Class::VtableVecSlot>));
  auto const vtableOff = extra.vtableIdx * sizeof(Class::VtableVecSlot) +
             offsetof(Class::VtableVecSlot, vtable);
  emitLdLowPtr(v, vtableVecReg[vtableOff], vtableReg,
               sizeof(Class::VtableVecSlot::vtable));
  emitLdLowPtr(v, vtableReg[extra.methodIdx * sizeof(LowPtr<Func>)],
               funcReg, sizeof(LowPtr<Func>));
}

void CodeGenerator::cgLookupClsMethodCache(IRInstruction* inst) {
  auto funcDestReg   = dstLoc(inst, 0).reg(0);

  auto const& extra = *inst->extra<ClsMethodData>();
  auto const cls = extra.clsName;
  auto const method = extra.methodName;
  auto const ne = extra.namedEntity;
  auto const ch = StaticMethodCache::alloc(
    cls, method, getContextName(getClass(inst->marker())));

  if (false) { // typecheck
    UNUSED TypedValue* fake_fp = nullptr;
    UNUSED TypedValue* fake_sp = nullptr;
    const UNUSED Func* f = StaticMethodCache::lookup(
      ch, ne, cls, method, fake_fp);
  }

  // can raise an error if class is undefined
  cgCallHelper(vmain(),
               CallSpec::direct(StaticMethodCache::lookup),
               callDest(funcDestReg),
               SyncOptions::Sync,
               argGroup(inst)
                 .imm(ch)       // Handle ch
                 .immPtr(ne)            // NamedEntity* np.second
                 .immPtr(cls)           // className
                 .immPtr(method)        // methodName
                 .reg(srcLoc(inst, 0).reg()) // frame pointer
              );
}

void CodeGenerator::cgLdClsMethodCacheFunc(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const& extra = *inst->extra<ClsMethodData>();
  auto const clsName = extra.clsName;
  auto const methodName = extra.methodName;
  auto const ch = StaticMethodCache::alloc(
    clsName, methodName, getContextName(getClass(inst->marker())));
  auto& v = vmain();

  static_assert(sizeof(LowPtr<const Class>) == sizeof(LowPtr<const Func>), "");

  auto const sf = checkRDSHandleInitialized(v, ch);
  emitFwdJcc(v, CC_NE, sf, inst->taken());
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodCache, m_func)],
               dstReg, sizeof(LowPtr<const Class>));
}

void CodeGenerator::cgLdClsMethodCacheCls(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const& extra = *inst->extra<ClsMethodData>();
  auto const clsName = extra.clsName;
  auto const methodName = extra.methodName;
  auto& v = vmain();

  auto const ch = StaticMethodCache::alloc(
    clsName, methodName, getContextName(getClass(inst->marker())));

  static_assert(sizeof(LowPtr<const Class>) == sizeof(LowPtr<const Func>), "");

  // The StaticMethodCache here is guaranteed to already be initialized in RDS
  // by the pre-conditions of this instruction.
  emitLdLowPtr(v, rvmtl()[ch + offsetof(StaticMethodCache, m_cls)],
               dstReg, sizeof(LowPtr<const Class>));
}

/**
 * Helper to emit getting the value for ActRec's m_this/m_cls slot
 * from a This pointer depending on whether the callee method is
 * static or not.
 */
void CodeGenerator::emitGetCtxFwdCallWithThis(Vreg srcCtx, Vreg dstCtx,
                                              bool staticCallee) {
  auto& v = vmain();
  if (staticCallee) {
    // Load (this->m_cls | 0x1) into ctxReg.
    auto vmclass = v.makeReg();
    emitLdLowPtr(v, srcCtx[ObjectData::getVMClassOffset()],
                 vmclass, sizeof(LowPtr<Class>));
    v << orqi{1, vmclass, dstCtx, v.makeReg()};
  } else {
    // Just incref $this.
    emitIncRef(v, srcCtx);
    v << copy{srcCtx, dstCtx};
  }
}

void CodeGenerator::cgGetCtxFwdCall(IRInstruction* inst) {
  auto destCtxReg = dstLoc(inst, 0).reg(0);
  auto srcCtxTmp = inst->src(0);
  auto srcCtxReg = srcLoc(inst, 0).reg(0);
  const Func* callee = inst->src(1)->funcVal();
  bool      withThis = srcCtxTmp->isA(TObj);
  auto& v = vmain();

  // If we don't know whether we have a This, we need to check dynamically
  if (!withThis) {
    auto const sf = v.makeReg();
    v << testqi{1, srcCtxReg, sf};
    cond(v, CC_Z, sf, destCtxReg, [&](Vout& v) {
      // If we have a This pointer in destCtxReg, then select either This
      // or its Class based on whether callee is static or not
      auto dst1 = v.makeReg();
      emitGetCtxFwdCallWithThis(srcCtxReg, dst1, callee->isStatic());
      return dst1;
    }, [&](Vout& v) {
      return srcCtxReg;
    });
  } else {
    // If we have a This pointer in destCtxReg, then select either This
    // or its Class based on whether callee is static or not
    emitGetCtxFwdCallWithThis(srcCtxReg, destCtxReg, callee->isStatic());
  }
}

void CodeGenerator::cgLdClsMethodFCacheFunc(IRInstruction* inst) {
  auto const& extra     = *inst->extra<ClsMethodData>();
  auto const clsName    = extra.clsName;
  auto const methodName = extra.methodName;
  auto const dstReg     = dstLoc(inst, 0).reg();
  auto& v               = vmain();
  auto const ch = StaticMethodFCache::alloc(
    clsName, methodName, getContextName(getClass(inst->marker()))
  );
  auto const sf = checkRDSHandleInitialized(v, ch);
  emitFwdJcc(v, CC_NE, sf, inst->taken());
  emitLdLowPtr(v, rvmtl()[ch], dstReg, sizeof(LowPtr<const Func>));
}

void CodeGenerator::cgLookupClsMethodFCache(IRInstruction* inst) {
  auto const funcDestReg = dstLoc(inst, 0).reg(0);
  auto const cls         = inst->src(0)->clsVal();
  auto const& extra      = *inst->extra<ClsMethodData>();
  auto const methName    = extra.methodName;
  auto const fpReg       = srcLoc(inst, 1).reg();
  auto const clsName     = cls->name();

  auto const ch = StaticMethodFCache::alloc(
    clsName, methName, getContextName(getClass(inst->marker()))
  );
  assertx(rds::isNormalHandle(ch));

  const Func* (*lookup)(
    rds::Handle, const Class*, const StringData*, TypedValue*) =
    StaticMethodFCache::lookup;
  cgCallHelper(vmain(),
               CallSpec::direct(lookup),
               callDest(funcDestReg),
               SyncOptions::Sync,
               argGroup(inst)
                 .imm(ch)
                 .immPtr(cls)
                 .immPtr(methName)
                 .reg(fpReg));
}

Vreg CodeGenerator::emitGetCtxFwdCallWithThisDyn(Vreg destCtxReg, Vreg thisReg,
                                                 rds::Handle ch) {
  auto& v = vmain();

  // thisReg is holding $this. Should we pass it to the callee?

  // RDS entry is guaranteed to be already initialized due to pre-conditions on
  // this op.
  auto const sf = v.makeReg();
  v << cmplim{1, rvmtl()[ch + offsetof(StaticMethodFCache, m_static)], sf};
  return cond(v, CC_E, sf, destCtxReg, [&](Vout& v) {
      // If calling a static method...
      // Load (this->m_cls | 0x1) into destCtxReg
      auto vmclass = v.makeReg();
      auto dst1 = v.makeReg();
      emitLdLowPtr(v, thisReg[ObjectData::getVMClassOffset()],
                   vmclass, sizeof(LowPtr<Class>));
      v << orqi{1, vmclass, dst1, v.makeReg()};
      return dst1;
    }, [&](Vout& v) {
      // Else: calling non-static method
      emitIncRef(v, thisReg);
      return thisReg;
    });
}

/**
 * This method is similar to emitGetCtxFwdCall above, but whether or not the
 * callee is a static method is unknown at JIT time, and that is determined
 * dynamically by looking up into the StaticMethodFCache.
 */
void CodeGenerator::cgGetCtxFwdCallDyn(IRInstruction* inst) {
  auto srcCtxTmp  = inst->src(0);
  auto srcCtxReg  = srcLoc(inst, 0).reg();
  auto destCtxReg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto const t = srcCtxTmp->type();

  // Allocate a StaticMethodFCache and return its RDS handle.
  auto make_cache = [&] {
    auto const& extra = *inst->extra<ClsMethodData>();
    return StaticMethodFCache::alloc(extra.clsName, extra.methodName,
                                     getContextName(getClass(inst->marker())));
  };

  if (t <= TCctx) {
    // Nothing to do. Forward the context as is.
    v << copy{srcCtxReg, destCtxReg};
    return;
  }
  if (t <= TObj) {
    // We definitely have $this, so always run code emitted by
    // emitGetCtxFwdCallWithThisDyn
    emitGetCtxFwdCallWithThisDyn(destCtxReg, srcCtxReg, make_cache());
    return;
  }

  // dynamically check if we have a This pointer and call
  // emitGetCtxFwdCallWithThisDyn below
  auto const sf = v.makeReg();
  v << testqi{1, srcCtxReg, sf};
  cond(v, CC_Z, sf, destCtxReg, [&](Vout& v) {
    // If we have a 'this' pointer ...
    return emitGetCtxFwdCallWithThisDyn(v.makeReg(), srcCtxReg, make_cache());
  }, [&](Vout& v) {
    return srcCtxReg;
  });
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  const StringData* className = inst->src(0)->strVal();
  auto ch = NamedEntity::get(className)->getClassHandle();
  auto const dst = dstLoc(inst, 0).reg();
  auto& v = vmain();

  auto const call_helper = [&] (Vout& v) {
    auto ptr = v.makeReg();
    cgCallHelper(
      v,
      CallSpec::direct(jit::lookupKnownClass),
      callDest(ptr),
      SyncOptions::Sync,
      argGroup(inst).
        imm(ch).
        ssa(0)
    );
    return ptr;
  };

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    unlikelyCond(
      v, vcold(), CC_NE, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) {
        auto const ptr = v.makeReg();
        emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Class>));
        return ptr;
      }
    );
  } else {
    auto const sf = v.makeReg();
    auto const ptr = v.makeReg();
    emitLdLowPtr(v, rvmtl()[ch], ptr, sizeof(LowPtr<Class>));
    v << testq{ptr, ptr, sf};
    unlikelyCond(
      v, vcold(), CC_Z, sf, dst,
      [&] (Vout& v) { return call_helper(v); },
      [&] (Vout& v) { return ptr; }
    );
  }
}

void CodeGenerator::cgLdClsCachedSafe(IRInstruction* inst) {
  const StringData* className = inst->src(0)->strVal();
  auto ch = NamedEntity::get(className)->getClassHandle();
  auto const dst = dstLoc(inst, 0).reg();
  auto& v = vmain();

  if (rds::isNormalHandle(ch)) {
    auto const sf = checkRDSHandleInitialized(v, ch);
    emitFwdJcc(v, CC_NE, sf, inst->taken());
  }
  emitLdLowPtr(v, rvmtl()[ch], dst, sizeof(LowPtr<Class>));
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  auto const ch = ClassCache::alloc();
  rds::recordRds(ch, sizeof(ClassCache),
                 "ClassCache", getFunc(inst->marker())->fullName()->data());
  cgCallHelper(vmain(),
               CallSpec::direct(ClassCache::lookup),
               callDest(inst),
               SyncOptions::Sync,
               argGroup(inst).imm(ch).ssa(0/*className*/));
}

void CodeGenerator::cgLdRDSAddr(IRInstruction* inst) {
  auto const handle = inst->extra<LdRDSAddr>()->handle;
  vmain() << lea{rvmtl()[handle], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgAKExistsArr(IRInstruction* inst) {
  auto const arrTy = inst->src(0)->type();
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  auto const keyInfo = checkStrictlyInteger(arrTy, keyTy);
  auto const target =
    keyInfo.checkForInt ? CallSpec::direct(ak_exist_string) :
    keyInfo.type == KeyType::Int ? CallSpec::array(&g_array_funcs.existsInt)
                                 : CallSpec::array(&g_array_funcs.existsStr);
  auto args = argGroup(inst).ssa(0);
  if (keyInfo.converted) {
    args.imm(keyInfo.convertedInt);
  } else {
    args.ssa(1);
  }

  cgCallHelper(
    v,
    target,
    callDest(inst),
    SyncOptions::None,
    args
  );
}

void CodeGenerator::cgAKExistsObj(IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  cgCallHelper(
    v,
    keyTy <= TInt
      ? CallSpec::direct(ak_exist_int_obj)
      : CallSpec::direct(ak_exist_string_obj),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgLdGblAddr(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  cgCallHelper(v,
               CallSpec::direct(ldGblAddrHelper),
               callDest(dstReg),
               SyncOptions::None,
               argGroup(inst).ssa(0));
  auto const sf = v.makeReg();
  v << testq{dstReg, dstReg, sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

Vreg CodeGenerator::emitTestZero(Vout& v, SSATmp* src, Vloc srcLoc) {
  auto reg = srcLoc.reg();
  auto const sf = v.makeReg();
  if (src->isA(TBool)) {
    v << testb{reg, reg, sf};
  } else {
    v << testq{reg, reg, sf};
  }
  return sf;
}

void CodeGenerator::cgJmpZero(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitTestZero(v, inst->src(0), srcLoc(inst, 0));
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitTestZero(v, inst->src(0), srcLoc(inst, 0));
  v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgJmp(IRInstruction* inst) {
  auto& v = vmain();
  auto target = label(inst->taken());
  auto arity = inst->numSrcs();
  if (arity == 0) {
    v << jmp{target};
    return;
  }

  auto& def = inst->taken()->front();
  always_assert(arity == def.numDsts());
  VregList args;
  for (unsigned i = 0; i < arity; i++) {
    auto src = inst->src(i);
    auto sloc = srcLoc(inst, i);
    auto dloc = m_state.locs[def.dst(i)];
    always_assert(sloc.numAllocated() <= dloc.numAllocated());
    always_assert(dloc.numAllocated() >= 1);
    auto valReg = sloc.reg(0);
    if (src->isA(TBool) && !def.dst(i)->isA(TBool)) {
      valReg = v.makeReg();
      v << movzbq{sloc.reg(0), valReg};
    }
    args.push_back(valReg); // handle value
    if (dloc.numAllocated() == 2) { // handle type
      auto type = sloc.numAllocated() == 2 ? sloc.reg(1) :
                  v.cns(src->type().toDataType());
      args.push_back(type);
    }
  }
  v << phijmp{target, v.makeTuple(std::move(args))};
}

void CodeGenerator::cgDefLabel(IRInstruction* inst) {
  auto arity = inst->numDsts();
  if (arity == 0) return;
  auto& v = vmain();
  VregList args;
  for (unsigned i = 0; i < arity; i++) {
    auto dloc = dstLoc(inst, i);
    args.push_back(dloc.reg(0));
    if (dloc.numAllocated() == 2) {
      args.push_back(dloc.reg(1));
    } else {
      always_assert(dloc.numAllocated() == 1);
    }
  }
  v << phidef{v.makeTuple(std::move(args))};
}

void CodeGenerator::cgJmpSSwitchDest(IRInstruction* inst) {
  auto const extra = inst->extra<JmpSSwitchDest>();
  auto const m = inst->marker();
  auto& v = vmain();
  maybe_syncsp(v, m, srcLoc(inst, 1).reg(), extra->offset);
  v << jmpr{srcLoc(inst, 0).reg(), cross_trace_args(m)};
}

void CodeGenerator::cgNewCol(IRInstruction* inst) {
  auto& v = vmain();
  auto const dest = callDest(inst);
  auto args = argGroup(inst);
  auto const target = [&]() -> CallSpec {
    auto collectionType = inst->extra<NewCol>()->type;
    auto helper = collections::allocEmptyFunc(collectionType);
    return CallSpec::direct(helper);
  }();
  cgCallHelper(v, target, dest, SyncOptions::Sync, args);
}

void CodeGenerator::cgNewColFromArray(IRInstruction* inst) {
  auto const target = [&]() -> CallSpec {
    auto collectionType = inst->extra<NewColFromArray>()->type;
    auto helper = collections::allocFromArrayFunc(collectionType);
    return CallSpec::direct(helper);
  }();

  cgCallHelper(vmain(),
               target,
               callDest(inst),
               SyncOptions::Sync,
               argGroup(inst).ssa(0));
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* taken = inst->taken();
  assertx(taken);
  SSATmp* src = inst->src(0);

  if (!src->type().maybe(TUninit)) return;

  auto typeReg = srcLoc(inst, 0).reg(1);
  assertx(typeReg != InvalidReg);

  static_assert(KindOfUninit == 0, "cgCheckInit assumes KindOfUninit == 0");
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testb{typeReg, typeReg, sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgCheckInitMem(IRInstruction* inst) {
  Block* taken = inst->taken();
  assertx(taken);
  SSATmp* base = inst->src(0);
  Type t = base->type().deref();
  if (!t.maybe(TUninit)) return;
  auto basereg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, basereg[TVOFF(m_type)]);
  v << jcc{CC_Z, sf, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgCheckSurpriseFlags(IRInstruction* inst) {
  // This is not a correctness assertion, but we want to know if we get it
  // wrong because it'll be a subtle perf bug:
  if (inst->marker().resumed()) {
    assertx(inst->src(0)->isA(TStkPtr));
  } else {
    assertx(inst->src(0)->isA(TFramePtr));
  }
  auto& v = vmain();
  auto const fp_or_sp = srcLoc(inst, 0).reg();
  auto const sf = v.makeReg();
  v << cmpqm{fp_or_sp, rvmtl()[rds::kSurpriseFlagsOff], sf};
  v << jcc{CC_NBE, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckCold(IRInstruction* inst) {
  Block*     taken = inst->taken();
  TransID  transId = inst->extra<CheckCold>()->transId;
  auto counterAddr = profData()->transCounterAddr(transId);
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << decqmlock{v.cns(counterAddr)[0], sf};
  v << jcc{CC_LE, sf, {label(inst->next()), label(taken)}};
}

static const StringData* s_ReleaseVV = makeStaticString("ReleaseVV");

void CodeGenerator::cgReleaseVVAndSkip(IRInstruction* inst) {
  auto* const label = inst->taken();
  auto const rFp = srcLoc(inst, 0).reg();
  auto& v = vmain();

  TargetProfile<ReleaseVVProfile> profile(m_state.unit.context(),
                                          inst->marker(), s_ReleaseVV);
  if (profile.profiling()) {
    v << incwm{rvmtl()[profile.handle() + offsetof(ReleaseVVProfile, executed)],
               v.makeReg()};
  }

  auto const sf = v.makeReg();
  v << cmpqim{0, rFp[AROFF(m_varEnv)], sf};

  bool releaseUnlikely = true;
  if (profile.optimizing()) {
    auto const data = profile.data(ReleaseVVProfile::reduce);
    FTRACE(3, "cgReleaseVVAndSkip({}): percentReleased = {}\n",
           inst->toString(), data.percentReleased());
    if (data.percentReleased() >= RuntimeOption::EvalJitPGOReleaseVVMinPercent)
    {
      releaseUnlikely = false;
    }
  }
  ifThen(v, vcold(), CC_NZ, sf, [&] (Vout& v) {
    if (profile.profiling()) {
      auto offsetof_release = offsetof(ReleaseVVProfile, released);
      v << incwm{rvmtl()[profile.handle() + offsetof_release], v.makeReg()};
    }
    auto const sf = v.makeReg();
    v << testqim{safe_cast<int32_t>(ActRec::kExtraArgsBit),
                 rFp[AROFF(m_varEnv)],
                 sf};
    ifThenElse(v, vcold(), CC_NZ, sf, [&] (Vout& v) {
        cgCallHelper(
          v,
          CallSpec::direct(static_cast<void (*)(ActRec*)>(
                            ExtraArgs::deallocate)),
          kVoidDest,
          SyncOptions::Sync,
          argGroup(inst).reg(rFp)
        );
      },
      [&] (Vout& v) {
        cgCallHelper(
          v,
          CallSpec::direct(static_cast<void (*)(ActRec*)>(
                            VarEnv::deallocate)),
          kVoidDest,
          SyncOptions::Sync,
          argGroup(inst).reg(rFp)
        );
        v << jmp{m_state.labels[label]};
      }, true /* else is unlikely */);
  },
  releaseUnlikely);
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  auto const extra = inst->extra<InterpOne>();
  auto const pcOff = extra->bcOff;
  auto const spOff = extra->spOffset;
  auto const op    = extra->opcode;
  auto const interpOneHelper = interpOneEntryPoints[size_t(op)];

  cgCallHelper(
    vmain(),
    CallSpec::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
    kVoidDest,
    SyncOptions::None, // interpOne syncs regs manually
    argGroup(inst)
      .ssa(1/*fp*/)
      .addr(srcLoc(inst, 0).reg(), cellsToBytes(spOff.offset))
      .imm(pcOff)
  );
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  auto const extra = inst->extra<InterpOneCF>();
  auto const op    = extra->opcode;
  auto const spOff = extra->spOffset;
  auto const pcOff = extra->bcOff;

  auto& v = vmain();
  auto const adjustedSp = v.makeReg();
  v << lea{srcLoc(inst, 0).reg()[cellsToBytes(spOff.offset)], adjustedSp};
  v << syncvmsp{adjustedSp};

  assertx(mcg->ustubs().interpOneCFHelpers.count(op));

  // We pass the Offset in the third argument register.
  v << ldimml{pcOff, rarg(2)};
  v << jmpi{mcg->ustubs().interpOneCFHelpers.at(op),
            interp_one_cf_regs()};
}

void CodeGenerator::cgContEnter(IRInstruction* inst) {
  auto const extra     = inst->extra<ContEnter>();
  auto const curSpReg  = srcLoc(inst, 0).reg();
  auto const curFpReg  = srcLoc(inst, 1).reg();
  auto const genFpReg  = srcLoc(inst, 2).reg();
  auto const addrReg   = srcLoc(inst, 3).reg();
  auto const spOff     = extra->spOffset;
  auto const returnOff = extra->returnBCOffset;
  auto& v = vmain();

  auto const catchBlock = m_state.labels[inst->taken()];
  auto const next = v.makeBlock();

  v << store{curFpReg, genFpReg[AROFF(m_sfp)]};
  v << storeli{returnOff, genFpReg[AROFF(m_soff)]};
  v << copy{genFpReg, curFpReg};
  auto const sync_sp = v.makeReg();
  v << lea{curSpReg[cellsToBytes(spOff.offset)], sync_sp};
  v << syncvmsp{sync_sp};
  v << contenter{curFpReg, addrReg, cross_trace_regs_resumed(),
                 {next, catchBlock}};
  m_state.catch_calls[inst->taken()] = CatchCall::PHP;
  v = next;
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg      = srcLoc(inst, 0).reg();
  auto checkStarted = inst->src(1)->boolVal();
  auto isAsync      = inst->extra<IsAsyncData>()->isAsync;
  auto stateOff     = BaseGenerator::stateOff() - genOffset(isAsync);
  auto& v           = vmain();
  auto const sf     = v.makeReg();

  // These asserts make sure that the startedCheck work.
  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");
  static_assert(uint8_t(BaseGenerator::State::Started) == 1, "used below");
  static_assert(uint8_t(BaseGenerator::State::Done) > 3, "");
  // These asserts ensure that the state transition works. If we're in the
  // Created state we want to transition to Priming, and if we're in the
  // Started state we want to transition to Running. By laying out the enum
  // this way we can avoid the branch and just transition by adding 2 to the
  // current state.
  static_assert(uint8_t(BaseGenerator::State::Priming) ==
                uint8_t(BaseGenerator::State::Created) +  2, "used below");
  static_assert(uint8_t(BaseGenerator::State::Running) ==
                uint8_t(BaseGenerator::State::Started) +  2, "used below");

  // Take exit if state != 1 (checkStarted) or state > 1 (!checkStarted).
  v << cmpbim{int8_t(BaseGenerator::State::Started), contReg[stateOff], sf};
  emitFwdJcc(v, checkStarted ? CC_NE : CC_A, sf, inst->taken());

  // Transition the generator into either the Priming state (if we were just
  // created) or the Running state (if we were started). Due to the way the
  // enum is layed out, we can model this by just adding 2.
  auto const isf = v.makeReg();
  v << addlim{int8_t(2), contReg[stateOff], isf};
}

void CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  auto contReg  = srcLoc(inst, 0).reg();
  auto isAsync  = inst->extra<IsAsyncData>()->isAsync;
  auto stateOff = BaseGenerator::stateOff() - genOffset(isAsync);
  auto& v       = vmain();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");

  // Take exit if state == 0.
  auto const sf = v.makeReg();
  v << testbim{int8_t(0xffu), contReg[stateOff], sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgContStarted(IRInstruction* inst) {
  auto contReg  = srcLoc(inst, 0).reg();
  auto dstReg   = dstLoc(inst, 0).reg();
  auto stateOff = BaseGenerator::stateOff() - genOffset(false /* isAsync */);
  auto& v       = vmain();

  // Return 1 if generator state is not in the Created state.
  auto const sf = v.makeReg();
  v << cmpbim{int8_t(BaseGenerator::State::Created), contReg[stateOff], sf};
  v << setcc{CC_NE, sf, dstReg};
}

void CodeGenerator::cgContValid(IRInstruction* inst) {
  auto contReg  = srcLoc(inst, 0).reg();
  auto dstReg   = dstLoc(inst, 0).reg();
  auto isAsync  = inst->extra<IsAsyncData>()->isAsync;
  auto stateOff = BaseGenerator::stateOff() - genOffset(isAsync);
  auto& v       = vmain();

  // Return 1 if generator state is not Done.
  auto const sf = v.makeReg();
  v << cmpbim{int8_t(BaseGenerator::State::Done), contReg[stateOff], sf};
  v << setcc{CC_NE, sf, dstReg};
}

void CodeGenerator::cgContArIncKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  v << incqm{contArReg[GENDATAOFF(m_key) + TVOFF(m_data)
             - Generator::arOff()], v.makeReg()};
}

void CodeGenerator::cgContArUpdateIdx(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto newIdxReg = srcLoc(inst, 1).reg();
  int64_t off = GENDATAOFF(m_index) - Generator::arOff();
  auto& v = vmain();
  auto mem_index = v.makeReg();
  auto res = v.makeReg();
  v << load{contArReg[off], mem_index};
  auto const sf = v.makeReg();
  v << cmpq{mem_index, newIdxReg, sf};
  v << cmovq{CC_G, sf, mem_index, newIdxReg, res};
  v << store{res, contArReg[off]};
}

void CodeGenerator::cgLdContActRec(IRInstruction* inst) {
  auto dest = dstLoc(inst, 0).reg();
  auto base = srcLoc(inst, 0).reg();
  auto isAsync  = inst->extra<IsAsyncData>()->isAsync;
  auto offset = BaseGenerator::arOff() - genOffset(isAsync);
  vmain() << lea{base[offset], dest};
}

void CodeGenerator::cgLdContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t valueOff = GENDATAOFF(m_value);
  int64_t off = valueOff - Generator::arOff();
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), contArReg[off]);
}

void CodeGenerator::cgStContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t valueOff = GENDATAOFF(m_value);
  const int64_t off = valueOff - Generator::arOff();
  storeTV(vmain(), contArReg[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgLdContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t keyOff = GENDATAOFF(m_key);
  int64_t off = keyOff - Generator::arOff();
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), contArReg[off]);
}

void CodeGenerator::cgStContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t keyOff = GENDATAOFF(m_key);
  const int64_t off = keyOff - Generator::arOff();
  storeTV(vmain(), contArReg[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgStAsyncArSucceeded(IRInstruction* inst) {
  auto const off = c_WaitHandle::stateOff()
                 - c_AsyncFunctionWaitHandle::arOff();
  vmain() << storebi{
    c_WaitHandle::toKindState(
      c_WaitHandle::Kind::AsyncFunction,
      c_WaitHandle::STATE_SUCCEEDED
    ),
    srcLoc(inst, 0).reg()[off]
  };
}

void CodeGenerator::resumableStResumeImpl(IRInstruction* inst,
                                          ptrdiff_t offAddr,
                                          ptrdiff_t offOffset) {
  vmain() << store{
    srcLoc(inst, 1).reg(),
    srcLoc(inst, 0).reg()[offAddr]
  };
  vmain() << storeli{
    inst->extra<ResumeOffset>()->off,
    srcLoc(inst, 0).reg()[offOffset]
  };
}

void CodeGenerator::cgStAsyncArResume(IRInstruction* inst) {
  resumableStResumeImpl(
    inst,
    c_AsyncFunctionWaitHandle::resumeAddrOff() -
      c_AsyncFunctionWaitHandle::arOff(),
    c_AsyncFunctionWaitHandle::resumeOffsetOff() -
      c_AsyncFunctionWaitHandle::arOff()
  );
}

void CodeGenerator::cgStContArResume(IRInstruction* inst) {
  resumableStResumeImpl(
    inst,
    BaseGenerator::resumeAddrOff() - BaseGenerator::arOff(),
    BaseGenerator::resumeOffsetOff() - BaseGenerator::arOff()
  );
}

void CodeGenerator::cgLdContResumeAddr(IRInstruction* inst) {
  auto isAsync  = inst->extra<IsAsyncData>()->isAsync;
  vmain() << load{
    srcLoc(inst, 0).reg()[
      BaseGenerator::resumeAddrOff() - genOffset(isAsync)],
    dstLoc(inst, 0).reg()
  };
}

void CodeGenerator::cgContArIncIdx(IRInstruction* inst) {
  auto& v = vmain();
  auto const idxOff = GENDATAOFF(m_index) - Generator::arOff();
  auto const dst    = dstLoc(inst, 0).reg();
  auto const src    = srcLoc(inst, 0).reg()[idxOff];
  auto const tmp    = v.makeReg();
  v << load{src, tmp};
  v << incq{tmp, dst, v.makeReg()};
  v << store{dst, src};
}

void CodeGenerator::cgStContArState(IRInstruction* inst) {
  auto const off = BaseGenerator::stateOff() - BaseGenerator::arOff();
  vmain() << storebi{
    static_cast<int8_t>(inst->extra<StContArState>()->state),
    srcLoc(inst, 0).reg()[off]
  };
}

void CodeGenerator::cgStAsyncArResult(IRInstruction* inst) {
  auto asyncArReg = srcLoc(inst, 0).reg();
  const int64_t off = c_AsyncFunctionWaitHandle::resultOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  storeTV(vmain(), asyncArReg[off], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgLdAsyncArParentChain(IRInstruction* inst) {
  auto asyncArReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  const int64_t off = c_AsyncFunctionWaitHandle::parentChainOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  auto& v = vmain();
  v << load{asyncArReg[off], dstReg};
}

void CodeGenerator::cgAFWHBlockOn(IRInstruction* inst) {
  typedef c_AsyncFunctionWaitHandle::Node Node;
  auto parentArReg = srcLoc(inst, 0).reg();
  auto childReg = srcLoc(inst, 1).reg();
  auto& v = vmain();
  const int8_t afblocked = c_WaitHandle::toKindState(
    c_WaitHandle::Kind::AsyncFunction,
    c_AsyncFunctionWaitHandle::STATE_BLOCKED
  );
  const int64_t firstParentOff = c_WaitableWaitHandle::parentChainOff()
                               + AsioBlockableChain::firstParentOff();
  const int64_t stateToArOff = c_AsyncFunctionWaitHandle::stateOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t nextParentToArOff = c_AsyncFunctionWaitHandle::childrenOff()
                                  + Node::blockableOff()
                                  + AsioBlockable::bitsOff()
                                  - c_AsyncFunctionWaitHandle::arOff();
  const int64_t childToArOff = c_AsyncFunctionWaitHandle::childrenOff()
                             + Node::childOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t blockableToArOff = c_AsyncFunctionWaitHandle::childrenOff()
                                 + Node::blockableOff()
                                 - c_AsyncFunctionWaitHandle::arOff();

  // parent->setState(STATE_BLOCKED);
  v << storebi{afblocked, parentArReg[stateToArOff]};

  // parent->m_blockable.m_bits = child->m_parentChain.m_firstParent|Kind::AFWH;
  auto firstParent = v.makeReg();
  assertx(uint8_t(AsioBlockable::Kind::AsyncFunctionWaitHandleNode) == 0);
  v << load{childReg[firstParentOff], firstParent};
  v << store{firstParent, parentArReg[nextParentToArOff]};

  // child->m_parentChain.m_firstParent = &parent->m_blockable;
  auto objToAr = v.makeReg();
  v << lea{parentArReg[blockableToArOff], objToAr};
  v << store{objToAr, childReg[firstParentOff]};

  // parent->m_child = child;
  v << store{childReg, parentArReg[childToArOff]};
}

// Packing the TV type and data into two registers.
void emitPackTVRegs(Vout& v, Vloc loc, const SSATmp* src,
                    Vreg rData, Vreg rType) {
  auto const type = src->type();
  auto const typeShort = v.makeReg();

  assertx(!loc.isFullSIMD());
  if (type.needsReg()) {
    assertx(loc.hasReg(1));
    v << copy{loc.reg(1), typeShort};
  } else {
    v << copy{v.cns(type.toDataType()), typeShort};
  }
  // We are passing the packed registers to asyncRetCtrl unique stub, which
  // expects both rType and rData to be i64 regs.
  v << movzbq{typeShort, rType};

  // Ignore the values of null type
  if (!(src->isA(TNull))) {
    if (src->hasConstVal()) {
      // Skip potential zero-extend if we know the value.
      v << copy{v.cns(src->rawVal()), rData};
    } else {
      assertx(loc.hasReg(0));
      auto const extended = zeroExtendIfBool(v, src, loc.reg(0));
      v << copy{extended, rData};
    }
  }
}

void CodeGenerator::cgAsyncRetFast(IRInstruction* inst) {
  auto const ret = inst->src(2);
  auto const retLoc = srcLoc(inst, 2);
  auto& v = vmain();

  adjustSPForReturn(m_state, inst);

  // The asyncRetCtrl stub takes the return TV as its arguments.
  emitPackTVRegs(v, retLoc, ret, rarg(0), rarg(1));
  auto args = vm_regs_with_sp() | rarg(1);
  if (!ret->isA(TNull)) args |= rarg(0);

  v << jmpi{mcg->ustubs().asyncRetCtrl, args};
}

void CodeGenerator::cgAsyncSwitchFast(IRInstruction* inst) {
  auto& v = vmain();
  adjustSPForReturn(m_state, inst);
  prepare_return_regs(v, inst->src(2), srcLoc(inst, 2),
                      inst->extra<AsyncSwitchFast>()->aux);
  v << jmpi{mcg->ustubs().asyncSwitchCtrl, php_return_regs()};
}

void CodeGenerator::cgIsWaitHandle(IRInstruction* inst) {
  auto const robj = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();

  static_assert(
    ObjectData::IsWaitHandle < 0xff,
    "we use byte instructions for IsWaitHandle"
  );
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testwim{ObjectData::IsWaitHandle, robj[ObjectData::attributeOff()], sf};
  v << setcc{CC_NZ, sf, rdst};
}

void CodeGenerator::cgLdWHState(IRInstruction* inst) {
  auto const robj = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto state = v.makeReg();
  v << loadzbq{robj[c_WaitHandle::stateOff()], state};
  v << andqi{0x0F, state, rdst, v.makeReg()};
}

void CodeGenerator::cgLdWHResult(IRInstruction* inst) {
  auto const robj = srcLoc(inst, 0).reg();
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0),
         robj[c_WaitHandle::resultOff()]);
}

void CodeGenerator::cgLdAFWHActRec(IRInstruction* inst) {
  auto const dest = dstLoc(inst, 0).reg();
  auto const base = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto asyncArOffset = c_AsyncFunctionWaitHandle::arOff();
  v << lea{base[asyncArOffset], dest};
}

void CodeGenerator::cgLdResumableArObj(IRInstruction* inst) {
  auto const dstReg = dstLoc(inst, 0).reg();
  auto const resumableArReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const objectOff = Resumable::dataOff() - Resumable::arOff();
  v << lea{resumableArReg[objectOff], dstReg};
}

void CodeGenerator::cgNewStructArray(IRInstruction* inst) {
  auto const data = inst->extra<NewStructData>();
  if (!RuntimeOption::EvalDisableStructArray) {
    if (auto shape = Shape::create(data->keys, data->numKeys)) {
      StructArray* (*f)(uint32_t, const TypedValue*, Shape*) =
        &MixedArray::MakeStructArray;
      cgCallHelper(vmain(),
        CallSpec::direct(f),
        callDest(inst),
        SyncOptions::None,
        argGroup(inst)
          .imm(data->numKeys)
          .addr(srcLoc(inst, 0).reg(), cellsToBytes(data->offset.offset))
          .imm(shape)
      );
      return;
    }
  }

  auto table = vmain().allocData<const StringData*>(data->numKeys);
  memcpy(table, data->keys, data->numKeys * sizeof(*data->keys));
  MixedArray* (*f)(uint32_t, const StringData* const*, const TypedValue*) =
    &MixedArray::MakeStruct;
  cgCallHelper(
    vmain(),
    CallSpec::direct(f),
    callDest(inst),
    SyncOptions::None,
    argGroup(inst)
      .imm(data->numKeys)
      .dataPtr(table)
      .addr(srcLoc(inst, 0).reg(), cellsToBytes(data->offset.offset))
  );
}

void CodeGenerator::cgIncStat(IRInstruction *inst) {
  auto stat = Stats::StatCounter(inst->src(0)->intVal());
  int n = inst->src(1)->intVal();
  bool force = inst->src(2)->boolVal();
  emitIncStat(vmain(), stat, n, force);
}

void CodeGenerator::cgIncTransCounter(IRInstruction* inst) {
  emitTransCounterInc(vmain());
}

void CodeGenerator::cgIncProfCounter(IRInstruction* inst) {
  auto const transId = inst->extra<TransIDData>()->transId;
  auto const counterAddr = profData()->transCounterAddr(transId);
  auto& v = vmain();
  v << decqmlock{v.cns(counterAddr)[0], v.makeReg()};
}

void CodeGenerator::cgDbgTraceCall(IRInstruction* inst) {
  auto const spOff = inst->extra<DbgTraceCall>()->offset;
  cgCallHelper(
    vmain(),
    CallSpec::direct(traceCallback),
    callDest(inst),
    SyncOptions::None,
    argGroup(inst)
      .ssa(0)
      .addr(srcLoc(inst, 1).reg(), cellsToBytes(spOff.offset))
      .imm(inst->marker().bcOff())
  );
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  ifRefCountedType(
    vmain(), vmain(), inst->src(0)->type(), srcLoc(inst, 0),
    [&] (Vout& v) {
      emitAssertRefCount(v, srcLoc(inst, 0).reg());
    }
  );
}

void CodeGenerator::cgDbgAssertType(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  auto data_reg = srcLoc(inst, 0).reg(0);
  auto type_reg = srcLoc(inst, 0).reg(0);
  irlower::emitTypeTest(
    vmain(), m_state, inst->typeParam(), type_reg, data_reg, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      ifThen(v, ccNegate(cc), sfTaken, [&](Vout& v) {
        v << ud2{};
      });
    });
}

void CodeGenerator::cgDbgAssertFunc(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  auto r0 = srcLoc(inst, 0).reg(0);
  auto r1 = srcLoc(inst, 1).reg(0);
  v << cmpq{r0, r1, sf};
  ifThen(v, CC_NE, sf, [&](Vout& v) {
    v << ud2{};
  });
}

void CodeGenerator::emitVerifyCls(IRInstruction* inst) {
  auto const objClass = inst->src(0);
  auto const objClassReg = srcLoc(inst, 0).reg();
  auto const constraint = inst->src(1);
  auto const constraintReg = srcLoc(inst, 1).reg();
  auto& v = vmain();
  if (constraint->hasConstVal(TCls) && objClass->hasConstVal()) {
    if (objClass->clsVal() != constraint->clsVal()) {
      cgCallNative(v, inst);
    }
    return;
  }
  auto const sf = v.makeReg();
  if (!constraint->hasConstVal(TCls) && objClass->hasConstVal()) {
    // Reverse the args because cmpq can only have a constant in the LHS.
    v << cmpq{objClassReg, constraintReg, sf};
  } else {
    v << cmpq{constraintReg, objClassReg, sf};
  }

  // The native call for this instruction is the slow path that does
  // proper subtype checking. The comparison above is just to
  // short-circuit the overhead when the Classes are an exact match.
  ifThen(v, CC_NE, sf, [&](Vout& v) { cgCallNative(v, inst); });
}

void CodeGenerator::cgVerifyParamCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgVerifyRetCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgRBTraceEntry(IRInstruction* inst) {
  auto const& extra = *inst->extra<RBTraceEntry>();
  auto& v = vmain();
  auto args = argGroup(inst);
  cgCallHelper(v,
               CallSpec::direct(Trace::ringbufferEntryRip),
               kVoidDest,
               SyncOptions::None,
               argGroup(inst)
                 .imm(extra.type)
                 .imm(extra.sk.toAtomicInt()));
}

void CodeGenerator::cgRBTraceMsg(IRInstruction* inst) {
  auto const& extra = *inst->extra<RBTraceMsg>();
  auto& v = vmain();
  assertx(extra.msg->isStatic());
  cgCallHelper(v,
               CallSpec::direct(Trace::ringbufferMsg),
               kVoidDest,
               SyncOptions::None,
               argGroup(inst)
                 .immPtr(extra.msg->data())
                 .imm(extra.msg->size())
                 .imm(extra.type));
}

void CodeGenerator::cgLdClsInitData(IRInstruction* inst) {
  const Vreg rds = rvmtl();
  auto clsReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  auto offset = Class::propDataCacheOff() +
                rds::Link<Class::PropInitVec*>::handleOff();
  auto& v = vmain();
  auto handle = v.makeReg();
  auto vec = v.makeReg();
  v << loadzlq{clsReg[offset], handle};
  v << load{rds[handle], vec};
  v << load{vec[Class::PropInitVec::dataOff()], dstReg};
}

void CodeGenerator::cgConjure(IRInstruction* inst) {
  auto dst = dstLoc(inst, 0);
  auto& v = vmain();
  if (dst.hasReg(0)) {
    v << conjure{dst.reg(0)};
  }
  if (dst.hasReg(1)) {
    v << conjure{dst.reg(1)};
  }
}

void CodeGenerator::cgConjureUse(IRInstruction* inst) {
  auto src = srcLoc(inst, 0);
  auto& v = vmain();
  if (src.hasReg(0)) {
    v << conjureuse{src.reg(0)};
  }
  if (src.hasReg(1)) {
    v << conjureuse{src.reg(1)};
  }
}

void CodeGenerator::cgCountArray(IRInstruction* inst) {
  auto const baseReg = srcLoc(inst, 0).reg();
  auto const dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto dst1 = v.makeReg();

  v << loadl{baseReg[ArrayData::offsetofSize()], dst1};
  auto const sf = v.makeReg();
  v << testl{dst1, dst1, sf};

  unlikelyCond(v, vcold(), CC_S, sf, dstReg,
    [&](Vout& v) {
      auto dst2 = v.makeReg();
      cgCallHelper(v, CallSpec::array(&g_array_funcs.vsize),
                   callDest(dst2), SyncOptions::None,
                   argGroup(inst).ssa(0/*base*/));
      return dst2;
    },
    [&](Vout& v) {
      return dst1;
    }
  );
}

void CodeGenerator::cgCountArrayFast(IRInstruction* inst) {
  auto const baseReg = srcLoc(inst, 0).reg();
  auto const dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << loadzlq{baseReg[ArrayData::offsetofSize()], dstReg};
}

void CodeGenerator::cgCountCollection(IRInstruction* inst) {
  auto const baseReg = srcLoc(inst, 0).reg();
  auto const dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << loadzlq{baseReg[collections::FAST_SIZE_OFFSET], dstReg};
}

void CodeGenerator::cgLdStrLen(IRInstruction* inst) {
  vmain() << loadzlq{
    srcLoc(inst, 0).reg()[StringData::sizeOff()],
    dstLoc(inst, 0).reg()
  };
}

void CodeGenerator::cgLdFuncNumParams(IRInstruction* inst) {
  auto& v = vmain();
  auto dst = dstLoc(inst, 0).reg();
  auto src = srcLoc(inst, 0).reg()[Func::paramCountsOff()];
  auto tmp = v.makeReg();
  // See Func::finishedEmittingParams and Func::numParams.
  v << loadzlq{src, tmp};
  v << shrqi{1, tmp, dst, v.makeReg()};
}

void CodeGenerator::cgInitPackedArray(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  auto const index = inst->extra<InitPackedArray>()->index;

  auto slotOffset = PackedArray::entriesOffset() + index * sizeof(TypedValue);
  storeTV(vmain(), arrReg[slotOffset], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgInitPackedArrayLoop(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  int const count = inst->extra<InitPackedArrayLoop>()->size;
  auto const offset = inst->extra<InitPackedArrayLoop>()->offset;
  auto const spIn = srcLoc(inst, 1).reg();

  auto& v = vmain();
  auto const firstEntry = PackedArray::entriesOffset();

  auto const sp = v.makeReg();
  v << lea{spIn[cellsToBytes(offset.offset)], sp};

  auto const i = v.cns(0);
  auto const j = v.cns((count - 1) * 2);

  // We know that we have at least one element in the array so we don't have to
  // do an initial bounds check.
  assertx(count);

  doWhile(v, CC_GE, {i, j},
    [&] (const VregList& in, const VregList& out) {
      auto const i1 = in[0],  j1 = in[1];
      auto const i2 = out[0], j2 = out[1];
      auto const sf = v.makeReg();
      auto const value = v.makeReg();

      // Load the value from the stack and store into the array.  It's safe to
      // copy all 16 bytes of the TV because packed arrays don't use m_aux.
      v << loadups{sp[j1 * 8], value};
      v << storeups{value, arrReg[i1 * 8] + firstEntry};

      // Add 2 to the loop variable because we can only scale by at most 8.
      v << lea{i1[2], i2};
      v << subqi{2, j1, j2, sf};
      return sf;
    }
  );
}

void CodeGenerator::cgLdStructArrayElem(IRInstruction* inst) {
  auto const array = srcLoc(inst, 0).reg();
  auto const key = inst->src(1)->strVal();
  auto const shape = inst->src(0)->type().arrSpec().shape();
  auto const offset = shape->offsetFor(key);
  assertx(offset != PropertyTable::kInvalidOffset);

  auto const actualOffset = StructArray::dataOffset() +
    sizeof(TypedValue) * offset;
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), array[actualOffset]);
}

void CodeGenerator::cgEnterFrame(IRInstruction* inst) {
  vmain() << phplogue{srcLoc(inst, 0).reg()};
}

void CodeGenerator::cgCheckStackOverflow(IRInstruction* inst) {
  auto const func = inst->marker().func();
  auto const fp = srcLoc(inst, 0).reg();

  auto& v = vmain();
  auto const r = v.makeReg();
  auto const sf = v.makeReg();

  auto const stackMask = int32_t{
    cellsToBytes(RuntimeOption::EvalVMStackElms) - 1
  };
  auto const depth = cellsToBytes(func->maxStackCells()) +
                     kStackCheckPadding * sizeof(Cell) +
                     Stack::sSurprisePageSize;

  v << andqi{stackMask, fp, r, v.makeReg()};
  v << subqi{safe_cast<int32_t>(depth), r, v.makeReg(), sf};

  unlikelyIfThen(v, vcold(), CC_L, sf, [&] (Vout& v) {
    cgCallHelper(v, CallSpec::direct(handleStackOverflow), kVoidDest,
                 SyncOptions::Sync, argGroup(inst).reg(fp));
  });
}

void CodeGenerator::cgInitExtraArgs(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  auto const extra = inst->extra<InitExtraArgs>();
  auto const func = extra->func;
  auto const argc = extra->argc;

  using Action = ExtraArgsAction;

  auto& v = vmain();
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

void CodeGenerator::cgInitCtx(IRInstruction* inst) {
  auto const ptr = srcLoc(inst, 0).reg();
  auto const ctx = srcLoc(inst, 1).reg();
  vmain() << store{ctx, ptr[AROFF(m_this)]};
}

void CodeGenerator::cgCheckSurpriseFlagsEnter(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  auto const extra = inst->extra<CheckSurpriseFlagsEnter>();
  auto const func = extra->func;
  auto const argc = extra->argc;

  auto const off = func->getEntryForNumArgs(argc) - func->base();
  auto const fixup = Fixup(off, func->numSlotsInFrame());

  auto const catchBlock = m_state.labels[inst->taken()];
  emitCheckSurpriseFlagsEnter(vmain(), vcold(), fp, rvmtl(), fixup, catchBlock);
  m_state.catch_calls[inst->taken()] = CatchCall::CPP;
}

void CodeGenerator::cgCheckSurpriseAndStack(IRInstruction* inst) {
  auto const fp    = srcLoc(inst, 0).reg();
  auto const extra = inst->extra<CheckSurpriseAndStack>();
  auto const func  = extra->func;
  auto const argc  = extra->argc;
  auto const off   = func->getEntryForNumArgs(argc) - func->base();
  auto const fixup = Fixup(off, func->numSlotsInFrame());

  auto& v = vmain();
  auto const sf = v.makeReg();
  auto const needed_top = v.makeReg();
  v << lea{fp[-cellsToBytes(func->maxStackCells())], needed_top};
  v << cmpqm{needed_top, rvmtl()[rds::kSurpriseFlagsOff], sf};
  unlikelyIfThen(v, vcold(), CC_AE, sf, [&] (Vout& v) {
    auto const stub = mcg->ustubs().functionSurprisedOrStackOverflow;
    auto const done = v.makeBlock();
    v << vinvoke{
      CallSpec::stub(stub),
      v.makeVcallArgs({}),
      v.makeTuple({}),
      {done, m_state.labels[inst->taken()]},
      fixup
    };
    v = done;
  });

  m_state.catch_calls[inst->taken()] = CatchCall::CPP;
}

void CodeGenerator::cgCheckARMagicFlag(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();

  auto& v = vmain();
  auto const sf = v.makeReg();

  auto const mask = static_cast<int32_t>(ActRec::Flags::MagicDispatch);

  if (mask & (mask - 1)) {
    auto const tmp = v.makeReg();
    auto const arflags = v.makeReg();
    // need to test multiple bits
    v << loadl{fp[AROFF(m_numArgsAndFlags)], arflags};
    v << andli{mask, arflags, tmp, v.makeReg()};
    v << cmpli{mask, tmp, sf};
    v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
  } else {
    v << testlim{mask, fp[AROFF(m_numArgsAndFlags)], sf};
    v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
  }
}

void CodeGenerator::cgLdARNumArgsAndFlags(IRInstruction* inst) {
  auto fp = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  vmain() << loadzlq{fp[AROFF(m_numArgsAndFlags)], dst};
}

void CodeGenerator::cgStARNumArgsAndFlags(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  auto const val = srcLoc(inst, 1).reg();
  auto &v = vmain();
  auto const tmp = v.makeReg();
  v << movtql{val, tmp};
  v << storel{tmp, fp[AROFF(m_numArgsAndFlags)]};
}

void CodeGenerator::cgLdTVAux(IRInstruction* inst) {
  auto const tv = srcLoc(inst, 0);
  assertx(tv.hasReg(1));
  auto const type = tv.reg(1);
  auto const dst = dstLoc(inst, 0).reg();

  auto& v = vmain();
  v << shrqi{32, type, dst, v.makeReg()};

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto const extra = inst->extra<LdTVAux>();
    auto const mask = -extra->valid - 1;

    if (mask) {
      auto const sf = v.makeReg();
      v << testqi{mask, dst, sf};
      ifThen(v, CC_NZ, sf, [](Vout& v) {
        v << ud2{};
      });
    }
  }
}

void CodeGenerator::cgLdARInvName(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  auto const dst = dstLoc(inst, 0).reg();
  vmain() << load{fp[AROFF(m_invName)], dst};
}

void CodeGenerator::cgStARInvName(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  auto const val = srcLoc(inst, 1).reg();
  vmain() << store{val, fp[AROFF(m_invName)]};
}

void CodeGenerator::cgPackMagicArgs(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();

  auto& v = vmain();
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

  cgCallHelper(
    v,
    CallSpec::direct(PackedArray::MakePacked),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .reg(num_args)
      .reg(values)
  );
}

void CodeGenerator::cgProfileMethod(IRInstruction* inst) {
  auto const extra = inst->extra<ProfileMethodData>();
  auto const sp = srcLoc(inst, 0).reg();

  auto& v = vmain();
  auto const profile = v.makeReg();
  auto const ar = v.makeReg();
  v << lea{rvmtl()[extra->handle], profile};
  v << lea{sp[cellsToBytes(extra->spOffset.offset)], ar};
  cgCallHelper(v, CallSpec::direct(profileClassMethodHelper),
               kVoidDest, SyncOptions::None,
               argGroup(inst).reg(profile).reg(ar).ssa(1));
}

void CodeGenerator::cgProfileType(IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  auto& v = vmain();
  auto const profile = v.makeReg();
  v << lea{rvmtl()[extra->handle], profile};
  cgCallHelper(v, CallSpec::direct(profileTypeHelper), kVoidDest,
               SyncOptions::None, argGroup(inst).reg(profile).typedValue(0));
}

void CodeGenerator::cgSetOpCell(IRInstruction* inst) {
  auto const op = inst->extra<SetOpData>()->op;
  auto const helper = [&] {
    switch (op) {
      case SetOpOp::PlusEqual:   return cellAddEq;
      case SetOpOp::MinusEqual:  return cellSubEq;
      case SetOpOp::MulEqual:    return cellMulEq;
      case SetOpOp::ConcatEqual: return cellConcatEq;
      case SetOpOp::DivEqual:    return cellDivEq;
      case SetOpOp::PowEqual:    return cellPowEq;
      case SetOpOp::ModEqual:    return cellModEq;
      case SetOpOp::AndEqual:    return cellBitAndEq;
      case SetOpOp::OrEqual:     return cellBitOrEq;
      case SetOpOp::XorEqual:    return cellBitXorEq;
      case SetOpOp::SlEqual:     return cellShlEq;
      case SetOpOp::SrEqual:     return cellShrEq;
      case SetOpOp::PlusEqualO:  return cellAddEqO;
      case SetOpOp::MinusEqualO: return cellSubEqO;
      case SetOpOp::MulEqualO:   return cellMulEqO;
    }
    not_reached();
  }();

  auto& v = vmain();
  cgCallHelper(v, CallSpec::direct(helper),
               kVoidDest, SyncOptions::Sync,
               argGroup(inst).ssa(0).typedValue(1));
}

///////////////////////////////////////////////////////////////////////////////

}}}
