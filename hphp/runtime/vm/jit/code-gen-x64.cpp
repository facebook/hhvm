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

DELEGATE_OPCODE(FinishMemberOp)
DELEGATE_OPCODE(CheckRefs)

DELEGATE_OPCODE(AddElemStrKey)
DELEGATE_OPCODE(AddElemIntKey)
DELEGATE_OPCODE(AddNewElem)
DELEGATE_OPCODE(DictAddElemStrKey)
DELEGATE_OPCODE(DictAddElemIntKey)
DELEGATE_OPCODE(ArrayAdd)
DELEGATE_OPCODE(MapAddElemC)
DELEGATE_OPCODE(ColAddNewElemC)

DELEGATE_OPCODE(ConvIntToBool);
DELEGATE_OPCODE(ConvDblToBool);
DELEGATE_OPCODE(ConvStrToBool);
DELEGATE_OPCODE(ConvArrToBool);
DELEGATE_OPCODE(ConvObjToBool);
DELEGATE_OPCODE(ConvCellToBool);

DELEGATE_OPCODE(ConvBoolToInt);
DELEGATE_OPCODE(ConvDblToInt);
DELEGATE_OPCODE(ConvStrToInt);
DELEGATE_OPCODE(ConvArrToInt);
DELEGATE_OPCODE(ConvObjToInt);
DELEGATE_OPCODE(ConvResToInt);
DELEGATE_OPCODE(ConvCellToInt);

DELEGATE_OPCODE(ConvBoolToDbl);
DELEGATE_OPCODE(ConvIntToDbl);
DELEGATE_OPCODE(ConvStrToDbl);
DELEGATE_OPCODE(ConvArrToDbl);
DELEGATE_OPCODE(ConvObjToDbl);
DELEGATE_OPCODE(ConvResToDbl);
DELEGATE_OPCODE(ConvCellToDbl);

DELEGATE_OPCODE(ConvBoolToStr);
DELEGATE_OPCODE(ConvIntToStr);
DELEGATE_OPCODE(ConvDblToStr);
DELEGATE_OPCODE(ConvObjToStr);
DELEGATE_OPCODE(ConvResToStr);
DELEGATE_OPCODE(ConvCellToStr);

DELEGATE_OPCODE(ConvBoolToArr);
DELEGATE_OPCODE(ConvDblToArr);
DELEGATE_OPCODE(ConvIntToArr);
DELEGATE_OPCODE(ConvStrToArr);
DELEGATE_OPCODE(ConvVecToArr);
DELEGATE_OPCODE(ConvDictToArr);
DELEGATE_OPCODE(ConvKeysetToArr);
DELEGATE_OPCODE(ConvObjToArr);
DELEGATE_OPCODE(ConvCellToArr);

DELEGATE_OPCODE(ConvArrToVec);
DELEGATE_OPCODE(ConvDictToVec);
DELEGATE_OPCODE(ConvKeysetToVec);
DELEGATE_OPCODE(ConvObjToVec);

DELEGATE_OPCODE(ConvArrToDict);
DELEGATE_OPCODE(ConvVecToDict);
DELEGATE_OPCODE(ConvKeysetToDict);
DELEGATE_OPCODE(ConvObjToDict);

DELEGATE_OPCODE(ConvArrToKeyset);
DELEGATE_OPCODE(ConvVecToKeyset);
DELEGATE_OPCODE(ConvDictToKeyset);
DELEGATE_OPCODE(ConvObjToKeyset);

DELEGATE_OPCODE(ConvCellToObj);

// Vector instruction helpers
DELEGATE_OPCODE(StringGet)
DELEGATE_OPCODE(BindElem)
DELEGATE_OPCODE(SetWithRefElem)
DELEGATE_OPCODE(SetOpElem)
DELEGATE_OPCODE(IncDecElem)
DELEGATE_OPCODE(SetNewElem)
DELEGATE_OPCODE(SetNewElemArray)
DELEGATE_OPCODE(SetNewElemVec)
DELEGATE_OPCODE(BindNewElem)
DELEGATE_OPCODE(VectorIsset)
DELEGATE_OPCODE(PairIsset)

DELEGATE_OPCODE(Count)

DELEGATE_OPCODE(GetMemoKey)

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

DELEGATE_OPCODE(ElemVecD)
DELEGATE_OPCODE(ElemVecU)

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
DELEGATE_OPCODE(SetOpCell)

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

DELEGATE_OPCODE(ProfileArrayKind)
DELEGATE_OPCODE(CountArray)
DELEGATE_OPCODE(CountArrayFast)
DELEGATE_OPCODE(CountVec)
DELEGATE_OPCODE(CountDict)
DELEGATE_OPCODE(CountKeyset)
DELEGATE_OPCODE(CheckPackedArrayBounds)
DELEGATE_OPCODE(AKExistsArr)
DELEGATE_OPCODE(AKExistsDict)
DELEGATE_OPCODE(AKExistsKeyset)
DELEGATE_OPCODE(AKExistsObj)
DELEGATE_OPCODE(NewArray)
DELEGATE_OPCODE(NewStructArray)
DELEGATE_OPCODE(NewMixedArray)
DELEGATE_OPCODE(NewLikeArray)
DELEGATE_OPCODE(NewDictArray)
DELEGATE_OPCODE(NewKeysetArray)
DELEGATE_OPCODE(AllocPackedArray)
DELEGATE_OPCODE(AllocVecArray)
DELEGATE_OPCODE(InitPackedLayoutArray)
DELEGATE_OPCODE(InitPackedLayoutArrayLoop)

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
DELEGATE_OPCODE(LdPackedArrayElemAddr)
DELEGATE_OPCODE(LdVecElemAddr)
DELEGATE_OPCODE(LdVecElem)
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
DELEGATE_OPCODE(MapIdx)

DELEGATE_OPCODE(IsCol)
DELEGATE_OPCODE(ColIsEmpty)
DELEGATE_OPCODE(ColIsNEmpty)
DELEGATE_OPCODE(CountCollection)
DELEGATE_OPCODE(NewCol)
DELEGATE_OPCODE(NewColFromArray)
DELEGATE_OPCODE(LdColArray)
DELEGATE_OPCODE(LdVectorSize)
DELEGATE_OPCODE(LdVectorBase)
DELEGATE_OPCODE(VectorHasImmCopy)
DELEGATE_OPCODE(VectorDoCow)
DELEGATE_OPCODE(LdPairBase)

DELEGATE_OPCODE(LdCls)
DELEGATE_OPCODE(LdClsCached)
DELEGATE_OPCODE(LdClsCachedSafe)
DELEGATE_OPCODE(LookupClsRDS)
DELEGATE_OPCODE(LdFunc)
DELEGATE_OPCODE(LdFuncCached)
DELEGATE_OPCODE(LdFuncCachedU)
DELEGATE_OPCODE(LdFuncCachedSafe)
DELEGATE_OPCODE(LdArrFuncCtx)
DELEGATE_OPCODE(LdArrFPushCuf)
DELEGATE_OPCODE(LdStrFPushCuf)
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

}}}
