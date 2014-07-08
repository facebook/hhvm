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

#include "hphp/runtime/vm/jit/code-gen-x64.h"

#include <cstring>
#include <unwind.h>
#include <limits>
#include <vector>

#include "folly/ScopeGuard.h"
#include "folly/Format.h"
#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/util/abi-cxx.h"

#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/layout.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/target-profile.h"

using HPHP::JIT::TCA;

namespace HPHP { namespace JIT { namespace X64 {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

using namespace JIT::reg;

/*
 * It's not normally ok to directly use tracelet abi registers in
 * codegen, unless you're directly dealing with an instruction that
 * does near-end-of-tracelet glue.  (Or also we sometimes use them
 * just for some static_assertions relating to calls to helpers from
 * mcg that hardcode these registers.)
 */

void cgPunt(const char* file, int line, const char* func, uint32_t bcOff,
            const Func* vmFunc, bool resumed, TransID profTransId) {
  if (dumpIREnabled()) {
    HPHP::Trace::trace("--------- CG_PUNT %s %d %s  bcOff: %d \n",
                       file, line, func, bcOff);
  }
  throw FailedCodeGen(file, line, func, bcOff, vmFunc, resumed, profTransId);
}

#define CG_PUNT(instr)                                            \
  cgPunt(__FILE__, __LINE__, #instr, m_curInst->marker().bcOff(), \
         curFunc(), resumed(), m_curInst->marker().profTransId())

const char* getContextName(const Class* ctx) {
  return ctx ? ctx->name()->data() : ":anonymous:";
}

} // unnamed namespace
//////////////////////////////////////////////////////////////////////

/*
 * Select a scratch register to use in the given instruction, prefering the
 * lower registers which don't require a REX prefix.  The selected register
 * must not be any of the instructions inputs or outputs, and neither a register
 * that is alive across this instruction.
 */
PhysReg CodeGenerator::selectScratchReg(IRInstruction* inst) {
  // TODO: #3634984 this function must die.
  static const RegSet kLowGPRegs = RegSet()
    | RegSet(reg::rax)
    | RegSet(reg::rcx)
    | RegSet(reg::rdx)
    | RegSet(reg::rsi)
    | RegSet(reg::rdi)
    ;
  RegSet liveRegs = m_state.liveRegs[inst];
  liveRegs |= m_state.regs.srcRegs(*inst);
  liveRegs |= m_state.regs.dstRegs(*inst);
  PhysReg selectedReg;
  if ((kLowGPRegs - liveRegs).findFirst(selectedReg)) {
    return selectedReg;
  }
  return rCgGP;
}

void CodeGenerator::cgInst(IRInstruction* inst) {
  Opcode opc = inst->op();
  m_curInst = inst;
  m_instRegs = &m_state.regs[inst];
  m_rScratch = selectScratchReg(inst);
  SCOPE_EXIT { m_curInst = nullptr; };

  switch (opc) {
#define O(name, dsts, srcs, flags)                                \
  case name: FTRACE(7, "cg" #name "\n");                          \
             cg ## name (inst);                                   \
             return;
  IR_OPCODES
#undef O

  default:
    always_assert(false);
  }
}

#define NOOP_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction*) {}

#define CALL_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction* i) { cgCallNative(m_as, i); }

#define CALL_STK_OPCODE(opcode) \
  CALL_OPCODE(opcode)           \
  CALL_OPCODE(opcode ## Stk)

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
NOOP_OPCODE(EndGuards)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(Box)
CALL_OPCODE(ColAddElemC)
CALL_OPCODE(ColAddNewElemC)

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

CALL_OPCODE(TypeProfileFunc)
CALL_OPCODE(CreateCont)
CALL_OPCODE(CreateAFWH)
CALL_OPCODE(CreateSSWH)
CALL_OPCODE(AFWHPrepareChild)
CALL_OPCODE(BWHUnblockChain)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(NewPackedArray)
CALL_OPCODE(NewCol)
CALL_OPCODE(Clone)
CALL_OPCODE(AllocObj)
CALL_OPCODE(CustomInstanceInit)
CALL_OPCODE(InitProps)
CALL_OPCODE(InitSProps)
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
CALL_OPCODE(WarnNonObjProp)
CALL_OPCODE(ThrowNonObjProp)
CALL_OPCODE(RaiseUndefProp)
CALL_OPCODE(RaiseError)
CALL_OPCODE(RaiseWarning)
CALL_OPCODE(RaiseNotice)
CALL_OPCODE(RaiseArrayIndexNotice)
CALL_OPCODE(IncStatGrouped)
CALL_OPCODE(ClosureStaticLocInit)
CALL_OPCODE(ArrayIdx)
CALL_OPCODE(GenericIdx)
CALL_OPCODE(LdClsPropAddrOrNull)
CALL_OPCODE(LdClsPropAddrOrRaise)
CALL_OPCODE(LdGblAddrDef)

// Vector instruction helpers
CALL_OPCODE(BaseG)
CALL_OPCODE(PropX)
CALL_STK_OPCODE(PropDX)
CALL_OPCODE(CGetProp)
CALL_STK_OPCODE(VGetProp)
CALL_STK_OPCODE(BindProp)
CALL_STK_OPCODE(SetProp)
CALL_OPCODE(UnsetProp)
CALL_STK_OPCODE(SetOpProp)
CALL_STK_OPCODE(IncDecProp)
CALL_OPCODE(EmptyProp)
CALL_OPCODE(IssetProp)
CALL_OPCODE(ElemX)
CALL_OPCODE(ElemArray)
CALL_STK_OPCODE(ElemDX)
CALL_STK_OPCODE(ElemUX)
CALL_OPCODE(ArrayGet)
CALL_OPCODE(StringGet)
CALL_OPCODE(MapGet)
CALL_OPCODE(CGetElem)
CALL_STK_OPCODE(VGetElem)
CALL_STK_OPCODE(BindElem)
CALL_STK_OPCODE(SetWithRefElem)
CALL_STK_OPCODE(SetWithRefNewElem)
CALL_OPCODE(ArraySet)
CALL_OPCODE(MapSet)
CALL_OPCODE(ArraySetRef)
CALL_STK_OPCODE(SetElem)
CALL_STK_OPCODE(UnsetElem)
CALL_STK_OPCODE(SetOpElem)
CALL_STK_OPCODE(IncDecElem)
CALL_STK_OPCODE(SetNewElem)
CALL_STK_OPCODE(SetNewElemArray)
CALL_STK_OPCODE(BindNewElem)
CALL_OPCODE(ArrayIsset)
CALL_OPCODE(VectorIsset)
CALL_OPCODE(PairIsset)
CALL_OPCODE(MapIsset)
CALL_OPCODE(IssetElem)
CALL_OPCODE(EmptyElem)

CALL_OPCODE(InstanceOfIface)
CALL_OPCODE(InterfaceSupportsArr)
CALL_OPCODE(InterfaceSupportsStr)
CALL_OPCODE(InterfaceSupportsInt)
CALL_OPCODE(InterfaceSupportsDbl)

CALL_OPCODE(ZeroErrorLevel)
CALL_OPCODE(RestoreErrorLevel)

CALL_OPCODE(Count)

CALL_OPCODE(SurpriseHook)
CALL_OPCODE(FunctionSuspendHook)
CALL_OPCODE(FunctionReturnHook)

CALL_OPCODE(OODeclExists)

#undef NOOP_OPCODE

// Thread chain of patch locations using the 4 byte space in each jmp/jcc
static void prependPatchAddr(CodegenState& state,
                             Block* block,
                             TCA patchAddr) {
  auto &patches = state.patches;
  ssize_t diff = patches[block] ? (patchAddr - (TCA)patches[block]) : 0;
  assert(deltaFits(diff, sz::dword));
  *(int32_t*)(patchAddr) = (int32_t)diff;
  patches[block] = patchAddr;
}

static void emitFwdJmp(Asm& a, Block* target, CodegenState& state) {
  if (auto addr = state.addresses[target]) {
    return a.jmpAuto(addr);
  }

  // TODO(#2101926): it'd be nice to get 1-byte forward jumps here
  a.jmp(a.frontier());
  TCA immPtr = a.frontier() - 4;
  prependPatchAddr(state, target, immPtr);
}

void emitFwdJmp(CodeBlock& cb, Block* target, CodegenState& state) {
  Asm a { cb };
  emitFwdJmp(a, target, state);
}

void CodeGenerator::emitFwdJcc(Asm& a, ConditionCode cc, Block* target) {
  if (auto addr = m_state.addresses[target]) {
    return a.jccAuto(cc, addr);
  }

  // TODO(#2101926): it'd be nice to get 1-byte forward jumps here
  a.jcc(cc, a.frontier());
  TCA immPtr = a.frontier() - 4;
  prependPatchAddr(m_state, target, immPtr);
}

void CodeGenerator::emitFwdJcc(ConditionCode cc, Block* target) {
  emitFwdJcc(m_as, cc, target);
}

void CodeGenerator::emitLoadImm(Asm& as, int64_t val, PhysReg dstReg) {
  assert(dstReg != InvalidReg);
  if (dstReg.isGP()) {
    as.emitImmReg(val, dstReg);
  } else {
    assert(dstReg.isSIMD());
    if (val == 0) {
      as.pxor(dstReg, dstReg);
    } else {
      // Can't move immediate directly into XMM register, so use m_rScratch
      as.emitImmReg(val, m_rScratch);
      emitMovRegReg(as, m_rScratch, dstReg);
    }
  }
}

/*
 * Returns a XMM register containing the value of SSATmp tmp,
 * which can be either a bool, an int, or a double.
 * If the value is already in a XMM register, simply returns it.
 * Otherwise, the value is moved into rCgXMM, which is returned.
 * If instructions to convert to a double at runtime are needed,
 * they're emitted in 'as'.
 * TODO: #3634984, #3727837 This function must die.
 */
PhysReg CodeGenerator::prepXMMReg(Asm& as, const SSATmp* src,
                                  const PhysLoc& srcLoc, RegXMM rtmp) {
  assert(src->isA(Type::Bool) || src->isA(Type::Int) || src->isA(Type::Dbl));
  always_assert(srcLoc.reg() != InvalidReg);
  auto rsrc = srcLoc.reg();

  // Case 1: src is already in a XMM register
  if (rsrc.isSIMD()) {
    return rsrc;
  }

  // Case 2: src Dbl stored in GP reg
  if (src->isA(Type::Dbl)) {
    emitMovRegReg(as, rsrc, rtmp);
    return rtmp;
  }

  // Case 2.b: Bool or Int stored in GP reg
  zeroExtendIfBool(as, src, rsrc);
  as.pxor(rtmp, rtmp);
  as.cvtsi2sd(rsrc, rtmp);
  return rtmp;
}

void CodeGenerator::emitCompare(IRInstruction* inst) {
  auto src0 = inst->src(0);
  auto src1 = inst->src(1);
  auto loc0 = srcLoc(0);
  auto loc1 = srcLoc(1);
  auto const type0 = src0->type();
  auto const type1 = src1->type();

  // can't generate CMP instructions correctly for anything that isn't
  // a bool or a numeric, and we can't mix bool/numerics because
  // -1 == true in PHP, but not in HHIR binary representation
  if (!((type0 <= Type::Int  && type1 <= Type::Int) ||
        (type0 <= Type::Bool && type1 <= Type::Bool) ||
        (type0 <= Type::Cls  && type1 <= Type::Cls))) {
    CG_PUNT(emitCompare);
  }
  auto reg0 = loc0.reg();
  auto reg1 = loc1.reg();

  if (reg1 == InvalidReg) {
    if (type0 <= Type::Bool) {
      m_as.    cmpb (src1->boolVal(), rbyte(reg0));
    } else {
      m_as.    cmpq (safe_cast<int32_t>(src1->intVal()), reg0);
    }
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute reg0 - reg1
    if (type0 <= Type::Bool) {
      m_as.    cmpb (rbyte(reg1), rbyte(reg0));
    } else {
      m_as.    cmpq (reg1, reg0);
    }
  }
}

void CodeGenerator::emitCompareInt(IRInstruction* inst) {
  auto srcReg0 = srcLoc(0).reg();
  auto srcReg1 = srcLoc(1).reg();
  if (srcReg1 == InvalidReg) {
    m_as.    cmpq(safe_cast<int32_t>(inst->src(1)->intVal()), srcReg0);
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute srcReg0 - srcReg1
    m_as.    cmpq(srcReg1, srcReg0);
  }
}

void CodeGenerator::emitReqBindJcc(ConditionCode cc,
                                   const ReqBindJccData* extra) {
  auto& a = m_as;

  mcg->backEnd().prepareForTestAndSmash(m_mainCode, 0,
                                        TestAndSmashFlags::kAlignJccAndJmp);
  auto const patchAddr = a.frontier();
  auto const jccStub =
    emitEphemeralServiceReq(m_frozenCode,
                            mcg->getFreeStub(m_frozenCode, &mcg->cgFixups()),
                            REQ_BIND_JMPCC_FIRST,
                            RipRelative(patchAddr),
                            extra->taken,
                            extra->notTaken,
                            cc,
                            ccServiceReqArgInfo(cc));

  mcg->setJmpTransID(a.frontier());
  a.    jcc    (cc, jccStub);

  mcg->setJmpTransID(a.frontier());
  a.    jmp    (jccStub);
}

void CodeGenerator::cgCheckNullptr(IRInstruction* inst) {
  if (!inst->taken()) return;
  auto reg = srcLoc(0).reg(0);
  m_as.testq (reg, reg);
  emitFwdJcc(CC_NZ, inst->taken());
}

void CodeGenerator::cgCheckNonNull(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto taken  = inst->taken();
  assert(taken);

  m_as.testq (srcReg, srcReg);
  emitFwdJcc(CC_Z, taken);
  if (dstReg != InvalidReg) emitMovRegReg(m_as, srcReg, dstReg);
}

void CodeGenerator::cgAssertNonNull(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    m_as.testq (srcReg, srcReg);
    ifThen(m_as, CC_Z, [&] {
      m_as.ud2();
    });
  }
  emitMovRegReg(m_as, srcReg, dstReg);
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  auto const srcRegs = srcLoc(0);
  auto const dstRegs = dstLoc(0);
  shuffle2(m_as, srcRegs.reg(0), srcRegs.reg(1),
           dstRegs.reg(0), dstRegs.reg(1));
}

void CodeGenerator::cgLdUnwinderValue(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0), rVmTl[unwinderTvOff()], inst->taken());
}

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  auto const& info = m_state.catches[inst->block()];
  assert(info.afterCall);

  mcg->registerCatchBlock(info.afterCall, m_as.frontier());

  emitIncStat(m_mainCode, Stats::TC_CatchTrace);

  // We want to restore state as though the call had completed
  // successfully, so skip over any stack arguments and pop any
  // saved registers.
  if (info.rspOffset) {
    m_as.addq(info.rspOffset, rsp);
  }
  PhysRegSaverParity::emitPops(m_as, info.savedRegs);
}

static void unwindResumeHelper(_Unwind_Exception* data) {
  tl_regState = VMRegState::CLEAN;
  _Unwind_Resume(data);
}

static void callUnwindResumeHelper(Asm& as) {
  as.loadq(rVmTl[unwinderScratchOff()], rdi);
  as.call ((TCA)unwindResumeHelper); // pass control back to the unwinder
  as.ud2();
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  callUnwindResumeHelper(m_as);
}

void CodeGenerator::cgTryEndCatch(IRInstruction* inst) {
  m_as.cmpb (0, rVmTl[unwinderSideExitOff()]);
  unlikelyIfBlock(CC_E, callUnwindResumeHelper);

  // doSideExit == true, so fall through to the side exit code
  emitIncStat(m_mainCode, Stats::TC_CatchSideExit);
}

void CodeGenerator::cgDeleteUnwinderException(IRInstruction* inst) {
  m_as.loadq(rVmTl[unwinderScratchOff()], rdi);
  m_as.call ((TCA)_Unwind_DeleteException);
}

void CodeGenerator::cgJcc(IRInstruction* inst) {
  emitCompare(inst);
  emitFwdJcc(opToConditionCode(inst->op()), inst->taken());
}

void CodeGenerator::cgJccInt(IRInstruction* inst) {
  emitCompareInt(inst);
  emitFwdJcc(opToConditionCode(inst->op()), inst->taken());
}

void CodeGenerator::cgReqBindJcc(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitCompare(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgReqBindJccInt(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitCompareInt(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgJmpGt(IRInstruction* i)    { cgJcc(i); }
void CodeGenerator::cgJmpGte(IRInstruction* i)   { cgJcc(i); }
void CodeGenerator::cgJmpLt(IRInstruction* i)    { cgJcc(i); }
void CodeGenerator::cgJmpLte(IRInstruction* i)   { cgJcc(i); }
void CodeGenerator::cgJmpEq(IRInstruction* i)    { cgJcc(i); }
void CodeGenerator::cgJmpNeq(IRInstruction* i)   { cgJcc(i); }
void CodeGenerator::cgJmpSame(IRInstruction* i)  { cgJcc(i); }
void CodeGenerator::cgJmpNSame(IRInstruction* i) { cgJcc(i); }

void CodeGenerator::cgReqBindJmpGt(IRInstruction* i)    { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpGte(IRInstruction* i)   { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpLt(IRInstruction* i)    { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpLte(IRInstruction* i)   { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpEq(IRInstruction* i)    { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpNeq(IRInstruction* i)   { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpSame(IRInstruction* i)  { cgReqBindJcc(i); }
void CodeGenerator::cgReqBindJmpNSame(IRInstruction* i) { cgReqBindJcc(i); }

void CodeGenerator::cgSideExitJmpGt(IRInstruction* i)    { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpGte(IRInstruction* i)   { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpLt(IRInstruction* i)    { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpLte(IRInstruction* i)   { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpEq(IRInstruction* i)    { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpNeq(IRInstruction* i)   { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpSame(IRInstruction* i)  { cgExitJcc(i); }
void CodeGenerator::cgSideExitJmpNSame(IRInstruction* i) { cgExitJcc(i); }

void CodeGenerator::cgJmpGtInt(IRInstruction* i)    { cgJccInt(i); }
void CodeGenerator::cgJmpGteInt(IRInstruction* i)   { cgJccInt(i); }
void CodeGenerator::cgJmpLtInt(IRInstruction* i)    { cgJccInt(i); }
void CodeGenerator::cgJmpLteInt(IRInstruction* i)   { cgJccInt(i); }
void CodeGenerator::cgJmpEqInt(IRInstruction* i)    { cgJccInt(i); }
void CodeGenerator::cgJmpNeqInt(IRInstruction* i)   { cgJccInt(i); }

void CodeGenerator::cgReqBindJmpGtInt(IRInstruction* i)  { cgReqBindJccInt(i); }
void CodeGenerator::cgReqBindJmpGteInt(IRInstruction* i) { cgReqBindJccInt(i); }
void CodeGenerator::cgReqBindJmpLtInt(IRInstruction* i)  { cgReqBindJccInt(i); }
void CodeGenerator::cgReqBindJmpLteInt(IRInstruction* i) { cgReqBindJccInt(i); }
void CodeGenerator::cgReqBindJmpEqInt(IRInstruction* i)  { cgReqBindJccInt(i); }
void CodeGenerator::cgReqBindJmpNeqInt(IRInstruction* i) { cgReqBindJccInt(i); }

void CodeGenerator::cgSideExitJmpGtInt(IRInstruction* i)  { cgExitJccInt(i); }
void CodeGenerator::cgSideExitJmpGteInt(IRInstruction* i) { cgExitJccInt(i); }
void CodeGenerator::cgSideExitJmpLtInt(IRInstruction* i)  { cgExitJccInt(i); }
void CodeGenerator::cgSideExitJmpLteInt(IRInstruction* i) { cgExitJccInt(i); }
void CodeGenerator::cgSideExitJmpEqInt(IRInstruction* i)  { cgExitJccInt(i); }
void CodeGenerator::cgSideExitJmpNeqInt(IRInstruction* i) { cgExitJccInt(i); }

//////////////////////////////////////////////////////////////////////

void CodeGenerator::cgHalt(IRInstruction* inst) {
  m_as.ud2();
}

//////////////////////////////////////////////////////////////////////

/**
 * Once the arg sources and dests are all assigned; emit moves and exchanges to
 * put all the args in desired registers. Any arguments that don't fit in
 * registers will be put on the stack. In addition to moves and exchanges,
 * shuffleArgs also handles adding lea-offsets for dest registers (dest = src +
 * lea-offset) and zero extending bools (dest = zeroExtend(src)).
 */
static bool shuffleArgsPlanningHelper(PhysReg::Map<PhysReg>& moves,
                                      PhysReg::Map<ArgDesc*>& argDescs,
                                      ArgDesc& arg) {
  auto kind = arg.kind();
  if (!(kind == ArgDesc::Kind::Reg  ||
        kind == ArgDesc::Kind::Addr ||
        kind == ArgDesc::Kind::TypeReg)) {
    return true;
  }
  auto dstReg = arg.dstReg();
  auto srcReg = arg.srcReg();
  if (dstReg != srcReg) {
    moves[dstReg] = srcReg;
    argDescs[dstReg] = &arg;
  }
  return false;
}

static int64_t shuffleArgs(Asm& a, ArgGroup& args, CppCall& call) {
  // Compute the move/shuffle plan.
  PhysReg::Map<PhysReg> moves;
  PhysReg::Map<ArgDesc*> argDescs;

  for (size_t i = 0; i < args.numRegArgs(); ++i) {
    if (shuffleArgsPlanningHelper(moves, argDescs, args[i])) {
      continue;
    }
    switch (call.kind()) {
    case CppCall::Kind::Indirect:
      if (args[i].dstReg() == call.reg()) {
        // an indirect call uses an argument register for the func ptr.
        // Use rax instead and update the CppCall
        moves[reg::rax] = call.reg();
        call.updateCallIndirect(reg::rax);
      }
      break;
    case CppCall::Kind::Direct:
    case CppCall::Kind::Virtual:
    case CppCall::Kind::ArrayVirt:
      break;
    }
  }
  for (size_t i = 0; i < args.numSIMDRegArgs(); ++i) {
    shuffleArgsPlanningHelper(moves, argDescs, args.regSIMD(i));
  }

  // The caller may be using rCgGP directly, or indirectly via m_rScratch.
  // Carefully avoid using any Assembler macro-instruction that would
  // clobber rAsm.
  auto rTmp = rAsm;

  // Store any arguments past the initial 6 to the stack. This has to happen
  // before the shuffles below in case the shuffles would clobber any of the
  // srcRegs here.
  for (int i = args.numStackArgs() - 1; i >= 0; --i) {
    auto& arg = args.stk(i);
    auto srcReg = arg.srcReg();
    assert(arg.dstReg() == InvalidReg);
    switch (arg.kind()) {
      case ArgDesc::Kind::Reg:
        always_assert(srcReg != rTmp);
        if (arg.isZeroExtend()) {
          a.  movzbl(rbyte(srcReg), r32(rTmp));
          a.  push(rTmp);
        } else {
          if (srcReg.isSIMD()) {
            emitMovRegReg(a, srcReg, rTmp);
            a.push(rTmp);
          } else {
            a.push(srcReg);
          }
        }
        break;

      case ArgDesc::Kind::TypeReg:
        static_assert(kTypeWordOffset == 0 || kTypeWordOffset == 1,
                      "kTypeWordOffset value not supported");
        always_assert(srcReg != rTmp);
        assert(srcReg.isGP());
        // x86 stacks grow down, so push higher offset items first
        if (kTypeWordOffset == 0) {
          a.  push (srcReg);
        } else {
          // 4 bytes of garbage:
          a.  pushl(eax);
          // get the type in the right place in rTmp before pushing it
          a.  movb (rbyte(srcReg), rbyte(rTmp));
          a.  shll (CHAR_BIT, r32(rTmp));
          a.  pushl(r32(rTmp));
        }
        break;

      case ArgDesc::Kind::Imm:
        a.    emitImmReg(arg.imm(), rTmp);
        a.    push(rTmp);
        break;

      case ArgDesc::Kind::Addr:
        a.    lea (arg.srcReg()[arg.disp().l()], rTmp);
        a.    push(rTmp);
        break;

      case ArgDesc::Kind::None:
        a.    push(rax);
        if (RuntimeOption::EvalHHIRGenerateAsserts) {
          emitImmStoreq(a, 0xbadbadbadbadbad, *rsp);
        }
        break;
    }
  }

  // Execute the plan
  int num_moves = 0;
  auto const howTo = doRegMoves(moves, rTmp);
  for (auto& how : howTo) {
    switch (how.m_kind) {
      case MoveInfo::Kind::Move: {
        num_moves++;
        if (how.m_dst == rTmp) {
          emitMovRegReg(a, how.m_src, how.m_dst);
        } else {
          ArgDesc* argDesc = argDescs[how.m_dst];
          if (argDesc == nullptr) {
            // when no ArgDesc is available is a straight reg to reg copy
            emitMovRegReg(a, how.m_src, how.m_dst);
          } else {
            ArgDesc::Kind kind = argDesc->kind();
            if (kind == ArgDesc::Kind::Reg || kind == ArgDesc::Kind::TypeReg) {
              if (argDesc->isZeroExtend()) {
                assert(how.m_src.isGP());
                assert(how.m_dst.isGP());
                a. movzbl (rbyte(how.m_src), r32(how.m_dst));
              } else {
                emitMovRegReg(a, how.m_src, how.m_dst);
              }
            } else {
              assert(kind == ArgDesc::Kind::Addr);
              assert(how.m_src.isGP());
              assert(how.m_dst.isGP());
              a. lea (how.m_src[argDesc->disp().l()], how.m_dst);
            }
            if (kind != ArgDesc::Kind::TypeReg) {
              argDesc->markDone();
            }
          }
        }
        break;
      }
    case MoveInfo::Kind::Xchg:
      num_moves++;
      assert(how.m_src.isGP());
      assert(how.m_dst.isGP());
      a.    xchgq  (how.m_src, how.m_dst);
      break;
    }
  }
  if (num_moves > 0) {
    TRACE(2, "arg-moves %d\n", num_moves);
  }

  // Handle const-to-register moves, type shifting,
  // load-effective address and zero extending for bools.
  // Ignore args that have been handled by the
  // move above.
  for (size_t i = 0; i < args.numRegArgs(); ++i) {
    if (!args[i].done()) {
      ArgDesc::Kind kind = args[i].kind();
      PhysReg dst = args[i].dstReg();
      assert(dst.isGP());
      if (kind == ArgDesc::Kind::Imm) {
        a.emitImmReg(args[i].imm().q(), dst);
      } else if (kind == ArgDesc::Kind::TypeReg) {
        if (kTypeShiftBits > 0) {
          a.    shlq   (kTypeShiftBits, dst);
        }
      } else if (kind == ArgDesc::Kind::Addr) {
        a.    addq   (args[i].disp(), dst);
      } else if (args[i].isZeroExtend()) {
        a.    movzbl (rbyte(dst), r32(dst));
      } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
                 kind == ArgDesc::Kind::None) {
        a.emitImmReg(0xbadbadbadbadbad, dst);
      }
    }
  }

  for (size_t i = 0; i < args.numSIMDRegArgs(); ++i) {
    auto &arg = args.regSIMD(i);
    if (arg.done()) continue;
    auto kind = arg.kind();
    auto dst = arg.dstReg();
    assert(dst.isSIMD());
    if (kind == ArgDesc::Kind::Imm) {
      a.emitImmReg(arg.imm().q(), rTmp);
      a.movq_rx(rTmp, dst);
    } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
               kind == ArgDesc::Kind::None) {
      a.emitImmReg(0xbadbadbadbadbad, rTmp);
      a.movq_rx(rTmp, dst);
    }
  }

  return args.numStackArgs() * sizeof(int64_t);
}

void CodeGenerator::cgCallNative(Asm& a, IRInstruction* inst) {
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
      return CppCall::direct(
        reinterpret_cast<void (*)()>(inst->src(info.func.srcIdx)->tcaVal()));
    }
    not_reached();
  }();

  auto const dest = [&]() -> CallDest {
    switch (info.dest) {
      case DestType::None:  return kVoidDest;
      case DestType::TV:    return callDestTV(inst);
      case DestType::SSA:   return callDest(inst);
      case DestType::Dbl:   return callDestDbl(inst);
    }
    not_reached();
  }();

  cgCallHelper(a, call, dest, info.sync, argGroup);
}

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

CallDest CodeGenerator::callDestDbl(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::Dbl, loc.reg(0), loc.reg(1) };
}

CallHelperInfo CodeGenerator::cgCallHelper(Asm& a,
                                           CppCall call,
                                           const CallDest& dstInfo,
                                           SyncOptions sync,
                                           ArgGroup& args) {
  return cgCallHelper(a, call, dstInfo, sync, args,
    m_state.liveRegs[m_curInst]);
}

CallHelperInfo CodeGenerator::cgCallHelper(Asm& a,
                                           CppCall call,
                                           const CallDest& dstInfo,
                                           SyncOptions sync,
                                           ArgGroup& args,
                                           RegSet toSave) {
  assert(m_curInst->isNative());

  auto const destType = dstInfo.type;
  auto const dstReg0  = dstInfo.reg0;
  auto const dstReg1  = dstInfo.reg1;

  CallHelperInfo ret;

  // Save the caller-saved registers that are live across this
  // instruction. The number of regs to save and the number of args
  // being passed on the stack affect the parity of the PhysRegSaver,
  // so we use the generic version here.
  toSave = toSave & kCallerSaved;
  assert((toSave & RegSet().add(dstReg0).add(dstReg1)).empty());
  assert(IMPLIES(m_curInst->is(Call, CallArray), toSave.empty()));
  PhysRegSaverParity regSaver(1 + args.numStackArgs(), a, toSave);

  // Assign registers to the arguments then prepare them for the call.
  for (size_t i = 0; i < args.numRegArgs(); i++) {
    args[i].setDstReg(argNumToRegName[i]);
  }
  for (size_t i = 0; i < args.numSIMDRegArgs(); i++) {
    args.regSIMD(i).setDstReg(argNumToSIMDRegName[i]);
  }
  regSaver.bytesPushed(shuffleArgs(a, args, call));

  // do the call; may use a trampoline
  if (sync == SyncOptions::kSmashableAndSyncPoint) {
    assert(call.kind() == CppCall::Kind::Direct);
    mcg->backEnd().prepareForSmash(a.code(), kCallLen);
    a.call((TCA)call.address());
  } else {
    emitCall(a, call);
  }
  ret.returnAddress = a.frontier();
  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    // if we are profiling the heap, we always need to sync because
    // regs need to be correct during smart allocations no matter
    // what
    recordSyncPoint(a, sync);
  }

  auto* taken = m_curInst->taken();
  if (taken && taken->isCatch()) {
    always_assert_flog(
      sync != SyncOptions::kNoSyncPoint,
      "cgCallHelper called with kNoSyncPoint but inst has a catch block: {}\n",
      *m_curInst
    );
    always_assert_flog(
      taken->catchMarker() == m_curInst->marker(),
      "Catch trace doesn't match fixup:\n"
      "Instruction: {}\n"
      "Catch trace: {}\n"
      "Fixup      : {}\n",
      m_curInst->toString(),
      taken->catchMarker().show(),
      m_curInst->marker().show()
    );

    auto& info = m_state.catches[taken];
    assert(!info.afterCall);
    info.afterCall = a.frontier();
    info.savedRegs = toSave;
    info.rspOffset = regSaver.rspAdjustment();
  } else if (!m_curInst->is(Call, CallArray, ContEnter)) {
    // The current instruction doesn't have a catch block so it'd better not
    // throw. Register a null catch trace to indicate this to the
    // unwinder. Call and CallArray don't have catch blocks because they smash
    // all live values and optimizations are aware of this.
    mcg->registerCatchBlock(a.frontier(), nullptr);
  }

  // copy the call result to the destination register(s)
  switch (destType) {
  case DestType::TV:
    {
      // rax contains m_type and m_aux but we're expecting just the
      // type in the lower bits, so shift the type result register.
      auto rval = packed_tv ? reg::rdx : reg::rax;
      auto rtyp = packed_tv ? reg::rax : reg::rdx;
      if (kTypeShiftBits > 0) a.shrq(kTypeShiftBits, rtyp);
      shuffle2(a, rval, rtyp, dstReg0, dstReg1);
    }
    break;
  case DestType::SSA:
    // copy the single-register result to dstReg0
    assert(dstReg1 == InvalidReg);
    if (dstReg0 != InvalidReg) emitMovRegReg(a, reg::rax, dstReg0);
    break;
  case DestType::None:
    // void return type, no registers have values
    assert(dstReg0 == InvalidReg && dstReg1 == InvalidReg);
    break;
  case DestType::Dbl:
    // copy the single-register result to dstReg0
    assert(dstReg1 == InvalidReg);
    if (dstReg0 != InvalidReg) emitMovRegReg(a, reg::xmm0, dstReg0);
    break;
  }
  return ret;
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  always_assert(inst->src(0)->numWords() == inst->dst(0)->numWords());
  if (srcLoc(0).hasReg(1)) {
    assert(srcLoc(0).reg() != InvalidReg);
    shuffle2(m_as, srcLoc(0).reg(), srcLoc(0).reg(1),
             dstLoc(0).reg(), dstLoc(0).reg(1));
    return;
  }
  auto sreg = srcLoc(0).reg();
  auto dreg = dstLoc(0).reg();
  if (sreg == InvalidReg) {
    emitLoadImm(m_as, inst->src(0)->rawVal(), dreg);
  } else {
    emitMovRegReg(m_as, sreg, dreg);
  }
}

template<class OpInstr>
void CodeGenerator::cgUnaryIntOp(PhysLoc dst_loc,
                                 SSATmp* src, PhysLoc src_loc,
                                 OpInstr instr) {
  assert(src->isA(Type::Int));
  assert(dst_loc.reg() != InvalidReg);
  assert(src_loc.reg() != InvalidReg);
  auto dstReg = dst_loc.reg();
  auto srcReg = src_loc.reg();
  auto& a = m_as;

  // Integer operations require 64-bit representations
  zeroExtendIfBool(a, src, srcReg);

  emitMovRegReg(a, srcReg, dstReg);
  (a.*instr)   (dstReg);
}

void CodeGenerator::cgAbsDbl(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto resReg = dstReg.isSIMD() ? dstReg : PhysReg(rCgXMM0);

  emitMovRegReg(m_as, srcReg, resReg);

  // clear the high bit
  m_as.    psllq  (1, resReg);
  m_as.    psrlq  (1, resReg);

  emitMovRegReg(m_as, resReg, dstReg);
}

inline static Reg8 convertToReg8(PhysReg reg) { return rbyte(reg); }
inline static Reg64 convertToReg64(PhysReg reg) { return reg; }

typedef void (Asm::*AsmInstrIR)(Immed, Reg64);

template<class RegType>
void CodeGenerator::cgBinaryIntOp(IRInstruction* inst,
                                  void (Asm::*instrIR)(Immed, RegType),
                                  void (Asm::*instrRR)(RegType, RegType),
                                  void (Asm::*movInstr)(RegType, RegType),
                                  RegType (*convertReg)(PhysReg),
                                  Commutativity commuteFlag) {
  assert(m_curInst == inst); // could remove the inst param
  const SSATmp* src1  = inst->src(0);
  const SSATmp* src2  = inst->src(1);

  // inputs must be ints, or a (bool,bool) operation that ends up behaving
  // like an int anyway (e.g. XorBool)
  assert((src1->isA(Type::Int) && src2->isA(Type::Int)) ||
         (src1->isA(Type::Bool) && src2->isA(Type::Bool)));

  bool const commutative = commuteFlag == Commutative;
  auto const dstReg      = dstLoc(0).reg();
  auto const src1Reg     = srcLoc(0).reg();
  auto const src2Reg     = srcLoc(1).reg();
  auto& a                = m_as;

  auto const dstOpReg    = convertReg(dstReg);
  auto const src1OpReg   = convertReg(src1Reg);
  auto const src2OpReg   = convertReg(src2Reg);
  auto const rOpScratch  = convertReg(m_rScratch);

  auto opWithScratch = [&]() {
    (a.*movInstr)(src1OpReg, rOpScratch);
    (a.*instrRR) (src2OpReg, rOpScratch);
    (a.*movInstr)(rOpScratch, dstOpReg);
  };

  // Two registers.
  if (src1Reg != InvalidReg && src2Reg != InvalidReg) {
    if (dstReg == src1Reg) {
      (a.*instrRR)  (src2OpReg, dstOpReg);
    } else if (dstReg == src2Reg) {
      if (commutative) {
        (a.*instrRR) (src1OpReg, dstOpReg);
      } else {
        opWithScratch();
      }
    } else {
      emitMovRegReg(a, src1Reg, dstReg);
      (a.*instrRR) (src2OpReg, dstOpReg);
    }
    return;
  }

  auto intVal = [] (const SSATmp* s) {
    return s->isA(Type::Int) ? safe_cast<int32_t>(s->intVal()) :
           int32_t(s->boolVal());
  };

  // One register, and one immediate.
  if (commutative) {
    assert(instrIR);
    auto imm = intVal(src2Reg == InvalidReg ? src2 : src1);
    auto srcReg = srcLoc(src2Reg == InvalidReg ? 0 : 1).reg();
    if (srcReg == dstReg) {
      (a.*instrIR) (imm, dstOpReg);
    } else {
      a. emitImmReg(imm, dstReg);
      (a.*instrRR) (convertReg(srcReg), dstOpReg);
    }
    return;
  }

  // NonCommutative:
  if (src1Reg == InvalidReg) {
    if (dstReg == src2Reg) {
      emitLoadImm(a, intVal(src1), m_rScratch);
      (a.*instrRR) (src2OpReg, rOpScratch);
      (a.*movInstr)(rOpScratch, dstOpReg);
    } else {
      emitLoadImm(a, intVal(src1), dstReg);
      (a.*instrRR) (src2OpReg, dstOpReg);
    }
    return;
  }

  assert(src2Reg == InvalidReg);
  emitMovRegReg(a, src1Reg, dstReg);
  (a.*instrIR) (intVal(src2), dstOpReg);
}

void CodeGenerator::cgBinaryDblOp(IRInstruction* inst,
                                  void (Asm::*fpInstr)(RegXMM, RegXMM)) {
  assert(inst == m_curInst);
  const SSATmp* src1  = inst->src(0);
  const SSATmp* src2  = inst->src(1);
  auto loc1 = srcLoc(0);
  auto loc2 = srcLoc(1);
  assert(src1->isA(Type::Dbl) && src2->isA(Type::Dbl));

  PhysReg dstReg  = dstLoc(0).reg();
  PhysReg resReg  = dstReg.isSIMD() && dstReg != loc2.reg() ?
                    dstReg : PhysReg(rCgXMM0);
  assert(resReg.isSIMD());

  PhysReg srcReg1 = prepXMMReg(m_as, src1, loc1, resReg);
  PhysReg srcReg2 = prepXMMReg(m_as, src2, loc2, rCgXMM1);
  assert(srcReg1 != rCgXMM1 && srcReg2 != rCgXMM0);

  emitMovRegReg(m_as, srcReg1, resReg);

  (m_as.*fpInstr)(srcReg2, resReg);

  emitMovRegReg(m_as, resReg, dstReg);
}

void CodeGenerator::cgAddIntO(IRInstruction* inst) {
  cgAddInt(inst);
  assert(inst->taken() != nullptr);
  emitFwdJcc(m_as, CC_O, inst->taken());
}

void CodeGenerator::cgSubIntO(IRInstruction* inst) {
  cgSubInt(inst);
  assert(inst->taken() != nullptr);
  emitFwdJcc(m_as, CC_O, inst->taken());
}

void CodeGenerator::cgMulIntO(IRInstruction* inst) {
  cgMulInt(inst);
  assert(inst->taken() != nullptr);
  emitFwdJcc(m_as, CC_O, inst->taken());
}

/*
 * If src2 is 1, this generates dst = src1 - 1 or src1 + 1 using the inc
 * or dec x86 instructions. The return value is whether or not the
 * instruction could be generated.
 */
bool CodeGenerator::emitIncDecHelper(PhysLoc dst, SSATmp* src1, PhysLoc loc1,
                                     SSATmp* src2, PhysLoc loc2,
                                     void(Asm::*emitFunc)(Reg64)) {
  if (loc1.reg() != InvalidReg && loc2.reg() != InvalidReg &&
      src2->isConst(1)) {
    emitMovRegReg(m_as, loc1.reg(), dst.reg());
    (m_as.*emitFunc)(dst.reg());
    return true;
  }
  return false;
}

void CodeGenerator::cgRoundCommon(IRInstruction* inst, RoundDirection dir) {
  auto src = inst->src(0);

  auto dstReg = dstLoc(0).reg();
  auto inReg  = prepXMMReg(m_as, src, srcLoc(0), rCgXMM0);
  auto outReg = dstReg.isSIMD() ? dstReg : PhysReg(rCgXMM1);

  m_as.   roundsd   (dir, inReg, outReg);
  emitMovRegReg(m_as, outReg, dstReg);
}

void CodeGenerator::cgFloor(IRInstruction* inst) {
  cgRoundCommon(inst, RoundDirection::floor);
}

void CodeGenerator::cgCeil(IRInstruction* inst) {
  cgRoundCommon(inst, RoundDirection::ceil);
}

void CodeGenerator::cgAddInt(IRInstruction* inst) {
  SSATmp* src1 = inst->src(0);
  SSATmp* src2 = inst->src(1);
  auto loc1 = srcLoc(0);
  auto loc2 = srcLoc(1);
  auto dst = dstLoc(0);

  // Special cases: x = y + 1, x = 1 + y
  if (emitIncDecHelper(dst, src1, loc1, src2, loc2, &Asm::incq) ||
      emitIncDecHelper(dst, src2, loc2, src1, loc1, &Asm::incq)) {
    return;
  }

  cgBinaryIntOp(
    inst,
    &Asm::addq,
    &Asm::addq,
    &Asm::movq,
    &convertToReg64,
    Commutative
  );
}

void CodeGenerator::cgSubInt(IRInstruction* inst) {
  auto src1 = inst->src(0);
  auto src2 = inst->src(1);
  auto loc1 = srcLoc(0);
  auto loc2 = srcLoc(1);
  auto dst = dstLoc(0);

  if (emitIncDecHelper(dst, src1, loc1, src2, loc2, &Asm::decq)) return;

  if (src1->isConst(0)) {
    // There is no unary negate HHIR instruction, so handle that here.
    cgUnaryIntOp(dst, src2, loc2, &Asm::neg);
    return;
  }

  cgBinaryIntOp(
    inst,
    &Asm::subq,
    &Asm::subq,
    &Asm::movq,
    &convertToReg64,
    NonCommutative
  );
}

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  cgBinaryIntOp(
    inst,
    (AsmInstrIR)nullptr, // there is no imul-immediate instruction
    &Asm::imul,
    &Asm::movq,
    &convertToReg64,
    Commutative
  );
}

void CodeGenerator::cgAddDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, &Asm::addsd);
}

void CodeGenerator::cgSubDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, &Asm::subsd);
}

void CodeGenerator::cgMulDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, &Asm::mulsd);
}

void CodeGenerator::cgDivDbl(IRInstruction* inst) {
  const SSATmp* src1  = inst->src(0);
  const SSATmp* src2  = inst->src(1);
  auto loc1 = srcLoc(0);
  auto loc2 = srcLoc(1);
  auto exit = inst->taken();

  auto dstReg  = dstLoc(0).reg();
  auto resReg  = dstReg.isSIMD() && dstReg != loc2.reg() ? dstReg :
                 PhysReg(rCgXMM0);
  assert(resReg.isSIMD());

  // only load divisor
  PhysReg srcReg2 = prepXMMReg(m_as, src2, loc2, rCgXMM1);
  assert(srcReg2 != rCgXMM0);

  // divide by zero check
  m_as.pxor(rCgXMM0, rCgXMM0);
  m_as.ucomisd(rCgXMM0, srcReg2);
  unlikelyIfBlock(CC_NP, [&] (Asm& a) {
    emitFwdJcc(a, CC_E, exit);
  });

  // now load dividend
  PhysReg srcReg1 = prepXMMReg(m_as, src1, loc1, resReg);
  assert(srcReg1 != rCgXMM1);

  emitMovRegReg(m_as, srcReg1, resReg);
  m_as.divsd(srcReg2, resReg);
  emitMovRegReg(m_as, resReg, dstReg);
}

void CodeGenerator::cgAndInt(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::andq,
                &Asm::andq,
                &Asm::movq,
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgOrInt(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::orq,
                &Asm::orq,
                &Asm::movq,
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgXorInt(IRInstruction* inst) {
  if (inst->src(1)->isConst(-1)) {
    return cgUnaryIntOp(dstLoc(0), inst->src(0), srcLoc(0), &Asm::not);
  }
  cgBinaryIntOp(inst,
                &Asm::xorq,
                &Asm::xorq,
                &Asm::movq,
                &convertToReg64,
                Commutative);
}

void CodeGenerator::cgXorBool(IRInstruction* inst) {
  cgBinaryIntOp(inst,
                &Asm::xorb,
                &Asm::xorb,
                &Asm::movb,
                &convertToReg8,
                Commutative);
}

void CodeGenerator::cgMod(IRInstruction* inst) {
  static_assert(rCgGP != rax && rCgGP != rdx, "");
  auto const dstReg = dstLoc(0).reg();
  auto const reg0 = srcLoc(0).reg();
  auto const reg1 = srcLoc(1).reg();
  auto& a = m_as;

  // spill rax and/or rdx
  bool spillRax = dstReg != rax;
  bool spillRdx = dstReg != rdx;
  if (spillRax) a.  push   (rax);
  if (spillRdx) a.  push   (rdx);
  // put divisor in rCgGP if it would get clobbered
  auto divisor = reg1 != rax && reg1 != rdx ? reg1 : PhysReg(rCgGP);
  emitMovRegReg(a, reg1, divisor);
  // put dividend in rax
  emitMovRegReg(a, reg0, rax);
  a.    cqo    ();        // sign-extend rax => rdx:rax
  a.    idiv   (divisor); // rdx:rax/divisor => quot:rax, rem:rdx
  emitMovRegReg(a, rdx, dstReg);
  // restore rax and/or rdx
  if (spillRdx) a.  pop    (reg::rdx);
  if (spillRax) a.  pop    (reg::rax);
}

void CodeGenerator::cgSqrt(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto resReg = dstReg.isSIMD() ? dstReg : PhysReg(rCgXMM0);
  emitMovRegReg(m_as, srcReg, resReg);
  m_as.  sqrtsd  (resReg, resReg);
  emitMovRegReg  (m_as, resReg, dstReg);
}

void CodeGenerator::cgShiftCommon(IRInstruction* inst,
                                  void (Asm::*instrIR)(Immed, Reg64),
                                  void (Asm::*instrR)(Reg64)) {
  auto const srcReg0 = srcLoc(0).reg();
  auto const srcReg1 = srcLoc(1).reg();
  auto const dstReg  = dstLoc(0).reg();
  assert(srcReg0 != InvalidReg);

  // one immediate (right hand side)
  if (srcReg1 == InvalidReg) {
    emitMovRegReg(m_as, srcReg0, dstReg);
    (m_as.*instrIR)(safe_cast<int32_t>(inst->src(1)->intVal()), dstReg);
    return;
  }

  // in order to shift by a variable amount src2 must be in rcx :(
  bool swapRCX = srcReg1 != reg::rcx;

  // will we be using dstReg as scratch storage?
  bool dstIsRHS = dstReg == srcReg1;
  bool tmpIsRCX = m_rScratch == reg::rcx;
  bool dstIsRCX = dstReg == reg::rcx;

  // we need rcx for srcReg1 so we use srcReg1 as a temp for rcx, we also need
  // to handle the cases where the destination is rcx or src2 or both...
  auto resReg = dstIsRCX ? (dstIsRHS ? PhysReg(m_rScratch) : srcReg1)
                         : (dstIsRHS ? (tmpIsRCX ? dstReg : PhysReg(m_rScratch))
                                     : dstReg);

  // if srcReg0 was in rcx it will be swapped with srcReg1 below
  auto regLeft = srcReg0 == reg::rcx ? srcReg1 : srcReg0;

  // we use srcReg1 as a scratch for whatever is in rcx
  if (swapRCX) {
    m_as.   xchgq(reg::rcx, srcReg1);
  }

  emitMovRegReg(m_as, regLeft, resReg);
  (m_as.*instrR)(resReg);

  if (resReg == dstReg && srcReg1 == dstReg) {
    // If we get here it means that m_rScratch was rcx and we shouldn't do any
    // more swapping because we stored the result in the right place
    return;
  }

  if (swapRCX) {
    m_as.   xchgq(reg::rcx, srcReg1);
  }

  // if resReg == srcReg1 then dstReg must have been rcx and the above swap
  // already repaired the situation
  if (resReg != srcReg1) {
    emitMovRegReg(m_as, resReg, dstReg);
  }
}

void CodeGenerator::cgShl(IRInstruction* inst) {
  cgShiftCommon(inst, &Asm::shlq, &Asm::shlq);
}

void CodeGenerator::cgShr(IRInstruction* inst) {
  cgShiftCommon(inst, &Asm::sarq, &Asm::sarq);
}

///////////////////////////////////////////////////////////////////////////////
// Comparison Operators
///////////////////////////////////////////////////////////////////////////////

#define DISPATCHER(name)\
  int64_t ccmp_ ## name (StringData* a1, StringData* a2)\
  { return name(a1, a2); }\
  int64_t ccmp_ ## name (StringData* a1, int64_t a2)\
  { return name(a1, a2); }\
  int64_t ccmp_ ## name (StringData* a1, ObjectData* a2)\
  { return name(a1, Object(a2)); }\
  int64_t ccmp_ ## name (ObjectData* a1, ObjectData* a2)\
  { return name(Object(a1), Object(a2)); }\
  int64_t ccmp_ ## name (ObjectData* a1, int64_t a2)\
  { return name(Object(a1), a2); }\
  int64_t ccmp_ ## name (ArrayData* a1, ArrayData* a2)\
  { return name(Array(a1), Array(a2)); }

DISPATCHER(same)
DISPATCHER(equal)
DISPATCHER(more)
DISPATCHER(less)

#undef DISPATCHER

template <typename A, typename B>
inline int64_t ccmp_nsame(A a, B b) { return !ccmp_same(a, b); }

template <typename A, typename B>
inline int64_t ccmp_nequal(A a, B b) { return !ccmp_equal(a, b); }

// TODO Task #2661083: We cannot assume that "(a <= b) === !(a > b)" for
// all types. In particular, this assumption does not hold when comparing
// two arrays or comparing two objects. We should fix this.
template <typename A, typename B>
inline int64_t ccmp_lte(A a, B b) { return !ccmp_more(a, b); }

template <typename A, typename B>
inline int64_t ccmp_gte(A a, B b) { return !ccmp_less(a, b); }

#define CG_OP_CMP(inst, setter, name)                                   \
  cgCmpHelper(inst, &Asm:: setter, ccmp_ ## name, ccmp_ ## name,        \
              ccmp_ ## name, ccmp_ ## name, ccmp_ ## name, ccmp_ ## name)

// SON - string, object, or number
static bool typeIsSON(Type t) {
  return t.subtypeOfAny(Type::Str, Type::Obj, Type::Int, Type::Dbl);
}

void CodeGenerator::cgCmpHelper(
          IRInstruction* inst,
          void (Asm::*setter)(Reg8),
          int64_t (*str_cmp_str)(StringData*, StringData*),
          int64_t (*str_cmp_int)(StringData*, int64_t),
          int64_t (*str_cmp_obj)(StringData*, ObjectData*),
          int64_t (*obj_cmp_obj)(ObjectData*, ObjectData*),
          int64_t (*obj_cmp_int)(ObjectData*, int64_t),
          int64_t (*arr_cmp_arr)(ArrayData*,  ArrayData*)
        ) {
  SSATmp* src1  = inst->src(0);
  SSATmp* src2  = inst->src(1);

  Type type1 = src1->type();
  Type type2 = src2->type();

  auto loc1 = srcLoc(0);
  auto loc2 = srcLoc(1);

  auto src1Reg = loc1.reg();
  auto src2Reg = loc2.reg();
  auto dstReg  = dstLoc(0).reg();

  auto setFromFlags = [&] {
    (m_as.*setter)(rbyte(dstReg));
  };
  // It is possible that some pass has been done after simplification; if such
  // a pass invalidates our invariants, then just punt.

  // simplifyCmp has done const-const optimization
  //
  // If the types are the same and there is only one constant,
  // simplifyCmp has moved it to the right.
  if (src1->isConst()) {
    // TODO: #3626251 will let us eliminate this punt.
    CG_PUNT(cgOpCmpHelper_const);
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 1: null/string cmp string
  // simplifyCmp has converted the null to ""
  if (type1 <= Type::Str && type2 <= Type::Str) {
    cgCallHelper(m_as, CppCall::direct(str_cmp_str), callDest(inst),
      SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 2: bool/null cmp anything
  // simplifyCmp has converted all args to bool
  else if (type1 <= Type::Bool && type2 <= Type::Bool) {
    if (src2->isConst()) {
      m_as.    cmpb (src2->boolVal(), rbyte(src1Reg));
    } else {
      m_as.    cmpb (rbyte(src2Reg), rbyte(src1Reg));
    }
    setFromFlags();
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 3, 4, and 7: string/resource/object/number (sron) cmp sron
  // These cases must be amalgamated because Type::Obj can refer to an object
  //  or to a resource.
  // strings are canonicalized to the left, ints to the right
  else if (typeIsSON(type1) && typeIsSON(type2)) {
    if (type1 <= Type::Str) {
      // string cmp string is dealt with in case 1
      // string cmp double is punted above

      if (type2 <= Type::Int) {
        cgCallHelper(m_as, CppCall::direct(str_cmp_int), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else if (type2 <= Type::Obj) {
        cgCallHelper(m_as, CppCall::direct(str_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else {
        CG_PUNT(cgOpCmpHelper_sx);
      }
    }

    else if (type1 <= Type::Obj) {
      // string cmp object is dealt with above
      // object cmp double is punted above

      if (type2 <= Type::Obj) {
        cgCallHelper(m_as, CppCall::direct(obj_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else if (type2 <= Type::Int) {
        cgCallHelper(m_as, CppCall::direct(obj_cmp_int), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else {
        CG_PUNT(cgOpCmpHelper_ox);
      }
    }
    else {
      CG_PUNT(cgOpCmpHelper_SON);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 5: array cmp array
  else if (type1 <= Type::Arr && type2 <= Type::Arr) {
    cgCallHelper(m_as, CppCall::direct(arr_cmp_arr),
      callDest(inst), SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 6: array cmp anything
  // simplifyCmp has already dealt with this case.

  /////////////////////////////////////////////////////////////////////////////
  else {
    // We have a type which is not a common type. It might be a cell or a box.
    CG_PUNT(cgOpCmpHelper_unimplemented);
  }
}

void CodeGenerator::cgEq(IRInstruction* inst) {
  CG_OP_CMP(inst, sete, equal);
}

void CodeGenerator::cgEqX(IRInstruction* inst) {
  CG_OP_CMP(inst, sete, equal);
}

void CodeGenerator::cgNeq(IRInstruction* inst) {
  CG_OP_CMP(inst, setne, nequal);
}

void CodeGenerator::cgNeqX(IRInstruction* inst) {
  CG_OP_CMP(inst, setne, nequal);
}

void CodeGenerator::cgSame(IRInstruction* inst) {
  CG_OP_CMP(inst, sete, same);
}

void CodeGenerator::cgNSame(IRInstruction* inst) {
  CG_OP_CMP(inst, setne, nsame);
}

void CodeGenerator::cgLt(IRInstruction* inst) {
  CG_OP_CMP(inst, setl, less);
}

void CodeGenerator::cgLtX(IRInstruction* inst) {
  CG_OP_CMP(inst, setl, less);
}

void CodeGenerator::cgGt(IRInstruction* inst) {
  CG_OP_CMP(inst, setg, more);
}

void CodeGenerator::cgGtX(IRInstruction* inst) {
  CG_OP_CMP(inst, setg, more);
}

void CodeGenerator::cgLte(IRInstruction* inst) {
  CG_OP_CMP(inst, setle, lte);
}

void CodeGenerator::cgLteX(IRInstruction* inst) {
  CG_OP_CMP(inst, setle, lte);
}

void CodeGenerator::cgGte(IRInstruction* inst) {
  CG_OP_CMP(inst, setge, gte);
}

void CodeGenerator::cgGteX(IRInstruction* inst) {
  CG_OP_CMP(inst, setge, gte);
}

void CodeGenerator::emitCmpInt(IRInstruction* inst, ConditionCode cc) {
  auto dstReg = dstLoc(0).reg();
  emitCompareInt(inst);
  m_as.setcc(cc, rbyte(dstReg));
}

void CodeGenerator::cgEqInt(IRInstruction* inst)  { emitCmpInt(inst, CC_E); }
void CodeGenerator::cgNeqInt(IRInstruction* inst) { emitCmpInt(inst, CC_NE); }
void CodeGenerator::cgLtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_L); }
void CodeGenerator::cgGtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_G); }
void CodeGenerator::cgLteInt(IRInstruction* inst) { emitCmpInt(inst, CC_LE); }
void CodeGenerator::cgGteInt(IRInstruction* inst) { emitCmpInt(inst, CC_GE); }

void CodeGenerator::emitCmpEqDbl(IRInstruction* inst, ComparisonPred pred) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg0 = prepXMMReg(m_as, inst->src(0), srcLoc(0), rCgXMM0);
  auto srcReg1 = prepXMMReg(m_as, inst->src(1), srcLoc(1), rCgXMM1);

  m_as.  movsd   (srcReg1, rCgXMM2);
  m_as.  cmpsd   (srcReg0, rCgXMM2, pred);
  m_as.  movq_xr (rCgXMM2, dstReg);
  m_as.  andb    (1, rbyte(dstReg));
}

void CodeGenerator::emitCmpRelDbl(IRInstruction* inst, ConditionCode cc,
                                  bool flipOperands) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg0 = prepXMMReg(m_as, inst->src(0), srcLoc(0), rCgXMM0);
  auto srcReg1 = prepXMMReg(m_as, inst->src(1), srcLoc(1), rCgXMM1);

  if (flipOperands) {
    std::swap(srcReg0, srcReg1);
  }

  m_as.  ucomisd  (srcReg0, srcReg1);
  m_as.  setcc    (cc, rbyte(dstReg));
}

void CodeGenerator::cgEqDbl(IRInstruction* inst)  {
  emitCmpEqDbl(inst, ComparisonPred::eq_ord);
}
void CodeGenerator::cgNeqDbl(IRInstruction* inst) {
  emitCmpEqDbl(inst, ComparisonPred::ne_unord);
}
void CodeGenerator::cgLtDbl(IRInstruction* inst)  {
  // This is a little tricky, because "unordered" is a thing.
  //
  //         ZF  PF  CF
  // x ?= y   1   1   1
  // x <  y   0   0   1
  // x == y   1   0   0
  // x >  y   0   0   0
  //
  // This trick lets us avoid needing to handle the unordered case specially.
  // The condition codes B and BE are true if CF == 1, which it is in the
  // unordered case, and that'll give incorrect results. So we just invert the
  // condition code (A and AE don't get set if CF == 1) and flip the operands.
  emitCmpRelDbl(inst, CC_A, true);
}
void CodeGenerator::cgGtDbl(IRInstruction* inst)  {
  emitCmpRelDbl(inst, CC_A, false);
}
void CodeGenerator::cgLteDbl(IRInstruction* inst) {
  emitCmpRelDbl(inst, CC_AE, true);
}
void CodeGenerator::cgGteDbl(IRInstruction* inst) {
  emitCmpRelDbl(inst, CC_AE, false);
}

///////////////////////////////////////////////////////////////////////////////
// Type check operators
///////////////////////////////////////////////////////////////////////////////

// Overloads to put the {Object,Array}Data* into a register so
// emitTypeTest can cmp to the Class*/ArrayKind expected by the
// specialized Type

// Nothing to do, return the register that contain the ObjectData already
Reg64 getDataPtrEnregistered(Asm& as, PhysReg dataSrc, Reg64 scratch) {
  return dataSrc;
}

// Enregister the memoryRef so it can be used with an offset by the
// cmp instruction
Reg64 getDataPtrEnregistered(Asm& as, MemoryRef dataSrc, Reg64 scratch) {
  as.loadq(dataSrc, scratch);
  return scratch;
}

template<class Loc1, class Loc2, class JmpFn>
void CodeGenerator::emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc,
                                 JmpFn doJcc) {
  assert(!(type <= Type::Cls));
  ConditionCode cc;
  if (type <= Type::StaticStr) {
    emitCmpTVType(m_as, KindOfStaticString, typeSrc);
    cc = CC_E;
  } else if (type <= Type::Str) {
    assert(type != Type::CountedStr &&
           "We don't support guarding on CountedStr");
    emitTestTVType(m_as, KindOfStringBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Null) {
    emitCmpTVType(m_as, KindOfNull, typeSrc);
    cc = CC_LE;
  } else if (type == Type::UncountedInit) {
    emitTestTVType(m_as, KindOfUncountedInitBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Uncounted) {
    emitCmpTVType(m_as, KindOfRefCountThreshold, typeSrc);
    cc = CC_LE;
  } else if (type == Type::Cell) {
    assert(!m_curInst->is(LdRef));
    emitCmpTVType(m_as, KindOfRef, typeSrc);
    cc = CC_L;
  } else if (type == Type::Gen) {
    // nothing to check
    return;
  } else if (type == Type::InitCell) {
    assert(m_curInst->is(LdRef));
    // nothing to check: Refs cannot contain Uninit or another Ref.
    return;
  } else {
    always_assert(type.isKnownDataType());
    DataType dataType = type.toDataType();
    assert(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfResource));
    emitCmpTVType(m_as, dataType, typeSrc);
    cc = CC_E;
  }
  doJcc(cc);

  if (type.isSpecialized()) {
    emitSpecializedTypeTest(type, dataSrc, doJcc);
  }
}

template<class DataLoc, class JmpFn>
void CodeGenerator::emitSpecializedTypeTest(Type type, DataLoc dataSrc,
                                            JmpFn doJcc) {
  assert(type.isSpecialized());
  if (type < Type::Res) {
    // No cls field in Resource
    always_assert(0 && "unexpected guard on specialized Resource");
  }

  if (type < Type::Obj) {
    // emit the specific class test
    assert(type.getClass()->attrs() & AttrFinal);
    auto reg = getDataPtrEnregistered(m_as, dataSrc, m_rScratch);
    emitCmpClass(m_as, type.getClass(), reg[ObjectData::getVMClassOffset()]);
    doJcc(CC_E);
  } else {
    assert(type < Type::Arr);
    auto reg = getDataPtrEnregistered(m_as, dataSrc, m_rScratch);
    m_as.cmpb(type.getArrayKind(), reg[ArrayData::offsetofKind()]);
    doJcc(CC_E);
  }
}

template<class JmpFn>
void CodeGenerator::emitIsTypeTest(IRInstruction* inst, JmpFn doJcc) {
  auto const src = inst->src(0);
  auto const loc = srcLoc(0);

  // punt if specialized object for now
  if (inst->typeParam() < Type::Obj || inst->typeParam() < Type::Res) {
    CG_PUNT(IsType-SpecializedUnsupported);
  }

  if (src->isA(Type::PtrToGen)) {
    PhysReg base = loc.reg();
    emitTypeTest(inst->typeParam(), base[TVOFF(m_type)],
                 base[TVOFF(m_data)], doJcc);
    return;
  }
  assert(src->isA(Type::Gen));

  PhysReg typeSrcReg = loc.reg(1); // type register
  if (typeSrcReg == InvalidReg) {
    // Should only get here if the simplifier didn't run
    // TODO: #3626251 will handle this case.
    CG_PUNT(IsType-KnownType);
  }
  PhysReg dataSrcReg = loc.reg(0); // data register
  emitTypeTest(inst->typeParam(), typeSrcReg, dataSrcReg, doJcc);
}

template<class Loc>
void CodeGenerator::emitTypeCheck(Type type,
                                  Loc typeSrc,
                                  Loc dataSrc,
                                  Block* taken) {
  emitTypeTest(
    type, typeSrc, dataSrc,
    [&](ConditionCode cc) {
      emitFwdJcc(ccNegate(cc), taken);
    });
}

template<class Loc>
void CodeGenerator::emitTypeGuard(Type type, Loc typeSrc, Loc dataSrc) {
  emitTypeTest(type, typeSrc, dataSrc,
    [&](ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
      auto const destSR = mcg->tx().getSrcRec(destSK);
      destSR->emitFallbackJump(m_mainCode, ccNegate(cc));
    });
}

void CodeGenerator::emitSetCc(IRInstruction* inst, ConditionCode cc) {
  m_as.setcc(cc, rbyte(dstLoc(0).reg()));
}

void CodeGenerator::cgIsTypeMemCommon(IRInstruction* inst, bool negate) {
  bool called = false; // check emitSetCc is called only once
  emitIsTypeTest(inst,
    [&](ConditionCode cc) {
      assert(!called);
      emitSetCc(inst, negate ? ccNegate(cc) : cc);
      called = true;
    });
}

void CodeGenerator::cgIsTypeCommon(IRInstruction* inst, bool negate) {
  bool called = false; // check emitSetCc is called only once
  emitIsTypeTest(inst,
    [&](ConditionCode cc) {
      assert(!called);
      emitSetCc(inst, negate ? ccNegate(cc) : cc);
      called = true;
    });
}

void CodeGenerator::cgJmpIsTypeCommon(IRInstruction* inst, bool negate) {
  emitIsTypeTest(inst,
    [&](ConditionCode cc) {
      emitFwdJcc(negate ? ccNegate(cc) : cc, inst->taken());
    });
}

void CodeGenerator::cgIsType(IRInstruction* inst) {
  cgIsTypeCommon(inst, false);
}

void CodeGenerator::cgIsScalarType(IRInstruction* inst) {
  PhysReg typeReg = srcLoc(0).reg(1);
  PhysReg dstReg  = dstLoc(0).reg(0);

  /* static asserts for KindOfBoolean <= scalar type <= KindOfString */
  static_assert(KindOfUninit < KindOfBoolean, "fix checks for IsScalar");
  static_assert(KindOfNull < KindOfBoolean, "fix checks for IsScalar");
  static_assert(KindOfInt64 > KindOfBoolean, "fix checks for IsScalar");
  static_assert(KindOfDouble > KindOfBoolean, "fix checks for IsScalar");
  static_assert(KindOfStaticString > KindOfBoolean, "fix checks for IsScalar");
  static_assert(KindOfString > KindOfBoolean, "fix checks for IsScalar");

  static_assert(KindOfInt64 < KindOfString, "fix checks for IsScalar");
  static_assert(KindOfDouble < KindOfString, "fix checks for IsScalar");
  static_assert(KindOfStaticString < KindOfString, "fix checks for IsScalar");
  static_assert(KindOfArray > KindOfString, "fix checks for IsScalar");
  static_assert(KindOfObject > KindOfString, "fix checks for IsScalar");
  static_assert(KindOfResource > KindOfString, "fix checks for IsScalar");

  static_assert(sizeof(DataType) == 1, "");

  if (typeReg == InvalidReg) {
    auto const type = inst->src(0)->type();
    auto const imm = type <= (Type::Bool | Type::Int | Type::Dbl | Type::Str);
    m_as.    movl(Immed(imm), r32(dstReg));
    return;
  }

  m_as.   movzbl(rbyte(typeReg), r32(dstReg));
  m_as.   subl(KindOfBoolean, r32(dstReg));
  m_as.   subl(KindOfString - KindOfBoolean + 1, r32(dstReg));
  m_as.   sbbl(r32(dstReg), r32(dstReg));
  m_as.   neg(dstReg);
}

void CodeGenerator::cgIsNType(IRInstruction* inst) {
  cgIsTypeCommon(inst, true);
}

void CodeGenerator::cgIsTypeMem(IRInstruction* inst) {
  cgIsTypeMemCommon(inst, false);
}

void CodeGenerator::cgIsNTypeMem(IRInstruction* inst) {
  cgIsTypeMemCommon(inst, true);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Check instanceof using instance bitmasks.
 *
 * Note it's not necessary to check whether the test class is defined:
 * if it doesn't exist than the candidate can't be an instance of it
 * and will fail this check.
 */
void CodeGenerator::emitInstanceBitmaskCheck(IRInstruction* inst) {
  auto const rObjClass     = srcLoc(0).reg(0);
  auto const testClassName = inst->src(1)->strVal();
  auto& a = m_as;

  int offset;
  uint8_t mask;
  if (!InstanceBits::getMask(testClassName, offset, mask)) {
    always_assert(!"cgInstanceOfBitmask had no bitmask");
  }
  a.    testb  (int8_t(mask), rObjClass[offset]);
}

void CodeGenerator::cgInstanceOfBitmask(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceBitmaskCheck(inst);
  a.    setnz  (rbyte(dstLoc(0).reg()));
}

void CodeGenerator::cgNInstanceOfBitmask(IRInstruction* inst) {
  auto& a = m_as;
  emitInstanceBitmaskCheck(inst);
  a.    setz   (rbyte(dstLoc(0).reg()));
}

void CodeGenerator::cgJmpInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitFwdJcc(CC_NZ, inst->taken());
}

void CodeGenerator::cgJmpNInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitFwdJcc(CC_Z, inst->taken());
}

void CodeGenerator::cgReqBindJmpInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgReqBindJmpNInstanceOfBitmask(IRInstruction* inst) {
  emitInstanceBitmaskCheck(inst);
  emitReqBindJcc(opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgSideExitJmpInstanceOfBitmask(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitInstanceBitmaskCheck(inst);
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgSideExitJmpNInstanceOfBitmask(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitInstanceBitmaskCheck(inst);
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgInstanceOf(IRInstruction* inst) {
  auto testReg = srcLoc(1).reg();
  auto destReg = dstLoc(0).reg();
  auto& a = m_as;

  if (testReg == InvalidReg) {
    // Don't need to do the null check when the class is const.
    assert(inst->src(1)->clsVal() != nullptr);
    cgCallNative(a, inst);
    return;
  }

  a.     testq   (testReg, testReg);
  ifThenElse(a, CC_NZ,
    [&](Asm& a) {
      cgCallNative(a, inst);
    },
    [&](Asm& a) {
      // testReg == 0, set dest to false (0)
      emitMovRegReg(a, testReg, destReg);
    }
  );
}

/*
 * Check instanceof using the superclass vector on the end of the
 * Class entry.
 */
void CodeGenerator::cgExtendsClass(IRInstruction* inst) {
  auto const rObjClass     = srcLoc(0).reg();
  auto const testClass     = inst->src(1)->clsVal();
  auto rTestClass          = srcLoc(1).reg();
  auto const rdst          = rbyte(dstLoc(0).reg());
  auto& a = m_as;

  Label out;
  Label notExact;
  Label falseLabel;

  if (rTestClass == InvalidReg) { // TODO(#2031606)
    rTestClass = m_rScratch; // careful below about asm-x64 smashing this
    emitLoadImm(a, (int64_t)testClass, rTestClass);
  }

  // Test if it is the exact same class.  TODO(#2044801): we should be
  // doing this control flow at the IR level.
  if (!(testClass->attrs() & AttrAbstract)) {
    emitCmpClass(a, rTestClass, rObjClass);
    // If the test class cannot be extended, we only need to do the
    // exact check.
    if (testClass->attrs() & AttrNoOverride) {
      a.  sete(rdst);
      return;
    }
    a.    jne8   (notExact);
    a.    movb   (1, rdst);
    a.    jmp8   (out);
  }

  auto const vecOffset = Class::classVecOff() +
    sizeof(LowClassPtr) * (testClass->classVecLen() - 1);

  // Check the length of the class vectors---if the candidate's is at
  // least as long as the potential base (testClass) it might be a
  // subclass.
asm_label(a, notExact);
  a.    cmpl   (safe_cast<int32_t>(testClass->classVecLen()),
                rObjClass[Class::classVecLenOff()]);
  a.    jb8    (falseLabel);

  // If it's a subclass, rTestClass must be at the appropriate index.
  emitCmpClass(a, rTestClass, rObjClass[vecOffset]);
  a.    sete   (rdst);
  a.    jmp8   (out);

asm_label(a, falseLabel);
  a.    xorl   (r32(rdst), r32(rdst));

asm_label(a, out);
}

void CodeGenerator::cgConvDblToInt(IRInstruction* inst) {
  auto src = inst->src(0);
  auto srcReg = prepXMMReg(m_as, src, srcLoc(0), rCgXMM0);
  auto dstReg = dstLoc(0).reg();
  auto rIndef = rAsm; // not clobbered by emitLoadImm()

  constexpr uint64_t maxULongAsDouble  = 0x43F0000000000000LL;
  constexpr uint64_t maxLongAsDouble   = 0x43E0000000000000LL;

  m_as.    emitImmReg   (1, rIndef);
  m_as.    rorq         (1, rIndef); // rIndef = 0x8000000000000000
  m_as.    cvttsd2siq   (srcReg, dstReg);
  m_as.    cmpq         (rIndef, dstReg);

  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    // result > max signed int or unordered
    a.    pxor               (rCgXMM1, rCgXMM1);
    a.    ucomisd            (rCgXMM1, srcReg);

    ifThen(a, CC_B, [&] {
      // src0 > 0 (CF = 1 -> less than 0 or unordered)
      ifThen(a, CC_NP, [&] {
        emitLoadImm(a, maxULongAsDouble, rCgXMM1);
        a.   ucomisd    (rCgXMM1, srcReg);
        ifThenElse(a, CC_B, [&](Asm& a) {
          // src0 > ULONG_MAX
          a.    xorq    (dstReg, dstReg);

        }, [&](Asm& a) {
          // 0 < src0 <= ULONG_MAX
          emitLoadImm(a, maxLongAsDouble, rCgXMM1);
          emitMovRegReg(a, srcReg, rCgXMM0);

          // we know that LONG_MAX < src0 <= UINT_MAX, therefore,
          // 0 < src0 - ULONG_MAX <= LONG_MAX
          a.    subsd            (rCgXMM1, rCgXMM0);
          a.    cvttsd2siq       (rCgXMM0, dstReg);

          // We want to simulate integer overflow so we take the resulting
          // integer and flip its sign bit (NB: we don't use orq here
          // because it's possible that src0 == LONG_MAX in which case
          // cvttsd2siq will yeild an indefiniteInteger, which we would
          // like to make zero)
          a.    xorq             (rIndef, dstReg);
        });
      });
    });
  });
}

void CodeGenerator::cgConvDblToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  emitMovRegReg(m_as, srcReg, dstReg);
  m_as.shlq(1, dstReg); // 0.0 stays zero and -0.0 is now 0.0
  m_as.setne(rbyte(dstReg)); // lower byte becomes 1 if dstReg != 0
  m_as.movzbl(rbyte(dstReg), r32(dstReg));
}

void CodeGenerator::cgConvIntToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  m_as.testq(srcReg, srcReg);
  m_as.setne(rbyte(dstReg));
  m_as.movzbl(rbyte(dstReg), r32(dstReg));
}

void CodeGenerator::cgConvArrToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();

  // This will incorrectly result in "true" for a NameValueTableWrapper that is
  // empty. You can only get such a thing through very contrived PHP, so the
  // savings of a branch and a block of cold code outweights the edge-case bug.
  m_as.    cmpl  (0, srcReg[ArrayData::offsetofSize()]);
  m_as.    setcc (CC_NZ, rbyte(dstReg));
}

/*
 * emit something equivalent to testl(val, mr),
 * but with a shorter encoding (eg testb(val, mr))
 * if possible.
 */
template <typename M>
void testimm(Asm& as, uint32_t val, const M& mr) {
  int off = 0;
  auto v = val;
  while (v > 0xff && !(v & 0xff)) {
    off++;
    v >>= 8;
  }
  if (v > 0xff) {
    as.testl((int32_t)val, mr);
  } else {
    as.testb((int8_t)v, *(mr.r + off));
  }
}

void CodeGenerator::cgColIsEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assert(ty < Type::Obj &&
         ty.getClass() &&
         ty.getClass()->isCollectionClass());
  auto& a = m_as;
  a.    cmpl   (0, srcLoc(0).reg()[FAST_COLLECTION_SIZE_OFFSET]);
  a.    sete   (rbyte(dstLoc(0).reg()));
}

void CodeGenerator::cgColIsNEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assert(ty < Type::Obj &&
         ty.getClass() &&
         ty.getClass()->isCollectionClass());
  auto& a = m_as;
  a.    cmpl   (0, srcLoc(0).reg()[FAST_COLLECTION_SIZE_OFFSET]);
  a.    setne  (rbyte(dstLoc(0).reg()));
}

void CodeGenerator::cgConvObjToBool(IRInstruction* inst) {
  auto const rdst = dstLoc(0).reg();
  auto const rsrc = srcLoc(0).reg();
  auto& a = m_as;

  testimm(a, ObjectData::CallToImpl, rsrc[ObjectData::attributeOff()]);
  unlikelyIfThenElse(
    CC_NZ,
    [&] (Asm& a) {
      testimm(a,
              ObjectData::IsCollection,
              rsrc[ObjectData::attributeOff()]);
      ifThenElse(
        a,
        CC_NZ,
        [&] (Asm& a) { // rsrc points to native collection
          a.cmpl(0, rsrc[FAST_COLLECTION_SIZE_OFFSET]);
          a.setne(rbyte(rdst)); // true iff size not zero
        },
        [&] (Asm& a) { // rsrc is not a native collection
          cgCallHelper(
            a,
            CppCall::method(&ObjectData::o_toBoolean),
            callDest(inst),
            SyncOptions::kSyncPoint,
            argGroup().ssa(0));
        }
      );
    },
    [&] (Asm& a) {
      a.    movb  (1, rbyte(rdst));
    }
  );
}

void CodeGenerator::emitConvBoolOrIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  assert(src->isA(Type::Bool) || src->isA(Type::Int));
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  // cvtsi2sd doesn't modify the high bits of its target, which can
  // cause false dependencies to prevent register renaming from kicking
  // in. Break the dependency chain by zeroing out the XMM reg.
  zeroExtendIfBool(m_as, src, srcReg);
  PhysReg xmmReg = dstReg.isSIMD() ? dstReg : PhysReg(rCgXMM0);
  m_as.pxor(xmmReg, xmmReg);
  m_as.cvtsi2sd(srcReg, xmmReg);
  emitMovRegReg(m_as, xmmReg, dstReg);
}

void CodeGenerator::cgConvBoolToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvIntToDbl(IRInstruction* inst) {
  emitConvBoolOrIntToDbl(inst);
}

void CodeGenerator::cgConvBoolToInt(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  m_as.movzbl(rbyte(srcReg), r32(dstReg));
}

void CodeGenerator::cgConvBoolToStr(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  m_as.testb(rbyte(srcReg), rbyte(srcReg));
  m_as.movq(makeStaticString(""), dstReg);
  m_as.movq(makeStaticString("1"), m_rScratch);
  m_as.cmov_reg64_reg64(CC_NZ, m_rScratch, dstReg);
}

void CodeGenerator::cgConvClsToCctx(IRInstruction* inst) {
  auto const sreg = srcLoc(0).reg();
  auto const dreg = dstLoc(0).reg();
  auto& a = m_as;
  emitMovRegReg(a, sreg, dreg);
  a.    orq   (1, dreg);
}

void CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(srcReg != InvalidReg);
  emitMovRegReg(m_as, srcReg, dstReg);
  emitDerefIfVariant(m_as, PhysReg(dstReg));
}

void CodeGenerator::cgLdFuncCachedCommon(IRInstruction* inst) {
  auto const dst  = dstLoc(0).reg();
  auto const name = inst->extra<LdFuncCachedData>()->name;
  auto const ch   = Unit::GetNamedEntity(name)->getFuncHandle();
  auto& a = m_as;

  if (dst == InvalidReg) {
    a.   cmpq  (0, rVmTl[ch]);
  } else {
    a.   loadq (rVmTl[ch], dst);
    a.   testq (dst, dst);
  }
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst);
  unlikelyIfBlock(CC_Z, [&] (Asm& a) {
    const Func* (*const func)(const StringData*) = lookupUnknownFunc;
    cgCallHelper(
      a,
      CppCall::direct(func),
      callDest(inst),
      SyncOptions::kSyncPoint,
      argGroup()
        .immPtr(inst->extra<LdFuncCached>()->name)
    );
  });
}

void CodeGenerator::cgLdFuncCachedSafe(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst);
  if (auto const taken = inst->taken()) {
    emitFwdJcc(m_as, CC_Z, taken);
  }
}

void CodeGenerator::cgLdFuncCachedU(IRInstruction* inst) {
  auto const dstReg    = dstLoc(0).reg();
  auto const extra     = inst->extra<LdFuncCachedU>();
  auto const hFunc     = Unit::GetNamedEntity(extra->name)->getFuncHandle();

  auto& a = m_as;

  // Check the first function handle, otherwise try to autoload.
  if (dstReg == InvalidReg) {
    a.   cmpq  (0, rVmTl[hFunc]);
  } else {
    a.   loadq (rVmTl[hFunc], dstReg);
    a.   testq (dstReg, dstReg);
  }

  unlikelyIfBlock(CC_Z, [&] (Asm& a) {
    // If we get here, things are going to be slow anyway, so do all the
    // autoloading logic in lookupFallbackFunc instead of ASM
    const Func* (*const func)(const StringData*, const StringData*) =
        lookupFallbackFunc;
    cgCallHelper(
      a,
      CppCall::direct(func),
      callDest(inst),
      SyncOptions::kSyncPoint,
      argGroup()
        .immPtr(extra->name)
        .immPtr(extra->fallback)
    );
  });
}

void CodeGenerator::cgLdFunc(IRInstruction* inst) {
  auto const ch = FuncCache::alloc();
  RDS::recordRds(ch, sizeof(FuncCache),
                 "FuncCache", curFunc()->fullName()->data());

  // raises an error if function not found
  cgCallHelper(m_as,
               CppCall::direct(FuncCache::lookup),
               callDest(dstLoc(0).reg()),
               SyncOptions::kSyncPoint,
               argGroup().imm(ch).ssa(0/*methodName*/));
}

void CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto objReg = srcLoc(0).reg();
  emitLdObjClass(m_as, objReg, dstReg);
}

void CodeGenerator::cgLdObjMethod(IRInstruction* inst) {
  using namespace MethodCache;

  auto const clsReg    = srcLoc(0).reg();
  auto const actRecReg = srcLoc(1).reg();
  auto const extra     = inst->extra<LdObjMethodData>();
  auto& a = m_as;

  auto const handle = RDS::alloc<Entry, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    auto const caddr_hand = reinterpret_cast<char*>(
      static_cast<intptr_t>(handle)
    );
    Debug::DebugInfo::recordDataMap(
      caddr_hand,
      caddr_hand + sizeof(TypedValue),
      folly::format("rds+MethodCache-{}",
        curFunc()->fullName()->data()).str());
  }

  auto const mcHandler = extra->fatal ? handlePrimeCacheInit<true>
                                      : handlePrimeCacheInit<false>;

  Label slow_path;
  Label done;

  /*
   * Inline cache: we "prime" the cache across requests by smashing
   * this immediate to hold a Func* in the upper 32 bits, and a Class*
   * in the lower 32 bits.  (If both are low-malloced pointers can
   * fit.)  See pmethodCacheMissPath.
   */
  mcg->backEnd().prepareForSmash(a.code(), kMovLen);
  auto const movAddr = a.frontier();
  a.    movq   (0x8000000000000000u, rAsm);
  assert(a.frontier() - kMovLen == movAddr);

  a.    movq   (rAsm, m_rScratch);
  a.    andl   (r32(rAsm), r32(rAsm));  // zero the top 32 bits
  a.    cmpq   (rAsm, clsReg);
  a.    jne8   (slow_path);
  a.    shrq   (32, m_rScratch);
  a.    storeq (m_rScratch, actRecReg[AROFF(m_func)]);
  a.    jmp8   (done);
asm_label(a, slow_path);
  cgCallHelper(
    a,
    CppCall::direct(mcHandler),
    kVoidDest,
    SyncOptions::kSmashableAndSyncPoint,
    argGroup()
      .addr(rVmTl, safe_cast<int32_t>(handle))
      .ssa(1/*actRec*/)
      .immPtr(extra->method)
      .ssa(0/*cls*/)
      .immPtr(curClass())
      // The scratch reg contains the prime data before we've smashed the call
      // to handleSlowPath.  After, it contains the primed Class/Func pair.
      .reg(m_rScratch)
  );
asm_label(a, done);

  /*
   * For the first time through, set the cache to hold the offset from
   * the return address of the call to the movq (*2 + 1), so we can find
   * the movq from the handler.
   *
   * We set the low bit for two reasons: the Class* will never be a valid
   * Class*, so we'll always miss the inline check before it's smashed, and
   * handlePrimeCacheMiss can tell it's not been smashed yet
   */
  auto movAddrUInt = reinterpret_cast<uintptr_t>(movAddr);
  *reinterpret_cast<uint64_t*>(movAddr + kMovImmOff) =
    (movAddrUInt << 1) | 1;
  mcg->cgFixups().m_addressImmediates.insert(
    reinterpret_cast<TCA>(~movAddrUInt));
}

void CodeGenerator::cgLdObjInvoke(IRInstruction* inst) {
  auto const rsrc = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();
  auto& a = m_as;

  a.   loadq  (rsrc[Class::invokeOff()], rdst);
  a.   testq  (rdst, rdst);
  emitFwdJcc  (a, CC_Z, inst->taken());
}

void CodeGenerator::cgStRetVal(IRInstruction* inst) {
  auto  const rFp = srcLoc(0).reg();
  auto* const val = inst->src(1);
  cgStore(rFp[AROFF(m_r)], val, srcLoc(1), Width::Full);
}

void CodeGenerator::cgRetAdjustStack(IRInstruction* inst) {
  auto const rFp   = srcLoc(0).reg();
  auto const dstSp = dstLoc(0).reg();
  auto& a = m_as;
  a.    lea   (rFp[AROFF(m_r)], dstSp);
}

void CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg(0);
  assert(fpReg != InvalidReg);
  m_as.push(fpReg[AROFF(m_savedRip)]);
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == mcg->tx().uniqueStubs.callToExit) {
    return;
  }
  checkFrame(fp, sp, /*checkLocals*/ false);
  assert(sp <= (Cell*)fp || fp->resumed());
  // check return value if stack not empty
  if (sp < (Cell*)fp) assertTv(sp);
}

void CodeGenerator::emitTraceRet(Asm& a) {
  // call to a trace function
  a.    movq  (rVmFp, rdi);
  a.    movq  (rVmSp, rsi);
  a.    loadq (*rsp, rdx); // return ip from native stack
  // do the call; may use a trampoline
  emitCall(a, TCA(traceRet));
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  // Make sure rVmFp and rVmSp are set appropriately
  emitMovRegReg(m_as, srcLoc(0/*sp*/).reg(), rVmSp);
  emitMovRegReg(m_as, srcLoc(1/*fp*/).reg(), rVmFp);

  // Return control to caller
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitTraceRet(m_as);
  }

  m_as.ret();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    m_as.ud2();
  }
}

void CodeGenerator::emitReqBindAddr(TCA& dest,
                                    SrcKey sk) {
  mcg->setJmpTransID((TCA)&dest);

  dest = emitEphemeralServiceReq(m_frozenCode,
                                 mcg->getFreeStub(m_frozenCode,
                                                  &mcg->cgFixups()),
                                 REQ_BIND_ADDR,
                                 &dest,
                                 sk.toAtomicInt(),
                                 TransFlags{}.packed);
  mcg->cgFixups().m_codePointers.insert(&dest);
}

void CodeGenerator::cgLdBindAddr(IRInstruction* inst) {
  auto data   = inst->extra<LdBindAddr>();
  auto dstReg = dstLoc(0).reg();

  // Emit service request to smash address of SrcKey into 'addr'.
  TCA* addrPtr = mcg->allocData<TCA>(sizeof(TCA), 1);
  emitReqBindAddr(*addrPtr, data->sk);

  // Load the maybe bound address.
  auto addr = intptr_t(addrPtr);
  // the tc/global data is intentionally layed out to guarantee
  // rip-relative addressing will work.
  // Also, a rip-relative load, is 1 byte smaller than the corresponding
  // baseless load.
  m_as.loadq(rip[addr], dstReg);
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  JmpSwitchData* data = inst->extra<JmpSwitchDest>();
  SSATmp* index       = inst->src(0);
  auto indexReg       = srcLoc(0).reg();
  auto rTmp           = m_rScratch;

  if (!index->isConst()) {
    if (data->bounded) {
      if (data->base) {
        if (deltaFits(data->base, sz::dword)) {
          m_as.  subq(safe_cast<int32_t>(data->base), indexReg);
        } else {
          m_as.  emitImmReg(data->base, rTmp);
          m_as.  subq(rTmp, indexReg);
        }
      }
      m_as.    cmpq(data->cases - 2, indexReg);
      mcg->backEnd().prepareForSmash(m_mainCode, kJmpccLen);
      TCA def = emitEphemeralServiceReq(
        m_frozenCode,
        mcg->getFreeStub(m_frozenCode, &mcg->cgFixups()),
        REQ_BIND_JMPCC_SECOND,
        RipRelative(m_as.frontier()),
        data->defaultOff,
        CC_AE);
      mcg->setJmpTransID(m_as.frontier());

      m_as.    jae(def);
    }

    TCA* table = mcg->allocData<TCA>(sizeof(TCA), data->cases);
    m_as.   lea(rip[(intptr_t)table], m_rScratch);
    assert(((int32_t*)m_as.frontier())[-1] + m_as.frontier() == (TCA)table);
    m_as.   jmp(m_rScratch[indexReg*8]);

    for (int i = 0; i < data->cases; i++) {
      emitReqBindAddr(table[i], SrcKey(curFunc(), data->targets[i], resumed()));
    }
  } else {
    int64_t indexVal = index->intVal();

    if (data->bounded) {
      indexVal -= data->base;
      if (indexVal >= data->cases - 2 || indexVal < 0) {
        emitBindJmp(m_mainCode, m_frozenCode,
                    SrcKey(curFunc(), data->defaultOff, resumed()));
        return;
      }
    }
    emitBindJmp(m_mainCode, m_frozenCode,
                SrcKey(curFunc(), data->targets[indexVal], resumed()));
  }
}

void CodeGenerator::cgLdSSwitchDestFast(IRInstruction* inst) {
  auto data = inst->extra<LdSSwitchDestFast>();

  auto table = mcg->allocData<SSwitchMap>(64);
  new (table) SSwitchMap(data->numCases);
  for (int64_t i = 0; i < data->numCases; ++i) {
    table->add(data->cases[i].str, nullptr);
    TCA* addr = table->find(data->cases[i].str);
    emitReqBindAddr(*addr, SrcKey(curFunc(), data->cases[i].dest, resumed()));
  }
  TCA* def = mcg->allocData<TCA>(sizeof(TCA), 1);
  emitReqBindAddr(*def, SrcKey(curFunc(), data->defaultOff, resumed()));

  cgCallHelper(m_as,
               CppCall::direct(sswitchHelperFast),
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup()
                 .ssa(0)
                 .immPtr(table)
                 .immPtr(def));
}

static TCA sswitchHelperSlow(TypedValue typedVal,
                             const StringData** strs,
                             int numStrs,
                             TCA* jmptab) {
  Cell* cell = tvToCell(&typedVal);
  for (int i = 0; i < numStrs; ++i) {
    if (cellEqual(*cell, strs[i])) return jmptab[i];
  }
  return jmptab[numStrs]; // default case
}

void CodeGenerator::cgLdSSwitchDestSlow(IRInstruction* inst) {
  auto data = inst->extra<LdSSwitchDestSlow>();

  auto strtab = mcg->allocData<const StringData*>(
    sizeof(const StringData*), data->numCases);
  auto jmptab = mcg->allocData<TCA>(sizeof(TCA), data->numCases + 1);
  for (int i = 0; i < data->numCases; ++i) {
    strtab[i] = data->cases[i].str;
    emitReqBindAddr(jmptab[i],
                    SrcKey(curFunc(), data->cases[i].dest, resumed()));
  }
  emitReqBindAddr(jmptab[data->numCases],
                  SrcKey(curFunc(), data->defaultOff, resumed()));

  cgCallHelper(m_as,
               CppCall::direct(sswitchHelperSlow),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup()
                 .typedValue(0)
                 .immPtr(strtab)
                 .imm(data->numCases)
                 .immPtr(jmptab));
}

/*
 * It'd be nice not to have the cgMov here (and just copy propagate
 * the source or something), but for now we're keeping it allocated to
 * rVmFp so inlined calls to C++ helpers that use the rbp chain to
 * find the caller's ActRec will work correctly.
 *
 * This instruction primarily exists to assist in optimizing away
 * unused activation records, so it's usually not going to happen
 * anyway.
 */
void CodeGenerator::cgDefInlineFP(IRInstruction* inst) {
  auto const calleeFP = srcLoc(0).reg();
  auto const callerFP = srcLoc(2).reg();
  auto const fakeRet  = mcg->tx().uniqueStubs.retInlHelper;
  auto const retBCOff = inst->extra<DefInlineFP>()->retBCOff;

  m_as.    storeq (callerFP, calleeFP[AROFF(m_sfp)]);
  emitImmStoreq(m_as, intptr_t(fakeRet), calleeFP[AROFF(m_savedRip)]);
  m_as.    storel (retBCOff, calleeFP[AROFF(m_soff)]);
  cgMov(inst);
}

void CodeGenerator::cgInlineReturn(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  assert(fpReg == rVmFp);
  m_as.    loadq  (fpReg[AROFF(m_sfp)], rVmFp);
}

void CodeGenerator::cgReDefSP(IRInstruction* inst) {
  // TODO(#2288359): this instruction won't be necessary (for
  // non-generator frames) when we don't track rVmSp independently
  // from rVmFp.  In generator frames we'll have to track offsets from
  // a DefResumableSP or something similar.
  auto fp  = srcLoc(1).reg();
  auto dst = dstLoc(0).reg();
  auto off = -inst->extra<ReDefSP>()->spOffset * sizeof(Cell);
  emitLea(m_as, fp[off], dst);
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto off = AROFF(m_sfp);
  auto dst = dstLoc(0).reg();
  m_as.loadq(ptr[off], dst);
}

void emitSpill(Asm& as, const PhysLoc& s, const PhysLoc& d, Type t) {
  assert(s.numWords() == d.numWords());
  assert(!s.spilled() && d.spilled());
  if (s.isFullSIMD()) {
    as.movdqu(s.reg(0), reg::rsp[d.offset(0)]);
  } else {
    for (int i = 0, n = s.numAllocated(); i < n; ++i) {
      // store the whole register even if it holds a bool or DataType
      emitStoreReg(as, s.reg(i), reg::rsp[d.offset(i)]);
    }
  }
}

void emitReload(Asm& as, const PhysLoc& s, const PhysLoc& d, Type t) {
  assert(s.numWords() == d.numWords());
  assert(s.spilled() && !d.spilled());
  if (d.isFullSIMD()) {
    as.movdqu(reg::rsp[s.offset(0)], d.reg(0));
  } else {
    for (int i = 0, n = d.numAllocated(); i < n; ++i) {
      // load the whole register even if it holds a bool or DataType
      emitLoadReg(as, reg::rsp[s.offset(i)], d.reg(i));
    }
  }
}

void CodeGenerator::cgShuffle(IRInstruction* inst) {
  // Each destination is unique, there are no mem-mem copies, and
  // there are no cycles involving spill slots.  So do the shuffling
  // in this order:
  // 1. reg->mem (stores)
  // 2. reg->reg (parallel copies)
  // 3. mem->reg (loads) & imm->reg (constants)
  PhysReg::Map<PhysReg> moves;    // moves[dst] = src
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; ++i) {
    auto& rd = inst->extra<Shuffle>()->dests[i];
    if (rd.numAllocated() == 0) continue; // ignore unused dests.
    auto src = inst->src(i);
    auto rs = srcLoc(i);
    if (rd.spilled()) {
      emitSpill(m_as, rs, rd, src->type());
    } else if (!rs.spilled()) {
      auto s0 = rs.reg(0);
      auto d0 = rd.reg(0);
      if (s0 != InvalidReg) moves[d0] = s0;
      auto s1 = rs.reg(1);
      auto d1 = rd.reg(1);
      if (s1 != InvalidReg) moves[d1] = s1;
    }
  }
  // Compute a serial order of moves and swaps.  We can use m_rScratch
  // here since cgShuffle is a standalone HHIR instruction, and sometimes
  // its a low-numbered register.
  auto rTmp = m_rScratch;
  auto howTo = doRegMoves(moves, rTmp);
  for (auto& how : howTo) {
    if (how.m_kind == MoveInfo::Kind::Move) {
      emitMovRegReg(m_as, how.m_src, how.m_dst);
    } else {
      // do swap - only support GPRs
      assert(how.m_src.isGP() && how.m_dst.isGP());
      m_as.xchgq(how.m_src, how.m_dst);
    }
  }
  // now do reg<-mem loads and reg<-imm moves. We have already
  // dealt with stores, moves, and swaps, so all rTmp is available
  // even if it originally contained a value.
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; ++i) {
    auto src = inst->src(i);
    auto rs = srcLoc(i);
    auto& rd = inst->extra<Shuffle>()->dests[i];
    if (rd.numAllocated() == 0) continue; // ignore unused dests.
    if (rd.spilled()) continue;
    if (rs.spilled()) {
      emitReload(m_as, rs, rd, src->type());
      continue;
    }
    if (rs.numAllocated() == 0) {
      assert(src->isConst());
      auto r = rd.reg(0);
      auto imm = src->type().needsValueReg() ? src->rawVal() :
                 0xdeadbeef;
      if (src->type().needsValueReg() ||
          RuntimeOption::EvalHHIRGenerateAsserts) {
        if (r.isGP()) {
          // never needs scratch register
          m_as.emitImmReg(imm, r);
        } else {
          // load imm -> simd.  We could do this without a scratch
          // using a pc-relative load.
          m_as.emitImmReg(imm, rTmp);
          emitMovRegReg(m_as, rTmp, r);
        }
      }
    }
    if (rd.numAllocated() == 2 && rs.numAllocated() < 2) {
      // move a src known type to a dest register
      //         a.emitImmReg(args[i].imm().q(), dst);
      assert(src->type().isKnownDataType());
      m_as.emitImmReg(src->type().toDataType(), rd.reg(1));
    }
  }
}

void CodeGenerator::cgStProp(IRInstruction* inst) {
  auto objReg = srcLoc(0).reg();
  auto propOff  = inst->src(1)->intVal();
  cgStore(objReg[propOff], inst->src(2), srcLoc(2), Width::Full);
}

void CodeGenerator::cgStMem(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto offset = inst->src(1)->intVal();
  cgStore(ptr[offset], inst->src(2), srcLoc(2), Width::Full);
}

void CodeGenerator::cgStRef(IRInstruction* inst) {
  always_assert(!srcLoc(1).isFullSIMD());
  auto destReg = dstLoc(0).reg();
  auto ptr = srcLoc(0).reg();
  auto off = RefData::tvOffset();
  cgStore(ptr[off], inst->src(1), srcLoc(1), Width::Full);
  if (destReg != InvalidReg) emitMovRegReg(m_as, ptr, destReg);
}

int CodeGenerator::iterOffset(uint32_t id) {
  const Func* func = curFunc();
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

void CodeGenerator::cgStLoc(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto off = localOffset(inst->extra<StLoc>()->locId);
  cgStore(ptr[off], inst->src(1), srcLoc(1), Width::Full);
}

void CodeGenerator::cgStLocNT(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto off = localOffset(inst->extra<StLocNT>()->locId);
  cgStore(ptr[off], inst->src(1), srcLoc(1), Width::Value);
}

void CodeGenerator::cgSyncABIRegs(IRInstruction* inst) {
  emitMovRegReg(m_as, srcLoc(0).reg(), rVmFp);
  emitMovRegReg(m_as, srcLoc(1).reg(), rVmSp);
}

void CodeGenerator::cgEagerSyncVMRegs(IRInstruction* inst) {
  always_assert(
    srcLoc(0).reg() == rVmFp &&
    srcLoc(1).reg() == rVmSp
  );
  emitEagerSyncPoint(
    m_as,
    reinterpret_cast<const Op*>(inst->marker().sk().pc())
  );
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  auto offset  = inst->extra<ReqBindJmp>()->offset;
  auto trflags = inst->extra<ReqBindJmp>()->trflags;

  emitBindJmp(
    m_mainCode,
    m_frozenCode,
    SrcKey(curFunc(), offset, resumed()),
    trflags
  );
}

void CodeGenerator::cgReqRetranslateOpt(IRInstruction* inst) {
  auto extra = inst->extra<ReqRetranslateOpt>();

  TCA sr = emitServiceReq(
    m_coldCode, REQ_RETRANSLATE_OPT,
    SrcKey(curFunc(), extra->offset, resumed()).toAtomicInt(),
    extra->transId);
  if (m_mainCode.frontier() != m_coldCode.frontier()) {
    m_as.jmp(sr);
  }
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  assert(m_unit.bcOff() == inst->marker().bcOff());
  auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
  auto const destSR = mcg->tx().getSrcRec(destSK);

  auto trflags = inst->extra<ReqRetranslate>()->trflags;

  if (trflags.packed == 0) {
    destSR->emitFallbackJump(m_mainCode);
  } else {
    destSR->emitFallbackJumpCustom(m_mainCode, m_frozenCode, destSK, trflags);
  }
}

void CodeGenerator::cgIncRefWork(Type type, SSATmp* src, PhysLoc srcLoc) {
  assert(type.maybeCounted());
  auto increfMaybeStatic = [&] {
    auto base = srcLoc.reg(0);
    if (!type.needsStaticBitCheck()) {
      emitIncRef(m_as, base);
    } else {
      m_as.cmpl(0, base[FAST_REFCOUNT_OFFSET]);
      static_assert(UncountedValue < 0 && StaticValue < 0, "");
      ifThen(m_as, CC_NS, [&] { emitIncRef(m_as, base); });
    }
  };

  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic();
  } else {
    emitCmpTVType(m_as, KindOfRefCountThreshold, srcLoc.reg(1));
    ifThen(m_as, CC_NLE, [&] { increfMaybeStatic(); });
  }
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  Type type   = src->type();

  if (type.notCounted()) return;

  cgIncRefWork(type, src, srcLoc(0));
}

void CodeGenerator::cgIncRefCtx(IRInstruction* inst) {
  if (inst->src(0)->isA(Type::Obj)) return cgIncRef(inst);

  auto const src = srcLoc(0).reg();
  auto& a = m_as;

  a.    testb  (0x1, rbyte(src));
  ifThen(a, CC_Z, [&] {
    emitIncRef(a, src);
  });
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  cgDecRefMem(inst->typeParam(),
              srcLoc(0).reg(),
              cellsToBytes(inst->extra<DecRefStack>()->offset));
}

void CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  auto scratchReg = m_rScratch;

  // Load AR->m_this into m_rScratch
  m_as.loadq(fpReg[AROFF(m_this)], scratchReg);

  auto decrefIfAvailable = [&] {
    // Check if this is available and we're not in a static context instead
    m_as.testb(1, rbyte(scratchReg));
    ifThen(m_as, CC_Z, [&] {
      cgDecRefStaticType(
        Type::Obj,
        scratchReg,
        true /* genZeroCheck */
      );
    });
  };

  if (curFunc()->isPseudoMain()) {
    // In pseudo-mains, emit check for presence of m_this
    m_as.testq(scratchReg, scratchReg);
    ifThen(m_as, CC_NZ, [&] { decrefIfAvailable(); });
  } else {
    decrefIfAvailable();
  }
}

void CodeGenerator::cgDecRefLoc(IRInstruction* inst) {
  cgDecRefMem(inst->typeParam(),
              srcLoc(0).reg(),
              localOffset(inst->extra<DecRefLoc>()->locId));
}

void CodeGenerator::cgGenericRetDecRefs(IRInstruction* inst) {
  auto const rFp       = srcLoc(0).reg();
  auto const numLocals = curFunc()->numLocals();
  auto& a = m_as;

  assert(rFp == rVmFp &&
         "free locals helper assumes the frame pointer is rVmFp");

  if (numLocals == 0) return;

  // The helpers called below use a special ABI, in which r14 and r15 is
  // not saved, and the stub expects the stack to be imbalanced (RSP%16==0)
  // on entry. So save r14 and r15 in addition to the caller-save registers,
  // and use PhysRegSaverStub which assumes the odd stack parity.
  auto toSave = m_state.liveRegs[inst] &
                (kCallerSaved | RegSet(r14) | RegSet(r15));
  PhysRegSaverStub saver(a, toSave);

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? mcg->tx().uniqueStubs.freeManyLocalsHelper
    : mcg->tx().uniqueStubs.freeLocalsHelpers[numLocals - 1];

  a.lea(rFp[-numLocals * sizeof(TypedValue)], r14);
  a.call(target);
  recordSyncPoint(a);
}

/*
 * Depending on the current translation kind, do nothing, profile, or collect
 * profiling data for the current DecRef* instruction
 *
 * Returns true iff the release path for this DecRef should be put in cold
 * code.
 */
bool CodeGenerator::decRefDestroyIsUnlikely(OptDecRefProfile& profile,
                                            Type type) {
  auto const kind = mcg->tx().mode();
  if (kind != TransKind::Profile && kind != TransKind::Optimize) return true;

  // For a profiling key, we use:
  // "DecRefProfile-{opcode name}-{stack/local id if present}-{type}"
  // This gives good uniqueness within a bytecode without requiring us to track
  // more complex things like "this is the 3rd DecRef in this bytecode".
  const int32_t profileId =
    m_curInst->is(DecRefLoc) ? m_curInst->extra<DecRefLoc>()->locId
  : m_curInst->is(DecRefStack) ? m_curInst->extra<DecRefStack>()->offset
  : 0;
  auto const profileKey =
    makeStaticString(folly::to<std::string>("DecRefProfile-",
                                            opcodeName(m_curInst->op()),
                                            '-',
                                            profileId,
                                            '-',
                                            type.toString()));
  profile.emplace(m_unit.context(), m_curInst->marker(), profileKey);

  if (profile->profiling()) {
    m_as.incw (rVmTl[profile->handle() + offsetof(DecRefProfile, decrement)]);
  } else if (profile->optimizing()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.hitRate() != 0 && data.hitRate() != 100) {
      // These are the only interesting cases where we could be doing better.
      FTRACE(5, "DecRefProfile: {}: {} {}\n",
             data, m_curInst->marker().show(), profileKey->data());
    }

    if (data.hitRate() == 0) {
      emitIncStat(m_mainCode, Stats::TC_DecRef_Profiled_0);
    } else if (data.hitRate() == 100) {
      emitIncStat(m_mainCode, Stats::TC_DecRef_Profiled_100);
    }

    return data.hitRate() < RuntimeOption::EvalJitUnlikelyDecRefPercent;
  }

  return true;
}

namespace {
template <typename T>
struct CheckValid {
  static bool valid(const T& f) { return true; }
};

template <>
struct CheckValid<void(*)(Asm&)> {
  static bool valid(void (*f)(Asm&)) { return f; }
};
}

//
// Using the given dataReg, this method generates code that checks the static
// bit out of dataReg, and emits a DecRef if needed.
// NOTE: the flags are left with the result of the DecRef's subtraction,
//       which can then be tested immediately after this.
//
// We've tried a variety of tweaks to this and found the current state of
// things optimal, at least when the measurements were made:
// - whether to load the count into a register (if one is available)
// - whether to use if (!--count) release(); if we don't need a static check
// - whether to skip using the register and just emit --count if we know
//   its not static, and can't hit zero.
//
// Return value: the address to be patched if a RefCountedStaticValue check is
//               emitted; NULL otherwise.
//
template <typename F>
Address CodeGenerator::cgCheckStaticBitAndDecRef(Type type,
                                                 PhysReg dataReg,
                                                 F destroyImpl) {
  always_assert(type.maybeCounted());
  bool hasDestroy = CheckValid<F>::valid(destroyImpl);

  OptDecRefProfile profile;
  auto const unlikelyDestroy =
    hasDestroy ? decRefDestroyIsUnlikely(profile, type) : false;

  if (hasDestroy) {
    emitIncStat(m_mainCode, unlikelyDestroy ? Stats::TC_DecRef_Normal_Decl
                                            : Stats::TC_DecRef_Likely_Decl);
  } else {
    emitIncStat(m_mainCode, Stats::TC_DecRef_NZ);
  }

  auto destroy = [&](Asm& a) {
    emitIncStat(a.code(), unlikelyDestroy ? Stats::TC_DecRef_Normal_Destroy
                                          : Stats::TC_DecRef_Likely_Destroy);
    if (profile && profile->profiling()) {
      a.incw (rVmTl[profile->handle() + offsetof(DecRefProfile, destroy)]);
    }
    destroyImpl(a);
  };

  if (!type.needsStaticBitCheck()) {
    m_as.decl(dataReg[FAST_REFCOUNT_OFFSET]);
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is not less than zero
      emitAssertFlagsNonNegative(m_as);
    }

    if (hasDestroy) ifBlock(CC_E, destroy, unlikelyDestroy);
    return nullptr;
  }

  Address addrToPatch = nullptr;
  auto static_check_and_decl = [&](Asm& as) {
    static_assert(UncountedValue == UNCOUNTED, "");
    static_assert(StaticValue == STATIC, "");

    if (type.needsStaticBitCheck()) {
      addrToPatch = as.frontier();
      as.jcc8(CC_L, addrToPatch);
    }

    // Decrement _count
    as.decl(dataReg[FAST_REFCOUNT_OFFSET]);

    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is not less than zero
      emitAssertFlagsNonNegative(as);
    }
  };

  if (hasDestroy) {
    m_as.cmpl(1, dataReg[FAST_REFCOUNT_OFFSET]);
    ifThenElse(CC_E, destroy, static_check_and_decl, unlikelyDestroy);
    return addrToPatch;
  } else if (type.needsStaticBitCheck()) {
    m_as.cmpl(0, dataReg[FAST_REFCOUNT_OFFSET]);
  }

  static_check_and_decl(m_as);

  return addrToPatch;
}

Address CodeGenerator::cgCheckStaticBitAndDecRef(Type type,
                                                 PhysReg dataReg) {
  return cgCheckStaticBitAndDecRef(type, dataReg, (void (*)(Asm&))nullptr);
}

//
// Returns the address to be patched with the address to jump to in case
// the type is not ref-counted.
//
Address CodeGenerator::cgCheckRefCountedType(PhysReg typeReg) {
  emitCmpTVType(m_as, KindOfRefCountThreshold, typeReg);
  Address addrToPatch = m_as.frontier();
  m_as.jcc8(CC_LE, addrToPatch);
  return addrToPatch;
}

Address CodeGenerator::cgCheckRefCountedType(PhysReg baseReg, int64_t offset) {
  emitCmpTVType(m_as, KindOfRefCountThreshold, baseReg[offset + TVOFF(m_type)]);
  Address addrToPatch = m_as.frontier();
  m_as.jcc8(CC_LE, addrToPatch);
  return addrToPatch;
}

//
// Generates dec-ref of a typed value with statically known type.
//
void CodeGenerator::cgDecRefStaticType(Type type,
                                       PhysReg dataReg,
                                       bool genZeroCheck) {
  assert(type != Type::Cell && type != Type::Gen);
  assert(type.isKnownDataType());

  if (type.notCounted()) return;

  // Check for UncountedValue or StaticValue if needed,
  // do the actual DecRef, and leave flags set based on the subtract result,
  // which is tested below
  Address patchStaticCheck = nullptr;
  if (genZeroCheck) {
    patchStaticCheck = cgCheckStaticBitAndDecRef(
      type, dataReg, [&] (Asm& a) {
        // Emit the call to release in m_acold
        cgCallHelper(a,
                     mcg->getDtorCall(type.toDataType()),
                     kVoidDest,
                     SyncOptions::kSyncPoint,
                     argGroup()
                     .reg(dataReg));
      });
  } else {
    patchStaticCheck = cgCheckStaticBitAndDecRef(type, dataReg);
  }

  if (patchStaticCheck) {
    m_as.patchJcc8(patchStaticCheck, m_as.frontier());
  }
}

//
// Generates dec-ref of a typed value with dynamic (statically unknown) type,
// when the type is stored in typeReg.
//
void CodeGenerator::cgDecRefDynamicType(PhysReg typeReg,
                                        PhysReg dataReg,
                                        bool genZeroCheck) {
  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(typeReg);

  // Emit check for UncountedValue or StaticValue and the actual DecRef
  Address patchStaticCheck;
  if (genZeroCheck) {
    patchStaticCheck =
      cgCheckStaticBitAndDecRef(
        Type::Cell, dataReg,
        [&] (Asm& a) {
          // load destructor fptr
          loadDestructorFunc(a, typeReg, m_rScratch);
          // Emit call to release in m_acold
          cgCallHelper(a,
                       CppCall::indirect(m_rScratch),
                       kVoidDest,
                       SyncOptions::kSyncPoint,
                       argGroup()
                       .reg(dataReg));
        });
  } else {
    patchStaticCheck =
      cgCheckStaticBitAndDecRef(Type::Cell, dataReg);
  }

  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.frontier());
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.frontier());
}

//
// Generates dec-ref of a typed value with dynamic (statically
// unknown) type, when all we have is the baseReg and offset of
// the typed value. This method assumes that baseReg is not the
// scratch register.
//
void CodeGenerator::cgDecRefDynamicTypeMem(PhysReg baseReg,
                                           int64_t offset) {
  auto scratchReg = m_rScratch;

  assert(baseReg != scratchReg);

  // Emit check for ref-counted type
  Address patchTypeCheck = cgCheckRefCountedType(baseReg, offset);

  m_as.loadq(baseReg[offset + TVOFF(m_data)], scratchReg);

  // Emit check for UncountedValue or StaticValue and the actual DecRef
  Address patchStaticCheck =
      cgCheckStaticBitAndDecRef(Type::Cell, scratchReg, [&](Asm& a) {
        // Emit call to release in m_acold
        a.lea(baseReg[offset], scratchReg);
        cgCallHelper(a,
                     CppCall::direct(tv_release_generic),
                     kVoidDest,
                     SyncOptions::kSyncPoint,
                     argGroup().reg(scratchReg));
      });

  // Patch checks to jump around the DecRef
  if (patchTypeCheck)   m_as.patchJcc8(patchTypeCheck,   m_as.frontier());
  if (patchStaticCheck) m_as.patchJcc8(patchStaticCheck, m_as.frontier());
}

//
// Generates the dec-ref of a typed value in memory address [baseReg + offset].
// This handles cases where type is either static or dynamic.
//
void CodeGenerator::cgDecRefMem(Type type,
                                PhysReg baseReg,
                                int64_t offset) {
  if (type.notCounted()) return;

  auto scratchReg = m_rScratch;
  assert(baseReg != scratchReg);

  if (type.needsReg()) {
    // The type is dynamic, but we don't have two registers available
    // to load the type and the data.
    cgDecRefDynamicTypeMem(baseReg, offset);
  } else if (type.maybeCounted()) {
    m_as.loadq(baseReg[offset + TVOFF(m_data)], scratchReg);
    cgDecRefStaticType(type, scratchReg, true);
  }
}

void CodeGenerator::cgDecRefMem(IRInstruction* inst) {
  assert(inst->src(0)->type().isPtr());
  cgDecRefMem(inst->typeParam(),
              srcLoc(0).reg(),
              inst->src(1)->intVal());
}

void CodeGenerator::cgDecRefWork(IRInstruction* inst, bool genZeroCheck) {
  SSATmp* src   = inst->src(0);
  if (!isRefCounted(src)) return;
  Type type = src->type();
  if (type.isKnownDataType()) {
    cgDecRefStaticType(type, srcLoc(0).reg(), genZeroCheck);
  } else {
    cgDecRefDynamicType(srcLoc(0).reg(1), srcLoc(0).reg(0), genZeroCheck);
  }
}

void CodeGenerator::cgDecRef(IRInstruction *inst) {
  // DecRef may bring the count to zero, and run the destructor.
  // Generate code for this.
  cgDecRefWork(inst, true);
}

void CodeGenerator::cgDecRefNZ(IRInstruction* inst) {
  // DecRefNZ cannot bring the count to zero.
  // Therefore, we don't generate zero-checking code.
  cgDecRefWork(inst, false);
}

void CodeGenerator::cgCufIterSpillFrame(IRInstruction* inst) {
  auto const nArgs = inst->extra<CufIterSpillFrame>()->args;
  auto const iterId = inst->extra<CufIterSpillFrame>()->iterId;
  auto const itOff = iterOffset(iterId);

  const auto spOffset = -safe_cast<int32_t>(kNumActRecCells * sizeof(Cell));
  auto spReg = srcLoc(0).reg();
  auto fpReg = srcLoc(1).reg();

  m_as.loadq   (fpReg[itOff + CufIter::funcOff()], m_rScratch);
  m_as.storeq  (m_rScratch, spReg[spOffset + int(AROFF(m_func))]);

  m_as.loadq   (fpReg[itOff + CufIter::ctxOff()], m_rScratch);
  m_as.storeq  (m_rScratch, spReg[spOffset + int(AROFF(m_this))]);

  m_as.shrq    (1, m_rScratch);
  ifThen(m_as, CC_NBE, [this] {
      m_as.shlq(1, m_rScratch);
      emitIncRef(m_as, m_rScratch);
    });
  m_as.loadq   (fpReg[itOff + CufIter::nameOff()], m_rScratch);
  m_as.testq    (m_rScratch, m_rScratch);
  ifThen(m_as, CC_NZ, [this] {
      m_as.cmpl(0, m_rScratch[FAST_REFCOUNT_OFFSET]);
      static_assert(UncountedValue < 0 && StaticValue < 0, "");
      ifThen(m_as, CC_NS, [&] { emitIncRef(m_as, m_rScratch); });
      m_as.orq (ActRec::kInvNameBit, m_rScratch);
    });
  m_as.storeq  (m_rScratch, spReg[spOffset + int(AROFF(m_invName))]);
  m_as.storel  (safe_cast<int32_t>(nArgs),
                spReg[spOffset + int(AROFF(m_numArgsAndFlags))]);

  emitAdjustSp(spReg, dstLoc(0).reg(), spOffset);
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func      = inst->src(1);
  auto const objOrCls  = inst->src(2);
  auto const magicName = inst->extra<SpillFrame>()->invName;
  auto const nArgs     = inst->extra<SpillFrame>()->numArgs;
  auto& a              = m_as;

  const auto spOffset = -safe_cast<int32_t>(kNumActRecCells * sizeof(Cell));

  auto spReg = srcLoc(0).reg();
  // actRec->m_this
  if (objOrCls->isA(Type::Cls)) {
    // store class
    if (objOrCls->isConst()) {
      emitImmStoreq(a, uintptr_t(objOrCls->clsVal()) | 1,
                    spReg[spOffset + int(AROFF(m_this))]);
    } else {
      Reg64 clsPtrReg = srcLoc(2/*objOrCls*/).reg();
      a.   movq   (clsPtrReg, m_rScratch);
      a.   orq    (1, m_rScratch);
      a.   storeq (m_rScratch, spReg[spOffset + int(AROFF(m_this))]);
    }
  } else if (objOrCls->isA(Type::Obj)) {
    // store this pointer
    a.     storeq (srcLoc(2/*objOrCls*/).reg(),
                   spReg[spOffset + int(AROFF(m_this))]);
  } else if (objOrCls->isA(Type::Ctx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    Reg64 objOrClsPtrReg = srcLoc(2/*objOrCls*/).reg();
    a.     storeq (objOrClsPtrReg, spReg[spOffset + int(AROFF(m_this))]);
  } else {
    assert(objOrCls->isA(Type::Nullptr));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + int(AROFF(m_this));
    a.     storeq (0, spReg[offset_m_this]);
  }
  // actRec->m_invName
  // ActRec::m_invName is encoded as a pointer with bit kInvNameBit
  // set to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName = !magicName
    ? 0
    : reinterpret_cast<uintptr_t>(magicName) | ActRec::kInvNameBit;
  emitImmStoreq(a, invName, spReg[spOffset + int(AROFF(m_invName))]);
  // actRec->m_func  and possibly actRec->m_cls
  // Note m_cls is unioned with m_this and may overwrite previous value
  if (func->isA(Type::Nullptr)) {
    // No need to store the null---we're always about to run another
    // instruction that will populate the Func.
  } else if (func->isConst()) {
    const Func* f = func->funcVal();
    emitImmStoreq(a, intptr_t(f), spReg[spOffset + int(AROFF(m_func))]);
  } else {
    int offset_m_func = spOffset + int(AROFF(m_func));
    auto funcLoc = srcLoc(1);
    a.  storeq (funcLoc.reg(0), spReg[offset_m_func]);
  }

  a.    storel (nArgs, spReg[spOffset + int(AROFF(m_numArgsAndFlags))]);

  emitAdjustSp(spReg, dstLoc(0).reg(), spOffset);
}

void CodeGenerator::cgStClosureFunc(IRInstruction* inst) {
  auto const obj  = srcLoc(0).reg();
  auto const func = inst->extra<StClosureFunc>()->func;
  emitImmStoreq(m_as, intptr_t(func), obj[c_Closure::funcOffset()]);
}

void CodeGenerator::cgStClosureArg(IRInstruction* inst) {
  cgStore(
    srcLoc(0).reg()[inst->extra<StClosureArg>()->offsetBytes],
    inst->src(1), srcLoc(1),
    Width::Full
  );
}

void CodeGenerator::cgStClosureCtx(IRInstruction* inst) {
  auto const obj = srcLoc(0).reg();

  if (inst->src(1)->isA(Type::Nullptr)) {
    m_as.  storeq  (0, obj[c_Closure::ctxOffset()]);
  } else {
    auto const ctx = srcLoc(1).reg();
    always_assert(ctx != InvalidReg);
    m_as.  storeq  (ctx, obj[c_Closure::ctxOffset()]);
  }
}

void CodeGenerator::emitInitObjProps(PhysReg dstReg,
                                     const Class* cls,
                                     size_t nProps) {
  // If the object has a small number of properties, just emit stores
  // inline.
  if (nProps < 8) {
    for (int i = 0; i < nProps; ++i) {
      auto propOffset =
        sizeof(ObjectData) + cls->builtinODTailSize() + sizeof(TypedValue) * i;
      auto propDataOffset = propOffset + TVOFF(m_data);
      auto propTypeOffset = propOffset + TVOFF(m_type);
      if (!IS_NULL_TYPE(cls->declPropInit()[i].m_type)) {
        emitImmStoreq(m_as, cls->declPropInit()[i].m_data.num,
                      dstReg[propDataOffset]);
      }
      m_as.storeb(cls->declPropInit()[i].m_type, dstReg[propTypeOffset]);
    }
    return;
  }

  // Use memcpy for large numbers of properties.
  auto args = argGroup()
    .addr(dstReg,
          safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
    .imm(int64_t(&cls->declPropInit()[0]))
    .imm(cellsToBytes(nProps));
  cgCallHelper(m_as,
               CppCall::direct(memcpy),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               args);
}

void CodeGenerator::cgConstructInstance(IRInstruction* inst) {
  auto const cls    = inst->extra<ConstructInstance>()->cls;
  auto const dstReg = dstLoc(0).reg();
  cgCallHelper(m_as,
               CppCall::direct(cls->instanceCtor()),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
               argGroup().immPtr(cls));
}

void CodeGenerator::cgCheckInitProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitProps>()->cls;
  auto const branch = inst->taken();
  m_as.cmpq(0, rVmTl[cls->propHandle()]);
  emitFwdJcc(m_as, CC_Z, branch);
}

void CodeGenerator::cgCheckInitSProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitSProps>()->cls;
  auto const branch = inst->taken();
  m_as.cmpb(0, rVmTl[cls->sPropInitHandle()]);
  emitFwdJcc(m_as, CC_Z, branch);
}

void CodeGenerator::cgNewInstanceRaw(IRInstruction* inst) {
  auto const cls    = inst->extra<NewInstanceRaw>()->cls;
  auto const dstReg = dstLoc(0).reg();
  size_t size = ObjectData::sizeForNProps(cls->numDeclProperties());
  cgCallHelper(m_as,
               size <= kMaxSmartSize
               ? CppCall::direct(ObjectData::newInstanceRaw)
               : CppCall::direct(ObjectData::newInstanceRawBig),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
               argGroup().imm((uint64_t)cls).imm(size));
}

void CodeGenerator::cgInitObjProps(IRInstruction* inst) {
  auto const cls    = inst->extra<InitObjProps>()->cls;
  auto const srcReg = srcLoc(0).reg();

  // Set the attributes, if any
  int odAttrs = cls->getODAttrs();
  if (odAttrs) {
    // o_attribute is 16 bits but the fact that we're or-ing a mask makes it ok
    assert(!(odAttrs & 0xffff0000));
    m_as.orq(odAttrs, srcReg[ObjectData::attributeOff()]);
  }

  // Initialize the properties
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() == 0) {
      // Fast case: copy from a known address in the Class
      emitInitObjProps(srcReg, cls, nProps);
    } else {
      // Slower case: we have to load the src address from the targetcache
      auto rPropData = m_rScratch;
      // Load the Class's propInitVec from the targetcache
      m_as.loadq(rVmTl[cls->propHandle()], rPropData);
      // propData holds the PropInitVec. We want &(*propData)[0]
      m_as.loadq(rPropData[Class::PropInitVec::dataOff()], rPropData);
      if (!cls->hasDeepInitProps()) {
        auto args = argGroup()
          .addr(srcReg,
              safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
          .reg(rPropData)
          .imm(cellsToBytes(nProps));
        cgCallHelper(m_as,
                     CppCall::direct(memcpy),
                     kVoidDest,
                     SyncOptions::kNoSyncPoint,
                     args);
      } else {
        auto args = argGroup()
          .addr(srcReg,
              safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
          .reg(rPropData)
          .imm(nProps);
        cgCallHelper(m_as,
                     CppCall::direct(deepInitHelper),
                     kVoidDest,
                     SyncOptions::kNoSyncPoint,
                     args);
      }
    }
  }
}

void CodeGenerator::cgCallArray(IRInstruction* inst) {
  Offset pc             = inst->extra<CallArray>()->pc;
  Offset after          = inst->extra<CallArray>()->after;
  cgCallHelper(
    m_as,
    CppCall::direct(
      reinterpret_cast<void (*)()>(mcg->tx().uniqueStubs.fcallArrayHelper)),
    kVoidDest,
    SyncOptions::kSyncPoint,
    argGroup()
      .imm(pc)
      .imm(after)
  );
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  auto const extra = inst->extra<Call>();
  auto const rSP   = srcLoc(0).reg();
  auto const rFP   = srcLoc(1).reg();
  auto& a = m_as;

  auto const ar = extra->numParams * sizeof(TypedValue);
  a.    storeq (rFP,                              rSP[ar + AROFF(m_sfp)]);
  a.    storel (safe_cast<int32_t>(extra->after), rSP[ar + AROFF(m_soff)]);

  if (extra->knownPrologue) {
    assert(extra->callee);
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      auto const off = cellsToBytes(extra->numParams) + AROFF(m_savedRip);
      emitImmStoreq(a, 0xff00ff00b00b00d0, rSP[off]);
    }
    emitLea(a, rSP[cellsToBytes(extra->numParams)], rStashedAR);
    /*
     * Normally there's no need to prepare for smash if this is a live
     * or optimized translation, since we know where we are going.
     *
     * However, if we're going to a profiling prologue, we want it to
     * be smashable later, so we need to tell the profiling module
     * about this and prepare for smashing the call.
     */
    if (mcg->code.prof().contains(extra->knownPrologue)) {
      auto const calleeNumParams = extra->callee->numNonVariadicParams();
      auto const prologIndex =
        extra->numParams <= calleeNumParams ? extra->numParams
                                            : calleeNumParams + 1;
      mcg->backEnd().prepareForSmash(a.code(), kCallLen);
      mcg->tx().profData()->addPrologueMainCaller(
        extra->callee,
        prologIndex,
        a.code().frontier()
      );
      always_assert(mcg->backEnd().isSmashable(a.code().frontier(), kCallLen));
    }
    a.  call   (extra->knownPrologue);
    return;
  }

  auto const srcKey = m_curInst->marker().sk();
  auto const adjust = emitBindCall(m_mainCode,
                                   m_coldCode,
                                   m_frozenCode,
                                   srcKey,
                                   extra->callee,
                                   extra->numParams);
  assert(dstLoc(0).reg() == rVmSp);
  if (adjust) {
    a.  addq   (adjust, rVmSp);
  }
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  uint32_t offset = inst->extra<CastStk>()->offset;
  PhysReg spReg   = srcLoc(0).reg();
  auto args = argGroup();
  args.addr(spReg, cellsToBytes(offset));

  TCA tvCastHelper;
  if (type <= Type::Bool) {
    tvCastHelper = (TCA)tvCastToBooleanInPlace;
  } else if (type <= Type::Int) {
    tvCastHelper = (TCA)tvCastToInt64InPlace;
  } else if (type <= Type::Dbl) {
    tvCastHelper = (TCA)tvCastToDoubleInPlace;
  } else if (type <= Type::Arr) {
    tvCastHelper = (TCA)tvCastToArrayInPlace;
  } else if (type <= Type::Str) {
    tvCastHelper = (TCA)tvCastToStringInPlace;
  } else if (type <= Type::Obj) {
    tvCastHelper = (TCA)tvCastToObjectInPlace;
  } else if (type <= Type::NullableObj) {
    tvCastHelper = (TCA)tvCastToNullableObjectInPlace;
  } else if (type <= Type::Res) {
    tvCastHelper = (TCA)tvCastToResourceInPlace;
  } else {
    not_reached();
  }
  cgCallHelper(m_as,
               CppCall::direct(reinterpret_cast<void (*)()>(tvCastHelper)),
               kVoidDest,
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgCastStkIntToDbl(IRInstruction* inst) {
  auto spReg = srcLoc(0).reg();
  auto offset = cellsToBytes(inst->extra<CastStkIntToDbl>()->offset);
  m_as.cvtsi2sd(refTVData(spReg[offset]), rCgXMM0);
  emitStoreReg(m_as, rCgXMM0, refTVData(spReg[offset]));
  emitStoreTVType(m_as, KindOfDouble, refTVType(spReg[offset]));
}

void CodeGenerator::cgCoerceStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  uint32_t offset = inst->extra<CoerceStk>()->offset;
  Block* exit     = inst->taken();
  PhysReg spReg   = srcLoc(0).reg();

  auto args = argGroup();
  args.addr(spReg, cellsToBytes(offset));

  TCA tvCoerceHelper;
  if (type <= Type::Bool) {
    tvCoerceHelper = (TCA)tvCoerceParamToBooleanInPlace;
  } else if (type <= Type::Int) {
    // if casting to integer, pass 10 as the base for the conversion
    args.imm(10);
    tvCoerceHelper = (TCA)tvCoerceParamToInt64InPlace;
  } else if (type <= Type::Dbl) {
    tvCoerceHelper = (TCA)tvCoerceParamToDoubleInPlace;
  } else if (type <= Type::Arr) {
    tvCoerceHelper = (TCA)tvCoerceParamToArrayInPlace;
  } else if (type <= Type::Str) {
    tvCoerceHelper = (TCA)tvCoerceParamToStringInPlace;
  } else if (type <= Type::Obj) {
    tvCoerceHelper = (TCA)tvCoerceParamToObjectInPlace;
  } else if (type <= Type::Res) {
    tvCoerceHelper = (TCA)tvCoerceParamToResourceInPlace;
  } else {
    not_reached();
  }

  auto tmpReg = PhysReg(m_rScratch);
  cgCallHelper(m_as,
    CppCall::direct(reinterpret_cast<void (*)()>(tvCoerceHelper)),
    callDest(tmpReg),
    SyncOptions::kSyncPoint,
    args
  );
  m_as.testb(1, rbyte(tmpReg));
  emitFwdJcc(m_as, CC_E, exit);
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const dst            = dstLoc(0);
  auto const dstReg         = dst.reg(0);
  auto const dstType        = dst.reg(1);
  auto const callee         = inst->extra<CallBuiltin>()->callee;
  auto const numArgs        = callee->numParams();
  auto const returnType     = inst->typeParam();
  auto const funcReturnType = callee->returnType();

  int returnOffset = MISOFF(tvBuiltinReturn);

  if (FixupMap::eagerRecord(callee)) {
    auto const pc = curUnit()->entry() + m_curInst->marker().bcOff();
    // we have spilled all args to stack, so spDiff is 0
    emitEagerSyncPoint(m_as, reinterpret_cast<const Op*>(pc));
  }
  // RSP points to the MInstrState we need to use.  Workaround the
  // fact that rsp moves when we spill registers around call
  PhysReg misReg = m_rScratch;
  emitMovRegReg(m_as, reg::rsp, misReg);

  auto callArgs = argGroup();
  if (isCppByRef(funcReturnType)) {
    // First arg is pointer to storage for that return value
    if (isSmartPtrRef(funcReturnType)) {
      returnOffset += TVOFF(m_data);
    }
    // misReg is pointing to an MInstrState struct on the C stack.  Pass
    // the address of tvBuiltinReturn to the native function as the location
    // it can construct the return Array, String, Object, or Variant.
    callArgs.addr(misReg, returnOffset); // &misReg[returnOffset]
  }

  // Non-pointer args are plain values passed by value.  String, Array,
  // Object, and Variant are passed by const&, ie a pointer to stack memory
  // holding the value, so expect PtrToT types for these.
  // Pointers to smartptr types (String, Array, Object) need adjusting to
  // point to &ptr->m_data.
  auto srcNum = uint32_t{0};
  if (callee->isMethod() && !(callee->attrs() & AttrStatic)) {
    // Note, we don't support objects with vtables here (if they may
    // need a this pointer adjustment).  This should be filtered out
    // earlier right now.
    callArgs.ssa(srcNum);
    ++srcNum;
  }
  for (uint32_t i = 0; i < numArgs; ++i, ++srcNum) {
    auto const& pi = callee->params()[i];
    if (TVOFF(m_data) && isSmartPtrRef(pi.builtinType)) {
      assert(inst->src(srcNum)->type().isPtr() &&
             srcLoc(srcNum).reg() != InvalidReg);
      callArgs.addr(srcLoc(srcNum).reg(), TVOFF(m_data));
    } else {
      callArgs.ssa(srcNum, pi.builtinType == KindOfDouble);
    }
  }

  // If the return value is returned by reference, we don't need the
  // return value from this call since we know where the value is.
  auto dest = isCppByRef(funcReturnType) ? kVoidDest :
              funcReturnType == KindOfDouble ? callDestDbl(inst) :
              callDest(inst);
  cgCallHelper(m_as, CppCall::direct(callee->nativeFuncPtr()),
               dest, SyncOptions::kSyncPoint, callArgs);

  // For primitive return types (int, bool, double), the return value
  // is already in dstReg (the builtin call returns in rax or xmm0).
  // For void return types, there is no return value, so just exit.
  if (dstReg == InvalidReg || returnType.isSimpleType()) {
    return;
  }

  // after the call, RSP is back pointing to MInstrState and rSratch
  // has been clobberred.
  misReg = rsp;

  // For return by reference (String, Object, Array, Variant),
  // the builtin writes the return value into MInstrState::tvBuiltinReturn
  // TV, from where it has to be tested and copied.
  if (returnType.isReferenceType()) {
    assert(isCppByRef(funcReturnType) && isSmartPtrRef(funcReturnType));
    // return type is String, Array, or Object; fold nullptr to KindOfNull
    m_as.   loadq (misReg[returnOffset], dstReg);
    emitLoadImm(m_as, returnType.toDataType(), dstType);
    emitLoadImm(m_as, KindOfNull, m_rScratch);
    m_as.   testq (dstReg, dstReg);
    m_as.   cmov_reg64_reg64 (CC_Z, m_rScratch, dstType);
    return;
  }
  if (returnType <= Type::Cell || returnType <= Type::BoxedCell) {
    // return type is Variant; fold KindOfUninit to KindOfNull
    assert(isCppByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    assert(misReg != dstType);
    emitLoadTVType(m_as, misReg[returnOffset + TVOFF(m_type)], dstType);
    m_as.   loadq (misReg[returnOffset + TVOFF(m_data)], dstReg);
    emitLoadImm(m_as, KindOfNull, m_rScratch);
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    m_as.   testb (rbyte(dstType), rbyte(dstType));
    m_as.   cmov_reg64_reg64 (CC_Z, m_rScratch, dstType);
    return;
  }
  not_reached();
}

void CodeGenerator::cgSpillStack(IRInstruction* inst) {
  auto const spDeficit    = inst->src(1)->intVal();
  auto const spillVals    = inst->srcs().subpiece(2);
  auto const numSpillSrcs = spillVals.size();
  auto const dstReg       = dstLoc(0).reg();
  auto const spReg        = srcLoc(0).reg();
  auto const spillCells   = spillValueCells(inst);

  int adjustment = safe_cast<int32_t>(
    (spDeficit - spillCells) * ssize_t(sizeof(Cell))
  );
  for (uint32_t i = 0; i < numSpillSrcs; ++i) {
    int offset = safe_cast<int32_t>(i * ssize_t(sizeof(Cell)) + adjustment);
    cgStore(spReg[offset], spillVals[i], srcLoc(i + 2), Width::Full);
  }
  emitAdjustSp(spReg, dstReg, adjustment);
}

void CodeGenerator::emitAdjustSp(PhysReg spReg, PhysReg dstReg,
                                 int adjustment /* bytes */) {
  if (adjustment != 0) {
    if (dstReg != spReg) {
      m_as.    lea   (spReg[adjustment], dstReg);
    } else {
      m_as.    addq  (adjustment, dstReg);
    }
  } else {
    emitMovRegReg(m_as, spReg, dstReg);
  }
}

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  auto const func = curFunc();
  auto const builtinFuncPtr = func->builtinFuncPtr();
  auto& a = m_as;

  emitMovRegReg(a, srcLoc(0).reg(), argNumToRegName[0]);
  if (FixupMap::eagerRecord(func)) {
    emitEagerSyncPoint(m_as, reinterpret_cast<const Op*>(func->getEntry()));
  }
  emitCall(a, CppCall::direct(builtinFuncPtr));
  recordSyncPoint(a);
}

void CodeGenerator::cgLdThis(IRInstruction* inst) {
  Block* label  = inst->taken();
  auto dstReg = dstLoc(0).reg();

  m_as.loadq(srcLoc(0).reg()[AROFF(m_this)], dstReg);
  if (label == NULL) return;  // no need to perform its checks

  if (curFunc()->isPseudoMain() || !curFunc()->mayHaveThis()) {
    // Check for a null $this pointer first.
    m_as.testq(dstReg, dstReg);
    emitFwdJcc(CC_Z, label);
  }

  m_as.testb(1, rbyte(dstReg));
  emitFwdJcc(CC_NZ, label);
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  PhysReg srcReg = srcLoc(0).reg();
  PhysReg dstReg = dstLoc(0).reg();
  // Context could be either a this object or a class ptr
  m_as.   testb(1, rbyte(srcReg));
  ifThenElse(CC_NZ,
             [&](Asm& a) { emitLdClsCctx(a, srcReg, dstReg);  }, // ctx is class
             [&](Asm& a) { emitLdObjClass(a, srcReg, dstReg); }  // ctx is $this
            );
}

void CodeGenerator::cgLdClsCctx(IRInstruction* inst) {
  PhysReg srcReg = srcLoc(0).reg();
  PhysReg dstReg = dstLoc(0).reg();
  emitLdClsCctx(m_as, srcReg, dstReg);
}

void CodeGenerator::cgLdCtx(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const srcReg = srcLoc(0).reg();
  m_as.loadq(srcReg[AROFF(m_this)], dstReg);
}

void CodeGenerator::cgLdCctx(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdClsName(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const srcReg = srcLoc(0).reg();

  m_as.loadq(srcReg[Class::preClassOff()], dstReg);
  m_as.loadq(dstReg[PreClass::nameOffset()], dstReg);
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  assert(inst->src(1)->isConst());
  auto const offset = inst->src(1);
  auto dstReg       = dstLoc(0).reg();
  auto baseReg      = srcLoc(0).reg();
  m_as.loadq(baseReg[offset->intVal() + AROFF(m_func)], dstReg);
}

void CodeGenerator::cgLdStaticLocCached(IRInstruction* inst) {
  auto const extra = inst->extra<LdStaticLocCached>();
  auto const link  = RDS::bindStaticLocal(extra->func, extra->name);
  auto const dst   = dstLoc(0).reg();
  auto& a = m_as;
  emitLea(a, rVmTl[link.handle()], dst);
}

void CodeGenerator::cgCheckStaticLocInit(IRInstruction* inst) {
  auto const src = srcLoc(0).reg();
  auto& a = m_as;
  emitCmpTVType(a, KindOfUninit, src[RefData::tvOffset() + TVOFF(m_type)]);
  emitFwdJcc(a, CC_E, inst->taken());
}

void CodeGenerator::cgStaticLocInitCached(IRInstruction* inst) {
  auto const rdSrc = srcLoc(0).reg();
  auto& a = m_as;

  // If we're here, the target-cache-local RefData is all zeros, so we
  // can initialize it by storing the new value into it's TypedValue
  // and incrementing the RefData reference count (which will set it
  // to 1).
  //
  // We are storing the rdSrc value into the static, but we don't need
  // to inc ref it because it's a bytecode invariant that it's not a
  // reference counted type.
  cgStore(rdSrc[RefData::tvOffset()], inst->src(1), srcLoc(1), Width::Full);
  a.    incl   (rdSrc[FAST_REFCOUNT_OFFSET]);
  if (debug) {
    static_assert(sizeof(RefData::Magic::kMagic) == sizeof(uint64_t), "");
    emitImmStoreq(a, static_cast<int64_t>(RefData::Magic::kMagic),
                  rdSrc[RefData::magicOffset()]);
  }
}

void CodeGenerator::cgStoreTypedValue(MemoryRef dst, SSATmp* src, PhysLoc loc) {
  assert(src->type().needsReg());
  auto srcReg0 = loc.reg(0);
  auto srcReg1 = loc.reg(1);
  if (srcReg0.isSIMD()) {
    // Whole typed value is stored in single SIMD reg srcReg0
    assert(RuntimeOption::EvalHHIRAllocSIMDRegs);
    assert(srcReg1 == InvalidReg);
    m_as.movdqu(srcReg0, refTVData(dst));
    return;
  }

  if (src->type().needsValueReg()) {
    assert(srcReg0 != InvalidReg);
    m_as.storeq(srcReg0, refTVData(dst));
  }

  assert(srcReg1 != InvalidReg);
  emitStoreTVType(m_as, srcReg1, refTVType(dst));
}

void CodeGenerator::cgStore(MemoryRef dst, SSATmp* src, PhysLoc srcLoc,
                            Width width) {
  Type type = src->type();
  if (type.needsReg()) {
    always_assert(width == Width::Full);
    cgStoreTypedValue(dst, src, srcLoc);
    return;
  }
  if (width == Width::Full) {
    emitStoreTVType(m_as, type.toDataType(), refTVType(dst));
  }
  if (!src->type().needsValueReg()) return; // no value to store

  auto memRef = refTVData(dst);
  auto srcReg = srcLoc.reg();
  if (srcReg == InvalidReg) {
    always_assert(type <= (Type::Bool | Type::Int | Type::Dbl |
                  Type::Arr | Type::StaticStr | Type::Cls));
    emitImmStoreq(m_as, src->rawVal(), memRef);
  } else {
    zeroExtendIfBool(m_as, src, srcReg);
    emitStoreReg(m_as, srcReg, memRef);
  }
}

void CodeGenerator::cgLoad(SSATmp* dst, PhysLoc dstLoc, MemoryRef base,
                           Block* label) {
  Type type = dst->type();
  if (type.needsReg()) {
    return cgLoadTypedValue(dst, dstLoc, base, label);
  }
  if (label != NULL) {
    emitTypeCheck(type, refTVType(base), refTVData(base), label);
  }
  if (dst->isA(Type::Null)) return; // these are constants
  auto dstReg = dstLoc.reg();
  // if dstReg == InvalidReg then the value of this load is dead
  if (dstReg == InvalidReg) return;

  if (type <= Type::Bool) {
    m_as.loadl(refTVData(base), toReg32(dstReg));
  } else {
    emitLoadReg(m_as, refTVData(base), dstReg);
  }
}

static MemoryRef makeMemoryRef(Asm& as, Reg64 scratch,
                               MemoryRef value) {
  always_assert(int(value.r.index) != -1);
  always_assert(value.r.scale == 1); //TASK(2731486): fix this... use imul?
  if (value.r.base != scratch && value.r.index != scratch) {
    as.movq(value.r.base, scratch);
    value.r.base = scratch;
  }
  if (value.r.base == scratch) {
    as.addq(value.r.index, value.r.base);
    return value.r.base[value.r.disp];
  }
  as.addq(value.r.base, value.r.index);
  return value.r.index[value.r.disp];
}

// If label is not null and type is not Gen, this method generates a check
// that bails to the label if the loaded typed value doesn't match dst type.
void CodeGenerator::cgLoadTypedValue(SSATmp* dst, PhysLoc dstLoc,
                                     MemoryRef ref, Block* label) {
  Type type = dst->type();
  auto valueDstReg = dstLoc.reg(0);
  auto typeDstReg  = dstLoc.reg(1);

  if (valueDstReg.isSIMD()) {
    assert(!label);
    // Whole typed value is stored in single SIMD reg valueDstReg
    assert(RuntimeOption::EvalHHIRAllocSIMDRegs);
    assert(typeDstReg == InvalidReg);
    m_as.movdqu(refTVData(ref), valueDstReg);
    return;
  }

  if (valueDstReg == InvalidReg && typeDstReg == InvalidReg &&
      (label == nullptr || type == Type::Gen)) {
    // a dead load
    return;
  }

  bool isResolved = true;
  auto origRef = ref;
  ref = resolveRegCollision(typeDstReg, ref, isResolved);
  if (!isResolved) {
    // there was a collision with the registers that could not be resolved.
    // Re-enter with a non-indexed MemoryRef (slow path).
    cgLoadTypedValue(dst, dstLoc, makeMemoryRef(m_as, m_rScratch, origRef),
                     label);
    return;
  }
  // Load type if it's not dead
  if (typeDstReg != InvalidReg) {
    emitLoadTVType(m_as, refTVType(ref), typeDstReg);
    if (label) {
      emitTypeCheck(type, typeDstReg,
                    valueDstReg, label);
    }
  } else if (label) {
    emitTypeCheck(type, refTVType(ref), refTVData(ref), label);
  }

  // Load value if it's not dead
  if (valueDstReg == InvalidReg) return;

  m_as.loadq(refTVData(ref), valueDstReg);
}

// May set isResolved = false to signal that there is no register conflict
// resolution. Callers should take proper action to solve the issue (e.g.
// change the indexed MemoryRef into a base+disp MemoryRef). In this case,
// the returned MemoryRef should be ignored.
MemoryRef CodeGenerator::resolveRegCollision(PhysReg dst, MemoryRef memRef,
                                             bool& isResolved) {
  isResolved = true;
  Reg64 base = memRef.r.base;
  Reg64 index = memRef.r.index;
  if (int(index) == -1) {
    if (base == dst) {
      assert(base != m_rScratch);
      // use the scratch register instead
      m_as.movq(base, m_rScratch);
      return m_rScratch[memRef.r.disp];
    }
    return memRef;
  }
  bool scratchTaken = (base == m_rScratch || index == m_rScratch);
  if (base == dst) {
    if (scratchTaken) {
      // bail, the caller will manage somehow
      isResolved = false;
      return memRef;
    }
    // use the scratch register instead
    m_as.movq(base, m_rScratch);
    return m_rScratch[index + memRef.r.disp];
  } else if (index == dst) {
    if (scratchTaken) {
      // bail, the caller will manage somehow
      isResolved = false;
      return memRef;
    }
    // use the scratch register instead
    m_as.movq(index, m_rScratch);
    return base[m_rScratch + memRef.r.disp];
  }
  return memRef;
}

void CodeGenerator::cgLdProp(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[inst->src(1)->intVal()],
         inst->taken());
}

void CodeGenerator::cgLdMem(IRInstruction * inst) {
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[inst->src(1)->intVal()],
         inst->taken());
}

void CodeGenerator::cgLdRef(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[RefData::tvOffset()],
         inst->taken());
}

void CodeGenerator::cgStringIsset(IRInstruction* inst) {
  auto strReg = srcLoc(0).reg();
  auto idxReg = srcLoc(1).reg();
  auto dstReg = dstLoc(0).reg();
  if (idxReg == InvalidReg) {
    m_as.cmpl(safe_cast<int32_t>(inst->src(1)->intVal()),
              strReg[StringData::sizeOff()]);
  } else {
    m_as.cmpl(r32(idxReg), strReg[StringData::sizeOff()]);
  }
  m_as.setnbe(rbyte(dstReg));
}

void CodeGenerator::cgProfileArray(IRInstruction* inst) {
  auto baseReg = srcLoc(0).reg();
  auto handle  = inst->extra<ProfileArray>()->handle;

  // If kPackedKind changes to a value that is not 0, change
  // this to a conditional add.
  static_assert(ArrayData::ArrayKind::kPackedKind == 0, "kPackedKind changed");
  m_as.  loadzbl(baseReg[ArrayData::offsetofKind()], r32(m_rScratch));
  m_as.  addl(r32(m_rScratch),
              rVmTl[handle + offsetof(NonPackedArrayProfile, count)]);
}

void CodeGenerator::cgCheckPackedArrayBounds(IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");
  // We may check packed array bounds on profiled arrays for which
  // we do not statically know that they are of kPackedKind.
  assert(inst->taken());
  auto arrReg = srcLoc(0).reg();
  auto idxReg = srcLoc(1).reg();
  if (idxReg == InvalidReg) {
    m_as.cmpl  (safe_cast<int32_t>(inst->src(1)->intVal()),
                arrReg[ArrayData::offsetofSize()]);
  } else {
    // ArrayData::m_size is a uint32_t but we need to do a 64-bit comparison
    // since idx is KindOfInt64.
    m_as.loadl (arrReg[ArrayData::offsetofSize()], r32(m_rScratch));
    m_as.cmpq  (idxReg, m_rScratch);
  }
  emitFwdJcc(CC_BE, inst->taken());
}

void CodeGenerator::cgLdPackedArrayElem(IRInstruction* inst) {
  if (inst->src(0)->isConst()) {
    // This would require two scratch registers and should be very
    // rare. t3626251
    CG_PUNT(LdPackedArrayElem-ConstArray);
  }

  auto const rArr = srcLoc(0).reg();
  auto const rIdx = srcLoc(1).reg();
  auto& a = m_as;

  // We don't know if we have the last use of rIdx, so we can't
  // clobber it.
  if (rIdx != InvalidReg) {
    /*
     * gcc 4.8 did something more like:
     *
     *    lea 1(%base), %scratch   ; sizeof(ArrayData) == sizeof(TypedValue)
     *    salq $4, %scratch
     *    movq (%base,%scratch,1), %r1
     *    movzxb 8(%base,%scratch,1), %r2
     *
     * Using this way for now (which is more like what clang produced)
     * just because it was 2 bytes smaller.
     */
    static_assert(sizeof(TypedValue) == 16, "");
    a.    movq  (rIdx, m_rScratch);
    a.    shlq  (0x4, m_rScratch);
    cgLoad(inst->dst(), dstLoc(0), rArr[m_rScratch + sizeof(ArrayData)]);
    return;
  }

  auto const idx    = inst->src(1)->intVal();
  auto const offset = sizeof(ArrayData) + idx * sizeof(TypedValue);
  cgLoad(inst->dst(), dstLoc(0), rArr[offset]);
}

void CodeGenerator::cgCheckPackedArrayElemNull(IRInstruction* inst) {
  if (inst->src(0)->isConst()) {
    // This would require two scratch registers and should be very
    // rare. t3626251
    CG_PUNT(LdPackedArrayElemAddr-ConstArray);
  }

  auto const rArr = srcLoc(0).reg();
  auto const rIdx = srcLoc(1).reg();
  auto& a = m_as;

  if (rIdx != InvalidReg) {
    static_assert(sizeof(TypedValue) == 16, "");
    a.    movq  (rIdx, m_rScratch);
    a.    shlq  (0x4, m_rScratch);
    emitCmpTVType(a, KindOfNull,
      rArr[m_rScratch + sizeof(ArrayData) + TVOFF(m_type)]);
  } else {
    auto const idx    = inst->src(1)->intVal();
    auto const offset = sizeof(ArrayData) + idx * sizeof(TypedValue);
    emitCmpTVType(a, KindOfNull, rArr[offset + TVOFF(m_type)]);
  }

  auto const dst = dstLoc(0).reg();
  a.    setne  (rbyte(dst));
  a.    movzbl (rbyte(dst), r32(dst));
}

void CodeGenerator::cgCheckBounds(IRInstruction* inst) {
  auto idx = inst->src(0);
  auto idxReg = srcLoc(0).reg();
  auto size = inst->src(1);
  auto sizeReg = srcLoc(1).reg();
  // caller made the check if both sources are constant and never
  // generate this opcode
  assert(!(idx->isConst() && size->isConst()));
  auto throwHelper =
    [&](Asm& as) {
      auto args = argGroup();
      args.ssa(0/*idx*/);
      cgCallHelper(as, CppCall::direct(throwOOB),
                   kVoidDest, SyncOptions::kSyncPoint, args);

    };

  if (idxReg == InvalidReg) {
    assert(idx->intVal() >= 0); // we would have punted otherwise
    auto idxVal = safe_cast<int32_t>(idx->intVal());
    m_as.cmpq(idxVal, sizeReg);
    unlikelyIfBlock(CC_LE, throwHelper);
    return;
  }

  if (sizeReg == InvalidReg) {
    assert(size->intVal() >= 0);
    auto sizeVal = safe_cast<int32_t>(size->intVal());
    m_as.cmpq(sizeVal, idxReg);
  } else {
    m_as.cmpq(sizeReg, idxReg);
  }
  unlikelyIfBlock(CC_AE, throwHelper);
}

void CodeGenerator::cgLdVectorSize(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());
  m_as.loadl(vecReg[c_Vector::sizeOffset()], toReg32(dstReg));
}

void CodeGenerator::cgLdVectorBase(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());
  m_as.loadq(vecReg[c_Vector::dataOffset()], dstReg);
}

/**
 * Given a vector, check if it has a immutable copy and jump to the taken
 * branch if so.
 */
void CodeGenerator::cgVectorHasImmCopy(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();

  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());

  // Vector::m_data field holds an address of an ArrayData plus
  // sizeof(ArrayData) bytes. We need to check this ArrayData's
  // m_count field to see if we need to call Vector::triggerCow().
  uint rawPtrOffset = c_Vector::dataOffset() + kExpectedMPxOffset;

  m_as.loadq(vecReg[rawPtrOffset], m_rScratch);
  m_as.cmpl(
    1,
    m_rScratch[(int64_t)FAST_REFCOUNT_OFFSET - (int64_t)sizeof(ArrayData)]
  );
  emitFwdJcc(CC_NE, inst->taken());
}

/**
 * Given the base of a vector object, pass it to a helper
 * which is responsible for triggering COW.
 */
void CodeGenerator::cgVectorDoCow(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());
  auto args = argGroup();
  args.ssa(0); // vec
  cgCallHelper(m_as, CppCall::direct(triggerCow),
               kVoidDest, SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgLdPairBase(IRInstruction* inst) {
  DEBUG_ONLY auto pair = inst->src(0);
  auto pairReg = srcLoc(0).reg();
  assert(pair->type().strictSubtypeOf(Type::Obj) &&
         pair->type().getClass() == c_Pair::classof());
  m_as.lea(pairReg[c_Pair::dataOffset()], dstLoc(0).reg());
}

void CodeGenerator::cgLdElem(IRInstruction* inst) {
  auto baseReg = srcLoc(0).reg();
  auto idx = inst->src(1);
  auto idxReg = srcLoc(1).reg();
  if (idx->isConst()) {
    cgLoad(inst->dst(), dstLoc(0), baseReg[idx->intVal()]);
  } else {
    cgLoad(inst->dst(), dstLoc(0), baseReg[idxReg]);
  }
}

void CodeGenerator::cgStElem(IRInstruction* inst) {
  auto baseReg = srcLoc(0).reg();
  auto srcValue = inst->src(2);
  auto idx = inst->src(1);
  auto idxReg = srcLoc(1).reg();
  if (idxReg == InvalidReg) {
    cgStore(baseReg[idx->intVal()], srcValue, srcLoc(2), Width::Full);
  } else {
    cgStore(baseReg[idxReg], srcValue, srcLoc(2), Width::Full);
  }
}

void CodeGenerator::recordSyncPoint(Asm& as,
                                    SyncOptions sync /* = kSyncPoint */) {
  auto const marker = m_curInst->marker();
  assert(m_curInst->marker().valid());

  Offset stackOff = marker.spOff();
  switch (sync) {
  case SyncOptions::kSyncPointAdjustOne:
    stackOff -= 1;
    break;
  case SyncOptions::kSyncPoint:
  case SyncOptions::kSmashableAndSyncPoint:
    break;
  case SyncOptions::kNoSyncPoint:
    // we can get here if we are memory profiling, since we override the
    // normal sync settings and sync anyway
    assert(RuntimeOption::HHProfServerEnabled);
  }

  Offset pcOff = marker.bcOff() - marker.func()->base();

  FTRACE(5, "IR recordSyncPoint: {} {} {}\n", as.frontier(), pcOff,
         stackOff);
  mcg->recordSyncPoint(as.frontier(), pcOff, stackOff);
}

void CodeGenerator::cgLdMIStateAddr(IRInstruction* inst) {
  auto base = srcLoc(0).reg();
  int64_t offset = inst->src(1)->intVal();
  m_as.lea (base[offset], dstLoc(0).reg());
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[localOffset(inst->extra<LdLoc>()->locId)]);
}

void CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  auto const fpReg  = srcLoc(0).reg();
  auto const offset = localOffset(inst->extra<LdLocAddr>()->locId);
  if (dstLoc(0).hasReg()) {
    m_as.lea(fpReg[offset], dstLoc(0).reg());
  }
}

void CodeGenerator::cgLdGbl(IRInstruction* inst) {
  cgLoad(
    inst->dst(),
    dstLoc(0),
    srcLoc(0).reg()[localOffset(inst->extra<LdGbl>()->locId)],
    inst->taken()
  );
}

void CodeGenerator::cgStGbl(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto off = localOffset(inst->extra<StGbl>()->locId);
  cgStore(ptr[off], inst->src(1), srcLoc(1), Width::Full);
}

void CodeGenerator::cgLdStackAddr(IRInstruction* inst) {
  auto const base   = srcLoc(0).reg();
  auto const offset = cellsToBytes(inst->extra<LdStackAddr>()->offset);
  auto const dst    = dstLoc(0).reg();
  m_as.lea (base[offset], dst);
}

void CodeGenerator::cgLdStack(IRInstruction* inst) {
  assert(inst->taken() == nullptr);
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[cellsToBytes(inst->extra<LdStack>()->offset)]);
}

void CodeGenerator::cgGuardStk(IRInstruction* inst) {
  auto const rSP = srcLoc(0).reg();
  auto const baseOff = cellsToBytes(inst->extra<GuardStk>()->offset);
  emitTypeGuard(inst->typeParam(),
                rSP[baseOff + TVOFF(m_type)],
                rSP[baseOff + TVOFF(m_data)]);
}

void CodeGenerator::cgCheckStk(IRInstruction* inst) {
  auto const rbase = srcLoc(0).reg();
  auto const baseOff = cellsToBytes(inst->extra<CheckStk>()->offset);

  emitTypeCheck(inst->typeParam(), rbase[baseOff + TVOFF(m_type)],
                rbase[baseOff + TVOFF(m_data)], inst->taken());
}

void CodeGenerator::cgGuardLoc(IRInstruction* inst) {
  auto const rFP = srcLoc(0).reg();
  auto const baseOff = localOffset(inst->extra<GuardLoc>()->locId);
  emitTypeGuard(inst->typeParam(),
                rFP[baseOff + TVOFF(m_type)],
                rFP[baseOff + TVOFF(m_data)]);
}

void CodeGenerator::cgCheckLoc(IRInstruction* inst) {
  auto const rbase = srcLoc(0).reg();
  auto const baseOff = localOffset(inst->extra<CheckLoc>()->locId);
  emitTypeCheck(inst->typeParam(), rbase[baseOff + TVOFF(m_type)],
                rbase[baseOff + TVOFF(m_data)], inst->taken());
}

template<class Loc>
void CodeGenerator::emitSideExitGuard(Type type,
                                      Loc typeSrc,
                                      Loc dataSrc,
                                      Offset taken) {
  emitTypeTest(
    type, typeSrc, dataSrc,
    [&](ConditionCode cc) {
      auto const sk = SrcKey(curFunc(), taken, resumed());
      emitBindSideExit(m_mainCode, m_frozenCode, ccNegate(cc), sk);
    });
}

void CodeGenerator::cgSideExitGuardLoc(IRInstruction* inst) {
  auto const fp    = srcLoc(0).reg();
  auto const extra = inst->extra<SideExitGuardLoc>();
  emitSideExitGuard(inst->typeParam(),
                    fp[localOffset(extra->checkedSlot) + TVOFF(m_type)],
                    fp[localOffset(extra->checkedSlot) + TVOFF(m_data)],
                    extra->taken);
}

void CodeGenerator::cgSideExitGuardStk(IRInstruction* inst) {
  auto const sp    = srcLoc(0).reg();
  auto const extra = inst->extra<SideExitGuardStk>();

  emitSideExitGuard(inst->typeParam(),
                    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_type)],
                    sp[cellsToBytes(extra->checkedSlot) + TVOFF(m_data)],
                    extra->taken);
}

void CodeGenerator::cgExitJcc(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitCompare(inst);
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgExitJccInt(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitCompareInt(inst);
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgDefMIStateBase(IRInstruction* inst) {
  assert(dstLoc(0).reg() == rsp);
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->src(0);
  auto const rData = srcLoc(0).reg(0);
  auto const rType = srcLoc(0).reg(1);
  assert(rData != InvalidReg);

  auto doJcc = [&](ConditionCode cc) {
    emitFwdJcc(ccNegate(cc), inst->taken());
  };
  auto doMov = [&]() {
    auto const valDst = dstLoc(0).reg(0);
    auto const typeDst = dstLoc(0).reg(1);
    // TODO: #3626251: XLS: Let Uses say whether a constant is
    // allowed, and if not, assign a register.
    if (valDst != InvalidReg) {
      emitMovRegReg(m_as, rData, valDst);
    }
    if (typeDst != InvalidReg) {
      if (rType != InvalidReg) emitMovRegReg(m_as, rType, typeDst);
      else m_as.emitImmReg(src->type().toDataType(), typeDst);
    }
  };

  Type typeParam = inst->typeParam();
  // CheckTypes that are known to succeed or fail may be kept around
  // by the simplifier in case the guard can be relaxed.
  if (src->isA(typeParam)) {
    // src is the target type or better. do nothing.
    doMov();
    return;
  } else if (src->type().not(typeParam)) {
    // src is definitely not the target type. always jump.
    emitFwdJmp(m_as, inst->taken(), m_state);
    return;
  }

  if (rType != InvalidReg) {
    emitTypeTest(typeParam, rType, rData, doJcc);
  } else {
    Type srcType = src->type();
    if (srcType.isBoxed() && typeParam.isBoxed()) {
      // Nothing to do here, since we check the inner type at the uses
    } else if (typeParam.isSpecialized()) {
      // We're just checking the array kind or object class of a value with a
      // mostly-known type.
      emitSpecializedTypeTest(typeParam, rData, doJcc);
    } else if (typeParam <= Type::Uncounted &&
               ((srcType <= Type::Str && typeParam.maybe(Type::StaticStr)) ||
                (srcType <= Type::Arr && typeParam.maybe(Type::StaticArr)))) {
      // We carry Str and Arr operands around without a type register,
      // even though they're union types.  The static and non-static
      // subtypes are distinguised by the refcount field.
      m_as.cmpl(0, rData[FAST_REFCOUNT_OFFSET]);
      doJcc(CC_L);
    } else {
      // We should only get here if this CheckType should've been simplified
      // away but wasn't for some reason, so do a simple version of what it
      // would've. Widen inner types first since CheckType ignores them.
      if (srcType.maybeBoxed()) srcType |= Type::BoxedCell;
      if (typeParam.maybeBoxed()) typeParam |= Type::BoxedCell;

      if (srcType <= typeParam) {
        // This will always succeed. Do nothing.
      } else if (srcType.not(typeParam)) {
        // This will always fail. Emit an unconditional jmp.
        emitFwdJmp(m_as, inst->taken(), m_state);
        return;
      } else {
        always_assert_log(
          false,
          [&] {
            return folly::format("Bad src: {} and dst: {} types in '{}'",
                                 srcType, typeParam, *inst).str();
          });
      }
    }
  }

  doMov();
}

void CodeGenerator::cgCheckTypeMem(IRInstruction* inst) {
  auto const reg = srcLoc(0).reg();
  emitTypeCheck(inst->typeParam(), reg[TVOFF(m_type)],
                reg[TVOFF(m_data)], inst->taken());
}

void CodeGenerator::cgCheckDefinedClsEq(IRInstruction* inst) {
  auto const clsName = inst->extra<CheckDefinedClsEq>()->clsName;
  auto const cls     = inst->extra<CheckDefinedClsEq>()->cls;
  auto const ch      = Unit::GetNamedEntity(clsName)->getClassHandle();
  auto clsImm = Immed64(cls);
  if (clsImm.fits(sz::dword)) {
    m_as.cmpq(clsImm.l(), rVmTl[ch]);
  } else {
    m_as.emitImmReg(cls, m_rScratch);
    m_as.cmpq(m_rScratch, rVmTl[ch]);
  }
  emitFwdJcc(CC_NZ, inst->taken());
}

void CodeGenerator::cgGuardRefs(IRInstruction* inst) {
  assert(inst->numSrcs() == 5);

  DEBUG_ONLY SSATmp* nParamsTmp = inst->src(1);
  SSATmp* firstBitNumTmp = inst->src(2);
  SSATmp* mask64Tmp  = inst->src(3);
  SSATmp* vals64Tmp  = inst->src(4);

  auto funcPtrReg = srcLoc(0).reg();
  auto nParamsReg = srcLoc(1).reg();
  auto mask64Reg = srcLoc(3).reg();
  auto vals64Reg = srcLoc(4).reg();

  // Get values in place
  assert(funcPtrReg != InvalidReg);

  assert(nParamsReg != InvalidReg || nParamsTmp->isConst());

  assert(firstBitNumTmp->isConst(Type::Int));
  auto firstBitNum = safe_cast<int32_t>(firstBitNumTmp->intVal());

  uint64_t mask64 = mask64Tmp->intVal();
  assert(mask64Reg != InvalidReg || mask64 == uint32_t(mask64));
  assert(mask64);

  uint64_t vals64 = vals64Tmp->intVal();
  assert(vals64Reg != InvalidReg || vals64 == uint32_t(vals64));
  assert((vals64 & mask64) == vals64);

  auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
  auto const destSR = mcg->tx().getSrcRec(destSK);

  auto thenBody = [&](Asm& a) {
    auto bitsOff = sizeof(uint64_t) * (firstBitNum / 64);
    auto cond = CC_NE;
    auto bitsPtrReg = m_rScratch;

    if (firstBitNum == 0) {
      bitsOff = Func::refBitValOff();
      bitsPtrReg = funcPtrReg;
    } else {
      a.loadq(funcPtrReg[Func::sharedOff()], bitsPtrReg);
      bitsOff -= sizeof(uint64_t);
    }

    if (vals64 == 0 || (mask64 & (mask64 - 1)) == 0) {
      // If vals64 is zero, or we're testing a single
      // bit, we can get away with a single test,
      // rather than mask-and-compare
      if (mask64Reg != InvalidReg) {
        a.testq  (mask64Reg, bitsPtrReg[bitsOff]);
      } else {
        if (mask64 < 256) {
          a.testb((int8_t)mask64, bitsPtrReg[bitsOff]);
        } else {
          a.testl((int32_t)mask64, bitsPtrReg[bitsOff]);
        }
      }
      if (vals64) cond = CC_E;
    } else {
      auto bitsValReg = m_rScratch;
      a.  loadq  (bitsPtrReg[bitsOff], bitsValReg);
      if (debug) bitsPtrReg = InvalidReg;

      //     bitsValReg <- bitsValReg & mask64
      if (mask64Reg != InvalidReg) {
        a.  andq   (mask64Reg, bitsValReg);
      } else if (mask64 < 256) {
        a.  andb   ((int8_t)mask64, rbyte(bitsValReg));
      } else {
        a.  andl   ((int32_t)mask64, r32(bitsValReg));
      }

      //   If bitsValReg != vals64, then goto Exit
      if (vals64Reg != InvalidReg) {
        a.  cmpq   (vals64Reg, bitsValReg);
      } else if (mask64 < 256) {
        assert(vals64 < 256);
        a.  cmpb   ((int8_t)vals64, rbyte(bitsValReg));
      } else {
        a.  cmpl   ((int32_t)vals64, r32(bitsValReg));
      }
    }
    destSR->emitFallbackJump(m_mainCode, cond);
  };

  if (firstBitNum == 0) {
    assert(nParamsReg == InvalidReg);
    // This is the first 64 bits. No need to check
    // nParams.
    thenBody(m_as);
  } else {
    assert(nParamsReg != InvalidReg);
    // Check number of args...
    m_as.    cmpq   (firstBitNum, nParamsReg);

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nParams, then either all params
      // are refs, or all params are non-refs, so if vals64
      // isn't 0 and isnt mask64, there's no possibility of
      // a match
      destSR->emitFallbackJump(m_mainCode, CC_LE);
      thenBody(m_as);
    } else {
      ifThenElse(CC_NLE, thenBody, /* else */ [&](Asm& a) {
          //   If not special builtin...
          a.testl(AttrVariadicByRef, funcPtrReg[Func::attrsOff()]);
          destSR->emitFallbackJump(m_mainCode, vals64 ? CC_Z : CC_NZ);
        });
    }
  }
}

void CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const objReg = srcLoc(0).reg();
  auto const prop = inst->src(1);
  auto& a = m_as;
  always_assert(objReg != InvalidReg);
  always_assert(dstReg != InvalidReg);
  a.    lea    (objReg[prop->intVal()], dstReg);
}

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto clsReg = srcLoc(0).reg();
  int32_t mSlotVal = inst->src(1)->rawVal();

  assert(dstReg != InvalidReg);

  auto methOff = int32_t(mSlotVal * sizeof(Func*));
  m_as.loadq(clsReg[methOff],  dstReg);
}

void CodeGenerator::cgLookupClsMethodCache(IRInstruction* inst) {
  auto funcDestReg   = dstLoc(0).reg(0);

  auto const& extra = *inst->extra<ClsMethodData>();
  auto const cls = extra.clsName;
  auto const method = extra.methodName;
  auto const ne = extra.namedEntity;
  auto const ch = StaticMethodCache::alloc(cls,
                                           method,
                                           getContextName(curClass()));

  if (false) { // typecheck
    UNUSED TypedValue* fake_fp = nullptr;
    UNUSED TypedValue* fake_sp = nullptr;
    const UNUSED Func* f = StaticMethodCache::lookup(
      ch, ne, cls, method, fake_fp);
  }
  if (inst->src(0)->isConst()) {
    PUNT(LookupClsMethodCache_const_fp);
  }

  // can raise an error if class is undefined
  cgCallHelper(m_as,
               CppCall::direct(StaticMethodCache::lookup),
               callDest(funcDestReg),
               SyncOptions::kSyncPoint,
               argGroup()
                 .imm(ch)       // Handle ch
                 .immPtr(ne)            // NamedEntity* np.second
                 .immPtr(cls)           // className
                 .immPtr(method)        // methodName
                 .reg(srcLoc(0).reg()) // frame pointer
              );
}

void CodeGenerator::cgLdClsMethodCacheCommon(IRInstruction* inst, Offset off) {
  auto dstReg = dstLoc(0).reg();
  if (dstReg == InvalidReg) return;

  auto const& extra = *inst->extra<ClsMethodData>();
  auto const clsName = extra.clsName;
  auto const methodName = extra.methodName;
  auto const ch = StaticMethodCache::alloc(clsName, methodName,
                                           getContextName(curClass()));
  if (dstReg != InvalidReg) {
    m_as.loadq(rVmTl[ch + off], dstReg);
  }
}

void CodeGenerator::cgLdClsMethodCacheFunc(IRInstruction* inst) {
  cgLdClsMethodCacheCommon(inst, offsetof(StaticMethodCache, m_func));

}

void CodeGenerator::cgLdClsMethodCacheCls(IRInstruction* inst) {
  cgLdClsMethodCacheCommon(inst, offsetof(StaticMethodCache, m_cls));
}

/**
 * Helper to emit getting the value for ActRec's m_this/m_cls slot
 * from a This pointer depending on whether the callee method is
 * static or not.
 */
void CodeGenerator::emitGetCtxFwdCallWithThis(PhysReg ctxReg,
                                              bool    staticCallee) {
  if (staticCallee) {
    // Load (this->m_cls | 0x1) into ctxReg.
    emitLdLowPtr(m_as, ctxReg[ObjectData::getVMClassOffset()],
                 ctxReg, sizeof(LowClassPtr));
    m_as.orq(1, ctxReg);
  } else {
    // Just incref $this.
    emitIncRef(m_as, ctxReg);
  }
}

/**
 * This method is similar to emitGetCtxFwdCallWithThis above, but
 * whether or not the callee is a static method is unknown at JIT
 * time, and that is determined dynamically by looking up into the
 * StaticMethodFCache.
 */
void CodeGenerator::cgGetCtxFwdCall(IRInstruction* inst) {
  PhysReg destCtxReg = dstLoc(0).reg(0);
  auto srcCtxTmp = inst->src(0);
  auto srcCtxReg = srcLoc(0).reg(0);
  const Func* callee = inst->src(1)->funcVal();
  bool      withThis = srcCtxTmp->isA(Type::Obj);

  // Eagerly move src into the dest reg
  emitMovRegReg(m_as, srcCtxReg, destCtxReg);

  Label End;
  // If we don't know whether we have a This, we need to check dynamically
  if (!withThis) {
    m_as.testb(1, rbyte(destCtxReg));
    m_as.jcc8(CC_NZ, End);
  }

  // If we have a This pointer in destCtxReg, then select either This
  // or its Class based on whether callee is static or not
  emitGetCtxFwdCallWithThis(destCtxReg, (callee->attrs() & AttrStatic));

  asm_label(m_as, End);
}

void CodeGenerator::cgLdClsMethodFCacheFunc(IRInstruction* inst) {
  auto const& extra     = *inst->extra<ClsMethodData>();
  auto const clsName    = extra.clsName;
  auto const methodName = extra.methodName;
  auto const dstReg     = dstLoc(0).reg();

  auto const ch = StaticMethodFCache::alloc(
    clsName, methodName, getContextName(curClass())
  );
  if (dstReg != InvalidReg) {
    m_as.loadq(rVmTl[ch], dstReg);
  }
}

void CodeGenerator::cgLookupClsMethodFCache(IRInstruction* inst) {
  auto const funcDestReg = dstLoc(0).reg(0);
  auto const cls         = inst->src(0)->clsVal();
  auto const& extra      = *inst->extra<ClsMethodData>();
  auto const methName    = extra.methodName;
  auto const fpReg       = srcLoc(1).reg();
  auto const clsName     = cls->name();

  auto ch = StaticMethodFCache::alloc(
    clsName, methName, getContextName(curClass())
  );

  const Func* (*lookup)(
    RDS::Handle, const Class*, const StringData*, TypedValue*) =
    StaticMethodFCache::lookup;
  cgCallHelper(m_as,
               CppCall::direct(lookup),
               callDest(funcDestReg),
               SyncOptions::kSyncPoint,
               argGroup()
                 .imm(ch)
                 .immPtr(cls)
                 .immPtr(methName)
                 .reg(fpReg));
}

void CodeGenerator::emitGetCtxFwdCallWithThisDyn(PhysReg      destCtxReg,
                                                 PhysReg      thisReg,
                                                 RDS::Handle ch) {
  Label NonStaticCall, End;

  // thisReg is holding $this. Should we pass it to the callee?
  m_as.cmpl(1,
            rVmTl[ch + offsetof(StaticMethodFCache, m_static)]);
  m_as.jcc8(CC_NE, NonStaticCall);
  // If calling a static method...
  {
    // Load (this->m_cls | 0x1) into destCtxReg
    emitLdLowPtr(m_as, thisReg[ObjectData::getVMClassOffset()],
                 destCtxReg, sizeof(LowClassPtr));
    m_as.orq(1, destCtxReg);
    m_as.jmp8(End);
  }
  // Else: calling non-static method
  {
    asm_label(m_as, NonStaticCall);
    emitMovRegReg(m_as, thisReg, destCtxReg);
    emitIncRef(m_as, destCtxReg);
  }
  asm_label(m_as, End);
}

void CodeGenerator::cgGetCtxFwdCallDyn(IRInstruction* inst) {
  auto srcCtxTmp  = inst->src(0);
  auto srcCtxReg  = srcLoc(0).reg();
  auto destCtxReg = dstLoc(0).reg();

  Label End;

  emitMovRegReg(m_as, srcCtxReg, destCtxReg);
  auto const t = srcCtxTmp->type();
  if (t <= Type::Cctx) {
    // Nothing to do. Forward the context as is.
    return;
  } else if (t <= Type::Obj) {
    // We definitely have $this, so always run code emitted by
    // emitGetCtxFwdCallWithThisDyn
  } else {
    assert(t <= Type::Ctx);
    // dynamically check if we have a This pointer and call
    // emitGetCtxFwdCallWithThisDyn below
    m_as.testb(1, rbyte(destCtxReg));
    m_as.jcc8(CC_NZ, End);
  }

  // If we have a 'this' pointer ...
  auto const& extra = *inst->extra<ClsMethodData>();
  auto const ch = StaticMethodFCache::alloc(
    extra.clsName, extra.methodName, getContextName(curClass())
  );
  emitGetCtxFwdCallWithThisDyn(destCtxReg, destCtxReg, ch);

  asm_label(m_as, End);
}

void CodeGenerator::cgLdClsPropAddrKnown(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();

  auto cls  = inst->src(0)->clsVal();
  auto name = inst->src(1)->strVal();

  auto ch = cls->sPropHandle(cls->lookupSProp(name));
  m_as.lea(rVmTl[ch], dstReg);
}

RDS::Handle CodeGenerator::cgLdClsCachedCommon(
  IRInstruction* inst) {
  const StringData* className = inst->src(0)->strVal();
  auto ch = Unit::GetNamedEntity(className)->getClassHandle();
  auto dstReg = dstLoc(0).reg();
  if (dstReg == InvalidReg) {
    m_as. cmpq   (0, rVmTl[ch]);
  } else {
    m_as.  loadq  (rVmTl[ch], dstReg);
    m_as.  testq  (dstReg, dstReg);
  }

  return ch;
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  auto ch = cgLdClsCachedCommon(inst);
  unlikelyIfBlock(CC_E, [&] (Asm& a) {
    Class* (*const func)(Class**, const StringData*) =
      JIT::lookupKnownClass;
    cgCallHelper(a,
                 CppCall::direct(func),
                 callDest(inst),
                 SyncOptions::kSyncPoint,
                 argGroup().addr(rVmTl, safe_cast<int32_t>(ch))
                           .ssa(0));
  });
}

void CodeGenerator::cgLdClsCachedSafe(IRInstruction* inst) {
  cgLdClsCachedCommon(inst);
  if (Block* taken = inst->taken()) {
    emitFwdJcc(CC_Z, taken);
  }
}

void CodeGenerator::cgDerefClsRDSHandle(IRInstruction* inst) {
  auto const dreg = dstLoc(0).reg();
  auto const ch   = inst->src(0);
  auto& a = m_as;

  if (dreg == InvalidReg) return;
  if (ch->isConst()) {
    a.    loadq  (rVmTl[ch->rdsHandleVal()], dreg);
  } else {
    a.    loadq  (rVmTl[srcLoc(0).reg()], dreg);
  }
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  auto const ch = ClassCache::alloc();
  RDS::recordRds(ch, sizeof(ClassCache),
                 "ClassCache", curFunc()->fullName()->data());

  cgCallHelper(m_as,
               CppCall::direct(ClassCache::lookup),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup().imm(ch).ssa(0/*className*/));
}

void CodeGenerator::cgLdClsCns(IRInstruction* inst) {
  auto const extra = inst->extra<LdClsCns>();
  auto const link  = RDS::bindClassConstant(extra->clsName, extra->cnsName);
  cgLoad(inst->dst(), dstLoc(0), rVmTl[link.handle()], inst->taken());
}

void CodeGenerator::cgLookupClsCns(IRInstruction* inst) {
  auto const extra = inst->extra<LookupClsCns>();
  auto const link  = RDS::bindClassConstant(extra->clsName, extra->cnsName);
  cgCallHelper(
    m_as,
    CppCall::direct(JIT::lookupClassConstantTv),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup()
      .addr(rVmTl, safe_cast<int32_t>(link.handle()))
      .immPtr(Unit::GetNamedEntity(extra->clsName))
      .immPtr(extra->clsName)
      .immPtr(extra->cnsName)
  );
}

void CodeGenerator::cgLdCns(IRInstruction* inst) {
  const StringData* cnsName = inst->src(0)->strVal();

  auto const ch = makeCnsHandle(cnsName, false);
  // Has an unlikely branch to a LookupCns
  cgLoad(inst->dst(), dstLoc(0), rVmTl[ch], inst->taken());
}

void CodeGenerator::cgLookupCnsCommon(IRInstruction* inst) {
  SSATmp* cnsNameTmp = inst->src(0);

  assert(cnsNameTmp->isConst(Type::StaticStr));

  auto const cnsName = cnsNameTmp->strVal();
  auto const ch = makeCnsHandle(cnsName, false);

  auto args = argGroup();
  args.addr(rVmTl, safe_cast<int32_t>(ch))
      .immPtr(cnsName)
      .imm(inst->op() == LookupCnsE);

  cgCallHelper(m_as,
               CppCall::direct(lookupCnsHelper),
               callDestTV(inst),
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgLookupCns(IRInstruction* inst) {
  cgLookupCnsCommon(inst);
}

void CodeGenerator::cgLookupCnsE(IRInstruction* inst) {
  cgLookupCnsCommon(inst);
}

void CodeGenerator::cgLookupCnsU(IRInstruction* inst) {
  SSATmp* cnsNameTmp = inst->src(0);
  SSATmp* fallbackNameTmp = inst->src(1);

  const StringData* cnsName = cnsNameTmp->strVal();

  const StringData* fallbackName = fallbackNameTmp->strVal();
  auto const fallbackCh = makeCnsHandle(fallbackName, false);

  auto args = argGroup();
  args.addr(rVmTl, safe_cast<int32_t>(fallbackCh))
      .immPtr(cnsName)
      .immPtr(fallbackName);

  cgCallHelper(m_as,
               CppCall::direct(lookupCnsUHelper),
               callDestTV(inst),
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgAKExists(IRInstruction* inst) {
  SSATmp* arr = inst->src(0);
  SSATmp* key = inst->src(1);

  bool (*obj_int_helper)(ObjectData*, int64_t) = &ak_exist_int_obj;
  bool (*obj_str_helper)(ObjectData*, StringData*) = &ak_exist_string_obj;
  bool (*arr_str_helper)(ArrayData*, StringData*) = &ak_exist_string;

  if (key->type() <= Type::Null) {
    if (arr->isA(Type::Arr)) {
      cgCallHelper(m_as,
                   CppCall::direct(arr_str_helper),
                   callDest(inst),
                   SyncOptions::kNoSyncPoint,
                   argGroup().ssa(0/*arr*/).immPtr(staticEmptyString()));
    } else {
      m_as.movq(0, dstLoc(0).reg());
    }
    return;
  }

  auto helper_func = arr->isA(Type::Obj)
    ? (key->isA(Type::Int)
       ? CppCall::direct(obj_int_helper)
       : CppCall::direct(obj_str_helper))
    : (key->isA(Type::Int)
       ? CppCall::array(&g_array_funcs.existsInt)
       : CppCall::direct(arr_str_helper));

  cgCallHelper(m_as,
               helper_func,
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup().ssa(0/*arr*/).ssa(1/*key*/));
}

void CodeGenerator::cgLdGblAddr(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  cgCallHelper(m_as,
               CppCall::direct(ldGblAddrHelper),
               callDest(dstReg),
               SyncOptions::kNoSyncPoint,
               argGroup().ssa(0));
  m_as.testq(dstReg, dstReg);
  emitFwdJcc(CC_Z, inst->taken());
}

void CodeGenerator::emitTestZero(SSATmp* src, PhysLoc srcLoc) {
  auto& a = m_as;
  auto reg = srcLoc.reg();

  /*
   * If src is const, normally a earlier optimization pass should have
   * converted the thing testing this condition into something
   * unconditional.  So rather than supporting constants efficiently
   * here, we just materialize the value into a register.
   */
  if (reg == InvalidReg) {
    reg = m_rScratch;
    a.    movq   (src->rawVal(), reg);
  }

  if (src->isA(Type::Bool)) {
    a.    testb  (rbyte(reg), rbyte(reg));
  } else {
    a.    testq  (reg, reg);
  }
}

void CodeGenerator::cgJmpZero(IRInstruction* inst) {
  emitTestZero(inst->src(0), srcLoc(0));
  emitFwdJcc(CC_Z, inst->taken());
}

void CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  emitTestZero(inst->src(0), srcLoc(0));
  emitFwdJcc(CC_NZ, inst->taken());
}

void CodeGenerator::cgReqBindJmpZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitTestZero(inst->src(0), srcLoc(0));
  emitReqBindJcc(CC_Z, inst->extra<ReqBindJmpZero>());
}

void CodeGenerator::cgReqBindJmpNZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  emitTestZero(inst->src(0), srcLoc(0));
  emitReqBindJcc(CC_NZ, inst->extra<ReqBindJmpNZero>());
}

void CodeGenerator::cgSideExitJmpZero(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitTestZero(inst->src(0), srcLoc(0));
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgSideExitJmpNZero(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  emitTestZero(inst->src(0), srcLoc(0));
  emitBindSideExit(m_mainCode, m_frozenCode,
                   opToConditionCode(inst->op()),
                   sk, extra->trflags);
}

void CodeGenerator::cgJmp(IRInstruction* inst) {
  if (!m_state.noTerminalJmp) {
    emitFwdJmp(m_as, inst->taken(), m_state);
  }
}

void CodeGenerator::cgJmpIndirect(IRInstruction* inst) {
  m_as.jmp(srcLoc(0).reg());
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* label = inst->taken();
  assert(label);
  SSATmp* src = inst->src(0);

  if (src->type().not(Type::Uninit)) return;

  auto typeReg = srcLoc(0).reg(1);
  assert(typeReg != InvalidReg);

  static_assert(KindOfUninit == 0, "cgCheckInit assumes KindOfUninit == 0");
  m_as.testb (rbyte(typeReg), rbyte(typeReg));
  emitFwdJcc(CC_Z, label);
}

void CodeGenerator::cgCheckInitMem(IRInstruction* inst) {
  Block* label = inst->taken();
  assert(label);
  SSATmp* base = inst->src(0);
  int64_t offset = inst->src(1)->intVal();
  Type t = base->type().deref();
  if (t.not(Type::Uninit)) return;
  auto basereg = srcLoc(0).reg();
  emitCmpTVType(m_as, KindOfUninit, basereg[offset + TVOFF(m_type)]);
  emitFwdJcc(CC_Z, label);
}

void CodeGenerator::cgCheckSurpriseFlags(IRInstruction* inst) {
  emitTestSurpriseFlags(m_as);
  emitFwdJcc(CC_NZ, inst->taken());
}

void CodeGenerator::cgCheckCold(IRInstruction* inst) {
  Block*     label = inst->taken();
  TransID  transId = inst->extra<CheckCold>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);

  emitLoadImm(m_as, uint64_t(counterAddr), m_rScratch);
  m_as.decq(m_rScratch[0]);
  emitFwdJcc(CC_LE, label);
}

static const StringData* s_ReleaseVV = makeStaticString("ReleaseVV");

void CodeGenerator::cgReleaseVVOrExit(IRInstruction* inst) {
  auto* const label = inst->taken();
  auto const rFp = srcLoc(0).reg();

  TargetProfile<ReleaseVVProfile> profile(m_unit.context(), m_curInst->marker(),
                                          s_ReleaseVV);
  if (profile.profiling()) {
    m_as.incw(rVmTl[profile.handle() + offsetof(ReleaseVVProfile, executed)]);
  }

  m_as.cmpq(0, rFp[AROFF(m_varEnv)]);

  bool releaseUnlikely = true;
  if (profile.optimizing()) {
    auto const data = profile.data(ReleaseVVProfile::reduce);
    FTRACE(3, "cgReleaseVVOrExit({}): percentReleased = {}\n",
           inst->toString(), data.percentReleased());
    if (data.percentReleased() >= RuntimeOption::EvalJitPGOReleaseVVMinPercent)
    {
      releaseUnlikely = false;
    }
  }
  ifBlock(CC_NZ, [&] (Asm& a) {
      if (profile.profiling()) {
        a.incw(rVmTl[profile.handle() + offsetof(ReleaseVVProfile, released)]);
      }
      a.testl(ActRec::kExtraArgsBit, rFp[AROFF(m_varEnv)]);
      emitFwdJcc(a, CC_Z, label);
      cgCallHelper(
        a,
        CppCall::direct(static_cast<void (*)(ActRec*)>(ExtraArgs::deallocate)),
        kVoidDest,
        SyncOptions::kSyncPoint,
        argGroup().reg(rFp)
      );
    },
    releaseUnlikely);
}

void CodeGenerator::cgBoxPtr(IRInstruction* inst) {
  auto base    = srcLoc(0).reg();
  auto dstReg  = dstLoc(0).reg();
  emitMovRegReg(m_as, base, dstReg);
  emitTypeTest(Type::BoxedCell, base[TVOFF(m_type)],
               base[TVOFF(m_data)],
    [&](ConditionCode cc) {
      ifThen(m_as, ccNegate(cc), [&] {
        cgCallHelper(m_as,
                     CppCall::direct(tvBox),
                     callDest(dstReg),
                     SyncOptions::kNoSyncPoint,
                     argGroup().ssa(0/*addr*/));
      });
    });
}

void CodeGenerator::cgConcatCellCell(IRInstruction* inst) {
  // Supported cases are all simplified into other instructions
  CG_PUNT(cgConcatCellCell);
}

void CodeGenerator::cgInterpOneCommon(IRInstruction* inst) {
  int64_t pcOff = inst->extra<InterpOneData>()->bcOff;

  auto opc = *(curFunc()->unit()->at(pcOff));
  void* interpOneHelper = interpOneEntryPoints[opc];

  auto dstReg = InvalidReg;
  if (inst->src(1)->isConst()) {
    PUNT(InterpOneCommon_const_fp);
  }
  cgCallHelper(m_as,
               CppCall::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
               argGroup().ssa(1/*fp*/).ssa(0/*sp*/).imm(pcOff));
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  auto const& extra = *inst->extra<InterpOne>();
  auto newSpReg = dstLoc(0).reg();
  assert(newSpReg == srcLoc(0).reg());

  auto spAdjustBytes = cellsToBytes(extra.cellsPopped - extra.cellsPushed);
  if (spAdjustBytes != 0) {
    m_as.addq(spAdjustBytes, newSpReg);
  }
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);

  m_as.loadq(rVmTl[RDS::kVmfpOff], rVmFp);
  m_as.loadq(rVmTl[RDS::kVmspOff], rVmSp);

  emitServiceReq(m_mainCode, REQ_RESUME);
}

void CodeGenerator::cgContEnter(IRInstruction* inst) {
  // ContEnter does not directly use SP, but the generator body we are jumping
  // to obviously does. We depend on SP via srcLoc(0) to avoid last SpillStack
  // be optimized away.
  auto curFpReg  = srcLoc(1).reg();
  auto genFpReg  = srcLoc(2).reg();
  auto addrReg   = srcLoc(3).reg();
  auto returnOff = safe_cast<int32_t>(inst->src(4)->intVal());
  assert(srcLoc(0).reg() == rVmSp);
  assert(curFpReg == rVmFp);

  Label Prologue, End;

  m_as.  storeq (curFpReg, genFpReg[AROFF(m_sfp)]);
  m_as.  storel (returnOff, genFpReg[AROFF(m_soff)]);
  m_as.  movq   (genFpReg, curFpReg);
  m_as.  jmp8   (End);

  asm_label(m_as, Prologue);
  m_as.  pop    (curFpReg[AROFF(m_savedRip)]);
  m_as.  jmp    (addrReg);

  asm_label(m_as, End);
  m_as.  call   (Prologue);
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg      = srcLoc(0).reg();
  auto checkStarted = inst->src(1)->boolVal();
  auto stateOff     = BaseGenerator::stateOff();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");
  static_assert(uint8_t(BaseGenerator::State::Started) == 1, "used below");

  // Take exit if state != 1 (checkStarted) or state > 1 (!checkStarted).
  m_as.cmpb(1, contReg[stateOff]);
  emitFwdJcc(checkStarted ? CC_NE : CC_A, inst->taken());

  // Set generator state as Running.
  m_as.storeb(int8_t(BaseGenerator::State::Running), contReg[stateOff]);
}

void CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  auto contReg  = srcLoc(0).reg();
  auto stateOff = BaseGenerator::stateOff();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");

  // Take exit if state == 0.
  m_as.testb(int8_t(0xff), contReg[stateOff]);
  emitFwdJcc(CC_Z, inst->taken());
}

void CodeGenerator::cgContValid(IRInstruction* inst) {
  auto contReg  = srcLoc(0).reg();
  auto dstReg   = dstLoc(0).reg();
  auto stateOff = BaseGenerator::stateOff();

  // Return 1 if generator state is not Done.
  m_as.cmpb(int8_t(BaseGenerator::State::Done), contReg[stateOff]);
  m_as.setne(rbyte(dstReg));
  m_as.movzbl(rbyte(dstReg), r32(dstReg));
}

void CodeGenerator::cgContArIncKey(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  m_as.incq(contArReg[CONTOFF(m_key) + TVOFF(m_data) -
                      c_Generator::arOff()]);
}

void CodeGenerator::cgContArUpdateIdx(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  int64_t off = CONTOFF(m_index) - c_Generator::arOff();
  auto newIdx = inst->src(1);
  auto newIdxReg = srcLoc(1).reg();

  // this is hacky and awful oh god
  if (newIdx->isConst()) {
    auto const val = newIdx->rawVal();
    m_as.emitImmReg(val, m_rScratch);
    m_as.cmpq  (m_rScratch, contArReg[off]);
    m_as.cload_reg64_disp_reg64(CC_G, contArReg, off, m_rScratch);
  } else {
    m_as.loadq (contArReg[off], m_rScratch);
    m_as.cmpq  (m_rScratch, newIdxReg);
    m_as.cmov_reg64_reg64(CC_G, newIdxReg, m_rScratch);
  }
  m_as.storeq  (m_rScratch, contArReg[off]);
}

void CodeGenerator::cgLdContActRec(IRInstruction* inst) {
  auto dest = dstLoc(0).reg();
  auto base = srcLoc(0).reg();
  ptrdiff_t offset = BaseGenerator::arOff();

  m_as.lea (base[offset], dest) ;
}

void CodeGenerator::emitLdRaw(IRInstruction* inst, size_t extraOff) {
  auto destReg = dstLoc(0).reg();
  auto offset  = inst->extra<RawMemData>()->info().offset;
  auto src     = srcLoc(0).reg()[offset + extraOff];

  switch (inst->extra<RawMemData>()->info().size) {
    case sz::byte:  m_as.loadzbl(src, r32(destReg)); break;
    case sz::dword:
      m_as.loadl(src, r32(destReg));
      if (inst->extra<RawMemData>()->type == RawMemData::FuncNumParams) {
        // See Func::finishedEmittingParams and Func::numParams for rationale
        m_as.shrl(0x1, r32(destReg));
      }
      break;
    case sz::qword: m_as.loadq(src, destReg); break;
    default:        not_implemented();
  }
}

void CodeGenerator::cgLdRaw(IRInstruction* inst) {
  emitLdRaw(inst, 0);
}

void CodeGenerator::cgLdContArRaw(IRInstruction* inst) {
  emitLdRaw(inst, -BaseGenerator::arOff());
}

void CodeGenerator::emitStRaw(IRInstruction* inst, size_t offset, int size) {
  auto dst    = srcLoc(0).reg()[offset];
  auto src    = inst->src(1);
  auto srcReg = srcLoc(1).reg();

  if (srcReg == InvalidReg) {
    auto val = Immed64(src->type().hasRawVal() ? src->rawVal() : 0);
    switch (size) {
      case sz::byte:  m_as.storeb(val.b(), dst); break;
      case sz::dword: m_as.storel(val.l(), dst); break;
      case sz::qword: emitImmStoreq(m_as, val.q(), dst); break;
      default:        not_implemented();
    }
  } else {
    switch (size) {
      case sz::byte:  m_as.storeb(rbyte(srcReg), dst); break;
      case sz::dword: m_as.storel(r32(srcReg), dst); break;
      case sz::qword: m_as.storeq(r64(srcReg), dst); break;
      default:        not_implemented();
    }
  }
}

void CodeGenerator::cgStRaw(IRInstruction* inst) {
  auto const info = inst->extra<RawMemData>()->info();
  emitStRaw(inst, info.offset, info.size);
}

void CodeGenerator::cgStContArRaw(IRInstruction* inst) {
  auto const info = inst->extra<RawMemData>()->info();
  emitStRaw(inst, -BaseGenerator::arOff() + info.offset, info.size);
}

void CodeGenerator::cgLdContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  const int64_t valueOff = CONTOFF(m_value);
  int64_t off = valueOff - c_Generator::arOff();
  cgLoad(inst->dst(), dstLoc(0), contArReg[off], inst->taken());
}

void CodeGenerator::cgStContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(1);
  const int64_t valueOff = CONTOFF(m_value);
  int64_t off = valueOff - c_Generator::arOff();
  cgStore(contArReg[off], value, valueLoc, Width::Full);
}

void CodeGenerator::cgLdContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  const int64_t keyOff = CONTOFF(m_key);
  int64_t off = keyOff - c_Generator::arOff();
  cgLoad(inst->dst(), dstLoc(0), contArReg[off], inst->taken());
}

void CodeGenerator::cgStContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(1);

  const int64_t keyOff = CONTOFF(m_key);
  int64_t off = keyOff - c_Generator::arOff();
  cgStore(contArReg[off], value, valueLoc, Width::Full);
}

void CodeGenerator::cgStAsyncArRaw(IRInstruction* inst) {
  auto const info = inst->extra<RawMemData>()->info();
  emitStRaw(inst, -c_AsyncFunctionWaitHandle::arOff() + info.offset,
            info.size);
}

void CodeGenerator::cgStAsyncArResult(IRInstruction* inst) {
  auto asyncArReg = srcLoc(0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(1);
  const int64_t off = c_AsyncFunctionWaitHandle::resultOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  cgStore(asyncArReg[off], value, valueLoc, Width::Full);
}

void CodeGenerator::cgLdAsyncArFParent(IRInstruction* inst) {
  auto asyncArReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  const int64_t off = c_AsyncFunctionWaitHandle::firstParentOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  m_as.loadq(asyncArReg[off], dstReg);
}

void CodeGenerator::cgAFWHBlockOn(IRInstruction* inst) {
  auto parentArReg = srcLoc(0).reg();
  auto childReg = srcLoc(1).reg();
  const int8_t blocked = c_WaitHandle::toKindState(
      c_WaitHandle::Kind::AsyncFunction, c_BlockableWaitHandle::STATE_BLOCKED);
  const int64_t firstParentOff = c_WaitableWaitHandle::firstParentOff();
  const int64_t stateToArOff = c_AsyncFunctionWaitHandle::stateOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t nextParentToArOff = c_AsyncFunctionWaitHandle::nextParentOff()
                                  - c_AsyncFunctionWaitHandle::arOff();
  const int64_t childToArOff = c_AsyncFunctionWaitHandle::childOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t objToArOff = -c_AsyncFunctionWaitHandle::arOff();

  // parent->setState(STATE_BLOCKED);
  m_as.storeb(blocked, parentArReg[stateToArOff]);

  // parent->m_nextParent = child->m_firstParent;
  m_as.loadq (childReg[firstParentOff], m_rScratch);
  m_as.storeq(m_rScratch, parentArReg[nextParentToArOff]);

  // child->m_firstParent = parent;
  m_as.lea   (parentArReg[objToArOff], m_rScratch);
  m_as.storeq(m_rScratch, childReg[firstParentOff]);

  // parent->m_child = child;
  m_as.storeq(childReg, parentArReg[childToArOff]);
}

void CodeGenerator::cgIsWaitHandle(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();
  auto& a = m_as;

  static_assert(
    ObjectData::IsWaitHandle < 0xff,
    "we use byte instructions for IsWaitHandle"
  );
  a.   testb   (ObjectData::IsWaitHandle, robj[ObjectData::attributeOff()]);
  a.   setnz   (rbyte(rdst));
}

void CodeGenerator::cgLdWHState(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();
  auto& a = m_as;
  a.    loadzbl (robj[ObjectData::whStateOffset()], r32(rdst));
  a.    andb    ((int8_t)0x0F, rbyte(rdst));
}

void CodeGenerator::cgLdWHResult(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  cgLoad(inst->dst(), dstLoc(0), robj[c_WaitHandle::resultOff()]);
}

void CodeGenerator::cgLdAFWHActRec(IRInstruction* inst) {
  auto const dest = dstLoc(0).reg();
  auto const base = srcLoc(0).reg();
  auto asyncArOffset = c_AsyncFunctionWaitHandle::arOff();
  m_as.lea (base[asyncArOffset], dest);
}

void CodeGenerator::cgLdResumableArObj(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const resumableArReg = srcLoc(0).reg();
  auto const objectOff = Resumable::objectOff() - Resumable::arOff();
  m_as.lea (resumableArReg[objectOff], dstReg);
}

void CodeGenerator::cgIterInit(IRInstruction* inst) {
  cgIterInitCommon(inst);
}

void CodeGenerator::cgIterInitK(IRInstruction* inst) {
  cgIterInitCommon(inst);
}

void CodeGenerator::cgWIterInit(IRInstruction* inst) {
  cgIterInitCommon(inst);
}

void CodeGenerator::cgWIterInitK(IRInstruction* inst) {
  cgIterInitCommon(inst);
}

void CodeGenerator::cgIterInitCommon(IRInstruction* inst) {
  bool isInitK = inst->op() == IterInitK || inst->op() == WIterInitK;
  bool isWInit = inst->op() == WIterInit || inst->op() == WIterInitK;

  PhysReg        fpReg = srcLoc(1).reg();
  int       iterOffset = this->iterOffset(inst->extra<IterData>()->iterId);
  int   valLocalOffset = localOffset(inst->extra<IterData>()->valId);
  SSATmp*          src = inst->src(0);
  auto args = argGroup();
  args.addr(fpReg, iterOffset).ssa(0/*src*/);
  if (src->isA(Type::Arr)) {
    args.addr(fpReg, valLocalOffset);
    if (isInitK) {
      args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
    } else if (isWInit) {
      args.imm(0);
    }
    TCA helperAddr = isWInit ? (TCA)new_iter_array_key<true> :
      isInitK ? (TCA)new_iter_array_key<false> : (TCA)new_iter_array;
    cgCallHelper(
      m_as,
      CppCall::direct(reinterpret_cast<void (*)()>(helperAddr)),
      callDest(inst),
      SyncOptions::kSyncPoint,
      args);
  } else {
    assert(src->type() <= Type::Obj);
    args.imm(uintptr_t(curClass())).addr(fpReg, valLocalOffset);
    if (isInitK) {
      args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
    } else {
      args.imm(0);
    }
    // new_iter_object decrefs its src object if it propagates an
    // exception out, so we use kSyncPointAdjustOne, which adjusts the
    // stack pointer by 1 stack element on an unwind, skipping over
    // the src object.
    cgCallHelper(m_as, CppCall::direct(new_iter_object), callDest(inst),
                 SyncOptions::kSyncPointAdjustOne, args);
  }
}

void CodeGenerator::cgMIterInit(IRInstruction* inst) {
  cgMIterInitCommon(inst);
}

void CodeGenerator::cgMIterInitK(IRInstruction* inst) {
  cgMIterInitCommon(inst);
}

void CodeGenerator::cgMIterInitCommon(IRInstruction* inst) {
  PhysReg          fpReg = srcLoc(1).reg();
  int         iterOffset = this->iterOffset(inst->extra<IterData>()->iterId);
  int     valLocalOffset = localOffset(inst->extra<IterData>()->valId);
  SSATmp*            src = inst->src(0);

  auto args = argGroup();
  args.addr(fpReg, iterOffset).ssa(0/*src*/);

  assert(src->type().isBoxed());
  auto innerType = src->type().innerType();
  assert(innerType.isKnownDataType());

  if (innerType <= Type::Arr) {
    args.addr(fpReg, valLocalOffset);
    if (inst->op() == MIterInitK) {
      args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
    } else {
      args.imm(0);
    }
    cgCallHelper(m_as,
                 CppCall::direct(new_miter_array_key),
                 callDest(inst),
                 SyncOptions::kSyncPoint,
                 args);
  } else if (innerType <= Type::Obj) {
    args.immPtr(curClass()).addr(fpReg, valLocalOffset);
    if (inst->op() == MIterInitK) {
      args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
    } else {
      args.imm(0);
    }
    // new_miter_object decrefs its src object if it propagates an
    // exception out, so we use kSyncPointAdjustOne, which adjusts the
    // stack pointer by 1 stack element on an unwind, skipping over
    // the src object.
    cgCallHelper(m_as,
                 CppCall::direct(new_miter_object),
                 callDest(inst),
                 SyncOptions::kSyncPointAdjustOne,
                 args);
  } else {
    CG_PUNT(MArrayIter-Unknown);
  }
}

void CodeGenerator::cgIterNext(IRInstruction* inst) {
  cgIterNextCommon(inst);
}

void CodeGenerator::cgIterNextK(IRInstruction* inst) {
  cgIterNextCommon(inst);
}

void CodeGenerator::cgWIterNext(IRInstruction* inst) {
  cgIterNextCommon(inst);
}

void CodeGenerator::cgWIterNextK(IRInstruction* inst) {
  cgIterNextCommon(inst);
}

void CodeGenerator::cgIterNextCommon(IRInstruction* inst) {
  bool isNextK = inst->op() == IterNextK || inst->op() == WIterNextK;
  bool isWNext = inst->op() == WIterNext || inst->op() == WIterNextK;
  PhysReg fpReg = srcLoc(0).reg();
  auto args = argGroup();
  args.addr(fpReg, iterOffset(inst->extra<IterData>()->iterId))
      .addr(fpReg, localOffset(inst->extra<IterData>()->valId));
  if (isNextK) {
    args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
  } else if (isWNext) {
    // We punt this case because nothing is using WIterNext opcodes
    // right now, and we don't want the witer_next_key helper to need
    // to check for null.
    CG_PUNT(WIterNext-nonKey);
  }
  TCA helperAddr = isWNext ? (TCA)witer_next_key :
    isNextK ? (TCA)iter_next_key_ind : (TCA)iter_next_ind;
  cgCallHelper(m_as,
               CppCall::direct(reinterpret_cast<void (*)()>(helperAddr)),
               callDest(inst),
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgMIterNext(IRInstruction* inst) {
  cgMIterNextCommon(inst);
}

void CodeGenerator::cgMIterNextK(IRInstruction* inst) {
  cgMIterNextCommon(inst);
}

void CodeGenerator::cgMIterNextCommon(IRInstruction* inst) {
  PhysReg fpReg = srcLoc(0).reg();
  auto args = argGroup();
  args.addr(fpReg, iterOffset(inst->extra<IterData>()->iterId))
      .addr(fpReg, localOffset(inst->extra<IterData>()->valId));
  if (inst->op() == MIterNextK) {
    args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
  } else {
    args.imm(0);
  }
  cgCallHelper(m_as, CppCall::direct(miter_next_key), callDest(inst),
               SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgIterFree(IRInstruction* inst) {
  PhysReg fpReg = srcLoc(0).reg();
  int     offset = iterOffset(inst->extra<IterFree>()->iterId);
  cgCallHelper(m_as,
               CppCall::method(&Iter::free),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset));
}

void CodeGenerator::cgMIterFree(IRInstruction* inst) {
  PhysReg fpReg = srcLoc(0).reg();
  int    offset = iterOffset(inst->extra<MIterFree>()->iterId);
  cgCallHelper(m_as,
               CppCall::method(&Iter::mfree),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset));
}

void CodeGenerator::cgDecodeCufIter(IRInstruction* inst) {
  PhysReg fpReg = srcLoc(1).reg();
  int     offset = iterOffset(inst->extra<DecodeCufIter>()->iterId);
  cgCallHelper(m_as,
               CppCall::direct(decodeCufIterHelper),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset)
                                  .typedValue(0));
}

void CodeGenerator::cgCIterFree(IRInstruction* inst) {
  PhysReg fpReg = srcLoc(0).reg();
  int      offset = iterOffset(inst->extra<CIterFree>()->iterId);
  cgCallHelper(m_as,
               CppCall::method(&Iter::cfree),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset));
}

void CodeGenerator::cgNewStructArray(IRInstruction* inst) {
  auto data = inst->extra<NewStructData>();
  StringData** table = mcg->allocData<StringData*>(sizeof(StringData*),
                                                      data->numKeys);
  memcpy(table, data->keys, data->numKeys * sizeof(*data->keys));
  MixedArray* (*f)(uint32_t, StringData**, const TypedValue*) =
    &MixedArray::MakeStruct;
  cgCallHelper(m_as,
               CppCall::direct(f),
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup().imm(data->numKeys)
                         .imm(uintptr_t(table))
                         .ssa(0/*values*/)
  );
}

void CodeGenerator::cgIncStat(IRInstruction *inst) {
  emitIncStat(m_mainCode,
              Stats::StatCounter(inst->src(0)->intVal()),
              inst->src(1)->intVal(),
              inst->src(2)->boolVal());
}

void CodeGenerator::cgIncTransCounter(IRInstruction* inst) {
  emitTransCounterInc(m_as);
}

void CodeGenerator::cgIncProfCounter(IRInstruction* inst) {
  TransID  transId = inst->extra<TransIDData>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);
  emitLoadImm(m_as, uint64_t(counterAddr), m_rScratch);
  m_as.decq(m_rScratch[0]);
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  emitAssertRefCount(m_as, srcLoc(0).reg());
}

void CodeGenerator::cgDbgAssertType(IRInstruction* inst) {
  emitTypeTest(inst->typeParam(),
               srcLoc(0).reg(1),
               srcLoc(0).reg(0),
               [&](ConditionCode cc) {
                 ifThen(m_as, ccNegate(cc), [&] { m_as.ud2(); });
               });
}

/*
 * Defined in translator-asm-helpers.S. Used for an assert in DbgAssertRetAddr.
 */
extern "C" void enterTCServiceReq();

void CodeGenerator::cgDbgAssertRetAddr(IRInstruction* inst) {
  // With the exception of FreeActRec and RetCtrl, the native return address
  // should always be the part of enterTCHelper that handles service
  // requests. To keep things reasonable we only emit this at the beginning of
  // a bytecode's translation, which should never begin with FreeActRec or
  // RetCtrl.
  always_assert(!inst->is(FreeActRec, RetCtrl));
  Immed64 imm = (uintptr_t)enterTCServiceReq;
  if (imm.fits(sz::dword)) {
    m_as.cmpq(imm.l(), *rsp);
  } else {
    m_as.emitImmReg(imm, m_rScratch);
    m_as.cmpq(m_rScratch, *rsp);
  }
  ifThen(m_as, CC_NE, [&] {
     m_as.ud2();
  });
}

void CodeGenerator::emitVerifyCls(IRInstruction* inst) {
  assert(!inst->src(0)->isConst());
  auto objClassReg = srcLoc(0).reg();
  SSATmp* constraint = inst->src(1);
  auto constraintReg = srcLoc(1).reg();

  if (constraintReg == InvalidReg) {
    auto imm = Immed64(constraint->clsVal());
    m_as.  cmpq(imm.l(), objClassReg);
  } else {
    m_as.  cmpq(constraintReg, objClassReg);
  }

  // The native call for this instruction is the slow path that does
  // proper subtype checking. The comparison above is just to
  // short-circuit the overhead when the Classes are an exact match.
  ifThen(m_as, CC_NE, [&]{ cgCallNative(m_as, inst); });
}

void CodeGenerator::cgVerifyParamCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgVerifyRetCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgRBTrace(IRInstruction* inst) {
  auto const& extra = *inst->extra<RBTrace>();

  TCA helper;
  auto args = argGroup();
  if (extra.msg) {
    auto const msg = extra.msg;
    assert(msg->isStatic());
    args.immPtr(msg->data());
    args.imm(msg->size());
    args.imm(extra.type);
    helper = (TCA)Trace::ringbufferMsg;
  } else {
    auto const sk = extra.sk;
    args.imm(extra.type);
    args.imm(sk.toAtomicInt());
    args.immPtr(m_as.frontier());
    helper = (TCA)Trace::ringbufferEntry;
  }

  cgCallHelper(m_as,
               CppCall::direct(reinterpret_cast<void (*)()>(helper)),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               args);
}

void CodeGenerator::cgLdClsInitData(IRInstruction* inst) {
  auto clsReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto offset = Class::propDataCacheOff() +
                RDS::Link<Class::PropInitVec*>::handleOff();
  m_as.  loadl(clsReg[offset], r32(dstReg));
  m_as.  loadq(rVmTl[dstReg], dstReg);
  m_as.  loadq(dstReg[Class::PropInitVec::dataOff()], dstReg);
}

void CodeGenerator::cgConjure(IRInstruction* inst) {
  m_as.ud2();
}

void CodeGenerator::cgProfileStr(IRInstruction* inst) {
  TargetProfile<StrProfile> profile(m_unit.context(), inst->marker(),
                                    inst->extra<ProfileStrData>()->key);
  assert(profile.profiling());
  auto const ch = profile.handle();

  auto ptrReg = srcLoc(0).reg();
  emitCmpTVType(m_as, KindOfStaticString, ptrReg[TVOFF(m_type)]);
  ifThenElse(
    m_as, CC_E,
    [&](Asm& a) { // m_type == KindOfStaticString
      a.incl(rVmTl[ch + offsetof(StrProfile, staticStr)]);
    },
    [&](Asm& a) { // m_type == KindOfString
      a.loadq(ptrReg[TVOFF(m_data)], m_rScratch);
      a.cmpl(StaticValue, m_rScratch[FAST_REFCOUNT_OFFSET]);

      ifThenElse(
        a, CC_E,
        [&](Asm& a) { // _count == StaticValue
          a.incl(rVmTl[ch + offsetof(StrProfile, strStatic)]);
        },
        [&](Asm& a) {
          a.incl(rVmTl[ch + offsetof(StrProfile, str)]);
        }
      );
    }
  );
}

void CodeGenerator::cgCountArray(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();

  m_as. cmpb(ArrayData::kNvtwKind, baseReg[ArrayData::offsetofKind()]);

  unlikelyIfThenElse(CC_Z,
    [&](Asm& a) { cgCallNative(a, inst); },
    [&](Asm& a) { a.  loadl(baseReg[ArrayData::offsetofSize()], r32(dstReg)); }
  );
}

void CodeGenerator::cgCountArrayFast(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();

  m_as. loadl(baseReg[ArrayData::offsetofSize()], r32(dstReg));
}

void CodeGenerator::cgCountCollection(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();

  m_as. loadl(baseReg[FAST_COLLECTION_SIZE_OFFSET], r32(dstReg));
}

void CodeGenerator::print() const {
  JIT::print(std::cout, m_unit, &m_state.regs, m_state.asmInfo);
}

}}}
