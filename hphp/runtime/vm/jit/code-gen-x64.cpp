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
#include <iostream>
#include <limits>
#include <unwind.h>
#include <vector>

#include <folly/ScopeGuard.h>
#include <folly/Format.h>
#include "hphp/util/trace.h"
#include "hphp/util/text-util.h"
#include "hphp/util/abi-cxx.h"

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/rds-util.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/asio/asio-blockable.h"
#include "hphp/runtime/ext/asio/async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/wait-handle.h"
#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/base/shape.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/native-calls.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/reg-algorithms.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/service-requests-x64.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stack-offsets-defs.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/runtime.h"

#define rVmSp DontUseRVmSpInThisFile

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
            const Func* vmFunc, bool resumed,
            TransID profTransId) ATTRIBUTE_NORETURN;

void cgPunt(const char* file, int line, const char* func, uint32_t bcOff,
            const Func* vmFunc, bool resumed, TransID profTransId) {
  if (dumpIREnabled()) {
    auto const phpFile = vmFunc->filename()->data();
    auto const phpLine = vmFunc->unit()->getLineNumber(bcOff);
    HPHP::Trace::trace("--------- CG_PUNT %s at %s:%d from %s:%d (bcOff %d)\n",
                       func, file, line, phpFile, phpLine, bcOff);
  }
  throw FailedCodeGen(file, line, func, bcOff, vmFunc, resumed, profTransId);
}

#define CG_PUNT(marker, instr)                                    \
  cgPunt(__FILE__, __LINE__, #instr, marker.bcOff(),              \
         getFunc(marker), resumed(marker), marker.profTransID())

const char* getContextName(const Class* ctx) {
  return ctx ? ctx->name()->data() : ":anonymous:";
}

/*
 * Generate an if-block that branches around some unlikely code, handling
 * the cases when a == astubs and a != astubs.  cc is the branch condition
 * to run the unlikely block.
 *
 * Passes the proper assembler to use to the unlikely function.
 */
template <class Then>
void unlikelyIfThen(Vout& vmain, Vout& vstub, ConditionCode cc, Vreg sf,
                    Then then) {
  auto unlikely = vstub.makeBlock();
  auto done = vmain.makeBlock();
  vmain << jcc{cc, sf, {done, unlikely}};
  vstub = unlikely;
  then(vstub);
  if (!vstub.closed()) vstub << jmp{done};
  vmain = done;
}

// Generate an if-then-else block
template <class Then, class Else>
void ifThenElse(Vout& v, ConditionCode cc, Vreg sf, Then thenBlock,
                Else elseBlock) {
  auto thenLabel = v.makeBlock();
  auto elseLabel = v.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {elseLabel, thenLabel}};
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
void unlikelyIfThenElse(Vout& vmain, Vout& vstub, ConditionCode cc, Vreg sf,
                        Then unlikelyBlock, Else elseBlock) {
  auto elseLabel = vmain.makeBlock();
  auto unlikelyLabel = vstub.makeBlock();
  auto done = vmain.makeBlock();
  vmain << jcc{cc, sf, {elseLabel, unlikelyLabel}};
  vmain = elseLabel;
  elseBlock(vmain);
  if (!vmain.closed()) vmain << jmp{done};
  vstub = unlikelyLabel;
  unlikelyBlock(vstub);
  if (!vstub.closed()) vstub << jmp{done};
  vmain = done;
}

// emit an if-then-else condition where the true case is unlikely.
template <class T, class F>
Vreg unlikelyCond(Vout& v, Vout& vc, ConditionCode cc, Vreg sf, Vreg d, T t,
                  F f) {
  auto fblock = v.makeBlock();
  auto tblock = vc.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {fblock, tblock}};
  vc = tblock;
  auto treg = t(vc);
  vc << phijmp{done, vc.makeTuple({treg})};
  v = fblock;
  auto freg = f(v);
  v << phijmp{done, v.makeTuple({freg})};
  v = done;
  v << phidef{v.makeTuple({d})};
  return d;
}

template<class Then>
void ifRefCountedType(Vout& v, Type ty, Vloc loc, Then then) {
  if (!ty.maybe(Type::Counted)) return;
  if (ty.isKnownDataType()) {
    if (IS_REFCOUNTED_TYPE(ty.toDataType())) then(v);
    return;
  }
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfRefCountThreshold, loc.reg(1));
  ifThen(v, CC_NLE, sf, [&] (Vout& v) {
    then(v);
  });
}

template<class Then>
void ifRefCountedNonStatic(Vout& v, Type ty, Vloc loc, Then then) {
  ifRefCountedType(v, ty, loc, [&] (Vout& v) {
    if (!ty.maybe(Type::Static)) {
      then(v);
      return;
    }
    auto const sf = v.makeReg();
    v << cmplim{0, loc.reg()[FAST_REFCOUNT_OFFSET], sf};
    static_assert(UncountedValue < 0 && StaticValue < 0, "");
    ifThen(v, CC_GE, sf, then);
  });
}

} // unnamed namespace
//////////////////////////////////////////////////////////////////////

template <class Then>
void CodeGenerator::unlikelyIfBlock(Vout& v, Vout& vcold, ConditionCode cc,
                                    Vreg sf, Then then) {
  auto unlikely = vcold.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {done, unlikely}};
  vcold = unlikely;
  then(vcold);
  if (!vcold.closed()) vcold << jmp{done};
  v = done;
}

template <class Block>
void CodeGenerator::ifBlock(Vout& v, Vout& vcold, ConditionCode cc, Vreg sf,
                            Block taken, bool unlikely) {
  if (unlikely) return unlikelyIfBlock(v, vcold, cc, sf, taken);
  auto takenLabel = v.makeBlock();
  auto doneLabel = v.makeBlock();
  v << jcc{cc, sf, {doneLabel, takenLabel}};
  v = takenLabel;
  taken(v);
  if (!v.closed()) v << jmp{doneLabel};
  v = doneLabel;
}

// Generate an if-then-else block
template <class Then, class Else>
void CodeGenerator::ifThenElse(Vout& v, ConditionCode cc, Vreg sf,
                               Then thenBlock, Else elseBlock) {
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

template <class Then, class Else>
void CodeGenerator::ifThenElse(Vout& v, Vout& vcold, ConditionCode cc, Vreg sf,
                               Then thenBlock, Else elseBlock, bool unlikely) {
  if (unlikely) {
    return unlikelyIfThenElse(v, vcold, cc, sf, thenBlock, elseBlock);
  }
  ifThenElse(v, cc, sf, thenBlock, elseBlock);
}

template <class Then, class Else>
void CodeGenerator::unlikelyIfThenElse(Vout& v, Vout& vcold, ConditionCode cc,
                                       Vreg sf, Then unlikelyBlock,
                                       Else elseBlock) {
  auto elseLabel = v.makeBlock();
  auto unlikelyLabel = vcold.makeBlock();
  auto done = v.makeBlock();
  v << jcc{cc, sf, {elseLabel, unlikelyLabel}};
  v = elseLabel;
  elseBlock(v);
  if (!v.closed()) v << jmp{done};
  vcold = unlikelyLabel;
  unlikelyBlock(vcold);
  if (!vcold.closed()) vcold << jmp{done};
  v = done;
}

Vloc CodeGenerator::srcLoc(const IRInstruction* inst, unsigned i) const {
  return m_state.locs[inst->src(i)];
}

Vloc CodeGenerator::dstLoc(const IRInstruction* inst, unsigned i) const {
  return m_state.locs[inst->dst(i)];
}

ArgGroup CodeGenerator::argGroup(const IRInstruction* inst) const {
  return ArgGroup(inst, m_state.locs);
}

void CodeGenerator::cgInst(IRInstruction* inst) {
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

#define CALL_OPCODE(opcode) \
  void CodeGenerator::cg##opcode(IRInstruction* i) { cgCallNative(vmain(), i); }

#define CALL_STK_OPCODE(opcode) \
  CALL_OPCODE(opcode)           \
  CALL_OPCODE(opcode ## Stk)

NOOP_OPCODE(DefConst)
NOOP_OPCODE(DefFP)
NOOP_OPCODE(AssertLoc)
NOOP_OPCODE(Nop)
NOOP_OPCODE(TakeStk)
NOOP_OPCODE(TakeRef)
NOOP_OPCODE(EndGuards)
NOOP_OPCODE(HintLocInner)
NOOP_OPCODE(HintStkInner)
NOOP_OPCODE(AssertStk)

CALL_OPCODE(AddElemStrKey)
CALL_OPCODE(AddElemIntKey)
CALL_OPCODE(AddNewElem)
CALL_OPCODE(ArrayAdd)
CALL_OPCODE(Box)
CALL_OPCODE(ColAddElemC)
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
CALL_OPCODE(CreateSSWH)
CALL_OPCODE(AFWHPrepareChild)
CALL_OPCODE(ABCUnblock)
CALL_OPCODE(NewArray)
CALL_OPCODE(NewMixedArray)
CALL_OPCODE(NewLikeArray)
CALL_OPCODE(AllocPackedArray)
CALL_OPCODE(Clone)
CALL_OPCODE(AllocObj)
CALL_OPCODE(CustomInstanceInit)
CALL_OPCODE(InitProps)
CALL_OPCODE(InitSProps)
CALL_OPCODE(RegisterLiveObj)
CALL_OPCODE(LdClsCtor)
CALL_OPCODE(LookupClsRDSHandle)
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
CALL_OPCODE(RaiseUndefProp)
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

#undef NOOP_OPCODE

Vlabel CodeGenerator::label(Block* b) {
  return m_state.labels[b];
}

void CodeGenerator::emitStoreTypedValue(Vout& v, Vreg base, ptrdiff_t offset,
                                        Vloc src, Type srcType) {
  if (srcType.needsValueReg()) {
    if (srcType <= Type::Bool) {
      auto extended = v.makeReg();
      v << movzbq{src.reg(0), extended};
      v << store{extended, base[offset + TVOFF(m_data)]};
    } else {
      v << store{src.reg(0), base[offset + TVOFF(m_data)]};
    }
  }

  if (src.hasReg(1)) {
    v << storeb{src.reg(1), base[offset + TVOFF(m_type)]};
  } else {
    assertx(srcType.isKnownDataType());
    v << storeb{v.cns(srcType.toDataType()), base[offset + TVOFF(m_type)]};
  }
}

void CodeGenerator::emitFwdJcc(Vout& v, ConditionCode cc, Vreg sf,
                               Block* target) {
  auto next = v.makeBlock();
  v << jcc{cc, sf, {next, m_state.labels[target]}};
  v = next;
}

Vreg CodeGenerator::emitCompare(Vout& v, IRInstruction* inst) {
  auto const type0 = inst->src(0)->type();
  auto const type1 = inst->src(1)->type();

  // can't generate CMP instructions correctly for anything that isn't
  // a bool or a numeric, and we can't mix bool/numerics because
  // -1 == true in PHP, but not in HHIR binary representation
  if (!((type0 <= Type::Int  && type1 <= Type::Int) ||
        (type0 <= Type::Bool && type1 <= Type::Bool) ||
        (type0 <= Type::Cls  && type1 <= Type::Cls))) {
    CG_PUNT(inst->marker(), emitCompare);
  }
  auto reg0 = srcLoc(inst, 0).reg();
  auto reg1 = srcLoc(inst, 1).reg();
  auto const sf = v.makeReg();
  if (type0 <= Type::Bool) {
    v << cmpb{reg1, reg0, sf};
  } else {
    v << cmpq{reg1, reg0, sf};
  }
  return sf;
}

void CodeGenerator::cgDefSP(IRInstruction* inst) {
  auto sp = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << defvmsp{sp};

  if (RuntimeOption::EvalHHIRGenerateAsserts && !inst->marker().resumed()) {
    auto const fp = srcLoc(inst, 0).reg();
    auto expectSp = v.makeReg();
    auto const sf = v.makeReg();
    // Verify that rVmSp == rbp - spOff
    v << lea{fp[-cellsToBytes(inst->extra<StackOffset>()->offset)], expectSp};
    v << cmpq{expectSp, sp, sf};
    ifBlock(v, vcold(), CC_NE, sf, [](Vout& v) { v << ud2(); });
  }
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
  emitLoad(inst->dst(), dstLoc(inst, 0), rVmTl[unwinderTvOff()]);
}

void CodeGenerator::cgBeginCatch(IRInstruction* inst) {
  auto& v = vmain();
  v << landingpad{};

  emitIncStat(v, Stats::TC_CatchTrace);
}

void CodeGenerator::cgEndCatch(IRInstruction* inst) {
  auto& v = vmain();
  // endCatchHelper only expects rVmTl and rVmFp to be live.
  v << jmpi{mcg->tx().uniqueStubs.endCatchHelper, rVmTl | rVmFp};
}

void CodeGenerator::cgUnwindCheckSideExit(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmpbim{0, rVmTl[unwinderSideExitOff()], sf};

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
  using namespace NativeCalls;
  always_assert(CallMap::hasInfo(inst->op()));
  auto const& info = CallMap::info(inst->op());

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

CallDest CodeGenerator::callDest(Vreg reg0) const {
  return { DestType::SSA, reg0 };
}

CallDest CodeGenerator::callDest(Vreg reg0, Vreg reg1) const {
  return { DestType::SSA, reg0, reg1 };
}

CallDest CodeGenerator::callDest(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;
  assertx(loc.numAllocated() == 1);
  return { inst->dst(0)->isA(Type::Bool) ? DestType::Byte : DestType::SSA,
           loc.reg(0) };
}

CallDest CodeGenerator::callDestTV(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(inst, 0);
  if (loc.numAllocated() == 0) return kVoidDest;
  if (loc.isFullSIMD()) {
    assertx(loc.numAllocated() == 1);
    return { DestType::SIMD, loc.reg(0) };
  }
  if (loc.numAllocated() == 2) {
    return { DestType::TV, loc.reg(0), loc.reg(1) };
  }
  assertx(loc.numAllocated() == 1);
  // Sometimes we statically know the type and only need the value.
  return { DestType::TV, loc.reg(0), InvalidReg };
}

CallDest CodeGenerator::callDestDbl(const IRInstruction* inst) const {
  if (!inst->numDsts()) return kVoidDest;
  auto loc = dstLoc(inst, 0);
  return { DestType::Dbl, loc.reg(0) };
}

// We can't really compile using the compact call if the address of the array
// vtable is in high memory (there is only an encoding for 32bit displacement).
// This can happen, for example, if we have address space randomization
// enabled.  For now just punt these cases.
template<class Arg> CppCall
CodeGenerator::arrayCallIfLowMem(const IRInstruction* inst, Arg vtable) const {
  if (!deltaFits(reinterpret_cast<uintptr_t>(vtable), sz::dword)) {
    CG_PUNT(inst->marker(), ArrayDataVtableHighMemory);
  }
  return CppCall::array(vtable);
}

/*
 * Prepare the given ArgDest for a call by shifting or zero-extending as
 * appropriate, then append its Vreg to the given VregList.
 */
static void prepareArg(const ArgDesc& arg, Vout& v, VregList& vargs) {
  switch (arg.kind()) {
    case ArgDesc::Kind::Reg: {
      auto reg = arg.srcReg();
      if (arg.isZeroExtend()) {
        reg = v.makeReg();
        v << movzbq{arg.srcReg(), reg};
      }
      vargs.push_back(reg);
      break;
    }

    case ArgDesc::Kind::TypeReg:
      static_assert(offsetof(TypedValue, m_type) % 8 == 0, "");
      vargs.push_back(arg.srcReg());
      break;

    case ArgDesc::Kind::Imm: {
      vargs.push_back(v.cns(arg.imm().q()));
      break;
    }

    case ArgDesc::Kind::Addr: {
      auto tmp = v.makeReg();
      v << lea{arg.srcReg()[arg.disp().l()], tmp};
      vargs.push_back(tmp);
      break;
    }
  }
}

void
CodeGenerator::cgCallHelper(Vout& v, CppCall call, const CallDest& dstInfo,
                            SyncOptions sync, const ArgGroup& args) {
  auto const inst = args.inst();
  jit::vector<Vreg> vargs, vSimdArgs, vStkArgs;
  for (size_t i = 0; i < args.numGpArgs(); ++i) {
    prepareArg(args.gpArg(i), v, vargs);
  }
  for (size_t i = 0; i < args.numSimdArgs(); ++i) {
    prepareArg(args.simdArg(i), v, vSimdArgs);
  }
  for (size_t i = 0; i < args.numStackArgs(); ++i) {
    prepareArg(args.stkArg(i), v, vStkArgs);
  }

  Fixup syncFixup;
  if (RuntimeOption::HHProfServerEnabled || sync != SyncOptions::kNoSyncPoint) {
    // If we are profiling the heap, we always need to sync because regs need
    // to be correct during smart allocations no matter what
    syncFixup = makeFixup(inst->marker(), sync);
  }

  Vlabel targets[2];
  bool nothrow = false;
  auto* taken = inst->taken();
  auto const do_catch = taken && taken->isCatch();
  if (do_catch) {
    always_assert_flog(
      inst->is(InterpOne) || sync != SyncOptions::kNoSyncPoint,
      "cgCallHelper called with kNoSyncPoint but inst has a catch block: {}\n",
      *inst
    );
    always_assert_flog(
      taken->catchMarker() == inst->marker(),
      "Catch trace doesn't match fixup:\n"
      "Instruction: {}\n"
      "Catch trace: {}\n"
      "Fixup      : {}\n",
      inst->toString(),
      taken->catchMarker().show(),
      inst->marker().show()
    );

    targets[0] = v.makeBlock();
    targets[1] = m_state.labels[taken];
  } else {
    // The current instruction doesn't have a catch block so it'd better not
    // throw. Register a null catch trace to indicate this to the
    // unwinder.
    nothrow = true;
  }

  VregList dstRegs;
  if (dstInfo.reg0.isValid()) {
    dstRegs.push_back(dstInfo.reg0);
    if (dstInfo.reg1.isValid()) {
      dstRegs.push_back(dstInfo.reg1);
    }
  }

  auto argsId = v.makeVcallArgs(
    {std::move(vargs), std::move(vSimdArgs), std::move(vStkArgs)});
  auto dstId = v.makeTuple(std::move(dstRegs));
  if (do_catch) {
    v << vinvoke{call, argsId, dstId, {targets[0], targets[1]},
        syncFixup, dstInfo.type, sync == SyncOptions::kSmashableAndSyncPoint};
    v = targets[0];
  } else {
    v << vcall{call, argsId, dstId, syncFixup, dstInfo.type, nothrow};
  }
}

void CodeGenerator::cgMov(IRInstruction* inst) {
  always_assert(inst->src(0)->numWords() == inst->dst(0)->numWords());
  copyTV(vmain(), srcLoc(inst, 0), dstLoc(inst, 0), inst->dst()->type());
}

void CodeGenerator::cgAbsDbl(IRInstruction* inst) {
  auto src = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << absdbl{src, dst};
}

Vreg CodeGenerator::emitAddInt(Vout& v, IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto const sf = v.makeReg();
  v << addq{s1, s0, d, sf};
  return sf;
}

Vreg CodeGenerator::emitSubInt(Vout& v, IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto const sf = v.makeReg();
  v << subq{s1, s0, d, sf};
  return sf;
}

Vreg CodeGenerator::emitMulInt(Vout& v, IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto const sf = v.makeReg();

  v << imul{s1, s0, d, sf};
  return sf;
}

void CodeGenerator::cgAddIntO(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitAddInt(v, inst);
  v << jcc{CC_O, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgSubIntO(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitSubInt(v, inst);
  assertx(sf != InvalidReg);
  v << jcc{CC_O, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgMulIntO(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitMulInt(v, inst);
  v << jcc{CC_O, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgFloor(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  vmain() << roundsd{RoundDirection::floor, srcReg, dstReg};
}

void CodeGenerator::cgCeil(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  vmain() << roundsd{RoundDirection::ceil, srcReg, dstReg};
}

void CodeGenerator::cgAddInt(IRInstruction* inst) {
  emitAddInt(vmain(), inst);
}

void CodeGenerator::cgSubInt(IRInstruction* inst) {
  emitSubInt(vmain(), inst);
}

void CodeGenerator::cgMulInt(IRInstruction* inst) {
  emitMulInt(vmain(), inst);
}

void CodeGenerator::cgAddDbl(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  vmain() << addsd{s1, s0, d};
}

void CodeGenerator::cgSubDbl(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  vmain() << subsd{s1, s0, d};
}

void CodeGenerator::cgMulDbl(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << mulsd{s1, s0, d};
}

void CodeGenerator::cgDivDbl(IRInstruction* inst) {
  auto srcReg0 = srcLoc(inst, 0).reg(); // dividend
  auto srcReg1 = srcLoc(inst, 1).reg(); // divisor
  auto dstReg  = dstLoc(inst, 0).reg();
  auto exit = inst->taken();
  auto& v = vmain();

  // divide by zero check
  auto const sf = v.makeReg();
  v << ucomisd{v.cns(0), srcReg1, sf};
  unlikelyIfBlock(v, vcold(), CC_NP, sf, [&] (Vout& v) {
    emitFwdJcc(v, CC_E, sf, exit);
  });
  v << divsd{srcReg1, srcReg0, dstReg};
}

void CodeGenerator::cgAndInt(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << andq{s1, s0, d, v.makeReg()};
}

void CodeGenerator::cgOrInt(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << orq{s1, s0, d, v.makeReg()};
}

void CodeGenerator::cgXorInt(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << xorq{s1, s0, d, v.makeReg()};
}

void CodeGenerator::cgXorBool(IRInstruction* inst) {
  auto s0 = srcLoc(inst, 0).reg();
  auto s1 = srcLoc(inst, 1).reg();
  auto d = dstLoc(inst, 0).reg();
  auto& v = vmain();
  v << xorb{s1, s0, d, v.makeReg()};
}

void CodeGenerator::cgMod(IRInstruction* inst) {
  auto const dst = dstLoc(inst, 0).reg();
  auto const dividend = srcLoc(inst, 0).reg();
  auto const divisor = srcLoc(inst, 1).reg();
  auto& v = vmain();

  v << srem{dividend, divisor, dst};
}

void CodeGenerator::cgSqrt(IRInstruction* inst) {
  auto src = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  vmain() << sqrtsd{src, dst};
}

template<class Op, class Opi>
void CodeGenerator::cgShiftCommon(IRInstruction* inst) {
  auto const src1 = inst->src(1);
  auto const srcReg0 = srcLoc(inst, 0).reg();
  auto const srcReg1 = srcLoc(inst, 1).reg();
  auto const dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();

  if (src1->hasConstVal()) {
    int n = src1->intVal() & 0x3f; // only use low 6 bits.
    v << Opi{n, srcReg0, dstReg, v.makeReg()};
  } else {
    v << Op{srcReg1, srcReg0, dstReg, v.makeReg()};
  }
}

void CodeGenerator::cgShl(IRInstruction* inst) {
  cgShiftCommon<shl,shlqi>(inst);
}

void CodeGenerator::cgShr(IRInstruction* inst) {
  cgShiftCommon<sar,sarqi>(inst);
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

  auto loc1 = srcLoc(inst, 0);
  auto loc2 = srcLoc(inst, 1);

  auto src1Reg = loc1.reg();
  auto src2Reg = loc2.reg();
  auto dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();

  /////////////////////////////////////////////////////////////////////////////
  // case 1: null/string cmp string
  // simplifyCmp has converted the null to ""
  if (type1 <= Type::Str && type2 <= Type::Str) {
    cgCallHelper(v, CppCall::direct(str_cmp_str), callDest(inst),
      SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 2: bool/null cmp anything
  // simplifyCmp has converted all args to bool
  else if (type1 <= Type::Bool && type2 <= Type::Bool) {
    auto const sf = v.makeReg();
    if (src2->hasConstVal()) {
      v << cmpbi{src2->boolVal(), src1Reg, sf};
    } else {
      v << cmpb{src2Reg, src1Reg, sf};
    }
    v << setcc{cc, sf, dstReg};
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
                     SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
      } else if (type2 <= Type::Obj) {
        cgCallHelper(v, CppCall::direct(str_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
      } else {
        CG_PUNT(inst->marker(), cgOpCmpHelper_sx);
      }
    }

    else if (type1 <= Type::Obj) {
      // string cmp object is dealt with above
      // object cmp double is punted above

      if (type2 <= Type::Obj) {
        cgCallHelper(v, CppCall::direct(obj_cmp_obj), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
      } else if (type2 <= Type::Int) {
        cgCallHelper(v, CppCall::direct(obj_cmp_int), callDest(inst),
                     SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
      } else {
        CG_PUNT(inst->marker(), cgOpCmpHelper_ox);
      }
    }
    else {
      CG_PUNT(inst->marker(), cgOpCmpHelper_SON);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 5: array cmp array
  else if (type1 <= Type::Arr && type2 <= Type::Arr) {
    cgCallHelper(v, CppCall::direct(arr_cmp_arr),
      callDest(inst), SyncOptions::kSyncPoint, argGroup(inst).ssa(0).ssa(1));
  }

  /////////////////////////////////////////////////////////////////////////////
  // case 6: array cmp anything
  // simplifyCmp has already dealt with this case.

  /////////////////////////////////////////////////////////////////////////////
  else {
    // We have a type which is not a common type. It might be a cell or a box.
    CG_PUNT(inst->marker(), cgOpCmpHelper_unimplemented);
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
  auto dst = dstLoc(inst, 0).reg();
  auto src0 = srcLoc(inst, 0).reg();
  auto src1 = srcLoc(inst, 1).reg();
  auto& v = vmain();
  auto sf = v.makeReg();
  // Note the reverse syntax in the assembler: will compute src0 - src1
  v << cmpq{src1, src0, sf};
  v << setcc{cc, sf, dst};
}

void CodeGenerator::cgEqInt(IRInstruction* inst)  { emitCmpInt(inst, CC_E); }
void CodeGenerator::cgNeqInt(IRInstruction* inst) { emitCmpInt(inst, CC_NE); }
void CodeGenerator::cgLtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_L); }
void CodeGenerator::cgGtInt(IRInstruction* inst)  { emitCmpInt(inst, CC_G); }
void CodeGenerator::cgLteInt(IRInstruction* inst) { emitCmpInt(inst, CC_LE); }
void CodeGenerator::cgGteInt(IRInstruction* inst) { emitCmpInt(inst, CC_GE); }

void CodeGenerator::emitCmpEqDbl(IRInstruction* inst, ComparisonPred pred) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg0 = srcLoc(inst, 0).reg();
  auto srcReg1 = srcLoc(inst, 1).reg();
  auto& v = vmain();
  auto tmp = v.makeReg();
  v << cmpsd{pred, srcReg0, srcReg1, tmp};
  v << andbi{1, tmp, dstReg, v.makeReg()};
}

void CodeGenerator::emitCmpRelDbl(IRInstruction* inst, ConditionCode cc,
                                  bool flipOperands) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg0 = srcLoc(inst, 0).reg();
  auto srcReg1 = srcLoc(inst, 1).reg();
  auto& v = vmain();
  if (flipOperands) {
    std::swap(srcReg0, srcReg1);
  }
  auto const sf = v.makeReg();
  v << ucomisd{srcReg0, srcReg1, sf};
  v << setcc{cc, sf, dstReg};
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
  v << load{dataSrc, t};
  return t;
}

template<class Loc1, class Loc2, class JmpFn>
void CodeGenerator::emitTypeTest(Type type, Loc1 typeSrc, Loc2 dataSrc,
                                 Vreg sf, JmpFn doJcc) {
  // Note: if you add new supported type tests, you should update
  // negativeCheckType() to indicate whether it is precise or not.
  always_assert(!(type <= Type::Cls));
  always_assert(!type.hasConstVal());
  auto& v = vmain();
  ConditionCode cc;
  if (type <= Type::StaticStr) {
    emitCmpTVType(v, sf, KindOfStaticString, typeSrc);
    cc = CC_E;
  } else if (type <= Type::Str) {
    always_assert(type != Type::CountedStr &&
                  "We don't support guarding on CountedStr");
    emitTestTVType(v, sf, KindOfStringBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Null) {
    emitCmpTVType(v, sf, KindOfNull, typeSrc);
    cc = CC_LE;
  } else if (type == Type::UncountedInit) {
    emitTestTVType(v, sf, KindOfUncountedInitBit, typeSrc);
    cc = CC_NZ;
  } else if (type == Type::Uncounted) {
    emitCmpTVType(v, sf, KindOfRefCountThreshold, typeSrc);
    cc = CC_LE;
  } else if (type == Type::Cell) {
    emitCmpTVType(v, sf, KindOfRef, typeSrc);
    cc = CC_L;
  } else if (type == Type::Gen) {
    // nothing to check
    return;
  } else {
    always_assert(type.isKnownDataType());
    always_assert(!(type < Type::BoxedInitCell));
    DataType dataType = type.toDataType();
    assertx(dataType == KindOfRef ||
           (dataType >= KindOfUninit && dataType <= KindOfResource));
    emitCmpTVType(v, sf, dataType, typeSrc);
    cc = CC_E;
  }
  doJcc(cc, sf);

  if (type.isSpecialized()) {
    auto const sf2 = v.makeReg();
    emitSpecializedTypeTest(type, dataSrc, sf2, doJcc);
  }
}

template<class DataLoc, class JmpFn>
void CodeGenerator::emitSpecializedTypeTest(Type type, DataLoc dataSrc, Vreg sf,
                                            JmpFn doJcc) {
  if (type < Type::Res) {
    // No cls field in Resource
    always_assert(0 && "unexpected guard on specialized Resource");
  }

  auto& v = vmain();
  if (type < Type::Obj) {
    // Emit the specific class test.
    assertx(type.clsSpec());
    assertx(type.clsSpec().cls()->attrs() & AttrNoOverride);

    auto reg = getDataPtrEnregistered(v, dataSrc);
    emitCmpClass(v, sf, type.clsSpec().cls(),
                 reg[ObjectData::getVMClassOffset()]);
    doJcc(CC_E, sf);
  } else {
    assertx(type < Type::Arr && type.arrSpec() && type.arrSpec().kind());
    assertx(type.arrSpec().type() == nullptr);

    auto arrSpec = type.arrSpec();
    auto reg = getDataPtrEnregistered(v, dataSrc);

    v << cmpbim{*arrSpec.kind(), reg[ArrayData::offsetofKind()], sf};
    doJcc(CC_E, sf);

    if (arrSpec.kind() == ArrayData::kStructKind && arrSpec.shape()) {
      auto newSf = v.makeReg();
      auto offset = StructArray::shapeOffset();
      v << cmpqm{v.cns(arrSpec.shape()), reg[offset], newSf};
      doJcc(CC_E, newSf);
    }
  }
}

template<class JmpFn>
void CodeGenerator::emitIsTypeTest(IRInstruction* inst, Vreg sf, JmpFn doJcc) {
  auto const src = inst->src(0);
  auto const loc = srcLoc(inst, 0);

  // punt if specialized object for now
  if (inst->typeParam() < Type::Obj || inst->typeParam() < Type::Res) {
    CG_PUNT(inst->marker(), IsType-SpecializedUnsupported);
  }

  if (src->isA(Type::PtrToGen)) {
    auto base = loc.reg();
    emitTypeTest(inst->typeParam(), base[TVOFF(m_type)],
                 base[TVOFF(m_data)], sf, doJcc);
    return;
  }
  assertx(src->isA(Type::Gen));

  auto typeSrcReg = loc.reg(1); // type register
  if (typeSrcReg == InvalidReg) {
    // Should only get here if the simplifier didn't run
    // TODO: #3626251 will handle this case.
    CG_PUNT(inst->marker(), IsType-KnownType);
  }
  auto dataSrcReg = loc.reg(0); // data register
  emitTypeTest(inst->typeParam(), typeSrcReg, dataSrcReg, sf, doJcc);
}

template<class Loc>
void CodeGenerator::emitTypeCheck(Type type, Loc typeSrc, Loc dataSrc,
                                  Block* taken) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitTypeTest(type, typeSrc, dataSrc, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      emitFwdJcc(v, ccNegate(cc), sfTaken, taken);
    });
}

void CodeGenerator::emitSetCc(IRInstruction* inst, ConditionCode cc, Vreg sf) {
  vmain() << setcc{cc, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgIsTypeMemCommon(IRInstruction* inst, bool negate) {
  bool called = false; // check emitSetCc is called only once
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitIsTypeTest(inst, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      assertx(!called);
      emitSetCc(inst, negate ? ccNegate(cc) : cc, sfTaken);
      called = true;
    });
}

void CodeGenerator::cgIsTypeCommon(IRInstruction* inst, bool negate) {
  bool called = false; // check emitSetCc is called only once
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitIsTypeTest(inst, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      assertx(!called);
      emitSetCc(inst, negate ? ccNegate(cc) : cc, sfTaken);
      called = true;
    });
}

void CodeGenerator::cgIsType(IRInstruction* inst) {
  cgIsTypeCommon(inst, false);
}

void CodeGenerator::cgIsScalarType(IRInstruction* inst) {
  auto typeReg = srcLoc(inst, 0).reg(1);
  auto dstReg  = dstLoc(inst, 0).reg(0);

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
    v << copy{v.cns(imm), dstReg};
    return;
  }

  auto diff = v.makeReg();
  v << subbi{KindOfBoolean, typeReg, diff, v.makeReg()};
  auto const sf = v.makeReg();
  v << cmpbi{KindOfString - KindOfBoolean, diff, sf};
  v << setcc{CC_BE, sf, dstReg};
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
Vreg CodeGenerator::emitInstanceBitmaskCheck(Vout& v, IRInstruction* inst) {
  auto const rObjClass     = srcLoc(inst, 0).reg(0);
  auto const testClassName = inst->src(1)->strVal();
  int offset;
  uint8_t mask;
  if (!InstanceBits::getMask(testClassName, offset, mask)) {
    always_assert(!"cgInstanceOfBitmask had no bitmask");
  }
  auto const sf = v.makeReg();
  v << testbim{int8_t(mask), rObjClass[offset], sf};
  return sf;
}

void CodeGenerator::cgInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitInstanceBitmaskCheck(v, inst);
  v << setcc{CC_NZ, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgNInstanceOfBitmask(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = emitInstanceBitmaskCheck(v, inst);
  v << setcc{CC_Z, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgInstanceOf(IRInstruction* inst) {
  auto test = inst->src(1);
  auto testReg = srcLoc(inst, 1).reg();
  auto destReg = dstLoc(inst, 0).reg();
  auto& v = vmain();

  auto call_classof = [&](Vreg dst) {
    cgCallHelper(
      v,
      CppCall::method(&Class::classof),
      {DestType::Byte, dst},
      SyncOptions::kNoSyncPoint,
      argGroup(inst).ssa(0).ssa(1)
    );
    return dst;
  };

  if (test->hasConstVal(Type::Cls)) {
    // Don't need to do the null check when the class is const.
    assertx(test->clsVal() != nullptr);
    call_classof(destReg);
    return;
  }

  auto const sf = v.makeReg();
  v << testq{testReg, testReg, sf};
  cond(v, CC_NZ, sf, destReg, [&](Vout& v) {
    return call_classof(v.makeReg());
  }, [&](Vout& v) {
    // testReg == 0, set dest to false
    return v.cns(false);
  });
}

/*
 * Check instanceof using the superclass vector on the end of the
 * Class entry.
 */
void CodeGenerator::cgExtendsClass(IRInstruction* inst) {
  auto const rdst          = dstLoc(inst, 0).reg();
  auto const rObjClass     = srcLoc(inst, 0).reg();
  auto const rTestClass    = srcLoc(inst, 1).reg();
  auto const testClass     = inst->src(1)->clsVal();
  auto& v = vmain();

  // check whether rObjClass points to a strict subclass of rTestClass,
  // set dst with the bool true/false result, and return dst.
  auto check_strict_subclass = [&](Vreg dst) {
    // Check the length of the class vectors. If the candidate's is at
    // least as long as the potential base (testClass) it might be a
    // subclass.
    auto const sf = v.makeReg();
    v << cmplim{safe_cast<int32_t>(testClass->classVecLen()),
                rObjClass[Class::classVecLenOff()], sf};
    return cond(v, CC_NB, sf, dst, [&](Vout& v) {
      // If it's a subclass, rTestClass must be at the appropriate index.
      auto const vecOffset = Class::classVecOff() +
        sizeof(LowClassPtr) * (testClass->classVecLen() - 1);
      auto const b = v.makeReg();
      auto const sf = v.makeReg();
      emitCmpClass(v, sf, rTestClass, rObjClass[vecOffset]);
      v << setcc{CC_E, sf, b};
      return b;
    }, [&](Vout& v) {
      return v.cns(false);
    });
  };

  if (testClass->attrs() & AttrAbstract) {
    // If the test must be extended, don't check for the same class.
    check_strict_subclass(rdst);
    return;
  }

  // Test if it is the exact same class.  TODO(#2044801): we should be
  // doing this control flow at the IR level.
  auto const sf = v.makeReg();
  emitCmpClass(v, sf, rTestClass, rObjClass);
  if (testClass->attrs() & AttrNoOverride) {
    // If the test class cannot be extended, we only need to do the
    // same-class check, never the strict-subclass check.
    v << setcc{CC_E, sf, rdst};
    return;
  }

  cond(v, CC_E, sf, rdst, [&](Vout& v) {
    return v.cns(true);
  }, [&](Vout& v) {
    return check_strict_subclass(v.makeReg());
  });
}

void CodeGenerator::cgClsNeq(IRInstruction* inst) {
  auto const rdst      = dstLoc(inst, 0).reg();
  const Vreg rObjClass = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto testClass       = v.cns(inst->extra<ClsNeqData>()->testClass);
  auto const sf = v.makeReg();
  emitCmpClass(v, sf, testClass, rObjClass);
  v << setcc{CC_NE, sf, rdst};
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
        return dst1;
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

  auto size = v.makeReg();
  v << loadl{srcReg[ArrayData::offsetofSize()], size};
  auto const sf = v.makeReg();
  v << testl{size, size, sf};

  unlikelyCond(v, vcold(), CC_S, sf, dstReg,
    [&](Vout& v) {
      auto vsize = v.makeReg();
      auto dst1 = v.makeReg();
      cgCallHelper(v, CppCall::method(&ArrayData::vsize),
                   callDest(vsize), SyncOptions::kNoSyncPoint,
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

void CodeGenerator::cgColIsEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assertx(ty < Type::Obj &&
         ty.clsSpec().cls() &&
         ty.clsSpec().cls()->isCollectionClass());
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmplim{0, srcLoc(inst, 0).reg()[FAST_COLLECTION_SIZE_OFFSET], sf};
  v << setcc{CC_E, sf, dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgColIsNEmpty(IRInstruction* inst) {
  DEBUG_ONLY auto const ty = inst->src(0)->type();
  assertx(ty < Type::Obj &&
         ty.clsSpec().cls() &&
         ty.clsSpec().cls()->isCollectionClass());
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmplim{0, srcLoc(inst, 0).reg()[FAST_COLLECTION_SIZE_OFFSET], sf};
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
          v << cmplim{0, rsrc[FAST_COLLECTION_SIZE_OFFSET], sf};
          v << setcc{CC_NE, sf, dst2}; // true iff size not zero
          return dst2;
        }, [&] (Vout& v) { // rsrc is not a native collection
          auto dst3 = v.makeReg();
          cgCallHelper(v,
            CppCall::method(&ObjectData::toBoolean),
            CallDest{DestType::Byte, dst3},
            SyncOptions::kSyncPoint,
            argGroup(inst).ssa(0));
          return dst3;
        });
    }, [&] (Vout& v) {
      return v.cns(true);
    }
  );
}

void CodeGenerator::emitConvBoolOrIntToDbl(IRInstruction* inst) {
  SSATmp* src = inst->src(0);
  assertx(src->isA(Type::Bool) || src->isA(Type::Int));
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

void CodeGenerator::cgConvBoolToStr(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto srcReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto f = v.cns(makeStaticString(""));
  auto t = v.cns(makeStaticString("1"));
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

void CodeGenerator::cgUnboxPtr(IRInstruction* inst) {
  auto src = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfRef, src[TVOFF(m_type)]);
  if (RefData::tvOffset() == 0) {
    v << cloadq{CC_E, sf, src, src[TVOFF(m_data)], dst};
    return;
  }
  cond(v, CC_E, sf, dst, [&](Vout& v) {
    auto ref_ptr = v.makeReg();
    auto cell_ptr = v.makeReg();
    v << load{src[TVOFF(m_data)], ref_ptr};
    v << lea{ref_ptr[RefData::tvOffset()], cell_ptr};
    return cell_ptr;
  }, [&](Vout& v) {
    return src;
  });
}

Vreg CodeGenerator::cgLdFuncCachedCommon(IRInstruction* inst, Vreg dst) {
  auto const name = inst->extra<LdFuncCachedData>()->name;
  auto const ch   = NamedEntity::get(name)->getFuncHandle();
  auto& v = vmain();
  v << load{rVmTl[ch], dst};
  auto const sf = v.makeReg();
  v << testq{dst, dst, sf};
  return sf;
}

void CodeGenerator::cgLdFuncCached(IRInstruction* inst) {
  auto& v = vmain();
  auto dst1 = v.makeReg();
  auto const sf = cgLdFuncCachedCommon(inst, dst1);
  unlikelyCond(v, vcold(), CC_Z, sf, dstLoc(inst, 0).reg(), [&] (Vout& v) {
    auto dst2 = v.makeReg();
    const Func* (*const func)(const StringData*) = lookupUnknownFunc;
    cgCallHelper(v,
      CppCall::direct(func),
      callDest(dst2),
      SyncOptions::kSyncPoint,
      argGroup(inst)
        .immPtr(inst->extra<LdFuncCached>()->name)
    );
    return dst2;
  }, [&](Vout& v) {
    return dst1;
  });
}

void CodeGenerator::cgLdFuncCachedSafe(IRInstruction* inst) {
  cgLdFuncCachedCommon(inst, dstLoc(inst, 0).reg());
}

void CodeGenerator::cgLdFuncCachedU(IRInstruction* inst) {
  auto const dstReg    = dstLoc(inst, 0).reg();
  auto const extra     = inst->extra<LdFuncCachedU>();
  auto const hFunc     = NamedEntity::get(extra->name)->getFuncHandle();
  auto& v = vmain();

  // Check the first function handle, otherwise try to autoload.
  auto dst1 = v.makeReg();
  v << load{rVmTl[hFunc], dst1};
  auto const sf = v.makeReg();
  v << testq{dst1, dst1, sf};

  unlikelyCond(v, vcold(), CC_Z, sf, dstReg, [&] (Vout& v) {
    // If we get here, things are going to be slow anyway, so do all the
    // autoloading logic in lookupFallbackFunc instead of ASM
    const Func* (*const func)(const StringData*, const StringData*) =
        lookupFallbackFunc;
    auto dst2 = v.makeReg();
    cgCallHelper(v, CppCall::direct(func), callDest(dst2),
      SyncOptions::kSyncPoint,
      argGroup(inst)
        .immPtr(extra->name)
        .immPtr(extra->fallback)
    );
    return dst2;
  }, [&](Vout& v) {
    return dst1;
  });
}

void CodeGenerator::cgLdFunc(IRInstruction* inst) {
  auto const ch = FuncCache::alloc();
  rds::recordRds(ch, sizeof(FuncCache),
                 "FuncCache", getFunc(inst->marker())->fullName()->data());

  // raises an error if function not found
  cgCallHelper(vmain(),
               CppCall::direct(FuncCache::lookup),
               callDest(dstLoc(inst, 0).reg()),
               SyncOptions::kSyncPoint,
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
    CppCall::direct(loadArrayFunctionContext),
    callDest(inst),
    SyncOptions::kSyncPoint,
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
    CppCall::direct(fpushCufHelperArray),
    callDest(inst),
    SyncOptions::kSyncPoint,
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
    CppCall::direct(fpushCufHelperString),
    callDest(inst),
    SyncOptions::kSyncPoint,
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
    CppCall::direct(lookupClsMethodHelper),
    callDest(inst),
    SyncOptions::kSyncPoint,
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

  auto const handle = rds::alloc<Entry, sizeof(Entry)>().handle();
  if (RuntimeOption::EvalPerfDataMap) {
    auto const caddr_hand = reinterpret_cast<char*>(
      static_cast<intptr_t>(handle)
    );
    Debug::DebugInfo::recordDataMap(
      caddr_hand,
      caddr_hand + sizeof(TypedValue),
      folly::format("rds+MethodCache-{}",
        getFunc(inst->marker())->fullName()->data()).str());
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
  auto const sf = v.makeReg();
  v << cmpq{classptr, clsReg, sf};
  v << jcc{CC_NE, sf, {fast_path, slow_path}};

  v = fast_path;
  auto funcptr = v.makeReg();
  v << shrqi{32, func_class, funcptr, v.makeReg()};
  v << store{funcptr, actRecReg[cellsToBytes(extra->offset.offset) +
    AROFF(m_func)]};
  v << jmp{done};

  v = slow_path;
  cgCallHelper(v,
    CppCall::direct(mcHandler),
    kVoidDest,
    SyncOptions::kSmashableAndSyncPoint,
    argGroup(inst)
      .addr(rVmTl, safe_cast<int32_t>(handle))
      .addr(srcLoc(inst, 1).reg(), cellsToBytes(extra->offset.offset))
      .immPtr(extra->method)
      .ssa(0/*cls*/)
      .immPtr(getClass(inst->marker()))
      // The scratch reg contains the prime data before we've smashed the call
      // to handleSlowPath.  After, it contains the primed Class/Func pair.
      .reg(func_class)
  );
  v << jmp{done};
  v = done;
}

void CodeGenerator::cgLdObjInvoke(IRInstruction* inst) {
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto& v = vmain();
  emitLdLowPtr(v, rsrc[Class::invokeOff()], rdst, sizeof(LowFuncPtr));
  auto const sf = v.makeReg();
  v << testq{rdst, rdst, sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgStRetVal(IRInstruction* inst) {
  auto  const rFp = srcLoc(inst, 0).reg();
  auto* const val = inst->src(1);
  emitStore(rFp[AROFF(m_r)], val, srcLoc(inst, 1), Width::Full);
}

void CodeGenerator::cgRetAdjustStk(IRInstruction* inst) {
  auto const rFp   = srcLoc(inst, 0).reg();
  auto const dstSp = dstLoc(inst, 0).reg();
  vmain() << lea{rFp[AROFF(m_r)], dstSp};
}

void CodeGenerator::cgLdRetAddr(IRInstruction* inst) {
  auto const fp = srcLoc(inst, 0).reg();
  vmain() << load{fp[AROFF(m_savedRip)], dstLoc(inst, 0).reg()};
}

void traceRet(ActRec* fp, Cell* sp, void* rip) {
  if (rip == mcg->tx().uniqueStubs.callToExit) {
    return;
  }
  checkFrame(fp, sp, /*fullCheck*/ false, 0);
  assertx(sp <= (Cell*)fp || fp->resumed());
  // check return value if stack not empty
  if (sp < (Cell*)fp) assertTv(sp);
}

void CodeGenerator::cgRetCtrl(IRInstruction* inst) {
  auto& v = vmain();
  auto const sp = srcLoc(inst, 0).reg();
  auto const fp = srcLoc(inst, 1).reg();
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(inst->extra<RetCtrl>()->spOffset.offset)], sync_sp};
  v << syncvmsp{sync_sp};

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    auto ripReg = v.makeReg();
    v << load{fp[AROFF(m_savedRip)], ripReg};
    auto prev_fp = v.makeReg();
    v << load{fp[AROFF(m_sfp)], prev_fp};
    v << vcall{CppCall::direct(traceRet),
               v.makeVcallArgs({{prev_fp, sync_sp, ripReg}}), v.makeTuple({})};
  }

  v << vretm{fp[AROFF(m_savedRip)], fp[AROFF(m_sfp)], rVmFp, kCrossTraceRegs};
}

void CodeGenerator::cgAsyncRetCtrl(IRInstruction* inst) {
  auto& v = vmain();
  auto const sp = srcLoc(inst, 0).reg();
  auto const fp = srcLoc(inst, 1).reg();
  auto const retAddr = srcLoc(inst, 2).reg();
  auto const sync_sp = v.makeReg();
  v << lea{sp[cellsToBytes(inst->extra<AsyncRetCtrl>()->offset.offset)],
           sync_sp};
  v << syncvmsp{sync_sp};

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    v << vcall{CppCall::direct(traceRet),
               v.makeVcallArgs({{fp, sync_sp, retAddr}}), v.makeTuple({})};
  }

  v << vret{retAddr, kCrossTraceRegs};
}

void CodeGenerator::cgLdBindAddr(IRInstruction* inst) {
  auto data   = inst->extra<LdBindAddr>();
  auto dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();

  // Emit service request to smash address of SrcKey into 'addr'.
  TCA* addrPtr = mcg->allocData<TCA>(sizeof(TCA), 1);
  v << bindaddr{addrPtr, data->sk};

  // Load the maybe bound address.
  auto addr = intptr_t(addrPtr);
  // the tc/global data is intentionally layed out to guarantee
  // rip-relative addressing will work.
  // Also, a rip-relative load, is 1 byte smaller than the corresponding
  // baseless load.
  v << loadqp{rip[addr], dstReg};
}

void CodeGenerator::cgJmpSwitchDest(IRInstruction* inst) {
  auto const data     = inst->extra<JmpSwitchDest>();
  auto const index    = inst->src(0);
  auto const indexReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  v << syncvmsp{srcLoc(inst, 1).reg()};

  if (!index->hasConstVal()) {
    auto idx = indexReg;
    if (data->bounded) {
      if (data->base) {
        idx = v.makeReg();
        v << subq{v.cns(data->base), indexReg, idx, v.makeReg()};
      }
      auto const sf = v.makeReg();
      v << cmpqi{data->cases - 2, idx, sf};
      v << bindjcc{CC_AE, sf, data->defaultSk, TransFlags{}, kCrossTraceRegs};
    }

    TCA* table = mcg->allocData<TCA>(sizeof(TCA), data->cases);
    auto t = v.makeReg();
    for (int i = 0; i < data->cases; i++) {
      v << bindaddr{&table[i], data->targets[i]};
    }
    v << leap{rip[(intptr_t)table], t};
    v << jmpm{t[idx*8], kCrossTraceRegs};
    return;
  }

  int64_t indexVal = index->intVal();
  if (data->bounded) {
    indexVal -= data->base;
    if (indexVal >= data->cases - 2 || indexVal < 0) {
      v << bindjmp{data->defaultSk, TransFlags(), kCrossTraceRegs};
      return;
    }
  }
  v << bindjmp{data->targets[indexVal], TransFlags(), kCrossTraceRegs};
}

void CodeGenerator::cgLdSSwitchDestFast(IRInstruction* inst) {
  auto data = inst->extra<LdSSwitchDestFast>();

  auto table = mcg->allocData<SSwitchMap>(64);
  new (table) SSwitchMap(data->numCases);
  auto& v = vmain();

  for (int64_t i = 0; i < data->numCases; ++i) {
    table->add(data->cases[i].str, nullptr);
    TCA* addr = table->find(data->cases[i].str);
    v << bindaddr{addr, data->cases[i].dest};
  }
  TCA* def = mcg->allocData<TCA>(sizeof(TCA), 1);
  v << bindaddr{def, data->defaultSk};
  cgCallHelper(v,
               CppCall::direct(sswitchHelperFast),
               callDest(inst),
               SyncOptions::kNoSyncPoint,
               argGroup(inst)
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

  for (int i = 0; i < data->numCases; ++i) {
    strtab[i] = data->cases[i].str;
    v << bindaddr{&jmptab[i], data->cases[i].dest};
  }
  v << bindaddr{&jmptab[data->numCases], data->defaultSk};
  cgCallHelper(v,
               CppCall::direct(sswitchHelperSlow),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup(inst)
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
  auto const callerSP = srcLoc(inst, 0).reg();
  auto const callerFP = srcLoc(inst, 2).reg();
  auto const fakeRet  = mcg->tx().uniqueStubs.retInlHelper;
  auto const extra    = inst->extra<DefInlineFP>();
  auto const retBCOff = extra->retBCOff;
  auto const offset   = cellsToBytes(extra->spOffset);
  auto& v = vmain();
  v << store{callerFP, callerSP[offset + AROFF(m_sfp)]};
  emitImmStoreq(v, intptr_t(fakeRet), callerSP[offset + AROFF(m_savedRip)]);
  v << storeli{retBCOff, callerSP[offset + AROFF(m_soff)]};
  v << lea{callerSP[offset], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgInlineReturn(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 0).reg();
  assertx(fpReg == rVmFp);
  vmain() << load{fpReg[AROFF(m_sfp)], rVmFp};
}

void CodeGenerator::cgReDefSP(IRInstruction* inst) {
  // TODO(#2288359): this instruction won't be necessary (for
  // non-generator frames) when we don't track rVmSp independently
  // from rVmFp.  In generator frames we'll have to track offsets from
  // a DefResumableSP or something similar.
  auto fp  = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  auto off = -cellsToBytes(inst->extra<ReDefSP>()->offset);
  vmain() << lea{fp[off], dst};
}

void CodeGenerator::cgResetSP(IRInstruction* inst) {
  auto fp  = srcLoc(inst, 0).reg();
  auto dst = dstLoc(inst, 0).reg();
  auto off = -cellsToBytes(inst->extra<ResetSP>()->offset);
  vmain() << lea{fp[off], dst};
}

void CodeGenerator::cgAdjustSP(IRInstruction* inst) {
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto const off  = inst->extra<AdjustSP>()->offset;
  vmain() << lea{rsrc[cellsToBytes(off.offset)], rdst};
}

void CodeGenerator::cgFreeActRec(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = AROFF(m_sfp);
  auto dst = dstLoc(inst, 0).reg();
  vmain() << load{ptr[off], dst};
}

void CodeGenerator::cgStMem(IRInstruction* inst) {
  emitStore(
    srcLoc(inst, 0).reg()[0],
    inst->src(1),
    srcLoc(inst, 1),
    Width::Full
  );
}

void CodeGenerator::cgStRef(IRInstruction* inst) {
  always_assert(!srcLoc(inst, 1).isFullSIMD());
  auto ptr = srcLoc(inst, 0).reg();
  auto off = RefData::tvOffset();
  emitStore(ptr[off], inst->src(1), srcLoc(inst, 1), Width::Full);
}

int CodeGenerator::iterOffset(const BCMarker& marker, uint32_t id) {
  const Func* func = getFunc(marker);
  return -cellsToBytes(((id + 1) * kNumIterCells + func->numLocals()));
}

void CodeGenerator::cgStLoc(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = localOffset(inst->extra<StLoc>()->locId);
  emitStore(ptr[off], inst->src(1), srcLoc(inst, 1), Width::Full);
}

void CodeGenerator::cgStLocNT(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = localOffset(inst->extra<StLocNT>()->locId);
  emitStore(ptr[off], inst->src(1), srcLoc(inst, 1), Width::Value);
}

void CodeGenerator::cgEagerSyncVMRegs(IRInstruction* inst) {
  auto& v = vmain();
  emitEagerSyncPoint(v, reinterpret_cast<const Op*>(inst->marker().sk().pc()),
                     rVmTl, srcLoc(inst, 0).reg(), srcLoc(inst, 1).reg());
}

void CodeGenerator::cgReqBindJmp(IRInstruction* inst) {
  auto dest    = inst->extra<ReqBindJmp>()->dest;
  auto trflags = inst->extra<ReqBindJmp>()->trflags;
  auto& v      = vmain();
  v << syncvmsp{srcLoc(inst, 0).reg()};
  v << bindjmp{dest, trflags, kCrossTraceRegs};
}

void CodeGenerator::cgReqRetranslateOpt(IRInstruction* inst) {
  auto const extra = inst->extra<ReqRetranslateOpt>();
  auto& v = vmain();
  v << syncvmsp{srcLoc(inst, 0).reg()};
  emitServiceReq(v, nullptr, REQ_RETRANSLATE_OPT, {
    {ServiceReqArgInfo::Immediate, {extra->sk.toAtomicInt()}},
    {ServiceReqArgInfo::Immediate, {static_cast<uint64_t>(extra->transId)}}
  });
}

void CodeGenerator::cgReqRetranslate(IRInstruction* inst) {
  auto const destSK = m_state.unit.initSrcKey();
  auto trflags = inst->extra<ReqRetranslate>()->trflags;
  auto& v = vmain();
  v << syncvmsp{srcLoc(inst, 0).reg()};
  v << fallback{destSK, trflags, kCrossTraceRegs};
}

void CodeGenerator::cgIncRef(IRInstruction* inst) {
  auto const ty = inst->src(0)->type();
  ifRefCountedNonStatic(
    vmain(), ty, srcLoc(inst, 0),
    [&] (Vout& v) {
      emitIncRef(v, srcLoc(inst, 0).reg());
    }
  );
}

void CodeGenerator::cgIncRefCtx(IRInstruction* inst) {
  if (inst->src(0)->isA(Type::Obj)) return cgIncRef(inst);

  auto const src = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();

  v << testbi{0x1, src, sf};
  ifThen(v, CC_Z, sf, [&](Vout& v) { emitIncRef(v, src); });
}

void CodeGenerator::cgDecRefThis(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto rthis = v.makeReg(); // Load AR->m_this into rthis
  v << load{fpReg[AROFF(m_this)], rthis};

  auto decrefIfAvailable = [&](Vout& v) {
    // Check if this is available and we're not in a static context instead
    auto const sf = v.makeReg();
    v << testbi{1, rthis, sf};
    ifThen(v, CC_Z, sf, [&](Vout& v) {
      cgDecRefStaticType(v, inst, Type::Obj, rthis);
    });
  };

  if (getFunc(inst->marker())->isPseudoMain()) {
    // In pseudo-mains, emit check for presence of m_this
    auto const sf = v.makeReg();
    v << testq{rthis, rthis, sf};
    ifThen(v, CC_NZ, sf, [&](Vout& v) { decrefIfAvailable(v); });
  } else {
    decrefIfAvailable(v);
  }
}

void CodeGenerator::cgGenericRetDecRefs(IRInstruction* inst) {
  auto const rFp       = srcLoc(inst, 0).reg();
  auto const numLocals = getFunc(inst->marker())->numLocals();
  auto& v = vmain();

  assertx(rFp == rVmFp &&
         "free locals helper assumes the frame pointer is rVmFp");

  if (numLocals == 0) return;

  // The helpers called below use a special ABI, in which r14 and r15 is
  // not saved, and the stub expects the stack to be imbalanced (RSP%16==0)
  // on entry. So use PhysRegSaverStub which assumes the odd stack parity.
  PhysRegSaverStub saver(v, RegSet());

  auto const target = numLocals > kNumFreeLocalsHelpers
    ? mcg->tx().uniqueStubs.freeManyLocalsHelper
    : mcg->tx().uniqueStubs.freeLocalsHelpers[numLocals - 1];

  auto const args = RegSet(r14) | RegSet(rVmFp);
  auto const kills = (abi.all() - abi.calleeSaved) | RegSet(r14) | RegSet(r15);

  auto& marker = inst->marker();
  auto const fix = Fixup {
    marker.bcOff() - marker.func()->base(),
    marker.spOff().offset
  };

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
bool CodeGenerator::decRefDestroyIsUnlikely(const IRInstruction* inst,
                                            OptDecRefProfile& profile,
                                            Type type) {
  auto const kind = mcg->tx().mode();
  if (kind != TransKind::Profile && kind != TransKind::Optimize) return true;

  auto const profileKey =
    makeStaticString(folly::to<std::string>("DecRefProfile-",
                                            opcodeName(inst->op()),
                                            '-',
                                            type.toString()));
  profile.emplace(m_state.unit.context(), inst->marker(), profileKey);

  auto& v = vmain();
  if (profile->profiling()) {
    v << incwm{rVmTl[profile->handle() + offsetof(DecRefProfile, decrement)],
               v.makeReg()};
  } else if (profile->optimizing()) {
    auto const data = profile->data(DecRefProfile::reduce);
    if (data.hitRate() != 0 && data.hitRate() != 100) {
      // These are the only interesting cases where we could be doing better.
      FTRACE(5, "DecRefProfile: {}: {} {}\n",
             data, inst->marker().show(), profileKey->data());
    }
    if (data.hitRate() == 0) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_0);
    } else if (data.hitRate() == 100) {
      emitIncStat(v, Stats::TC_DecRef_Profiled_100);
    }
    return data.hitRate() < RuntimeOption::EvalJitUnlikelyDecRefPercent;
  }

  return true;
}

//
// Using the given dataReg, this method generates code that checks the static
// bit out of dataReg, and emits a DecRef if needed.
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
template <typename F> void
CodeGenerator::cgCheckStaticBitAndDecRef(Vout& v, const IRInstruction* inst,
                                         Vlabel done, Type type,
                                         Vreg dataReg, F destroyImpl) {
  always_assert(type.maybe(Type::Counted));

  OptDecRefProfile profile;
  auto const unlikelyDestroy = decRefDestroyIsUnlikely(inst, profile, type);

  emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Decl
                                 : Stats::TC_DecRef_Likely_Decl);

  Vreg sf;
  auto destroy = [&](Vout& v) {
    emitIncStat(v, unlikelyDestroy ? Stats::TC_DecRef_Normal_Destroy :
                   Stats::TC_DecRef_Likely_Destroy);
    if (profile && profile->profiling()) {
      v << incwm{rVmTl[profile->handle() + offsetof(DecRefProfile, destroy)],
                 v.makeReg()};
    }
    destroyImpl(v);
  };

  if (!type.maybe(Type::Static)) {
    sf = v.makeReg();
    v << declm{dataReg[FAST_REFCOUNT_OFFSET], sf};
    emitAssertFlagsNonNegative(v, sf);
    ifBlock(v, vcold(), CC_E, sf, destroy, unlikelyDestroy);
    return;
  }

  auto static_check_and_decl = [&](Vout& v) {
    auto next = v.makeBlock();
    assertx(sf != InvalidReg);
    v << jcc{CC_L, sf, {next, done}};
    v = next;

    // Decrement _count
    sf = v.makeReg();
    v << declm{dataReg[FAST_REFCOUNT_OFFSET], sf};
    emitAssertFlagsNonNegative(v, sf);
  };

  sf = v.makeReg();
  v << cmplim{1, dataReg[FAST_REFCOUNT_OFFSET], sf};
  ifThenElse(v, vcold(), CC_E, sf, destroy, static_check_and_decl,
             unlikelyDestroy);
}

//
// Generates dec-ref of a typed value with statically known type.
//
void CodeGenerator::cgDecRefStaticType(Vout& v,
                                       const IRInstruction* inst,
                                       Type type, Vreg dataReg) {
  assertx(type != Type::Cell && type != Type::Gen);
  assertx(type.isKnownDataType());

  if (!type.maybe(Type::Counted)) return;

  auto done = v.makeBlock();
  cgCheckStaticBitAndDecRef(v, inst, done, type, dataReg, [&] (Vout& v) {
    cgCallHelper(v,
                 mcg->getDtorCall(type.toDataType()),
                 kVoidDest,
                 SyncOptions::kSyncPoint,
                 argGroup(inst)
                 .reg(dataReg));
  });
  if (!v.closed()) v << jmp{done};
  v = done;
}

void CodeGenerator::cgDecRef(IRInstruction *inst) {
  auto const ty   = inst->src(0)->type();
  auto const base = srcLoc(inst, 0).reg(0);

  auto& v = vmain();
  auto const done = v.makeBlock();

  ifRefCountedType(
    v, ty, srcLoc(inst, 0),
    [&] (Vout& v) {
      cgCheckStaticBitAndDecRef(
        v, inst, done, ty,
        base,
        [&] (Vout& v) {
          cgCallHelper(
            v,
            ty.isKnownDataType()
              ? mcg->getDtorCall(ty.toDataType())
              : CppCall::destruct(srcLoc(inst, 0).reg(1)),
            kVoidDest,
            SyncOptions::kSyncPoint,
            argGroup(inst)
              .reg(base)
          );
        }
      );
    }
  );
  if (!v.closed()) v << jmp{done};
  v = done;
}

void CodeGenerator::cgDecRefNZ(IRInstruction* inst) {
  emitIncStat(vmain(), Stats::TC_DecRef_NZ);
  auto const ty = inst->src(0)->type();
  ifRefCountedNonStatic(
    vmain(), ty, srcLoc(inst, 0),
    [&] (Vout& v) {
      emitDecRef(v, srcLoc(inst, 0).reg());
    }
  );
}

void CodeGenerator::cgCufIterSpillFrame(IRInstruction* inst) {
  auto const nArgs = inst->extra<CufIterSpillFrame>()->args;
  auto const iterId = inst->extra<CufIterSpillFrame>()->iterId;
  auto const itOff = iterOffset(inst->marker(), iterId);

  auto const spOffset = cellsToBytes(
    inst->extra<CufIterSpillFrame>()->spOffset.offset);
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
  auto name = v.makeReg();
  v << load{fpReg[itOff + CufIter::nameOff()], name};
  auto const sf = v.makeReg();
  v << testq{name, name, sf};
  ifThenElse(v, CC_NZ, sf, [&](Vout& v) {
    auto const sf = v.makeReg();
    v << cmplim{0, name[FAST_REFCOUNT_OFFSET], sf};
    static_assert(UncountedValue < 0 && StaticValue < 0, "");
    ifThen(v, CC_NS, sf, [&](Vout& v) { emitIncRef(v, name); });
    auto name2 = v.makeReg();
    v << orqi{ActRec::kInvNameBit, name, name2, v.makeReg()};
    v << store{name2, spReg[spOffset + int(AROFF(m_invName))]};
  }, [&](Vout& v) {
    v << store{name, spReg[spOffset + int(AROFF(m_invName))]};
  });
  v << storeli{safe_cast<int32_t>(nArgs),
               spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
}

void CodeGenerator::cgSpillFrame(IRInstruction* inst) {
  auto const func      = inst->src(1);
  auto const objOrCls  = inst->src(2);
  auto const magicName = inst->extra<SpillFrame>()->invName;
  auto const nArgs     = inst->extra<SpillFrame>()->numArgs;
  auto& v              = vmain();

  auto const spOffset = cellsToBytes(
    inst->extra<SpillFrame>()->spOffset.offset);

  auto spReg = srcLoc(inst, 0).reg();
  // actRec->m_this
  if (objOrCls->isA(Type::Cls)) {
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
  } else if (objOrCls->isA(Type::Obj)) {
    // store this pointer
    v << store{srcLoc(inst, 2/*objOrCls*/).reg(),
               spReg[spOffset + int(AROFF(m_this))]};
  } else if (objOrCls->isA(Type::Ctx)) {
    // Stores either a this pointer or a Cctx -- statically unknown.
    auto objOrClsPtrReg = srcLoc(inst, 2/*objOrCls*/).reg();
    v << store{objOrClsPtrReg, spReg[spOffset + int(AROFF(m_this))]};
  } else {
    assertx(objOrCls->isA(Type::Nullptr));
    // no obj or class; this happens in FPushFunc
    int offset_m_this = spOffset + int(AROFF(m_this));
    v << storeqi{0, spReg[offset_m_this]};
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
  } else if (func->hasConstVal()) {
    const Func* f = func->funcVal();
    emitImmStoreq(v, intptr_t(f), spReg[spOffset + int(AROFF(m_func))]);
  } else {
    int offset_m_func = spOffset + int(AROFF(m_func));
    auto funcLoc = srcLoc(inst, 1);
    v << store{funcLoc.reg(0), spReg[offset_m_func]};
  }

  v << storeli{nArgs, spReg[spOffset + int(AROFF(m_numArgsAndFlags))]};
}

void CodeGenerator::cgStClosureFunc(IRInstruction* inst) {
  auto const obj  = srcLoc(inst, 0).reg();
  auto const func = inst->extra<StClosureFunc>()->func;
  emitImmStoreq(vmain(), intptr_t(func), obj[c_Closure::funcOffset()]);
}

void CodeGenerator::cgStClosureArg(IRInstruction* inst) {
  emitStore(
    srcLoc(inst, 0).reg()[inst->extra<StClosureArg>()->offsetBytes],
    inst->src(1), srcLoc(inst, 1),
    Width::Full
  );
}

void CodeGenerator::cgStClosureCtx(IRInstruction* inst) {
  auto const obj = srcLoc(inst, 0).reg();
  auto& v = vmain();
  if (inst->src(1)->isA(Type::Nullptr)) {
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
      if (!IS_NULL_TYPE(cls->declPropInit()[i].m_type)) {
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
               CppCall::direct(memcpy),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               args);
}

void CodeGenerator::cgConstructInstance(IRInstruction* inst) {
  auto const cls    = inst->extra<ConstructInstance>()->cls;
  auto const dstReg = dstLoc(inst, 0).reg();
  cgCallHelper(vmain(),
               CppCall::direct(cls->instanceCtor().get()),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
               argGroup(inst).immPtr(cls));
}

void CodeGenerator::cgCheckInitProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitProps>()->cls;
  auto const branch = inst->taken();
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmpqim{0, rVmTl[cls->propHandle()], sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(branch)}};
}

void CodeGenerator::cgCheckInitSProps(IRInstruction* inst) {
  auto const cls = inst->extra<CheckInitSProps>()->cls;
  auto const branch = inst->taken();
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << cmpbim{0, rVmTl[cls->sPropInitHandle()], sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(branch)}};
}

void CodeGenerator::cgNewInstanceRaw(IRInstruction* inst) {
  auto const cls    = inst->extra<NewInstanceRaw>()->cls;
  auto const dstReg = dstLoc(inst, 0).reg();
  size_t size = ObjectData::sizeForNProps(cls->numDeclProperties());
  cgCallHelper(vmain(),
               size <= kMaxSmartSize
               ? CppCall::direct(ObjectData::newInstanceRaw)
               : CppCall::direct(ObjectData::newInstanceRawBig),
               callDest(dstReg),
               SyncOptions::kSyncPoint,
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
      // Load the Class's propInitVec from the targetcache
      v << load{rVmTl[cls->propHandle()], propInitVec};
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
                     CppCall::direct(memcpy),
                     kVoidDest,
                     SyncOptions::kNoSyncPoint,
                     args);
      } else {
        auto args = argGroup(inst)
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
  auto& v = vmain();
  auto const extra  = inst->extra<CallArray>();
  auto const pc     = v.cns(extra->pc);
  auto const after  = v.cns(extra->after);
  auto const target = mcg->tx().uniqueStubs.fcallArrayHelper;
  auto const rSP    = srcLoc(inst, 0 /* sp */).reg();
  auto const syncSP = v.makeReg();
  v << lea{rSP[cellsToBytes(extra->spOffset.offset)], syncSP};
  v << syncvmsp{syncSP};
  v << copy2{pc, after, argNumToRegName[0], argNumToRegName[1]};

  auto done = v.makeBlock();
  v << callstub{target, argSet(2) | kCrossTraceRegs,
                x64::abi.all(), makeFixup(inst->marker())};
  v << unwind{{done, m_state.labels[inst->taken()]}};
  v = done;
  v << defvmsp{dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgCall(IRInstruction* inst) {
  auto const rSP    = srcLoc(inst, 0).reg();
  auto const rFP    = srcLoc(inst, 1).reg();
  auto const rNewSP = dstLoc(inst, 0).reg();
  auto const extra  = inst->extra<Call>();
  auto const callee = extra->callee;
  auto const argc = extra->numParams;
  auto const rds = rVmTl;
  auto& v = vmain();
  auto& vc = vcold();

  auto const ar = argc * sizeof(TypedValue);
  v << store{rFP, rSP[cellsToBytes(extra->spOffset.offset) +
    ar + AROFF(m_sfp)]};
  v << storeli{safe_cast<int32_t>(extra->after),
               rSP[cellsToBytes(extra->spOffset.offset) + ar + AROFF(m_soff)]};

  auto const sync_sp = v.makeReg();
  v << lea{rSP[cellsToBytes(extra->spOffset.offset)], sync_sp};

  auto catchBlock = m_state.labels[inst->taken()];
  if (isNativeImplCall(callee, argc)) {
    // The assumption here is that for builtins, the generated func contains
    // only a single opcode (NativeImpl), and there are no non-argument locals.
    assertx(argc == callee->numLocals() && callee->numIterators() == 0);
    assertx(*reinterpret_cast<const Op*>(callee->getEntry()) == Op::NativeImpl);
    assertx(instrLen((Op*)callee->getEntry()) == callee->past()-callee->base());
    auto retAddr = (int64_t)mcg->tx().uniqueStubs.retHelper;
    v << store{v.cns(retAddr),
               sync_sp[cellsToBytes(argc) + AROFF(m_savedRip)]};
    v << lea{sync_sp[cellsToBytes(argc)], rVmFp};
    emitCheckSurpriseFlagsEnter(v, vc, rds, Fixup(0, argc), catchBlock);
    BuiltinFunction builtinFuncPtr = callee->builtinFuncPtr();
    TRACE(2, "calling builtin preClass %p func %p\n", callee->preClass(),
          builtinFuncPtr);
    // We sometimes call this while curFunc() isn't really the builtin, so
    // make sure to record the sync point as if we are inside the builtin.
    if (mcg->fixupMap().eagerRecord(callee)) {
      emitEagerSyncPoint(v, reinterpret_cast<const Op*>(callee->getEntry()),
                         rds, rVmFp, sync_sp);
    }
    // Call the native implementation. This will free the locals for us in the
    // normal case. In the case where an exception is thrown, the VM unwinder
    // will handle it for us.
    auto next = v.makeBlock();
    v << vinvoke{CppCall::direct(builtinFuncPtr), v.makeVcallArgs({{rVmFp}}),
                 v.makeTuple({}), {next, catchBlock}, Fixup(0, argc)};
    v = next;
    // The native implementation already put the return value on the stack for
    // us, and handled cleaning up the arguments.  We have to update the frame
    // pointer and the stack pointer, and load the return value into the return
    // register so the trace we are returning to has it where it expects.
    // TODO(#1273094): we should probably modify the actual builtins to return
    // values via registers using the C ABI and do a reg-to-reg move.
    int nLocalCells = callee->numSlotsInFrame();
    v << load{rVmFp[AROFF(m_sfp)], rVmFp};
    emitRB(v, Trace::RBTypeFuncExit, callee->fullName()->data());
    auto adjust = safe_cast<int>(sizeof(ActRec) + cellsToBytes(nLocalCells-1));
    v << lea{sync_sp[adjust], rNewSP};
  } else {
    // Emit a smashable call that initially calls a recyclable service request
    // stub. The stub and the eventual targets take rStashedAR as an argument.
    auto& us = mcg->tx().uniqueStubs;
    auto addr = callee ? us.immutableBindCallStub : us.bindCallStub;
    v << syncvmsp{sync_sp};
    v << lea{sync_sp[cellsToBytes(argc)], rStashedAR};
    if (debug && RuntimeOption::EvalHHIRGenerateAsserts) {
      emitImmStoreq(v, kUninitializedRIP, rStashedAR[AROFF(m_savedRip)]);
    }
    auto next = v.makeBlock();
    v << bindcall{addr, kCrossCallRegs, {next, catchBlock}};
    v = next;
    v << defvmsp{rNewSP};
  }
}

void CodeGenerator::cgCastStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  auto offset     = inst->extra<CastStk>()->offset;
  auto spReg      = srcLoc(inst, 0).reg();
  auto args = argGroup(inst);
  args.addr(spReg, cellsToBytes(offset.offset));

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

void CodeGenerator::cgCoerceStk(IRInstruction *inst) {
  Type type       = inst->typeParam();
  auto extra      = inst->extra<CoerceStk>();
  auto spReg      = srcLoc(inst, 0).reg();
  auto offset     = cellsToBytes(extra->offset.offset);

  auto& v = vmain();

  // Short-circuit call to tvCoerceParamTo*()
  // if we're already of the appropriate type (common case)
  auto const coerceTypeTest = [&](const Type& t) {
    auto const sf = v.makeReg();
    emitTypeTest(t,
                 spReg[offset + TVOFF(m_type)],
                 spReg[offset + TVOFF(m_data)], sf,
                 [&](ConditionCode cc, Vreg sfTaken) {
                   auto next = v.makeBlock();
                   v << jcc{ccNegate(cc), sf, {label(inst->next()), next}};
                   v = next;
                 });
  };
  if (!type.isKnownDataType()) {
    assertx(Type::Null <= type);
    coerceTypeTest(Type::Null);
    type -= Type::Null;
    assertx(type.isKnownDataType());
  }
  coerceTypeTest(type);

  // If the type-specific test(s) failed,
  // fallback on actually calling the tvCoerceParamTo*() helper
  auto args = argGroup(inst);
  args.addr(spReg, offset);
  args.imm(extra->callee);
  args.imm(extra->argNum);

  TCA tvCoerceHelper;
  if (type <= Type::Bool) {
    tvCoerceHelper = (TCA)tvCoerceParamToBooleanOrThrow;
  } else if (type <= Type::Int) {
    tvCoerceHelper = (TCA)tvCoerceParamToInt64OrThrow;
  } else if (type <= Type::Dbl) {
    tvCoerceHelper = (TCA)tvCoerceParamToDoubleOrThrow;
  } else if (type <= Type::Arr) {
    tvCoerceHelper = (TCA)tvCoerceParamToArrayOrThrow;
  } else if (type <= Type::Str) {
    tvCoerceHelper = (TCA)tvCoerceParamToStringOrThrow;
  } else if (type <= Type::Obj) {
    tvCoerceHelper = (TCA)tvCoerceParamToObjectOrThrow;
  } else if (type <= Type::Res) {
    tvCoerceHelper = (TCA)tvCoerceParamToResourceOrThrow;
  } else {
    not_reached();
  }

  cgCallHelper(v,
    CppCall::direct(reinterpret_cast<void (*)()>(tvCoerceHelper)),
    kVoidDest,
    SyncOptions::kSyncPoint,
    args
  );
}

void CodeGenerator::cgCallBuiltin(IRInstruction* inst) {
  auto const dst            = dstLoc(inst, 0);
  auto const dstReg         = dst.reg(0);
  auto const dstType        = dst.reg(1);
  auto const callee         = inst->extra<CallBuiltin>()->callee;
  auto const numArgs        = callee->numParams();
  auto const returnType     = inst->typeParam();
  auto const funcReturnType = callee->returnType();
  auto& v = vmain();

  int returnOffset = rds::kVmMInstrStateOff +
    offsetof(MInstrState, tvBuiltinReturn);

  if (FixupMap::eagerRecord(callee)) {
    auto const rSP       = srcLoc(inst, 1).reg();
    auto const spOffset  = cellsToBytes(
      inst->extra<CallBuiltin>()->spOffset.offset);
    auto const& marker   = inst->marker();
    auto const pc        = getUnit(marker)->entry() + marker.bcOff();
    auto const synced_sp = v.makeReg();
    v << lea{rSP[spOffset], synced_sp};
    emitEagerSyncPoint(
      v,
      reinterpret_cast<const Op*>(pc),
      rVmTl,
      srcLoc(inst, 0).reg(),
      synced_sp
    );
  }

  // The MInstrState we need to use is at a constant offset from the base of
  // the RDS header.
  PhysReg rdsReg(rVmTl);

  auto callArgs = argGroup(inst);
  if (isBuiltinByRef(funcReturnType)) {
    // First arg is pointer to storage for that return value
    if (isSmartPtrRef(funcReturnType)) {
      returnOffset += TVOFF(m_data);
    }
    // Pass the address of tvBuiltinReturn to the native function as the
    // location it can construct the return Array, String, Object, or Variant.
    callArgs.addr(rdsReg, returnOffset); // &rdsReg[returnOffset]
  }

  // Non-pointer args are plain values passed by value.  String, Array,
  // Object, and Variant are passed by const&, ie a pointer to stack memory
  // holding the value, so expect PtrToT types for these.
  // Pointers to smartptr types (String, Array, Object) need adjusting to
  // point to &ptr->m_data.
  auto srcNum = uint32_t{2};
  if (callee->isMethod()) {
    if (callee->isStatic()) {
      // This isn't entirely accurate.  HNI functions expect the Class*
      // of the class used for the call which may be callee->cls() or
      // one of its children. Currently we don't support FCallBuiltin on
      // these functions (disabled in inlining-decider.cpp); (t5360661)
      if (callee->isNative()) {
        callArgs.imm(callee->cls());
      }
    } else {
      // Note, we don't support objects with vtables here (if they may
      // need a this pointer adjustment).  This should be filtered out
      // earlier right now.
      callArgs.ssa(srcNum);
      ++srcNum;
    }
  }
  for (uint32_t i = 0; i < numArgs; ++i, ++srcNum) {
    auto const& pi = callee->params()[i];
    if (TVOFF(m_data) && isSmartPtrRef(pi.builtinType)) {
      assertx(inst->src(srcNum)->type() <= Type::PtrToGen);
      callArgs.addr(srcLoc(inst, srcNum).reg(), TVOFF(m_data));
    } else {
      callArgs.ssa(srcNum, pi.builtinType == KindOfDouble);
    }
  }

  // If the return value is returned by reference, we don't need the
  // return value from this call since we know where the value is.
  auto dest = isBuiltinByRef(funcReturnType) ? kVoidDest :
              funcReturnType == KindOfDouble ? callDestDbl(inst) :
              callDest(inst);
  cgCallHelper(v, CppCall::direct(callee->nativeFuncPtr()),
               dest, SyncOptions::kSyncPoint, callArgs);

  // For primitive return types (int, bool, double), the return value
  // is already in dstReg (the builtin call returns in rax or xmm0).
  if (returnType.isSimpleType()) {
    return;
  }

  // For return by reference (String, Object, Array, Variant),
  // the builtin writes the return value into MInstrState::tvBuiltinReturn
  // TV, from where it has to be tested and copied.
  if (returnType.isReferenceType()) {
    assertx(isBuiltinByRef(funcReturnType) && isSmartPtrRef(funcReturnType));
    // return type is String, Array, or Object; fold nullptr to KindOfNull
    auto rtype = v.cns(returnType.toDataType());
    auto nulltype = v.cns(KindOfNull);
    v << load{rdsReg[returnOffset], dstReg};
    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      v << testq{dstReg, dstReg, sf};
      v << cmovq{CC_Z, sf, rtype, nulltype, dstType};
    }
    return;
  }
  if (returnType <= Type::Cell || returnType <= Type::BoxedCell) {
    // return type is Variant; fold KindOfUninit to KindOfNull
    assertx(isBuiltinByRef(funcReturnType) && !isSmartPtrRef(funcReturnType));
    auto nulltype = v.cns(KindOfNull);
    auto tmp_type = v.makeReg();
    emitLoadTVType(v, rdsReg[returnOffset + TVOFF(m_type)], tmp_type);
    v << load{rdsReg[returnOffset + TVOFF(m_data)], dstReg};
    static_assert(KindOfUninit == 0, "KindOfUninit must be 0 for test");
    if (dstType.isValid()) {
      auto const sf = v.makeReg();
      v << testb{tmp_type, tmp_type, sf};
      v << cmovq{CC_Z, sf, tmp_type, nulltype, dstType};
    }
    return;
  }
  not_reached();
}

void CodeGenerator::cgStStk(IRInstruction* inst) {
  auto const spReg = srcLoc(inst, 0).reg();
  auto const offset = cellsToBytes(inst->extra<StStk>()->offset.offset);
  emitStore(spReg[offset], inst->src(1), srcLoc(inst, 1), Width::Full);
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

void CodeGenerator::cgNativeImpl(IRInstruction* inst) {
  auto const func = getFunc(inst->marker());
  auto const builtinFuncPtr = func->builtinFuncPtr();
  auto& v = vmain();
  auto fp = srcLoc(inst, 0).reg();
  auto sp = srcLoc(inst, 1).reg();

  if (FixupMap::eagerRecord(func)) {
    emitEagerSyncPoint(v, reinterpret_cast<const Op*>(func->getEntry()),
                       rVmTl, fp, sp);
  }
  v << vinvoke{
    CppCall::direct(builtinFuncPtr),
    v.makeVcallArgs({{fp}}),
    v.makeTuple({}),
    {m_state.labels[inst->next()], m_state.labels[inst->taken()]},
    makeFixup(inst->marker(), SyncOptions::kSyncPoint)
  };
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
  v << testbi{1, rctx, sf};
  v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgLdClsCtx(IRInstruction* inst) {
  auto srcReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  // Context could be either a this object or a class ptr
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << testbi{1, srcReg, sf};
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
  auto const offset = cellsToBytes(inst->extra<LdARFuncPtr>()->offset);
  auto dstReg       = dstLoc(inst, 0).reg();
  auto baseReg      = srcLoc(inst, 0).reg();
  vmain() << load{baseReg[offset + AROFF(m_func)], dstReg};
}

void CodeGenerator::cgLdStaticLocCached(IRInstruction* inst) {
  auto const extra = inst->extra<LdStaticLocCached>();
  auto const link  = rds::bindStaticLocal(extra->func, extra->name);
  auto const dst   = dstLoc(inst, 0).reg();
  vmain() << lea{rVmTl[link.handle()], dst};
}

void CodeGenerator::cgCheckStaticLocInit(IRInstruction* inst) {
  auto const src = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, src[RefData::tvOffset() + TVOFF(m_type)]);
  v << jcc{CC_E, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgStaticLocInitCached(IRInstruction* inst) {
  auto const rdSrc = srcLoc(inst, 0).reg();
  auto& v = vmain();

  // If we're here, the target-cache-local RefData is all zeros, so we
  // can initialize it by storing the new value into it's TypedValue
  // and incrementing the RefData reference count (which will set it
  // to 1).
  //
  // We are storing the rdSrc value into the static, but we don't need
  // to inc ref it because it's a bytecode invariant that it's not a
  // reference counted type.
  emitStore(rdSrc[RefData::tvOffset()], inst->src(1), srcLoc(inst, 1),
            Width::Full);
  v << inclm{rdSrc[FAST_REFCOUNT_OFFSET], v.makeReg()};
  v << storebi{uint8_t(HeaderKind::Ref), rdSrc[HeaderKindOffset]};
  static_assert(sizeof(HeaderKind) == 1, "");
}

void CodeGenerator::emitStoreTypedValue(Vptr dst, SSATmp* src, Vloc loc) {
  assertx(src->type().needsReg());
  auto srcReg0 = loc.reg(0);
  auto srcReg1 = loc.reg(1);
  auto& v = vmain();
  if (loc.isFullSIMD()) {
    // Whole typed value is stored in single SIMD reg srcReg0
    assertx(RuntimeOption::EvalHHIRAllocSIMDRegs);
    assertx(!srcReg1.isValid());
    v << storedqu{srcReg0, refTVData(dst)};
    return;
  }

  if (src->type().needsValueReg()) {
    v << store{srcReg0, refTVData(dst)};
  }
  assertx(srcReg1.isValid());            // a precondition for this call
  emitStoreTVType(v, srcReg1, refTVType(dst));
}

void CodeGenerator::emitStore(Vptr dst,
                              SSATmp* src,
                              Vloc srcLoc,
                              Width width) {
  Type type = src->type();
  if (type.needsReg()) {
    always_assert(width == Width::Full);
    emitStoreTypedValue(dst, src, srcLoc);
    return;
  }
  auto& v = vmain();
  if (width == Width::Full) {
    emitStoreTVType(v, type.toDataType(), refTVType(dst));
  }
  if (!src->type().needsValueReg()) return; // no value to store

  auto memRef = refTVData(dst);
  auto srcReg = srcLoc.reg();
  if (src->hasConstVal()) {
    always_assert(type <= (Type::Bool | Type::Int | Type::Dbl |
                  Type::Arr | Type::StaticStr | Type::Cls));
    emitImmStoreq(v, src->rawVal(), memRef);
  } else {
    auto s2 = zeroExtendIfBool(v, src, srcReg);
    v << store{s2, memRef};
  }
}

void CodeGenerator::emitLoad(SSATmp* dst, Vloc dstLoc, Vptr base) {
  auto const type = dst->type();
  if (type.needsReg()) {
    return emitLoadTypedValue(dst, dstLoc, base);
  }
  auto const dstReg = dstLoc.reg();
  if (type <= Type::Bool) {
    vmain() << loadtqb{refTVData(base), dstReg};
  } else {
    vmain() << load{refTVData(base), dstReg};
  }
}

void CodeGenerator::emitLoadTypedValue(SSATmp* dst, Vloc dstLoc, Vptr ref) {
  auto const valueDstReg = dstLoc.reg(0);
  auto& v = vmain();
  if (dstLoc.isFullSIMD()) {
    // Whole typed value is stored in single SIMD reg valueDstReg
    v << loaddqu{refTVData(ref), valueDstReg};
    return;
  }
  auto const typeDstReg = dstLoc.reg(1);
  if (typeDstReg.isValid()) {
    emitLoadTVType(v, refTVType(ref), typeDstReg);
  }
  v << load{refTVData(ref), valueDstReg};
}

void CodeGenerator::cgLdContField(IRInstruction* inst) {
  emitLoad(inst->dst(), dstLoc(inst, 0),
           srcLoc(inst, 0).reg()[inst->src(1)->intVal()]);
}

void CodeGenerator::cgLdMem(IRInstruction* inst) {
  emitLoad(inst->dst(), dstLoc(inst, 0), srcLoc(inst, 0).reg()[0]);
}

void CodeGenerator::cgLdRef(IRInstruction* inst) {
  emitLoad(inst->dst(), dstLoc(inst, 0),
           srcLoc(inst, 0).reg()[RefData::tvOffset()]);
}

void CodeGenerator::cgCheckRefInner(IRInstruction* inst) {
  if (inst->typeParam() >= Type::InitCell) return;
  auto const base = srcLoc(inst, 0).reg()[RefData::tvOffset()];
  emitTypeCheck(inst->typeParam(), refTVType(base), refTVData(base),
    inst->taken());
}

void CodeGenerator::cgStringIsset(IRInstruction* inst) {
  auto strReg = srcLoc(inst, 0).reg();
  auto idxReg = srcLoc(inst, 1).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto const idxTrunc = v.makeReg();
  v << movtql{idxReg, idxTrunc};
  auto const sf = v.makeReg();
  v << cmplm{idxTrunc, strReg[StringData::sizeOff()], sf};
  v << setcc{CC_NBE, sf, dstReg};
}

void CodeGenerator::cgProfilePackedArray(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto handle  = inst->extra<ProfilePackedArray>()->handle;
  auto& v = vmain();

  // If kPackedKind changes to a value that is not 0, change
  // this to a conditional add.
  static_assert(ArrayData::ArrayKind::kPackedKind == 0, "kPackedKind changed");
  auto tmp_kind = v.makeReg();
  auto const sf = v.makeReg();
  v << loadzbl{baseReg[ArrayData::offsetofKind()], tmp_kind};
  v << addlm{tmp_kind, rVmTl[handle + offsetof(NonPackedArrayProfile, count)],
             sf};
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
  v << cmpbim{ArrayData::kStructKind, baseReg[ArrayData::offsetofKind()], sf0};
  v << jcc{CC_E, sf0, {notStruct, isStruct}};

  auto const shape = v.makeReg();
  auto const sf1 = v.makeReg();
  v = isStruct;
  v << load{baseReg[StructArray::shapeOffset()], shape};
  v << cmpqm{shape, rVmTl[handle + offsetof(StructArrayProfile, shape)], sf1};
  v << jcc{CC_E, sf1, {shapeIsDifferent, done}};

  v = shapeIsDifferent;
  v << addlm{v.cns(uint32_t{1}),
             rVmTl[handle + offsetof(StructArrayProfile, numShapesSeen)],
             v.makeReg()};
  v << store{shape, rVmTl[handle + offsetof(StructArrayProfile, shape)]};
  v << jmp{done};

  v = notStruct;
  v << addlm{v.cns(uint32_t{1}),
             rVmTl[handle + offsetof(StructArrayProfile, nonStructCount)],
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
   * logic of `scaaledIdx * 16` is split in the following two instructions, in
   * order to save a byte in the shl instruction.
   */
  auto scaledIdx = v.makeReg();
  v << shlli{1, rIdx, scaledIdx, v.makeReg()};
  v << lea{Vptr{rArr, scaledIdx, 8, 16}, dst};
}

void CodeGenerator::cgCheckBounds(IRInstruction* inst) {
  auto idx = inst->src(0);
  auto idxReg = srcLoc(inst, 0).reg();
  auto sizeReg = srcLoc(inst, 1).reg();

  auto throwHelper = [&](Vout& v) {
    auto args = argGroup(inst);
    args.ssa(0/*idx*/);
    cgCallHelper(v, CppCall::direct(throwOOB),
                 kVoidDest, SyncOptions::kSyncPoint, args);
  };

  auto& v = vmain();
  if (idx->hasConstVal()) {
    auto const sf = v.makeReg();
    v << cmpq{idxReg, sizeReg, sf};
    unlikelyIfBlock(v, vcold(), CC_BE, sf, throwHelper);
    return;
  }

  auto const sf = v.makeReg();
  v << cmpq{sizeReg, idxReg, sf};
  unlikelyIfBlock(v, vcold(), CC_AE, sf, throwHelper);
}

void CodeGenerator::cgLdVectorSize(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  assertx(vec->type() < Type::Obj &&
         vec->type().clsSpec().cls() == c_Vector::classof());
  vmain() << loadzlq{vecReg[c_Vector::sizeOffset()], dstReg};
}

void CodeGenerator::cgLdVectorBase(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  assertx(vec->type() < Type::Obj &&
         vec->type().clsSpec().cls() == c_Vector::classof());
  vmain() << load{vecReg[c_Vector::dataOffset()], dstReg};
}

void CodeGenerator::cgLdColArray(IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const cls = src->type().clsSpec().cls();
  auto const rsrc = srcLoc(inst, 0).reg();
  auto const rdst = dstLoc(inst, 0).reg();
  auto& v = vmain();

  if (cls == c_Vector::classof()) {
    auto const rdata = v.makeReg();
    v << load{rsrc[c_Vector::dataOffset()], rdata};
    v << lea{rdata[-int32_t{sizeof(ArrayData)}], rdst};
    return;
  }

  if (cls == c_Map::classof()) {
    auto const rdata = v.makeReg();
    v << load{rsrc[HashCollection::dataOffset()], rdata};
    v << lea{rdata[-int32_t{sizeof(MixedArray)}], rdst};
    return;
  }

  always_assert_flog(0, "LdColArray received an unsupported type: {}\n",
    src->type().toString());
}

void CodeGenerator::cgVectorHasImmCopy(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  auto vecReg = srcLoc(inst, 0).reg();
  auto& v = vmain();

  assertx(vec->type() < Type::Obj &&
         vec->type().clsSpec().cls() == c_Vector::classof());

  // Vector::m_data field holds an address of an ArrayData plus
  // sizeof(ArrayData) bytes. We need to check this ArrayData's
  // m_count field to see if we need to call Vector::triggerCow().
  auto rawPtrOffset = c_Vector::dataOffset() + kExpectedMPxOffset;
  auto countOffset = (int64_t)FAST_REFCOUNT_OFFSET - (int64_t)sizeof(ArrayData);

  auto ptr = v.makeReg();
  v << load{vecReg[rawPtrOffset], ptr};
  auto const sf = v.makeReg();
  v << cmplim{1, ptr[countOffset], sf};
  v << jcc{CC_NE, sf, {label(inst->next()), label(inst->taken())}};
}

/**
 * Given the base of a vector object, pass it to a helper
 * which is responsible for triggering COW.
 */
void CodeGenerator::cgVectorDoCow(IRInstruction* inst) {
  DEBUG_ONLY auto vec = inst->src(0);
  assertx(vec->type() < Type::Obj &&
         vec->type().clsSpec().cls() == c_Vector::classof());
  auto args = argGroup(inst);
  args.ssa(0); // vec
  cgCallHelper(vmain(), CppCall::direct(triggerCow),
               kVoidDest, SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgLdPairBase(IRInstruction* inst) {
  DEBUG_ONLY auto pair = inst->src(0);
  auto pairReg = srcLoc(inst, 0).reg();
  assertx(pair->type() < Type::Obj &&
         pair->type().clsSpec().cls() == c_Pair::classof());
  vmain() << lea{pairReg[c_Pair::dataOffset()], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLdElem(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto idx = inst->src(1);
  auto idxReg = srcLoc(inst, 1).reg();
  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    emitLoad(inst->dst(), dstLoc(inst, 0), baseReg[idx->intVal()]);
  } else {
    emitLoad(inst->dst(), dstLoc(inst, 0), baseReg[idxReg]);
  }
}

void CodeGenerator::cgStElem(IRInstruction* inst) {
  auto baseReg = srcLoc(inst, 0).reg();
  auto srcValue = inst->src(2);
  auto idx = inst->src(1);
  auto idxReg = srcLoc(inst, 1).reg();
  if (idx->hasConstVal() && deltaFits(idx->intVal(), sz::dword)) {
    emitStore(baseReg[idx->intVal()], srcValue, srcLoc(inst, 2), Width::Full);
  } else {
    emitStore(baseReg[idxReg], srcValue, srcLoc(inst, 2), Width::Full);
  }
}

Fixup CodeGenerator::makeFixup(const BCMarker& marker, SyncOptions sync) {
  assertx(marker.valid());
  auto stackOff = marker.spOff();
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
    always_assert(RuntimeOption::HHProfServerEnabled);
    break;
  }

  Offset pcOff = marker.bcOff() - marker.func()->base();
  return Fixup{pcOff, stackOff.offset};
}

void CodeGenerator::cgLdMIStateAddr(IRInstruction* inst) {
  auto base = srcLoc(inst, 0).reg();
  int64_t offset = inst->src(1)->intVal();
  vmain() << lea{base[offset], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLdLoc(IRInstruction* inst) {
  emitLoad(inst->dst(), dstLoc(inst, 0),
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
  emitTypeCheck(
    inst->typeParam(),
    refTVType(lmem),
    refTVData(lmem),
    inst->taken()
  );
  emitLoad(inst->dst(), dstLoc(inst, 0), lmem);
}

void CodeGenerator::cgStLocPseudoMain(IRInstruction* inst) {
  auto ptr = srcLoc(inst, 0).reg();
  auto off = localOffset(inst->extra<StLocPseudoMain>()->locId);
  emitStore(ptr[off], inst->src(1), srcLoc(inst, 1), Width::Full);
}

void CodeGenerator::cgLdStkAddr(IRInstruction* inst) {
  auto const base   = srcLoc(inst, 0).reg();
  auto const offset = cellsToBytes(inst->extra<LdStkAddr>()->offset.offset);
  auto const dst    = dstLoc(inst, 0).reg();
  vmain() << lea{base[offset], dst};
}

void CodeGenerator::cgLdStk(IRInstruction* inst) {
  assertx(inst->taken() == nullptr);
  emitLoad(
    inst->dst(),
    dstLoc(inst, 0),
    srcLoc(inst, 0).reg()[cellsToBytes(inst->extra<LdStk>()->offset.offset)]
  );
}

void CodeGenerator::cgCheckStk(IRInstruction* inst) {
  auto const rbase = srcLoc(inst, 0).reg();
  auto const baseOff = cellsToBytes(inst->extra<CheckStk>()->irSpOffset.offset);
  emitTypeCheck(inst->typeParam(), rbase[baseOff + TVOFF(m_type)],
                rbase[baseOff + TVOFF(m_data)], inst->taken());
}

void CodeGenerator::cgCheckLoc(IRInstruction* inst) {
  auto const rbase = srcLoc(inst, 0).reg();
  auto const baseOff = localOffset(inst->extra<CheckLoc>()->locId);
  emitTypeCheck(inst->typeParam(), rbase[baseOff + TVOFF(m_type)],
                rbase[baseOff + TVOFF(m_data)], inst->taken());
}

void CodeGenerator::cgDefMIStateBase(IRInstruction* inst) {
  vmain() << lea{rVmTl[rds::kVmMInstrStateOff], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgCheckType(IRInstruction* inst) {
  auto const src   = inst->src(0);
  auto const dst   = inst->dst();
  auto const rData = srcLoc(inst, 0).reg(0);
  auto const rType = srcLoc(inst, 0).reg(1);
  auto& v = vmain();
  auto const sf = v.makeReg();

  auto doJcc = [&] (ConditionCode cc, Vreg sfTaken) {
    emitFwdJcc(v, ccNegate(cc), sfTaken, inst->taken());
  };

  auto doMov = [&] () {
    auto const valDst = dstLoc(inst, 0).reg(0);
    auto const typeDst = dstLoc(inst, 0).reg(1);
    if (dst->isA(Type::Bool) && !src->isA(Type::Bool)) {
      v << movtqb{rData, valDst};
    } else {
      v << copy{rData, valDst};
    }
    if (typeDst == InvalidReg) return;
    if (rType != InvalidReg) {
      v << copy{rType, typeDst};
    } else {
      v << ldimmq{src->type().toDataType(), typeDst};
    }
  };

  // Note: if you make changes to the behavior here you may need to update
  // negativeCheckType().
  auto const typeParam = inst->typeParam();
  auto const srcType = src->type();

  if (src->isA(typeParam)) {
    // src is the target type or better.  Just define our dst.
    doMov();
    return;
  }
  if (!srcType.maybe(typeParam)) {
    // src is definitely not the target type.  Always jump.
    v << jmp{label(inst->taken())};
    return;
  }

  if (rType != InvalidReg) {
    emitTypeTest(typeParam, rType, rData, sf, doJcc);
    doMov();
    return;
  }

  if (srcType <= Type::BoxedCell && typeParam <= Type::BoxedCell) {
    always_assert(!(typeParam < Type::BoxedInitCell));
    doMov();
    return;
  }

  /*
   * See if we're just checking the array kind or object class of a value
   * with a mostly-known type.
   *
   * Important: we don't support typeParam being something like
   * StaticArr=kPackedKind unless the srcType also already knows its
   * staticness.  We do allow things like CheckType<Arr=Packed> t1:StaticArr,
   * though.  This is why we have to check that the unspecialized type is at
   * least as big as the srcType.
   */
  if (typeParam.isSpecialized() && typeParam.unspecialize() >= srcType) {
    emitSpecializedTypeTest(typeParam, rData, sf, doJcc);
    doMov();
    return;
  }

  /*
   * Since not all of our unions carry a type register, there are some
   * situations with strings and arrays that are neither constantly-foldable
   * nor in the emitTypeTest code path.
   *
   * We currently actually check their static bit here.  Note (importantly)
   * that this is why toDataType can't return KindOfStaticString for
   * Type::StaticStr---this code will let an apc string through.  Also note
   * that CheckType<Uncounted> t1:{Null|Str} doesn't get this treatment
   * currently---in the emitTypeTest path above it will only check the type
   * register.
   */
  if (!typeParam.isSpecialized() &&
      typeParam <= Type::Uncounted &&
      srcType.subtypeOfAny(Type::Str, Type::Arr) &&
      srcType.maybe(typeParam)) {
    assertx(srcType.maybe(Type::Static));
    v << cmplim{0, rData[FAST_REFCOUNT_OFFSET], sf};
    doJcc(CC_L, sf);
    doMov();
    return;
  }

  always_assert_flog(
    false,
    "Bad src: {} and dst: {} types in '{}'", srcType, typeParam, *inst
  );
}

void CodeGenerator::cgCheckTypeMem(IRInstruction* inst) {
  auto const reg = srcLoc(inst, 0).reg();
  emitTypeCheck(inst->typeParam(), reg[TVOFF(m_type)],
                reg[TVOFF(m_data)], inst->taken());
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
  assertx(firstBitNumTmp->hasConstVal(Type::Int));
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

void CodeGenerator::cgLdClsMethod(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto clsReg = srcLoc(inst, 0).reg();
  int32_t mSlotVal = inst->src(1)->rawVal();
  auto methOff = int32_t(mSlotVal * sizeof(LowFuncPtr));
  auto& v = vmain();
  emitLdLowPtr(v, clsReg[methOff], dstReg, sizeof(LowFuncPtr));
}

void CodeGenerator::cgLookupClsMethodCache(IRInstruction* inst) {
  auto funcDestReg   = dstLoc(inst, 0).reg(0);

  auto const& extra = *inst->extra<ClsMethodData>();
  auto const cls = extra.clsName;
  auto const method = extra.methodName;
  auto const ne = extra.namedEntity;
  auto const ch = StaticMethodCache::alloc(cls, method,
                                   getContextName(getClass(inst->marker())));

  if (false) { // typecheck
    UNUSED TypedValue* fake_fp = nullptr;
    UNUSED TypedValue* fake_sp = nullptr;
    const UNUSED Func* f = StaticMethodCache::lookup(
      ch, ne, cls, method, fake_fp);
  }

  // can raise an error if class is undefined
  cgCallHelper(vmain(),
               CppCall::direct(StaticMethodCache::lookup),
               callDest(funcDestReg),
               SyncOptions::kSyncPoint,
               argGroup(inst)
                 .imm(ch)       // Handle ch
                 .immPtr(ne)            // NamedEntity* np.second
                 .immPtr(cls)           // className
                 .immPtr(method)        // methodName
                 .reg(srcLoc(inst, 0).reg()) // frame pointer
              );
}

void CodeGenerator::cgLdClsMethodCacheCommon(IRInstruction* inst, Offset off) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto const& extra = *inst->extra<ClsMethodData>();
  auto const clsName = extra.clsName;
  auto const methodName = extra.methodName;
  auto const ch = StaticMethodCache::alloc(clsName, methodName,
                                     getContextName(getClass(inst->marker())));
  vmain() << load{rVmTl[ch + off], dstReg};
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
void CodeGenerator::emitGetCtxFwdCallWithThis(Vreg srcCtx, Vreg dstCtx,
                                              bool staticCallee) {
  auto& v = vmain();
  if (staticCallee) {
    // Load (this->m_cls | 0x1) into ctxReg.
    auto vmclass = v.makeReg();
    emitLdLowPtr(v, srcCtx[ObjectData::getVMClassOffset()],
                 vmclass, sizeof(LowClassPtr));
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
  bool      withThis = srcCtxTmp->isA(Type::Obj);
  auto& v = vmain();

  // If we don't know whether we have a This, we need to check dynamically
  if (!withThis) {
    auto const sf = v.makeReg();
    v << testbi{1, srcCtxReg, sf};
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
  auto const ch = StaticMethodFCache::alloc(
    clsName, methodName, getContextName(getClass(inst->marker()))
  );
  vmain() << load{rVmTl[ch], dstReg};
}

void CodeGenerator::cgLookupClsMethodFCache(IRInstruction* inst) {
  auto const funcDestReg = dstLoc(inst, 0).reg(0);
  auto const cls         = inst->src(0)->clsVal();
  auto const& extra      = *inst->extra<ClsMethodData>();
  auto const methName    = extra.methodName;
  auto const fpReg       = srcLoc(inst, 1).reg();
  auto const clsName     = cls->name();

  auto ch = StaticMethodFCache::alloc(
    clsName, methName, getContextName(getClass(inst->marker()))
  );

  const Func* (*lookup)(
    rds::Handle, const Class*, const StringData*, TypedValue*) =
    StaticMethodFCache::lookup;
  cgCallHelper(vmain(),
               CppCall::direct(lookup),
               callDest(funcDestReg),
               SyncOptions::kSyncPoint,
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
  auto const sf = v.makeReg();
  v << cmplim{1, rVmTl[ch + offsetof(StaticMethodFCache, m_static)], sf};
  return cond(v, CC_E, sf, destCtxReg, [&](Vout& v) {
    // If calling a static method...
    // Load (this->m_cls | 0x1) into destCtxReg
    auto vmclass = v.makeReg();
    auto dst1 = v.makeReg();
    emitLdLowPtr(v, thisReg[ObjectData::getVMClassOffset()],
                 vmclass, sizeof(LowClassPtr));
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

  if (t <= Type::Cctx) {
    // Nothing to do. Forward the context as is.
    v << copy{srcCtxReg, destCtxReg};
    return;
  }
  if (t <= Type::Obj) {
    // We definitely have $this, so always run code emitted by
    // emitGetCtxFwdCallWithThisDyn
    emitGetCtxFwdCallWithThisDyn(destCtxReg, srcCtxReg, make_cache());
    return;
  }

  // dynamically check if we have a This pointer and call
  // emitGetCtxFwdCallWithThisDyn below
  auto const sf = v.makeReg();
  v << testbi{1, srcCtxReg, sf};
  cond(v, CC_Z, sf, destCtxReg, [&](Vout& v) {
    // If we have a 'this' pointer ...
    return emitGetCtxFwdCallWithThisDyn(v.makeReg(), srcCtxReg, make_cache());
  }, [&](Vout& v) {
    return srcCtxReg;
  });
}

rds::Handle CodeGenerator::cgLdClsCachedCommon(Vout& v, IRInstruction* inst,
                                               Vreg dst, Vreg sf) {
  const StringData* className = inst->src(0)->strVal();
  auto ch = NamedEntity::get(className)->getClassHandle();
  v << load{rVmTl[ch], dst};
  v << testq{dst, dst, sf};
  return ch;
}

void CodeGenerator::cgLdClsCached(IRInstruction* inst) {
  auto& v = vmain();
  auto dst1 = v.makeReg();
  auto const sf = v.makeReg();
  auto ch = cgLdClsCachedCommon(v, inst, dst1, sf);
  unlikelyCond(v, vcold(), CC_E, sf, dstLoc(inst, 0).reg(), [&] (Vout& v) {
    auto dst2 = v.makeReg();
    Class* (*const func)(Class**, const StringData*) = jit::lookupKnownClass;
    cgCallHelper(v, CppCall::direct(func), callDest(dst2),
                 SyncOptions::kSyncPoint,
                 argGroup(inst).addr(rVmTl, safe_cast<int32_t>(ch))
                           .ssa(0));
    return dst2;
  }, [&](Vout& v) {
    return dst1;
  });
}

void CodeGenerator::cgLdClsCachedSafe(IRInstruction* inst) {
  auto& v = vmain();
  auto const sf = v.makeReg();
  cgLdClsCachedCommon(v, inst, dstLoc(inst, 0).reg(), sf);
}

void CodeGenerator::cgDerefClsRDSHandle(IRInstruction* inst) {
  auto const dreg = dstLoc(inst, 0).reg();
  auto const ch   = srcLoc(inst, 0).reg();
  const Vreg rds = rVmTl;
  auto& v = vmain();
  v << load{rds[ch], dreg};
}

void CodeGenerator::cgLdCls(IRInstruction* inst) {
  auto const ch = ClassCache::alloc();
  rds::recordRds(ch, sizeof(ClassCache),
                 "ClassCache", getFunc(inst->marker())->fullName()->data());
  cgCallHelper(vmain(),
               CppCall::direct(ClassCache::lookup),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup(inst).imm(ch).ssa(0/*className*/));
}

void CodeGenerator::cgLdRDSAddr(IRInstruction* inst) {
  auto const handle = inst->extra<LdRDSAddr>()->handle;
  vmain() << lea{rVmTl[handle], dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgLookupClsCns(IRInstruction* inst) {
  auto const extra = inst->extra<LookupClsCns>();
  auto const link  = rds::bindClassConstant(extra->clsName, extra->cnsName);
  cgCallHelper(vmain(),
    CppCall::direct(jit::lookupClassConstantTv),
    callDestTV(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .addr(rVmTl, safe_cast<int32_t>(link.handle()))
      .immPtr(NamedEntity::get(extra->clsName))
      .immPtr(extra->clsName)
      .immPtr(extra->cnsName)
  );
}

void CodeGenerator::cgLdCns(IRInstruction* inst) {
  auto const cnsName = inst->src(0)->strVal();
  auto const ch = makeCnsHandle(cnsName, false);
  emitLoad(inst->dst(), dstLoc(inst, 0), rVmTl[ch]);
}

void CodeGenerator::cgLookupCnsCommon(IRInstruction* inst) {
  SSATmp* cnsNameTmp = inst->src(0);

  assertx(cnsNameTmp->hasConstVal(Type::StaticStr));

  auto const cnsName = cnsNameTmp->strVal();
  auto const ch = makeCnsHandle(cnsName, false);

  auto args = argGroup(inst);
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

  auto args = argGroup(inst);
  args.addr(rVmTl, safe_cast<int32_t>(fallbackCh))
      .immPtr(cnsName)
      .immPtr(fallbackName);

  cgCallHelper(vmain(), CppCall::direct(lookupCnsUHelper),
               callDestTV(inst),
               SyncOptions::kSyncPoint,
               args);
}

void CodeGenerator::cgAKExistsArr(IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  if (keyTy <= Type::InitNull) {
    cgCallHelper(
      v,
      arrayCallIfLowMem(inst, &g_array_funcs.existsStr),
      callDest(inst),
      SyncOptions::kNoSyncPoint,
      argGroup(inst)
        .ssa(0)
        .immPtr(staticEmptyString())
    );
    return;
  }

  auto const keyInfo = checkStrictlyInteger(keyTy);
  auto const target =
    keyInfo.checkForInt ? CppCall::direct(ak_exist_string) :
    keyInfo.type == KeyType::Int ?
      arrayCallIfLowMem(inst, &g_array_funcs.existsInt) :
    arrayCallIfLowMem(inst, &g_array_funcs.existsStr);
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
    SyncOptions::kNoSyncPoint,
    args
  );
}

void CodeGenerator::cgAKExistsObj(IRInstruction* inst) {
  auto const keyTy = inst->src(1)->type();
  auto& v = vmain();

  if (keyTy <= Type::InitNull) {
    v << ldimmq{0, dstLoc(inst, 0).reg()};
    return;
  }

  cgCallHelper(
    v,
    keyTy <= Type::Int
      ? CppCall::direct(ak_exist_int_obj)
      : CppCall::direct(ak_exist_string_obj),
    callDest(inst),
    SyncOptions::kSyncPoint,
    argGroup(inst)
      .ssa(0)
      .ssa(1)
  );
}

void CodeGenerator::cgLdGblAddr(IRInstruction* inst) {
  auto dstReg = dstLoc(inst, 0).reg();
  auto& v = vmain();
  cgCallHelper(v,
               CppCall::direct(ldGblAddrHelper),
               callDest(dstReg),
               SyncOptions::kNoSyncPoint,
               argGroup(inst).ssa(0));
  auto const sf = v.makeReg();
  v << testq{dstReg, dstReg, sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

Vreg CodeGenerator::emitTestZero(Vout& v, SSATmp* src, Vloc srcLoc) {
  auto reg = srcLoc.reg();
  auto const sf = v.makeReg();
  if (src->isA(Type::Bool)) {
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
    if (src->isA(Type::Bool) && !def.dst(i)->isA(Type::Bool)) {
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
  auto& v = vmain();
  v << syncvmsp{srcLoc(inst, 1).reg()};
  v << jmpr{srcLoc(inst, 0).reg(), kCrossTraceRegs};
}

void CodeGenerator::cgNewCol(IRInstruction* inst) {
  auto& v = vmain();
  auto const dest = callDest(inst);
  auto args = argGroup(inst);
  if (inst->extra<NewCol>()->type != CollectionType::Pair) {
    args.imm(inst->extra<NewCol>()->size);
  }
  auto const target = [&]() -> CppCall {
    switch(inst->extra<NewCol>()->type) {
      case CollectionType::Vector:
        return CppCall::direct(newColHelper<c_Vector>);
      case CollectionType::Map:
        return CppCall::direct(newColHelper<c_Map>);
      case CollectionType::Set:
        return CppCall::direct(newColHelper<c_Set>);
      case CollectionType::Pair:
        return CppCall::direct(newPairHelper);
      case CollectionType::ImmVector:
        return CppCall::direct(newColHelper<c_ImmVector>);
      case CollectionType::ImmMap:
        return CppCall::direct(newColHelper<c_ImmMap>);
      case CollectionType::ImmSet:
        return CppCall::direct(newColHelper<c_ImmSet>);
    }
    not_reached();
  }();
  cgCallHelper(v, target, dest, SyncOptions::kSyncPoint, args);
}

void CodeGenerator::cgCheckInit(IRInstruction* inst) {
  Block* taken = inst->taken();
  assertx(taken);
  SSATmp* src = inst->src(0);

  if (!src->type().maybe(Type::Uninit)) return;

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
  if (!t.maybe(Type::Uninit)) return;
  auto basereg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitCmpTVType(v, sf, KindOfUninit, basereg[TVOFF(m_type)]);
  v << jcc{CC_Z, sf, {label(inst->next()), label(taken)}};
}

void CodeGenerator::cgCheckSurpriseFlags(IRInstruction* inst) {
  auto&v = vmain();
  auto const sf = emitTestSurpriseFlags(v, rVmTl);
  v << jcc{CC_NZ, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgCheckCold(IRInstruction* inst) {
  Block*     taken = inst->taken();
  TransID  transId = inst->extra<CheckCold>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);
  auto& v = vmain();
  auto const sf = v.makeReg();
  v << decqm{v.cns(counterAddr)[0], sf};
  v << jcc{CC_LE, sf, {label(inst->next()), label(taken)}};
}

static const StringData* s_ReleaseVV = makeStaticString("ReleaseVV");

void CodeGenerator::cgReleaseVVOrExit(IRInstruction* inst) {
  auto* const label = inst->taken();
  auto const rFp = srcLoc(inst, 0).reg();
  auto& v = vmain();

  TargetProfile<ReleaseVVProfile> profile(m_state.unit.context(),
                                          inst->marker(), s_ReleaseVV);
  if (profile.profiling()) {
    v << incwm{rVmTl[profile.handle() + offsetof(ReleaseVVProfile, executed)],
               v.makeReg()};
  }

  auto const sf = v.makeReg();
  v << cmpqim{0, rFp[AROFF(m_varEnv)], sf};

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
  ifBlock(v, vcold(), CC_NZ, sf, [&] (Vout& v) {
    if (profile.profiling()) {
      auto offsetof_release = offsetof(ReleaseVVProfile, released);
      v << incwm{rVmTl[profile.handle() + offsetof_release], v.makeReg()};
    }
    auto const sf = v.makeReg();
    v << testqim{ActRec::kExtraArgsBit, rFp[AROFF(m_varEnv)], sf};
    emitFwdJcc(v, CC_Z, sf, label);
    cgCallHelper(
      v,
      CppCall::direct(static_cast<void (*)(ActRec*)>(ExtraArgs::deallocate)),
      kVoidDest,
      SyncOptions::kSyncPoint,
      argGroup(inst).reg(rFp)
    );
  },
  releaseUnlikely);
}

void CodeGenerator::cgBoxPtr(IRInstruction* inst) {
  auto base    = srcLoc(inst, 0).reg();
  auto dstReg  = dstLoc(inst, 0).reg();
  auto& v = vmain();
  auto const sf = v.makeReg();
  emitTypeTest(Type::BoxedCell, base[TVOFF(m_type)], base[TVOFF(m_data)],
    sf, [&](ConditionCode cc, Vreg sfTaken) {
      cond(v, cc, sfTaken, dstReg, [&](Vout& v) {
        return base;
      }, [&](Vout& v) {
        auto dst2 = v.makeReg();
        cgCallHelper(v, CppCall::direct(tvBox), callDest(dst2),
                     SyncOptions::kNoSyncPoint,
                     argGroup(inst).ssa(0/*addr*/));
        return dst2;
      });
    });
}

void CodeGenerator::cgConcatCellCell(IRInstruction* inst) {
  // Supported cases are all simplified into other instructions
  CG_PUNT(inst->marker(), cgConcatCellCell);
}

void CodeGenerator::cgInterpOne(IRInstruction* inst) {
  auto const extra = inst->extra<InterpOne>();
  auto const pcOff = extra->bcOff;
  auto const spOff = extra->spOffset;
  auto const op    = extra->opcode;
  auto const interpOneHelper = interpOneEntryPoints[size_t(op)];

  cgCallHelper(
    vmain(),
    CppCall::direct(reinterpret_cast<void (*)()>(interpOneHelper)),
    kVoidDest,
    SyncOptions::kNoSyncPoint, // interpOne syncs regs manually
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
  v << ldimml{pcOff, r32(rAsm)};
  assertx(mcg->tx().uniqueStubs.interpOneCFHelpers.count(op));
  v << jmpi{mcg->tx().uniqueStubs.interpOneCFHelpers[op],
            kCrossTraceRegs | rAsm};
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
  v << contenter{curFpReg, addrReg, kCrossTraceRegs, {next, catchBlock}};
  v = next;
  // curFpReg->m_savedRip will point here, and the next HHIR opcode must
  // also start here.
  v << defvmsp{dstLoc(inst, 0).reg()};
}

void CodeGenerator::cgContPreNext(IRInstruction* inst) {
  auto contReg      = srcLoc(inst, 0).reg();
  auto checkStarted = inst->src(1)->boolVal();
  auto stateOff     = BaseGenerator::stateOff();
  auto& v = vmain();
  auto const sf = v.makeReg();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");
  static_assert(uint8_t(BaseGenerator::State::Started) == 1, "used below");
  static_assert(uint8_t(BaseGenerator::State::Running) > 1, "");
  static_assert(uint8_t(BaseGenerator::State::Done) > 1, "");

  // Take exit if state != 1 (checkStarted) or state > 1 (!checkStarted).
  v << cmpbim{int8_t(BaseGenerator::State::Started), contReg[stateOff], sf};
  emitFwdJcc(v, checkStarted ? CC_NE : CC_A, sf, inst->taken());

  // Set generator state as Running.
  v << storebi{int8_t(BaseGenerator::State::Running), contReg[stateOff]};
}

void CodeGenerator::cgContStartedCheck(IRInstruction* inst) {
  auto contReg  = srcLoc(inst, 0).reg();
  auto stateOff = BaseGenerator::stateOff();
  auto& v = vmain();

  static_assert(uint8_t(BaseGenerator::State::Created) == 0, "used below");

  // Take exit if state == 0.
  auto const sf = v.makeReg();
  v << testbim{int8_t(0xff), contReg[stateOff], sf};
  v << jcc{CC_Z, sf, {label(inst->next()), label(inst->taken())}};
}

void CodeGenerator::cgContValid(IRInstruction* inst) {
  auto contReg  = srcLoc(inst, 0).reg();
  auto dstReg   = dstLoc(inst, 0).reg();
  auto stateOff = BaseGenerator::stateOff();
  auto& v = vmain();

  // Return 1 if generator state is not Done.
  auto const sf = v.makeReg();
  v << cmpbim{int8_t(BaseGenerator::State::Done), contReg[stateOff], sf};
  v << setcc{CC_NE, sf, dstReg};
}

void CodeGenerator::cgContArIncKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto& v = vmain();
  v << incqm{contArReg[CONTOFF(m_key) + TVOFF(m_data) - c_Generator::arOff()],
             v.makeReg()};
}

void CodeGenerator::cgContArUpdateIdx(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto newIdxReg = srcLoc(inst, 1).reg();
  int64_t off = CONTOFF(m_index) - c_Generator::arOff();
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
  ptrdiff_t offset = BaseGenerator::arOff();
  vmain() << lea{base[offset], dest};
}

void CodeGenerator::cgLdContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t valueOff = CONTOFF(m_value);
  int64_t off = valueOff - c_Generator::arOff();
  emitLoad(inst->dst(), dstLoc(inst, 0), contArReg[off]);
}

void CodeGenerator::cgStContArValue(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(inst, 1);
  const int64_t valueOff = CONTOFF(m_value);
  int64_t off = valueOff - c_Generator::arOff();
  emitStore(contArReg[off], value, valueLoc, Width::Full);
}

void CodeGenerator::cgLdContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  const int64_t keyOff = CONTOFF(m_key);
  int64_t off = keyOff - c_Generator::arOff();
  emitLoad(inst->dst(), dstLoc(inst, 0), contArReg[off]);
}

void CodeGenerator::cgStContArKey(IRInstruction* inst) {
  auto contArReg = srcLoc(inst, 0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(inst, 1);

  const int64_t keyOff = CONTOFF(m_key);
  int64_t off = keyOff - c_Generator::arOff();
  emitStore(contArReg[off], value, valueLoc, Width::Full);
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
    c_Generator::resumeAddrOff() - BaseGenerator::arOff(),
    c_Generator::resumeOffsetOff() - BaseGenerator::arOff()
  );
}

void CodeGenerator::cgLdContResumeAddr(IRInstruction* inst) {
  vmain() << load{
    srcLoc(inst, 0).reg()[c_Generator::resumeAddrOff()],
    dstLoc(inst, 0).reg()
  };
}

void CodeGenerator::cgContArIncIdx(IRInstruction* inst) {
  auto& v = vmain();
  auto const idxOff = CONTOFF(m_index) - BaseGenerator::arOff();
  auto const dst    = dstLoc(inst, 0).reg();
  auto const src    = srcLoc(inst, 0).reg()[idxOff];
  auto const tmp    = v.makeReg();
  v << load{src, tmp};
  v << incq{tmp, dst, v.makeReg()};
  v << store{dst, src};
}

void CodeGenerator::cgStContArState(IRInstruction* inst) {
  auto const off = c_Generator::stateOff() - BaseGenerator::arOff();
  vmain() << storebi{
    static_cast<int8_t>(inst->extra<StContArState>()->state),
    srcLoc(inst, 0).reg()[off]
  };
}

void CodeGenerator::cgStAsyncArResult(IRInstruction* inst) {
  auto asyncArReg = srcLoc(inst, 0).reg();
  auto value = inst->src(1);
  auto valueLoc = srcLoc(inst, 1);
  const int64_t off = c_AsyncFunctionWaitHandle::resultOff()
                    - c_AsyncFunctionWaitHandle::arOff();
  emitStore(asyncArReg[off], value, valueLoc, Width::Full);
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
  const int8_t blocked = c_WaitHandle::toKindState(
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
  v << storebi{blocked, parentArReg[stateToArOff]};

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
  emitLoad(inst->dst(), dstLoc(inst, 0), robj[c_WaitHandle::resultOff()]);
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

  auto           fpReg = srcLoc(inst, 1).reg();
  int       iterOffset = this->iterOffset(inst->marker(),
                                          inst->extra<IterData>()->iterId);
  int   valLocalOffset = localOffset(inst->extra<IterData>()->valId);
  SSATmp*          src = inst->src(0);
  auto args = argGroup(inst);
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
    assertx(src->type() <= Type::Obj);
    args.imm(uintptr_t(getClass(inst->marker()))).addr(fpReg, valLocalOffset);
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
  auto const fpReg = srcLoc(inst, 1).reg();
  auto const iterOffset = this->iterOffset(inst->marker(),
                                           inst->extra<IterData>()->iterId);
  auto const valLocalOffset = localOffset(inst->extra<IterData>()->valId);

  auto args = argGroup(inst);
  args.addr(fpReg, iterOffset).ssa(0/*src*/);

  auto innerType = inst->typeParam();
  assertx(innerType.isKnownDataType());

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
    return;
  }

  always_assert(innerType <= Type::Obj);

  args.immPtr(getClass(inst->marker())).addr(fpReg, valLocalOffset);
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
  auto fpReg = srcLoc(inst, 0).reg();
  auto args = argGroup(inst);
  auto& marker = inst->marker();
  args.addr(fpReg, iterOffset(marker, inst->extra<IterData>()->iterId))
      .addr(fpReg, localOffset(inst->extra<IterData>()->valId));
  if (isNextK) {
    args.addr(fpReg, localOffset(inst->extra<IterData>()->keyId));
  } else if (isWNext) {
    // We punt this case because nothing is using WIterNext opcodes
    // right now, and we don't want the witer_next_key helper to need
    // to check for null.
    CG_PUNT(inst->marker(), WIterNext-nonKey);
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
  auto fpReg = srcLoc(inst, 0).reg();
  auto& marker = inst->marker();
  auto args = argGroup(inst);
  args.addr(fpReg, iterOffset(marker, inst->extra<IterData>()->iterId))
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
  auto fpReg = srcLoc(inst, 0).reg();
  int offset = iterOffset(inst->marker(), inst->extra<IterFree>()->iterId);
  cgCallHelper(vmain(),
               CppCall::method(&Iter::free),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup(inst).addr(fpReg, offset));
}

void CodeGenerator::cgMIterFree(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 0).reg();
  int offset = iterOffset(inst->marker(), inst->extra<MIterFree>()->iterId);
  cgCallHelper(vmain(),
               CppCall::method(&Iter::mfree),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup(inst).addr(fpReg, offset));
}

void CodeGenerator::cgDecodeCufIter(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 1).reg();
  int offset = iterOffset(inst->marker(), inst->extra<DecodeCufIter>()->iterId);
  cgCallHelper(vmain(),
               CppCall::direct(decodeCufIterHelper),
               callDest(inst),
               SyncOptions::kSyncPoint,
               argGroup(inst).addr(fpReg, offset)
                                  .typedValue(0));
}

void CodeGenerator::cgCIterFree(IRInstruction* inst) {
  auto fpReg = srcLoc(inst, 0).reg();
  int offset = iterOffset(inst->marker(), inst->extra<CIterFree>()->iterId);
  cgCallHelper(vmain(),
               CppCall::method(&Iter::cfree),
               kVoidDest,
               SyncOptions::kSyncPoint,
               argGroup(inst).addr(fpReg, offset));
}

void CodeGenerator::cgNewStructArray(IRInstruction* inst) {
  auto const data = inst->extra<NewStructData>();
  if (!RuntimeOption::EvalDisableStructArray) {
    if (auto shape = Shape::create(data->keys, data->numKeys)) {
      StructArray* (*f)(uint32_t, const TypedValue*, Shape*) =
        &MixedArray::MakeStructArray;
      cgCallHelper(vmain(),
        CppCall::direct(f),
        callDest(inst),
        SyncOptions::kNoSyncPoint,
        argGroup(inst)
          .imm(data->numKeys)
          .addr(srcLoc(inst, 0).reg(), cellsToBytes(data->offset.offset))
          .imm(shape)
      );
      return;
    }
  }

  StringData** table = mcg->allocData<StringData*>(sizeof(StringData*),
                                                      data->numKeys);
  memcpy(table, data->keys, data->numKeys * sizeof(*data->keys));
  MixedArray* (*f)(uint32_t, StringData**, const TypedValue*) =
    &MixedArray::MakeStruct;
  cgCallHelper(
    vmain(),
    CppCall::direct(f),
    callDest(inst),
    SyncOptions::kNoSyncPoint,
    argGroup(inst)
      .imm(data->numKeys)
      .imm(uintptr_t(table))
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
  TransID  transId = inst->extra<TransIDData>()->transId;
  auto counterAddr = mcg->tx().profData()->transCounterAddr(transId);
  auto& v = vmain();
  v << decqm{v.cns(counterAddr)[0], v.makeReg()};
}

void CodeGenerator::cgDbgAssertRefCount(IRInstruction* inst) {
  ifRefCountedType(
    vmain(), inst->src(0)->type(), srcLoc(inst, 0),
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
  emitTypeTest(inst->typeParam(), type_reg, data_reg, sf,
    [&](ConditionCode cc, Vreg sfTaken) {
      ifThen(v, ccNegate(cc), sfTaken, [&](Vout& v) {
        v << ud2{};
      });
    });
}

void CodeGenerator::emitVerifyCls(IRInstruction* inst) {
  auto const objClass = inst->src(0);
  auto const objClassReg = srcLoc(inst, 0).reg();
  auto const constraint = inst->src(1);
  auto const constraintReg = srcLoc(inst, 1).reg();
  auto& v = vmain();
  if (constraint->hasConstVal(Type::Cls) && objClass->hasConstVal()) {
    if (objClass->clsVal() != constraint->clsVal()) {
      cgCallNative(v, inst);
    }
    return;
  }
  auto const sf = v.makeReg();
  if (!constraint->hasConstVal(Type::Cls) && objClass->hasConstVal()) {
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
               CppCall::direct(Trace::ringbufferEntryRip),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               argGroup(inst)
                 .imm(extra.type)
                 .imm(extra.sk.toAtomicInt()));
}

void CodeGenerator::cgRBTraceMsg(IRInstruction* inst) {
  auto const& extra = *inst->extra<RBTraceMsg>();
  auto& v = vmain();
  assertx(extra.msg->isStatic());
  cgCallHelper(v,
               CppCall::direct(Trace::ringbufferMsg),
               kVoidDest,
               SyncOptions::kNoSyncPoint,
               argGroup(inst)
                 .immPtr(extra.msg->data())
                 .imm(extra.msg->size())
                 .imm(extra.type));
}

void CodeGenerator::cgCountBytecode(IRInstruction* inst) {
  auto& v = vmain();
  v << countbytecode{rVmTl, v.makeReg()};
}

void CodeGenerator::cgLdClsInitData(IRInstruction* inst) {
  const Vreg rds = rVmTl;
  auto clsReg = srcLoc(inst, 0).reg();
  auto dstReg = dstLoc(inst, 0).reg();
  auto offset = Class::propDataCacheOff() +
                rds::Link<Class::PropInitVec*>::handleOff();
  auto& v = vmain();
  auto handle = v.makeReg();
  auto vec = v.makeReg();
  v << loadl{clsReg[offset], handle};
  v << load{rds[handle], vec};
  v << load{vec[Class::PropInitVec::dataOff()], dstReg};
}

void CodeGenerator::cgConjure(IRInstruction* inst) {
  vmain() << ud2();
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
      cgCallHelper(v, CppCall::method(&ArrayData::vsize),
                   callDest(dst2), SyncOptions::kNoSyncPoint,
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
  v << loadzlq{baseReg[FAST_COLLECTION_SIZE_OFFSET], dstReg};
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
  v << shrli{1, tmp, dst, v.makeReg()};
}

void CodeGenerator::cgInitPackedArray(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  auto const index = inst->extra<InitPackedArray>()->index;
  auto const value = srcLoc(inst, 1);
  auto& v = vmain();

  auto slotOffset = PackedArray::entriesOffset() + index * sizeof(TypedValue);
  emitStoreTypedValue(v, arrReg, slotOffset, value, inst->src(1)->type());
}

void CodeGenerator::cgInitPackedArrayLoop(IRInstruction* inst) {
  auto const arrReg = srcLoc(inst, 0).reg();
  int const count = inst->extra<InitPackedArrayLoop>()->size;
  auto const offset = inst->extra<InitPackedArrayLoop>()->offset;
  auto const spIn = srcLoc(inst, 1).reg();

  auto& v = vmain();
  auto const loopBody   = v.makeBlock();
  auto const done       = v.makeBlock();
  auto const firstEntry = PackedArray::entriesOffset();

  auto const sp = v.makeReg();
  v << lea{spIn[cellsToBytes(offset.offset)], sp};

  // Initialize loop variables and jump to the first condition check.
  Vreg i0 = v.makeReg(), i1 = v.makeReg(), i2 = v.makeReg(), i3 = v.makeReg();
  Vreg j0 = v.makeReg(), j1 = v.makeReg(), j2 = v.makeReg(), j3 = v.makeReg();
  auto const value = v.makeReg();
  i0 = v.cns(0);
  j0 = v.cns((count - 1) * 2);
  v << phijmp{loopBody, v.makeTuple({i0, j0})};

  // We know that we have at least one element in the array so we don't have
  // to do an initial bounds check.
  assertx(count);

  v = loopBody;
  v << phidef{v.makeTuple({i1, j1})};

  // Load the value from the stack and store into the array. It's safe
  // to copy all 16 bytes of the value because packed arrays don't use
  // The TypedValueAux::m_aux field.
  v << loaddqu{sp[j1 * 8], value};
  v << storedqu{value, arrReg[i1 * 8] + firstEntry};
  // Increment the loop variable by 2 because we can only scale by at most 8.
  v << lea{i1[2], i2};
  auto subFlags = v.makeReg();
  v << subqi{2, j1, j2, subFlags};

  // Jump back to the body if we're still in bounds, fall through otherwise.
  v << phijcc{CC_GE, subFlags, {done, loopBody}, v.makeTuple({i2, j2})};

  v = done;
  v << phidef{v.makeTuple({i3, j3})};
}

void CodeGenerator::cgLdStructArrayElem(IRInstruction* inst) {
  auto const array = srcLoc(inst, 0).reg();
  auto const key = inst->src(1)->strVal();
  auto const shape = inst->src(0)->type().arrSpec().shape();
  auto const offset = shape->offsetFor(key);
  assertx(offset != PropertyTable::kInvalidOffset);

  auto const actualOffset = StructArray::dataOffset() +
    sizeof(TypedValue) * offset;
  emitLoad(inst->dst(), dstLoc(inst, 0), array[actualOffset]);
}

void CodeGenerator::print() const {
  jit::print(std::cout, m_state.unit, m_state.asmInfo);
}

}}}
