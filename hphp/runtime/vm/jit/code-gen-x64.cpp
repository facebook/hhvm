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
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"

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

NOOP_OPCODE(FinishMemberOp)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(DictAddElemStrKey)
CALL_OPCODE(DictAddElemIntKey)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(MapAddElemC)
CALL_OPCODE(ColAddNewElemC)

CALL_OPCODE(ConvBoolToArr);
CALL_OPCODE(ConvDblToArr);
CALL_OPCODE(ConvIntToArr);
CALL_OPCODE(ConvObjToArr);
CALL_OPCODE(ConvStrToArr);
CALL_OPCODE(ConvVecToArr);
CALL_OPCODE(ConvDictToArr);
CALL_OPCODE(ConvKeysetToArr);
CALL_OPCODE(ConvCellToArr);

CALL_OPCODE(ConvArrToVec);
CALL_OPCODE(ConvDictToVec);
CALL_OPCODE(ConvKeysetToVec);
CALL_OPCODE(ConvObjToVec);

CALL_OPCODE(ConvArrToDict);
CALL_OPCODE(ConvVecToDict);
CALL_OPCODE(ConvKeysetToDict);
CALL_OPCODE(ConvObjToDict);

CALL_OPCODE(ConvArrToKeyset);
CALL_OPCODE(ConvVecToKeyset);
CALL_OPCODE(ConvDictToKeyset);
CALL_OPCODE(ConvObjToKeyset);

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

CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewDictArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(AllocPackedArray)
CALL_OPCODE(AllocVecArray)

CALL_OPCODE(MapIdx)

// Vector instruction helpers
CALL_OPCODE(StringGet)
CALL_OPCODE(BindElem)
CALL_OPCODE(SetWithRefElem)
CALL_OPCODE(SetOpElem)
CALL_OPCODE(IncDecElem)
CALL_OPCODE(SetNewElem)
CALL_OPCODE(SetNewElemArray)
CALL_OPCODE(SetNewElemVec)
CALL_OPCODE(BindNewElem)
CALL_OPCODE(VectorIsset)
CALL_OPCODE(PairIsset)

CALL_OPCODE(Count)

CALL_OPCODE(GetMemoKey)

DELEGATE_OPCODE(Nop)
DELEGATE_OPCODE(DefConst)
DELEGATE_OPCODE(EndGuards)
DELEGATE_OPCODE(ExitPlaceholder)
DELEGATE_OPCODE(DefFP)
DELEGATE_OPCODE(DefSP)
DELEGATE_OPCODE(EagerSyncVMRegs)
DELEGATE_OPCODE(Mov)
DELEGATE_OPCODE(Halt)
DELEGATE_OPCODE(InterpOne)
DELEGATE_OPCODE(InterpOneCF)
DELEGATE_OPCODE(PrintBool)
DELEGATE_OPCODE(PrintInt)
DELEGATE_OPCODE(PrintStr)
DELEGATE_OPCODE(RBTraceEntry)
DELEGATE_OPCODE(RBTraceMsg)

DELEGATE_OPCODE(IncStat)
DELEGATE_OPCODE(IncStatGrouped)
DELEGATE_OPCODE(IncTransCounter)
DELEGATE_OPCODE(IncProfCounter)
DELEGATE_OPCODE(CheckCold)

CALL_OPCODE(ElemVecD)
CALL_OPCODE(ElemVecU)

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
DELEGATE_OPCODE(CheckRange)
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
DELEGATE_OPCODE(GtStr);
DELEGATE_OPCODE(GteStr);
DELEGATE_OPCODE(LtStr);
DELEGATE_OPCODE(LteStr);
DELEGATE_OPCODE(EqStr);
DELEGATE_OPCODE(NeqStr);
DELEGATE_OPCODE(SameStr);
DELEGATE_OPCODE(NSameStr);
DELEGATE_OPCODE(CmpStr);
DELEGATE_OPCODE(GtStrInt);
DELEGATE_OPCODE(GteStrInt);
DELEGATE_OPCODE(LtStrInt);
DELEGATE_OPCODE(LteStrInt);
DELEGATE_OPCODE(EqStrInt);
DELEGATE_OPCODE(NeqStrInt);
DELEGATE_OPCODE(CmpStrInt);
DELEGATE_OPCODE(GtObj);
DELEGATE_OPCODE(GteObj);
DELEGATE_OPCODE(LtObj);
DELEGATE_OPCODE(LteObj);
DELEGATE_OPCODE(EqObj);
DELEGATE_OPCODE(NeqObj);
DELEGATE_OPCODE(SameObj)
DELEGATE_OPCODE(NSameObj)
DELEGATE_OPCODE(CmpObj);
DELEGATE_OPCODE(GtArr);
DELEGATE_OPCODE(GteArr);
DELEGATE_OPCODE(LtArr);
DELEGATE_OPCODE(LteArr);
DELEGATE_OPCODE(EqArr);
DELEGATE_OPCODE(NeqArr);
DELEGATE_OPCODE(SameArr);
DELEGATE_OPCODE(NSameArr);
DELEGATE_OPCODE(CmpArr);
DELEGATE_OPCODE(GtVec);
DELEGATE_OPCODE(GteVec);
DELEGATE_OPCODE(LtVec);
DELEGATE_OPCODE(LteVec);
DELEGATE_OPCODE(EqVec);
DELEGATE_OPCODE(NeqVec);
DELEGATE_OPCODE(SameVec);
DELEGATE_OPCODE(NSameVec);
DELEGATE_OPCODE(CmpVec);
DELEGATE_OPCODE(EqDict);
DELEGATE_OPCODE(NeqDict);
DELEGATE_OPCODE(SameDict);
DELEGATE_OPCODE(NSameDict);
DELEGATE_OPCODE(EqKeyset);
DELEGATE_OPCODE(NeqKeyset);
DELEGATE_OPCODE(GtRes);
DELEGATE_OPCODE(GteRes);
DELEGATE_OPCODE(LtRes);
DELEGATE_OPCODE(LteRes);
DELEGATE_OPCODE(EqRes)
DELEGATE_OPCODE(NeqRes)
DELEGATE_OPCODE(CmpRes);
DELEGATE_OPCODE(EqFunc)
DELEGATE_OPCODE(DbgAssertFunc)
DELEGATE_OPCODE(EqStrPtr)

DELEGATE_OPCODE(EqCls)
DELEGATE_OPCODE(ProfileInstanceCheck)
DELEGATE_OPCODE(InstanceOf)
DELEGATE_OPCODE(InstanceOfIface)
DELEGATE_OPCODE(InstanceOfIfaceVtable)
DELEGATE_OPCODE(ExtendsClass)
DELEGATE_OPCODE(InstanceOfBitmask)
DELEGATE_OPCODE(NInstanceOfBitmask)
DELEGATE_OPCODE(InterfaceSupportsArr)
DELEGATE_OPCODE(InterfaceSupportsVec)
DELEGATE_OPCODE(InterfaceSupportsDict)
DELEGATE_OPCODE(InterfaceSupportsKeyset)
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
DELEGATE_OPCODE(AssertType)
DELEGATE_OPCODE(AssertLoc)
DELEGATE_OPCODE(AssertStk)
DELEGATE_OPCODE(HintLocInner)
DELEGATE_OPCODE(HintStkInner)
DELEGATE_OPCODE(ProfileType)

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
DELEGATE_OPCODE(LdGblAddrDef)
DELEGATE_OPCODE(LdPropAddr)
DELEGATE_OPCODE(LdClsPropAddrOrNull)
DELEGATE_OPCODE(LdClsPropAddrOrRaise)
DELEGATE_OPCODE(LdRDSAddr)
DELEGATE_OPCODE(LdTVAux)

DELEGATE_OPCODE(IncRef)
DELEGATE_OPCODE(DecRef)
DELEGATE_OPCODE(DecRefNZ)
DELEGATE_OPCODE(DbgAssertRefCount)

DELEGATE_OPCODE(DefLabel)
DELEGATE_OPCODE(Jmp)
DELEGATE_OPCODE(JmpZero)
DELEGATE_OPCODE(JmpNZero)
DELEGATE_OPCODE(CheckNullptr)
DELEGATE_OPCODE(CheckNonNull)
DELEGATE_OPCODE(AssertNonNull)
DELEGATE_OPCODE(CheckInit)
DELEGATE_OPCODE(CheckInitMem)
DELEGATE_OPCODE(ProfileSwitchDest)
DELEGATE_OPCODE(JmpSwitchDest)
DELEGATE_OPCODE(JmpSSwitchDest)
DELEGATE_OPCODE(LdSSwitchDestFast)
DELEGATE_OPCODE(LdSSwitchDestSlow)
DELEGATE_OPCODE(LdSwitchDblIndex)
DELEGATE_OPCODE(LdSwitchStrIndex)
DELEGATE_OPCODE(LdSwitchObjIndex)

DELEGATE_OPCODE(ReqBindJmp)
DELEGATE_OPCODE(ReqRetranslate)
DELEGATE_OPCODE(ReqRetranslateOpt)
DELEGATE_OPCODE(LdBindAddr)

DELEGATE_OPCODE(LdClsCtx)
DELEGATE_OPCODE(LdClsCctx)
DELEGATE_OPCODE(ConvClsToCctx)
DELEGATE_OPCODE(CheckCtxThis)
DELEGATE_OPCODE(CheckFuncStatic)

DELEGATE_OPCODE(LdClsName)
DELEGATE_OPCODE(MethodExists)
DELEGATE_OPCODE(LdClsMethod)
DELEGATE_OPCODE(LdIfaceMethod)
DELEGATE_OPCODE(LdObjInvoke)
DELEGATE_OPCODE(HasToString)
DELEGATE_OPCODE(LdFuncVecLen)
DELEGATE_OPCODE(LdClsInitData)
DELEGATE_OPCODE(CheckInitProps)
DELEGATE_OPCODE(CheckInitSProps)
DELEGATE_OPCODE(InitProps)
DELEGATE_OPCODE(InitSProps)

DELEGATE_OPCODE(LdFuncNumParams)

DELEGATE_OPCODE(LdObjClass)
DELEGATE_OPCODE(AllocObj)
DELEGATE_OPCODE(NewInstanceRaw)
DELEGATE_OPCODE(ConstructInstance)
DELEGATE_OPCODE(Clone)
DELEGATE_OPCODE(RegisterLiveObj)
DELEGATE_OPCODE(InitObjProps)

DELEGATE_OPCODE(LdStrLen)
DELEGATE_OPCODE(OrdStr)
DELEGATE_OPCODE(OrdStrIdx)
DELEGATE_OPCODE(ChrInt)
DELEGATE_OPCODE(StringIsset)
DELEGATE_OPCODE(ConcatStrStr);
DELEGATE_OPCODE(ConcatStrInt);
DELEGATE_OPCODE(ConcatIntStr);
DELEGATE_OPCODE(ConcatStr3);
DELEGATE_OPCODE(ConcatStr4);

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
DELEGATE_OPCODE(ProfileDictOffset)
DELEGATE_OPCODE(CheckDictOffset)
DELEGATE_OPCODE(ProfileKeysetOffset)
DELEGATE_OPCODE(CheckKeysetOffset)
DELEGATE_OPCODE(ElemArray)
DELEGATE_OPCODE(ElemArrayW)
DELEGATE_OPCODE(ElemArrayD)
DELEGATE_OPCODE(ElemArrayU)
DELEGATE_OPCODE(ElemMixedArrayK)
DELEGATE_OPCODE(ElemDict)
DELEGATE_OPCODE(ElemDictW)
DELEGATE_OPCODE(ElemDictD)
DELEGATE_OPCODE(ElemDictU)
DELEGATE_OPCODE(ElemDictK)
DELEGATE_OPCODE(ElemKeyset)
DELEGATE_OPCODE(ElemKeysetW)
DELEGATE_OPCODE(ElemKeysetU)
DELEGATE_OPCODE(ElemKeysetK)
DELEGATE_OPCODE(ArrayGet)
DELEGATE_OPCODE(ArraySet)
DELEGATE_OPCODE(ArraySetRef)
DELEGATE_OPCODE(ArrayIsset)
DELEGATE_OPCODE(ArrayIdx)
DELEGATE_OPCODE(MixedArrayGetK)
DELEGATE_OPCODE(DictGet)
DELEGATE_OPCODE(DictGetQuiet)
DELEGATE_OPCODE(DictGetK)
DELEGATE_OPCODE(DictIsset)
DELEGATE_OPCODE(DictEmptyElem)
DELEGATE_OPCODE(DictSet)
DELEGATE_OPCODE(DictSetRef)
DELEGATE_OPCODE(DictIdx)
DELEGATE_OPCODE(VecSet)
DELEGATE_OPCODE(VecSetRef)
DELEGATE_OPCODE(KeysetGet)
DELEGATE_OPCODE(KeysetGetQuiet)
DELEGATE_OPCODE(KeysetGetK)
DELEGATE_OPCODE(SetNewElemKeyset)
DELEGATE_OPCODE(KeysetIsset)
DELEGATE_OPCODE(KeysetEmptyElem)
DELEGATE_OPCODE(KeysetIdx)
DELEGATE_OPCODE(MapGet)
DELEGATE_OPCODE(MapSet)
DELEGATE_OPCODE(MapIsset)

DELEGATE_OPCODE(LdCls)
DELEGATE_OPCODE(LdClsCached)
DELEGATE_OPCODE(LdClsCachedSafe)
DELEGATE_OPCODE(LookupClsRDS)
DELEGATE_OPCODE(LdFunc)
DELEGATE_OPCODE(LdFuncCached)
DELEGATE_OPCODE(LdFuncCachedU)
DELEGATE_OPCODE(LdFuncCachedSafe)
DELEGATE_OPCODE(OODeclExists)

DELEGATE_OPCODE(LdObjMethod)
DELEGATE_OPCODE(LookupClsMethod)
DELEGATE_OPCODE(LdClsCtor)
DELEGATE_OPCODE(ProfileMethod)
DELEGATE_OPCODE(LookupClsMethodCache)
DELEGATE_OPCODE(LdClsMethodCacheFunc)
DELEGATE_OPCODE(LdClsMethodCacheCls)
DELEGATE_OPCODE(LookupClsMethodFCache)
DELEGATE_OPCODE(LdClsMethodFCacheFunc)

DELEGATE_OPCODE(FwdCtxStaticCall)

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

DELEGATE_OPCODE(BeginCatch)
DELEGATE_OPCODE(EndCatch)
DELEGATE_OPCODE(UnwindCheckSideExit)
DELEGATE_OPCODE(LdUnwinderValue)
DELEGATE_OPCODE(DebugBacktrace)
DELEGATE_OPCODE(InitThrowableFileAndLine)
DELEGATE_OPCODE(ZeroErrorLevel)
DELEGATE_OPCODE(RestoreErrorLevel)

DELEGATE_OPCODE(RaiseArrayIndexNotice)
DELEGATE_OPCODE(RaiseArrayKeyNotice)
DELEGATE_OPCODE(RaiseError)
DELEGATE_OPCODE(RaiseMissingArg)
DELEGATE_OPCODE(RaiseNotice)
DELEGATE_OPCODE(RaiseUndefProp)
DELEGATE_OPCODE(RaiseUninitLoc)
DELEGATE_OPCODE(RaiseWarning)
DELEGATE_OPCODE(RaiseMissingThis)
DELEGATE_OPCODE(FatalMissingThis)
DELEGATE_OPCODE(ThrowArithmeticError)
DELEGATE_OPCODE(ThrowDivisionByZeroError)
DELEGATE_OPCODE(ThrowInvalidArrayKey)
DELEGATE_OPCODE(ThrowInvalidOperation);
DELEGATE_OPCODE(ThrowOutOfBounds)

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
DELEGATE_OPCODE(SuspendHookE)
DELEGATE_OPCODE(SuspendHookR)
DELEGATE_OPCODE(ReturnHook)

DELEGATE_OPCODE(CastStk)
DELEGATE_OPCODE(CastMem)
DELEGATE_OPCODE(CoerceStk)
DELEGATE_OPCODE(CoerceMem)
DELEGATE_OPCODE(CoerceCellToBool);
DELEGATE_OPCODE(CoerceCellToInt);
DELEGATE_OPCODE(CoerceCellToDbl);
DELEGATE_OPCODE(CoerceStrToDbl);
DELEGATE_OPCODE(CoerceStrToInt);
DELEGATE_OPCODE(VerifyParamCallable)
DELEGATE_OPCODE(VerifyRetCallable)
DELEGATE_OPCODE(VerifyParamCls)
DELEGATE_OPCODE(VerifyRetCls)
DELEGATE_OPCODE(VerifyParamFail)
DELEGATE_OPCODE(VerifyParamFailHard)
DELEGATE_OPCODE(VerifyRetFail)

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

///////////////////////////////////////////////////////////////////////////////

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
    auto const sf  = v.makeReg();
#ifdef NO_M_DATA
    v << cmpbim{'0', src[sizeof(StringData)], sf};
#else
    auto const sd  = v.makeReg();
    v << load{src[StringData::dataOff()], sd};
    v << cmpbim{'0', sd[0], sf};
#endif
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

void CodeGenerator::cgProfileArrayKind(IRInstruction* inst) {
  auto const extra = inst->extra<RDSHandleData>();
  auto& v = vmain();
  auto const profile = v.makeReg();
  v << lea{rvmtl()[extra->handle], profile};
  cgCallHelper(v, CallSpec::direct(profileArrayKindHelper), kVoidDest,
               SyncOptions::None, argGroup(inst).reg(profile).ssa(0));
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

Vptr CodeGenerator::emitPackedLayoutAddr(SSATmp* idx, Vloc idxLoc,
                                         Vloc arrLoc) {
  auto const rArr = arrLoc.reg();
  auto const rIdx = idxLoc.reg();
  auto& v = vmain();

  static_assert(sizeof(TypedValue) == 16, "");

  if (idx->hasConstVal()) {
    auto const offset =
      PackedArray::entriesOffset() + idx->intVal() * sizeof(TypedValue);
    if (deltaFits(offset, sz::dword)) {
      return rArr[offset];
    }
  }

  /*
   * This computes `rArr + rIdx * sizeof(TypedValue) +
   * PackedArray::entriesOffset()`. The logic of `scaledIdx * 16` is split in
   * the following two instructions, in order to save a byte in the shl
   * instruction.
   *
   * TODO(#7728856): We should really move this into vasm-x64.cpp...
   */
  auto idxl = v.makeReg();
  auto scaled_idxl = v.makeReg();
  auto scaled_idx = v.makeReg();
  v << movtql{rIdx, idxl};
  v << shlli{1, idxl, scaled_idxl, v.makeReg()};
  v << movzlq{scaled_idxl, scaled_idx};
  return rArr[scaled_idx * int(sizeof(TypedValue) / 2)
              + PackedArray::entriesOffset()];
}

void CodeGenerator::cgLdPackedArrayElemAddr(IRInstruction* inst) {
  auto const addr =
    emitPackedLayoutAddr(inst->src(1), srcLoc(inst, 1), srcLoc(inst, 0));
  vmain() << lea{addr, dstLoc(inst, 0).reg()};
}


void CodeGenerator::cgLdVecElem(IRInstruction* inst) {
  auto const addr =
    emitPackedLayoutAddr(inst->src(1), srcLoc(inst, 1), srcLoc(inst, 0));
  loadTV(vmain(), inst->dst(), dstLoc(inst, 0), addr);
}

void CodeGenerator::cgLdVecElemAddr(IRInstruction* inst) {
  auto const addr =
    emitPackedLayoutAddr(inst->src(1), srcLoc(inst, 1), srcLoc(inst, 0));
  vmain() << lea{addr, dstLoc(inst, 0).reg()};
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

void CodeGenerator::cgAKExistsDict(IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(MixedArray::ExistsIntDict)
    : CallSpec::direct(MixedArray::ExistsStrDict);

  cgCallHelper(
    v,
    target,
    callDest(inst),
    SyncOptions::None,
    argGroup(inst).ssa(0).ssa(1)
  );
}

void CodeGenerator::cgAKExistsKeyset(IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  auto const target = (keyTy <= TInt)
    ? CallSpec::direct(SetArray::ExistsInt)
    : CallSpec::direct(SetArray::ExistsStr);

  cgCallHelper(
    v,
    target,
    callDest(inst),
    SyncOptions::None,
    argGroup(inst).ssa(0).ssa(1)
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

void CodeGenerator::cgNewStructArray(IRInstruction* inst) {
  auto const data = inst->extra<NewStructData>();

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

void CodeGenerator::cgNewKeysetArray(IRInstruction* inst) {
  auto const data = inst->extra<NewKeysetArray>();
  cgCallHelper(
    vmain(),
    CallSpec::direct(SetArray::MakeSet),
    callDest(inst),
    SyncOptions::Sync,
    argGroup(inst)
      .imm(data->size)
      .addr(srcLoc(inst, 0).reg(), cellsToBytes(data->offset.offset))
  );
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

void CodeGenerator::arrayLikeCountImpl(IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");
  vmain() << loadzlq{
    srcLoc(inst, 0).reg()[ArrayData::offsetofSize()],
    dstLoc(inst, 0).reg()
  };
}

void CodeGenerator::cgCountArrayFast(IRInstruction* inst) {
  arrayLikeCountImpl(inst);
}

void CodeGenerator::cgCountVec(IRInstruction* inst) {
  arrayLikeCountImpl(inst);
}

void CodeGenerator::cgCountDict(IRInstruction* inst) {
  arrayLikeCountImpl(inst);
}

void CodeGenerator::cgCountKeyset(IRInstruction* inst) {
  arrayLikeCountImpl(inst);
}

void CodeGenerator::cgCountCollection(IRInstruction* inst) {
  auto const baseReg = srcLoc(inst, 0).reg();
  auto const dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << loadzlq{baseReg[collections::FAST_SIZE_OFFSET], dstReg};
}

void CodeGenerator::cgInitPackedLayoutArray(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  auto const index = inst->extra<InitPackedLayoutArray>()->index;

  auto slotOffset = PackedArray::entriesOffset() + index * sizeof(TypedValue);
  storeTV(vmain(), arrReg[slotOffset], srcLoc(inst, 1), inst->src(1));
}

void CodeGenerator::cgInitPackedLayoutArrayLoop(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  int const count = inst->extra<InitPackedLayoutArrayLoop>()->size;
  auto const offset = inst->extra<InitPackedLayoutArrayLoop>()->offset;
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
