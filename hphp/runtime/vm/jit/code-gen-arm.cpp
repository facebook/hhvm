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

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/back-end-arm.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stack-offsets-defs.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include <folly/Optional.h>

#include <vector>

namespace HPHP { namespace jit { namespace arm {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

#define NOOP_OPCODE(name) void CodeGenerator::cg##name(IRInstruction*) {}

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(Nop)
NOOP_OPCODE(ExitPlaceholder)
NOOP_OPCODE(DefLabel)
NOOP_OPCODE(EndGuards)
NOOP_OPCODE(HintLocInner)
NOOP_OPCODE(HintStkInner)
NOOP_OPCODE(DbgTraceCall)

// When implemented this shouldn't be a nop, but there's no reason to make us
// punt on everything until then.
NOOP_OPCODE(CountBytecode)

#undef NOOP_OPCODE

//////////////////////////////////////////////////////////////////////

#define CALL_OPCODE(name) \
  void CodeGenerator::cg##name(IRInstruction* i) { \
    cgCallNative(vmain(), i); \
  }

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(Box)
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
CALL_OPCODE(ConvCellToArr);

CALL_OPCODE(ConvStrToBool);
CALL_OPCODE(ConvCellToBool);

CALL_OPCODE(ConvArrToDbl);
CALL_OPCODE(ConvObjToDbl);
CALL_OPCODE(ConvStrToDbl);
CALL_OPCODE(ConvCellToDbl);

CALL_OPCODE(ConvArrToInt);
CALL_OPCODE(ConvObjToInt);
CALL_OPCODE(ConvStrToInt);
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

CALL_OPCODE(CreateCont)
CALL_OPCODE(CreateAFWH)
CALL_OPCODE(CreateAFWHNoVV)
CALL_OPCODE(CreateSSWH)
CALL_OPCODE(AFWHPrepareChild)
CALL_OPCODE(ABCUnblock)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(AllocPackedArray)
CALL_OPCODE(Clone)
CALL_OPCODE(AllocObj)
CALL_OPCODE(InitProps)
CALL_OPCODE(InitSProps)
CALL_OPCODE(RegisterLiveObj)
CALL_OPCODE(LdClsCtor)
CALL_OPCODE(LookupClsMethod)
CALL_OPCODE(LookupClsRDSHandle)
CALL_OPCODE(LdArrFuncCtx)
CALL_OPCODE(LdArrFPushCuf)
CALL_OPCODE(LdStrFPushCuf)
CALL_OPCODE(PrintStr)
CALL_OPCODE(PrintInt)
CALL_OPCODE(PrintBool)
CALL_OPCODE(DbgAssertPtr)
CALL_OPCODE(LdSwitchDblIndex)
CALL_OPCODE(LdSwitchStrIndex)
CALL_OPCODE(LdSwitchObjIndex)
CALL_OPCODE(VerifyParamCallable)
CALL_OPCODE(VerifyParamFail)
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
CALL_OPCODE(ClosureStaticLocInit)
CALL_OPCODE(GenericIdx)
CALL_OPCODE(LdClsPropAddrOrNull)
CALL_OPCODE(LdClsPropAddrOrRaise)
CALL_OPCODE(LdGblAddrDef)

// Vector instruction helpers
CALL_OPCODE(StringGet)
CALL_OPCODE(BindElem)
CALL_OPCODE(SetWithRefElem)
CALL_OPCODE(SetWithRefNewElem)
CALL_OPCODE(SetOpElem)
CALL_OPCODE(IncDecElem)
CALL_OPCODE(SetNewElem)
CALL_OPCODE(SetNewElemArray)
CALL_OPCODE(BindNewElem)
CALL_OPCODE(VectorIsset)
CALL_OPCODE(PairIsset)
CALL_OPCODE(ThrowOutOfBounds)

CALL_OPCODE(InstanceOfIface)
CALL_OPCODE(InterfaceSupportsArr)
CALL_OPCODE(InterfaceSupportsStr)
CALL_OPCODE(InterfaceSupportsInt)
CALL_OPCODE(InterfaceSupportsDbl)

CALL_OPCODE(ZeroErrorLevel)
CALL_OPCODE(RestoreErrorLevel)

CALL_OPCODE(Count)

CALL_OPCODE(SuspendHookE)
CALL_OPCODE(SuspendHookR)
CALL_OPCODE(ReturnHook)

CALL_OPCODE(OODeclExists)

CALL_OPCODE(GetMemoKey)


//////////////////////////////////////////////////////////////////////

#define DELEGATE_OPCODE(name) \
  void CodeGenerator::cg##name(IRInstruction* inst) { \
    m_xcg.cg##name(inst); \
  }

DELEGATE_OPCODE(DefSP)
DELEGATE_OPCODE(CheckNullptr)
DELEGATE_OPCODE(CheckNonNull)
DELEGATE_OPCODE(AssertNonNull)
DELEGATE_OPCODE(AssertStk)
DELEGATE_OPCODE(AssertType)
DELEGATE_OPCODE(LdARFuncPtr)
DELEGATE_OPCODE(LdARNumParams)

DELEGATE_OPCODE(CheckStk)
DELEGATE_OPCODE(CheckType)

DELEGATE_OPCODE(AddInt)
DELEGATE_OPCODE(SubInt)
DELEGATE_OPCODE(AddIntO)
DELEGATE_OPCODE(SubIntO)
DELEGATE_OPCODE(AndInt)
DELEGATE_OPCODE(OrInt)
DELEGATE_OPCODE(XorInt)

DELEGATE_OPCODE(LtInt)
DELEGATE_OPCODE(GtInt)
DELEGATE_OPCODE(LteInt)
DELEGATE_OPCODE(GteInt)
DELEGATE_OPCODE(EqInt)
DELEGATE_OPCODE(NeqInt)

DELEGATE_OPCODE(ConvBoolToInt)
DELEGATE_OPCODE(NewCol)
DELEGATE_OPCODE(NewColFromArray)

/////////////////////////////////////////////////////////////////////
void cgPunt(const char* file, int line, const char* func, uint32_t bcOff,
            const Func* vmFunc, bool resumed, TransID profTransId) {
  FTRACE(1, "punting: {}\n", func);
  throw FailedCodeGen(file, line, func, bcOff, vmFunc, resumed, profTransId);
}

#define PUNT_OPCODE(name)                                               \
  void CodeGenerator::cg##name(IRInstruction* inst) {                   \
    cgPunt(__FILE__, __LINE__, #name, m_curInst->marker().bcOff(),      \
           curFunc(), resumed(), m_curInst->marker().profTransID());    \
  }

#define CG_PUNT(instr)                                              \
    cgPunt(__FILE__, __LINE__, #instr, m_curInst->marker().bcOff(), \
           curFunc(), resumed(), m_curInst->marker().profTransID())

//////////////////////////////////////////////////////////////////////
PUNT_OPCODE(ArrayIdx)
PUNT_OPCODE(MapIdx)
PUNT_OPCODE(CountArray)
PUNT_OPCODE(LdColArray)

PUNT_OPCODE(DbgAssertRefCount)
PUNT_OPCODE(DbgTrashStk)
PUNT_OPCODE(DbgTrashFrame)
PUNT_OPCODE(DbgTrashMem)
PUNT_OPCODE(ConvArrToBool)
PUNT_OPCODE(ConvDblToBool)
PUNT_OPCODE(ConvIntToBool)
PUNT_OPCODE(ConvObjToBool)
PUNT_OPCODE(ConvBoolToDbl)
PUNT_OPCODE(ConvIntToDbl)
PUNT_OPCODE(SpillFrame)
PUNT_OPCODE(Call)

PUNT_OPCODE(ConvDblToInt)

PUNT_OPCODE(ConvBoolToStr)

PUNT_OPCODE(ProfilePackedArray)
PUNT_OPCODE(ProfileStructArray)
PUNT_OPCODE(ProfileObjClass)
PUNT_OPCODE(CheckTypeMem)
PUNT_OPCODE(CheckLoc)
PUNT_OPCODE(CastStk)
PUNT_OPCODE(CastMem)
PUNT_OPCODE(CoerceStk)
PUNT_OPCODE(CoerceMem)
PUNT_OPCODE(UnwindCheckSideExit)
PUNT_OPCODE(LdUnwinderValue)
PUNT_OPCODE(AddDbl)
PUNT_OPCODE(SubDbl)
PUNT_OPCODE(MulDbl)
PUNT_OPCODE(DivDbl)
PUNT_OPCODE(Mod)
PUNT_OPCODE(Sqrt)
PUNT_OPCODE(AbsDbl)
PUNT_OPCODE(XorBool)
PUNT_OPCODE(ExtendsClass)
PUNT_OPCODE(ClsNeq)
PUNT_OPCODE(IsWaitHandle)
PUNT_OPCODE(InstanceOf)
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
PUNT_OPCODE(GtStr)
PUNT_OPCODE(GteStr)
PUNT_OPCODE(LtStr)
PUNT_OPCODE(LteStr)
PUNT_OPCODE(EqStr)
PUNT_OPCODE(NeqStr)
PUNT_OPCODE(SameStr)
PUNT_OPCODE(NSameStr)
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
PUNT_OPCODE(JmpZero)
PUNT_OPCODE(JmpNZero)
PUNT_OPCODE(JmpSSwitchDest)
PUNT_OPCODE(CheckSurpriseFlags)
PUNT_OPCODE(ReleaseVVAndSkip)
PUNT_OPCODE(CheckInit)
PUNT_OPCODE(CheckInitMem)
PUNT_OPCODE(CheckCold)
PUNT_OPCODE(CheckRange)
PUNT_OPCODE(LdVectorSize)
PUNT_OPCODE(CheckPackedArrayBounds)
PUNT_OPCODE(VectorHasImmCopy)
PUNT_OPCODE(VectorDoCow)
PUNT_OPCODE(UnboxPtr)
PUNT_OPCODE(BoxPtr)
PUNT_OPCODE(LdVectorBase)
PUNT_OPCODE(LdPairBase)
PUNT_OPCODE(LdLocAddr)
PUNT_OPCODE(LdMem)
PUNT_OPCODE(LdContField)
PUNT_OPCODE(LdElem)
PUNT_OPCODE(LdPackedArrayElemAddr)
PUNT_OPCODE(CheckRefInner)
PUNT_OPCODE(LdStructArrayElem)
PUNT_OPCODE(LdRef)
PUNT_OPCODE(LdLocPseudoMain)
PUNT_OPCODE(ConvClsToCctx)
PUNT_OPCODE(CheckCtxThis)
PUNT_OPCODE(CastCtxThis)
PUNT_OPCODE(LdCtx)
PUNT_OPCODE(LdCctx)
PUNT_OPCODE(LdClosure)
PUNT_OPCODE(LdCls)
PUNT_OPCODE(LdClsCached)
PUNT_OPCODE(LdClsCachedSafe)
PUNT_OPCODE(LdClsCtx)
PUNT_OPCODE(LdClsCctx)
PUNT_OPCODE(LdRDSAddr)
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
PUNT_OPCODE(LdIfaceMethod)
PUNT_OPCODE(InstanceOfIfaceVtable)
PUNT_OPCODE(LdPropAddr)
PUNT_OPCODE(LdObjMethod)
PUNT_OPCODE(LdObjInvoke)
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
PUNT_OPCODE(CheckInitSProps)
PUNT_OPCODE(NewInstanceRaw)
PUNT_OPCODE(InitObjProps)
PUNT_OPCODE(LdClosureCtx)
PUNT_OPCODE(StClosureCtx)
PUNT_OPCODE(StClosureArg)
PUNT_OPCODE(NewStructArray)
PUNT_OPCODE(FreeActRec)
PUNT_OPCODE(CallArray)
PUNT_OPCODE(NativeImpl)
PUNT_OPCODE(RetCtrl)
PUNT_OPCODE(AsyncRetCtrl)
PUNT_OPCODE(StRetVal)
PUNT_OPCODE(StLocRange)
PUNT_OPCODE(StMem)
PUNT_OPCODE(StRef)
PUNT_OPCODE(StElem)
PUNT_OPCODE(LdStaticLocCached)
PUNT_OPCODE(CheckStaticLocInit)
PUNT_OPCODE(StaticLocInitCached)
PUNT_OPCODE(CufIterSpillFrame)
PUNT_OPCODE(ReqRetranslateOpt)
PUNT_OPCODE(Mov)
PUNT_OPCODE(LdMIStateAddr)
PUNT_OPCODE(IncRefCtx)
PUNT_OPCODE(GenericRetDecRefs)
PUNT_OPCODE(DecRef)
PUNT_OPCODE(DecRefNZ)
PUNT_OPCODE(DefInlineFP)
PUNT_OPCODE(InlineReturn)
PUNT_OPCODE(VerifyParamCls)
PUNT_OPCODE(VerifyRetCls)
PUNT_OPCODE(AKExistsArr)
PUNT_OPCODE(AKExistsObj)
PUNT_OPCODE(ContEnter)
PUNT_OPCODE(ContPreNext)
PUNT_OPCODE(ContStartedCheck)
PUNT_OPCODE(ContValid)
PUNT_OPCODE(ContArIncKey)
PUNT_OPCODE(ContArUpdateIdx)
PUNT_OPCODE(LdContActRec)
PUNT_OPCODE(LdContArValue)
PUNT_OPCODE(StContArValue)
PUNT_OPCODE(LdContArKey)
PUNT_OPCODE(StContArKey)
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
PUNT_OPCODE(PropQ)
PUNT_OPCODE(PropDX)
PUNT_OPCODE(CGetProp)
PUNT_OPCODE(CGetPropQ)
PUNT_OPCODE(VGetProp)
PUNT_OPCODE(BindProp)
PUNT_OPCODE(SetProp)
PUNT_OPCODE(UnsetProp)
PUNT_OPCODE(SetOpProp)
PUNT_OPCODE(IncDecProp)
PUNT_OPCODE(EmptyProp)
PUNT_OPCODE(IssetProp)
PUNT_OPCODE(ElemX)
PUNT_OPCODE(ElemArray)
PUNT_OPCODE(ElemArrayW)
PUNT_OPCODE(ElemDX)
PUNT_OPCODE(ElemUX)
PUNT_OPCODE(ArrayGet)
PUNT_OPCODE(MapGet)
PUNT_OPCODE(CGetElem)
PUNT_OPCODE(VGetElem)
PUNT_OPCODE(ArraySet)
PUNT_OPCODE(MapSet)
PUNT_OPCODE(ArraySetRef)
PUNT_OPCODE(SetElem)
PUNT_OPCODE(UnsetElem)
PUNT_OPCODE(ArrayIsset)
PUNT_OPCODE(StringIsset)
PUNT_OPCODE(MapIsset)
PUNT_OPCODE(IssetElem)
PUNT_OPCODE(EmptyElem)
PUNT_OPCODE(IncStat)
PUNT_OPCODE(RBTraceEntry)
PUNT_OPCODE(RBTraceMsg)
PUNT_OPCODE(IncTransCounter)
PUNT_OPCODE(IncProfCounter)
PUNT_OPCODE(DbgAssertType)
PUNT_OPCODE(MulIntO)
PUNT_OPCODE(EagerSyncVMRegs)
PUNT_OPCODE(ColIsEmpty)
PUNT_OPCODE(ColIsNEmpty)
PUNT_OPCODE(InitPackedArray)
PUNT_OPCODE(InitPackedArrayLoop)
PUNT_OPCODE(LdStrLen)
PUNT_OPCODE(StAsyncArSucceeded)
PUNT_OPCODE(StAsyncArResume)
PUNT_OPCODE(StContArResume)
PUNT_OPCODE(LdContResumeAddr)
PUNT_OPCODE(ContArIncIdx)
PUNT_OPCODE(StContArState)
PUNT_OPCODE(OrdStr)
PUNT_OPCODE(EnterFrame)
PUNT_OPCODE(CheckStackOverflow)
PUNT_OPCODE(InitExtraArgs)
PUNT_OPCODE(InitCtx)
PUNT_OPCODE(CheckSurpriseFlagsEnter)
PUNT_OPCODE(CheckARMagicFlag)
PUNT_OPCODE(StARNumArgsAndFlags)
PUNT_OPCODE(LdARInvName)
PUNT_OPCODE(StARInvName)
PUNT_OPCODE(PackMagicArgs)
PUNT_OPCODE(ProfileSwitchDest)
PUNT_OPCODE(CheckSurpriseAndStack)

#undef PUNT_OPCODE

//////////////////////////////////////////////////////////////////////

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

Vloc CodeGenerator::srcLoc(unsigned i) const {
  return m_state.locs[m_curInst->src(i)];
}

Vloc CodeGenerator::dstLoc(unsigned i) const {
  return m_state.locs[m_curInst->dst(i)];
}

ArgGroup CodeGenerator::argGroup() const {
  return ArgGroup(m_curInst, m_state.locs);
}

Vlabel CodeGenerator::label(Block* b) {
  return m_state.labels[b];
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::recordHostCallSyncPoint(Vout& v, Vpoint p) {
  auto stackOff = m_curInst->marker().spOff();
  auto pcOff = m_curInst->marker().bcOff() - m_curInst->marker().func()->base();
  v << hcsync{Fixup{pcOff, stackOff.offset}, p};
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

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  auto loc = srcLoc(0);
  Type type = src->type();
  if (!type.maybe(TCounted)) return;

  auto increfMaybeStatic = [&](Vout& v) {
    auto base = loc.reg(0);
    auto rCount = v.makeReg();
    v << loadl{base[FAST_REFCOUNT_OFFSET], rCount};
    if (!type.maybe(TStatic)) {
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
    assertx(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic(v);
  } else {
    auto const sf = v.makeReg();
    v << cmpli{KindOfRefCountThreshold, loc.reg(1), sf};
    ifThen(v, CC_G, sf, [&](Vout& v) { increfMaybeStatic(v); });
  }
}

//////////////////////////////////////////////////////////////////////
// Arithmetic Instructions

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();
  auto& v = vmain();
  v << mul{srcRegR, srcRegL, dstReg};
}

//////////////////////////////////////////////////////////////////////
// Bitwise Operators

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
      v << ldimmq{arg.imm().q(), dst};
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
      static_assert(offsetof(TypedValue, m_type) % 8 == 0, "");
      if (src.isVirt()) {
        v << copy{src, dst};
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
  ArgGroup argGroup = toArgGroup(info, m_state.locs, inst);

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:  return kVoidDest;
      case DestType::TV:
      case DestType::SIMD:  return callDestTV(inst);
      case DestType::SSA:
      case DestType::Byte:  return callDest(inst);
      case DestType::Dbl:   return callDestDbl(inst);
    }
    not_reached();
  }();

  cgCallHelper(v, info.func.call, dest, info.sync, argGroup);
}

void CodeGenerator::cgCallHelper(Vout& v,
                                 CppCall call,
                                 const CallDest& dstInfo,
                                 SyncOptions sync,
                                 ArgGroup& args) {
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
    "Stack arguments not yet supported on ARM: `{}'",
    *m_curInst
  );
  shuffleArgs(v, args, call);

  auto syncPoint = emitCall(v, call, argRegs);
  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    recordHostCallSyncPoint(v, syncPoint);
  }

  auto* taken = m_curInst->taken();
  if (taken && taken->isCatch()) {
    assert_not_implemented(args.numStackArgs() == 0);
    auto next = v.makeBlock();
    v << hcunwind{syncPoint, {next, m_state.labels[taken]}};
    v = next;
  } else if (!m_curInst->is(Call, CallArray, ContEnter)) {
    v << hcnocatch{syncPoint};
  }

  switch (dstInfo.type) {
    case DestType::TV: CG_PUNT(cgCall-ReturnTV);
    case DestType::SIMD: CG_PUNT(cgCall-ReturnSIMD);
    case DestType::SSA:
    case DestType::Byte:
      assertx(dstReg1 == InvalidReg);
      v << copy{PhysReg(vixl::x0), dstReg0};
      break;
    case DestType::None:
      assertx(dstReg0 == InvalidReg && dstReg1 == InvalidReg);
      break;
    case DestType::Dbl:
      assertx(dstReg1 == InvalidReg);
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
  return { inst->dst()->isA(TBool) ? DestType::Byte : DestType::SSA,
           loc.reg(0), loc.reg(1) };
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

template <class JmpFn>
void CodeGenerator::emitReffinessTest(IRInstruction* inst, Vreg sf,
                                      JmpFn doJcc) {
  assertx(inst->numSrcs() == 7);

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
  assertx(funcPtrReg.isValid());

  auto nParamsReg = nParamsLoc.reg();

  auto firstBitNum = static_cast<uint32_t>(firstBitNumTmp->intVal());
  auto mask64Reg = mask64Loc.reg();
  uint64_t mask64 = mask64Tmp->intVal();
  assertx(mask64);

  auto vals64Reg = vals64Loc.reg();
  uint64_t vals64 = vals64Tmp->intVal();
  assertx((vals64 & mask64) == vals64);

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
    assertx(nParamsTmp->hasConstVal());
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

void CodeGenerator::cgStStk(IRInstruction* inst) {
  auto const spReg = srcLoc(0).reg();
  auto const offset = cellsToBytes(inst->extra<StStk>()->offset.offset);
  emitStore(vmain(), spReg, offset, inst->src(1), srcLoc(1));
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  not_implemented();
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  not_implemented();
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const func           = inst->extra<CallBuiltinData>()->callee;
  auto const numArgs        = func->numParams();
  auto const funcReturnType = func->returnType();
  auto& v = vmain();

  int returnOffset = rds::kVmMInstrStateOff +
    offsetof(MInstrState, tvBuiltinReturn);

  if (FixupMap::eagerRecord(func)) {
    // Save VM registers
    PhysReg vmfp(rVmFp), vmsp(rVmSp), rds(rVmTl);
    auto const* pc = curFunc()->unit()->entry() + m_curInst->marker().bcOff();
    v << store{vmfp, rds[rds::kVmfpOff]};
    v << store{vmsp, rds[rds::kVmspOff]};
    v << store{v.cns(pc), rds[rds::kVmpcOff]};
  }

  PhysReg mis(rVmTl);

  auto callArgs = argGroup();
  if (isBuiltinByRef(funcReturnType)) {
    if (isReqPtrRef(funcReturnType)) {
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
    if (TVOFF(m_data) && isReqPtrRef(pi.builtinType)) {
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
               isBuiltinByRef(funcReturnType) ? kVoidDest : callDest(dst),
               SyncOptions::kSyncPoint,
               callArgs);

  auto returnType = inst->typeParam();
  if (!dst.isValid() || returnType.isSimpleType()) {
    return;
  }

  if (returnType.isReferenceType()) {
    // this should use some kind of cmov
    assertx(isBuiltinByRef(funcReturnType) && isReqPtrRef(funcReturnType));
    v << load{mis[returnOffset + TVOFF(m_data)], dst};
    if (dstType.isValid()) {
      condZero(v, dst, dstType, [&](Vout& v) {
          return v.cns(KindOfNull);
        }, [&](Vout& v) {
          return v.cns(returnType.toDataType());
        });
    }
    return;
  }

  if (returnType <= TCell || returnType <= TBoxedCell) {
    // this should use some kind of cmov
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    assertx(isBuiltinByRef(funcReturnType) && !isReqPtrRef(funcReturnType));
    auto tmp_dst_type = v.makeReg();
    v << load{mis[returnOffset + TVOFF(m_data)], dst};
    if (dstType.isValid()) {
      v << loadzbl{mis[returnOffset + TVOFF(m_type)], tmp_dst_type};
      condZero(v, tmp_dst_type, dstType, [&](Vout& v) {
          return v.cns(KindOfNull);
        }, [&](Vout& v) {
          return tmp_dst_type;
        });
    }
    return;
  }

  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  // stack args are not supported yet
  assertx(m_state.catch_offsets[inst->block()] == 0);
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
  assertx(src.numWords() == 2);
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
  if (type <= TNull) {
    return;
  }

  auto data = srcLoc.reg();
  if (src->isA(TBool)) {
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
void CodeGenerator::cgStLocPseudoMain(IRInstruction* inst) {
  cgStLocWork(inst);
}

void CodeGenerator::cgLdStk(IRInstruction* inst) {
  assertx(inst->taken() == nullptr);
  auto src = srcLoc(0).reg();
  auto offset = cellsToBytes(inst->extra<LdStk>()->offset.offset);
  emitLoad(vmain(), inst->dst()->type(), dstLoc(0), src, offset);
}

void CodeGenerator::cgLdFuncNumParams(IRInstruction* inst) {
  auto& v = vmain();
  auto dst = dstLoc(0).reg();
  auto src = srcLoc(0).reg()[Func::paramCountsOff()];
  // See Func::finishedEmittingParams and Func::numParams.
  v << loadl{src, dst};
  // TODO(#4894527): this should be doing the following, but we don't support
  // it yet in vasm-arm.
  //
  // auto tmp = v.makeReg();
  // v << loadl{src, tmp};
  // v << shrli{1, tmp, dst, v.makeReg()};
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

void CodeGenerator::cgLdStkAddr(IRInstruction* inst) {
  auto const dst     = dstLoc(0).reg();
  auto const base    = srcLoc(0).reg();
  auto const offset  = cellsToBytes(inst->extra<LdStkAddr>()->offset.offset);
  vmain() << lea{base[offset], dst};
}

void CodeGenerator::cgInterpOneCommon(IRInstruction* inst) {
  auto pcOff = inst->extra<InterpOneData>()->bcOff;
  auto spOff = inst->extra<InterpOneData>()->spOffset;

  auto opc = *(curFunc()->unit()->at(pcOff));
  auto* interpOneHelper = interpOneEntryPoints[opc];

  cgCallHelper(
    vmain(),
    CppCall::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
    kVoidDest,
    SyncOptions::kSyncPoint,
    argGroup()
      .ssa(1/*fp*/)
      .addr(srcLoc(0).reg()/*sp*/, cellsToBytes(spOff.offset))
      .imm(pcOff)
  );
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  cgInterpOneCommon(inst);
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);
  auto& v = vmain();
  PhysReg rds(rVmTl), fp(rVmFp), sp(rVmSp);
  v << load{rds[rds::kVmfpOff], fp};
  v << load{rds[rds::kVmspOff], sp};
  v << jmpi{mcg->tx().uniqueStubs.resumeHelper};
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
  m_curInst = inst;
  SCOPE_EXIT { m_curInst = nullptr; };

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
