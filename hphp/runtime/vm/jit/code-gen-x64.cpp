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

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

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

DELEGATE_OPCODE(LdLoc)
DELEGATE_OPCODE(LdLocAddr)
DELEGATE_OPCODE(StLoc)
DELEGATE_OPCODE(StLocRange)
DELEGATE_OPCODE(DbgTrashFrame)
DELEGATE_OPCODE(LdLocPseudoMain)
DELEGATE_OPCODE(StLocPseudoMain)
DELEGATE_OPCODE(LdStk)
DELEGATE_OPCODE(LdStkAddr)
DELEGATE_OPCODE(StStk)
DELEGATE_OPCODE(DbgTrashStk)
DELEGATE_OPCODE(LdMem)
DELEGATE_OPCODE(StMem)
DELEGATE_OPCODE(DbgTrashMem)
DELEGATE_OPCODE(LdRef)
DELEGATE_OPCODE(StRef)
DELEGATE_OPCODE(LdElem)
DELEGATE_OPCODE(StElem)
DELEGATE_OPCODE(LdMIStateAddr)
DELEGATE_OPCODE(LdMBase)
DELEGATE_OPCODE(StMBase)
DELEGATE_OPCODE(LdGblAddr)
DELEGATE_OPCODE(LdPropAddr)
DELEGATE_OPCODE(LdRDSAddr)
DELEGATE_OPCODE(LdTVAux)

DELEGATE_OPCODE(IncRef)
DELEGATE_OPCODE(IncRefCtx)
DELEGATE_OPCODE(DecRef)
DELEGATE_OPCODE(DecRefNZ)
DELEGATE_OPCODE(DbgAssertRefCount)

DELEGATE_OPCODE(DefLabel)
DELEGATE_OPCODE(Jmp)
DELEGATE_OPCODE(JmpZero)
DELEGATE_OPCODE(JmpNZero)
DELEGATE_OPCODE(ProfileSwitchDest)
DELEGATE_OPCODE(JmpSwitchDest)
DELEGATE_OPCODE(JmpSSwitchDest)
DELEGATE_OPCODE(LdSSwitchDestFast)
DELEGATE_OPCODE(LdSSwitchDestSlow)

DELEGATE_OPCODE(ReqBindJmp)
DELEGATE_OPCODE(ReqRetranslate)
DELEGATE_OPCODE(ReqRetranslateOpt)

DELEGATE_OPCODE(LdClsCtx)
DELEGATE_OPCODE(LdClsCctx)
DELEGATE_OPCODE(CastCtxThis)
DELEGATE_OPCODE(CheckCtxThis)

DELEGATE_OPCODE(LdClsName)
DELEGATE_OPCODE(LdClsMethod)
DELEGATE_OPCODE(LdIfaceMethod)
DELEGATE_OPCODE(LdObjInvoke)
DELEGATE_OPCODE(LdFuncVecLen)
DELEGATE_OPCODE(LdFuncNumParams)

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

DELEGATE_OPCODE(LdCls)
DELEGATE_OPCODE(LdClsCached)
DELEGATE_OPCODE(LdClsCachedSafe)
DELEGATE_OPCODE(LdFunc)
DELEGATE_OPCODE(LdFuncCached)
DELEGATE_OPCODE(LdFuncCachedU)
DELEGATE_OPCODE(LdFuncCachedSafe)

DELEGATE_OPCODE(LdObjMethod)
DELEGATE_OPCODE(LookupClsMethodCache)
DELEGATE_OPCODE(LdClsMethodCacheFunc)
DELEGATE_OPCODE(LdClsMethodCacheCls)
DELEGATE_OPCODE(LookupClsMethodFCache)
DELEGATE_OPCODE(LdClsMethodFCacheFunc)

DELEGATE_OPCODE(GetCtxFwdCall)
DELEGATE_OPCODE(GetCtxFwdCallDyn)

DELEGATE_OPCODE(LdCns)
DELEGATE_OPCODE(LookupCns)
DELEGATE_OPCODE(LookupCnsE)
DELEGATE_OPCODE(LookupCnsU)
DELEGATE_OPCODE(LdClsCns)
DELEGATE_OPCODE(InitClsCns)

DELEGATE_OPCODE(SpillFrame)
DELEGATE_OPCODE(CufIterSpillFrame)
DELEGATE_OPCODE(LdARFuncPtr)
DELEGATE_OPCODE(LdARNumArgsAndFlags)
DELEGATE_OPCODE(StARNumArgsAndFlags)
DELEGATE_OPCODE(LdARNumParams)
DELEGATE_OPCODE(CheckARMagicFlag)
DELEGATE_OPCODE(LdCtx)
DELEGATE_OPCODE(LdCctx)
DELEGATE_OPCODE(InitCtx)
DELEGATE_OPCODE(LdARInvName)
DELEGATE_OPCODE(StARInvName)
DELEGATE_OPCODE(InitExtraArgs)
DELEGATE_OPCODE(PackMagicArgs)

DELEGATE_OPCODE(Call)
DELEGATE_OPCODE(CallArray)
DELEGATE_OPCODE(CallBuiltin)
DELEGATE_OPCODE(NativeImpl)
DELEGATE_OPCODE(DbgTraceCall)
DELEGATE_OPCODE(EnterFrame)
DELEGATE_OPCODE(RetCtrl)
DELEGATE_OPCODE(AsyncRetCtrl)
DELEGATE_OPCODE(AsyncRetFast)
DELEGATE_OPCODE(AsyncSwitchFast)
DELEGATE_OPCODE(LdRetVal)
DELEGATE_OPCODE(DbgTrashRetVal)
DELEGATE_OPCODE(FreeActRec)
DELEGATE_OPCODE(GenericRetDecRefs)
DELEGATE_OPCODE(ReleaseVVAndSkip)

DELEGATE_OPCODE(BeginInlining)
DELEGATE_OPCODE(DefInlineFP)
DELEGATE_OPCODE(InlineReturn)
DELEGATE_OPCODE(InlineReturnNoFrame)
DELEGATE_OPCODE(SyncReturnBC)
DELEGATE_OPCODE(Conjure)
DELEGATE_OPCODE(ConjureUse)

DELEGATE_OPCODE(LdStaticLoc)
DELEGATE_OPCODE(InitStaticLoc)
DELEGATE_OPCODE(CheckClosureStaticLocInit)
DELEGATE_OPCODE(LdClosureStaticLoc)
DELEGATE_OPCODE(InitClosureStaticLoc)

DELEGATE_OPCODE(LdClosure)
DELEGATE_OPCODE(LdClosureCtx)
DELEGATE_OPCODE(StClosureCtx)
DELEGATE_OPCODE(StClosureArg)

DELEGATE_OPCODE(LdResumableArObj)
DELEGATE_OPCODE(CreateCont)
DELEGATE_OPCODE(ContEnter)
DELEGATE_OPCODE(ContPreNext)
DELEGATE_OPCODE(ContStartedCheck)
DELEGATE_OPCODE(ContStarted)
DELEGATE_OPCODE(ContValid)
DELEGATE_OPCODE(StContArState)
DELEGATE_OPCODE(LdContField)
DELEGATE_OPCODE(LdContActRec)
DELEGATE_OPCODE(LdContArValue)
DELEGATE_OPCODE(StContArValue)
DELEGATE_OPCODE(LdContResumeAddr)
DELEGATE_OPCODE(StContArResume)
DELEGATE_OPCODE(LdContArKey)
DELEGATE_OPCODE(StContArKey)
DELEGATE_OPCODE(ContArIncKey)
DELEGATE_OPCODE(ContArIncIdx)
DELEGATE_OPCODE(ContArUpdateIdx)
DELEGATE_OPCODE(CreateAFWH)
DELEGATE_OPCODE(CreateAFWHNoVV)
DELEGATE_OPCODE(CreateSSWH)
DELEGATE_OPCODE(AFWHPrepareChild)
DELEGATE_OPCODE(AFWHBlockOn)
DELEGATE_OPCODE(ABCUnblock)
DELEGATE_OPCODE(IsWaitHandle)
DELEGATE_OPCODE(LdWHState)
DELEGATE_OPCODE(StAsyncArSucceeded)
DELEGATE_OPCODE(LdWHResult)
DELEGATE_OPCODE(StAsyncArResult)
DELEGATE_OPCODE(LdAFWHActRec)
DELEGATE_OPCODE(LdAsyncArParentChain)
DELEGATE_OPCODE(StAsyncArResume)

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

DELEGATE_OPCODE(CheckSurpriseFlags)
DELEGATE_OPCODE(CheckStackOverflow)
DELEGATE_OPCODE(CheckSurpriseFlagsEnter)
DELEGATE_OPCODE(CheckSurpriseAndStack)

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

  constexpr uint64_t maxULong = std::numeric_limits<unsigned long>::max();
  constexpr uint64_t maxLong = std::numeric_limits<long>::max();

  auto rIndef = v.cns(0x8000000000000000L);
  auto dst1 = v.makeReg();
  v << cvttsd2siq{srcReg, dst1};
  auto const sf = v.makeReg();
  v << cmpq{rIndef, dst1, sf};
  unlikelyCond(v, vcold(), CC_E, sf, dstReg, [&](Vout& v) {
    // result > max signed int or unordered
    auto const sf = v.makeReg();
    v << ucomisd{v.cns(0.0), srcReg, sf};
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
        v << ucomisd{v.cns(static_cast<double>(maxULong)), srcReg, sf};
        return cond(v, CC_B, sf, v.makeReg(), [&](Vout& v) { // src0 > ULONG_MAX
            return v.cns(0);
        }, [&](Vout& v) {
          // 0 < src0 <= ULONG_MAX
          // we know that LONG_MAX < src0 <= UINT_MAX, therefore,
          // 0 < src0 - ULONG_MAX <= LONG_MAX
          auto tmp_sub = v.makeReg();
          auto tmp_int = v.makeReg();
          auto dst5 = v.makeReg();
          v << subsd{v.cns(static_cast<double>(maxLong)), srcReg, tmp_sub};
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
  auto const movtdq_res = v.makeReg();

  v << movtdq{src, movtdq_res};
  v << shlqi{1, movtdq_res, t1, sf}; // 0.0 stays zero and -0.0 is now 0.0
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

void CodeGenerator::cgLdBindAddr(IRInstruction* inst) {
  auto const extra  = inst->extra<LdBindAddr>();
  auto const dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();

  // Emit service request to smash address of SrcKey into 'addr'.
  auto const addrPtr = v.allocData<TCA>();
  v << bindaddr{addrPtr, extra->sk, extra->spOff};
  v << loadqd{reinterpret_cast<uint64_t*>(addrPtr), dstReg};
}

void CodeGenerator::cgEagerSyncVMRegs(IRInstruction* inst) {
  auto const spOff = inst->extra<EagerSyncVMRegs>()->offset;
  auto& v = vmain();
  auto const sync_sp = v.makeReg();
  v << lea{srcLoc(inst, 1).reg()[cellsToBytes(spOff.offset)], sync_sp};
  emitEagerSyncPoint(v, inst->marker().fixupSk().pc(),
                     rvmtl(), srcLoc(inst, 0).reg(), sync_sp);
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

void CodeGenerator::cgCheckCold(IRInstruction* inst) {
  Block*     taken = inst->taken();
  TransID  transId = inst->extra<CheckCold>()->transId;
  auto counterAddr = profData()->transCounterAddr(transId);
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << decqmlock{v.cns(counterAddr)[0], sf};
  v << jcc{CC_LE, sf, {label(inst->next()), label(taken)}};
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
