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
#include "hphp/runtime/ext/asio/asio_blockable.h"
#include "hphp/runtime/ext/asio/wait_handle.h"
#include "hphp/runtime/ext/asio/async_function_wait_handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/layout.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-x64.h"

using HPHP::jit::TCA;

namespace HPHP { namespace jit { namespace x64 {

TRACE_SET_MOD(hhir);

namespace {

//////////////////////////////////////////////////////////////////////

using namespace jit::reg;

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

template <class Then>
void CodeGenerator::unlikelyIfBlock(Vout& v, Vout& vcold,
                                    ConditionCode cc, Then then) {
  auto unlikely = vcold.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, {done, unlikely}};
  vcold = unlikely;
  then(vcold);
  if (!vcold.closed()) vcold << jmp{done};
  v = done;
}

template <class Block>
void CodeGenerator::ifBlock(Vout& v, Vout& vcold, ConditionCode cc,
                            Block taken, bool unlikely) {
  if (unlikely) return unlikelyIfBlock(v, vcold, cc, taken);
  auto takenLabel = v.makeBlock();
  auto doneLabel = v.makeBlock();
  v << jcc{cc, {doneLabel, takenLabel}};
  v = takenLabel;
  taken(v);
  if (!v.closed()) v << jmp{doneLabel};
  v = doneLabel;
}

// Generate an if-then-else block
template <class Then, class Else>
void CodeGenerator::ifThenElse(Vout& v, ConditionCode cc,
                               Then thenBlock, Else elseBlock) {
  auto thenLabel = v.makeBlock();
  auto elseLabel = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, {elseLabel, thenLabel}};
  v = thenLabel;
  thenBlock(v);
  if (!v.closed()) v << jmp{done};
  v = elseLabel;
  elseBlock(v);
  if (!v.closed()) v << jmp{done};
  v = done;
}

template <class Then, class Else>
void CodeGenerator::ifThenElse(Vout& v, Vout& vcold, ConditionCode cc,
                               Then thenBlock, Else elseBlock, bool unlikely) {
  if (unlikely) return unlikelyIfThenElse(v, vcold, cc, thenBlock, elseBlock);
  ifThenElse(v, cc, thenBlock, elseBlock);
}

template <class Then, class Else>
void CodeGenerator::unlikelyIfThenElse(Vout& v, Vout& vcold, ConditionCode cc,
                                       Then unlikelyBlock, Else elseBlock) {
  auto elseLabel = v.makeBlock();
  auto unlikelyLabel = vcold.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, {elseLabel, unlikelyLabel}};
  v = elseLabel;
  elseBlock(v);
  if (!v.closed()) v << jmp{done};
  vcold = unlikelyLabel;
  unlikelyBlock(vcold);
  if (!vcold.closed()) vcold << jmp{done};
  v = done;
}

/*
 * Generate an if-block that branches around some unlikely code, handling
 * the cases when a == astubs and a != astubs.  cc is the branch condition
 * to run the unlikely block.
 *
 * Passes the proper assembler to use to the unlikely function.
 */
template <class Then>
void unlikelyIfThen(Vout& vmain, Vout& vstub, ConditionCode cc, Then then) {
  auto unlikely = vstub.makeBlock();
  auto done = vmain.makeBlock();
  vmain << jcc{cc, {done, unlikely}};
  vstub = unlikely;
  then(vstub);
  if (!vstub.closed()) vstub << jmp{done};
  vmain = done;
}

// Generate an if-then-else block
template <class Then, class Else>
void ifThenElse(Vout& v, ConditionCode cc, Then thenBlock, Else elseBlock) {
  auto thenLabel = v.makeBlock();
  auto elseLabel = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, {elseLabel, thenLabel}};
  v = thenLabel;
  thenBlock();
  if (!v.closed()) v << jmp{done};
  v = elseLabel;
  elseBlock();
  if (!v.closed()) v << jmp{done};
  v = done;
}

/*
 * Same as ifThenElse except the first block is off in astubs
 */
template <class Then, class Else>
void unlikelyIfThenElse(Vout& vmain, Vout& vstub, ConditionCode cc,
                        Then unlikelyBlock, Else elseBlock) {
  auto elseLabel = vmain.makeBlock();
  auto unlikelyLabel = vstub.makeBlock();
  auto done = vmain.makeBlock();
  vmain << jcc{cc, {elseLabel, unlikelyLabel}};
  vmain = elseLabel;
  elseBlock(vmain);
  if (!vmain.closed()) vmain << jmp{done};
  vstub = unlikelyLabel;
  unlikelyBlock(vstub);
  if (!vstub.closed()) vstub << jmp{done};
  vmain = done;
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

void CodeGenerator::cgInst(IRInstruction* inst) {
  assert(!m_curInst && m_slocs.empty() && m_dlocs.empty());
  Opcode opc = inst->op();
  m_curInst = inst;
  SCOPE_EXIT {
    m_curInst = nullptr;
    m_slocs.clear();
    m_dlocs.clear();
  };
  if (!inst->is(Shuffle)) {
    // convert PhysLocs to Vlocs for all instructions except Shuffle.
    // Shuffle must use original PhysLocs because they can be spilled.
    auto& locs = m_state.regs[inst];
    for (unsigned i = 0, n = inst->numSrcs(); i < n; i++) {
      m_slocs.push_back(locs.src(i));
    }
    for (unsigned i = 0, n = inst->numDsts(); i < n; i++) {
      m_dlocs.push_back(locs.dst(i));
    }
  }
  switch (opc) {
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
      v << ud2{}; // or end?
    }
  }
}

#define NOOP_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction*) {}

#define CALL_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction* i) { cgCallNative(vmain(), i); }

#define CALL_STK_OPCODE(opcode) \
  CALL_OPCODE(opcode)           \
  CALL_OPCODE(opcode ## Stk)

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(TrackLoc)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(AssertStk)
NOOP_OPCODE(Nop)
NOOP_OPCODE(DefLabel)
NOOP_OPCODE(ExceptionBarrier)
NOOP_OPCODE(TakeStack)
NOOP_OPCODE(TakeRef)
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
CALL_OPCODE(ABCUnblock)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewVArray)
CALL_OPCODE(NewMIArray)
CALL_OPCODE(NewMSArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(NewPackedArray)
CALL_OPCODE(NewCol)
CALL_OPCODE(Clone)
CALL_OPCODE(AllocObj)
CALL_OPCODE(CustomInstanceInit)
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

Vlabel CodeGenerator::label(Block* b) {
  return m_state.labels[b];
}

void CodeGenerator::emitFwdJcc(Vout& v, ConditionCode cc, Block* target) {
  auto next = v.makeBlock();
  v << jcc{cc, {next, m_state.labels[target]}};
  v = next;
}

VregXMM CodeGenerator::prepXMM(Vout& v, const SSATmp* src, Vloc srcLoc) {
  assert(src->isA(Type::Bool) || src->isA(Type::Int) || src->isA(Type::Dbl));
  always_assert(srcLoc.reg().isValid());
  auto rsrc = srcLoc.reg();

  if (src->isA(Type::Dbl)) {
    if (rsrc.isGP()) {
      // double val in gpr
      auto tmp = v.makeReg();
      v << copy{rsrc, tmp};
      return tmp;
    }
    // src already in simd or vreg
    return rsrc;
  }

  // Case 2: src Dbl stored in GP reg
  if (src->isA(Type::Dbl)) {
    auto rtmp = v.makeReg();
    v << copy{rsrc, rtmp};
    return rtmp;
  }

  // Case 2.b: Bool or Int stored in GP reg
  auto s2 = zeroExtendIfBool(v, src, rsrc);
  auto rtmp = v.makeReg();
  v << cvtsi2sd{s2, rtmp};
  return rtmp;
}

void CodeGenerator::emitCompare(Vout& v, IRInstruction* inst) {
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
      v << cmpbi{src1->boolVal(), reg0};
    } else {
      v << cmpqi{safe_cast<int32_t>(src1->intVal()), reg0};
    }
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute reg0 - reg1
    if (type0 <= Type::Bool) {
      v << cmpb{reg1, reg0};
    } else {
      v << cmpq{reg1, reg0};
    }
  }
}

void CodeGenerator::emitCompareInt(Vout& v, IRInstruction* inst) {
  auto srcReg0 = srcLoc(0).reg();
  auto srcReg1 = srcLoc(1).reg();
  if (srcReg1 == InvalidReg) {
    v << cmpqi{safe_cast<int32_t>(inst->src(1)->intVal()), srcReg0};
  } else {
    // Note the reverse syntax in the assembler.
    // This cmp will compute srcReg0 - srcReg1
    v << cmpq{srcReg1, srcReg0};
  }
}

void CodeGenerator::emitReqBindJcc(Vout& v, ConditionCode cc,
                                   const ReqBindJccData* extra) {
  v << bindjcc1{cc, {extra->notTaken, extra->taken}};
}

void CodeGenerator::cgDefSP(IRInstruction* inst) {
  if (RuntimeOption::EvalHHIRGenerateAsserts && !inst->marker().resumed()) {
    auto& v = vmain();
    auto sp = v.makeReg();
    // Verify that rVmSp == rbp - spOff
    v << lea{rbp[-cellsToBytes(inst->extra<StackOffset>()->offset)], sp};
    v << cmpq{sp, rVmSp};
    ifBlock(v, vcold(), CC_NE, [](Vout& v) { v << ud2(); });
  }
}

void CodeGenerator::cgCheckNullptr(IRInstruction* inst) {
  if (!inst->taken()) return;
  auto reg = srcLoc(0).reg(0);
  auto& v = vmain();
  v << testq{reg, reg};
  v << jcc{CC_NZ, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckNonNull(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto taken  = inst->taken();
  assert(taken);

  auto& v = vmain();
  v << testq{srcReg, srcReg};
  if (dstReg != InvalidReg) {
    emitFwdJcc(v, CC_Z, taken);
    v << copy{srcReg, dstReg};
  } else {
    v << jcc{CC_Z, {label(inst->next()), label(taken)}};
  }
}

void CodeGenerator::cgAssertNonNull(IRInstruction* inst) {
  auto& v = vmain();
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << testq{srcReg, srcReg};
    ifThen(v, CC_Z, [&](Vout& v) {
      v << ud2{};
    });
  }
  v << copy{srcReg, dstReg};
}

void CodeGenerator::cgAssertType(IRInstruction* inst) {
  copyTV(vmain(), srcLoc(0), dstLoc(0));
}

void CodeGenerator::cgLdUnwinderValue(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0), rVmTl[unwinderTvOff()], inst->taken());
}

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  auto const& info = m_state.catches[inst->block()];
  assert(info.valid);
  auto& v = vmain();
  v << incstat{Stats::TC_CatchTrace, 1, false};

  // We want to restore state as though the call had completed
  // successfully, so skip over any stack arguments and pop any
  // saved registers.
  if (info.rspOffset) {
    v << addqi{info.rspOffset, rsp, rsp};
  }
  PhysRegSaverParity::emitPops(v, info.savedRegs);
}

static void unwindResumeHelper(_Unwind_Exception* data) {
  tl_regState = VMRegState::CLEAN;
  _Unwind_Resume(data);
}

static void callUnwindResumeHelper(Vout& v) {
  v << loadq{rVmTl[unwinderScratchOff()], rdi};
  v << call{(TCA)unwindResumeHelper, argSet(1)}; // call back to unwinder
  v << ud2{};
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  callUnwindResumeHelper(vmain());
}

void CodeGenerator::cgTryEndCatch(IRInstruction* inst) {
  auto& v = vmain();
  v << cmpbim{0, rVmTl[unwinderSideExitOff()]};
  unlikelyIfBlock(v, vcold(), CC_E, callUnwindResumeHelper);

  // doSideExit == true, so fall through to the side exit code
  v << incstat{Stats::TC_CatchSideExit, 1, false};
}

void CodeGenerator::cgDeleteUnwinderException(IRInstruction* inst) {
  auto& v = vmain();
  v << loadq{rVmTl[unwinderScratchOff()], rdi};
  v << call{(TCA)_Unwind_DeleteException, argSet(1)};
}

void CodeGenerator::cgJcc(IRInstruction* inst) {
  auto& v = vmain();
  auto cc = opToConditionCode(inst->op());
  emitCompare(v, inst);
  v << jcc{cc, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgJccInt(IRInstruction* inst) {
  auto& v = vmain();
  auto cc = opToConditionCode(inst->op());
  emitCompareInt(v, inst);
  v << jcc{cc, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgReqBindJcc(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  auto& v = vmain();
  emitCompare(v, inst);
  emitReqBindJcc(v, opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgReqBindJccInt(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  auto& v = vmain();
  emitCompareInt(v, inst);
  emitReqBindJcc(v, opToConditionCode(inst->op()),
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
  vmain() << ud2{};
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
  if (srcReg != Vreg{dstReg} && srcReg.isPhys()) {
    moves[dstReg] = srcReg;
    argDescs[dstReg] = &arg;
  }
  return false;
}

static int64_t shuffleArgs(Vout& v, ArgGroup& args, CppCall& call) {
  // Compute the move/shuffle plan.
  PhysReg::Map<PhysReg> moves;
  PhysReg::Map<ArgDesc*> argDescs;

  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    auto& arg = args.gpArg(i);
    if (shuffleArgsPlanningHelper(moves, argDescs, arg)) {
      continue;
    }
    switch (call.kind()) {
    case CppCall::Kind::IndirectReg:
      if (call.reg() == arg.dstReg()) {
        // an indirect call uses an argument register for the func ptr.
        // Use rax instead and update the CppCall
        moves[reg::rax] = call.reg();
        call.updateCallIndirect(reg::rax);
      }
      break;
    case CppCall::Kind::IndirectVreg:
      if (call.vreg() == Vreg{arg.dstReg()}) {
        // an indirect call uses an argument register for the func ptr.
        // use Rax instead and update the CppCall
        moves[reg::rax] = call.vreg();
        call.updateCallIndirect(reg::rax);
      }
      break;
    case CppCall::Kind::Direct:
    case CppCall::Kind::Virtual:
    case CppCall::Kind::ArrayVirt:
    case CppCall::Kind::Destructor:
      break;
    }
  }
  for (size_t i = 0; i < args.numSimdArgs(); ++i) {
    shuffleArgsPlanningHelper(moves, argDescs, args.simdArg(i));
  }

  // Store any arguments past the initial 6 to the stack. This has to happen
  // before the shuffles below in case the shuffles would clobber any of the
  // srcRegs here.
  for (int i = args.numStackArgs() - 1; i >= 0; --i) {
    auto& arg = args.stkArg(i);
    auto srcReg = arg.srcReg();
    assert(arg.dstReg() == InvalidReg);
    switch (arg.kind()) {
      case ArgDesc::Kind::Reg:
        if (arg.isZeroExtend()) {
          auto tmp = v.makeReg();
          v << movzbl{srcReg, tmp};
          v << push{tmp};
        } else {
          if (srcReg.isSIMD()) {
            auto tmp = v.makeReg();
            v << copy{srcReg, tmp};
            v << push{tmp};
          } else {
            v << push{srcReg};
          }
        }
        break;

      case ArgDesc::Kind::TypeReg:
        static_assert(kTypeWordOffset == 0 || kTypeWordOffset == 1,
                      "kTypeWordOffset value not supported");
        assert(srcReg.isGP());
        // x86 stacks grow down, so push higher offset items first
        if (kTypeWordOffset == 0) {
          v << push{srcReg};
        } else {
          // 4 bytes of garbage:
          v << pushl{eax};
          // get the type in the right place in rTmp before pushing it
          auto tmp1 = v.makeReg();
          v << movb{srcReg, tmp1};
          auto tmp2 = v.makeReg();
          v << shlli{CHAR_BIT, tmp1, tmp2};
          v << pushl{tmp2};
        }
        break;

      case ArgDesc::Kind::Imm: {
        auto tmp = v.makeReg();
        v << ldimm{arg.imm(), tmp};
        v << push{tmp};
        break;
      }

      case ArgDesc::Kind::Addr: {
        auto tmp = v.makeReg();
        auto base = arg.srcReg();
        v << lea{base[arg.disp().l()], tmp};
        v << push{tmp};
        break;
      }

      case ArgDesc::Kind::None:
        v << push{rax};
        if (RuntimeOption::EvalHHIRGenerateAsserts) {
          emitImmStoreq(v, 0xbadbadbadbadbad, *rsp);
        }
        break;
    }
  }

  // Execute the plan
  int num_moves = 0;
  auto const howTo = doVregMoves(v.unit(), moves);
  for (auto& how : howTo) {
    auto src = how.m_src;
    auto dst = how.m_dst;
    switch (how.m_kind) {
      case VMoveInfo::Kind::Move: {
        num_moves++;
        if (dst.isVirt()) {
          v << copy{src, dst};
        } else {
          ArgDesc* argDesc = argDescs[dst];
          if (argDesc == nullptr) {
            // when no ArgDesc is available is a straight reg to reg copy
            v << copy{src, dst};
          } else {
            ArgDesc::Kind kind = argDesc->kind();
            if (kind == ArgDesc::Kind::Reg || kind == ArgDesc::Kind::TypeReg) {
              if (argDesc->isZeroExtend()) {
                v << movzbl{src, dst};
              } else {
                v << copy{src, dst};
              }
            } else {
              assert(kind == ArgDesc::Kind::Addr);
              assert(!src.isSIMD());
              assert(!dst.isSIMD());
              auto base = src;
              v << lea{base[argDesc->disp().l()], dst};
            }
            if (kind != ArgDesc::Kind::TypeReg) {
              argDesc->markDone();
            }
          }
        }
        break;
      }
      case VMoveInfo::Kind::Xchg: {
        num_moves++;
        assert(!src.isSIMD());
        assert(!dst.isSIMD());
        v << copy2{src, dst, dst, src};
        break;
      }
    }
  }
  if (num_moves > 0) {
    TRACE(2, "arg-moves %d\n", num_moves);
  }

  // Handle const-to-register moves, virt-to-physical moves, type shifting,
  // load-effective address and zero extending for bools.
  // Ignore args that have been handled by doRegMoves already.
  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    auto& arg = args.gpArg(i);
    if (arg.done()) continue;
    ArgDesc::Kind kind = arg.kind();
    auto src = arg.srcReg();
    auto dst = arg.dstReg();
    assert(dst.isGP());
    if (kind == ArgDesc::Kind::Imm) {
      v << ldimm{arg.imm().q(), dst};
    } else if (kind == ArgDesc::Kind::None) {
      if (RuntimeOption::EvalHHIRGenerateAsserts) {
        v << ldimm{0xbadbadbadbadbad, dst};
      }
    } else if (kind == ArgDesc::Kind::TypeReg) {
      if (kTypeShiftBits > 0) {
        if (src.isVirt()) {
          v << shlqi{kTypeShiftBits, src, dst};
        } else {
          v << shlqi{kTypeShiftBits, dst, dst};
        }
      }
    } else if (kind == ArgDesc::Kind::Addr) {
      if (src.isVirt()) {
        v << addqi{arg.disp(), src, dst};
      } else {
        v << addqi{arg.disp(), dst, dst};
      }
    } else if (arg.isZeroExtend()) {
      if (src.isVirt()) {
        v << movzbl{src, dst};
      } else {
        v << movzbl{dst, dst};
      }
    } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
                 kind == ArgDesc::Kind::None) {
      v << ldimm{0xbadbadbadbadbad, dst};
    } else if (src.isVirt()) {
      v << copy{src, dst};
    }
  }

  for (size_t i = 0; i < args.numSimdArgs(); ++i) {
    auto &arg = args.simdArg(i);
    if (arg.done()) continue;
    auto kind = arg.kind();
    auto src = arg.srcReg();
    auto dst = arg.dstReg();
    assert(dst.isSIMD());
    if (kind == ArgDesc::Kind::Imm) {
      v << ldimm{arg.imm().q(), dst};
    } else if (RuntimeOption::EvalHHIRGenerateAsserts &&
               kind == ArgDesc::Kind::None) {
      v << ldimm{0xbadbadbadbadbad, dst};
    } else if (src.isVirt()) {
      v << copy{src, dst};
    }
  }

  return args.numStackArgs() * sizeof(int64_t);
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
        reinterpret_cast<void (*)()>(inst->src(info.func.srcIdx)->tcaVal()));
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

CallDest CodeGenerator::callDest(Vreg reg0) const {
  return { DestType::SSA, reg0 };
}

CallDest CodeGenerator::callDest(Vreg reg0, Vreg reg1) const {
  return { DestType::SSA, reg0, reg1 };
}

CallDest CodeGenerator::callDest(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  if (loc.numAllocated() == 0) return kVoidDest;
  assert(loc.numAllocated() == 1);
  return { DestType::SSA, loc.reg(0) };
}

CallDest CodeGenerator::callDestTV(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  if (loc.numAllocated() == 0) return kVoidDest;
  if (loc.isFullSIMD()) {
    assert(loc.numAllocated() == 1);
    return { DestType::SIMD, loc.reg(0) };
  }
  assert(loc.numAllocated() == 2);
  return { DestType::TV, loc.reg(0), loc.reg(1) };
}

CallDest CodeGenerator::callDestDbl(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(0);
  return { DestType::Dbl, loc.reg(0) };
}

void
CodeGenerator::cgCallHelper(Vout& v, CppCall call, const CallDest& dstInfo,
                            SyncOptions sync, ArgGroup& args) {
  return cgCallHelper(v, call, dstInfo, sync, args,
                      m_state.liveRegs[m_curInst]);
}

void
CodeGenerator::cgCallHelper(Vout& v, CppCall call, const CallDest& dstInfo,
                            SyncOptions sync, ArgGroup& args, RegSet toSave) {
  assert(m_curInst->isNative());

  auto const destType = dstInfo.type;
  auto const dstReg0  = dstInfo.reg0;
  auto const dstReg1  = dstInfo.reg1;

  // Save the caller-saved registers that are live across this
  // instruction. The number of regs to save and the number of args
  // being passed on the stack affect the parity of the PhysRegSaver,
  // so we use the generic version here.
  toSave = toSave & kCallerSaved;
  if (dstReg0.isPhys()) assert(!toSave.contains(dstReg0));
  if (dstReg1.isPhys()) assert(!toSave.contains(dstReg1));
  if (m_curInst->is(Call, CallArray)) assert(toSave.empty());
  PhysRegSaverParity regSaver(1 + args.numStackArgs(), v, toSave);

  // Assign registers to the arguments then prepare them for the call.
  RegSet argRegs;
  for (size_t i = 0; i < args.numGpArgs(); i++) {
    auto r = argNumToRegName[i];
    args.gpArg(i).setDstReg(r);
    argRegs.add(r);
  }
  for (size_t i = 0; i < args.numSimdArgs(); i++) {
    auto r = argNumToSIMDRegName[i];
    args.simdArg(i).setDstReg(r);
    argRegs.add(r);
  }
  regSaver.bytesPushed(shuffleArgs(v, args, call));

  // do the call; may use a trampoline
  if (sync == SyncOptions::kSmashableAndSyncPoint) {
    assert(call.kind() == CppCall::Kind::Direct);
    v << mccall{(TCA)call.address(), argRegs};
  } else {
    emitCall(v, call, argRegs);
  }
  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    // if we are profiling the heap, we always need to sync because
    // regs need to be correct during smart allocations no matter
    // what
    recordSyncPoint(v, sync);
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
    info.savedRegs = toSave;
    info.rspOffset = regSaver.rspAdjustment();
    assert(!info.valid);
    info.valid = true;
    auto next = v.makeBlock();
    v << unwind{{next, m_state.labels[taken]}};
    v = next;
  } else if (!m_curInst->is(Call, CallArray, ContEnter)) {
    // The current instruction doesn't have a catch block so it'd better not
    // throw. Register a null catch trace to indicate this to the
    // unwinder. Call and CallArray don't have catch blocks because they smash
    // all live values and optimizations are aware of this.
    v << nocatch{};
  }

  // copy the call result to the destination register(s)
  switch (destType) {
  case DestType::TV: {
      // rax contains m_type and m_aux but we're expecting just the
      // type in the lower bits, so shift the type result register.
      auto rval = packed_tv ? reg::rdx : reg::rax;
      auto rtyp = packed_tv ? reg::rax : reg::rdx;
      if (kTypeShiftBits > 0) v << shrqi{kTypeShiftBits, rtyp, rtyp};
      v << copy2{rval, rtyp, dstReg0, dstReg1};
    }
    break;
  case DestType::SIMD: {
      // rax contains m_type and m_aux but we're expecting just the
      // type in the lower bits, so shift the type result register.
      auto rval = packed_tv ? reg::rdx : reg::rax;
      auto rtyp = packed_tv ? reg::rax : reg::rdx;
      if (kTypeShiftBits > 0) v << shrqi{kTypeShiftBits, rtyp, rtyp};
      pack2(v, rval, rtyp, dstReg0);
    }
    break;
  case DestType::SSA:
    // copy the single-register result to dstReg0
    assert(!dstReg1.isValid());
    if (dstReg0.isValid()) v << copy{reg::rax, dstReg0};
    break;
  case DestType::None:
    break;
  case DestType::Dbl:
    // copy the single-register result to dstReg0
    assert(!dstReg1.isValid());
    if (dstReg0.isValid()) v << copy{reg::xmm0, dstReg0};
    break;
  }
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  always_assert(inst->src(0)->numWords() == inst->dst(0)->numWords());
  auto& v = vmain();
  if (srcLoc(0).hasReg(1)) {
    copyTV(v, srcLoc(0), dstLoc(0));
    return;
  }
  auto const src = inst->src(0);

  auto sreg = srcLoc(0).reg();
  auto dreg = dstLoc(0).reg();
  if (sreg != InvalidReg && dreg != InvalidReg) {
    v << copy{sreg, dreg};
  } else if (sreg == InvalidReg && dreg != InvalidReg) {
    // It won't have a raw value if it's null.
    auto const stype = src->type();
    assert(stype.hasRawVal() || stype <= Type::Null);
    auto const raw = stype.hasRawVal() ? src->rawVal() : 0;
    v << ldimm{raw, dreg};
  } else {
    assert(sreg == InvalidReg && dreg == InvalidReg);
  }
}

template<class OpInstr>
void CodeGenerator::cgUnaryIntOp(Vloc dst_loc, SSATmp* src, Vloc src_loc) {
  assert(src->isA(Type::Int));
  assert(dst_loc.reg().isValid());
  assert(src_loc.reg().isValid());
  auto dstReg = dst_loc.reg();
  auto srcReg = src_loc.reg();
  auto& v = vmain();

  // Integer operations require 64-bit representations
  auto s2 = zeroExtendIfBool(v, src, srcReg);
  v << OpInstr{s2, dstReg};
}

void CodeGenerator::cgAbsDbl(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();

  // clear the high bit
  auto resReg = dstReg.isSIMD() ? Vreg(dstReg) : v.makeReg();
  Vreg tmp_src;
  if (srcReg.isSIMD()) {
    tmp_src = srcReg;
  } else {
    tmp_src = v.makeReg();
    v << copy{srcReg, tmp_src};
  }
  auto tmp1 = v.makeReg();
  v << psllq{1, tmp_src, tmp1};
  v << psrlq{1, tmp1, resReg};
  if (resReg != Vreg(dstReg)) v << copy{resReg, dstReg};
}

template<class Op, class Opi>
void CodeGenerator::cgBinaryIntOp(IRInstruction* inst) {
  assert(m_curInst == inst); // could remove the inst param
  UNUSED const SSATmp* src0  = inst->src(0);
  UNUSED const SSATmp* src1  = inst->src(1);

  // inputs must be ints, or a (bool,bool) operation that ends up behaving
  // like an int anyway (e.g. XorBool)
  assert((src0->isA(Type::Int) && src1->isA(Type::Int)) ||
         (src0->isA(Type::Bool) && src1->isA(Type::Bool)));

  auto const dstReg      = dstLoc(0).reg();
  auto const src0Reg     = srcLoc(0).reg();
  auto const src1Reg     = srcLoc(1).reg();
  auto& v                = vmain();

  // LHS must always be assigned a register.
  assert(src0Reg != InvalidReg);

  if (src1Reg != InvalidReg) {
    // Two registers
    v << Op{src1Reg, src0Reg, dstReg};
  } else {
    // One register, one immediate
    auto imm = src1->isA(Type::Int) ? safe_cast<int32_t>(src1->intVal()) :
               int32_t(src1->boolVal());
    v << Opi{imm, src0Reg, dstReg};
  }
}

template<class Emit>
void CodeGenerator::cgBinaryDblOp(IRInstruction* inst, Emit emit) {
  assert(inst == m_curInst);
  const SSATmp* src0  = inst->src(0);
  const SSATmp* src1  = inst->src(1);
  auto loc0 = srcLoc(0);
  auto loc1 = srcLoc(1);
  assert(src0->isA(Type::Dbl) && src1->isA(Type::Dbl));

  auto& v = vmain();
  auto dstReg = dstLoc(0).reg();
  auto resReg = dstReg.isSIMD() && dstReg != loc1.reg() ? Vreg(dstReg) :
                v.makeReg();
  assert(resReg.isVirt() || resReg.isSIMD());

  auto srcReg0 = prepXMM(v, src0, loc0);
  auto srcReg1 = prepXMM(v, src1, loc1);

  emit(v, srcReg1, srcReg0, resReg);
  if (resReg != Vreg(dstReg)) v << copy{resReg, dstReg};
}

void CodeGenerator::cgAddIntO(IRInstruction* inst) {
  cgAddInt(inst);
  vmain() << jcc{CC_O, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgSubIntO(IRInstruction* inst) {
  cgSubInt(inst);
  vmain() << jcc{CC_O, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgMulIntO(IRInstruction* inst) {
  cgMulInt(inst);
  vmain() << jcc{CC_O, {label(inst->next()), label(inst->taken())}};
}

/*
 * If src2 is 1, this generates dst = src1 - 1 or src1 + 1 using the inc
 * or dec x86 instructions. The return value is whether or not the
 * instruction could be generated.
 */
template<class Inst>
bool CodeGenerator::emitIncDec(Vloc dst, SSATmp* src1, Vloc loc1,
                               SSATmp* src2, Vloc loc2) {
  auto& v = vmain();
  if (loc1.reg().isValid() && loc2.reg().isValid() &&
      src2->isConst(1)) {
    v << copy{loc1.reg(), dst.reg()};
    v << Inst{dst.reg(), dst.reg()};
    return true;
  }
  return false;
}

void CodeGenerator::cgRoundCommon(IRInstruction* inst, RoundDirection dir) {
  auto src = inst->src(0);
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  auto inReg  = prepXMM(v, src, srcLoc(0));
  auto outReg = dstReg.isSIMD() ? Vreg(dstReg) : v.makeReg();
  v << roundsd{dir, inReg, outReg};
  v << copy{outReg, dstReg};
}

void CodeGenerator::cgFloor(IRInstruction* inst) {
  cgRoundCommon(inst, RoundDirection::floor);
}

void CodeGenerator::cgCeil(IRInstruction* inst) {
  cgRoundCommon(inst, RoundDirection::ceil);
}

void CodeGenerator::cgAddInt(IRInstruction* inst) {
  SSATmp* src0 = inst->src(0);
  SSATmp* src1 = inst->src(1);
  auto loc0 = srcLoc(0);
  auto loc1 = srcLoc(1);
  auto dst = dstLoc(0);

  // Special cases: x = y + 1, x = 1 + y
  if (emitIncDec<incq>(dst, src0, loc0, src1, loc1) ||
      emitIncDec<incq>(dst, src1, loc1, src0, loc1)) {
    return;
  }

  cgBinaryIntOp<addq,addqi>(inst);
}

void CodeGenerator::cgSubInt(IRInstruction* inst) {
  auto src0 = inst->src(0);
  auto src1 = inst->src(1);
  auto loc0 = srcLoc(0);
  auto loc1 = srcLoc(1);
  auto dst = dstLoc(0);

  if (emitIncDec<decq>(dst, src0, loc0, src1, loc1)) return;

  if (src0->isConst(0)) {
    // There is no unary negate HHIR instruction, so handle that here.
    cgUnaryIntOp<neg>(dst, src1, loc1);
    return;
  }

  // not using cgBinaryIntOp because sub is not commutative, and we can do
  // r1=r0-r1 by doing neg r1; addq r1+=r0 without a scratch register.
  auto s0 = loc0.reg();
  auto s1 = loc1.reg();
  auto d = dst.reg();
  assert(s0 != InvalidReg && d != InvalidReg);
  auto& v = vmain();
  if (s1 == d) {
    v << neg{s1, d};
    v << addq{s0, s1, d};
  } else if (s1 == InvalidReg) {
    auto imm = src1->isA(Type::Int) ? safe_cast<int32_t>(src1->intVal()) :
               int32_t(src1->boolVal());
    v << subqi{imm, s0, d};
  } else {
    v << subq{s1, s0, d};
  }
}

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  // not using cgBinaryIntOp() here because x64 imul does not have
  // an immediate form. This means we can't provide a well-formed
  // Op class, and most of the complicated logic in cgBinaryIntOp
  // isn't necessary anyway.
  auto src0 = srcLoc(0).reg();
  auto src1 = srcLoc(1).reg();
  auto dst = dstLoc(0).reg();
  auto& v = vmain();
  assert(src0 != InvalidReg && src1 != InvalidReg && dst != InvalidReg);
  v << imul{src1, src0, dst};
}

void CodeGenerator::cgAddDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, [&](Vout& v, VregXMM s0, VregXMM s1, VregXMM res) {
    v << addsd{s0, s1, res};
  });
}

void CodeGenerator::cgSubDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, [&](Vout& v, VregXMM s0, VregXMM s1, VregXMM res) {
    v << subsd{s0, s1, res};
  });
}

void CodeGenerator::cgMulDbl(IRInstruction* inst) {
  cgBinaryDblOp(inst, [&](Vout& v, VregXMM s0, VregXMM s1, VregXMM res) {
    v << mulsd{s0, s1, res};
  });
}

void CodeGenerator::cgDivDbl(IRInstruction* inst) {
  const SSATmp* src0  = inst->src(0);
  const SSATmp* src1  = inst->src(1);
  auto loc0 = srcLoc(0);
  auto loc1 = srcLoc(1);
  auto exit = inst->taken();
  auto& v = vmain();
  auto dstReg  = dstLoc(0).reg();
  auto resReg = dstReg;
  if (!dstReg.isSIMD() || dstReg == loc1.reg()) {
    resReg = v.makeReg();
  }

  // only load divisor
  auto srcReg1 = prepXMM(v, src1, loc1);

  // divide by zero check
  auto zero = v.makeReg();
  v << ldimm{0, zero};
  v << ucomisd{zero, srcReg1};
  unlikelyIfBlock(v, vcold(), CC_NP, [&] (Vout& v) {
    emitFwdJcc(v, CC_E, exit);
  });

  // now load dividend
  auto srcReg0 = prepXMM(v, src0, loc0);

  v << divsd{srcReg1, srcReg0, resReg};
  v << copy{resReg, dstReg};
}

void CodeGenerator::cgAndInt(IRInstruction* inst) {
  cgBinaryIntOp<andq,andqi>(inst);
}

void CodeGenerator::cgOrInt(IRInstruction* inst) {
  cgBinaryIntOp<orq,orqi>(inst);
}

void CodeGenerator::cgXorInt(IRInstruction* inst) {
  if (inst->src(1)->isConst(-1)) {
    return cgUnaryIntOp<not>(dstLoc(0), inst->src(0), srcLoc(0));
  }
  cgBinaryIntOp<xorq,xorqi>(inst);
}

void CodeGenerator::cgXorBool(IRInstruction* inst) {
  cgBinaryIntOp<xorb,xorbi>(inst);
}

void CodeGenerator::cgMod(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const reg0 = srcLoc(0).reg();
  auto const reg1 = srcLoc(1).reg();
  auto& v = vmain();

  // spill rax and/or rdx
  Vreg save_rax, save_rdx;
  if (dstReg != rax) {
    save_rax = v.makeReg();
    v << copy{rax, save_rax};
  }
  if (dstReg != rdx) {
    save_rdx = v.makeReg();
    v << copy{rdx, save_rdx};
  }
  // put divisor in tmp if it would get clobbered
  auto divisor = reg1 != rax && reg1 != rdx ? Vreg{reg1} : v.makeReg();
  v << copy{reg1, divisor};
  // put dividend in rax
  v << copy{reg0, rax};
  v << cqo{};            // sign-extend rax => rdx:rax
  v << idiv{divisor};    // rdx:rax/divisor => quot:rax, rem:rdx
  v << copy{rdx, dstReg};
  // restore rax and/or rdx
  if (save_rax.isValid()) v << copy{save_rax, rax};
  if (save_rdx.isValid()) v << copy{save_rdx, rdx};
}

void CodeGenerator::cgSqrt(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  auto tmp1 = srcReg.isSIMD() ? Vreg(srcReg) : v.makeReg();
  auto tmp2 = dstReg.isSIMD() ? Vreg(dstReg) : v.makeReg();
  v << copy{srcReg, tmp1};
  v << sqrtsd{tmp1, tmp2};
  v << copy{tmp2, dstReg};
}

template<class Op, class Opi>
void CodeGenerator::cgShiftCommon(IRInstruction* inst) {
  auto const srcReg0 = srcLoc(0).reg();
  auto const srcReg1 = srcLoc(1).reg();
  auto const dstReg  = dstLoc(0).reg();
  assert(srcReg0 != InvalidReg);
  auto& v = vmain();

  // one immediate (right hand side)
  if (srcReg1 == InvalidReg) {
    v << copy{srcReg0, dstReg};
    v << Opi{safe_cast<int32_t>(inst->src(1)->intVal()), dstReg, dstReg};
    return;
  }

  // in order to shift by a variable amount src2 must be in rcx :(
  bool swapRCX = srcReg1 != reg::rcx;

  // we need rcx for srcReg1 so we use srcReg1 as a temp for rcx, we also need
  // to handle the cases where the destination is rcx or src2 or both...
  auto resReg = dstReg == srcReg1 ? v.makeReg() :
                dstReg == rcx ? Vreg(srcReg1) :
                Vreg(dstReg);

  // if srcReg0 was in rcx it will be swapped with srcReg1 below
  auto regLeft = srcReg0 == reg::rcx ? srcReg1 : srcReg0;

  // we use srcReg1 as a scratch for whatever is in rcx
  if (swapRCX) {
    v << copy2{reg::rcx, srcReg1, srcReg1, reg::rcx}; // emits xchgq
  }

  v << copy{regLeft, resReg};
  v << Op{resReg, resReg};

  if (resReg.isPhys() && resReg == dstReg && srcReg1 == dstReg) {
    // If we get here it, we shouldn't do any more swapping because
    // we stored the result in the right place
    return;
  }

  if (swapRCX) {
    v << copy2{reg::rcx, srcReg1, srcReg1, reg::rcx};
  }

  // if resReg == srcReg1 then dstReg must have been rcx and the above swap
  // already repaired the situation
  if (resReg.isVirt() || resReg != srcReg1) {
    v << copy{resReg, dstReg};
  }
}

void CodeGenerator::cgShl(IRInstruction* inst) {
  cgShiftCommon<shlq,shlqi>(inst);
}

void CodeGenerator::cgShr(IRInstruction* inst) {
  cgShiftCommon<sarq,sarqi>(inst);
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

#define CG_OP_CMP(inst, cc, name)                                   \
  cgCmpHelper(inst, cc, ccmp_ ## name, ccmp_ ## name,               \
              ccmp_ ## name, ccmp_ ## name, ccmp_ ## name, ccmp_ ## name)

// SON - string, object, or number
static bool typeIsSON(Type t) {
  return t.subtypeOfAny(Type::Str, Type::Obj, Type::Int, Type::Dbl);
}

void CodeGenerator::cgCmpHelper(IRInstruction* inst, ConditionCode cc,
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
  auto& v = vmain();

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
    cgCallHelper(v, CppCall::direct(str_cmp_str), callDest(inst),
      SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 2: bool/null cmp anything
  // simplifyCmp has converted all args to bool
  else if (type1 <= Type::Bool && type2 <= Type::Bool) {
    if (src2->isConst()) {
      v << cmpbi{src2->boolVal(), src1Reg};
    } else {
      v << cmpb{src2Reg, src1Reg};
    }
    v << setcc{cc, dstReg};
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
        cgCallHelper(v, CppCall::direct(str_cmp_int), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else if (type2 <= Type::Obj) {
        cgCallHelper(v, CppCall::direct(str_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else {
        CG_PUNT(cgOpCmpHelper_sx);
      }
    }

    else if (type1 <= Type::Obj) {
      // string cmp object is dealt with above
      // object cmp double is punted above

      if (type2 <= Type::Obj) {
        cgCallHelper(v, CppCall::direct(obj_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup().ssa(0).ssa(1));
      } else if (type2 <= Type::Int) {
        cgCallHelper(v, CppCall::direct(obj_cmp_int), callDest(inst),
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
    cgCallHelper(v, CppCall::direct(arr_cmp_arr),
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
  CG_OP_CMP(inst, CC_E, equal);
}

void CodeGenerator::cgEqX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_E, equal);
}

void CodeGenerator::cgNeq(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_NE, nequal);
}

void CodeGenerator::cgNeqX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_NE, nequal);
}

void CodeGenerator::cgSame(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_E, same);
}

void CodeGenerator::cgNSame(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_NE, nsame);
}

void CodeGenerator::cgLt(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_L, less);
}

void CodeGenerator::cgLtX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_L, less);
}

void CodeGenerator::cgGt(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_G, more);
}

void CodeGenerator::cgGtX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_G, more);
}

void CodeGenerator::cgLte(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_LE, lte);
}

void CodeGenerator::cgLteX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_LE, lte);
}

void CodeGenerator::cgGte(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_GE, gte);
}

void CodeGenerator::cgGteX(IRInstruction* inst) {
  CG_OP_CMP(inst, CC_GE, gte);
}

void CodeGenerator::emitCmpInt(IRInstruction* inst, ConditionCode cc) {
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  emitCompareInt(v, inst);
  v << setcc{cc, dstReg};
}

void CodeGenerator::cgEqInt(IRInstruction* inst)  { emitCmpInt(inst, CC_E); }
void CodeGenerator::cgNeqInt(IRInstruction* inst) { emitCmpInt(inst, CC_NE); }
void CodeGenerator::cgLtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_L); }
void CodeGenerator::cgGtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_G); }
void CodeGenerator::cgLteInt(IRInstruction* inst) { emitCmpInt(inst, CC_LE); }
void CodeGenerator::cgGteInt(IRInstruction* inst) { emitCmpInt(inst, CC_GE); }

void CodeGenerator::emitCmpEqDbl(IRInstruction* inst, ComparisonPred pred) {
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  auto srcReg0 = prepXMM(v, inst->src(0), srcLoc(0));
  auto srcReg1 = prepXMM(v, inst->src(1), srcLoc(1));
  auto tmp = v.makeReg();
  v << copy{srcReg1, tmp};
  v << cmpsd{pred, srcReg0, tmp, tmp};
  v << copy{tmp, dstReg};
  v << andbi{1, dstReg, dstReg};
}

void CodeGenerator::emitCmpRelDbl(IRInstruction* inst, ConditionCode cc,
                                  bool flipOperands) {
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  auto srcReg0 = prepXMM(v, inst->src(0), srcLoc(0));
  auto srcReg1 = prepXMM(v, inst->src(1), srcLoc(1));

  if (flipOperands) {
    std::swap(srcReg0, srcReg1);
  }

  v << ucomisd{srcReg0, srcReg1};
  v << setcc{cc, dstReg};
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
Vreg getDataPtrEnregistered(Vout&, Vreg dataSrc) {
  return dataSrc;
}

// Enregister the memoryRef so it can be used with an offset by the
// cmp instruction
Vreg getDataPtrEnregistered(Vout& v, Vptr dataSrc) {
  auto t = v.makeReg();
  v << loadq{dataSrc, t};
  return t;
}

template<class Loc1, class Loc2, class JmpFn>
void CodeGenerator::emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc,
                                 JmpFn doJcc) {
  assert(!(type <= Type::Cls));
  auto& v = vmain();
  ConditionCode cc;
  if (type <= Type::StaticStr) {
    emitCmpTVType(v, KindOfStaticString, typeSrc);
    cc = CC_E;
  } else if (type <= Type::Str) {
    assert(type != Type::CountedStr &&
           "We don't support guarding on CountedStr");
    emitTestTVType(v, KindOfStringBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Null) {
    emitCmpTVType(v, KindOfNull, typeSrc);
    cc = CC_LE;
  } else if (type == Type::UncountedInit) {
    emitTestTVType(v, KindOfUncountedInitBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Uncounted) {
    emitCmpTVType(v, KindOfRefCountThreshold, typeSrc);
    cc = CC_LE;
  } else if (type == Type::Cell) {
    assert(!m_curInst->is(LdRef));
    emitCmpTVType(v, KindOfRef, typeSrc);
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
    emitCmpTVType(v, dataType, typeSrc);
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

  auto& v = vmain();
  if (type < Type::Obj) {
    // emit the specific class test
    assert(type.getClass()->attrs() & AttrNoOverride);
    auto reg = getDataPtrEnregistered(v, dataSrc);
    emitCmpClass(v, type.getClass(), reg[ObjectData::getVMClassOffset()]);
    doJcc(CC_E);
  } else {
    assert(type < Type::Arr);
    auto reg = getDataPtrEnregistered(v, dataSrc);
    v << cmpbim{type.getArrayKind(), reg[ArrayData::offsetofKind()]};
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
    auto base = loc.reg();
    emitTypeTest(inst->typeParam(), base[TVOFF(m_type)],
                 base[TVOFF(m_data)], doJcc);
    return;
  }
  assert(src->isA(Type::Gen));

  auto typeSrcReg = loc.reg(1); // type register
  if (typeSrcReg == InvalidReg) {
    // Should only get here if the simplifier didn't run
    // TODO: #3626251 will handle this case.
    CG_PUNT(IsType-KnownType);
  }
  auto dataSrcReg = loc.reg(0); // data register
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
      emitFwdJcc(vmain(), ccNegate(cc), taken);
    });
}

template<class Loc>
void CodeGenerator::emitTypeGuard(Type type, Loc typeSrc, Loc dataSrc) {
  emitTypeTest(type, typeSrc, dataSrc,
    [&](ConditionCode cc) {
      auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
      vmain() << fallbackcc{ccNegate(cc), destSK};
    });
}

void CodeGenerator::emitSetCc(IRInstruction* inst, ConditionCode cc) {
  vmain() << setcc{cc, dstLoc(0).reg()};
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

void CodeGenerator::cgIsType(IRInstruction* inst) {
  cgIsTypeCommon(inst, false);
}

void CodeGenerator::cgIsScalarType(IRInstruction* inst) {
  auto typeReg = srcLoc(0).reg(1);
  auto dstReg  = dstLoc(0).reg(0);

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
  auto& v = vmain();
  if (typeReg == InvalidReg) {
    auto const type = inst->src(0)->type();
    auto const imm = type <= (Type::Bool | Type::Int | Type::Dbl | Type::Str);
    v << ldimm{imm, dstReg};
    return;
  }
  v << movzbl{typeReg, dstReg};
  v << subli{KindOfBoolean, dstReg, dstReg};
  v << subli{KindOfString - KindOfBoolean + 1, dstReg, dstReg};
  v << sbbl{dstReg, dstReg, dstReg};
  v << neg{dstReg, dstReg};
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
void CodeGenerator::emitInstanceBitmaskCheck(Vout& v, IRInstruction* inst) {
  auto const rObjClass     = srcLoc(0).reg(0);
  auto const testClassName = inst->src(1)->strVal();
  int offset;
  uint8_t mask;
  if (!InstanceBits::getMask(testClassName, offset, mask)) {
    always_assert(!"cgInstanceOfBitmask had no bitmask");
  }
  v << testbim{int8_t(mask), rObjClass[offset]};
}

void CodeGenerator::cgInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << setcc{CC_NZ, dstLoc(0).reg()};
}

void CodeGenerator::cgNInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << setcc{CC_Z, dstLoc(0).reg()};
}

void CodeGenerator::cgJmpInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << jcc{CC_NZ, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgJmpNInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << jcc{CC_Z, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgReqBindJmpInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  emitReqBindJcc(v, opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgReqBindJmpNInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  emitReqBindJcc(v, opToConditionCode(inst->op()),
                 inst->extra<ReqBindJccData>());
}

void CodeGenerator::cgSideExitJmpInstanceOfBitmask(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgSideExitJmpNInstanceOfBitmask(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  auto& v = vmain();
  emitInstanceBitmaskCheck(v, inst);
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgInstanceOf(IRInstruction* inst) {
  auto testReg = srcLoc(1).reg();
  auto destReg = dstLoc(0).reg();
  auto& v = vmain();

  if (testReg == InvalidReg) {
    // Don't need to do the null check when the class is const.
    assert(inst->src(1)->clsVal() != nullptr);
    cgCallNative(v, inst);
    return;
  }

  v << testq{testReg, testReg};
  ifThenElse(v, CC_NZ,
    [&](Vout& v) {
      cgCallNative(v, inst);
    },
    [&](Vout& v) {
      // testReg == 0, set dest to false (0)
      v << copy{testReg, destReg};
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
  auto const rdst          = dstLoc(0).reg();
  auto& v = vmain();

  auto done = v.makeBlock();
  auto notExact = v.makeBlock();
  auto falseLabel = v.makeBlock();

  Vreg rTestClass;
  auto s1 = srcLoc(1).reg();
  if (s1 != InvalidReg) {
    rTestClass = s1;
  } else {
    // even though src 1 is C(Cls), another use of the same tmp might
    // have given it a register. If that happened, use the register.
    rTestClass = v.makeReg();
    v << ldimm{testClass, rTestClass};
  }

  // Test if it is the exact same class.  TODO(#2044801): we should be
  // doing this control flow at the IR level.
  if (!(testClass->attrs() & AttrAbstract)) {
    emitCmpClass(v, rTestClass, rObjClass);
    // If the test class cannot be extended, we only need to do the
    // exact check.
    if (testClass->attrs() & AttrNoOverride) {
      v << setcc{CC_E, rdst};
      return;
    }
    auto next = v.makeBlock();
    v << jcc{CC_NE, {next, notExact}};
    v = next;
    v << ldimm{1, rdst};
    v << jmp{done};
  } else {
    v << jmp{notExact};
  }

  auto const vecOffset = Class::classVecOff() +
    sizeof(LowClassPtr) * (testClass->classVecLen() - 1);

  // Check the length of the class vectors---if the candidate's is at
  // least as long as the potential base (testClass) it might be a
  // subclass.
  v = notExact;
  v << cmplim{safe_cast<int32_t>(testClass->classVecLen()),
              rObjClass[Class::classVecLenOff()]};
  auto next = v.makeBlock();
  v << jcc{CC_B, {next, falseLabel}};
  v = next;

  // If it's a subclass, rTestClass must be at the appropriate index.
  emitCmpClass(v, rTestClass, rObjClass[vecOffset]);
  v << setcc{CC_E, rdst};
  v << jmp{done};

  v = falseLabel;
  v << ldimm{0, rdst};
  v << jmp{done};

  v = done;
}

void CodeGenerator::cgConvDblToInt(IRInstruction* inst) {
  auto src = inst->src(0);
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();

  auto srcReg = prepXMM(v, src, srcLoc(0));

  constexpr uint64_t maxULongAsDouble  = 0x43F0000000000000LL;
  constexpr uint64_t maxLongAsDouble   = 0x43E0000000000000LL;

  auto rIndef = v.makeReg();
  v << ldimm{0x8000000000000000LL, rIndef};
  v << cvttsd2siq{srcReg, dstReg};
  v << cmpq{rIndef, dstReg};

  unlikelyIfBlock(v, vcold(), CC_E, [&] (Vout& v) {
    // result > max signed int or unordered
    auto tmp = v.makeReg();
    v << ldimm{0, tmp};
    v << ucomisd{tmp, srcReg};
    ifThen(v, CC_B, [&](Vout& v) {
      // src0 > 0 (CF = 1 -> less than 0 or unordered)
      ifThen(v, CC_NP, [&](Vout& v) {
        auto max_ulong = v.makeReg();
        v << ldimm{maxULongAsDouble, max_ulong};
        v << ucomisd{max_ulong, srcReg};
        ifThenElse(v, CC_B, [&](Vout& v) {
          // src0 > ULONG_MAX
          v << ldimm{0, dstReg};
        }, [&](Vout& v) {
          // 0 < src0 <= ULONG_MAX
          auto max_long = v.makeReg();
          v << ldimm{maxLongAsDouble, max_long};

          // we know that LONG_MAX < src0 <= UINT_MAX, therefore,
          // 0 < src0 - ULONG_MAX <= LONG_MAX
          auto tmp_sub = v.makeReg();
          v << subsd{max_long, srcReg, tmp_sub};
          v << cvttsd2siq{tmp_sub, dstReg};

          // We want to simulate integer overflow so we take the resulting
          // integer and flip its sign bit (NB: we don't use orq here
          // because it's possible that src0 == LONG_MAX in which case
          // cvttsd2siq will yeild an indefiniteInteger, which we would
          // like to make zero)
          v << xorq{rIndef, dstReg, dstReg};
        });
      });
    });
  });
}

void CodeGenerator::cgConvDblToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  auto& v = vmain();
  v << copy{srcReg, dstReg};
  v << shlqi{1, dstReg, dstReg}; // 0.0 stays zero and -0.0 is now 0.0
  v << setcc{CC_NE, dstReg}; // lower byte becomes 1 if dstReg != 0
  v << movzbl{dstReg, dstReg};
}

void CodeGenerator::cgConvIntToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  auto& v = vmain();
  v << testq{srcReg, srcReg};
  v << setcc{CC_NE, dstReg};
  v << movzbl{dstReg, dstReg};
}

void CodeGenerator::cgConvArrToBool(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  auto& v = vmain();

  // This will incorrectly result in "true" for a NameValueTableWrapper that is
  // empty. You can only get such a thing through very contrived PHP, so the
  // savings of a branch and a block of cold code outweights the edge-case bug.
  v << cmplim{0, srcReg[ArrayData::offsetofSize()]};
  v << setcc{CC_NZ, dstReg};
}

/*
 * emit something equivalent to testl(val, mr),
 * but with a shorter encoding (eg testb(val, mr))
 * if possible.
 */
void testimm(Vout& v, uint32_t val, Vptr mr) {
  int off = 0;
  auto val2 = val;
  while (val2 > 0xff && !(val2 & 0xff)) {
    off++;
    val2 >>= 8;
  }
  if (val2 > 0xff) {
    v << testlim{(int32_t)val, mr};
  } else {
    v << testbim{(int8_t)val2, mr + off};
  }
}

void CodeGenerator::cgColIsEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assert(ty < Type::Obj &&
         ty.getClass() &&
         ty.getClass()->isCollectionClass());
  auto& v = vmain();
  v << cmplim{0, srcLoc(0).reg()[FAST_COLLECTION_SIZE_OFFSET]};
  v << setcc{CC_E, dstLoc(0).reg()};
}

void CodeGenerator::cgColIsNEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assert(ty < Type::Obj &&
         ty.getClass() &&
         ty.getClass()->isCollectionClass());
  auto& v = vmain();
  v << cmplim{0, srcLoc(0).reg()[FAST_COLLECTION_SIZE_OFFSET]};
  v << setcc{CC_NE, dstLoc(0).reg()};
}

void CodeGenerator::cgConvObjToBool(IRInstruction* inst) {
  auto const rdst = dstLoc(0).reg();
  auto const rsrc = srcLoc(0).reg();
  auto& v = vmain();

  testimm(v, ObjectData::CallToImpl, rsrc[ObjectData::attributeOff()]);
  unlikelyIfThenElse(v, vcold(),
    CC_NZ,
    [&] (Vout& v) {
      testimm(v,
              ObjectData::IsCollection,
              rsrc[ObjectData::attributeOff()]);
      ifThenElse(
        v,
        CC_NZ,
        [&] (Vout& v) { // rsrc points to native collection
          v << cmplim{0, rsrc[FAST_COLLECTION_SIZE_OFFSET]};
          v << setcc{CC_NE, rdst}; // true iff size not zero
        },
        [&] (Vout& v) { // rsrc is not a native collection
          cgCallHelper(
            v,
            CppCall::method(&ObjectData::o_toBoolean),
            callDest(inst),
            SyncOptions::kSyncPoint,
            argGroup().ssa(0));
        }
      );
    },
    [&] (Vout& v) {
      v << ldimm{1, rdst};
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
  auto& v = vmain();
  auto s2 = zeroExtendIfBool(v, src, srcReg);
  auto tmp_dbl = v.makeReg();
  v << cvtsi2sd{s2, tmp_dbl};
  v << copy{tmp_dbl, dstReg};
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
  vmain() << movzbl{srcReg, dstReg};
}

void CodeGenerator::cgConvBoolToStr(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto srcReg = srcLoc(0).reg();
  auto& v = vmain();
  auto f = v.makeReg();
  auto t = v.makeReg();
  v << testb{srcReg, srcReg};
  v << ldimm{makeStaticString(""), f};
  v << ldimm{makeStaticString("1"), t};
  v << cmovq{CC_NZ, f, t, dstReg};
}

void CodeGenerator::cgConvClsToCctx(IRInstruction* inst) {
  auto const sreg = srcLoc(0).reg();
  auto const dreg = dstLoc(0).reg();
  auto& v = vmain();
  v << copy{sreg, dreg};
  v << orqi{1, dreg, dreg};
}

void CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(srcReg != InvalidReg);
  auto& v = vmain();
  v << copy{srcReg, dstReg};
  emitDerefIfVariant(v, dstReg);
}

void CodeGenerator::cgLdFuncCachedCommon(IRInstruction* inst) {
  auto const dst  = dstLoc(0).reg();
  auto const name = inst->extra<LdFuncCachedData>()->name;
  auto const ch   = NamedEntity::get(name)->getFuncHandle();
  auto& v = vmain();
  if (dst == InvalidReg) {
    v << cmpqim{0, rVmTl[ch]};
  } else {
    v << loadq{rVmTl[ch], dst};
    v << testq{dst, dst};
  }
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst);
  unlikelyIfBlock(vmain(), vcold(), CC_Z, [&] (Vout& v) {
    const Func* (*const func)(const StringData*) = lookupUnknownFunc;
    cgCallHelper(v,
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
    vmain() << jcc{CC_Z, {label(inst->next()), label(taken)}};
  }
}

void CodeGenerator::cgLdFuncCachedU(IRInstruction* inst) {
  auto const dstReg    = dstLoc(0).reg();
  auto const extra     = inst->extra<LdFuncCachedU>();
  auto const hFunc     = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain();

  // Check the first function handle, otherwise try to autoload.
  if (dstReg == InvalidReg) {
    v << cmpqim{0, rVmTl[hFunc]};
  } else {
    v << loadq{rVmTl[hFunc], dstReg};
    v << testq{dstReg, dstReg};
  }

  unlikelyIfBlock(v, vcold(), CC_Z, [&] (Vout& v) {
    // If we get here, things are going to be slow anyway, so do all the
    // autoloading logic in lookupFallbackFunc instead of ASM
    const Func* (*const func)(const StringData*, const StringData*) =
        lookupFallbackFunc;
    cgCallHelper(
      v,
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
  cgCallHelper(vmain(),
               CppCall::direct(FuncCache::lookup),
               callDest(dstLoc(0).reg()),
               SyncOptions::kSyncPoint,
               argGroup().imm(ch).ssa(0/*methodName*/));
}

void CodeGenerator::cgLdObjClass(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto objReg = srcLoc(0).reg();
  emitLdObjClass(vmain(), objReg, dstReg);
}

void CodeGenerator::cgLdObjMethod(IRInstruction* inst) {
  assert(inst->taken() && inst->taken()->isCatch()); // must have catch block
  using namespace MethodCache;

  auto const clsReg    = srcLoc(0).reg();
  auto const actRecReg = srcLoc(1).reg();
  auto const extra     = inst->extra<LdObjMethodData>();
  auto& v = vmain();

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

  auto fast_path = v.makeBlock();
  auto slow_path = v.makeBlock();
  auto done = v.makeBlock();

  /*
   * Inline cache: we "prime" the cache across requests by smashing
   * this immediate to hold a Func* in the upper 32 bits, and a Class*
   * in the lower 32 bits.  (If both are low-malloced pointers can
   * fit.)  See pmethodCacheMissPath.
   */
  auto func_class = v.makeReg();
  auto classptr = v.makeReg();
  v << mcprep{func_class};
  v << movl{func_class, classptr};  // zeros the top 32 bits
  v << cmpq{classptr, clsReg};
  v << jcc{CC_NE, {fast_path, slow_path}};

  v = fast_path;
  auto funcptr = v.makeReg();
  v << shrqi{32, func_class, funcptr};
  v << storeq{funcptr, actRecReg[AROFF(m_func)]};
  v << jmp{done};

  v = slow_path;
  cgCallHelper(v,
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
      .reg(func_class)
  );
  v << jmp{done};
  v = done;
}

void CodeGenerator::cgLdObjInvoke(IRInstruction* inst) {
  auto const rsrc = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();
  auto& v = vmain();
  v << loadq{rsrc[Class::invokeOff()], rdst};
  v << testq{rdst, rdst};
  v << jcc{CC_Z, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgStRetVal(IRInstruction* inst) {
  auto  const rFp = srcLoc(0).reg();
  auto* const val = inst->src(1);
  cgStore(rFp[AROFF(m_r)], val, srcLoc(1), Width::Full);
}

void CodeGenerator::cgRetAdjustStack(IRInstruction* inst) {
  auto const rFp   = srcLoc(0).reg();
  auto const dstSp = dstLoc(0).reg();
  vmain() << lea{rFp[AROFF(m_r)], dstSp};
}

void CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg(0);
  assert(fpReg != InvalidReg);
  vmain() << pushm{fpReg[AROFF(m_savedRip)]};
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == mcg->tx().uniqueStubs.callToExit) {
    return;
  }
  checkFrame(fp, sp, /*fullCheck*/ false, 0);
  assert(sp <= (Cell*)fp || fp->resumed());
  // check return value if stack not empty
  if (sp < (Cell*)fp) assertTv(sp);
}

void CodeGenerator::emitTraceRet(Vout& v) {
  // call to a trace function
  v << movq{rVmFp, rdi};
  v << movq{rVmSp, rsi};
  v << loadq{*rsp, rdx}; // return ip from native stack
  // do the call; may use a trampoline
  v << call{TCA(traceRet), argSet(3)};
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  auto& v = vmain();
  // Make sure rVmFp and rVmSp are set appropriately
  auto sp = srcLoc(0).reg();
  auto fp = srcLoc(1).reg();
  if (sp != rVmSp) v << copy{sp, rVmSp};
  if (fp != rVmFp) v << copy{fp, rVmFp};

  // Return control to caller
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    emitTraceRet(v);
  }

  v << ret{};
}

void CodeGenerator::cgLdBindAddr(IRInstruction* inst) {
  auto data   = inst->extra<LdBindAddr>();
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  auto& vf = vfrozen();

  // Emit service request to smash address of SrcKey into 'addr'.
  TCA* addrPtr = mcg->allocData<TCA>(sizeof(TCA), 1);
  vf = vf.makeEntry();
  vf << bindaddr{addrPtr, data->sk};

  // Load the maybe bound address.
  auto addr = intptr_t(addrPtr);
  // the tc/global data is intentionally layed out to guarantee
  // rip-relative addressing will work.
  // Also, a rip-relative load, is 1 byte smaller than the corresponding
  // baseless load.
  v << loadqp{rip[addr], dstReg};
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  JmpSwitchData* data = inst->extra<JmpSwitchDest>();
  SSATmp* index       = inst->src(0);
  auto indexReg       = srcLoc(0).reg();
  auto& v = vmain();
  auto& vf = vfrozen();

  if (!index->isConst()) {
    if (data->bounded) {
      if (data->base) {
        //XXX it is unsound to mutate indexReg
        if (deltaFits(data->base, sz::dword)) {
          v << subqi{safe_cast<int32_t>(data->base), indexReg, indexReg};
        } else {
          auto t = v.makeReg();
          v << ldimm{data->base, t};
          v << subq{t, indexReg, indexReg};
        }
      }
      v << cmpqi{data->cases - 2, indexReg};
      v << bindjcc2{CC_AE, data->defaultOff};
    }

    TCA* table = mcg->allocData<TCA>(sizeof(TCA), data->cases);
    auto t = v.makeReg();
    v << leap{rip[(intptr_t)table], t};
    v << jmpm{t[indexReg*8]};
    for (int i = 0; i < data->cases; i++) {
      auto sk = SrcKey(curFunc(), data->targets[i], resumed());
      vf = vf.makeEntry();
      vf << bindaddr{&table[i], sk};
    }
  } else {
    int64_t indexVal = index->intVal();
    if (data->bounded) {
      indexVal -= data->base;
      if (indexVal >= data->cases - 2 || indexVal < 0) {
        auto dest = SrcKey(curFunc(), data->defaultOff, resumed());
        v << bindjmp{dest};
        return;
      }
    }
    auto dest = SrcKey(curFunc(), data->targets[indexVal], resumed());
    v << bindjmp{dest};
  }
}

void CodeGenerator::cgLdSSwitchDestFast(IRInstruction* inst) {
  auto data = inst->extra<LdSSwitchDestFast>();

  auto table = mcg->allocData<SSwitchMap>(64);
  new (table) SSwitchMap(data->numCases);
  auto& v = vmain();
  auto& vf = vfrozen();
  for (int64_t i = 0; i < data->numCases; ++i) {
    table->add(data->cases[i].str, nullptr);
    TCA* addr = table->find(data->cases[i].str);
    auto sk = SrcKey(curFunc(), data->cases[i].dest, resumed());
    vf = vf.makeEntry();
    vf << bindaddr{addr, sk};
  }
  TCA* def = mcg->allocData<TCA>(sizeof(TCA), 1);
  auto sk = SrcKey(curFunc(), data->defaultOff, resumed());
  vf = vf.makeEntry();
  vf << bindaddr{def, sk};
  cgCallHelper(v,
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
  auto& v = vmain();
  auto& vf = vfrozen();
  for (int i = 0; i < data->numCases; ++i) {
    strtab[i] = data->cases[i].str;
    auto sk = SrcKey(curFunc(), data->cases[i].dest, resumed());
    vf = vf.makeEntry();
    vf << bindaddr{&jmptab[i], sk};
  }
  auto sk = SrcKey(curFunc(), data->defaultOff, resumed());
  vf = vf.makeEntry();
  vf << bindaddr{&jmptab[data->numCases], sk};
  cgCallHelper(v,
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
  auto& v = vmain();
  v << storeq{callerFP, calleeFP[AROFF(m_sfp)]};
  emitImmStoreq(v, intptr_t(fakeRet), calleeFP[AROFF(m_savedRip)]);
  v << storelim{retBCOff, calleeFP[AROFF(m_soff)]};
  cgMov(inst);
}

void CodeGenerator::cgInlineReturn(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  assert(fpReg == rVmFp);
  vmain() << loadq{fpReg[AROFF(m_sfp)], rVmFp};
}

void CodeGenerator::cgReDefSP(IRInstruction* inst) {
  // TODO(#2288359): this instruction won't be necessary (for
  // non-generator frames) when we don't track rVmSp independently
  // from rVmFp.  In generator frames we'll have to track offsets from
  // a DefResumableSP or something similar.
  auto fp  = srcLoc(1).reg();
  auto dst = dstLoc(0).reg();
  auto off = -inst->extra<ReDefSP>()->spOffset * sizeof(Cell);
  vmain() << lea{fp[off], dst};
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  auto ptr = srcLoc(0).reg();
  auto off = AROFF(m_sfp);
  auto dst = dstLoc(0).reg();
  vmain() << loadq{ptr[off], dst};
}

void emitSpill(Vout& v, PhysLoc s, PhysLoc d, Type t) {
  assert(s.numWords() == d.numWords());
  assert(d.spilled());
  if (s.isFullSIMD()) {
    v << storedqu{s.reg(0), reg::rsp[d.offset(0)]};
  } else {
    for (int i = 0, n = s.numAllocated(); i < n; ++i) {
      // store the whole register even if it holds a bool or DataType
      v << store{s.reg(i), reg::rsp[d.offset(i)]};
    }
  }
}

void emitReload(Vout& v, PhysLoc s, PhysLoc d, Type t) {
  assert(s.numWords() == d.numWords());
  assert(s.spilled());
  if (d.isFullSIMD()) {
    v << loaddqu{reg::rsp[s.offset(0)], d.reg(0)};
  } else {
    for (int i = 0, n = d.numAllocated(); i < n; ++i) {
      // load the whole register even if it holds a bool or DataType
      v << load{reg::rsp[s.offset(i)], d.reg(i)};
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
  auto& v = vmain();
  auto& locs = m_state.regs[inst];
  PhysReg::Map<PhysReg> moves;    // moves[dst] = src
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; ++i) {
    auto rd = inst->extra<Shuffle>()->dests[i];
    if (rd.numAllocated() == 0) continue; // ignore unused dests.
    auto src = inst->src(i);
    auto rs = locs.src(i);
    if (rd.spilled()) {
      emitSpill(v, rs, rd, src->type());
    } else if (!rs.spilled()) {
      auto s0 = rs.reg(0);
      auto d0 = rd.reg(0);
      if (s0 != InvalidReg) moves[d0] = s0;
      auto s1 = rs.reg(1);
      auto d1 = rd.reg(1);
      if (s1 != InvalidReg) moves[d1] = s1;
    }
  }
  // Compute a serial order of moves and swaps.
  auto howTo = doVregMoves(v.unit(), moves);
  for (auto& how : howTo) {
    if (how.m_kind == VMoveInfo::Kind::Move) {
      v << copy{how.m_src, how.m_dst};
    } else {
      // do swap - only support GPRs
      assert(how.m_src.isGP() && how.m_dst.isGP());
      v << copy2{how.m_src, how.m_dst, how.m_dst, how.m_src};
    }
  }
  // now do reg<-mem loads and reg<-imm moves. We have already
  // dealt with stores, moves, and swaps.
  for (uint32_t i = 0, n = inst->numSrcs(); i < n; ++i) {
    auto src = inst->src(i);
    auto rs = locs.src(i);
    auto rd = inst->extra<Shuffle>()->dests[i];
    if (rd.numAllocated() == 0) continue; // ignore unused dests.
    if (rd.spilled()) continue;
    if (rs.spilled()) {
      emitReload(v, rs, rd, src->type());
      continue;
    }
    if (rs.numAllocated() == 0) {
      assert(src->isConst());
      auto r = rd.reg(0);
      auto imm = src->type().needsValueReg() ? src->rawVal() :
                 0xdeadbeef;
      if (src->type().needsValueReg() ||
          RuntimeOption::EvalHHIRGenerateAsserts) {
        v << ldimm{imm, r};
      }
    }
    if (rd.numAllocated() == 2 && rs.numAllocated() < 2) {
      // move a src known type to a dest register
      //         a.emitImmReg(arg.imm().q(), dst);
      assert(src->type().isKnownDataType());
      v << ldimm{src->type().toDataType(), rd.reg(1)};
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
  if (destReg != InvalidReg) vmain() << copy{ptr, destReg};
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
  auto& v = vmain();
  auto fp = srcLoc(0).reg();
  auto sp = srcLoc(1).reg();
  if (fp != rVmFp) v << copy{fp, rVmFp};
  if (sp != rVmSp) v << copy{sp, rVmSp};
}

void CodeGenerator::cgEagerSyncVMRegs(IRInstruction* inst) {
  always_assert(
    srcLoc(0).reg() == rVmFp &&
    srcLoc(1).reg() == rVmSp
  );
  auto& v = vmain();
  emitEagerSyncPoint(v, reinterpret_cast<const Op*>(inst->marker().sk().pc()));
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  auto offset  = inst->extra<ReqBindJmp>()->offset;
  auto trflags = inst->extra<ReqBindJmp>()->trflags;
  auto dest = SrcKey(curFunc(), offset, resumed());
  vmain() << bindjmp{dest, trflags};
}

void CodeGenerator::cgReqRetranslateOpt(IRInstruction* inst) {
  auto extra = inst->extra<ReqRetranslateOpt>();
  auto& v = vmain();
  auto& vc = vcold();
  auto sr = vc.makeBlock();
  v << jmp{sr};
  vc = sr;
  auto sk = SrcKey(curFunc(), extra->offset, resumed());
  vc << retransopt{sk, extra->transId};
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  assert(m_unit.bcOff() == inst->marker().bcOff());
  auto const destSK = SrcKey(curFunc(), m_unit.bcOff(), resumed());
  auto trflags = inst->extra<ReqRetranslate>()->trflags;
  vmain() << fallback{destSK, trflags};
}

void CodeGenerator::cgIncRefWork(Type type, SSATmp* src, Vloc srcLoc) {
  assert(type.maybeCounted());
  auto& v = vmain();
  auto increfMaybeStatic = [&](Vout& v) {
    auto base = srcLoc.reg(0);
    if (!type.needsStaticBitCheck()) {
      emitIncRef(v, base);
    } else {
      v << cmplim{0, base[FAST_REFCOUNT_OFFSET]};
      static_assert(UncountedValue < 0 && StaticValue < 0, "");
      ifThen(v, CC_NS, [&](Vout& v) { emitIncRef(v, base); });
    }
  };

  if (type.isKnownDataType()) {
    assert(IS_REFCOUNTED_TYPE(type.toDataType()));
    increfMaybeStatic(v);
  } else {
    emitCmpTVType(v, KindOfRefCountThreshold, srcLoc.reg(1));
    ifThen(v, CC_NLE, [&](Vout& v) { increfMaybeStatic(v); });
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
  auto& v = vmain();

  v << testbi{0x1, src};
  ifThen(v, CC_Z, [&](Vout& v) {
    emitIncRef(v, src);
  });
}

void CodeGenerator::cgDecRefStack(IRInstruction* inst) {
  cgDecRefMem(inst->typeParam(),
              srcLoc(0).reg(),
              cellsToBytes(inst->extra<DecRefStack>()->offset));
}

void CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  auto& v = vmain();
  auto rthis = v.makeReg(); // Load AR->m_this into rthis
  v << loadq{fpReg[AROFF(m_this)], rthis};

  auto decrefIfAvailable = [&](Vout& v) {
    // Check if this is available and we're not in a static context instead
    v << testbi{1, rthis};
    ifThen(v, CC_Z, [&](Vout& v) {
      cgDecRefStaticType(v, Type::Obj, rthis, true /* genZeroCheck */);
    });
  };

  if (curFunc()->isPseudoMain()) {
    // In pseudo-mains, emit check for presence of m_this
    v << testq{rthis, rthis};
    ifThen(v, CC_NZ, [&](Vout& v) { decrefIfAvailable(v); });
  } else {
    decrefIfAvailable(v);
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
  auto& v = vmain();

  assert(rFp == rVmFp &&
         "free locals helper assumes the frame pointer is rVmFp");

  if (numLocals == 0) return;

  // The helpers called below use a special ABI, in which r14 and r15 is
  // not saved, and the stub expects the stack to be imbalanced (RSP%16==0)
  // on entry. So save r14 and r15 in addition to the caller-save registers,
  // and use PhysRegSaverStub which assumes the odd stack parity.
  auto toSave = m_state.liveRegs[inst] &
                (kCallerSaved | RegSet(r14) | RegSet(r15));
  PhysRegSaverStub saver(v, toSave);

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? mcg->tx().uniqueStubs.freeManyLocalsHelper
    : mcg->tx().uniqueStubs.freeLocalsHelpers[numLocals - 1];

  auto args = RegSet(r14) | RegSet(rVmFp);
  auto kills = (abi.all() - abi.calleeSaved) | RegSet(r14) | RegSet(r15);

  auto& marker = inst->marker();
  auto fix = Fixup{marker.bcOff()-marker.func()->base(), marker.spOff()};

  v << lea{rFp[-numLocals * sizeof(TypedValue)], r14};
  v << callstub{target, args, kills, fix};
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

  auto& v = vmain();
  if (profile->profiling()) {
    v << incwm{rVmTl[profile->handle() + offsetof(DecRefProfile, decrement)]};
  } else if (profile->optimizing()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.hitRate() != 0 && data.hitRate() != 100) {
      // These are the only interesting cases where we could be doing better.
      FTRACE(5, "DecRefProfile: {}: {} {}\n",
             data, m_curInst->marker().show(), profileKey->data());
    }
    if (data.hitRate() == 0) {
      v << incstat{Stats::TC_DecRef_Profiled_0};
    } else if (data.hitRate() == 100) {
      v << incstat{Stats::TC_DecRef_Profiled_100};
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
struct CheckValid<void(*)(Vout&)> {
  static bool valid(void (*f)(Vout&)) { return f != nullptr; }
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
void CodeGenerator::cgCheckStaticBitAndDecRef(Vout& v, Vlabel done, Type type,
                                              Vreg dataReg, F destroyImpl) {
  always_assert(type.maybeCounted());
  bool hasDestroy = CheckValid<F>::valid(destroyImpl);

  OptDecRefProfile profile;
  auto const unlikelyDestroy =
    hasDestroy ? decRefDestroyIsUnlikely(profile, type) : false;

  if (hasDestroy) {
    v << incstat{unlikelyDestroy ? Stats::TC_DecRef_Normal_Decl :
                 Stats::TC_DecRef_Likely_Decl};
  } else {
    v << incstat{Stats::TC_DecRef_NZ};
  }

  auto destroy = [&](Vout& v) {
    v << incstat{unlikelyDestroy ? Stats::TC_DecRef_Normal_Destroy :
                 Stats::TC_DecRef_Likely_Destroy};
    if (profile && profile->profiling()) {
      v << incwm{rVmTl[profile->handle() + offsetof(DecRefProfile, destroy)]};
    }
    destroyImpl(v);
  };

  if (!type.needsStaticBitCheck()) {
    v << declm{dataReg[FAST_REFCOUNT_OFFSET]};
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is not less than zero
      emitAssertFlagsNonNegative(v);
    }

    if (hasDestroy) {
      ifBlock(v, vcold(), CC_E, destroy, unlikelyDestroy);
    }
    return;
  }

  auto static_check_and_decl = [&](Vout& v) {
    static_assert(UncountedValue == UNCOUNTED, "");
    static_assert(StaticValue == STATIC, "");

    if (type.needsStaticBitCheck()) {
      auto next = v.makeBlock();
      v << jcc{CC_L, {next, done}};
      v = next;
    }

    // Decrement _count
    v << declm{dataReg[FAST_REFCOUNT_OFFSET]};
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      // Assert that the ref count is not less than zero
      emitAssertFlagsNonNegative(v);
    }
  };

  if (hasDestroy) {
    v << cmplim{1, dataReg[FAST_REFCOUNT_OFFSET]};
    ifThenElse(v, vcold(), CC_E, destroy, static_check_and_decl,
               unlikelyDestroy);
    return;
  }
  if (type.needsStaticBitCheck()) {
    v << cmplim{0, dataReg[FAST_REFCOUNT_OFFSET]};
  }

  static_check_and_decl(v);
}

void CodeGenerator::cgCheckStaticBitAndDecRef(Vout& v, Vlabel done,
                                              Type type, Vreg dataReg) {
  return cgCheckStaticBitAndDecRef(v, done, type, dataReg,
                                   (void (*)(Vout&))nullptr);
}

//
// Returns the address to be patched with the address to jump to in case
// the type is not ref-counted.
//
void CodeGenerator::cgCheckRefCountedType(Vreg typeReg, Vlabel done) {
  auto& v = vmain();
  auto next = v.makeBlock();
  emitCmpTVType(v, KindOfRefCountThreshold, typeReg);
  v << jcc{CC_LE, {next, done}};
  v = next;
}

void CodeGenerator::cgCheckRefCountedType(Vreg baseReg, int64_t offset,
                                          Vlabel done) {
  auto& v = vmain();
  auto next = v.makeBlock();
  emitCmpTVType(v, KindOfRefCountThreshold, baseReg[offset + TVOFF(m_type)]);
  v << jcc{CC_LE, {next, done}};
  v = next;
}

//
// Generates dec-ref of a typed value with statically known type.
//
void CodeGenerator::cgDecRefStaticType(Vout& v, Type type, Vreg dataReg,
                                       bool genZeroCheck) {
  assert(type != Type::Cell && type != Type::Gen);
  assert(type.isKnownDataType());

  if (type.notCounted()) return;

  // Check for UncountedValue or StaticValue if needed,
  // do the actual DecRef, and leave flags set based on the subtract result,
  // which is tested below
  auto done = v.makeBlock();
  if (genZeroCheck) {
    cgCheckStaticBitAndDecRef(v, done, type, dataReg, [&] (Vout& v) {
        // Emit the call to release in m_acold
        cgCallHelper(v,
                     mcg->getDtorCall(type.toDataType()),
                     kVoidDest,
                     SyncOptions::kSyncPoint,
                     argGroup()
                     .reg(dataReg));
      });
  } else {
    cgCheckStaticBitAndDecRef(v, done, type, dataReg);
  }
  if (!v.closed()) v << jmp{done};
  v = done;
}

//
// Generates dec-ref of a typed value with dynamic (statically unknown) type,
// when the type is stored in typeReg.
//
void CodeGenerator::cgDecRefDynamicType(Vreg typeReg, Vreg dataReg,
                                        bool genZeroCheck) {
  // Emit check for ref-counted type
  auto& v = vmain();
  auto done = v.makeBlock();
  cgCheckRefCountedType(typeReg, done);

  // Emit check for UncountedValue or StaticValue and the actual DecRef
  if (genZeroCheck) {
      cgCheckStaticBitAndDecRef(v, done, Type::Cell, dataReg, [&] (Vout& v) {
          // Emit call to release in m_acold
          cgCallHelper(v, CppCall::destruct(argNumToRegName[1]),
                       kVoidDest,
                       SyncOptions::kSyncPoint,
                       argGroup()
                       .reg(dataReg)
                       .reg(typeReg));
        });
  } else {
    cgCheckStaticBitAndDecRef(v, done, Type::Cell, dataReg);
  }
  if (!v.closed()) v << jmp{done};
  v = done;
}

//
// Generates dec-ref of a typed value with dynamic (statically
// unknown) type, when all we have is the baseReg and offset of
// the typed value. This method assumes that baseReg is not the
// scratch register.
//
void CodeGenerator::cgDecRefDynamicTypeMem(Vreg baseReg, int64_t offset) {
  auto& v = vmain();
  auto dataReg = v.makeReg();
  auto done = v.makeBlock();

  // Emit check for ref-counted type
  cgCheckRefCountedType(baseReg, offset, done);

  v << loadq{baseReg[offset + TVOFF(m_data)], dataReg};

  // Emit check for UncountedValue or StaticValue and the actual DecRef
  cgCheckStaticBitAndDecRef(v, done, Type::Cell, dataReg, [&](Vout& v) {
    // Emit call to release in stubsCode
    auto tvPtr = v.makeReg();
    v << lea{baseReg[offset], tvPtr};
    cgCallHelper(v, CppCall::direct(tv_release_generic),
                 kVoidDest,
                 SyncOptions::kSyncPoint,
                 argGroup().reg(tvPtr));
  });

  if (!v.closed()) v << jmp{done};
  v = done;
}

//
// Generates the dec-ref of a typed value in memory address [baseReg + offset].
// This handles cases where type is either static or dynamic.
//
void CodeGenerator::cgDecRefMem(Type type, Vreg baseReg, int64_t offset) {
  if (type.notCounted()) return;
  auto& v = vmain();
  if (type.needsReg()) {
    // The type is dynamic, but we don't have two registers available
    // to load the type and the data.
    cgDecRefDynamicTypeMem(baseReg, offset);
  } else if (type.maybeCounted()) {
    auto dataReg = v.makeReg();
    v << loadq{baseReg[offset + TVOFF(m_data)], dataReg};
    cgDecRefStaticType(v, type, dataReg, true);
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
    cgDecRefStaticType(vmain(), type, srcLoc(0).reg(), genZeroCheck);
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
  auto& v = vmain();

  auto func = v.makeReg();
  v << loadq{fpReg[itOff + CufIter::funcOff()], func};
  v << storeq{func, spReg[spOffset + int(AROFF(m_func))]};

  auto ctx = v.makeReg();
  v << loadq{fpReg[itOff + CufIter::ctxOff()], ctx};
  v << storeq{ctx, spReg[spOffset + int(AROFF(m_this))]};

  auto ctx2 = v.makeReg();
  v << shrqi{1, ctx, ctx2};
  ifThen(v, CC_NBE, [&](Vout& v) {
    auto ctx3 = v.makeReg();
    v << shlqi{1, ctx2, ctx3};
    emitIncRef(v, ctx3);
  });
  auto name = v.makeReg();
  v << loadq{fpReg[itOff + CufIter::nameOff()], name};
  v << testq{name, name};
  ifThenElse(v, CC_NZ, [&](Vout& v) {
    v << cmplim{0, name[FAST_REFCOUNT_OFFSET]};
    static_assert(UncountedValue < 0 && StaticValue < 0, "");
    ifThen(v, CC_NS, [&](Vout& v) { emitIncRef(v, name); });
    auto name2 = v.makeReg();
    v << orqi{ActRec::kInvNameBit, name, name2};
    v << storeq{name2, spReg[spOffset + int(AROFF(m_invName))]};
  }, [&](Vout& v) {
    v << storeq{name, spReg[spOffset + int(AROFF(m_invName))]};
  });
  v << storelim{safe_cast<int32_t>(nArgs),
                spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
  emitAdjustSp(spReg, dstLoc(0).reg(), spOffset);
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func      = inst->src(1);
  auto const objOrCls  = inst->src(2);
  auto const magicName = inst->extra<SpillFrame>()->invName;
  auto const nArgs     = inst->extra<SpillFrame>()->numArgs;
  auto& v              = vmain();

  const auto spOffset = -safe_cast<int32_t>(kNumActRecCells * sizeof(Cell));

  auto spReg = srcLoc(0).reg();
  // actRec->m_this
  if (objOrCls->isA(Type::Cls)) {
    // store class
    if (objOrCls->isConst()) {
      emitImmStoreq(v, uintptr_t(objOrCls->clsVal()) | 1,
                    spReg[spOffset + int(AROFF(m_this))]);
    } else {
      auto clsPtrReg = srcLoc(2/*objOrCls*/).reg();
      auto thisptr = v.makeReg();
      v << orqi{1, clsPtrReg, thisptr};
      v << storeq{thisptr, spReg[spOffset + int(AROFF(m_this))]};
    }
  } else if (objOrCls->isA(Type::Obj)) {
    // store this pointer
    v << storeq{srcLoc(2/*objOrCls*/).reg(),
                spReg[spOffset + int(AROFF(m_this))]};
  } else if (objOrCls->isA(Type::Ctx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    auto objOrClsPtrReg = srcLoc(2/*objOrCls*/).reg();
    v << storeq{objOrClsPtrReg, spReg[spOffset + int(AROFF(m_this))]};
  } else {
    assert(objOrCls->isA(Type::Nullptr));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + int(AROFF(m_this));
    v << storeqim{0, spReg[offset_m_this]};
  }
  // actRec->m_invName
  // ActRec::m_invName is encoded as a pointer with bit kInvNameBit
  // set to distinguish it from m_varEnv and m_extrArgs
  uintptr_t invName = !magicName
    ? 0
    : reinterpret_cast<uintptr_t>(magicName) | ActRec::kInvNameBit;
  emitImmStoreq(v, invName, spReg[spOffset + int(AROFF(m_invName))]);
  // actRec->m_func  and possibly actRec->m_cls
  // Note m_cls is unioned with m_this and may overwrite previous value
  if (func->isA(Type::Nullptr)) {
    // No need to store the null---we're always about to run another
    // instruction that will populate the Func.
  } else if (func->isConst()) {
    const Func* f = func->funcVal();
    emitImmStoreq(v, intptr_t(f), spReg[spOffset + int(AROFF(m_func))]);
  } else {
    int offset_m_func = spOffset + int(AROFF(m_func));
    auto funcLoc = srcLoc(1);
    v << storeq{funcLoc.reg(0), spReg[offset_m_func]};
  }

  v << storelim{nArgs, spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
  emitAdjustSp(spReg, dstLoc(0).reg(), spOffset);
}

void CodeGenerator::cgStClosureFunc(IRInstruction* inst) {
  auto const obj  = srcLoc(0).reg();
  auto const func = inst->extra<StClosureFunc>()->func;
  emitImmStoreq(vmain(), intptr_t(func), obj[c_Closure::funcOffset()]);
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
  auto& v = vmain();
  if (inst->src(1)->isA(Type::Nullptr)) {
    v << storeqim{0, obj[c_Closure::ctxOffset()]};
  } else {
    auto const ctx = srcLoc(1).reg();
    always_assert(ctx != InvalidReg);
    v << storeq{ctx, obj[c_Closure::ctxOffset()]};
  }
}

void CodeGenerator::emitInitObjProps(Vreg dstReg, const Class* cls,
                                     size_t nProps) {
  // If the object has a small number of properties, just emit stores
  // inline.
  auto& v = vmain();
  if (nProps < 8) {
    for (int i = 0; i < nProps; ++i) {
      auto propOffset =
        sizeof(ObjectData) + cls->builtinODTailSize() + sizeof(TypedValue) * i;
      auto propDataOffset = propOffset + TVOFF(m_data);
      auto propTypeOffset = propOffset + TVOFF(m_type);
      if (!IS_NULL_TYPE(cls->declPropInit()[i].m_type)) {
        emitImmStoreq(v, cls->declPropInit()[i].m_data.num,
                      dstReg[propDataOffset]);
      }
      v << storebim{cls->declPropInit()[i].m_type, dstReg[propTypeOffset]};
    }
    return;
  }

  // Use memcpy for large numbers of properties.
  auto args = argGroup()
    .addr(dstReg,
          safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
    .imm(int64_t(&cls->declPropInit()[0]))
    .imm(cellsToBytes(nProps));
  cgCallHelper(v,
               CppCall::direct(memcpy),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               args);
}

void CodeGenerator::cgConstructInstance(IRInstruction* inst) {
  auto const cls    = inst->extra<ConstructInstance>()->cls;
  auto const dstReg = dstLoc(0).reg();
  cgCallHelper(vmain(),
               CppCall::direct(cls->instanceCtor()),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
               argGroup().immPtr(cls));
}

void CodeGenerator::cgCheckInitProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitProps>()->cls;
  auto const branch = inst->taken();
  auto& v = vmain();
  v << cmpqim{0, rVmTl[cls->propHandle()]};
  v << jcc{CC_Z, {label(inst->next()), label(branch)}};
}

void CodeGenerator::cgCheckInitSProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitSProps>()->cls;
  auto const branch = inst->taken();
  auto& v = vmain();
  v << cmpbim{0, rVmTl[cls->sPropInitHandle()]};
  v << jcc{CC_Z, {label(inst->next()), label(branch)}};
}

void CodeGenerator::cgNewInstanceRaw(IRInstruction* inst) {
  auto const cls    = inst->extra<NewInstanceRaw>()->cls;
  auto const dstReg = dstLoc(0).reg();
  size_t size = ObjectData::sizeForNProps(cls->numDeclProperties());
  cgCallHelper(vmain(),
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
  auto& v = vmain();

  // Set the attributes, if any
  int odAttrs = cls->getODAttrs();
  if (odAttrs) {
    // o_attribute is 16 bits but the fact that we're or-ing a mask makes it ok
    assert(!(odAttrs & 0xffff0000));
    v << orqim{odAttrs, srcReg[ObjectData::attributeOff()]};
  }

  // Initialize the properties
  size_t nProps = cls->numDeclProperties();
  if (nProps > 0) {
    if (cls->pinitVec().size() == 0) {
      // Fast case: copy from a known address in the Class
      emitInitObjProps(srcReg, cls, nProps);
    } else {
      // Slower case: we have to load the src address from the targetcache
      auto propInitVec = v.makeReg();
      // Load the Class's propInitVec from the targetcache
      v << loadq{rVmTl[cls->propHandle()], propInitVec};
      // We want &(*propData)[0]
      auto rPropData = v.makeReg();
      v << loadq{propInitVec[Class::PropInitVec::dataOff()], rPropData};
      if (!cls->hasDeepInitProps()) {
        auto args = argGroup()
          .addr(srcReg,
              safe_cast<int32_t>(sizeof(ObjectData) + cls->builtinODTailSize()))
          .reg(rPropData)
          .imm(cellsToBytes(nProps));
        cgCallHelper(v,
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
        cgCallHelper(v,
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
    vmain(),
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
  auto& v = vmain();

  auto const ar = extra->numParams * sizeof(TypedValue);
  v << storeq{rFP, rSP[ar + AROFF(m_sfp)]};
  v << storelim{safe_cast<int32_t>(extra->after), rSP[ar + AROFF(m_soff)]};

  if (extra->knownPrologue) {
    assert(extra->callee);
    if (RuntimeOption::EvalHHIRGenerateAsserts) {
      auto const off = cellsToBytes(extra->numParams) + AROFF(m_savedRip);
      emitImmStoreq(v, 0xff00ff00b00b00d0, rSP[off]);
    }
    v << lea{rSP[cellsToBytes(extra->numParams)], rStashedAR};
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
      v << kpcall{extra->knownPrologue, extra->callee, prologIndex};
    } else {
      v << call{extra->knownPrologue};
    }
    return;
  }

  assert(dstLoc(0).reg() == rVmSp);
  auto const srcKey = m_curInst->marker().sk();
  v << bindcall{srcKey, extra->callee, extra->numParams};
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  uint32_t offset = inst->extra<CastStk>()->offset;
  auto spReg      = srcLoc(0).reg();
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
  cgCallHelper(vmain(),
               CppCall::direct(reinterpret_cast<void (*)()>(tvCastHelper)),
               kVoidDest,
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgCastStkIntToDbl(IRInstruction* inst) {
  auto spReg = srcLoc(0).reg();
  auto offset = cellsToBytes(inst->extra<CastStkIntToDbl>()->offset);
  auto& v = vmain();
  auto tmp_dbl = v.makeReg();
  v << cvtsi2sdm{refTVData(spReg[offset]), tmp_dbl};
  v << store{tmp_dbl, refTVData(spReg[offset])};
  emitStoreTVType(v, KindOfDouble, refTVType(spReg[offset]));
}

void CodeGenerator::cgCoerceStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  uint32_t offset = inst->extra<CoerceStk>()->offset;
  Block* exit     = inst->taken();
  auto spReg      = srcLoc(0).reg();

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

  auto& v = vmain();
  auto tmpReg = v.makeReg(); // XXX maybe force to rax?
  cgCallHelper(v,
    CppCall::direct(reinterpret_cast<void (*)()>(tvCoerceHelper)),
    callDest(tmpReg),
    SyncOptions::kSyncPoint,
    args
  );
  v << testbi{1, tmpReg};
  v << jcc{CC_E, {label(inst->next()), label(exit)}};
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const dst            = dstLoc(0);
  auto const dstReg         = dst.reg(0);
  auto const dstType        = dst.reg(1);
  auto const callee         = inst->extra<CallBuiltin>()->callee;
  auto const numArgs        = callee->numParams();
  auto const returnType     = inst->typeParam();
  auto const funcReturnType = callee->returnType();
  auto& v = vmain();

  int returnOffset = MISOFF(tvBuiltinReturn);

  if (FixupMap::eagerRecord(callee)) {
    auto const pc = curUnit()->entry() + m_curInst->marker().bcOff();
    // we have spilled all args to stack, so spDiff is 0
    emitEagerSyncPoint(v, reinterpret_cast<const Op*>(pc));
  }
  // RSP points to the MInstrState we need to use.  Workaround the
  // fact that rsp moves when we spill registers around call
  auto misReg = v.makeReg();
  v << copy{reg::rsp, misReg};

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
  cgCallHelper(v, CppCall::direct(callee->nativeFuncPtr()),
               dest, SyncOptions::kSyncPoint, callArgs);

  // For primitive return types (int, bool, double), the return value
  // is already in dstReg (the builtin call returns in rax or xmm0).
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
    auto rtype = v.makeReg();
    auto nulltype = v.makeReg();
    v << loadq{misReg[returnOffset], dstReg};
    v << ldimm{returnType.toDataType(), rtype};
    v << ldimm{KindOfNull, nulltype};
    v << testq{dstReg, dstReg};
    v << cmovq{CC_Z, rtype, nulltype, dstType};
    return;
  }
  if (returnType <= Type::Cell || returnType <= Type::BoxedCell) {
    // return type is Variant; fold KindOfUninit to KindOfNull
    assert(isCppByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    assert(misReg != Vreg{dstType});
    auto tmp_type = v.makeReg();
    auto nulltype = v.makeReg();
    emitLoadTVType(v, misReg[returnOffset + TVOFF(m_type)], tmp_type);
    v << loadq{misReg[returnOffset + TVOFF(m_data)], dstReg};
    v << ldimm{KindOfNull, nulltype};
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    v << testb{tmp_type, tmp_type};
    v << cmovq{CC_Z, tmp_type, nulltype, dstType};
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

void CodeGenerator::emitAdjustSp(Vreg spReg, Vreg dstReg,
                                 int adjustment /* bytes */) {
  auto& v = vmain();
  if (adjustment != 0) {
    if (dstReg != spReg) {
      v << lea{spReg[adjustment], dstReg};
    } else {
      v << addqi{adjustment, dstReg, dstReg};
    }
  } else {
    v << copy{spReg, dstReg};
  }
}

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  auto const func = curFunc();
  auto const builtinFuncPtr = func->builtinFuncPtr();
  auto& v = vmain();

  v << copy{srcLoc(0).reg(), argNumToRegName[0]};
  if (FixupMap::eagerRecord(func)) {
    emitEagerSyncPoint(v, reinterpret_cast<const Op*>(func->getEntry()));
  }
  v << call{(TCA)builtinFuncPtr, argSet(1)};
  recordSyncPoint(v);
}

void CodeGenerator::cgLdThis(IRInstruction* inst) {
  Block* taken  = inst->taken();
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();

  v << loadq{srcLoc(0).reg()[AROFF(m_this)], dstReg};
  if (!taken) return;  // no need to perform its checks

  if (curFunc()->isPseudoMain() || !curFunc()->mayHaveThis()) {
    // Check for a null $this pointer first.
    v << testq{dstReg, dstReg};
    emitFwdJcc(v, CC_Z, taken);
  }

  v << testbi{1, dstReg};
  v << jcc{CC_NZ, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  // Context could be either a this object or a class ptr
  auto& v = vmain();
  v << testbi{1, srcReg};
  ifThenElse(v, CC_NZ,
    [&](Vout& v) { emitLdClsCctx(v, srcReg, dstReg);  }, // ctx is a class
    [&](Vout& v) { emitLdObjClass(v, srcReg, dstReg); }  // ctx is this ptr
  );
}

void CodeGenerator::cgLdClsCctx(IRInstruction* inst) {
  auto srcReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  emitLdClsCctx(vmain(), srcReg, dstReg);
}

void CodeGenerator::cgLdCtx(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const srcReg = srcLoc(0).reg();
  vmain() << loadq{srcReg[AROFF(m_this)], dstReg};
}

void CodeGenerator::cgLdCctx(IRInstruction* inst) {
  return cgLdCtx(inst);
}

void CodeGenerator::cgLdClsName(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const srcReg = srcLoc(0).reg();
  auto& v = vmain();

  v << loadq{srcReg[Class::preClassOff()], dstReg};
  emitLdLowPtr(v, dstReg[PreClass::nameOffset()],
               dstReg, sizeof(LowStringPtr));
}

void CodeGenerator::cgLdARFuncPtr(IRInstruction* inst) {
  assert(inst->src(1)->isConst());
  auto const offset = inst->src(1);
  auto dstReg       = dstLoc(0).reg();
  auto baseReg      = srcLoc(0).reg();
  vmain() << loadq{baseReg[offset->intVal() + AROFF(m_func)], dstReg};
}

void CodeGenerator::cgLdStaticLocCached(IRInstruction* inst) {
  auto const extra = inst->extra<LdStaticLocCached>();
  auto const link  = RDS::bindStaticLocal(extra->func, extra->name);
  auto const dst   = dstLoc(0).reg();
  vmain() << lea{rVmTl[link.handle()], dst};
}

void CodeGenerator::cgCheckStaticLocInit(IRInstruction* inst) {
  auto const src = srcLoc(0).reg();
  auto& v = vmain();
  emitCmpTVType(v, KindOfUninit, src[RefData::tvOffset() + TVOFF(m_type)]);
  v << jcc{CC_E, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgStaticLocInitCached(IRInstruction* inst) {
  auto const rdSrc = srcLoc(0).reg();
  auto& v = vmain();

  // If we're here, the target-cache-local RefData is all zeros, so we
  // can initialize it by storing the new value into it's TypedValue
  // and incrementing the RefData reference count (which will set it
  // to 1).
  //
  // We are storing the rdSrc value into the static, but we don't need
  // to inc ref it because it's a bytecode invariant that it's not a
  // reference counted type.
  cgStore(rdSrc[RefData::tvOffset()], inst->src(1), srcLoc(1), Width::Full);
  v << inclm{rdSrc[FAST_REFCOUNT_OFFSET]};
  if (debug) {
    static_assert(sizeof(RefData::Magic::kMagic) == sizeof(uint64_t), "");
    emitImmStoreq(v, static_cast<int64_t>(RefData::Magic::kMagic),
                  rdSrc[RefData::magicOffset()]);
  }
}

void CodeGenerator::cgStoreTypedValue(Vptr dst, SSATmp* src, Vloc loc) {
  assert(src->type().needsReg());
  auto srcReg0 = loc.reg(0);
  auto srcReg1 = loc.reg(1);
  auto& v = vmain();
  if (srcReg0.isSIMD()) {
    // Whole typed value is stored in single SIMD reg srcReg0
    assert(RuntimeOption::EvalHHIRAllocSIMDRegs);
    assert(!srcReg1.isValid());
    v << storedqu{srcReg0, refTVData(dst)};
    return;
  }

  if (src->type().needsValueReg()) {
    assert(srcReg0.isValid());
    v << storeq{srcReg0, refTVData(dst)};
  }

  assert(srcReg1.isValid());
  emitStoreTVType(v, srcReg1, refTVType(dst));
}

void CodeGenerator::cgStore(Vptr dst, SSATmp* src, Vloc srcLoc, Width width) {
  Type type = src->type();
  if (type.needsReg()) {
    always_assert(width == Width::Full);
    cgStoreTypedValue(dst, src, srcLoc);
    return;
  }
  auto& v = vmain();
  if (width == Width::Full) {
    emitStoreTVType(v, type.toDataType(), refTVType(dst));
  }
  if (!src->type().needsValueReg()) return; // no value to store

  auto memRef = refTVData(dst);
  auto srcReg = srcLoc.reg();
  if (!srcReg.isValid()) {
    always_assert(type <= (Type::Bool | Type::Int | Type::Dbl |
                  Type::Arr | Type::StaticStr | Type::Cls));
    emitImmStoreq(v, src->rawVal(), memRef);
  } else {
    auto s2 = zeroExtendIfBool(v, src, srcReg);
    v << store{s2, memRef};
  }
}

void CodeGenerator::cgLoad(SSATmp* dst, Vloc dstLoc, Vptr base, Block* label) {
  Type type = dst->type();
  if (type.needsReg()) {
    return cgLoadTypedValue(dst, dstLoc, base, label);
  }
  if (label) {
    emitTypeCheck(type, refTVType(base), refTVData(base), label);
  }
  auto dstReg = dstLoc.reg();
  // if dstReg == InvalidReg then there's nothing to load.
  if (!dstReg.isValid()) return;
  if (type <= Type::Bool) {
    vmain() << loadl{refTVData(base), dstReg};
  } else {
    vmain() << load{refTVData(base), dstReg};
  }
}

Vptr resolveRegCollision(Vout& v, Vreg dst, Vptr memRef) {
  assert(memRef.scale == 1);
  Vreg base = memRef.base;
  Vreg index = memRef.index;
  if (!index.isValid()) {
    if (base == dst) {
      // use a scratch register instead
      auto tmp = v.makeReg();
      v << copy{base, tmp};
      return tmp[memRef.disp];
    }
    return memRef;
  }
  if (base == dst) {
    // use a scratch register instead
    auto tmp = v.makeReg();
    v << copy{base, tmp};
    return tmp[index + memRef.disp];
  }
  if (index == dst) {
    // use a scratch register instead
    auto tmp = v.makeReg();
    v << copy{index, tmp};
    return tmp[base + memRef.disp];
  }
  return memRef;
}

// If label is not null and type is not Gen, this method generates a check
// that bails to the label if the loaded typed value doesn't match dst type.
void CodeGenerator::cgLoadTypedValue(SSATmp* dst, Vloc dstLoc,
                                     Vptr origRef, Block* label) {
  Type type = dst->type();
  auto valueDstReg = dstLoc.reg(0);
  auto typeDstReg  = dstLoc.reg(1);
  auto& v = vmain();
  if (valueDstReg.isSIMD()) {
    assert(!label);
    // Whole typed value is stored in single SIMD reg valueDstReg
    assert(RuntimeOption::EvalHHIRAllocSIMDRegs);
    assert(!typeDstReg.isValid());
    v << loaddqu{refTVData(origRef), valueDstReg};
    return;
  }

  if (!valueDstReg.isValid() && !typeDstReg.isValid() &&
      (!label || type == Type::Gen)) {
    // a dead load
    return;
  }

  auto ref = !typeDstReg.isValid() ? origRef :
             resolveRegCollision(v, typeDstReg, origRef);
  // Load type if it's not dead
  if (typeDstReg.isValid()) {
    emitLoadTVType(v, refTVType(ref), typeDstReg);
    if (label) {
      emitTypeCheck(type, typeDstReg, valueDstReg, label);
    }
  } else if (label) {
    emitTypeCheck(type, refTVType(ref), refTVData(ref), label);
  }

  // Load value if it's not dead
  if (!valueDstReg.isValid()) return;
  v << loadq{refTVData(ref), valueDstReg};
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
  auto& v = vmain();
  if (idxReg == InvalidReg) {
    v << cmplim{safe_cast<int32_t>(inst->src(1)->intVal()),
                strReg[StringData::sizeOff()]};
  } else {
    v << cmplm{idxReg, strReg[StringData::sizeOff()]};
  }
  v << setcc{CC_NBE, dstReg};
}

void CodeGenerator::cgProfileArray(IRInstruction* inst) {
  auto baseReg = srcLoc(0).reg();
  auto handle  = inst->extra<ProfileArray>()->handle;
  auto& v = vmain();

  // If kPackedKind changes to a value that is not 0, change
  // this to a conditional add.
  static_assert(ArrayData::ArrayKind::kPackedKind == 0, "kPackedKind changed");
  auto tmp_kind = v.makeReg();
  v << loadzbl{baseReg[ArrayData::offsetofKind()], tmp_kind};
  v << addlm{tmp_kind, rVmTl[handle + offsetof(NonPackedArrayProfile, count)]};
}

void CodeGenerator::cgCheckPackedArrayBounds(IRInstruction* inst) {
  static_assert(ArrayData::sizeofSize() == 4, "");
  // We may check packed array bounds on profiled arrays for which
  // we do not statically know that they are of kPackedKind.
  assert(inst->taken());
  auto arrReg = srcLoc(0).reg();
  auto idxReg = srcLoc(1).reg();
  auto& v = vmain();
  if (idxReg == InvalidReg) {
    v << cmplim{safe_cast<int32_t>(inst->src(1)->intVal()),
                arrReg[ArrayData::offsetofSize()]};
  } else {
    // ArrayData::m_size is a uint32_t but we need to do a 64-bit comparison
    // since idx is KindOfInt64.
    auto tmp_size = v.makeReg();
    v << loadl{arrReg[ArrayData::offsetofSize()], tmp_size};
    v << cmpq{idxReg, tmp_size};
  }
  v << jcc{CC_BE, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgLdPackedArrayElem(IRInstruction* inst) {
  if (inst->src(0)->isConst()) {
    // This would require two scratch registers and should be very
    // rare. t3626251
    CG_PUNT(LdPackedArrayElem-ConstArray);
  }

  auto const rArr = srcLoc(0).reg();
  auto const rIdx = srcLoc(1).reg();
  auto& v = vmain();

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
    Vreg base = rArr;
    auto idx = v.makeReg();
    v << shlqi{0x4, rIdx, idx}; // multiply by 16
    cgLoad(inst->dst(), dstLoc(0), base[idx + sizeof(ArrayData)]);
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
  auto& v = vmain();

  if (rIdx != InvalidReg) {
    static_assert(sizeof(TypedValue) == 16, "");
    auto idx = v.makeReg();
    Vreg base = rArr;
    v << shlqi{0x4, rIdx, idx};
    emitCmpTVType(v, KindOfNull, base[idx + sizeof(ArrayData) + TVOFF(m_type)]);
  } else {
    auto const idx    = inst->src(1)->intVal();
    auto const offset = sizeof(ArrayData) + idx * sizeof(TypedValue);
    emitCmpTVType(v, KindOfNull, rArr[offset + TVOFF(m_type)]);
  }

  auto const dst = dstLoc(0).reg();
  v << setcc{CC_NE, dst};
  v << movzbl{dst, dst};
}

void CodeGenerator::cgCheckBounds(IRInstruction* inst) {
  auto idx = inst->src(0);
  auto idxReg = srcLoc(0).reg();
  auto size = inst->src(1);
  auto sizeReg = srcLoc(1).reg();
  // caller made the check if both sources are constant and never
  // generate this opcode
  assert(!(idx->isConst() && size->isConst()));
  auto throwHelper = [&](Vout& v) {
      auto args = argGroup();
      args.ssa(0/*idx*/);
      cgCallHelper(v, CppCall::direct(throwOOB),
                   kVoidDest, SyncOptions::kSyncPoint, args);

    };

  if (idxReg == InvalidReg) {
    assert(idx->intVal() >= 0); // we would have punted otherwise
    auto idxVal = safe_cast<int32_t>(idx->intVal());
    vmain() << cmpqi{idxVal, sizeReg};
    unlikelyIfBlock(vmain(), vcold(), CC_LE, throwHelper);
    return;
  }

  if (sizeReg == InvalidReg) {
    assert(size->intVal() >= 0);
    auto sizeVal = safe_cast<int32_t>(size->intVal());
    vmain() << cmpqi{sizeVal, idxReg};
  } else {
    vmain() << cmpq{sizeReg, idxReg};
  }
  unlikelyIfBlock(vmain(), vcold(), CC_AE, throwHelper);
}

void CodeGenerator::cgLdVectorSize(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());
  vmain() << loadl{vecReg[c_Vector::sizeOffset()], dstReg};
}

void CodeGenerator::cgLdVectorBase(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());
  vmain() << loadq{vecReg[c_Vector::dataOffset()], dstReg};
}

/**
 * Given a vector, check if it has a immutable copy and jump to the taken
 * branch if so.
 */
void CodeGenerator::cgVectorHasImmCopy(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(0).reg();
  auto& v = vmain();

  assert(vec->type().strictSubtypeOf(Type::Obj) &&
         vec->type().getClass() == c_Vector::classof());

  // Vector::m_data field holds an address of an ArrayData plus
  // sizeof(ArrayData) bytes. We need to check this ArrayData's
  // m_count field to see if we need to call Vector::triggerCow().
  auto rawPtrOffset = c_Vector::dataOffset() + kExpectedMPxOffset;
  auto countOffset = (int64_t)FAST_REFCOUNT_OFFSET - (int64_t)sizeof(ArrayData);

  auto ptr = v.makeReg();
  v << loadq{vecReg[rawPtrOffset], ptr};
  v << cmplim{1, ptr[countOffset]};
  v << jcc{CC_NE, {label(inst->next()), label(inst->taken())}};
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
  cgCallHelper(vmain(), CppCall::direct(triggerCow),
               kVoidDest, SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgLdPairBase(IRInstruction* inst) {
  DEBUG_ONLY auto pair = inst->src(0);
  auto pairReg = srcLoc(0).reg();
  assert(pair->type().strictSubtypeOf(Type::Obj) &&
         pair->type().getClass() == c_Pair::classof());
  vmain() << lea{pairReg[c_Pair::dataOffset()], dstLoc(0).reg()};
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

void CodeGenerator::recordSyncPoint(Vout& v,
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
  v << syncpoint{Fixup{pcOff, stackOff}};
}

void CodeGenerator::cgLdMIStateAddr(IRInstruction* inst) {
  auto base = srcLoc(0).reg();
  int64_t offset = inst->src(1)->intVal();
  vmain() << lea{base[offset], dstLoc(0).reg()};
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  cgLoad(inst->dst(), dstLoc(0),
         srcLoc(0).reg()[localOffset(inst->extra<LdLoc>()->locId)]);
}

void CodeGenerator::cgLdLocAddr(IRInstruction* inst) {
  auto const fpReg  = srcLoc(0).reg();
  auto const offset = localOffset(inst->extra<LdLocAddr>()->locId);
  if (dstLoc(0).hasReg()) {
    vmain() << lea{fpReg[offset], dstLoc(0).reg()};
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
  vmain() << lea{base[offset], dst};
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
      vmain() << bindexit{ccNegate(cc), sk};
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
  auto& v = vmain();
  emitCompare(v, inst);
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgExitJccInt(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  auto& v = vmain();
  emitCompareInt(v, inst);
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgDefMIStateBase(IRInstruction* inst) {
  assert(dstLoc(0).reg() == rsp);
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->src(0);
  auto const rData = srcLoc(0).reg(0);
  auto const rType = srcLoc(0).reg(1);
  assert(rData != InvalidReg);
  auto& v = vmain();
  auto doJcc = [&](ConditionCode cc) {
    emitFwdJcc(v, ccNegate(cc), inst->taken());
  };
  auto doMov = [&]() {
    auto const valDst = dstLoc(0).reg(0);
    auto const typeDst = dstLoc(0).reg(1);
    // TODO: #3626251: XLS: Let Uses say whether a constant is
    // allowed, and if not, assign a register.
    if (valDst != InvalidReg) {
      v << copy{rData, valDst};
    }
    if (typeDst != InvalidReg) {
      if (rType != InvalidReg) v << copy{rType, typeDst};
      else v << ldimm{src->type().toDataType(), typeDst};
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
    v << jmp{label(inst->taken())};
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
      v << cmplim{0, rData[FAST_REFCOUNT_OFFSET]};
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
        v << jmp{label(inst->taken())};
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
  auto const ch      = NamedEntity::get(clsName)->getClassHandle();
  auto clsImm = Immed64(cls);
  auto& v = vmain();
  if (clsImm.fits(sz::dword)) {
    v << cmpqim{clsImm.l(), rVmTl[ch]};
  } else {
    auto clsReg = v.makeReg();
    v << ldimm{cls, clsReg};
    v << cmpqm{clsReg, rVmTl[ch]};
  }
  v << jcc{CC_NZ, {label(inst->next()), label(inst->taken())}};
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
  auto& v = vmain();

  auto thenBody = [&](Vout& v) {
    auto bitsOff = sizeof(uint64_t) * (firstBitNum / 64);
    auto cond = CC_NE;
    auto bitsPtrReg = v.makeReg();
    if (firstBitNum == 0) {
      bitsOff = Func::refBitValOff();
      bitsPtrReg = funcPtrReg;
    } else {
      v << loadq{funcPtrReg[Func::sharedOff()], bitsPtrReg};
      bitsOff -= sizeof(uint64_t);
    }

    if (vals64 == 0 || (mask64 & (mask64 - 1)) == 0) {
      // If vals64 is zero, or we're testing a single
      // bit, we can get away with a single test,
      // rather than mask-and-compare
      if (mask64Reg != InvalidReg) {
        v << testqm{mask64Reg, bitsPtrReg[bitsOff]};
      } else {
        if (mask64 < 256) {
          v << testbim{(int8_t)mask64, bitsPtrReg[bitsOff]};
        } else {
          v << testlim{(int32_t)mask64, bitsPtrReg[bitsOff]};
        }
      }
      if (vals64) cond = CC_E;
    } else {
      auto bitsValReg = v.makeReg();
      v << loadq{bitsPtrReg[bitsOff], bitsValReg};

      //     bitsVal2 <- bitsValReg & mask64
      auto bitsVal2 = v.makeReg();
      if (mask64Reg != InvalidReg) {
        v << andq{mask64Reg, bitsValReg, bitsVal2};
      } else if (mask64 < 256) {
        v << andbi{(int8_t)mask64, bitsValReg, bitsVal2};
      } else {
        v << andli{(int32_t)mask64, bitsValReg, bitsVal2};
      }

      //   If bitsVal2 != vals64, then goto Exit
      if (vals64Reg != InvalidReg) {
        v << cmpq{vals64Reg, bitsVal2};
      } else if (mask64 < 256) {
        assert(vals64 < 256);
        v << cmpbi{(int8_t)vals64, bitsVal2};
      } else {
        v << cmpli{(int32_t)vals64, bitsVal2};
      }
    }
    v << fallbackcc{cond, destSK};
  };

  if (firstBitNum == 0) {
    assert(nParamsReg == InvalidReg);
    // This is the first 64 bits. No need to check
    // nParams.
    thenBody(v);
  } else {
    assert(nParamsReg != InvalidReg);
    // Check number of args...
    v << cmpqi{firstBitNum, nParamsReg};

    if (vals64 != 0 && vals64 != mask64) {
      // If we're beyond nParams, then either all params
      // are refs, or all params are non-refs, so if vals64
      // isn't 0 and isnt mask64, there's no possibility of
      // a match
      v << fallbackcc{CC_LE, destSK};
      thenBody(v);
    } else {
      ifThenElse(v, CC_NLE, thenBody, /* else */ [&](Vout& v) {
          //   If not special builtin...
          v << testlim{AttrVariadicByRef, funcPtrReg[Func::attrsOff()]};
          v << fallbackcc{vals64 ? CC_Z : CC_NZ, destSK};
        });
    }
  }
}

void CodeGenerator::cgLdPropAddr(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const objReg = srcLoc(0).reg();
  auto const prop = inst->src(1);
  auto& v = vmain();
  always_assert(objReg != InvalidReg);
  always_assert(dstReg != InvalidReg);
  v << lea{objReg[prop->intVal()], dstReg};
}

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto clsReg = srcLoc(0).reg();
  int32_t mSlotVal = inst->src(1)->rawVal();

  assert(dstReg != InvalidReg);

  auto methOff = int32_t(mSlotVal * sizeof(Func*));
  auto& v = vmain();
  v << loadq{clsReg[methOff], dstReg};
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
  cgCallHelper(vmain(),
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
    vmain() << loadq{rVmTl[ch + off], dstReg};
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
void CodeGenerator::emitGetCtxFwdCallWithThis(Vreg ctxReg, bool staticCallee) {
  auto& v = vmain();
  if (staticCallee) {
    // Load (this->m_cls | 0x1) into ctxReg.
    emitLdLowPtr(v, ctxReg[ObjectData::getVMClassOffset()],
                 ctxReg, sizeof(LowClassPtr));
    v << orqi{1, ctxReg, ctxReg};
  } else {
    // Just incref $this.
    emitIncRef(v, ctxReg);
  }
}

/**
 * This method is similar to emitGetCtxFwdCallWithThis above, but
 * whether or not the callee is a static method is unknown at JIT
 * time, and that is determined dynamically by looking up into the
 * StaticMethodFCache.
 */
void CodeGenerator::cgGetCtxFwdCall(IRInstruction* inst) {
  auto destCtxReg = dstLoc(0).reg(0);
  auto srcCtxTmp = inst->src(0);
  auto srcCtxReg = srcLoc(0).reg(0);
  const Func* callee = inst->src(1)->funcVal();
  bool      withThis = srcCtxTmp->isA(Type::Obj);

  // Eagerly move src into the dest reg
  auto& v = vmain();
  v << copy{srcCtxReg, destCtxReg};

  auto done = v.makeBlock();
  // If we don't know whether we have a This, we need to check dynamically
  if (!withThis) {
    v << testbi{1, destCtxReg};
    auto next = v.makeBlock();
    v << jcc{CC_NZ, {next, done}};
    v = next;
  }

  // If we have a This pointer in destCtxReg, then select either This
  // or its Class based on whether callee is static or not
  emitGetCtxFwdCallWithThis(destCtxReg, (callee->attrs() & AttrStatic));
  v << jmp{done};
  v = done;
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
    vmain() << loadq{rVmTl[ch], dstReg};
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
  cgCallHelper(vmain(),
               CppCall::direct(lookup),
               callDest(funcDestReg),
               SyncOptions::kSyncPoint,
               argGroup()
                 .imm(ch)
                 .immPtr(cls)
                 .immPtr(methName)
                 .reg(fpReg));
}

void CodeGenerator::emitGetCtxFwdCallWithThisDyn(Vreg destCtxReg, Vreg thisReg,
                                                 RDS::Handle ch) {
  auto& v = vmain();
  auto NonStaticCall = v.makeBlock();
  auto End = v.makeBlock();

  // thisReg is holding $this. Should we pass it to the callee?
  v << cmplim{1, rVmTl[ch + offsetof(StaticMethodFCache, m_static)]};
  auto next = v.makeBlock();
  v << jcc{CC_NE, {next, NonStaticCall}};
  v = next;

  // If calling a static method...
  // Load (this->m_cls | 0x1) into destCtxReg
  emitLdLowPtr(v, thisReg[ObjectData::getVMClassOffset()],
               destCtxReg, sizeof(LowClassPtr));
  v << orqi{1, destCtxReg, destCtxReg};
  v << jmp{End};

  // Else: calling non-static method
  v = NonStaticCall;
  v << copy{thisReg, destCtxReg};
  emitIncRef(v, destCtxReg);
  v << jmp{End};
  v = End;
}

void CodeGenerator::cgGetCtxFwdCallDyn(IRInstruction* inst) {
  auto srcCtxTmp  = inst->src(0);
  auto srcCtxReg  = srcLoc(0).reg();
  auto destCtxReg = dstLoc(0).reg();
  auto& v = vmain();
  v << copy{srcCtxReg, destCtxReg};
  auto const t = srcCtxTmp->type();
  if (t <= Type::Cctx) {
    // Nothing to do. Forward the context as is.
    return;
  }
  auto End = v.makeBlock();
  if (t <= Type::Obj) {
    // We definitely have $this, so always run code emitted by
    // emitGetCtxFwdCallWithThisDyn
  } else {
    assert(t <= Type::Ctx);
    // dynamically check if we have a This pointer and call
    // emitGetCtxFwdCallWithThisDyn below
    v << testbi{1, destCtxReg};
    auto next = v.makeBlock();
    v << jcc{CC_NZ, {next, End}};
    v = next;
  }

  // If we have a 'this' pointer ...
  auto const& extra = *inst->extra<ClsMethodData>();
  auto const ch = StaticMethodFCache::alloc(
    extra.clsName, extra.methodName, getContextName(curClass())
  );
  emitGetCtxFwdCallWithThisDyn(destCtxReg, destCtxReg, ch);
  v << jmp{End};
  v = End;
}

void CodeGenerator::cgLdClsPropAddrKnown(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();

  auto cls  = inst->src(0)->clsVal();
  auto name = inst->src(1)->strVal();

  auto ch = cls->sPropHandle(cls->lookupSProp(name));
  vmain() << lea{rVmTl[ch], dstReg};
}

RDS::Handle CodeGenerator::cgLdClsCachedCommon(IRInstruction* inst) {
  const StringData* className = inst->src(0)->strVal();
  auto ch = NamedEntity::get(className)->getClassHandle();
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  if (dstReg == InvalidReg) {
    v << cmpqim{0, rVmTl[ch]};
  } else {
    v << loadq{rVmTl[ch], dstReg};
    v << testq{dstReg, dstReg};
  }
  return ch;
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  auto ch = cgLdClsCachedCommon(inst);
  unlikelyIfBlock(vmain(), vcold(), CC_E, [&] (Vout& v) {
    Class* (*const func)(Class**, const StringData*) = jit::lookupKnownClass;
    cgCallHelper(v,
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
    vmain() << jcc{CC_Z, {label(inst->next()), label(taken)}};
  }
}

void CodeGenerator::cgDerefClsRDSHandle(IRInstruction* inst) {
  auto const dreg = dstLoc(0).reg();
  auto const ch   = inst->src(0);
  const Vreg rds = rVmTl;
  if (dreg == InvalidReg) return;
  auto& v = vmain();
  if (ch->isConst()) {
    v << loadq{rds[ch->rdsHandleVal()], dreg};
  } else {
    v << loadq{rds[srcLoc(0).reg()], dreg};
  }
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  auto const ch = ClassCache::alloc();
  RDS::recordRds(ch, sizeof(ClassCache),
                 "ClassCache", curFunc()->fullName()->data());
  cgCallHelper(vmain(),
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
  cgCallHelper(vmain(),
    CppCall::direct(jit::lookupClassConstantTv),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup()
      .addr(rVmTl, safe_cast<int32_t>(link.handle()))
      .immPtr(NamedEntity::get(extra->clsName))
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

  cgCallHelper(vmain(), CppCall::direct(lookupCnsHelper),
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

  cgCallHelper(vmain(), CppCall::direct(lookupCnsUHelper),
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
  auto& v = vmain();
  if (key->type() <= Type::Null) {
    if (arr->isA(Type::Arr)) {
      cgCallHelper(v, CppCall::direct(arr_str_helper),
                   callDest(inst),
                   SyncOptions::kNoSyncPoint,
                   argGroup().ssa(0/*arr*/).immPtr(staticEmptyString()));
    } else {
      v << ldimm{0, dstLoc(0).reg()};
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

  cgCallHelper(v, helper_func,
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup().ssa(0/*arr*/).ssa(1/*key*/));
}

void CodeGenerator::cgLdGblAddr(IRInstruction* inst) {
  auto dstReg = dstLoc(0).reg();
  auto& v = vmain();
  cgCallHelper(v,
               CppCall::direct(ldGblAddrHelper),
               callDest(dstReg),
               SyncOptions::kNoSyncPoint,
               argGroup().ssa(0));
  v << testq{dstReg, dstReg};
  v << jcc{CC_Z, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::emitTestZero(Vout& v, SSATmp* src, Vloc srcLoc) {
  auto reg = srcLoc.reg();
  assert(reg.isValid());
  if (src->isA(Type::Bool)) {
    v << testb{reg, reg};
  } else {
    v << testq{reg, reg};
  }
}

void CodeGenerator::cgJmpZero(IRInstruction* inst) {
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  v << jcc{CC_Z, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgJmpNZero(IRInstruction* inst) {
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  v << jcc{CC_NZ, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgReqBindJmpZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  emitReqBindJcc(v, CC_Z, inst->extra<ReqBindJmpZero>());
}

void CodeGenerator::cgReqBindJmpNZero(IRInstruction* inst) {
  // TODO(#2404427): prepareForTestAndSmash?
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  emitReqBindJcc(v, CC_NZ, inst->extra<ReqBindJmpNZero>());
}

void CodeGenerator::cgSideExitJmpZero(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgSideExitJmpNZero(IRInstruction* inst) {
  auto const extra = inst->extra<SideExitJccData>();
  auto const sk = SrcKey(curFunc(), extra->taken, resumed());
  auto& v = vmain();
  emitTestZero(v, inst->src(0), srcLoc(0));
  v << bindexit{opToConditionCode(inst->op()), sk, extra->trflags};
}

void CodeGenerator::cgJmp(IRInstruction* inst) {
  auto& v = vmain();
  v << jmp{label(inst->taken())};
}

void CodeGenerator::cgJmpIndirect(IRInstruction* inst) {
  auto& v = vmain();
  v << jmpr{srcLoc(0).reg()};
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* taken = inst->taken();
  assert(taken);
  SSATmp* src = inst->src(0);

  if (src->type().not(Type::Uninit)) return;

  auto typeReg = srcLoc(0).reg(1);
  assert(typeReg != InvalidReg);

  static_assert(KindOfUninit == 0, "cgCheckInit assumes KindOfUninit == 0");
  auto& v = vmain();
  v << testb{typeReg, typeReg};
  v << jcc{CC_Z, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgCheckInitMem(IRInstruction* inst) {
  Block* taken = inst->taken();
  assert(taken);
  SSATmp* base = inst->src(0);
  int64_t offset = inst->src(1)->intVal();
  Type t = base->type().deref();
  if (t.not(Type::Uninit)) return;
  auto basereg = srcLoc(0).reg();
  auto& v = vmain();
  emitCmpTVType(v, KindOfUninit, basereg[offset + TVOFF(m_type)]);
  v << jcc{CC_Z, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgCheckSurpriseFlags(IRInstruction* inst) {
  auto&v = vmain();
  emitTestSurpriseFlags(v);
  v << jcc{CC_NZ, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckCold(IRInstruction* inst) {
  Block*     taken = inst->taken();
  TransID  transId = inst->extra<CheckCold>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);
  auto& v = vmain();
  auto addr = v.makeReg();
  v << ldimm{counterAddr, addr};
  v << decqm{addr[0]};
  v << jcc{CC_LE, {label(inst->next()), label(taken)}};
}

static const StringData* s_ReleaseVV = makeStaticString("ReleaseVV");

void CodeGenerator::cgReleaseVVOrExit(IRInstruction* inst) {
  auto* const label = inst->taken();
  auto const rFp = srcLoc(0).reg();
  auto& v = vmain();

  TargetProfile<ReleaseVVProfile> profile(m_unit.context(), m_curInst->marker(),
                                          s_ReleaseVV);
  if (profile.profiling()) {
    v << incwm{rVmTl[profile.handle() + offsetof(ReleaseVVProfile, executed)]};
  }

  v << cmpqim{0, rFp[AROFF(m_varEnv)]};

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
  ifBlock(v, vcold(), CC_NZ, [&] (Vout& v) {
    if (profile.profiling()) {
      auto offsetof_release = offsetof(ReleaseVVProfile, released);
      v << incwm{rVmTl[profile.handle() + offsetof_release]};
    }
    v << testlim{ActRec::kExtraArgsBit, rFp[AROFF(m_varEnv)]};
    emitFwdJcc(v, CC_Z, label);
    cgCallHelper(
      v,
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
  auto& v = vmain();
  v << copy{base, dstReg};
  emitTypeTest(Type::BoxedCell, base[TVOFF(m_type)], base[TVOFF(m_data)],
    [&](ConditionCode cc) {
      ifThen(v, ccNegate(cc), [&](Vout& v) {
        cgCallHelper(v,
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

  if (inst->src(1)->isConst()) {
    PUNT(InterpOneCommon_const_fp);
  }
  cgCallHelper(vmain(),
               CppCall::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
               kVoidDest,
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
    vmain() << addqi{spAdjustBytes, newSpReg, newSpReg};
  }
}

void CodeGenerator::cgInterpOneCF(IRInstruction* inst) {
  cgInterpOneCommon(inst);
  auto& v = vmain();
  v << loadq{rVmTl[RDS::kVmfpOff], rVmFp};
  v << loadq{rVmTl[RDS::kVmspOff], rVmSp};
  v << resume{};
}

void CodeGenerator::cgContEnter(IRInstruction* inst) {
  // ContEnter does not directly use SP, but the generator body we are jumping
  // to obviously does. We depend on SP via srcLoc(0) to avoid last SpillStack
  // be optimized away.
  auto curFpReg  = srcLoc(1).reg();
  auto genFpReg  = srcLoc(2).reg();
  auto addrReg   = srcLoc(3).reg();
  auto returnOff = safe_cast<int32_t>(inst->src(4)->intVal());
  auto& v = vmain();
  assert(srcLoc(0).reg() == rVmSp);
  assert(curFpReg == rVmFp);

  v << storeq{curFpReg, genFpReg[AROFF(m_sfp)]};
  v << storelim{returnOff, genFpReg[AROFF(m_soff)]};
  v << movq{genFpReg, curFpReg};
  v << contenter{curFpReg, addrReg};
  // curFpReg->m_savedRip will point here, and the next HHIR opcode must
  // also start here.
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg      = srcLoc(0).reg();
  auto checkStarted = inst->src(1)->boolVal();
  auto stateOff     = BaseGenerator::stateOff();
  auto& v = vmain();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");
  static_assert(uint8_t(BaseGenerator::State::Started) == 1, "used below");

  // Take exit if state != 1 (checkStarted) or state > 1 (!checkStarted).
  v << cmpbim{1, contReg[stateOff]};
  emitFwdJcc(v, checkStarted ? CC_NE : CC_A, inst->taken());

  // Set generator state as Running.
  v << storebim{int8_t(BaseGenerator::State::Running), contReg[stateOff]};
}

void CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  auto contReg  = srcLoc(0).reg();
  auto stateOff = BaseGenerator::stateOff();
  auto& v = vmain();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");

  // Take exit if state == 0.
  v << testbim{int8_t(0xff), contReg[stateOff]};
  v << jcc{CC_Z, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgContValid(IRInstruction* inst) {
  auto contReg  = srcLoc(0).reg();
  auto dstReg   = dstLoc(0).reg();
  auto stateOff = BaseGenerator::stateOff();
  auto& v = vmain();

  // Return 1 if generator state is not Done.
  v << cmpbim{int8_t(BaseGenerator::State::Done), contReg[stateOff]};
  v << setcc{CC_NE, dstReg};
  v << movzbl{dstReg, dstReg};
}

void CodeGenerator::cgContArIncKey(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  vmain() << incqm{contArReg[CONTOFF(m_key) + TVOFF(m_data) -
                             c_Generator::arOff()]};
}

void CodeGenerator::cgContArUpdateIdx(IRInstruction* inst) {
  auto contArReg = srcLoc(0).reg();
  int64_t off = CONTOFF(m_index) - c_Generator::arOff();
  auto newIdx = inst->src(0);
  auto newIdxReg = srcLoc(1).reg();
  assert(contArReg != InvalidReg);
  assert(newIdxReg != InvalidReg);

  // this is hacky and awful oh god
  auto& v = vmain();
  auto res = v.makeReg();
  if (newIdx->isConst()) {
    v << ldimm{newIdx->rawVal(), res};
    v << cmpqm{res, contArReg[off]};
    v << cloadq{CC_G, contArReg[off], res};
  } else {
    auto mem_index = v.makeReg();
    v << loadq{contArReg[off], mem_index};
    v << cmpq{mem_index, newIdxReg};
    v << cmovq{CC_G, mem_index, newIdxReg, res};
  }
  v << storeq{res, contArReg[off]};
}

void CodeGenerator::cgLdContActRec(IRInstruction* inst) {
  auto dest = dstLoc(0).reg();
  auto base = srcLoc(0).reg();
  ptrdiff_t offset = BaseGenerator::arOff();
  vmain() << lea{base[offset], dest};
}

void CodeGenerator::emitLdRaw(IRInstruction* inst, size_t extraOff) {
  auto destReg = dstLoc(0).reg();
  auto offset  = inst->extra<RawMemData>()->info().offset;
  auto src     = srcLoc(0).reg()[offset + extraOff];
  auto& v = vmain();
  switch (inst->extra<RawMemData>()->info().size) {
    case sz::byte:  v << loadzbl{src, destReg}; break;
    case sz::dword:
      v << loadl{src, destReg};
      if (inst->extra<RawMemData>()->type == RawMemData::FuncNumParams) {
        // See Func::finishedEmittingParams and Func::numParams for rationale
        v << shrli{1, destReg, destReg};
      }
      break;
    case sz::qword: v << loadq{src, destReg}; break;
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

  auto& v = vmain();
  if (srcReg == InvalidReg) {
    auto val = Immed64(src->type().hasRawVal() ? src->rawVal() : 0);
    switch (size) {
      case sz::byte:  v << storebim{val.b(), dst}; break;
      case sz::dword: v << storelim{val.l(), dst}; break;
      case sz::qword: emitImmStoreq(v, val.q(), dst); break;
      default:        not_implemented();
    }
  } else {
    switch (size) {
      case sz::byte:  v << storeb{srcReg, dst}; break;
      case sz::dword: v << storel{srcReg, dst}; break;
      case sz::qword: v << storeq{srcReg, dst}; break;
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

void CodeGenerator::cgLdAsyncArParentChain(IRInstruction* inst) {
  auto asyncArReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  const int64_t off = c_AsyncFunctionWaitHandle::parentChainOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  auto& v = vmain();
  v << loadq{asyncArReg[off], dstReg};
}

void CodeGenerator::cgAFWHBlockOn(IRInstruction* inst) {
  auto parentArReg = srcLoc(0).reg();
  auto childReg = srcLoc(1).reg();
  auto& v = vmain();
  const int8_t blocked = c_WaitHandle::toKindState(
      c_WaitHandle::Kind::AsyncFunction, c_BlockableWaitHandle::STATE_BLOCKED);
  const int64_t firstParentOff = c_WaitableWaitHandle::parentChainOff()
                               + AsioBlockableChain::firstParentOff();
  const int64_t stateToArOff = c_AsyncFunctionWaitHandle::stateOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t nextParentToArOff = c_AsyncFunctionWaitHandle::blockableOff()
                                  + AsioBlockable::bitsOff()
                                  - c_AsyncFunctionWaitHandle::arOff();
  const int64_t childToArOff = c_AsyncFunctionWaitHandle::childOff()
                             - c_AsyncFunctionWaitHandle::arOff();
  const int64_t blockableToArOff = c_AsyncFunctionWaitHandle::blockableOff()
                                 - c_AsyncFunctionWaitHandle::arOff();

  // parent->setState(STATE_BLOCKED);
  v << storebim{blocked, parentArReg[stateToArOff]};

  // parent->m_blockable.m_bits = child->m_parentChain.m_firstParent|Kind::BWH;
  auto firstParent = v.makeReg();
  assert(uint8_t(AsioBlockable::Kind::BlockableWaitHandle) == 0);
  v << loadq{childReg[firstParentOff], firstParent};
  v << storeq{firstParent, parentArReg[nextParentToArOff]};

  // child->m_parentChain.m_firstParent = &parent->m_blockable;
  auto objToAr = v.makeReg();
  v << lea{parentArReg[blockableToArOff], objToAr};
  v << storeq{objToAr, childReg[firstParentOff]};

  // parent->m_child = child;
  v << storeq{childReg, parentArReg[childToArOff]};
}

void CodeGenerator::cgIsWaitHandle(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();

  static_assert(
    ObjectData::IsWaitHandle < 0xff,
    "we use byte instructions for IsWaitHandle"
  );
  auto& v = vmain();
  v << testbim{ObjectData::IsWaitHandle, robj[ObjectData::attributeOff()]};
  v << setcc{CC_NZ, rdst};
}

void CodeGenerator::cgLdWHState(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  auto const rdst = dstLoc(0).reg();
  auto& v = vmain();
  v << loadzbl{robj[ObjectData::whStateOffset()], rdst};
  v << andbi{0x0F, rdst, rdst};
}

void CodeGenerator::cgLdWHResult(IRInstruction* inst) {
  auto const robj = srcLoc(0).reg();
  cgLoad(inst->dst(), dstLoc(0), robj[c_WaitHandle::resultOff()]);
}

void CodeGenerator::cgLdAFWHActRec(IRInstruction* inst) {
  auto const dest = dstLoc(0).reg();
  auto const base = srcLoc(0).reg();
  auto& v = vmain();
  auto asyncArOffset = c_AsyncFunctionWaitHandle::arOff();
  v << lea{base[asyncArOffset], dest};
}

void CodeGenerator::cgLdResumableArObj(IRInstruction* inst) {
  auto const dstReg = dstLoc(0).reg();
  auto const resumableArReg = srcLoc(0).reg();
  auto& v = vmain();
  auto const objectOff = Resumable::objectOff() - Resumable::arOff();
  v << lea{resumableArReg[objectOff], dstReg};
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

  auto           fpReg = srcLoc(1).reg();
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
      vmain(),
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
    cgCallHelper(vmain(), CppCall::direct(new_iter_object), callDest(inst),
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
  auto             fpReg = srcLoc(1).reg();
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
    cgCallHelper(vmain(),
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
    cgCallHelper(vmain(),
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
  auto fpReg = srcLoc(0).reg();
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
  cgCallHelper(vmain(),
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
  auto fpReg = srcLoc(0).reg();
  auto args = argGroup();
  args.addr(fpReg, iterOffset(inst->extra<IterData>()->iterId))
      .addr(fpReg, localOffset(inst->extra<IterData>()->valId));
  if (inst->op() == MIterNextK) {
    args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
  } else {
    args.imm(0);
  }
  cgCallHelper(vmain(), CppCall::direct(miter_next_key), callDest(inst),
               SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgIterFree(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  int offset = iterOffset(inst->extra<IterFree>()->iterId);
  cgCallHelper(vmain(),
               CppCall::method(&Iter::free),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset));
}

void CodeGenerator::cgMIterFree(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  int offset = iterOffset(inst->extra<MIterFree>()->iterId);
  cgCallHelper(vmain(),
               CppCall::method(&Iter::mfree),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset));
}

void CodeGenerator::cgDecodeCufIter(IRInstruction* inst) {
  auto fpReg = srcLoc(1).reg();
  int offset = iterOffset(inst->extra<DecodeCufIter>()->iterId);
  cgCallHelper(vmain(),
               CppCall::direct(decodeCufIterHelper),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup().addr(fpReg, offset)
                                  .typedValue(0));
}

void CodeGenerator::cgCIterFree(IRInstruction* inst) {
  auto fpReg = srcLoc(0).reg();
  int offset = iterOffset(inst->extra<CIterFree>()->iterId);
  cgCallHelper(vmain(),
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
  cgCallHelper(vmain(),
               CppCall::direct(f),
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup().imm(data->numKeys)
                         .imm(uintptr_t(table))
                         .ssa(0/*values*/)
  );
}

void CodeGenerator::cgIncStat(IRInstruction *inst) {
  auto stat = Stats::StatCounter(inst->src(0)->intVal());
  int n = inst->src(1)->intVal();
  bool force = inst->src(2)->boolVal();
  vmain() << incstat{stat, n, force};
}

void CodeGenerator::cgIncTransCounter(IRInstruction* inst) {
  emitTransCounterInc(vmain());
}

void CodeGenerator::cgIncProfCounter(IRInstruction* inst) {
  TransID  transId = inst->extra<TransIDData>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);
  auto& v = vmain();
  auto ptr = v.makeReg();
  v << ldimm{uint64_t(counterAddr), ptr};
  v << decqm{ptr[0]};
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  emitAssertRefCount(vmain(), srcLoc(0).reg());
}

void CodeGenerator::cgDbgAssertType(IRInstruction* inst) {
  auto& v = vmain();
  emitTypeTest(inst->typeParam(),
               srcLoc(0).reg(1),
               srcLoc(0).reg(0),
               [&](ConditionCode cc) {
                 ifThen(v, ccNegate(cc), [&](Vout& v) { v << ud2{}; });
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
  auto v = vmain();
  Immed64 imm = (uintptr_t)enterTCServiceReq;
  if (imm.fits(sz::dword)) {
    v << cmpqim{imm.l(), *rsp};
  } else {
    auto funcptr = v.makeReg();
    v << ldimm{imm, funcptr};
    v << cmpqm{funcptr, *rsp};
  }
  ifThen(v, CC_NE, [&](Vout& v) {
     v << ud2{};
  });
}

void CodeGenerator::emitVerifyCls(IRInstruction* inst) {
  auto const objClass = inst->src(0);
  auto const objClassReg = srcLoc(0).reg();
  auto const constraint = inst->src(1);
  auto const constraintReg = srcLoc(1).reg();

  auto& v = vmain();
  if (constraintReg == InvalidReg) {
    if (objClassReg != InvalidReg) {
      auto constraintCls = constraint->clsVal();
      auto constraintImm = Immed64(constraintCls);
      if (constraintImm.fits(sz::dword)) {
        v << cmpqi{constraintImm.l(), objClassReg};
      } else {
        auto constraintTmp = v.makeReg();
        v << ldimm{constraintCls, constraintTmp};
        v << cmpq{constraintTmp, objClassReg};
      }
    } else {
      // Both constant.
      if (objClass->clsVal() == constraint->clsVal()) return;
      return cgCallNative(v, inst);
    }
  } else if (objClassReg != InvalidReg) {
    v << cmpq{constraintReg, objClassReg};
  } else {
    // Reverse the args because cmpq can only have a constant in the LHS.
    auto objCls = objClass->clsVal();
    auto objImm = Immed64(objCls);
    if (objImm.fits(sz::dword)) {
      v << cmpqi{objImm.l(), constraintReg};
    } else {
      auto objTmp = v.makeReg();
      v << ldimm{objCls, objTmp};
      v << cmpq{objTmp, constraintReg};
    }
  }

  // The native call for this instruction is the slow path that does
  // proper subtype checking. The comparison above is just to
  // short-circuit the overhead when the Classes are an exact match.
  ifThen(v, CC_NE, [&](Vout& v){ cgCallNative(v, inst); });
}

void CodeGenerator::cgVerifyParamCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgVerifyRetCls(IRInstruction* inst) {
  emitVerifyCls(inst);
}

void CodeGenerator::cgRBTrace(IRInstruction* inst) {
  auto const& extra = *inst->extra<RBTrace>();
  auto& v = vmain();
  if (auto const msg = extra.msg) {
    assert(msg->isStatic());
    cgCallHelper(v,
     CppCall::direct(reinterpret_cast<void (*)()>(Trace::ringbufferMsg)),
                 kVoidDest,
                 SyncOptions::kNoSyncPoint,
                 argGroup()
                   .immPtr(msg->data())
                   .imm(msg->size())
                   .imm(extra.type));
  } else {
    auto beforeArgs = v.makePoint();
    v << point{beforeArgs};
    v << ldpoint{beforeArgs, rAsm};
    auto args = argGroup();
    cgCallHelper(v,
      CppCall::direct(reinterpret_cast<void (*)()>(Trace::ringbufferEntry)),
      kVoidDest,
      SyncOptions::kNoSyncPoint,
      argGroup()
        .imm(extra.type)
        .imm(extra.sk.toAtomicInt())
        .reg(rAsm));
  }
}

void CodeGenerator::cgLdClsInitData(IRInstruction* inst) {
  const Vreg rds = rVmTl;
  auto clsReg = srcLoc(0).reg();
  auto dstReg = dstLoc(0).reg();
  auto offset = Class::propDataCacheOff() +
                RDS::Link<Class::PropInitVec*>::handleOff();
  auto& v = vmain();
  v << loadl{clsReg[offset], dstReg};
  v << loadq{rds[dstReg], dstReg};
  v << loadq{dstReg[Class::PropInitVec::dataOff()], dstReg};
}

void CodeGenerator::cgConjure(IRInstruction* inst) {
  vmain() << ud2();
}

void CodeGenerator::cgProfileStr(IRInstruction* inst) {
  auto& v = vmain();
  TargetProfile<StrProfile> profile(m_unit.context(), inst->marker(),
                                    inst->extra<ProfileStrData>()->key);
  assert(profile.profiling());
  auto const ch = profile.handle();

  auto ptrReg = srcLoc(0).reg();
  emitCmpTVType(v, KindOfStaticString, ptrReg[TVOFF(m_type)]);
  ifThenElse(
    v, CC_E,
    [&](Vout& v) { // m_type == KindOfStaticString
      v << inclm{rVmTl[ch + offsetof(StrProfile, staticStr)]};
    },
    [&](Vout& v) { // m_type == KindOfString
      auto ptr = v.makeReg();
      v << loadq{ptrReg[TVOFF(m_data)], ptr};
      v << cmplim{StaticValue, ptr[FAST_REFCOUNT_OFFSET]};

      ifThenElse(
        v, CC_E,
        [&](Vout& v) { // _count == StaticValue
          v << inclm{rVmTl[ch + offsetof(StrProfile, strStatic)]};
        },
        [&](Vout& v) {
          v << inclm{rVmTl[ch + offsetof(StrProfile, str)]};
        }
      );
    }
  );
}

void CodeGenerator::cgCountArray(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();
  auto& v = vmain();

  v << cmpbim{ArrayData::kNvtwKind, baseReg[ArrayData::offsetofKind()]};
  unlikelyIfThenElse(v, vcold(), CC_Z,
    [&](Vout& v) {
      cgCallNative(v, inst);
    },
    [&](Vout& v) {
      v << loadl{baseReg[ArrayData::offsetofSize()], dstReg};
    }
  );
}

void CodeGenerator::cgCountArrayFast(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();
  auto& v = vmain();
  v << loadl{baseReg[ArrayData::offsetofSize()], dstReg};
}

void CodeGenerator::cgCountCollection(IRInstruction* inst) {
  auto const baseReg = srcLoc(0).reg();
  auto const dstReg  = dstLoc(0).reg();
  auto& v = vmain();
  v << loadl{baseReg[FAST_COLLECTION_SIZE_OFFSET], dstReg};
}

void CodeGenerator::print() const {
  jit::print(std::cout, m_unit, &m_state.regs, m_state.asmInfo);
}

}}}
