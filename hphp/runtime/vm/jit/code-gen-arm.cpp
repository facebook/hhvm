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

#include "hphp/runtime/ext/ext_continuation.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace JIT { namespace ARM {

TRACE_SET_MOD(hhir);

using namespace vixl;

//////////////////////////////////////////////////////////////////////

#define NOOP_OPCODE(name) void CodeGenerator::cg##name(IRInstruction*) {}

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(DefSP)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(Nop)
NOOP_OPCODE(DefLabel)
NOOP_OPCODE(ExceptionBarrier)
NOOP_OPCODE(TakeStack)

// XXX
NOOP_OPCODE(DbgAssertPtr);

// When implemented this shouldn't be a nop, but there's no reason to make us
// punt on everything until then.
NOOP_OPCODE(DbgAssertRetAddr)

#undef NOOP_OPCODE

//////////////////////////////////////////////////////////////////////

#define CALL_OPCODE(name) \
  void CodeGenerator::cg##name(IRInstruction* i) { cgCallNative(m_as, i); }

CALL_OPCODE(Box)
CALL_OPCODE(ConvIntToStr)

CALL_OPCODE(AllocObj)
CALL_OPCODE(NewPackedArray)

CALL_OPCODE(ConcatStrStr)
CALL_OPCODE(ConcatIntStr)
CALL_OPCODE(ConcatStrInt)

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
CALL_OPCODE(CreateContFunc)
CALL_OPCODE(CreateContMeth)
CALL_OPCODE(CreateAFWHFunc)
CALL_OPCODE(CreateAFWHMeth)
CALL_OPCODE(CreateSRWH)
CALL_OPCODE(TypeProfileFunc)
CALL_OPCODE(IncStatGrouped)

/////////////////////////////////////////////////////////////////////
void cgPunt(const char* file, int line, const char* func, uint32_t bcOff,
            const Func* vmFunc) {
  FTRACE(1, "punting: {}\n", func);
  throw FailedCodeGen(file, line, func, bcOff, vmFunc);
}

#define PUNT_OPCODE(name)                                         \
  void CodeGenerator::cg##name(IRInstruction* inst) {             \
    cgPunt(__FILE__, __LINE__, #name, m_curInst->marker().bcOff,  \
           curFunc());                                            \
  }

#define CG_PUNT(instr) \
    cgPunt(__FILE__, __LINE__, #instr, m_curInst->marker().bcOff, curFunc())

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

PUNT_OPCODE(CheckTypeMem)
PUNT_OPCODE(CheckLoc)
PUNT_OPCODE(CastStk)
PUNT_OPCODE(CoerceStk)
PUNT_OPCODE(CheckDefinedClsEq)
PUNT_OPCODE(TryEndCatch)
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
PUNT_OPCODE(FunctionExitSurpriseHook)
PUNT_OPCODE(ExitOnVarEnv)
PUNT_OPCODE(ReleaseVVOrExit)
PUNT_OPCODE(CheckInit)
PUNT_OPCODE(CheckInitMem)
PUNT_OPCODE(CheckCold)
PUNT_OPCODE(CheckNullptr)
PUNT_OPCODE(CheckBounds)
PUNT_OPCODE(LdVectorSize)
PUNT_OPCODE(CheckPackedArrayBounds)
PUNT_OPCODE(CheckPackedArrayElemNull)
PUNT_OPCODE(VectorHasFrozenCopy)
PUNT_OPCODE(VectorDoCow)
PUNT_OPCODE(CheckNonNull)
PUNT_OPCODE(AssertNonNull)
PUNT_OPCODE(Unbox)
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
PUNT_OPCODE(LdClsStaticInitData)
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
PUNT_OPCODE(LdClsPropAddr)
PUNT_OPCODE(LdClsPropAddrCached)
PUNT_OPCODE(LdObjMethod)
PUNT_OPCODE(LdObjInvoke)
PUNT_OPCODE(LdGblAddrDef)
PUNT_OPCODE(LdGblAddr)
PUNT_OPCODE(LdObjClass)
PUNT_OPCODE(LdFunc)
PUNT_OPCODE(LdFuncCachedU)
PUNT_OPCODE(LdFuncCachedSafe)
PUNT_OPCODE(LdSSwitchDestFast)
PUNT_OPCODE(LdSSwitchDestSlow)
PUNT_OPCODE(JmpSwitchDest)
PUNT_OPCODE(ConstructInstance)
PUNT_OPCODE(InitProps)
PUNT_OPCODE(InitSProps)
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
PUNT_OPCODE(IterCopy)
PUNT_OPCODE(LdStaticLocCached)
PUNT_OPCODE(CheckStaticLocInit)
PUNT_OPCODE(StaticLocInitCached)
PUNT_OPCODE(CufIterSpillFrame)
PUNT_OPCODE(ReqRetranslateOpt)
PUNT_OPCODE(Mov)
PUNT_OPCODE(LdAddr)
PUNT_OPCODE(IncRefCtx)
PUNT_OPCODE(DecRefThis)
PUNT_OPCODE(GenericRetDecRefs)
PUNT_OPCODE(DecRef)
PUNT_OPCODE(DecRefNZ)
PUNT_OPCODE(DefInlineFP)
PUNT_OPCODE(InlineReturn)
PUNT_OPCODE(DefInlineSP)
PUNT_OPCODE(ReDefSP)
PUNT_OPCODE(ThingExists);
PUNT_OPCODE(PassSP)
PUNT_OPCODE(PassFP)
PUNT_OPCODE(StashGeneratorSP)
PUNT_OPCODE(ReDefGeneratorSP)
PUNT_OPCODE(VerifyParamCls)
PUNT_OPCODE(VerifyRetCls)
PUNT_OPCODE(ConcatCellCell)
PUNT_OPCODE(AKExists)
PUNT_OPCODE(ContEnter)
PUNT_OPCODE(ContPreNext)
PUNT_OPCODE(ContStartedCheck)
PUNT_OPCODE(ContSetRunning)
PUNT_OPCODE(ContValid)
PUNT_OPCODE(ContArIncKey)
PUNT_OPCODE(ContArUpdateIdx)
PUNT_OPCODE(LdContActRec)
PUNT_OPCODE(StContArRaw)
PUNT_OPCODE(LdContArValue)
PUNT_OPCODE(StContArValue)
PUNT_OPCODE(LdContArKey)
PUNT_OPCODE(StContArKey)
PUNT_OPCODE(LdWHState)
PUNT_OPCODE(LdWHResult)
PUNT_OPCODE(LdAFWHActRec)
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

#undef PUNT_OPCODE

//////////////////////////////////////////////////////////////////////

static TCA kEndOfTargetChain = reinterpret_cast<TCA>(0xf00ffeeffaaff11f);

void CodeGenerator::emitJumpToBlock(CodeBlock& cb,
                                    Block* target,
                                    ConditionCode cc) {
  vixl::MacroAssembler as { cb };

  if (m_state.addresses[target]) {
    not_implemented();
  }

  // The block hasn't been emitted yet. Record the location in CodegenState.
  // CodegenState holds a map from Block* to the head of a linked list, where
  // the jump instructions themselves are the list nodes.
  auto next = reinterpret_cast<TCA>(m_state.patches[target]);
  auto here = cb.frontier();

  // To avoid encoding 0x0 as the jump target. That would conflict with the use
  // of nullptr as a sentinel return value from jmpTarget() and jccTarget().
  // Consider switching those to use folly::Optional or something?
  if (!next) next = kEndOfTargetChain;

  // This will never actually be executed as a jump to "next". It's just a
  // pointer to the next jump instruction to retarget.
  emitSmashableJump(cb, next, cc);
  m_state.patches[target] = here;
}

void patchJumps(CodeBlock& cb, CodegenState& state, Block* block) {
  auto dest = cb.frontier();
  auto jump = reinterpret_cast<TCA>(state.patches[block]);

  while (jump && jump != kEndOfTargetChain) {
    auto nextIfJmp = jmpTarget(jump);
    auto nextIfJcc = jccTarget(jump);

    // Exactly one of them must be non-nullptr
    assert(!(nextIfJmp && nextIfJcc));
    assert(nextIfJmp || nextIfJcc);

    if (nextIfJmp) {
      smashJmp(jump, dest);
      jump = nextIfJmp;
    } else {
      smashJcc(jump, dest);
      jump = nextIfJcc;
    }
  }
}

void emitFwdJmp(CodeBlock& cb, Block* target, CodegenState& state) {
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::recordHostCallSyncPoint(vixl::MacroAssembler& as,
                                            TCA tca) {
  auto stackOff = m_curInst->marker().spOff;
  auto pcOff = m_curInst->marker().bcOff - m_curInst->marker().func->base();
  m_mcg->fixupMap().recordSyncPoint(tca, pcOff, stackOff);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgConjure(IRInstruction* inst) {
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgJmp(IRInstruction* inst) {
  emitJumpToBlock(m_mcg->code.main(), inst->taken(), CC_None);
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  vixl::Label done;
  auto base = x2a(srcLoc(0).reg());
  auto rCount = rAsm;
  m_as.  Ldr   (rCount.W(), base[FAST_REFCOUNT_OFFSET]);
  m_as.  Tbnz  (rCount, UncountedBitPos, &done);
  m_as.  Cmp   (rCount, RefCountMaxRealistic);
  m_as.  B     (&done, vixl::ls);
  m_as.  Brk   (0);
  m_as.  bind  (&done);
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  auto loc = srcLoc(0);
  Type type = src->type();

  if (type.notCounted()) return;

  auto increfMaybeStatic = [&] {
    auto base = x2a(loc.reg(0));
    auto rCount = rAsm.W();
    m_as.    Ldr  (rCount, base[FAST_REFCOUNT_OFFSET]);
    if (!type.needsStaticBitCheck()) {
      m_as.  Add  (rCount, rAsm.W(), 1);
      m_as.  Str  (rCount, base[FAST_REFCOUNT_OFFSET]);
    } else {
      m_as.  Cmp  (rCount, 0);
      static_assert(UncountedValue < 0 && StaticValue < 0, "");
      ifThen(m_as, vixl::ge, [&] {
        m_as.Add(rCount, rCount, 1);
        m_as.Str(rCount, base[FAST_REFCOUNT_OFFSET]);
      });
    }
  };

  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic();
  } else {
    m_as.    Cmp (x2a(loc.reg(1)).W(), KindOfRefCountThreshold);
    ifThen(m_as, vixl::gt, [&] { increfMaybeStatic(); });
  }
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  auto const srcRegs = srcLoc(0);
  auto const dstRegs = dstLoc(0);

  PhysReg::Map<PhysReg> moves;
  if (dstRegs.reg(0) != InvalidReg)
    moves[dstRegs.reg(0)] = srcRegs.reg(0);
  if (dstRegs.reg(1) != InvalidReg)
    moves[dstRegs.reg(1)] = srcRegs.reg(1);

  auto howTo = doRegMoves(moves, rAsm);
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      m_as.  Mov  (x2a(how.m_dst), x2a(how.m_src));
    } else {
      emitXorSwap(m_as, x2a(how.m_dst), x2a(how.m_src));
    }
  }
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::emitDecRefStaticType(Type type,
                                         vixl::Register dataReg) {
  assert(type.isKnownDataType());
  assert(!dataReg.Is(rAsm2));

  vixl::Label allDone;

  m_as.  Ldr  (rAsm2.W(), dataReg[FAST_REFCOUNT_OFFSET]);

  if (type.needsStaticBitCheck()) {
    m_as.Tbnz (rAsm2, UncountedBitPos, &allDone);
  }

  m_as.  Sub  (rAsm2.W(), rAsm2.W(), 1, vixl::SetFlags);
  m_as.  Str  (rAsm2.W(), dataReg[FAST_REFCOUNT_OFFSET]);

  m_as.  B    (&allDone, vixl::ne);
  cgCallHelper(m_as,
               MCGenerator::getDtorCall(type.toDataType()),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().reg(dataReg));

  m_as.  bind (&allDone);
}

void CodeGenerator::emitDecRefDynamicType(vixl::Register baseReg,
                                          int offset) {
  // Make sure both temp registers are still available
  assert(!baseReg.Is(rAsm));
  assert(!baseReg.Is(rAsm2));

  vixl::Label allDone;

  // Check the type
  m_as.  Ldrb (rAsm.W(), baseReg[offset + TVOFF(m_type)]);
  m_as.  Cmp  (rAsm.W(), KindOfRefCountThreshold);
  m_as.  B    (&allDone, vixl::le);

  // Type is refcounted. Load the refcount.
  m_as.  Ldr  (rAsm, baseReg[offset + TVOFF(m_data)]);
  m_as.  Ldr  (rAsm2.W(), rAsm[FAST_REFCOUNT_OFFSET]);

  // Is it static? Note that only the lower 32 bits of rAsm2 are valid right
  // now, but tbnz is only looking at a single one of them, so this is OK.
  m_as.  Tbnz (rAsm2, UncountedBitPos, &allDone);

  // Not static. Decrement and write back.
  m_as.  Sub  (rAsm2.W(), rAsm2.W(), 1, vixl::SetFlags);
  m_as.  Str  (rAsm2.W(), rAsm[FAST_REFCOUNT_OFFSET]);

  // Did it go to zero?
  m_as.  B    (&allDone, vixl::ne);

  // Went to zero. Have to destruct.
  cgCallHelper(m_as,
               CppCall(tv_release_generic),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(baseReg, offset));

  m_as.  bind (&allDone);
}

void CodeGenerator::emitDecRefMem(Type type,
                                  vixl::Register baseReg,
                                  int offset) {
  if (type.needsReg()) {
    emitDecRefDynamicType(baseReg, offset);
  } else if (type.maybeCounted()) {
    m_as.  Ldr  (rAsm, baseReg[offset + TVOFF(m_data)]);
    emitDecRefStaticType(type, rAsm);
  }
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(srcLoc(0).reg()),
                cellsToBytes(inst->extra<DecRefStack>()->offset));
}

void CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(srcLoc(0).reg()),
                localOffset(inst->extra<DecRefLoc>()->locId));
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(srcLoc(0).reg()),
                inst->src(1)->intVal());
}

//////////////////////////////////////////////////////////////////////
// Arithmetic Instructions

void CodeGenerator::cgAddInt(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegR != InvalidReg) {
    m_as. Add(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. Add(x2a(dstReg), x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgSubInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegR != InvalidReg) {
    m_as. Sub(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. Sub(x2a(dstReg), x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  m_as. Mul(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
}

//////////////////////////////////////////////////////////////////////
// Bitwise Operators

void CodeGenerator::cgAndInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegL != InvalidReg) {
    m_as. And(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. And(x2a(dstReg), x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgOrInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegL != InvalidReg) {
    m_as. Orr(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. Orr(x2a(dstReg), x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgXorInt(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegL != InvalidReg) {
    m_as. Eor(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. Eor(x2a(dstReg), x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgShl(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  // TODO: t3870154 add shift-by-immediate support to vixl
  m_as. lslv(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
}

void CodeGenerator::cgShr(IRInstruction* inst) {
  auto dstReg  = dstLoc(0).reg();
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  // TODO: t3870154 add shift-by-immediate support to vixl
  m_as. asrv(x2a(dstReg), x2a(srcRegL), x2a(srcRegR));
}
//////////////////////////////////////////////////////////////////////
// Comparison Operations

void CodeGenerator::emitCompareIntAndSet(IRInstruction *inst,
                                         vixl::Condition cond) {
  auto dstReg = dstLoc(0).reg();
  emitCompareInt(inst);
  m_as. Cset(x2a(dstReg),cond);
}

void CodeGenerator::emitCompareInt(IRInstruction* inst) {
  auto srcRegL = srcLoc(0).reg();
  auto srcRegR = srcLoc(1).reg();

  if (srcRegR != InvalidReg) {
    m_as. Cmp(x2a(srcRegL), x2a(srcRegR));
  } else {
    m_as. Cmp(x2a(srcRegL), inst->src(1)->intVal());
  }
}

void CodeGenerator::cgLtInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::lt);
}

void CodeGenerator::cgGtInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::gt);
}


void CodeGenerator::cgGteInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::ge);
}

void CodeGenerator::cgLteInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::le);
}


void CodeGenerator::cgEqInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::eq);
}

void CodeGenerator::cgNeqInt(IRInstruction* inst) {
  emitCompareIntAndSet(inst,vixl::Condition::ne);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgShuffle(IRInstruction* inst) {
  PhysReg::Map<PhysReg> moves;

  // Put required reg-reg moves in the map, and do spills at the same time.
  for (auto i = 0; i < inst->numSrcs(); ++i) {
    auto& rd = inst->extra<Shuffle>()->dests[i];
    auto& rs = srcLoc(i);

    if (rd.numAllocated() == 0) continue;
    if (rd.spilled()) {
      for (auto j = 0; j < rd.numAllocated(); ++j) {
        m_as.  Str  (x2a(rs.reg(j)), vixl::MemOperand(vixl::sp, rd.offset(j)));
      }
    } else if (!rs.spilled()) {
      if (rs.reg(0) != InvalidReg) moves[rd.reg(0)] = rs.reg(0);
      if (rs.reg(1) != InvalidReg) moves[rd.reg(1)] = rs.reg(1);
    }
  }

  // Do reg-reg moves.
  auto howTo = doRegMoves(moves, rAsm);
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      emitRegGetsRegPlusImm(m_as, x2a(how.m_dst), x2a(how.m_src), 0);
    } else {
      emitXorSwap(m_as, x2a(how.m_src), x2a(how.m_dst));
    }
  }

  // Now do reloads and reg<-imm.
  for (auto i = 0; i < inst->numSrcs(); ++i) {
    auto src = inst->src(i);
    auto& rd = inst->extra<Shuffle>()->dests[i];
    auto& rs = srcLoc(i);
    if (rd.numAllocated() == 0) continue;
    if (rd.spilled()) continue;
    if (rs.spilled()) {
      for (auto j = 0; j < rd.numAllocated(); ++j) {
        m_as.  Ldr  (x2a(rd.reg(i)), vixl::MemOperand(vixl::sp, rs.offset(j)));
      }
      continue;
    }
    if (rs.numAllocated() == 0) {
      assert(src->inst()->op() == DefConst);
      m_as.  Mov  (x2a(rd.reg(0)), src->rawVal());
    }
    if (rd.numAllocated() == 2 && rs.numAllocated() < 2) {
      // Move src known type to register
      m_as.  Mov  (x2a(rd.reg(1)), src->type().toDataType());
    }
  }

}

//////////////////////////////////////////////////////////////////////

static void shuffleArgs(vixl::MacroAssembler& a,
                        ArgGroup& args,
                        CppCall& call) {
  assert_not_implemented(args.numStackArgs() == 0);

  PhysReg::Map<PhysReg> moves;
  PhysReg::Map<ArgDesc*> argDescs;

  for (size_t i = 0; i < args.numRegArgs(); i++) {
    auto kind = args[i].kind();
    if (!(kind == ArgDesc::Kind::Reg  ||
          kind == ArgDesc::Kind::Addr ||
          kind == ArgDesc::Kind::TypeReg)) {
      continue;
    }
    auto dstReg = args[i].dstReg();
    auto srcReg = args[i].srcReg();
    if (dstReg != srcReg) {
      moves[dstReg] = srcReg;
      argDescs[dstReg] = &args[i];
    }
    if (call.isIndirect() && dstReg == call.getReg()) {
      // an indirect call uses an argument register for the func ptr.
      // Use rAsm2 instead and update the CppCall
      moves[rAsm2] = call.getReg();
      call.updateCallIndirect(rAsm2);
    }
  }

  auto const howTo = doRegMoves(moves, rAsm);

  for (auto& how : howTo) {
    auto srcReg = x2a(how.m_src);
    auto dstReg = x2a(how.m_dst);
    if (how.m_kind == MoveInfo::Kind::Move) {
      auto* argDesc = argDescs[how.m_dst];
      if (argDesc) {
        auto kind = argDesc->kind();
        if (kind == ArgDesc::Kind::Addr) {
          emitRegGetsRegPlusImm(a, dstReg, srcReg, argDesc->disp().l());
        } else {
          if (argDesc->isZeroExtend()) {
            // "Unsigned eXTend Byte". The dest reg is a 32-bit reg but this
            // zeroes the top 32 bits, so the intended effect is achieved.
            a.Uxtb (dstReg.W(), srcReg.W());
          } else {
            a.Mov  (dstReg, srcReg);
          }
        }
        if (kind != ArgDesc::Kind::TypeReg) {
          argDesc->markDone();
        }
      } else {
        a.  Mov  (dstReg, srcReg);
      }
    } else {
      emitXorSwap(a, dstReg, srcReg);
    }
  }

  for (size_t i = 0; i < args.numRegArgs(); ++i) {
    if (!args[i].done()) {
      auto kind = args[i].kind();
      auto dstReg = x2a(args[i].dstReg());
      if (kind == ArgDesc::Kind::Imm) {
        a.  Mov  (dstReg, args[i].imm().q());
      } else if (kind == ArgDesc::Kind::Reg || kind == ArgDesc::Kind::TypeReg) {
        // Should have already been done
      } else {
        not_implemented();
      }
    }
  }
}

void CodeGenerator::cgCallNative(vixl::MacroAssembler& as,
                                 IRInstruction* inst) {
  using namespace NativeCalls;

  Opcode opc = inst->op();
  always_assert(CallMap::hasInfo(opc));

  auto const& info = CallMap::info(opc);
  ArgGroup argGroup = info.toArgGroup(m_state.regs, inst);

  auto call = [&]() -> CppCall {
    switch (info.func.type) {
    case FuncType::Call:
      return CppCall(info.func.call);
    case FuncType::SSA:
      return CppCall(inst->src(info.func.srcIdx)->tcaVal());
    }
    not_reached();
  }();

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:  return kVoidDest;
      case DestType::TV:    return callDestTV(inst);
      case DestType::SSA:   return callDest(inst);
      case DestType::SSA2:  return callDest2(inst);
    }
    not_reached();
  }();

  cgCallHelper(as, call, dest, info.sync, argGroup);
}

void CodeGenerator::cgCallHelper(vixl::MacroAssembler& a,
                                 CppCall call,
                                 const CallDest& dstInfo,
                                 SyncOptions sync,
                                 ArgGroup& args,
                                 RegSet toSave) {
  assert(m_curInst->isNative());

  auto dstReg0 = dstInfo.reg0;
  auto dstReg1 = dstInfo.reg1;

  if (debug) {
    toSave.forEach([](PhysReg r) { assert(r.isGP()); });
  }

  toSave = toSave & kCallerSaved;
  assert((toSave & RegSet().add(dstReg0).add(dstReg1)).empty());

  // Use vixl's handy helper to push caller-save regs. It uses ldp/stp when
  // possible.
  CPURegList pushedRegs(vixl::CPURegister::kRegister, vixl::kXRegSize, 0);
  toSave.forEach([&](PhysReg r) { pushedRegs.Combine(r); });

  // The vixl helper requires you to pass it an even number of registers. If we
  // have an odd number of regs to save, remove one from the list we pass, and
  // save it ourselves.
  folly::Optional<vixl::CPURegister> maybeOddOne;
  if (pushedRegs.Count() % 2 == 1) {
    maybeOddOne = pushedRegs.PopHighestIndex();
  }
  a.    PushCPURegList(pushedRegs);
  if (maybeOddOne) {
    // We're only storing a single reg, but the stack pointer must always be
    // 16-byte aligned. This instruction subtracts 16 from the stack pointer,
    // then writes the value.
    a.  Str  (maybeOddOne.value(), MemOperand(vixl::sp, -16, vixl::PreIndex));
  }

  SCOPE_EXIT {
    if (maybeOddOne) {
      // Read the value, then add 16 to the stack pointer.
      a.Ldr  (maybeOddOne.value(), MemOperand(vixl::sp, 16, vixl::PostIndex));
    }
    a.  PopCPURegList(pushedRegs);
  };

  for (size_t i = 0; i < args.numRegArgs(); i++) {
    args[i].setDstReg(PhysReg{argReg(i)});
  }
  shuffleArgs(a, args, call);

  auto syncPoint = emitCall(a, call);

  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    recordHostCallSyncPoint(a, syncPoint);
  }

  auto armDst0 = x2a(dstReg0);
  DEBUG_ONLY auto armDst1 = x2a(dstReg1);

  switch (dstInfo.type) {
    case DestType::TV: not_implemented();
    case DestType::SSA:
      assert(!armDst1.IsValid());
      if (armDst0.IsValid() && !armDst0.Is(vixl::x0)) a.Mov(armDst0, vixl::x0);
      break;
    case DestType::SSA2: not_implemented();
    case DestType::None:
      assert(!armDst0.IsValid() && !armDst1.IsValid());
      break;
  }
}

void CodeGenerator::cgCallHelper(vixl::MacroAssembler& a,
                                 CppCall call,
                                 const CallDest& dstInfo,
                                 SyncOptions sync,
                                 ArgGroup& args) {
  cgCallHelper(a, call, dstInfo, sync, args, m_state.liveRegs[m_curInst]);
}

/*
 * XXX copypasta but has to be in the class because of curPhysLoc and
 * changing that would make callsites real messy
 */

CallDest CodeGenerator::callDest(PhysReg reg0,
                                 PhysReg reg1 /* = InvalidReg */) const {
  return { DestType::SSA, reg0, reg1 };
}

CallDest CodeGenerator::callDest(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::SSA, loc.reg(0), loc.reg(1) };
}

CallDest CodeGenerator::callDestTV(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::TV, loc.reg(0), loc.reg(1) };
}

CallDest CodeGenerator::callDest2(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::SSA2, loc.reg(0), loc.reg(1) };
}

//////////////////////////////////////////////////////////////////////

static vixl::Register enregister(vixl::MacroAssembler& a,
                                 vixl::MemOperand memRef,
                                 vixl::Register scratch) {
  a.  Ldr  (scratch, memRef);
  return scratch;
}

static vixl::Register enregister(vixl::MacroAssembler& a,
                                 vixl::Register reg,
                                 vixl::Register scratch) {
  return reg;
}

template<class Loc, class JmpFn>
void CodeGenerator::emitTypeTest(Type type, vixl::Register typeReg, Loc dataSrc,
                                 JmpFn doJcc) {
  assert(!(type <= Type::Cls));
  assert(typeReg.Is32Bits());

  if (type.equals(Type::Gen)) {
    return;
  }

  ConditionCode cc;
  if (type <= Type::Str) {
    // Note: ARM can actually do better here; it has a fused test-and-branch
    // instruction. The way this code is factored makes it difficult to use,
    // though; the jump instruction will be written by some other code.
    m_as.   Tst   (typeReg, KindOfStringBit);
    cc = CC_NE;
  } else if (type.equals(Type::UncountedInit)) {
    m_as.   Tst   (typeReg, KindOfUncountedInitBit);
    cc = CC_NE;
  } else if (type.equals(Type::Uncounted)) {
    m_as.   Cmp   (typeReg, KindOfRefCountThreshold);
    cc = CC_LE;
  } else if (type.equals(Type::Cell)) {
    m_as.   Cmp   (typeReg, KindOfRef);
    cc = CC_L;
  } else {
    assert(type.isKnownDataType());
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfResource));
    m_as.   Cmp   (typeReg, dataType);
    cc = CC_E;
  }
  doJcc(cc);
  if (type < Type::Obj) {
    assert(type.getClass()->attrs() & AttrFinal);
    auto dataReg = enregister(m_as, dataSrc, rAsm);
    m_as.   Ldr   (rAsm, dataReg[ObjectData::getVMClassOffset()]);
    m_as.   Cmp   (rAsm, reinterpret_cast<int64_t>(type.getClass()));
    doJcc(CC_E);
  } else if (type < Type::Res) {
    CG_PUNT(TypeTest-on-Resource);
  } else if (type <= Type::Arr && type.hasArrayKind()) {
    auto dataReg = enregister(m_as, dataSrc, rAsm);
    m_as.   Ldrb  (rAsm.W(), dataReg[ArrayData::offsetofKind()]);
    m_as.   Cmp   (rAsm.W(), type.getArrayKind());
    doJcc(CC_E);
  }
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  auto const rFP = x2a(srcLoc(0).reg());
  auto const baseOff = localOffset(inst->extra<GuardLoc>()->locId);
  m_as.  Ldrb  (rAsm.W(), rFP[baseOff + TVOFF(m_type)]);
  emitTypeTest(
    inst->typeParam(),
    rAsm.W(),
    rFP[baseOff + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
      auto const destSR = m_mcg->tx().getSrcRec(destSK);
      destSR->emitFallbackJump(this->m_mainCode, ccNegate(cc));
    });
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  auto const rSP = x2a(srcLoc(0).reg());
  auto const baseOff = cellsToBytes(inst->extra<GuardStk>()->offset);
  m_as.  Ldrb  (rAsm.W(), rSP[baseOff + TVOFF(m_type)]);
  emitTypeTest(
    inst->typeParam(),
    rAsm.W(),
    rSP[baseOff + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
      auto const destSR = m_mcg->tx().getSrcRec(destSK);
      destSR->emitFallbackJump(this->m_mainCode, ccNegate(cc));
    });
}

void CodeGenerator::cgCheckStk(IRInstruction* inst) {
  auto const rSP = x2a(srcLoc(0).reg());
  auto const baseOff = cellsToBytes(inst->extra<CheckStk>()->offset);
  m_as.  Ldrb  (rAsm.W(), rSP[baseOff + TVOFF(m_type)]);
  emitTypeTest(
    inst->typeParam(),
    rAsm.W(),
    rSP[baseOff + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      emitJumpToBlock(m_mcg->code.main(), inst->taken(), ccNegate(cc));
    }
  );
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->src(0);
  Type   srcType   = src->type();
  auto const rVal  = x2a(srcLoc(0).reg(0));
  auto const rType = x2a(srcLoc(0).reg(1));

  auto doMov = [&] {
    auto const valDst = x2a(dstLoc(0).reg(0));
    auto const typeDst = x2a(dstLoc(0).reg(1));
    // TODO: #3626251: XLS: Let Uses say whether a constant is
    // allowed, and if not, assign a register.
    if (valDst.IsValid()) {
      if (rVal.IsValid()) {
        if (!valDst.Is(rVal)) m_as.Mov(valDst, rVal);
      } else {
        if (src->isConst()) m_as.Mov(valDst, src->rawVal());
      }
    }
    if (typeDst.IsValid()) {
      if (rType.IsValid()) {
        if (!typeDst.Is(rType)) m_as.Mov(typeDst, rType);
      } else {
        m_as.Mov(typeDst, srcType.toDataType());
      }
    }
  };

  auto doJcc = [&] (ConditionCode cc) {
    emitJumpToBlock(m_mcg->code.main(), inst->taken(), ccNegate(cc));
  };

  Type typeParam = inst->typeParam();
  if (src->isA(typeParam) ||
      // Boxed types are checked lazily, so there's nothing to be done here.
      (srcType.isBoxed() && typeParam.isBoxed())) {
    doMov();
    return;
  }
  if (srcType.not(typeParam)) {
    emitJumpToBlock(m_mcg->code.main(), inst->taken(), CC_None);
    return;
  }

  if (rType.IsValid()) {
    emitTypeTest(typeParam, rType.W(), rVal, doJcc);
  } else if (typeParam <= Type::Uncounted &&
             ((srcType == Type::Str && typeParam.maybe(Type::StaticStr)) ||
              (srcType == Type::Arr && typeParam.maybe(Type::StaticArr)))) {
    // We carry Str and Arr operands around without a type register,
    // even though they're union types.  The static and non-static
    // subtypes are distinguised by the refcount field.
    assert(rVal.IsValid());
    m_as.  Ldr  (rAsm.W(), rVal[FAST_REFCOUNT_OFFSET]);
    m_as.  Cmp  (rAsm, 0);
    doJcc(CC_L);
  } else {
    always_assert_log(
      false,
      [&] {
        return folly::format("Bad src: {} and dst: {} types in '{}'",
                             srcType, typeParam, *inst).str();
      });
  }
  doMov();
}

void CodeGenerator::cgSideExitGuardStk(IRInstruction* inst) {
  auto const sp = x2a(srcLoc(0).reg());
  auto const extra = inst->extra<SideExitGuardStk>();

  m_as.  Ldrb  (rAsm.W(), sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_type)]);
  emitTypeTest(
    inst->typeParam(),
    rAsm.W(),
    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const sk = SrcKey(curFunc(), extra->taken);
      emitBindSideExit(this->m_mainCode, this->m_stubsCode, sk, ccNegate(cc));
    }
  );
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
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
  auto funcPtrReg = x2a(funcPtrLoc.reg());
  assert(funcPtrReg.IsValid());

  auto nParamsReg = x2a(nParamsLoc.reg());
  assert(nParamsReg.IsValid() || nParamsTmp->isConst());

  auto firstBitNum = static_cast<uint32_t>(firstBitNumTmp->intVal());
  auto mask64Reg = x2a(mask64Loc.reg());
  uint64_t mask64 = mask64Tmp->intVal();
  assert(mask64Reg.IsValid() || mask64 == uint32_t(mask64));
  assert(mask64);

  auto vals64Reg = x2a(vals64Loc.reg());
  uint64_t vals64 = vals64Tmp->intVal();
  assert(vals64Reg.IsValid() || vals64 == uint32_t(vals64));
  assert((vals64 & mask64) == vals64);

  auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
  auto const destSR = m_mcg->tx().getSrcRec(destSK);

  auto thenBody = [&] {
    auto bitsOff = sizeof(uint64_t) * (firstBitNum / 64);
    auto cond = CC_NE;
    auto bitsPtrReg = rAsm;

    if (firstBitNum == 0) {
      bitsOff = Func::refBitValOff();
      bitsPtrReg = funcPtrReg;
    } else {
      m_as.    Ldr  (bitsPtrReg, funcPtrReg[Func::sharedOff()]);
      bitsOff -= sizeof(uint64_t);
    }

    // Don't need the bits pointer after this point
    auto bitsReg = rAsm;
    // Load the bits
    m_as.    Ldr  (bitsReg, bitsPtrReg[bitsOff]);

    // Mask the bits. There are restrictions on what can be encoded as an
    // immediate in ARM's logical instructions, and if they're not met,
    // the assembler will compensate using ip0 or ip1 as tmps.
    if (mask64Reg.IsValid()) {
      m_as.  And  (bitsReg, bitsReg, mask64Reg);
    } else {
      m_as.  And  (bitsReg, bitsReg, mask64);
    }

    // Now do the compare. There are also restrictions on immediates in
    // arithmetic instructions (of which Cmp is one; it's just a subtract that
    // sets flags), so same deal as with the mask immediate above.
    if (vals64Reg.IsValid()) {
      m_as.  Cmp  (bitsReg, vals64Reg);
    } else {
      m_as.  Cmp  (bitsReg, vals64);
    }
    destSR->emitFallbackJump(m_mainCode, cond);
  };

  if (firstBitNum == 0) {
    assert(!nParamsReg.IsValid());
    // This is the first 64 bits. No need to check
    // nParams.
    thenBody();
  } else {
    assert(nParamsReg.IsValid());
    // Check number of args...
    m_as.    Cmp   (nParamsReg, firstBitNum);

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nParams, then either all params
      // are refs, or all params are non-refs, so if vals64
      // isn't 0 and isnt mask64, there's no possibility of
      // a match
      destSR->emitFallbackJump(m_mainCode, CC_LE);
      thenBody();
    } else {
      ifThenElse(m_as, vixl::gt, thenBody, /* else */ [&] {
          //   If not special builtin...
          m_as.  Ldr  (rAsm, funcPtrReg[Func::attrsOff()]);
          m_as.  Tst  (rAsm, AttrVariadicByRef);
          destSR->emitFallbackJump(m_mainCode, vals64 ? CC_Z : CC_NZ);
        });
    }
  }
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgSyncABIRegs(IRInstruction* inst) {
  emitRegGetsRegPlusImm(m_as, rVmFp, x2a(srcLoc(0).reg()), 0);
  emitRegGetsRegPlusImm(m_as, rVmSp, x2a(srcLoc(1).reg()), 0);
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  emitBindJmp(
    m_mainCode,
    m_stubsCode,
    SrcKey(curFunc(), inst->extra<ReqBindJmp>()->offset)
  );
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  assert(m_unit.bcOff() == inst->marker().bcOff);
  auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
  auto const destSR = m_mcg->tx().getSrcRec(destSK);
  destSR->emitFallbackJump(m_mainCode);
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func     = inst->src(2);
  auto const objOrCls = inst->src(3);
  auto const invName  = inst->extra<SpillFrame>()->invName;
  auto const nArgs    = inst->extra<SpillFrame>()->numArgs;

  auto spReg = x2a(srcLoc(0).reg());
  auto fpReg = x2a(srcLoc(1).reg());
  auto funcLoc = srcLoc(2);
  auto objClsReg = x2a(srcLoc(3).reg());
  auto spOff = -kNumActRecCells * sizeof(Cell);

  // Saved rbp.
  m_as.    Str  (fpReg, spReg[spOff + AROFF(m_savedRbp)]);

  // Num args. Careful here: nArgs is 32 bits and the high bit may be set. Mov's
  // immediate argument is intptr_t, and the implicit int32->intptr conversion
  // will sign-extend, which isn't what we want.
  m_as.    Mov  (rAsm.W(), (uint32_t)nArgs);
  m_as.    Str  (rAsm.W(), spReg[spOff + AROFF(m_numArgsAndGenCtorFlags)]);

  // Magic-call name.
  if (invName) {
    auto bits = reinterpret_cast<uintptr_t>(invName) | ActRec::kInvNameBit;
    m_as.  Mov  (rAsm, bits);
    m_as.  Str  (rAsm, spReg[spOff + AROFF(m_invName)]);
  } else {
    m_as.  Str  (vixl::xzr, spReg[spOff + AROFF(m_invName)]);
  }

  // Func and this/class are slightly tricky. The func may be a tuple of a Func*
  // and context.

  if (objOrCls->isA(Type::Cls)) {
    if (objOrCls->isConst()) {
      m_as.Mov  (rAsm, uintptr_t(objOrCls->clsVal()) | 1);
      m_as.Str  (rAsm, spReg[spOff + AROFF(m_this)]);
    } else {
      m_as.Orr  (rAsm, objClsReg, 1);
      m_as.Str  (rAsm, spReg[spOff + AROFF(m_this)]);
    }
  } else if (objOrCls->isA(Type::Obj) || objOrCls->isA(Type::Ctx)) {
    m_as.  Str  (objClsReg, spReg[spOff + AROFF(m_this)]);
  } else {
    assert(objOrCls->isA(Type::InitNull));
    m_as.Str  (vixl::xzr, spReg[spOff + AROFF(m_this)]);
  }

  // Now set func, and possibly this/cls
  if (func->isA(Type::Null)) {
    // Do nothing
    assert(func->isConst());
  } else if (func->isConst()) {
    m_as.  Mov  (rAsm, func->funcVal());
    m_as.  Str  (rAsm, spReg[spOff + AROFF(m_func)]);
  } else {
    auto reg0 = x2a(funcLoc.reg(0));
    m_as.  Str  (reg0, spReg[spOff + AROFF(m_func)]);
  }

  // Adjust stack pointer
  emitRegGetsRegPlusImm(m_as, x2a(dstLoc(0).reg()), spReg, spOff);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto func = inst->src(0)->funcVal();
  auto args = inst->srcs().subpiece(2);
  auto numArgs = args.size();

  DataType funcReturnType = func->returnType();
  int returnOffset = MISOFF(tvBuiltinReturn);

  if (FixupMap::eagerRecord(func)) {
    // Save VM registers
    auto const* pc = curFunc()->unit()->entry() + m_curInst->marker().bcOff;
    m_as.Str  (rVmFp, rGContextReg[offsetof(ExecutionContext, m_fp)]);
    m_as.Str  (rVmSp, rGContextReg[offsetof(ExecutionContext, m_stack) +
                                   Stack::topOfStackOffset()]);
    m_as.Mov  (rAsm, pc);
    m_as.Str  (rAsm, rGContextReg[offsetof(ExecutionContext, m_pc)]);
  }

  // The stack pointer currently points to the MInstrState we need to use.
  auto misReg = rAsm;
  m_as.  Mov  (rAsm, vixl::sp);

  auto callArgs = argGroup();
  if (isCppByRef(funcReturnType)) {
    if (isSmartPtrRef(funcReturnType)) {
      // first arg is pointer to storage for the return value
      returnOffset += TVOFF(m_data);
    }
    callArgs.addr(misReg, returnOffset);
  }

  for (int i = 0; i < numArgs; ++i) {
    auto const& pi = func->params()[i];
    if (TVOFF(m_data) && isSmartPtrRef(pi.builtinType())) {
      callArgs.addr(srcLoc(i + 2).reg(), TVOFF(m_data));
    } else {
      callArgs.ssa(i + 2);
    }
  }

  misReg = vixl::sp;

  auto dloc = dstLoc(0);
  auto dstReg = x2a(dloc.reg(0));
  auto dstTypeReg = x2a(dloc.reg(1));

  cgCallHelper(m_as,
               CppCall((TCA)func->nativeFuncPtr()),
               isCppByRef(funcReturnType) ? kVoidDest : callDest(dstReg),
               SyncOptions::kSyncPoint,
               callArgs);

  auto returnType = inst->typeParam();
  if (!dstReg.IsValid() || returnType.isSimpleType()) {
    return;
  }

  if (returnType.isReferenceType()) {
    assert(isCppByRef(funcReturnType) && isSmartPtrRef(funcReturnType));
    vixl::Label notNullptr;
    vixl::Label done;
    m_as.  Ldr  (dstReg, misReg[returnOffset + TVOFF(m_data)]);
    m_as.  Cbnz (dstReg, &notNullptr);
    m_as.  Mov  (dstTypeReg, KindOfNull);
    m_as.  B    (&done);
    m_as.  bind (&notNullptr);
    m_as.  Mov  (dstTypeReg, returnType.toDataType());
    m_as.  bind (&done);
    return;
  }

  if (returnType <= Type::Cell || returnType <= Type::BoxedCell) {
    assert(isCppByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    vixl::Label notUninit;
    vixl::Label done;
    m_as.  Ldrb (dstTypeReg.W(), misReg[returnOffset + TVOFF(m_type)]);
    m_as.  Cbnz (dstTypeReg, &notUninit);
    m_as.  Mov  (dstTypeReg, KindOfNull);
    m_as.  B    (&done);
    m_as.  bind (&notUninit);
    m_as.  Ldr  (dstReg, misReg[returnOffset + TVOFF(m_data)]);
    m_as.  bind (&done);
    return;
  }

  always_assert(false);
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  auto spReg = x2a(srcLoc(0).reg());
  auto* returnBcOffset = inst->src(1);
  auto* func = inst->src(2);
  SrcRange args = inst->srcs().subpiece(3);
  int32_t numArgs = args.size();

  int64_t adjustment = cellsToBytes((int64_t)-numArgs);
  for (int32_t i = 0; i < numArgs; ++i) {
    emitStore(spReg, cellsToBytes(-(i + 1)), args[i], srcLoc(i + 3));
  }

  m_as.  Mov  (rAsm.W(), returnBcOffset->intVal());
  m_as.  Str  (rAsm.W(), spReg[AROFF(m_soff)]);
  emitRegGetsRegPlusImm(m_as, spReg, spReg, adjustment);

  assert(m_curInst->marker().valid());
  SrcKey srcKey = SrcKey(m_curInst->marker().func, m_curInst->marker().bcOff);
  bool isImmutable = func->isConst() && !func->isA(Type::Null);
  const Func* funcd = isImmutable ? func->funcVal() : nullptr;
  int32_t adjust  = emitBindCall(mcg->code.main(), mcg->code.stubs(),
                                 srcKey, funcd, numArgs);

  emitRegGetsRegPlusImm(m_as, rVmSp, rVmSp, adjust);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  m_as.  Brk  (0);
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  m_as.  Brk  (0);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::emitLoadTypedValue(PhysLoc dst,
                                       vixl::Register base,
                                       ptrdiff_t offset,
                                       Block* label) {
  auto valueDstReg = x2a(dst.reg(0));
  auto typeDstReg  = x2a(dst.reg(1));

  if (label) not_implemented();

  if (valueDstReg.IsFPRegister()) {
    not_implemented();
  }

  // Avoid clobbering the base reg if we'll need it later
  if (base.Is(typeDstReg) && valueDstReg.IsValid()) {
    m_as.  Mov  (rAsm, base);
    base = rAsm;
  }

  if (typeDstReg.IsValid()) {
    m_as.  Ldrb (typeDstReg.W(), base[offset + TVOFF(m_type)]);
  }

  if (valueDstReg.IsValid()) {
    m_as.  Ldr  (valueDstReg, base[offset + TVOFF(m_data)]);
  }
}

void CodeGenerator::emitStoreTypedValue(vixl::Register base,
                                        ptrdiff_t offset,
                                        PhysLoc src) {
  assert(src.numWords() == 2);
  auto reg0 = x2a(src.reg(0));
  auto reg1 = x2a(src.reg(1));
  m_as.  Str  (reg0, base[offset + TVOFF(m_data)]);
  m_as.  Strb (reg1.W(), base[offset + TVOFF(m_type)]);
}

void CodeGenerator::emitLoad(Type type, PhysLoc dstLoc,
                             vixl::Register base,
                             ptrdiff_t offset,
                             Block* label /* = nullptr */) {
  if (type.needsReg()) {
    return emitLoadTypedValue(dstLoc, base, offset, label);
  }
  if (label) {
    not_implemented();
  }
  if (type <= Type::Null) return;

  auto dstReg = x2a(dstLoc.reg());
  if (!dstReg.IsValid()) return;

  m_as.  Ldr  (dstReg, base[offset + TVOFF(m_data)]);
}

void CodeGenerator::emitStore(vixl::Register base,
                              ptrdiff_t offset,
                              SSATmp* src, PhysLoc srcLoc,
                              bool genStoreType /* = true */) {
  auto type = src->type();
  if (type.needsReg()) {
    return emitStoreTypedValue(base, offset, srcLoc);
  }
  if (genStoreType) {
    auto dt = type.toDataType();
    if (dt == KindOfUninit) {
      static_assert(KindOfUninit == 0, "zero register hack");
      m_as.  Strb  (vixl::wzr, base[offset + TVOFF(m_type)]);
    } else {
      m_as.  Mov   (rAsm, dt);
      m_as.  Strb  (rAsm.W(), base[offset + TVOFF(m_type)]);
    }
  }
  if (type <= Type::Null) {
    return;
  }
  if (src->isConst()) {
    int64_t val = 0;
    if (type <= (Type::Bool | Type::Int | Type::Dbl |
                 Type::Arr | Type::StaticStr | Type::Cls)) {
      val = src->rawVal();
    } else {
      not_reached();
    }
    m_as.    Mov  (rAsm, val);
    m_as.    Str  (rAsm, base[offset + TVOFF(m_data)]);
  } else {
    auto reg = x2a(srcLoc.reg());
    if (src->isA(Type::Bool)) {
      m_as.  Uxtb (reg.W(), reg.W());
    }
    m_as.    Str  (x2a(srcLoc.reg()), base[offset + TVOFF(m_data)]);
  }
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  auto baseReg = x2a(srcLoc(0).reg());
  auto offset = localOffset(inst->extra<LdLoc>()->locId);
  emitLoad(inst->dst()->type(), dstLoc(0), baseReg, offset);
}

void CodeGenerator::cgStLoc(IRInstruction* inst) {
  auto baseReg = x2a(srcLoc(0).reg());
  auto offset = localOffset(inst->extra<StLoc>()->locId);
  emitStore(baseReg, offset, inst->src(1), srcLoc(1), true /* store type */);
}

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->taken() == nullptr);
  auto srcReg = x2a(srcLoc(0).reg());
  auto offset = cellsToBytes(inst->extra<LdStack>()->offset);
  emitLoad(inst->dst()->type(), dstLoc(0), srcReg, offset);
}

void CodeGenerator::emitLdRaw(IRInstruction* inst, size_t extraOff) {
  auto destReg = x2a(dstLoc(0).reg());
  auto offset  = inst->extra<RawMemData>()->info().offset;
  auto src     = x2a(srcLoc(0).reg())[offset + extraOff];

  switch (inst->extra<RawMemData>()->info().size) {
    case sz::byte:  m_as.  Ldrb  (destReg.W(), src); break;
    case sz::dword: m_as.  Ldr   (destReg.W(), src); break;
    case sz::qword: m_as.  Ldr   (destReg, src); break;
    default:        not_implemented();
  }
}

void CodeGenerator::cgLdRaw(IRInstruction* inst) {
  emitLdRaw(inst, 0);
}

void CodeGenerator::cgLdContArRaw(IRInstruction* inst) {
  emitLdRaw(inst, -c_Continuation::getArOffset());
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  auto dstReg  = x2a(dstLoc(0).reg());
  auto baseReg = x2a(srcLoc(0).reg());
  auto offset  = inst->src(1)->intVal();
  m_as.  Ldr  (dstReg, baseReg[offset + AROFF(m_func)]);
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  auto dstReg = x2a(dstLoc(0).reg());
  auto const name = inst->extra<LdFuncCachedData>()->name;
  auto const ch = Unit::GetNamedEntity(name)->getFuncHandle();
  vixl::Label noLookup;

  if (!dstReg.IsValid()) {
    m_as.  Ldr  (rAsm, rVmTl[ch]);
    dstReg = rAsm;
  } else {
    m_as.  Ldr  (dstReg, rVmTl[ch]);
  }
  m_as.    Cbnz (dstReg, &noLookup);

  const Func* (*const func)(const StringData*) = lookupUnknownFunc;
  cgCallHelper(
    m_as,
    CppCall(func),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup().immPtr(inst->extra<LdFuncCached>()->name)
  );

  m_as.    bind (&noLookup);
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto const dstReg  = x2a(dstLoc(0).reg());
  auto const baseReg = x2a(srcLoc(0).reg());
  auto const offset  = cellsToBytes(inst->extra<LdStackAddr>()->offset);
  emitRegGetsRegPlusImm(m_as, dstReg, baseReg, offset);
}

void CodeGenerator::cgSpillStack(IRInstruction* inst) {
  // TODO(2966414): so much of this logic could be shared. The opcode itself
  // should probably be broken up.
  auto const spDeficit    = inst->src(1)->intVal();
  auto const spillVals    = inst->srcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const dstReg       = x2a(dstLoc(0).reg());
  auto const spReg        = x2a(srcLoc(0).reg());
  auto const spillCells   = spillValueCells(inst);

  int64_t adjustment = (spDeficit - spillCells) * sizeof(Cell);
  for (uint32_t i = 0; i < numSpillSrcs; ++i) {
    const int64_t offset = i * sizeof(Cell) + adjustment;
    emitStore(spReg, offset, spillVals[i], srcLoc(i + 2));
  }
  emitRegGetsRegPlusImm(m_as, dstReg, spReg, adjustment);
}

void CodeGenerator::cgInterpOneCommon(IRInstruction* inst) {
  auto pcOff = inst->extra<InterpOneData>()->bcOff;

  auto opc = *(curFunc()->unit()->at(pcOff));
  auto* interpOneHelper = interpOneEntryPoints[opc];

  cgCallHelper(m_as,
               CppCall(interpOneHelper),
               callDest(InvalidReg),
               SyncOptions::kSyncPoint,
               argGroup().ssa(1/*fp*/).ssa(0/*sp*/).imm(pcOff));
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  auto const& extra = *inst->extra<InterpOne>();
  auto newSpReg = x2a(dstLoc(0).reg());

  auto spAdjustBytes = cellsToBytes(extra.cellsPopped - extra.cellsPushed);
  emitRegGetsRegPlusImm(m_as, newSpReg, newSpReg, spAdjustBytes);
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  m_as.   Ldr   (rVmFp, rReturnReg[offsetof(ExecutionContext, m_fp)]);
  m_as.   Ldr   (rVmSp, rReturnReg[offsetof(ExecutionContext, m_stack) +
                                   Stack::topOfStackOffset()]);

  emitServiceReq(mcg->code.main(), REQ_RESUME);
}

//////////////////////////////////////////////////////////////////////

Address CodeGenerator::cgInst(IRInstruction* inst) {
  Opcode opc = inst->op();
  auto const start = m_as.frontier();
  m_curInst = inst;
  SCOPE_EXIT { m_curInst = nullptr; };

  switch (opc) {
#define O(name, dsts, srcs, flags)                                \
  case name: FTRACE(7, "cg" #name "\n");                          \
             cg ## name (inst);                                   \
             return m_as.frontier() == start ? nullptr : start;
    IR_OPCODES
#undef O
  default:
    assert(0);
    return nullptr;
  }
}

}}}
