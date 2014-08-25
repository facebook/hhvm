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

#include "hphp/runtime/vm/jit/ir-translator.h"

#include <stdint.h>
#include <algorithm>
#include <functional>
#include "hphp/runtime/base/strings.h"

#include "folly/Format.h"
#include "folly/Conv.h"
#include "hphp/util/trace.h"
#include "hphp/util/stack-trace.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/bc-pattern.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP { namespace jit {

using namespace reg;
using namespace Trace;
using std::max;

TRACE_SET_MOD(hhir);

#define HHIR_EMIT(op, ...)                      \
  do {                                          \
    m_hhbcTrans.emit ## op(__VA_ARGS__);        \
    return;                                     \
  } while (0)

#define HHIR_UNIMPLEMENTED_OP(op)                       \
  do {                                                  \
    throw jit::FailedIRGen(__FILE__, __LINE__, op);     \
  } while (0)


#define HHIR_UNIMPLEMENTED(op)                          \
  do {                                                  \
    throw jit::FailedIRGen(__FILE__, __LINE__, #op);    \
  } while (0)

#define HHIR_UNIMPLEMENTED_WHEN(expr, op)               \
  do {                                                  \
    if (expr) {                                         \
      throw jit::FailedIRGen(__FILE__, __LINE__, #op);  \
    }                                                   \
  } while (0)

static JmpFlags instrJmpFlags(const NormalizedInstruction& ni) {
  JmpFlags flags = JmpFlagNone;
  if (ni.endsRegion) {
    flags = flags | JmpFlagEndsRegion;
  }
  if (ni.nextIsMerge) {
    flags = flags | JmpFlagNextIsMerge;
  }
  return flags;
}

IRTranslator::IRTranslator(TransContext context)
  : m_hhbcTrans{context}
{}

void
IRTranslator::translateBinaryArithOp(const NormalizedInstruction& i) {
  switch (i.op()) {
  case Op::Add:    HHIR_EMIT(Add);
  case Op::Sub:    HHIR_EMIT(Sub);
  case Op::Mul:    HHIR_EMIT(Mul);
  case Op::AddO:   HHIR_EMIT(AddO);
  case Op::SubO:   HHIR_EMIT(SubO);
  case Op::MulO:   HHIR_EMIT(MulO);
  case Op::BitAnd: HHIR_EMIT(BitAnd);
  case Op::BitOr:  HHIR_EMIT(BitOr);
  case Op::BitXor: HHIR_EMIT(BitXor);
  default: break;
  }
  not_reached();
}

void
IRTranslator::translateSameOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Same || op == Op::NSame);
  if (op == Op::Same) {
    HHIR_EMIT(Same);
  } else {
    HHIR_EMIT(NSame);
  }
}

void
IRTranslator::translateEqOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Eq || op == Op::Neq);
  if (op == Op::Eq) {
    HHIR_EMIT(Eq);
  } else {
    HHIR_EMIT(Neq);
  }
}

void
IRTranslator::translateLtGtOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == Op::Lt || op == Op::Lte || op == Op::Gt || op == Op::Gte);

  auto leftType = m_hhbcTrans.topType(1, DataTypeGeneric);
  auto rightType = m_hhbcTrans.topType(0, DataTypeGeneric);
  if (!leftType.isKnownDataType() || !rightType.isKnownDataType()) {
    HHIR_UNIMPLEMENTED(LtGtOp-UnknownInput);
  }
  bool ok =
    leftType.subtypeOfAny (Type::Null, Type::Bool, Type::Int, Type::Dbl) &&
    rightType.subtypeOfAny(Type::Null, Type::Bool, Type::Int, Type::Dbl);

  HHIR_UNIMPLEMENTED_WHEN(!ok, LtGtOp);
  switch (op) {
    case Op::Lt  : HHIR_EMIT(Lt);
    case Op::Lte : HHIR_EMIT(Lte);
    case Op::Gt  : HHIR_EMIT(Gt);
    case Op::Gte : HHIR_EMIT(Gte);
    default    : HHIR_UNIMPLEMENTED(LtGtOp);
  }
}

void
IRTranslator::translateUnaryBooleanOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == OpCastBool || op == OpEmptyL);
  if (op == OpCastBool) {
    HHIR_EMIT(CastBool);
  } else {
    HHIR_EMIT(EmptyL, i.imm[0].u_LA);
  }
}

void
IRTranslator::translateBranchOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  assert(op == OpJmpZ || op == OpJmpNZ);

  Offset takenOffset    = i.offset() + i.imm[0].u_BA;
  Offset fallthruOffset = i.offset() + instrLen((Op*)(i.pc()));

  auto jmpFlags = instrJmpFlags(i);

  if (i.nextOffset == takenOffset) {
    // invert the branch
    if (op == OpJmpZ) {
      HHIR_EMIT(JmpNZ, fallthruOffset, jmpFlags);
    } else {
      HHIR_EMIT(JmpZ,  fallthruOffset, jmpFlags);
    }
    if (i.nextOffset != takenOffset) {
      always_assert(RuntimeOption::EvalJitPGORegionSelector == "wholecfg");
      HHIR_EMIT(Jmp, takenOffset, jmpFlags);
    }
    return;
  }

  if (op == OpJmpZ) {
    HHIR_EMIT(JmpZ,  takenOffset, jmpFlags);
  } else {
    HHIR_EMIT(JmpNZ, takenOffset, jmpFlags);
  }
}

void
IRTranslator::translateAssignToLocalOp(const NormalizedInstruction& ni) {
  auto const op = ni.op();
  assert(op == OpSetL || op == OpBindL);

  if (op == OpSetL) {
    HHIR_EMIT(SetL, ni.imm[0].u_LA);
  } else {
    HHIR_EMIT(BindL, ni.imm[0].u_LA);
  }
}

void
IRTranslator::translateInitProp(const NormalizedInstruction& i) {
  HHIR_EMIT(InitProp, i.imm[0].u_SA, static_cast<InitPropOp>(i.imm[1].u_OA));
}

void IRTranslator::translateAssertRATL(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertRATL, i.imm[0].u_IVA, i.imm[1].u_RATA);
}

void IRTranslator::translateAssertRATStk(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertRATStk, i.imm[0].u_IVA, i.imm[1].u_RATA);
}

void IRTranslator::translateBreakTraceHint(const NormalizedInstruction&) {
}

void
IRTranslator::translateClsCnsD(const NormalizedInstruction& i) {
  HHIR_EMIT(ClsCnsD, (i.imm[0].u_SA), (i.imm[1].u_SA), i.outPred);
}

void
IRTranslator::translateAdd(const NormalizedInstruction& i) {
  auto leftType = m_hhbcTrans.topType(1);
  auto rightType = m_hhbcTrans.topType(0);
  if (leftType <= Type::Arr && rightType <= Type::Arr) {
    HHIR_EMIT(ArrayAdd);
  } else {
    HHIR_EMIT(Add);
  }
}

void
IRTranslator::translateAddO(const NormalizedInstruction& i) {
  auto leftType = m_hhbcTrans.topType(1);
  auto rightType = m_hhbcTrans.topType(0);
  if (leftType <= Type::Arr && rightType <= Type::Arr) {
    HHIR_EMIT(ArrayAdd);
  } else {
    HHIR_EMIT(AddO);
  }
}

void IRTranslator::translateConcatN(const NormalizedInstruction& i) {
  HHIR_EMIT(ConcatN, i.imm[0].u_IVA);
}

void IRTranslator::translateJmp(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA,
            instrJmpFlags(i) | JmpFlagSurprise);
}

void IRTranslator::translateJmpNS(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, instrJmpFlags(i));
}

void
IRTranslator::translateSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(Switch, i.immVec, i.imm[1].u_I64A, i.imm[2].u_IVA);
}

void
IRTranslator::translateSSwitch(const NormalizedInstruction& i) {
  HHIR_EMIT(SSwitch, i.immVec);
}

/*
 * translateRetC --
 *
 *   Return to caller with the current activation record replaced with the
 *   top-of-stack return value.
 */
void
IRTranslator::translateRetC(const NormalizedInstruction& i) {
  HHIR_EMIT(RetC, i.inlineReturn);
}

void
IRTranslator::translateRetV(const NormalizedInstruction& i) {
  HHIR_EMIT(RetV, i.inlineReturn);
}

void IRTranslator::translateCreateCont(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCont, i.nextSk().offset());
}

void IRTranslator::translateContEnter(const NormalizedInstruction& i) {
  HHIR_EMIT(ContEnter, i.nextSk().offset());
}

void IRTranslator::translateContRaise(const NormalizedInstruction& i) {
  HHIR_UNIMPLEMENTED(ContRaise);
}

void IRTranslator::translateYield(const NormalizedInstruction& i) {
  HHIR_EMIT(Yield, i.nextSk().offset());
}

void IRTranslator::translateYieldK(const NormalizedInstruction& i) {
  HHIR_EMIT(YieldK, i.nextSk().offset());
}

void IRTranslator::translateAwait(const NormalizedInstruction& i) {
  HHIR_EMIT(Await, i.nextSk().offset(), i.imm[0].u_IVA);
}

void IRTranslator::translateIncStat(const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA, false);
}

void IRTranslator::translateFPassL(const NormalizedInstruction& ni) {
  auto locId = ni.imm[1].u_LA;
  if (ni.preppedByRef) {
    HHIR_EMIT(VGetL, locId);
  } else {
    HHIR_EMIT(FPassL, locId);
  }
}

void IRTranslator::translateFPassS(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    unpackVGetS(nullptr, ni);
  } else {
    unpackCGetS(nullptr, ni);
  }
}

void IRTranslator::translateFPassG(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    unpackVGetG(nullptr, ni);
  } else {
    unpackCGetG(nullptr, ni);
  }
}

static inline DataType typeOpToDataType(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:  return KindOfNull;
  case IsTypeOp::Int:   return KindOfInt64;
  case IsTypeOp::Dbl:   return KindOfDouble;
  case IsTypeOp::Bool:  return KindOfBoolean;
  case IsTypeOp::Str:   return KindOfString;
  case IsTypeOp::Arr:   return KindOfArray;
  case IsTypeOp::Obj:   return KindOfObject;
  case IsTypeOp::Scalar: not_reached();
  }
  not_reached();
}

void
IRTranslator::translateCheckTypeLOp(const NormalizedInstruction& ni) {
  auto const locId = ni.imm[0].u_LA;
  auto const op    = static_cast<IsTypeOp>(ni.imm[1].u_OA);
  if (op == IsTypeOp::Scalar) {
    HHIR_EMIT(IsScalarL, locId);
  } else {
    DataType t = typeOpToDataType(op);
    HHIR_EMIT(IsTypeL, locId, t);
  }
}

void
IRTranslator::translateCheckTypeCOp(const NormalizedInstruction& ni) {
  auto const op = static_cast<IsTypeOp>(ni.imm[0].u_OA);
  if (op == IsTypeOp::Scalar) {
    HHIR_EMIT(IsScalarC);
  } else {
    DataType t = typeOpToDataType(op);
    HHIR_EMIT(IsTypeC, t);
  }
}

void
IRTranslator::translateSetOpL(const NormalizedInstruction& i) {
  auto const opc = [&] {
    switch (static_cast<SetOpOp>(i.imm[1].u_OA)) {
    case SetOpOp::PlusEqual:   return Op::Add;
    case SetOpOp::MinusEqual:  return Op::Sub;
    case SetOpOp::MulEqual:    return Op::Mul;
    case SetOpOp::PlusEqualO:  return Op::AddO;
    case SetOpOp::MinusEqualO: return Op::SubO;
    case SetOpOp::MulEqualO:   return Op::MulO;
    case SetOpOp::DivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpOp::ConcatEqual: return Op::Concat;
    case SetOpOp::ModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpOp::PowEqual:    HHIR_UNIMPLEMENTED(SetOpL_Pow);;
    case SetOpOp::AndEqual:    return Op::BitAnd;
    case SetOpOp::OrEqual:     return Op::BitOr;
    case SetOpOp::XorEqual:    return Op::BitXor;
    case SetOpOp::SlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpOp::SrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    }
    not_reached();
  }();
  HHIR_EMIT(SetOpL, opc, i.imm[0].u_LA);
}

void
IRTranslator::translateIncDecL(const NormalizedInstruction& i) {
  auto const op = static_cast<IncDecOp>(i.imm[1].u_OA);
  HHIR_EMIT(IncDecL, isPre(op), isInc(op), isIncDecO(op), i.imm[0].u_LA);
}

void IRTranslator::translateDefCls(const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void
IRTranslator::translateFPassCOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  always_assert(op == OpFPassCW || op == OpFPassCE);

  // These cases might have to raise a warning or an error
  HHIR_UNIMPLEMENTED_WHEN(i.preppedByRef, FPassCW_FPassCE_byref);

  // Nothing to do otherwise.
}

void
IRTranslator::translateFPassV(const NormalizedInstruction& i) {
  if (i.preppedByRef) {
    TRACE(1, "HHIR: translateFPassV: noOp\n");
    return;
  }
  HHIR_EMIT(FPassV);
}

void
IRTranslator::translateFPushCufOp(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCufOp, i.op(), i.imm[0].u_IVA);
}

void
IRTranslator::translateFPassR(const NormalizedInstruction& i) {
  /*
   * Like FPassC, FPassR is able to cheat on boxing if the current
   * parameter is pass by reference but we have a cell: the box would refer
   * to exactly one datum (the value currently on the stack).
   *
   * However, if the callee wants a cell and we have a variant we must
   * unbox; otherwise we might accidentally make callee changes to its
   * parameter globally visible.
   */
  if (!i.preppedByRef) {
    HHIR_EMIT(FPassR);
  } else {
    HHIR_UNIMPLEMENTED(FPassR);
  }
}

void
IRTranslator::translateFCallBuiltin(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  int numNonDefault  = i.imm[1].u_IVA;
  Id funcId = i.imm[2].u_SA;

  HHIR_EMIT(FCallBuiltin, numArgs, numNonDefault, funcId,
            jit::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void
IRTranslator::translateFCall(const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  const PC after = m_hhbcTrans.curUnit()->at(i.nextSk().offset());
  const Func* srcFunc = m_hhbcTrans.curFunc();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  HHIR_EMIT(FCall, numArgs, returnBcOffset, i.funcd,
            jit::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void IRTranslator::translateFCallD(const NormalizedInstruction& i) {
  translateFCall(i);
}

void
IRTranslator::translateFCallArray(const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = i.nextSk();
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after,
            jit::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void
IRTranslator::translateNewStructArray(const NormalizedInstruction& i) {
  auto numArgs = i.immVec.size();
  auto ids = i.immVec.vec32();
  auto unit = m_hhbcTrans.curUnit();
  StringData* keys[MixedArray::MaxMakeSize];
  for (size_t i = 0; i < numArgs; i++) {
    keys[i] = unit->lookupLitstrId(ids[i]);
  }
  HHIR_EMIT(NewStructArray, numArgs, keys);
}

/*
 * This function returns the offset of instruction i's branch target.
 * This is normally the offset corresponding to the branch being
 * taken.  However, if i does not break a trace and it's followed in
 * the trace by the instruction in the taken branch, then this
 * function returns the offset of the i's fall-through instruction.
 * In that case, the invertCond output argument is set to true;
 * otherwise it's set to false.
 */
static Offset getBranchTarget(const NormalizedInstruction& i,
                              bool& invertCond) {
  assert(instrJumpOffset((Op*)(i.pc())) != nullptr);
  Offset targetOffset = i.offset() + i.imm[1].u_BA;
  invertCond = false;

  if (!i.endsRegion && i.nextOffset == targetOffset) {
    invertCond = true;
    Offset fallthruOffset = i.offset() + instrLen((Op*)i.pc());
    targetOffset = fallthruOffset;
  }

  return targetOffset;
}

void
IRTranslator::translateIterInit(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterInit,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateIterInitK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterInitK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateIterNextK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterNextK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateMIterInit(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            instrJmpFlags(i));
}

void
IRTranslator::translateMIterInitK(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            instrJmpFlags(i));
}

void
IRTranslator::translateMIterNext(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            instrJmpFlags(i));
}

void
IRTranslator::translateMIterNextK(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            instrJmpFlags(i));
}

void
IRTranslator::translateWIterInit(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterInit,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateWIterInitK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterInitK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateWIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateWIterNextK(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterNextK,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA,
            invertCond,
            instrJmpFlags(i));
}

void
IRTranslator::translateIterBreak(const NormalizedInstruction& i) {

  assert(i.endsRegion);
  HHIR_EMIT(IterBreak, i.immVec, i.offset() + i.imm[1].u_BA, i.endsRegion);
}

void
IRTranslator::translateDecodeCufIter(const NormalizedInstruction& i) {

  HHIR_EMIT(DecodeCufIter, i.imm[0].u_IVA, i.offset() + i.imm[1].u_BA,
            instrJmpFlags(i));
}

bool
IRTranslator::tryTranslateSingletonInline(const NormalizedInstruction& i,
                                          const Func* funcd) {
  using Atom = BCPattern::Atom;
  using Captures = BCPattern::CaptureVec;

  if (!funcd) return false;

  // Make sure we have an acceptable FPush and non-null callee.
  assert(i.op() == Op::FPushFuncD ||
         i.op() == Op::FPushClsMethodD);

  auto fcall = i.nextSk();

  // Check if the next instruction is an acceptable FCall.
  if ((fcall.op() != Op::FCall && fcall.op() != Op::FCallD) ||
      funcd->isResumable() || funcd->isReturnRef()) {
    return false;
  }

  // First, check for the static local singleton pattern...

  // Lambda to check if CGetL and StaticLocInit refer to the same local.
  auto has_same_local = [] (PC pc, const Captures& captures) {
    if (captures.size() == 0) return false;

    auto cgetl = (const Op*)pc;
    auto sli = (const Op*)captures[0];

    assert(*cgetl == Op::CGetL);
    assert(*sli == Op::StaticLocInit);

    return (getImm(sli, 0).u_IVA == getImm(cgetl, 0).u_IVA);
  };

  auto cgetl = Atom(Op::CGetL).onlyif(has_same_local);
  auto retc  = Atom(Op::RetC);

  // Look for a static local singleton pattern.
  auto result = BCPattern {
    Atom(Op::Null),
    Atom(Op::StaticLocInit).capture(),
    Atom(Op::IsTypeL),
    Atom::alt(
      Atom(Op::JmpZ).taken({cgetl, retc}),
      Atom::seq(Atom(Op::JmpNZ), cgetl, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      hhbcTrans().emitSingletonSLoc(
        funcd,
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sloc] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  // Not found; check for the static property pattern.

  // Factory for String atoms that are required to match another captured
  // String opcode.
  auto same_string_as = [&] (int i) {
    return Atom(Op::String).onlyif([=] (PC pc, const Captures& captures) {
      auto string1 = (const Op*)pc;
      auto string2 = (const Op*)captures[i];
      assert(*string1 == Op::String);
      assert(*string2 == Op::String);

      auto const unit = funcd->unit();
      auto sd1 = unit->lookupLitstrId(getImmPtr(string1, 0)->u_SA);
      auto sd2 = unit->lookupLitstrId(getImmPtr(string2, 0)->u_SA);

      return (sd1 && sd1 == sd2);
    });
  };

  auto stringProp = same_string_as(0);
  auto stringCls  = same_string_as(1);
  auto agetc = Atom(Op::AGetC);
  auto cgets = Atom(Op::CGetS);

  // Look for a class static singleton pattern.
  result = BCPattern {
    Atom(Op::String).capture(),
    Atom(Op::String).capture(),
    Atom(Op::AGetC),
    Atom(Op::CGetS),
    Atom(Op::IsTypeC),
    Atom::alt(
      Atom(Op::JmpZ).taken({stringProp, stringCls, agetc, cgets, retc}),
      Atom::seq(Atom(Op::JmpNZ), stringProp, stringCls, agetc, cgets, retc)
    )
  }.ignore(
    {Op::AssertRATL, Op::AssertRATStk}
  ).matchAnchored(funcd);

  if (result.found()) {
    try {
      hhbcTrans().emitSingletonSProp(
        funcd,
        (const Op*)result.getCapture(1),
        (const Op*)result.getCapture(0)
      );
    } catch (const FailedIRGen& e) {
      return false;
    } catch (const FailedCodeGen& e) {
      return false;
    }
    TRACE(1, "[singleton-sprop] %s <- %s\n",
        funcd->fullName()->data(),
        fcall.func()->fullName()->data());
    return true;
  }

  return false;
}

/*
 * Generate HhbcTranslator method callers for all bytecodes, using its
 * table-defined signature.
 *
 * The static_cast is to make it so that the invalid emit##nm call is due to
 * template parameter substitution failure and thus Not An Error.
 */
#define O(nm, imms, pop, push, flags) \
  template<class HT> \
  typename std::enable_if<HT::supports##nm, void>::type \
  IRTranslator::unpack##nm(std::nullptr_t, const NormalizedInstruction& ni) { \
    static_cast<HT&>(m_hhbcTrans).emit##nm(imms);  \
  }
#define NA /**/
#define ONE(T) ni.imm[0].u_##T
#define TWO(T1, T2) ni.imm[0].u_##T1, ni.imm[1].u_##T2
#define THREE(T1, T2, T3) \
  ni.imm[0].u_##T1, ni.imm[1].u_##T2, ni.imm[2].u_##T3
#define FOUR(T1, T2, T3, T4) \
  ni.imm[0].u_##T1, ni.imm[1].u_##T2, ni.imm[2].u_##T3, ni.imm[3].u_##T4
#define u_OA(_) u_OA

OPCODES

#undef FOUR
#undef THREE
#undef TWO
#undef ONE
#undef NA
#undef u_OA
#undef O

// All vector instructions are handled by one HhbcTranslator method.
#define MII(instr, ...)                                                 \
  void IRTranslator::translate##instr##M(const NormalizedInstruction& ni) { \
    m_hhbcTrans.emitMInstr(ni);                                        \
  }
MINSTRS
MII(FPass)
#undef MII

void
IRTranslator::translateInstrWork(const NormalizedInstruction& i) {
  auto const op = i.op();

  switch (op) {
#define CASE(iNm) \
    case Op::iNm: return unpack ## iNm(nullptr, i);

    REGULAR_INSTRS
#undef CASE

#define CASE(nm) \
    case Op::nm: return translate ## nm(i); break;
#define TRANSLATE(name, inst) translate ## name(i); break;
    IRREGULAR_INSTRS
    PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
    default:
      always_assert(false);
  }
}

void IRTranslator::interpretInstr(const NormalizedInstruction& i) {
  FTRACE(5, "HHIR: BC Instr {}\n",  i.toString());
  m_hhbcTrans.emitInterpOne(i);
}

static Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return Type::Cell;  // TODO(#3029148) this could be InitCell
    case UV: return Type::Uninit;
    case VV: return Type::BoxedCell;
    case AV: return Type::Cls;
    case RV: case FV: case CVV: case CVUV: return Type::Gen;
  }
  not_reached();
}

void IRTranslator::translateInstr(const NormalizedInstruction& ni) {
  auto& ht = m_hhbcTrans;
  ht.setBcOff(ni.source.offset(),
              ni.endsRegion && !m_hhbcTrans.isInlining());
  FTRACE(1, "\n{:-^60}\n", folly::format("Translating {}: {} with stack:\n{}",
                                         ni.offset(), ni.toString(),
                                         ht.showStack()));
  // When profiling, we disable type predictions to avoid side exits
  assert(IMPLIES(mcg->tx().mode() == TransKind::Profile, !ni.outputPredicted));

  ht.emitRB(RBTypeBytecodeStart, ni.source, 2);

  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    if (type != Type::Gen) m_hhbcTrans.assertTypeStack(i, type);
  }

  if (RuntimeOption::EvalHHIRGenerateAsserts >= 2) {
    ht.emitDbgAssertRetAddr();
  }

  if (isAlwaysNop(ni.op())) {
    // Do nothing
  } else if (instrMustInterp(ni) || ni.interp) {
    interpretInstr(ni);
  } else {
    translateInstrWork(ni);
  }
}

}}
