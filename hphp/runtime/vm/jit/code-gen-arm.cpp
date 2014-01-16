/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "folly/Optional.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
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
NOOP_OPCODE(OverrideLocVal)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(AssertStkVal)
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

CALL_OPCODE(ConvIntToStr)

//////////////////////////////////////////////////////////////////////

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

PUNT_OPCODE(CheckType)
PUNT_OPCODE(CheckTypeMem)
PUNT_OPCODE(CheckStk)
PUNT_OPCODE(CheckLoc)
PUNT_OPCODE(CastStk)
PUNT_OPCODE(CoerceStk)
PUNT_OPCODE(CheckDefinedClsEq)
PUNT_OPCODE(TryEndCatch)
PUNT_OPCODE(LdUnwinderValue)
PUNT_OPCODE(DeleteUnwinderException)
PUNT_OPCODE(Add)
PUNT_OPCODE(Sub)
PUNT_OPCODE(Mul)
PUNT_OPCODE(DivDbl)
PUNT_OPCODE(Mod)
PUNT_OPCODE(Sqrt)
PUNT_OPCODE(AbsInt)
PUNT_OPCODE(AbsDbl)
PUNT_OPCODE(BitAnd)
PUNT_OPCODE(BitOr)
PUNT_OPCODE(BitXor)
PUNT_OPCODE(BitNot)
PUNT_OPCODE(LogicXor)
PUNT_OPCODE(Not)
PUNT_OPCODE(Shl)
PUNT_OPCODE(Shr)
PUNT_OPCODE(ConvBoolToArr)
PUNT_OPCODE(ConvDblToArr)
PUNT_OPCODE(ConvIntToArr)
PUNT_OPCODE(ConvObjToArr)
PUNT_OPCODE(ConvStrToArr)
PUNT_OPCODE(ConvCellToArr)
PUNT_OPCODE(ConvArrToBool)
PUNT_OPCODE(ConvDblToBool)
PUNT_OPCODE(ConvIntToBool)
PUNT_OPCODE(ConvStrToBool)
PUNT_OPCODE(ConvObjToBool)
PUNT_OPCODE(ConvCellToBool)
PUNT_OPCODE(ConvArrToDbl)
PUNT_OPCODE(ConvBoolToDbl)
PUNT_OPCODE(ConvIntToDbl)
PUNT_OPCODE(ConvObjToDbl)
PUNT_OPCODE(ConvStrToDbl)
PUNT_OPCODE(ConvCellToDbl)
PUNT_OPCODE(ConvArrToInt)
PUNT_OPCODE(ConvBoolToInt)
PUNT_OPCODE(ConvDblToInt)
PUNT_OPCODE(ConvObjToInt)
PUNT_OPCODE(ConvStrToInt)
PUNT_OPCODE(ConvCellToInt)
PUNT_OPCODE(ConvCellToObj)
PUNT_OPCODE(ConvBoolToStr)
PUNT_OPCODE(ConvDblToStr)
PUNT_OPCODE(ConvObjToStr)
PUNT_OPCODE(ConvResToStr)
PUNT_OPCODE(ConvCellToStr)
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
PUNT_OPCODE(GtX)
PUNT_OPCODE(Gte)
PUNT_OPCODE(GteX)
PUNT_OPCODE(Lt)
PUNT_OPCODE(LtX)
PUNT_OPCODE(Lte)
PUNT_OPCODE(LteX)
PUNT_OPCODE(Eq)
PUNT_OPCODE(EqX)
PUNT_OPCODE(Neq)
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
PUNT_OPCODE(JmpSame)
PUNT_OPCODE(JmpNSame)
PUNT_OPCODE(JmpInstanceOfBitmask)
PUNT_OPCODE(JmpNInstanceOfBitmask)
PUNT_OPCODE(JmpIsType)
PUNT_OPCODE(JmpIsNType)
PUNT_OPCODE(JmpZero)
PUNT_OPCODE(JmpNZero)
PUNT_OPCODE(Jmp)
PUNT_OPCODE(ReqBindJmpGt)
PUNT_OPCODE(ReqBindJmpGte)
PUNT_OPCODE(ReqBindJmpLt)
PUNT_OPCODE(ReqBindJmpLte)
PUNT_OPCODE(ReqBindJmpEq)
PUNT_OPCODE(ReqBindJmpNeq)
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
PUNT_OPCODE(RaiseError)
PUNT_OPCODE(RaiseWarning)
PUNT_OPCODE(RaiseNotice)
PUNT_OPCODE(RaiseArrayIndexNotice)
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
PUNT_OPCODE(Box)
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
PUNT_OPCODE(LookupCns)
PUNT_OPCODE(LookupCnsE)
PUNT_OPCODE(LookupCnsU)
PUNT_OPCODE(LookupClsMethod)
PUNT_OPCODE(LookupClsRDSHandle)
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
PUNT_OPCODE(LdSwitchDblIndex)
PUNT_OPCODE(LdSwitchStrIndex)
PUNT_OPCODE(LdSwitchObjIndex)
PUNT_OPCODE(JmpSwitchDest)
PUNT_OPCODE(AllocObj)
PUNT_OPCODE(AllocObjFast)
PUNT_OPCODE(LdClsCtor)
PUNT_OPCODE(LdArrFuncCtx)
PUNT_OPCODE(LdArrFPushCuf)
PUNT_OPCODE(LdStrFPushCuf)
PUNT_OPCODE(StClosureFunc)
PUNT_OPCODE(StClosureArg)
PUNT_OPCODE(StClosureCtx)
PUNT_OPCODE(NewArray)
PUNT_OPCODE(NewPackedArray)
PUNT_OPCODE(NewStructArray)
PUNT_OPCODE(NewCol)
PUNT_OPCODE(Clone)
PUNT_OPCODE(FreeActRec)
PUNT_OPCODE(Call)
PUNT_OPCODE(CallArray)
PUNT_OPCODE(CallBuiltin)
PUNT_OPCODE(NativeImpl)
PUNT_OPCODE(RetCtrl)
PUNT_OPCODE(StRetVal)
PUNT_OPCODE(RetAdjustStack)
PUNT_OPCODE(StMem)
PUNT_OPCODE(StMemNT)
PUNT_OPCODE(StProp)
PUNT_OPCODE(StPropNT)
PUNT_OPCODE(StLoc)
PUNT_OPCODE(StLocNT)
PUNT_OPCODE(StRef)
PUNT_OPCODE(StRefNT)
PUNT_OPCODE(StRaw)
PUNT_OPCODE(StElem)
PUNT_OPCODE(IterCopy)
PUNT_OPCODE(LdStaticLocCached)
PUNT_OPCODE(CheckStaticLocInit)
PUNT_OPCODE(ClosureStaticLocInit)
PUNT_OPCODE(StaticLocInitCached)
PUNT_OPCODE(SpillFrame)
PUNT_OPCODE(CufIterSpillFrame)
PUNT_OPCODE(ReqRetranslateOpt)
PUNT_OPCODE(ReqRetranslate)
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
PUNT_OPCODE(VerifyParamCallable)
PUNT_OPCODE(VerifyParamFail)
PUNT_OPCODE(RaiseUninitLoc)
PUNT_OPCODE(WarnNonObjProp)
PUNT_OPCODE(ThrowNonObjProp)
PUNT_OPCODE(RaiseUndefProp)
PUNT_OPCODE(PrintStr)
PUNT_OPCODE(PrintInt)
PUNT_OPCODE(PrintBool)
PUNT_OPCODE(AddElemStrKey)
PUNT_OPCODE(AddElemIntKey)
PUNT_OPCODE(AddNewElem)
PUNT_OPCODE(ColAddElemC)
PUNT_OPCODE(ColAddNewElemC)
PUNT_OPCODE(ConcatStrStr)
PUNT_OPCODE(ConcatIntStr)
PUNT_OPCODE(ConcatStrInt)
PUNT_OPCODE(ConcatCellCell)
PUNT_OPCODE(ArrayAdd)
PUNT_OPCODE(AKExists)
PUNT_OPCODE(Spill)
PUNT_OPCODE(Reload)
PUNT_OPCODE(CreateContFunc)
PUNT_OPCODE(CreateContMeth)
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
PUNT_OPCODE(CreateAFWHFunc)
PUNT_OPCODE(CreateAFWHMeth)
PUNT_OPCODE(CreateSRWH)
PUNT_OPCODE(CreateSEWH)
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
PUNT_OPCODE(StableMapGet)
PUNT_OPCODE(CGetElem)
PUNT_OPCODE(VGetElem)
PUNT_OPCODE(VGetElemStk)
PUNT_OPCODE(BindElem)
PUNT_OPCODE(BindElemStk)
PUNT_OPCODE(ArraySet)
PUNT_OPCODE(MapSet)
PUNT_OPCODE(StableMapSet)
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
PUNT_OPCODE(StableMapIsset)
PUNT_OPCODE(IssetElem)
PUNT_OPCODE(EmptyElem)
PUNT_OPCODE(IncStat)
PUNT_OPCODE(TypeProfileFunc)
PUNT_OPCODE(IncStatGrouped)
PUNT_OPCODE(RBTrace)
PUNT_OPCODE(IncTransCounter)
PUNT_OPCODE(IncProfCounter)
PUNT_OPCODE(ArrayIdx)
PUNT_OPCODE(GenericIdx)
PUNT_OPCODE(DbgAssertType)

#undef PUNT_OPCODE

//////////////////////////////////////////////////////////////////////

void CodeGenerator::recordHostCallSyncPoint(vixl::MacroAssembler& as,
                                            TCA tca) {
  auto stackOff = m_curInst->marker().spOff;
  auto pcOff = m_curInst->marker().bcOff - m_curInst->marker().func->base();
  m_tx64->fixupMap().recordSyncPoint(tca, pcOff, stackOff);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  vixl::Label done;
  auto base = x2a(curOpd(inst->src(0)).reg());
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
  Type type = src->type();

  if (type.notCounted()) return;

  auto increfMaybeStatic = [&] {
    auto base = x2a(curOpd(src).reg(0));
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
    m_as.    Cmp (x2a(curOpd(src).reg(1)).W(), KindOfRefCountThreshold);
    ifThen(m_as, vixl::gt, [&] { increfMaybeStatic(); });
  }
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  auto const& srcRegs = curOpd(inst->src(0));
  auto const& dstRegs = curOpd(inst->dst());

  PhysReg::Map<PhysReg> moves;
  if (dstRegs.reg(0) != InvalidReg)
    moves[dstRegs.reg(0)] = srcRegs.reg(0);
  if (dstRegs.reg(1) != InvalidReg)
    moves[dstRegs.reg(1)] = srcRegs.reg(1);

  auto howTo = doRegMoves(moves, rAsm);
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      m_as.  Mov  (x2a(how.m_reg2), x2a(how.m_reg1));
    } else {
      emitXorSwap(m_as, x2a(how.m_reg2), x2a(how.m_reg1));
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
               TranslatorX64::getDtorCall(type.toDataType()),
               kVoidDest,
               SyncOptions::kSyncPoint,
               ArgGroup(curOpds()).reg(dataReg));

  m_as.  bind (&allDone);
}

void CodeGenerator::emitDecRefDynamicType(vixl::Register baseReg,
                                          ptrdiff_t offset) {
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
               ArgGroup(curOpds()).addr(baseReg, offset));

  m_as.  bind (&allDone);
}

void CodeGenerator::emitDecRefMem(Type type,
                                  vixl::Register baseReg,
                                  ptrdiff_t offset) {
  if (type.needsReg()) {
    emitDecRefDynamicType(baseReg, offset);
  } else if (type.maybeCounted()) {
    m_as.  Ldr  (rAsm, baseReg[offset + TVOFF(m_data)]);
    emitDecRefStaticType(type, rAsm);
  }
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(curOpd(inst->src(0)).reg()),
                cellsToBytes(inst->extra<DecRefStack>()->offset));
}

void CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(curOpd(inst->src(0)).reg()),
                localOffset(inst->extra<DecRefLoc>()->locId));
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  emitDecRefMem(inst->typeParam(),
                x2a(curOpd(inst->src(0)).reg()),
                inst->src(1)->getValInt());
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgShuffle(IRInstruction* inst) {
  PhysReg::Map<PhysReg> moves;

  // Put required reg-reg moves in the map, and do spills at the same time.
  for (auto i = 0; i < inst->numSrcs(); ++i) {
    auto src = inst->src(i);
    auto& rd = inst->extra<Shuffle>()->dests[i];
    auto& rs = curOpd(src);

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
      emitRegGetsRegPlusImm(m_as, x2a(how.m_reg2), x2a(how.m_reg1), 0);
    } else {
      emitXorSwap(m_as, x2a(how.m_reg1), x2a(how.m_reg2));
    }
  }

  // Now do reloads and reg<-imm.
  for (auto i = 0; i < inst->numSrcs(); ++i) {
    auto src = inst->src(i);
    auto& rd = inst->extra<Shuffle>()->dests[i];
    auto& rs = curOpd(src);
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
      m_as.  Mov  (x2a(rd.reg(0)), src->getValBits());
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
    auto srcReg = x2a(how.m_reg1);
    auto dstReg = x2a(how.m_reg2);
    if (how.m_kind == MoveInfo::Kind::Move) {
      auto* argDesc = argDescs[how.m_reg2];
      if (argDesc) {
        auto kind = argDesc->kind();
        if (kind == ArgDesc::Kind::Addr) {
          emitRegGetsRegPlusImm(a, dstReg, srcReg, argDesc->imm().q());
        } else {
          if (argDesc->isZeroExtend()) {
            // "Unsigned eXTend Byte"
            a.Uxtb (dstReg, srcReg.W());
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
  ArgGroup argGroup = info.toArgGroup(curOpds(), inst);

  auto call = [&]() -> CppCall {
    switch (info.func.type) {
    case FuncType::Call:
      return CppCall(info.func.call);
    case FuncType::SSA:
      return CppCall(inst->src(info.func.srcIdx)->getValTCA());
    }
    not_reached();
  }();

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:  return kVoidDest;
      case DestType::TV:    return callDestTV(inst->dst(0));
      case DestType::SSA:   return callDest(inst->dst(0));
      case DestType::SSA2:  return callDest2(inst->dst(0));
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
 * XXX copypasta but has to be in the class because of curOpd and changing that
 * would make callsites real messy
 */

CallDest CodeGenerator::callDest(PhysReg reg0,
                                 PhysReg reg1 /* = InvalidReg */) const {
  return { DestType::SSA, reg0, reg1 };
}

CallDest CodeGenerator::callDest(SSATmp* ssa) const {
  if (!ssa) return kVoidDest;
  return { DestType::SSA, curOpd(ssa).reg(0), curOpd(ssa).reg(1) };
}

CallDest CodeGenerator::callDestTV(SSATmp* ssa) const {
  if (!ssa) return kVoidDest;
  return { DestType::TV, curOpd(ssa).reg(0), curOpd(ssa).reg(1) };
}

CallDest CodeGenerator::callDest2(SSATmp* ssa) const {
  if (!ssa) return kVoidDest;
  return { DestType::SSA2, curOpd(ssa).reg(0), curOpd(ssa).reg(1) };
}

//////////////////////////////////////////////////////////////////////

template<class Loc, class JmpFn>
void CodeGenerator::emitTypeTest(Type type, Loc typeSrc, Loc dataSrc,
                                 JmpFn doJcc) {
  assert(!(type <= Type::Cls));

  if (type.equals(Type::Gen)) {
    return;
  }

  // You can't compare against memory. Load the type into scratch.
  m_as.     Ldrb  (rAsm.W(), typeSrc);

  ConditionCode cc;
  if (type.isString()) {
    // Note: ARM can actually do better here; it has a fused test-and-branch
    // instruction. The way this code is factored makes it difficult to use,
    // though; the jump instruction will be written by some other code.
    m_as.   Tst   (rAsm.W(), KindOfStringBit);
    cc = CC_NE;
  } else if (type.equals(Type::UncountedInit)) {
    m_as.   Tst   (rAsm.W(), KindOfUncountedInitBit);
    cc = CC_NE;
  } else if (type.equals(Type::Uncounted)) {
    m_as.   Cmp   (rAsm.W(), KindOfRefCountThreshold);
    cc = CC_LE;
  } else if (type.equals(Type::Cell)) {
    m_as.   Cmp   (rAsm.W(), KindOfRef);
    cc = CC_L;
  } else {
    assert(type.isKnownDataType());
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfResource));
    m_as.   Cmp   (rAsm.W(), dataType);
    cc = CC_E;
  }
  doJcc(cc);
  if (type < Type::Obj) {
    assert(type.getClass()->attrs() & AttrFinal);
    m_as.   Ldr   (rAsm, dataSrc);
    m_as.   Ldr   (rAsm, rAsm[ObjectData::getVMClassOffset()]);
    m_as.   Cmp   (rAsm, reinterpret_cast<int64_t>(type.getClass()));
    doJcc(CC_E);
  } else if (type < Type::Res) {
    CG_PUNT(TypeTest-on-Resource);
  } else if (type <= Type::Arr && type.hasArrayKind()) {
    m_as.   Ldr   (rAsm, dataSrc);
    m_as.   Ldrb  (rAsm.W(), rAsm[ArrayData::offsetofKind()]);
    m_as.   Cmp   (rAsm.W(), type.getArrayKind());
    doJcc(CC_E);
  }
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  auto const rFP = x2a(curOpd(inst->src(0)).reg());
  auto const baseOff = localOffset(inst->extra<GuardLoc>()->locId);
  emitTypeTest(
    inst->typeParam(),
    rFP[baseOff + TVOFF(m_type)],
    rFP[baseOff + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
      auto const destSR = m_tx64->getSrcRec(destSK);
      destSR->emitFallbackJump(this->m_mainCode, ccNegate(cc));
    });
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  auto const rSP = x2a(curOpd(inst->src(0)).reg());
  auto const baseOff = cellsToBytes(inst->extra<GuardStk>()->offset);
  emitTypeTest(
    inst->typeParam(),
    rSP[baseOff + TVOFF(m_type)],
    rSP[baseOff + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
      auto const destSR = m_tx64->getSrcRec(destSK);
      destSR->emitFallbackJump(this->m_mainCode, ccNegate(cc));
    });
}

void CodeGenerator::cgSideExitGuardStk(IRInstruction* inst) {
  auto const sp = x2a(curOpd(inst->src(0)).reg());
  auto const extra = inst->extra<SideExitGuardStk>();

  emitTypeTest(
    inst->typeParam(),
    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_type)],
    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_data)],
    [&] (ConditionCode cc) {
      auto const sk = SrcKey(curFunc(), extra->taken);
      emitBindSideExit(this->m_mainCode, this->m_stubsCode, sk, ccNegate(cc));
    }
  );
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  assert(inst->numSrcs() == 5);

  SSATmp* funcPtrTmp = inst->src(0);
  SSATmp* nParamsTmp = inst->src(1);
  SSATmp* firstBitNumTmp = inst->src(2);
  SSATmp* mask64Tmp  = inst->src(3);
  SSATmp* vals64Tmp  = inst->src(4);

  // Get values in place
  assert(funcPtrTmp->type() == Type::Func);
  auto funcPtrReg = x2a(curOpd(funcPtrTmp).reg());
  assert(funcPtrReg.IsValid());

  assert(nParamsTmp->type() == Type::Int);
  auto nParamsReg = x2a(curOpd(nParamsTmp).reg());
  assert(nParamsReg.IsValid() || nParamsTmp->isConst());

  assert(firstBitNumTmp->isConst() && firstBitNumTmp->type() == Type::Int);
  uint32_t firstBitNum = (uint32_t)(firstBitNumTmp->getValInt());

  assert(mask64Tmp->type() == Type::Int);
  assert(mask64Tmp->isConst());
  auto mask64Reg = x2a(curOpd(mask64Tmp).reg());
  assert(mask64Reg.IsValid() || mask64Tmp->inst()->op() != LdConst);
  uint64_t mask64 = mask64Tmp->getValInt();
  assert(mask64);

  assert(vals64Tmp->type() == Type::Int);
  assert(vals64Tmp->isConst());
  auto vals64Reg = x2a(curOpd(vals64Tmp).reg());
  assert(vals64Reg.IsValid() || vals64Tmp->inst()->op() != LdConst);
  uint64_t vals64 = vals64Tmp->getValInt();
  assert((vals64 & mask64) == vals64);

  auto const destSK = SrcKey(curFunc(), m_unit.bcOff());
  auto const destSR = m_tx64->getSrcRec(destSK);

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
    // immediate in ARM's logical instructions, and if they're not met, we'll
    // have to use a register.
    if (vixl::Assembler::IsImmLogical(mask64, vixl::kXRegSize)) {
      m_as.  And  (bitsReg, bitsReg, mask64);
    } else {
      if (mask64Reg.IsValid()) {
        m_as.And  (bitsReg, bitsReg, mask64Reg);
      } else {
        m_as.Mov  (rAsm2, mask64);
        m_as.And  (bitsReg, bitsReg, rAsm2);
      }
    }

    // Now do the compare. There are also restrictions on immediates in
    // arithmetic instructions (of which Cmp is one; it's just a subtract that
    // sets flags), so same deal as with the mask immediate above.
    if (vixl::Assembler::IsImmArithmetic(vals64)) {
      m_as.  Cmp  (bitsReg, vals64);
    } else {
      if (vals64Reg.IsValid()) {
        m_as.Cmp  (bitsReg, vals64Reg);
      } else {
        m_as.Mov  (rAsm2, vals64);
        m_as.Cmp  (bitsReg, rAsm2);
      }
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
  emitRegGetsRegPlusImm(m_as, rVmFp, x2a(curOpd(inst->src(0)).reg()), 0);
  emitRegGetsRegPlusImm(m_as, rVmSp, x2a(curOpd(inst->src(1)).reg()), 0);
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  emitBindJmp(
    m_mainCode,
    m_stubsCode,
    SrcKey(curFunc(), inst->extra<ReqBindJmp>()->offset)
  );
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  m_as.  Brk  (0);
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  m_as.  Brk  (0);
}

//////////////////////////////////////////////////////////////////////

void CodeGenerator::emitLoadTypedValue(SSATmp* dst,
                                       vixl::Register base,
                                       ptrdiff_t offset,
                                       Block* label) {
  auto valueDstReg = x2a(curOpd(dst).reg(0));
  auto typeDstReg  = x2a(curOpd(dst).reg(1));

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
                                        SSATmp* src) {
  assert(src->type().needsReg());
  auto reg0 = x2a(curOpd(src).reg(0));
  auto reg1 = x2a(curOpd(src).reg(1));
  m_as.  Str  (reg0, base[offset + TVOFF(m_data)]);
  m_as.  Strb (reg1.W(), base[offset + TVOFF(m_type)]);
}

void CodeGenerator::emitLoad(SSATmp* dst,
                             vixl::Register base,
                             ptrdiff_t offset,
                             Block* label /* = nullptr */) {
  auto type = dst->type();
  if (type.needsReg()) {
    return emitLoadTypedValue(dst, base, offset, label);
  }
  if (label) {
    not_implemented();
  }
  if (type.isNull()) return;

  auto dstReg = x2a(curOpd(dst).reg());
  if (!dstReg.IsValid()) return;

  m_as.  Ldr  (dstReg, base[offset + TVOFF(m_data)]);
}

void CodeGenerator::emitStore(vixl::Register base,
                              ptrdiff_t offset,
                              SSATmp* src,
                              bool genStoreType /* = true */) {
  Type type = src->type();
  if (type.needsReg()) {
    return emitStoreTypedValue(base, offset, src);
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
  if (type.isNull()) {
    return;
  }
  if (src->isConst()) {
    int64_t val = 0;
    if (type <= (Type::Bool | Type::Int | Type::Dbl |
                 Type::Arr | Type::StaticStr | Type::Cls)) {
      val = src->getValBits();
    } else {
      not_reached();
    }
    m_as.    Mov  (rAsm, val);
    m_as.    Str  (rAsm, base[offset + TVOFF(m_data)]);
  } else {
    m_as.    Str  (x2a(curOpd(src).reg()), base[offset + TVOFF(m_data)]);
  }
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  auto baseReg = x2a(curOpd(inst->src(0)).reg());
  auto offset = localOffset(inst->extra<LdLoc>()->locId);
  emitLoad(inst->dst(), baseReg, offset);
}

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->taken() == nullptr);
  auto srcReg = x2a(curOpd(inst->src(0)).reg());
  auto offset = cellsToBytes(inst->extra<LdStack>()->offset);
  emitLoad(inst->dst(), srcReg, offset);
}

void CodeGenerator::cgLdConst(IRInstruction* inst) {
  auto const dstReg = x2a(curOpd(inst->dst()).reg());
  auto const val    = inst->extra<LdConst>()->as<uintptr_t>();
  if (dstReg.IsValid()) {
    m_as.  Mov  (dstReg, val);
  }
}

void CodeGenerator::cgLdRaw(IRInstruction* inst) {
  auto* addr   = inst->src(0);
  auto* offset = inst->src(1);

  auto destReg = x2a(curOpd(inst->dst()).reg());
  auto addrReg = x2a(curOpd(addr).reg());

  if (addr->isConst()) {
    not_implemented();
  }

  if (offset->isConst()) {
    auto kind   = offset->getValInt();
    auto& slot  = RawMemSlot::Get(RawMemSlot::Kind(kind));
    auto ldSize = slot.size();
    auto offs   = slot.offset();

    switch (ldSize) {
      case sz::qword:
        m_as.  Ldr  (destReg, addrReg[offs]);
        break;
      case sz::dword:
        m_as.  Ldr  (destReg.W(), addrReg[offs]);
        break;
      case sz::byte:
        // Ldrb zero-extends
        m_as.  Ldrb (destReg.W(), addrReg[offs]);
        break;
      default: not_reached();
    }
  } else {
    auto offsetReg = x2a(curOpd(offset).reg());
    assert(inst->dst()->type().nativeSize() == sz::qword);
    m_as.  Ldr  (destReg, addrReg[offsetReg]);
  }
}

void CodeGenerator::cgLdContArRaw(IRInstruction* inst) {
  auto destReg     = x2a(curOpd(inst->dst()).reg());
  auto contArReg   = x2a(curOpd(inst->src(0)).reg());
  auto kind        = inst->src(1)->getValInt();
  auto const& slot = RawMemSlot::Get(RawMemSlot::Kind(kind));

  auto off = slot.offset() - c_Continuation::getArOffset();
  switch (slot.size()) {
    case sz::byte:  m_as.  Ldrb  (destReg.W(), contArReg[off]); break;
    case sz::dword: m_as.  Ldr   (destReg.W(), contArReg[off]); break;
    case sz::qword: m_as.  Ldr   (destReg, contArReg[off]); break;
    default:        not_implemented();
  }
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  auto dstReg  = x2a(curOpd(inst->dst()).reg());
  auto baseReg = x2a(curOpd(inst->src(0)).reg());
  auto offset  = inst->src(1)->getValInt();
  m_as.  Ldr  (dstReg, baseReg[offset + AROFF(m_func)]);
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  auto dstReg = x2a(curOpd(inst->dst()).reg());
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
    callDest(inst->dst()),
    SyncOptions::kSyncPoint,
    ArgGroup(curOpds())
      .immPtr(inst->extra<LdFuncCached>()->name)
  );

  m_as.    bind (&noLookup);
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto const dstReg  = x2a(curOpd(inst->dst()).reg());
  auto const baseReg = x2a(curOpd(inst->src(0)).reg());
  auto const offset  = cellsToBytes(inst->extra<LdStackAddr>()->offset);
  emitRegGetsRegPlusImm(m_as, dstReg, baseReg, offset);
}

void CodeGenerator::cgSpillStack(IRInstruction* inst) {
  // TODO(2966414): so much of this logic could be shared. The opcode itself
  // should probably be broken up.
  SSATmp* dst             = inst->dst();
  SSATmp* sp              = inst->src(0);
  auto const spDeficit    = inst->src(1)->getValInt();
  auto const spillVals    = inst->srcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const dstReg       = x2a(curOpd(dst).reg());
  auto const spReg        = x2a(curOpd(sp).reg());
  auto const spillCells   = spillValueCells(inst);

  int64_t adjustment = (spDeficit - spillCells) * sizeof(Cell);
  for (uint32_t i = 0; i < numSpillSrcs; ++i) {
    const int64_t offset = i * sizeof(Cell) + adjustment;
    auto* val = spillVals[i];
    if (val->type() == Type::None) {
      // The simplifier detected that this store was redundnant.
      continue;
    }
    emitStore(spReg, offset, val);
  }
  emitRegGetsRegPlusImm(m_as, dstReg, spReg, adjustment);
}

void CodeGenerator::cgInterpOneCommon(IRInstruction* inst) {
  auto sp = inst->src(0);
  auto fp = inst->src(1);
  auto pcOff = inst->extra<InterpOneData>()->bcOff;

  auto opc = *(curFunc()->unit()->at(pcOff));
  auto* interpOneHelper = interpOneEntryPoints[opc];

  cgCallHelper(m_as,
               CppCall(interpOneHelper),
               callDest(InvalidReg),
               SyncOptions::kSyncPoint,
               ArgGroup(curOpds()).ssa(fp).ssa(sp).imm(pcOff));
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  auto const& extra = *inst->extra<InterpOne>();
  auto newSpReg = x2a(curOpd(inst->dst()).reg());

  auto spAdjustBytes = cellsToBytes(extra.cellsPopped - extra.cellsPushed);
  emitRegGetsRegPlusImm(m_as, newSpReg, newSpReg, spAdjustBytes);
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  m_as.   Ldr   (rVmFp, rReturnReg[offsetof(VMExecutionContext, m_fp)]);
  m_as.   Ldr   (rVmSp, rReturnReg[offsetof(VMExecutionContext, m_stack) +
                                   Stack::topOfStackOffset()]);

  emitServiceReq(tx64->code.main(), REQ_RESUME);
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

void CodeGenerator::cgBlock(Block* block, std::vector<TransBCMapping>* bcMap) {
  FTRACE(6, "cgBlock: {}\n", block->id());

  BCMarker prevMarker;
  for (IRInstruction& instr : *block) {
    IRInstruction* inst = &instr;
    // If we're on the first instruction of the block or we have a new
    // marker since the last instruction, update the bc mapping.
    if ((!prevMarker.valid() || inst->marker() != prevMarker) &&
        m_tx64->isTransDBEnabled() && bcMap) {
      bcMap->push_back(TransBCMapping{inst->marker().func->unit()->md5(),
                                      inst->marker().bcOff,
                                      m_as.frontier(),
                                      m_astubs.frontier()});
      prevMarker = inst->marker();
    }
    auto* addr = cgInst(inst);
    if (m_state.asmInfo && addr) {
      m_state.asmInfo->updateForInstruction(inst, addr, m_as.frontier());
    }
  }
}

}}}
