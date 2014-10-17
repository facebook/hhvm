/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/code-gen-arm.h"
#include <vector>

#include "folly/Optional.h"

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_generator.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/back-end-arm.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace jit { namespace arm {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

#define NOOP_OPCODE(name) void CodeGenerator::cg##name(IRInstruction*) {}

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(DefSP)
NOOP_OPCODE(TrackLoc)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(Nop)
NOOP_OPCODE(DefLabel)
NOOP_OPCODE(ExceptionBarrier)
NOOP_OPCODE(TakeStack)
NOOP_OPCODE(TakeRef)
NOOP_OPCODE(EndGuards)

// XXX
NOOP_OPCODE(DbgAssertPtr);

// When implemented this shouldn't be a nop, but there's no reason to make us
// punt on everything until then.
NOOP_OPCODE(DbgAssertRetAddr)

#undef NOOP_OPCODE

//////////////////////////////////////////////////////////////////////

#define CALL_OPCODE(name) \
  void CodeGenerator::cg##name(IRInstruction* i) { \
    cgCallNative(vmain(), i); \
  }

CALL_OPCODE(Box)
CALL_OPCODE(ConvIntToStr)

CALL_OPCODE(AllocObj)

CALL_OPCODE(ConcatStrStr)
CALL_OPCODE(ConcatIntStr)
CALL_OPCODE(ConcatStrInt)
CALL_OPCODE(ConcatStr3);
CALL_OPCODE(ConcatStr4);

CALL_OPCODE(PrintStr)
CALL_OPCODE(PrintInt)
CALL_OPCODE(PrintBool)

CALL_OPCODE(AddElemStrKey)

CALL_OPCODE(ConvBoolToArr)
CALL_OPCODE(ConvDblToArr)
CALL_OPCODE(ConvIntToArr)
CALL_OPCODE(ConvObjToArr)
CALL_OPCODE(ConvStrToArr)
CALL_OPCODE(ConvCellToArr)

CALL_OPCODE(ConvStrToBool)
CALL_OPCODE(ConvCellToBool)
CALL_OPCODE(ConvArrToDbl)
CALL_OPCODE(ConvObjToDbl)
CALL_OPCODE(ConvStrToDbl)
CALL_OPCODE(ConvCellToDbl)

CALL_OPCODE(ConvObjToInt)
CALL_OPCODE(ConvArrToInt)
CALL_OPCODE(ConvStrToInt)

CALL_OPCODE(RaiseWarning)
CALL_OPCODE(RaiseError)
CALL_OPCODE(ConvCellToObj)
CALL_OPCODE(LookupClsMethod)
CALL_OPCODE(RaiseNotice)
CALL_OPCODE(LookupClsRDSHandle)
CALL_OPCODE(LdSwitchStrIndex)
CALL_OPCODE(LdSwitchDblIndex)
CALL_OPCODE(LdSwitchObjIndex)
CALL_OPCODE(CustomInstanceInit)
CALL_OPCODE(LdClsCtor)

CALL_OPCODE(LdArrFuncCtx)
CALL_OPCODE(LdArrFPushCuf)
CALL_OPCODE(LdStrFPushCuf)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewVArray)
CALL_OPCODE(NewMIArray)
CALL_OPCODE(NewMSArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(NewPackedArray)
CALL_OPCODE(NewCol)
CALL_OPCODE(Clone)
CALL_OPCODE(ClosureStaticLocInit)
CALL_OPCODE(VerifyParamCallable)
CALL_OPCODE(VerifyParamFail)
CALL_OPCODE(WarnNonObjProp)
CALL_OPCODE(ThrowNonObjProp)
CALL_OPCODE(RaiseUndefProp)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ColAddElemC)
CALL_OPCODE(ColAddNewElemC)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(CreateCont)
CALL_OPCODE(CreateAFWH)
CALL_OPCODE(CreateSSWH)
CALL_OPCODE(AFWHPrepareChild)
CALL_OPCODE(ABCUnblock)
CALL_OPCODE(TypeProfileFunc)
CALL_OPCODE(IncStatGrouped)
CALL_OPCODE(ZeroErrorLevel)
CALL_OPCODE(RestoreErrorLevel)
CALL_OPCODE(Count)
CALL_OPCODE(CountArray)

/////////////////////////////////////////////////////////////////////
void cgPunt(const char* file, int line, const char* func, uint32_t bcOff,
            const Func* vmFunc, bool resumed, TransID profTransId) {
  FTRACE(1, "punting: {}\n", func);
  throw FailedCodeGen(file, line, func, bcOff, vmFunc, resumed, profTransId);
}

#define PUNT_OPCODE(name)                                               \
  void CodeGenerator::cg##name(IRInstruction* inst) {                   \
    cgPunt(__FILE__, __LINE__, #name, m_curInst->marker().bcOff(),      \
           curFunc(), resumed(), m_curInst->marker().profTransId());    \
  }

#define CG_PUNT(instr)                                              \
    cgPunt(__FILE__, __LINE__, #instr, m_curInst->marker().bcOff(), \
           curFunc(), resumed(), m_curInst->marker().profTransId())

/////////////////////////////////////////////////////////////////////
//TODO t3702757: Convert to CALL_OPCODE, the following set works on
//   x86 but needs a closer look on arm
PUNT_OPCODE(AddElemIntKey)
PUNT_OPCODE(ConvCellToInt)
PUNT_OPCODE(ArrayIdx)
PUNT_OPCODE(RaiseArrayIndexNotice)
PUNT_OPCODE(RaiseUninitLoc)
PUNT_OPCODE(VerifyRetCallable)
PUNT_OPCODE(VerifyRetFail)
PUNT_OPCODE(GenericIdx)
// End of failing set
/////////////////////////////////////////////////////////////////////

PUNT_OPCODE(ProfileStr)
PUNT_OPCODE(ConvArrToBool)
PUNT_OPCODE(ConvDblToBool)
PUNT_OPCODE(ConvIntToBool)
PUNT_OPCODE(ConvObjToBool)
PUNT_OPCODE(ConvBoolToDbl)
PUNT_OPCODE(ConvIntToDbl)

PUNT_OPCODE(ConvBoolToInt)
PUNT_OPCODE(ConvDblToInt)

PUNT_OPCODE(ConvBoolToStr)
PUNT_OPCODE(ConvDblToStr)
PUNT_OPCODE(ConvObjToStr)
PUNT_OPCODE(ConvResToStr)
PUNT_OPCODE(ConvCellToStr)

PUNT_OPCODE(CoerceCellToBool)
PUNT_OPCODE(CoerceCellToInt)
PUNT_OPCODE(CoerceCellToDbl)
PUNT_OPCODE(CoerceStrToDbl)
PUNT_OPCODE(CoerceStrToInt)

PUNT_OPCODE(ProfileArray)
PUNT_OPCODE(CheckTypeMem)
PUNT_OPCODE(CheckLoc)
PUNT_OPCODE(CastStk)
PUNT_OPCODE(CastStkIntToDbl)
PUNT_OPCODE(CoerceStk)
PUNT_OPCODE(CheckDefinedClsEq)
PUNT_OPCODE(TryEndCatch)
PUNT_OPCODE(CheckSideExit)
PUNT_OPCODE(LdUnwinderValue)
PUNT_OPCODE(DeleteUnwinderException)
PUNT_OPCODE(AddDbl)
PUNT_OPCODE(SubDbl)
PUNT_OPCODE(MulDbl)
PUNT_OPCODE(DivDbl)
PUNT_OPCODE(Mod)
PUNT_OPCODE(Sqrt)
PUNT_OPCODE(AbsDbl)
PUNT_OPCODE(XorBool)
PUNT_OPCODE(ExtendsClass)
PUNT_OPCODE(IsWaitHandle)
PUNT_OPCODE(InstanceOf)
PUNT_OPCODE(InstanceOfIface)
PUNT_OPCODE(InterfaceSupportsArr)
PUNT_OPCODE(InterfaceSupportsStr)
PUNT_OPCODE(InterfaceSupportsInt)
PUNT_OPCODE(InterfaceSupportsDbl)
PUNT_OPCODE(IsTypeMem)
PUNT_OPCODE(IsNTypeMem)
PUNT_OPCODE(Gt)
PUNT_OPCODE(Gte)
PUNT_OPCODE(Lt)
PUNT_OPCODE(Lte)
PUNT_OPCODE(Eq)
PUNT_OPCODE(Neq)
PUNT_OPCODE(GtDbl)
PUNT_OPCODE(GteDbl)
PUNT_OPCODE(LtDbl)
PUNT_OPCODE(LteDbl)
PUNT_OPCODE(EqDbl)
PUNT_OPCODE(NeqDbl)
PUNT_OPCODE(LtX)
PUNT_OPCODE(GtX)
PUNT_OPCODE(GteX)
PUNT_OPCODE(LteX)
PUNT_OPCODE(EqX)
PUNT_OPCODE(NeqX)
PUNT_OPCODE(Same)
PUNT_OPCODE(NSame)
PUNT_OPCODE(Floor)
PUNT_OPCODE(Ceil)
PUNT_OPCODE(InstanceOfBitmask)
PUNT_OPCODE(NInstanceOfBitmask)
PUNT_OPCODE(IsType)
PUNT_OPCODE(IsScalarType)
PUNT_OPCODE(IsNType)
PUNT_OPCODE(JmpGt)
PUNT_OPCODE(JmpGte)
PUNT_OPCODE(JmpLt)
PUNT_OPCODE(JmpLte)
PUNT_OPCODE(JmpEq)
PUNT_OPCODE(JmpNeq)
PUNT_OPCODE(JmpGtInt)
PUNT_OPCODE(JmpGteInt)
PUNT_OPCODE(JmpLtInt)
PUNT_OPCODE(JmpLteInt)
PUNT_OPCODE(JmpEqInt)
PUNT_OPCODE(JmpNeqInt)
PUNT_OPCODE(JmpSame)
PUNT_OPCODE(JmpNSame)
PUNT_OPCODE(JmpInstanceOfBitmask)
PUNT_OPCODE(JmpNInstanceOfBitmask)
PUNT_OPCODE(JmpZero)
PUNT_OPCODE(JmpNZero)
PUNT_OPCODE(ReqBindJmpGt)
PUNT_OPCODE(ReqBindJmpGte)
PUNT_OPCODE(ReqBindJmpLt)
PUNT_OPCODE(ReqBindJmpLte)
PUNT_OPCODE(ReqBindJmpEq)
PUNT_OPCODE(ReqBindJmpNeq)
PUNT_OPCODE(ReqBindJmpGtInt)
PUNT_OPCODE(ReqBindJmpGteInt)
PUNT_OPCODE(ReqBindJmpLtInt)
PUNT_OPCODE(ReqBindJmpLteInt)
PUNT_OPCODE(ReqBindJmpEqInt)
PUNT_OPCODE(ReqBindJmpNeqInt)
PUNT_OPCODE(ReqBindJmpSame)
PUNT_OPCODE(ReqBindJmpNSame)
PUNT_OPCODE(ReqBindJmpInstanceOfBitmask)
PUNT_OPCODE(ReqBindJmpNInstanceOfBitmask)
PUNT_OPCODE(ReqBindJmpZero)
PUNT_OPCODE(ReqBindJmpNZero)
PUNT_OPCODE(SideExitJmpGt)
PUNT_OPCODE(SideExitJmpGte)
PUNT_OPCODE(SideExitJmpLt)
PUNT_OPCODE(SideExitJmpLte)
PUNT_OPCODE(SideExitJmpEq)
PUNT_OPCODE(SideExitJmpNeq)
PUNT_OPCODE(SideExitJmpGtInt)
PUNT_OPCODE(SideExitJmpGteInt)
PUNT_OPCODE(SideExitJmpLtInt)
PUNT_OPCODE(SideExitJmpLteInt)
PUNT_OPCODE(SideExitJmpEqInt)
PUNT_OPCODE(SideExitJmpNeqInt)
PUNT_OPCODE(SideExitJmpSame)
PUNT_OPCODE(SideExitJmpNSame)
PUNT_OPCODE(SideExitJmpInstanceOfBitmask)
PUNT_OPCODE(SideExitJmpNInstanceOfBitmask)
PUNT_OPCODE(SideExitJmpZero)
PUNT_OPCODE(SideExitJmpNZero)
PUNT_OPCODE(SideExitGuardLoc)
PUNT_OPCODE(JmpIndirect)
PUNT_OPCODE(CheckSurpriseFlags)
PUNT_OPCODE(SurpriseHook)
PUNT_OPCODE(FunctionSuspendHook)
PUNT_OPCODE(FunctionReturnHook)
PUNT_OPCODE(ReleaseVVOrExit)
PUNT_OPCODE(CheckInit)
PUNT_OPCODE(CheckInitMem)
PUNT_OPCODE(CheckCold)
PUNT_OPCODE(CheckNullptr)
PUNT_OPCODE(CheckBounds)
PUNT_OPCODE(LdVectorSize)
PUNT_OPCODE(CheckPackedArrayBounds)
PUNT_OPCODE(CheckPackedArrayElemNull)
PUNT_OPCODE(VectorHasImmCopy)
PUNT_OPCODE(VectorDoCow)
PUNT_OPCODE(CheckNonNull)
PUNT_OPCODE(AssertNonNull)
PUNT_OPCODE(UnboxPtr)
PUNT_OPCODE(BoxPtr)
PUNT_OPCODE(LdVectorBase)
PUNT_OPCODE(LdPairBase)
PUNT_OPCODE(LdLocAddr)
PUNT_OPCODE(LdMem)
PUNT_OPCODE(LdProp)
PUNT_OPCODE(LdElem)
PUNT_OPCODE(LdPackedArrayElem)
PUNT_OPCODE(LdRef)
PUNT_OPCODE(LdGbl)
PUNT_OPCODE(LdThis)
PUNT_OPCODE(LdRetAddr)
PUNT_OPCODE(ConvClsToCctx)
PUNT_OPCODE(LdCtx)
PUNT_OPCODE(LdCctx)
PUNT_OPCODE(LdCls)
PUNT_OPCODE(LdClsCached)
PUNT_OPCODE(LdClsCachedSafe)
PUNT_OPCODE(LdClsCtx)
PUNT_OPCODE(LdClsCctx)
PUNT_OPCODE(LdClsCns)
PUNT_OPCODE(LookupClsCns)
PUNT_OPCODE(LdCns)
PUNT_OPCODE(LdClsInitData)
PUNT_OPCODE(LookupCns)
PUNT_OPCODE(LookupCnsE)
PUNT_OPCODE(LookupCnsU)
PUNT_OPCODE(DerefClsRDSHandle)
PUNT_OPCODE(LookupClsMethodCache)
PUNT_OPCODE(LdClsMethodCacheFunc)
PUNT_OPCODE(LdClsMethodCacheCls)
PUNT_OPCODE(LdClsMethodFCacheFunc)
PUNT_OPCODE(LookupClsMethodFCache)
PUNT_OPCODE(GetCtxFwdCallDyn);
PUNT_OPCODE(GetCtxFwdCall)
PUNT_OPCODE(LdClsMethod)
PUNT_OPCODE(LdPropAddr)
PUNT_OPCODE(LdClsPropAddrKnown)
PUNT_OPCODE(LdClsPropAddrOrNull)
PUNT_OPCODE(LdClsPropAddrOrRaise)
PUNT_OPCODE(LdObjMethod)
PUNT_OPCODE(LdObjInvoke)
PUNT_OPCODE(LdGblAddrDef)
PUNT_OPCODE(LdGblAddr)
PUNT_OPCODE(LdObjClass)
PUNT_OPCODE(LdFunc)
PUNT_OPCODE(LdFuncCachedU)
PUNT_OPCODE(LdFuncCachedSafe)
PUNT_OPCODE(LdBindAddr)
PUNT_OPCODE(LdSSwitchDestFast)
PUNT_OPCODE(LdSSwitchDestSlow)
PUNT_OPCODE(JmpSwitchDest)
PUNT_OPCODE(ConstructInstance)
PUNT_OPCODE(CheckInitProps)
PUNT_OPCODE(InitProps)
PUNT_OPCODE(CheckInitSProps)
PUNT_OPCODE(InitSProps)
PUNT_OPCODE(RegisterLiveObj)
PUNT_OPCODE(NewInstanceRaw)
PUNT_OPCODE(InitObjProps)
PUNT_OPCODE(StClosureFunc)
PUNT_OPCODE(StClosureArg)
PUNT_OPCODE(StClosureCtx)
PUNT_OPCODE(NewStructArray)
PUNT_OPCODE(FreeActRec)
PUNT_OPCODE(CallArray)
PUNT_OPCODE(NativeImpl)
PUNT_OPCODE(RetCtrl)
PUNT_OPCODE(StRetVal)
PUNT_OPCODE(RetAdjustStack)
PUNT_OPCODE(StMem)
PUNT_OPCODE(StProp)
PUNT_OPCODE(StLocNT)
PUNT_OPCODE(StRef)
PUNT_OPCODE(StRaw)
PUNT_OPCODE(StElem)
PUNT_OPCODE(LdStaticLocCached)
PUNT_OPCODE(CheckStaticLocInit)
PUNT_OPCODE(StaticLocInitCached)
PUNT_OPCODE(CufIterSpillFrame)
PUNT_OPCODE(ReqRetranslateOpt)
PUNT_OPCODE(Mov)
PUNT_OPCODE(LdMIStateAddr)
PUNT_OPCODE(IncRefCtx)
PUNT_OPCODE(DecRefThis)
PUNT_OPCODE(GenericRetDecRefs)
PUNT_OPCODE(DecRef)
PUNT_OPCODE(DecRefNZ)
PUNT_OPCODE(DefInlineFP)
PUNT_OPCODE(InlineReturn)
PUNT_OPCODE(ReDefSP)
PUNT_OPCODE(OODeclExists);
PUNT_OPCODE(VerifyParamCls)
PUNT_OPCODE(VerifyRetCls)
PUNT_OPCODE(ConcatCellCell)
PUNT_OPCODE(AKExists)
PUNT_OPCODE(ContEnter)
PUNT_OPCODE(ContPreNext)
PUNT_OPCODE(ContStartedCheck)
PUNT_OPCODE(ContValid)
PUNT_OPCODE(ContArIncKey)
PUNT_OPCODE(ContArUpdateIdx)
PUNT_OPCODE(LdContActRec)
PUNT_OPCODE(StContArRaw)
PUNT_OPCODE(LdContArValue)
PUNT_OPCODE(StContArValue)
PUNT_OPCODE(LdContArKey)
PUNT_OPCODE(StContArKey)
PUNT_OPCODE(StAsyncArRaw)
PUNT_OPCODE(StAsyncArResult)
PUNT_OPCODE(LdAsyncArParentChain)
PUNT_OPCODE(AFWHBlockOn)
PUNT_OPCODE(LdWHState)
PUNT_OPCODE(LdWHResult)
PUNT_OPCODE(LdAFWHActRec)
PUNT_OPCODE(LdResumableArObj)
PUNT_OPCODE(IterInit)
PUNT_OPCODE(IterInitK)
PUNT_OPCODE(IterNext)
PUNT_OPCODE(IterNextK)
PUNT_OPCODE(WIterInit)
PUNT_OPCODE(WIterInitK)
PUNT_OPCODE(WIterNext)
PUNT_OPCODE(WIterNextK)
PUNT_OPCODE(MIterInit)
PUNT_OPCODE(MIterInitK)
PUNT_OPCODE(MIterNext)
PUNT_OPCODE(MIterNextK)
PUNT_OPCODE(IterFree)
PUNT_OPCODE(MIterFree)
PUNT_OPCODE(DecodeCufIter)
PUNT_OPCODE(CIterFree)
PUNT_OPCODE(DefMIStateBase)
PUNT_OPCODE(BaseG)
PUNT_OPCODE(PropX)
PUNT_OPCODE(PropDX)
PUNT_OPCODE(PropDXStk)
PUNT_OPCODE(CGetProp)
PUNT_OPCODE(VGetProp)
PUNT_OPCODE(VGetPropStk)
PUNT_OPCODE(BindProp)
PUNT_OPCODE(BindPropStk)
PUNT_OPCODE(SetProp)
PUNT_OPCODE(SetPropStk)
PUNT_OPCODE(UnsetProp)
PUNT_OPCODE(SetOpProp)
PUNT_OPCODE(SetOpPropStk)
PUNT_OPCODE(IncDecProp)
PUNT_OPCODE(IncDecPropStk)
PUNT_OPCODE(EmptyProp)
PUNT_OPCODE(IssetProp)
PUNT_OPCODE(ElemX)
PUNT_OPCODE(ElemArray)
PUNT_OPCODE(ElemDX)
PUNT_OPCODE(ElemDXStk)
PUNT_OPCODE(ElemUX)
PUNT_OPCODE(ElemUXStk)
PUNT_OPCODE(ArrayGet)
PUNT_OPCODE(StringGet)
PUNT_OPCODE(MapGet)
PUNT_OPCODE(CGetElem)
PUNT_OPCODE(VGetElem)
PUNT_OPCODE(VGetElemStk)
PUNT_OPCODE(BindElem)
PUNT_OPCODE(BindElemStk)
PUNT_OPCODE(ArraySet)
PUNT_OPCODE(MapSet)
PUNT_OPCODE(ArraySetRef)
PUNT_OPCODE(SetElem)
PUNT_OPCODE(SetElemStk)
PUNT_OPCODE(SetWithRefElem)
PUNT_OPCODE(SetWithRefElemStk)
PUNT_OPCODE(UnsetElem)
PUNT_OPCODE(UnsetElemStk)
PUNT_OPCODE(SetOpElem)
PUNT_OPCODE(SetOpElemStk)
PUNT_OPCODE(IncDecElem)
PUNT_OPCODE(IncDecElemStk)
PUNT_OPCODE(SetNewElem)
PUNT_OPCODE(SetNewElemStk)
PUNT_OPCODE(SetNewElemArray)
PUNT_OPCODE(SetNewElemArrayStk)
PUNT_OPCODE(SetWithRefNewElem)
PUNT_OPCODE(SetWithRefNewElemStk)
PUNT_OPCODE(BindNewElem)
PUNT_OPCODE(BindNewElemStk)
PUNT_OPCODE(ArrayIsset)
PUNT_OPCODE(StringIsset)
PUNT_OPCODE(VectorIsset)
PUNT_OPCODE(PairIsset)
PUNT_OPCODE(MapIsset)
PUNT_OPCODE(IssetElem)
PUNT_OPCODE(EmptyElem)
PUNT_OPCODE(IncStat)
PUNT_OPCODE(RBTrace)
PUNT_OPCODE(IncTransCounter)
PUNT_OPCODE(IncProfCounter)
PUNT_OPCODE(DbgAssertType)
PUNT_OPCODE(AddIntO)
PUNT_OPCODE(SubIntO)
PUNT_OPCODE(MulIntO)
PUNT_OPCODE(EagerSyncVMRegs)
PUNT_OPCODE(ColIsEmpty)
PUNT_OPCODE(ColIsNEmpty)
PUNT_OPCODE(AllocPackedArray)
PUNT_OPCODE(InitPackedArray)
PUNT_OPCODE(InitPackedArrayLoop)

#undef PUNT_OPCODE

//////////////////////////////////////////////////////////////////////

// copy of ifThen in mc-generator-internal.h
template <class Then>
void ifThen(Vout& v, ConditionCode cc, Vreg sf, Then thenBlock) {
  auto then = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {done, then}};
  v = then;
  thenBlock(v);
  if (!v.closed()) v << jmp{done};
  v = done;
}

template <class Then>
void ifZero(Vout& v, unsigned bit, Vreg r, Then thenBlock) {
  auto then = v.makeBlock();
  auto done = v.makeBlock();
  v << tbcc{vixl::eq, bit, r, {done, then}};
  v = then;
  thenBlock(v);
  if (!v.closed()) v << jmp{done};
  v = done;
}

template <class T, class F>
Vreg condZero(Vout& v, Vreg r, Vreg dst, T t, F f) {
  using namespace x64;
  auto fblock = v.makeBlock();
  auto tblock = v.makeBlock();
  auto done = v.makeBlock();
  v << cbcc{vixl::eq, r, {fblock, tblock}};
  v = tblock;
  auto treg = t(v);
  v << phijmp{done, v.makeTuple(VregList{treg})};
  v = fblock;
  auto freg = f(v);
  v << phijmp{done, v.makeTuple(VregList{freg})};
  v = done;
  v << phidef{v.makeTuple(VregList{dst})};
  return dst;
}

// copy of ifThenElse from code-gen-x64.cpp
template <class Then, class Else>
void ifThenElse(Vout& v, ConditionCode cc, Vreg sf, Then thenBlock,
                Else elseBlock) {
  auto thenLabel = v.makeBlock();
  auto elseLabel = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {elseLabel, thenLabel}};
  v = thenLabel;
  thenBlock(v);
  if (!v.closed()) v << jmp{done};
  v = elseLabel;
  elseBlock(v);
  if (!v.closed()) v << jmp{done};
  v = done;
}

Vloc CodeGenerator::srcLoc(unsigned i) const {
  return m_slocs[i];
}

Vloc CodeGenerator::dstLoc(unsigned i) const {
  return m_dlocs[i];
}

ArgGroup CodeGenerator::argGroup() const {
  return ArgGroup(m_curInst, m_slocs);
}

Vlabel CodeGenerator::label(Block* b) {
  return m_state.labels[b];
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::recordHostCallSyncPoint(Vout& v, Vpoint p) {
  auto stackOff = m_curInst->marker().spOff();
  auto pcOff = m_curInst->marker().bcOff() - m_curInst->marker().func()->base();
  v << hcsync{Fixup{pcOff, stackOff}, p};
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgConjure(IRInstruction* inst) {
  always_assert(false);
}

void CodeGenerator::cgHalt(IRInstruction* inst) {
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgJmp(IRInstruction* inst) {
  auto& v = vmain();
  v << jmp{label(inst->taken())};
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  // maybe reuse emitAssertRefCount
  auto base = srcLoc(0).reg();
  auto& v = vmain();
  auto rCount = v.makeReg();
  v << loadl{base[FAST_REFCOUNT_OFFSET], rCount};
  ifZero(v, UncountedBitPos, rCount, [&](Vout& v) {
    auto const sf = v.makeReg();
    v << cmpli{RefCountMaxRealistic, rCount, sf};
    ifThen(v, CC_A, sf, [&](Vout& v) {
      v << brk{0};
    });
  });
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  auto loc = srcLoc(0);
  Type type = src->type();
  if (type.notCounted()) return;

  auto increfMaybeStatic = [&](Vout& v) {
    auto base = loc.reg(0);
    auto rCount = v.makeReg();
    v << loadl{base[FAST_REFCOUNT_OFFSET], rCount};
    if (!type.needsStaticBitCheck()) {
      auto count1 = v.makeReg();
      v << addli{1, rCount, count1, v.makeReg()};
      v << storel{count1, base[FAST_REFCOUNT_OFFSET]};
    } else {
      auto const sf = v.makeReg();
      v << cmpli{0, rCount, sf};
      static_assert(UncountedValue < 0 && StaticValue < 0, "");
      ifThen(v, CC_GE, sf, [&](Vout& v) {
        auto count1 = v.makeReg();
        v << addli{1, rCount, count1, v.makeReg()};
        v << storel{count1, base[FAST_REFCOUNT_OFFSET]};
      });
    }
  };

  auto& v = vmain();
  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic(v);
  } else {
    auto const sf = v.makeReg();
    v << cmpli{KindOfRefCountThreshold, loc.reg(1), sf};
    ifThen(v, CC_G, sf, [&](Vout& v) { increfMaybeStatic(v); });
  }
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  auto const src = srcLoc(0);
  auto const dst = dstLoc(0);
  auto& v = vmain();
  if (dst.reg(0) != InvalidReg && dst.reg(1) != InvalidReg) {
    v << copy2{src.reg(0), src.reg(1), dst.reg(0), dst.reg(1)};
  } else if (dst.reg(0) != InvalidReg) {
    v << copy{src.reg(0), dst.reg(0)};
  } else if (dst.reg(1) != InvalidReg) {
    v << copy{src.reg(1), dst.reg(1)};
  }
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::emitDecRefStaticType(Vout& v, Type type, Vreg data) {
  assert(type.isKnownDataType());
  auto done = v.makeBlock();
  auto count = v.makeReg();
  v << loadl{data[FAST_REFCOUNT_OFFSET], count};
  if (type.needsStaticBitCheck()) {
    auto next = v.makeBlock();
    v << tbcc{vixl::ne, UncountedBitPos, count, {next, done}};
    v = next;
  }
  auto count1 = v.makeReg();
  auto destruct = v.makeBlock();
  auto const sf = v.makeReg();
  v << subli{1, count, count1, sf};
  v << storel{count1, data[FAST_REFCOUNT_OFFSET]};
  v << jcc{CC_Z, sf, {done, destruct}};
  v = destruct;
  cgCallHelper(v,
               MCGenerator::getDtorCall(type.toDataType()),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().reg(data));
  v << jmp{done};
  v = done;
}

void CodeGenerator::emitDecRefDynamicType(Vout& v, Vreg base, int offset) {
  auto counted_type = v.makeBlock();
  auto counted_obj = v.makeBlock();
  auto destruct = v.makeBlock();
  auto done = v.makeBlock();
  auto type = v.makeReg();
  auto data = v.makeReg();
  auto count = v.makeReg();
  auto count1 = v.makeReg();

  // Check the type
  {
    v << loadzbl{base[offset + TVOFF(m_type)], type};
    auto const sf = v.makeReg();
    v << cmpli{KindOfRefCountThreshold, type, sf};
    v << jcc{CC_LE, sf, {counted_type, done}};
    v = counted_type;
  }

  // Type is refcounted. Load the refcount.
  v << load{base[offset + TVOFF(m_data)], data};
  v << loadl{data[FAST_REFCOUNT_OFFSET], count};

  // Is it static? Note that only the lower 32 bits of count are valid right
  // now, but tbcc is only looking at a single one of them, so this is OK.
  v << tbcc{vixl::ne, UncountedBitPos, count, {counted_obj, done}};
  v = counted_obj;

  {
    // Not static. Decrement and write back.
    auto const sf = v.makeReg();
    v << subli{1, count, count1, sf};
    v << storel{count1, data[FAST_REFCOUNT_OFFSET]};

    // Did it go to zero?
    v << jcc{CC_NZ, sf, {destruct, done}};
    v = destruct;
  }

  // Went to zero. Have to destruct.
  cgCallHelper(v,
               CppCall::direct(tv_release_generic),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(base, offset));
  v << jmp{done};
  v = done;
}

void CodeGenerator::emitDecRefMem(Vout& v, Type type, Vreg base, int offset) {
  if (type.needsReg()) {
    emitDecRefDynamicType(v, base, offset);
  } else if (type.maybeCounted()) {
    auto data = v.makeReg();
    v << load{base[offset + TVOFF(m_data)], data};
    emitDecRefStaticType(v, type, data);
  }
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  emitDecRefMem(vmain(), inst->typeParam(),
                srcLoc(0).reg(),
                cellsToBytes(inst->extra<DecRefStack>()->offset));
}

void CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  emitDecRefMem(vmain(), inst->typeParam(),
                srcLoc(0).reg(),
                localOffset(inst->extra<DecRefLoc>()->locId));
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  emitDecRefMem(vmain(), inst->typeParam(),
                srcLoc(0).reg(),
                inst->src(1)->intVal());
}

//////////////////////////////////////////////////////////////////////
// Arithmetic Instructions

void CodeGenerator::cgAddInt(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << addq{srcRegR, srcRegL, dstReg, v.makeReg()};
}

void CodeGenerator::cgSubInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << subq{srcRegR, srcRegL, dstReg, v.makeReg()};
}

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << imul{srcRegR, srcRegL, dstReg, v.makeReg()};
}

//////////////////////////////////////////////////////////////////////
// Bitwise Operators

void CodeGenerator::cgAndInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << andq{srcRegR, srcRegL, dstReg, v.makeReg()};
}

void CodeGenerator::cgOrInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << orq{srcRegR, srcRegL, dstReg, v.makeReg()};
}

void CodeGenerator::cgXorInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << xorq{srcRegR, srcRegL, dstReg, v.makeReg()};
}

void CodeGenerator::cgShl(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  // TODO: t3870154 add shift-by-immediate support to vixl
  vmain() << lslv{srcRegL, srcRegR, dstReg};
}

void CodeGenerator::cgShr(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  // TODO: t3870154 add shift-by-immediate support to vixl
  vmain() << asrv{srcRegL, srcRegR, dstReg};
}

//////////////////////////////////////////////////////////////////////
// Comparison Operations

void CodeGenerator::emitCompareIntAndSet(IRInstruction *inst,
                                         ConditionCode cc) {
  auto const sf = emitCompareInt(inst);
  auto dst = dstLoc(0).reg();
  vmain() << setcc{cc, sf, dst};
}

Vreg CodeGenerator::emitCompareInt(IRInstruction* inst) {
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmpq{srcRegR, srcRegL, sf}; // att-style
  return sf;
}

void CodeGenerator::cgLtInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_L);
}

void CodeGenerator::cgGtInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_G);
}


void CodeGenerator::cgGteInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_GE);
}

void CodeGenerator::cgLteInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_LE);
}


void CodeGenerator::cgEqInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_E);
}

void CodeGenerator::cgNeqInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst, CC_NE);
}

//////////////////////////////////////////////////////////////////////

static void shuffleArgs(Vout& v, ArgGroup& args, CppCall& call) {
  MovePlan moves;
  PhysReg::Map<ArgDesc*> argDescs;

  for (size_t i = 0; i < args.numGpArgs(); i++) {
    auto& arg = args.gpArg(i);
    auto kind = arg.kind();
    if (!(kind == ArgDesc::Kind::Reg  ||
          kind == ArgDesc::Kind::Addr ||
          kind == ArgDesc::Kind::TypeReg)) {
      continue;
    }
    auto srcReg = arg.srcReg();
    auto dstReg = arg.dstReg();
    if (srcReg != dstReg && srcReg.isPhys()) {
      moves[dstReg] = srcReg;
      argDescs[dstReg] = &arg;
    }
  }

  auto const howTo = doVregMoves(v.unit(), moves);
  for (auto& how : howTo) {
    auto src = how.m_src;
    auto dst = how.m_dst;
    if (how.m_kind == VMoveInfo::Kind::Move) {
      if (dst.isVirt()) {
        v << copy{src, dst};
      } else {
        auto* argDesc = argDescs[how.m_dst];
        if (argDesc) {
          auto kind = argDesc->kind();
          if (kind == ArgDesc::Kind::Addr) {
            v << lea{src[argDesc->disp().l()], dst};
          } else {
            if (argDesc->isZeroExtend()) {
              v << movzbl{src, dst};
            } else {
              v << copy{src, dst};
            }
          }
          if (kind != ArgDesc::Kind::TypeReg) {
            argDesc->markDone();
          }
        } else {
          v << copy{src, dst};
        }
      }
    } else {
      v << copy2{src, dst, dst, src};
    }
  }

  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    auto& arg = args.gpArg(i);
    if (arg.done()) continue;
    auto kind = arg.kind();
    auto src = arg.srcReg();
    auto dst = arg.dstReg();
    if (kind == ArgDesc::Kind::Imm) {
      v << ldimm{arg.imm().q(), dst};
    } else if (kind == ArgDesc::Kind::Reg) {
      if (arg.isZeroExtend()) {
        if (src.isVirt()) {
          v << movzbl{src, dst};
        } else {
          v << movzbl{dst, dst};
        }
      } else {
        if (src.isVirt()) {
          v << copy{src, dst};
        }
      }
    } else if (kind == ArgDesc::Kind::TypeReg) {
      if (kTypeShiftBits > 0) {
        if (src.isVirt()) {
          v << shlqi{kTypeShiftBits, src, dst, v.makeReg()};
        } else {
          v << shlqi{kTypeShiftBits, dst, dst, v.makeReg()};
        }
      } else {
        if (src.isVirt()) {
          v << copy{src, dst};
        }
      }
    } else if (kind == ArgDesc::Kind::Addr) {
      if (src.isVirt()) {
        v << addqi{arg.disp(), src, dst, v.makeReg()};
      } else {
        v << addqi{arg.disp(), dst, dst, v.makeReg()};
      }
    } else {
      not_implemented();
    }
  }
}

void CodeGenerator::cgCallNative(Vout& v, IRInstruction* inst) {
  using namespace NativeCalls;

  Opcode opc = inst->op();
  always_assert(CallMap::hasInfo(opc));

  auto const& info = CallMap::info(opc);
  ArgGroup argGroup = toArgGroup(info, m_slocs, inst);

  auto call = [&]() -> CppCall {
    switch (info.func.type) {
    case FuncType::Call:
      return CppCall(info.func.call);
    case FuncType::SSA:
      return CppCall::direct(
        reinterpret_cast<void(*)()>(inst->src(info.func.srcIdx)->tcaVal()));
    }
    not_reached();
  }();

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:  return kVoidDest;
      case DestType::TV:
      case DestType::SIMD:  return callDestTV(inst);
      case DestType::SSA:   return callDest(inst);
      case DestType::Dbl:   return callDestDbl(inst);
    }
    not_reached();
  }();

  cgCallHelper(v, call, dest, info.sync, argGroup);
}

void CodeGenerator::cgCallHelper(Vout& v,
                                 CppCall call,
                                 const CallDest& dstInfo,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  assert(m_curInst->isNative());

  auto dstReg0 = dstInfo.reg0;
  DEBUG_ONLY auto dstReg1 = dstInfo.reg1;

  RegSet argRegs;
  for (size_t i = 0; i < args.numGpArgs(); i++) {
    PhysReg r(argReg(i));
    args.gpArg(i).setDstReg(r);
    argRegs.add(r);
  }
  always_assert_flog(
    args.numStackArgs() == 0,
    "Stack arguments not yet supported on ARM: `{}'\n\n{}",
    *m_curInst, m_unit
  );
  shuffleArgs(v, args, call);

  auto syncPoint = emitCall(v, call, argRegs);
  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    recordHostCallSyncPoint(v, syncPoint);
  }

  auto* taken = m_curInst->taken();
  if (taken && taken->isCatch()) {
    auto& info = m_state.catches[taken];
    assert_not_implemented(args.numStackArgs() == 0);
    info.rspOffset = args.numStackArgs();
    auto next = v.makeBlock();
    v << hcunwind{syncPoint, {next, m_state.labels[taken]}};
    v = next;
  } else if (!m_curInst->is(Call, CallArray, ContEnter)) {
    v << hcnocatch{syncPoint};
  }

  switch (dstInfo.type) {
    case DestType::TV: not_implemented();
    case DestType::SIMD: not_implemented();
    case DestType::SSA:
      assert(dstReg1 == InvalidReg);
      v << copy{PhysReg(vixl::x0), dstReg0};
      break;
    case DestType::None:
      assert(dstReg0 == InvalidReg && dstReg1 == InvalidReg);
      break;
    case DestType::Dbl:
      assert(dstReg1 == InvalidReg);
      v << copy{PhysReg(vixl::d0), dstReg0};
      break;
  }
}

CallDest CodeGenerator::callDest(Vreg reg0) const {
  return { DestType::SSA, reg0 };
}

CallDest CodeGenerator::callDest(Vreg reg0, Vreg reg1) const {
  return { DestType::SSA, reg0, reg1 };
}

/*
 * XXX copypasta below, but has to be in the class because of srcLoc()
 * and dstLoc(). Changing that would make callsites real messy.
 */

CallDest CodeGenerator::callDest(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::SSA, loc.reg(0), loc.reg(1) };
}

CallDest CodeGenerator::callDestTV(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  if (loc.isFullSIMD()) {
    return { DestType::SIMD, loc.reg(0), InvalidReg };
  }
  return { DestType::TV, loc.reg(0), loc.reg(1) };
}

CallDest CodeGenerator::callDestDbl(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::Dbl, loc.reg(0), loc.reg(1) };
}

//////////////////////////////////////////////////////////////////////

static Vreg enregister(Vout& v, Vptr memRef) {
  auto r = v.makeReg();
  v << load{memRef, r};
  return r;
}

static Vreg enregister(Vout& v, Vreg r) {
  return r;
}

template<class Loc, class JmpFn>
void CodeGenerator::emitTypeTest(Vout& v, Type type, Vreg typeReg, Loc dataSrc,
                                 Vreg sf, JmpFn doJcc) {
  assert(!(type <= Type::Cls));
  assert(typeReg.isVirt() || typeReg.isGP()); // expected W-type, ie 32-bit

  if (type.equals(Type::Gen)) {
    return;
  }

  ConditionCode cc;
  if (type <= Type::Str) {
    // Note: ARM can actually do better here; it has a fused test-and-branch
    // instruction. The way this code is factored makes it difficult to use,
    // though; the jump instruction will be written by some other code.
    v << testli{KindOfStringBit, typeReg, sf};
    cc = CC_NE;
  } else if (type == Type::Null) {
    v << cmpli{KindOfNull, typeReg, sf};
    cc = CC_LE;
  } else if (type == Type::UncountedInit) {
    v << testli{KindOfUncountedInitBit, typeReg, sf};
    cc = CC_NE;
  } else if (type == Type::Uncounted) {
    v << cmpli{KindOfRefCountThreshold, typeReg, sf};
    cc = CC_LE;
  } else if (type == Type::Cell) {
    v << cmpli{KindOfRef, typeReg, sf};
    cc = CC_L;
  } else {
    assert(type.isKnownDataType());
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfResource));
    v << cmpli{dataType, typeReg, sf};
    cc = CC_E;
  }
  doJcc(cc);
  if (type < Type::Obj) {
    assert(type.getClass()->attrs() & AttrNoOverride);
    auto dataReg = enregister(v, dataSrc);
    auto vmclass = v.makeReg();
    emitLdLowPtr(v, vmclass, dataReg[ObjectData::getVMClassOffset()],
                 sizeof(LowClassPtr));
    emitCmpClass(v, sf, vmclass, type.getClass());
    doJcc(CC_E);
  } else if (type < Type::Res) {
    CG_PUNT(TypeTest-on-Resource);
  } else if (type <= Type::Arr && type.hasArrayKind()) {
    auto dataReg = enregister(v, dataSrc);
    auto kind = v.makeReg();
    v << loadzbl{dataReg[ArrayData::offsetofKind()], kind};
    v << cmpli{type.getArrayKind(), kind, sf};
    doJcc(CC_E);
  }
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  auto const rFP = srcLoc(0).reg();
  auto const baseOff = localOffset(inst->extra<GuardLoc>()->locId);
  auto& v = vmain();
  auto type = v.makeReg();
  v << loadzbl{rFP[baseOff + TVOFF(m_type)], type};
  auto const sf = v.makeReg();
  emitTypeTest(v, inst->typeParam(), type, rFP[baseOff + TVOFF(m_data)], sf,
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
      v << fallbackcc{ccNegate(cc), sf, destSK};
    });
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  auto const rSP = srcLoc(0).reg();
  auto const baseOff = cellsToBytes(inst->extra<GuardStk>()->offset);
  auto& v = vmain();
  auto type = v.makeReg();
  v << loadzbl{rSP[baseOff + TVOFF(m_type)], type};
  auto const sf = v.makeReg();
  emitTypeTest(v, inst->typeParam(), type, rSP[baseOff + TVOFF(m_data)], sf,
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
      v << fallbackcc{ccNegate(cc), sf, destSK};
    });
}

void CodeGenerator::cgCheckStk(IRInstruction* inst) {
  auto const rSP = srcLoc(0).reg();
  auto const baseOff = cellsToBytes(inst->extra<CheckStk>()->offset);
  auto& v = vmain();
  auto type = v.makeReg();
  v << loadzbl{rSP[baseOff + TVOFF(m_type)], type};
  auto const sf = v.makeReg();
  emitTypeTest(v, inst->typeParam(), type, rSP[baseOff + TVOFF(m_data)], sf,
    [&] (ConditionCode cc) {
      auto next = v.makeBlock();
      v << jcc{ccNegate(cc), sf, {next, label(inst->taken())}};
      v = next;
    }
  );
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->src(0);
  Type   srcType   = src->type();
  auto const rVal  = srcLoc(0).reg(0);
  auto const rType = srcLoc(0).reg(1);
  auto& v = vmain();

  auto doMov = [&] {
    auto const valDst = dstLoc(0).reg(0);
    auto const typeDst = dstLoc(0).reg(1);
    v << copy{rVal, valDst};
    if (typeDst.isValid()) {
      if (rType.isValid()) {
        v << copy{rType, typeDst};
      } else {
        v << ldimm{srcType.toDataType(), typeDst};
      }
    }
  };

  Type typeParam = inst->typeParam();
  if (src->isA(typeParam) ||
      // Boxed types are checked lazily, so there's nothing to be done here.
      (srcType.isBoxed() && typeParam.isBoxed())) {
    doMov();
    return;
  }
  if (srcType.not(typeParam)) {
    v << jmp{label(inst->taken())};
    return;
  }

  if (rType.isValid()) {
    auto const sf = v.makeReg();
    emitTypeTest(v, typeParam, rType, rVal, sf,
      [&] (ConditionCode cc) {
        auto next = v.makeBlock();
        v << jcc{ccNegate(cc), sf, {next, label(inst->taken())}};
        v = next;
      });
  } else if (typeParam <= Type::Uncounted &&
             ((srcType == Type::Str && typeParam.maybe(Type::StaticStr)) ||
              (srcType == Type::Arr && typeParam.maybe(Type::StaticArr)))) {
    // We carry Str and Arr operands around without a type register, even
    // though they're union types. The static and non-static subtypes are
    // distinguised by the refcount field.
    assert(rVal.isValid());
    auto count = v.makeReg();
    auto next = v.makeBlock();
    v << loadl{rVal[FAST_REFCOUNT_OFFSET], count};
    v << tbcc{vixl::eq, UncountedBitPos, count, {next, label(inst->taken())}};
    v = next;
  } else {
    always_assert_log( false, [&] {
      return folly::format("Bad src: {} and dst: {} types in '{}'",
                           srcType, typeParam, *inst).str();
    });
  }
  doMov();
}

void CodeGenerator::cgSideExitGuardStk(IRInstruction* inst) {
  auto const sp = srcLoc(0).reg();
  auto const extra = inst->extra<SideExitGuardStk>();
  auto& v = vmain();

  auto type = v.makeReg();
  v << loadzbl{sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_type)], type};
  auto const sf = v.makeReg();
  emitTypeTest(v, inst->typeParam(), type,
    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_data)], sf,
    [&] (ConditionCode cc) {
      auto const sk = SrcKey(curFunc(), extra->taken, resumed());
      v << bindexit{ccNegate(cc), sf, sk};
    }
  );
}

template <class JmpFn>
void CodeGenerator::emitReffinessTest(IRInstruction* inst, Vreg sf,
                                      JmpFn doJcc) {
  assert(inst->numSrcs() == 5);

  DEBUG_ONLY SSATmp* nParamsTmp = inst->src(1);
  DEBUG_ONLY SSATmp* firstBitNumTmp = inst->src(2);
  DEBUG_ONLY SSATmp* mask64Tmp  = inst->src(3);
  DEBUG_ONLY SSATmp* vals64Tmp  = inst->src(4);

  auto funcPtrLoc = srcLoc(0);
  auto nParamsLoc = srcLoc(1);
  auto mask64Loc = srcLoc(3);
  auto vals64Loc = srcLoc(4);

  // Get values in place
  auto funcPtrReg = funcPtrLoc.reg();
  assert(funcPtrReg.isValid());

  auto nParamsReg = nParamsLoc.reg();

  auto firstBitNum = static_cast<uint32_t>(firstBitNumTmp->intVal());
  auto mask64Reg = mask64Loc.reg();
  uint64_t mask64 = mask64Tmp->intVal();
  assert(mask64);

  auto vals64Reg = vals64Loc.reg();
  uint64_t vals64 = vals64Tmp->intVal();
  assert((vals64 & mask64) == vals64);

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

    // Don't need the bits pointer after this point
    auto bits = v.makeReg();
    // Load the bits
    v << load{bitsPtrReg[bitsOff], bits};

    // Mask the bits. There are restrictions on what can be encoded as an
    // immediate in ARM's logical instructions, and if they're not met,
    // the assembler will compensate using ip0 or ip1 as tmps.
    auto masked = v.makeReg();
    v << andq{mask64Reg, bits, masked, v.makeReg()};

    // Now do the compare. There are also restrictions on immediates in
    // arithmetic instructions (of which Cmp is one; it's just a subtract that
    // sets flags), so same deal as with the mask immediate above.
    v << cmpq{vals64Reg, masked, sf};
    doJcc(cond, sf);
  };

  auto& v = vmain();
  if (firstBitNum == 0) {
    assert(nParamsTmp->isConst());
    // This is the first 64 bits. No need to check nParams.
    thenBody(v);
  } else {
    // Check number of args...
    auto const sf2 = v.makeReg();
    v << cmpq{v.cns(firstBitNum), nParamsReg, sf2};

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nParams, then either all params
      // are refs, or all params are non-refs, so if vals64
      // isn't 0 and isnt mask64, there's no possibility of
      // a match
      doJcc(CC_LE, sf2);
      thenBody(v);
    } else {
      ifThenElse(v, CC_G, sf2, thenBody, /* else */ [&](Vout& v) {
        //   If not special builtin...
        static_assert(sizeof(HPHP::Attr) == 4, "");
        auto attr = v.makeReg();
        v << loadl{funcPtrReg[Func::attrsOff()], attr};
        auto const sf = v.makeReg();
        v << testli{AttrVariadicByRef, attr, sf};
        doJcc(vals64 ? CC_Z : CC_NZ, sf);
      });
    }
  }
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitReffinessTest(inst, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      auto const destSK = SrcKey(curFunc(), inst->marker().bcOff(), resumed());
      v << fallbackcc{cc, sfTaken, destSK};
    });
}

void CodeGenerator::cgCheckRefs(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitReffinessTest(inst, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      auto next = v.makeBlock();
      v << jcc{cc, sfTaken, {next, label(inst->taken())}};
      v = next;
    });
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgSyncABIRegs(IRInstruction* inst) {
  auto& v = vmain();
  v << copy{srcLoc(0).reg(), PhysReg(rVmFp)};
  v << copy{srcLoc(1).reg(), PhysReg(rVmSp)};
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  auto to = SrcKey(curFunc(), inst->extra<ReqBindJmp>()->offset, resumed());
  vmain() << bindjmp{to};
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  assert(m_unit.bcOff() == inst->marker().bcOff());
  auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
  auto& v = vmain();
  v << fallback{destSK};
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func     = inst->src(1);
  auto const objOrCls = inst->src(2);
  auto const invName  = inst->extra<SpillFrame>()->invName;
  auto const nArgs    = inst->extra<SpillFrame>()->numArgs;

  auto spReg = srcLoc(0).reg();
  auto funcLoc = srcLoc(1);
  auto objClsReg = srcLoc(2).reg();
  ptrdiff_t spOff = -kNumActRecCells * sizeof(Cell);
  auto& v = vmain();

  v << storel{v.cns(nArgs), spReg[spOff + AROFF(m_numArgsAndFlags)]};

  // Magic-call name.
  if (invName) {
    auto bits = reinterpret_cast<uintptr_t>(invName) | ActRec::kInvNameBit;
    v << store{v.cns(bits), spReg[spOff + AROFF(m_invName)]};
  } else {
    v << store{PhysReg(vixl::xzr), spReg[spOff + AROFF(m_invName)]};
  }

  // Func and this/class are slightly tricky. The func may be a tuple of a Func*
  // and context.

  if (objOrCls->isA(Type::Cls)) {
    if (objOrCls->isConst()) {
      v << store{v.cns(objOrCls->rawVal() | 1), spReg[spOff + AROFF(m_this)]};
    } else {
      auto ctx = v.makeReg();
      v << orqi{1, objClsReg, ctx, v.makeReg()};
      v << store{ctx, spReg[spOff + AROFF(m_this)]};
    }
  } else if (objOrCls->isA(Type::Obj) || objOrCls->isA(Type::Ctx)) {
    v << store{objClsReg, spReg[spOff + AROFF(m_this)]};
  } else {
    assert(objOrCls->isA(Type::Nullptr));
    v << store{PhysReg(vixl::xzr), spReg[spOff + AROFF(m_this)]};
  }

  // Now set func, and possibly this/cls
  if (!func->isA(Type::Nullptr)) {
    auto func = funcLoc.reg(0);
    v << store{func, spReg[spOff + AROFF(m_func)]};
  }

  // Adjust stack pointer
  v << addqi{safe_cast<int32_t>(spOff), spReg, dstLoc(0).reg(), v.makeReg()};
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const func           = inst->extra<CallBuiltinData>()->callee;
  auto const numArgs        = func->numParams();
  auto const funcReturnType = func->returnType();
  int returnOffset          = MISOFF(tvBuiltinReturn);
  auto& v = vmain();

  if (FixupMap::eagerRecord(func)) {
    // Save VM registers
    PhysReg vmfp(rVmFp), vmsp(rVmSp), rds(rVmTl);
    auto const* pc = curFunc()->unit()->entry() + m_curInst->marker().bcOff();
    v << store{vmfp, rds[RDS::kVmfpOff]};
    v << store{vmsp, rds[RDS::kVmspOff]};
    v << store{v.cns(pc), rds[RDS::kVmpcOff]};
  }

  // The stack pointer currently points to the MInstrState we need to use.
  PhysReg sp(vixl::sp); // C++ sp, not vmsp
  auto mis = v.makeReg();
  v << copy{sp, mis};//XXX why do this copy?

  auto callArgs = argGroup();
  if (isCppByRef(funcReturnType)) {
    if (isSmartPtrRef(funcReturnType)) {
      // first arg is pointer to storage for the return value
      returnOffset += TVOFF(m_data);
    }
    callArgs.addr(mis, returnOffset);
  }

  auto srcNum = uint32_t{0};
  if (func->isMethod()) {
    callArgs.ssa(srcNum);
    ++srcNum;
  }
  for (auto i = uint32_t{0}; i < numArgs; ++i, ++srcNum) {
    auto const& pi = func->params()[i];
    if (TVOFF(m_data) && isSmartPtrRef(pi.builtinType)) {
      callArgs.addr(srcLoc(srcNum).reg(), TVOFF(m_data));
    } else {
      callArgs.ssa(srcNum);
    }
  }

  auto dst = dstLoc(0).reg(0);
  auto dstType = dstLoc(0).reg(1);

  if (callArgs.numStackArgs() != 0) {
    CG_PUNT(cgCallBuiltin-StackArgs);
  }
  cgCallHelper(v,
               CppCall::direct(func->nativeFuncPtr()),
               isCppByRef(funcReturnType) ? kVoidDest : callDest(dst),
               SyncOptions::kSyncPoint,
               callArgs);

  auto returnType = inst->typeParam();
  if (!dst.isValid() || returnType.isSimpleType()) {
    return;
  }

  mis = sp;
  if (returnType.isReferenceType()) {
    // this should use some kind of cmov
    assert(isCppByRef(funcReturnType) && isSmartPtrRef(funcReturnType));
    v << load{mis[returnOffset + TVOFF(m_data)], dst};
    condZero(v, dst, dstType, [&](Vout& v) {
      return v.cns(KindOfNull);
    }, [&](Vout& v) {
      return v.cns(returnType.toDataType());
    });
    return;
  }

  if (returnType <= Type::Cell || returnType <= Type::BoxedCell) {
    // this should use some kind of cmov
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    assert(isCppByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    auto tmp_dst_type = v.makeReg();
    v << load{mis[returnOffset + TVOFF(m_data)], dst};
    v << loadzbl{mis[returnOffset + TVOFF(m_type)], tmp_dst_type};
    condZero(v, tmp_dst_type, dstType, [&](Vout& v) {
      return v.cns(KindOfNull);
    }, [&](Vout& v) {
      return tmp_dst_type;
    });
    return;
  }

  always_assert(false);
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  auto const extra = inst->extra<Call>();
  auto const rSP   = srcLoc(0).reg();
  auto const rFP   = srcLoc(1).reg();
  auto const ar = extra->numParams * sizeof(TypedValue);
  auto const srcKey = m_curInst->marker().sk();
  auto& v = vmain();
  v << store{rFP, rSP[ar + AROFF(m_sfp)]};
  v << storel{v.cns(extra->after), rSP[ar + AROFF(m_soff)]};
  if (isNativeImplCall(extra->callee, extra->numParams)) {
    // emitCallNativeImpl will adjust rVmSp
    assert(dstLoc(0).reg() == PhysReg(rVmSp));
    emitCallNativeImpl(v, vcold(), srcKey, extra->callee, extra->numParams);
  } else {
    v << bindcall{srcKey, extra->callee, extra->numParams};
  }
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  UNUSED auto const& info = m_state.catches[inst->block()];
  assert(info.rspOffset == 0); // stack args not supported yet
}

static void unwindResumeHelper() {
  // We don't have this sorted out for native mode yet
  always_assert(RuntimeOption::EvalSimulateARM);

  tl_regState = VMRegState::CLEAN;
  g_context->m_activeSims.back()->resume_last_exception();
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  emitCall(vmain(), CppCall::direct(unwindResumeHelper), RegSet{});
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::emitLoadTypedValue(Vout& v, Vloc dst, Vreg base,
                                       ptrdiff_t offset, Block* label) {
  if (label) not_implemented();
  if (dst.isFullSIMD()) not_implemented();

  auto valueDst = dst.reg(0);
  auto typeDst  = dst.reg(1);

  // Avoid clobbering the base reg if we'll need it later
  if (base == typeDst && valueDst.isValid()) {
    auto tmp = v.makeReg();
    v << copy{base, tmp};
    base = tmp;
  }

  if (typeDst.isValid()) {
    v << loadzbl{base[offset + TVOFF(m_type)], typeDst};
  }

  if (valueDst.isValid()) {
    v << load{base[offset + TVOFF(m_data)], valueDst};
  }
}

void CodeGenerator::emitStoreTypedValue(Vout& v, Vreg base, ptrdiff_t offset,
                                        Vloc src) {
  assert(src.numWords() == 2);
  auto reg0 = src.reg(0);
  auto reg1 = src.reg(1);
  v << store{reg0, base[offset + TVOFF(m_data)]};
  v << storeb{reg1, base[offset + TVOFF(m_type)]};
}

void CodeGenerator::emitLoad(Vout& v, Type type, Vloc dst, Vreg base,
                             ptrdiff_t offset, Block* label /* = nullptr */) {
  if (type.needsReg()) {
    return emitLoadTypedValue(v, dst, base, offset, label);
  }
  if (label) {
    not_implemented();
  }
  auto data = dst.reg();
  v << load{base[offset + TVOFF(m_data)], data};
}

void CodeGenerator::emitStore(Vout& v, Vreg base, ptrdiff_t offset,
                              SSATmp* src, Vloc srcLoc,
                              bool genStoreType /* = true */) {
  auto type = src->type();
  if (type.needsReg()) {
    return emitStoreTypedValue(v, base, offset, srcLoc);
  }
  if (genStoreType) {
    auto dt = type.toDataType();
    v << storeb{v.cns(dt), base[offset + TVOFF(m_type)]};
  }
  if (type <= Type::Null) {
    return;
  }

  auto data = srcLoc.reg();
  if (src->isA(Type::Bool)) {
    auto extended = v.makeReg();
    v << movzbl{data, extended};
    data = extended;
  }
  v << store{data, base[offset + TVOFF(m_data)]};
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  auto base = srcLoc(0).reg();
  auto offset = localOffset(inst->extra<LdLoc>()->locId);
  emitLoad(vmain(), inst->dst()->type(), dstLoc(0), base, offset);
}

void CodeGenerator::cgStLocWork(IRInstruction* inst) {
  auto base = srcLoc(0).reg();
  auto offset = localOffset(inst->extra<LocalId>()->locId);
  emitStore(vmain(), base, offset, inst->src(1), srcLoc(1),
            true /* store type */);
}

void CodeGenerator::cgStLoc(IRInstruction* inst) { cgStLocWork(inst); }
void CodeGenerator::cgStGbl(IRInstruction* inst) { cgStLocWork(inst); }

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->taken() == nullptr);
  auto src = srcLoc(0).reg();
  auto offset = cellsToBytes(inst->extra<LdStack>()->offset);
  emitLoad(vmain(), inst->dst()->type(), dstLoc(0), src, offset);
}

void CodeGenerator::emitLdRaw(IRInstruction* inst, size_t extraOff) {
  auto dest    = dstLoc(0).reg();
  auto offset  = inst->extra<RawMemData>()->info().offset;
  auto ptr     = srcLoc(0).reg()[offset + extraOff];
  auto& v = vmain();
  switch (inst->extra<RawMemData>()->info().size) {
    case sz::byte:  v << loadzbl{ptr, dest}; break;
    case sz::dword: v << loadl{ptr, dest}; break;
    case sz::qword: v << load{ptr, dest}; break;
    default:        not_implemented();
  }
}

void CodeGenerator::cgLdRaw(IRInstruction* inst) {
  emitLdRaw(inst, 0);
}

void CodeGenerator::cgLdContArRaw(IRInstruction* inst) {
  emitLdRaw(inst, -c_Generator::arOff());
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  auto dst     = dstLoc(0).reg();
  auto base    = srcLoc(0).reg();
  auto offset  = inst->src(1)->intVal();
  vmain() << load{base[offset + AROFF(m_func)], dst};
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  auto dst = dstLoc(0).reg();
  auto const name = inst->extra<LdFuncCachedData>()->name;
  auto const ch = NamedEntity::get(name)->getFuncHandle();
  PhysReg rds(rVmTl);
  auto& v = vmain();

  auto dst1 = v.makeReg();
  v << load{rds[ch], dst1};
  condZero(v, dst1, dst, [&](Vout& v) {
    auto dst2 = v.makeReg();
    const Func* (*const func)(const StringData*) = lookupUnknownFunc;
    cgCallHelper(v, CppCall::direct(func),
      callDest(dst2),
      SyncOptions::kSyncPoint,
      argGroup().immPtr(inst->extra<LdFuncCached>()->name));
    return dst2;
  }, [&](Vout& v) {
    return dst1;
  });
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto const dst     = dstLoc(0).reg();
  auto const base    = srcLoc(0).reg();
  auto const offset  = cellsToBytes(inst->extra<LdStackAddr>()->offset);
  vmain() << lea{base[offset], dst};
}

void CodeGenerator::cgSpillStack(IRInstruction* inst) {
  // TODO(2966414): so much of this logic could be shared. The opcode itself
  // should probably be broken up.
  auto const spDeficit    = inst->src(1)->intVal();
  auto const spillVals    = inst->srcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const dst          = dstLoc(0).reg();
  auto const sp           = srcLoc(0).reg();
  auto const spillCells   = spillValueCells(inst);
  auto& v = vmain();
  ptrdiff_t adjustment = (spDeficit - spillCells) * sizeof(Cell);
  for (uint32_t i = 0; i < numSpillSrcs; ++i) {
    const ptrdiff_t offset = i * sizeof(Cell) + adjustment;
    emitStore(v, sp, offset, spillVals[i], srcLoc(i + 2));
  }
  v << addqi{safe_cast<int32_t>(adjustment), sp, dst, v.makeReg()};
}

void CodeGenerator::cgInterpOneCommon(IRInstruction* inst) {
  auto pcOff = inst->extra<InterpOneData>()->bcOff;

  auto opc = *(curFunc()->unit()->at(pcOff));
  auto* interpOneHelper = interpOneEntryPoints[opc];

  cgCallHelper(vmain(),
               CppCall::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().ssa(1/*fp*/).ssa(0/*sp*/).imm(pcOff));
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  cgInterpOneCommon(inst);
  auto const& extra = *inst->extra<InterpOne>();
  auto newSp = dstLoc(0).reg();
  auto spAdjustBytes = cellsToBytes(extra.cellsPopped - extra.cellsPushed);
  auto& v = vmain();
  v << addqi{spAdjustBytes, newSp, newSp, v.makeReg()};
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);
  auto& v = vmain();
  PhysReg rds(rVmTl), fp(rVmFp), sp(rVmSp);
  v << load{rds[RDS::kVmfpOff], fp};
  v << load{rds[RDS::kVmspOff], sp};
  emitServiceReq(v, SRFlags::Persist, REQ_RESUME, {});
}

void CodeGenerator::cgLdClsName(IRInstruction* inst) {
  auto const dst = dstLoc(0).reg();
  auto const src = srcLoc(0).reg();
  auto& v = vmain();
  auto preclass = v.makeReg();
  v << load{src[Class::preClassOff()], preclass};
  v << load{preclass[PreClass::nameOffset()], dst};
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgCountArrayFast(IRInstruction* inst) {
  auto const array = srcLoc(0).reg();
  auto const size = dstLoc(0).reg();
  vmain() << loadl{array[ArrayData::offsetofSize()], size};
}

void CodeGenerator::cgCountCollection(IRInstruction* inst) {
  auto const collection = srcLoc(0).reg();
  auto const size = dstLoc(0).reg();
  vmain() << loadl{collection[FAST_COLLECTION_SIZE_OFFSET], size};
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgInst(IRInstruction* inst) {
  assert(!m_curInst && m_slocs.empty() && m_dlocs.empty());
  m_curInst = inst;
  SCOPE_EXIT {
    m_curInst = nullptr;
    m_slocs.clear();
    m_dlocs.clear();
  };
  for (auto s : inst->srcs()) {
    m_slocs.push_back(m_state.locs[s]);
    assert(m_slocs.back().reg(0).isValid());
  }
  for (auto& d : inst->dsts()) {
    m_dlocs.push_back(m_state.locs[d]);
    assert(m_dlocs.back().reg(0).isValid());
  }

  switch (inst->op()) {
#define O(name, dsts, srcs, flags)                                \
  case name: FTRACE(7, "cg" #name "\n");                          \
             cg ## name (inst);                                   \
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
      v << brk{0}; // or end?
    }
  }
}

}}}
