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

#include "hphp/runtime/vm/jit/ir-translator.h"

#include <stdint.h>
#include "hphp/runtime/base/strings.h"

#include "folly/Format.h"
#include "folly/Conv.h"
#include "hphp/util/trace.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/util.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/tracelet.h"

// Include last to localize effects to this file
#include "hphp/util/assert-throw.h"

namespace HPHP {
namespace JIT {

using namespace reg;
using namespace Util;
using namespace Trace;
using std::max;

TRACE_SET_MOD(hhir);
#ifdef DEBUG
static const bool debug = true;
#else
static const bool debug = false;
#endif

#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)

#define HHIR_EMIT(op, ...)                      \
  do {                                          \
    m_hhbcTrans.emit ## op(__VA_ARGS__);        \
    return;                                     \
  } while (0)

#define HHIR_UNIMPLEMENTED_OP(op)                       \
  do {                                                  \
    throw JIT::FailedIRGen(__FILE__, __LINE__, op);     \
  } while (0)


#define HHIR_UNIMPLEMENTED(op)                          \
  do {                                                  \
    throw JIT::FailedIRGen(__FILE__, __LINE__, #op);    \
  } while (0)

#define HHIR_UNIMPLEMENTED_WHEN(expr, op)               \
  do {                                                  \
    if (expr) {                                         \
      throw JIT::FailedIRGen(__FILE__, __LINE__, #op);  \
    }                                                   \
  } while (0)

IRTranslator::IRTranslator(Offset bcOff, Offset spOff, const Func* curFunc)
  : m_hhbcTrans(bcOff, spOff, curFunc)
{
}

void IRTranslator::checkType(const JIT::Location& l,
                             const JIT::RuntimeType& rtt,
                             bool outerOnly) {
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue() || l.isThis()) return;

  switch (l.space) {
    case Location::Stack: {
      uint32_t stackOffset = locPhysicalOffset(l);
      JIT::Type type = JIT::Type(rtt);
      if (type <= Type::Cls) {
        m_hhbcTrans.assertTypeStack(stackOffset, type);
      } else {
        m_hhbcTrans.guardTypeStack(stackOffset, type, outerOnly);
      }
      break;
    }
    case Location::Local:
      m_hhbcTrans.guardTypeLocal(l.offset, Type(rtt), outerOnly);
      break;

    case Location::Iter:
    case Location::Invalid:
    case Location::Litstr:
    case Location::Litint:
    case Location::This:
      not_reached();
  }
}

void IRTranslator::assertType(const JIT::Location& l,
                              const JIT::RuntimeType& rtt) {
  if (rtt.isVagueValue()) return;

  switch (l.space) {
    case Location::Stack: {
      // tx64LocPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32_t stackOffset = locPhysicalOffset(l);
      m_hhbcTrans.assertTypeStack(stackOffset, Type(rtt));
      break;
    }
    case Location::Local:  // Stack frame's registers; offset == local register
      m_hhbcTrans.assertTypeLocal(l.offset, Type(rtt));
      break;

    case Location::Invalid:           // Unknown location
      HHIR_UNIMPLEMENTED(Invalid);
      break;

    case Location::Iter:              // Stack frame's iterators
      HHIR_UNIMPLEMENTED(AssertType_Iter);
      break;

    case Location::Litstr:            // Literal string pseudo-location
      HHIR_UNIMPLEMENTED(AssertType_Litstr);
      break;

    case Location::Litint:            // Literal int pseudo-location
      HHIR_UNIMPLEMENTED(AssertType_Litint);
      break;

    case Location::This:
      HHIR_UNIMPLEMENTED(AssertType_This);
      break;
  }
}

void
IRTranslator::translateMod(const NormalizedInstruction& i) {
  HHIR_EMIT(Mod);
}


void
IRTranslator::translateDiv(const NormalizedInstruction& i) {
  HHIR_EMIT(Div);
}

void
IRTranslator::translateBinaryArithOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  switch (op) {
#define CASE(OpBc) case Op::OpBc: HHIR_EMIT(OpBc);
    CASE(Add)
    CASE(Sub)
    CASE(BitAnd)
    CASE(BitOr)
    CASE(BitXor)
    CASE(Mul)
#undef CASE
    default: {
      not_reached();
    };
  }
  NOT_REACHED();
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
  bool ok = equivDataTypes(leftType.toDataType(), rightType.toDataType()) &&
    leftType.subtypeOfAny(Type::Null, Type::Bool, Type::Int);

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
  assert(i.breaksTracelet ||
         i.nextOffset == takenOffset ||
         i.nextOffset == fallthruOffset);

  if (!i.breaksTracelet && takenOffset == fallthruOffset) return;

  if (i.breaksTracelet || i.nextOffset == fallthruOffset) {
    if (op == OpJmpZ) {
      HHIR_EMIT(JmpZ,  takenOffset);
    } else {
      HHIR_EMIT(JmpNZ, takenOffset);
    }
    return;
  }
  assert(i.nextOffset == takenOffset);
  // invert the branch
  if (op == OpJmpZ) {
    HHIR_EMIT(JmpNZ, fallthruOffset);
  } else {
    HHIR_EMIT(JmpZ,  fallthruOffset);
  }
}

void
IRTranslator::translateCGetL(const NormalizedInstruction& i) {
  HHIR_EMIT(CGetL, i.imm[0].u_LA);
}

void
IRTranslator::translatePushL(const NormalizedInstruction& ni) {
  HHIR_EMIT(PushL, ni.imm[0].u_LA);
}

void
IRTranslator::translateCGetL2(const NormalizedInstruction& ni) {
  HHIR_EMIT(CGetL2, ni.imm[0].u_LA);
}

void
IRTranslator::translateVGetL(const NormalizedInstruction& i) {
  HHIR_EMIT(VGetL, i.imm[0].u_LA);
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

void IRTranslator::translatePopA(const NormalizedInstruction&) {
  HHIR_EMIT(PopA);
}

void
IRTranslator::translatePopC(const NormalizedInstruction& i) {
  HHIR_EMIT(PopC);
}

void
IRTranslator::translatePopV(const NormalizedInstruction& i) {
  HHIR_EMIT(PopV);
}

void
IRTranslator::translatePopR(const NormalizedInstruction& i) {
  HHIR_EMIT(PopR);
}

void
IRTranslator::translateUnboxR(const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: translateUnboxR: output inferred to be Cell\n");
    m_hhbcTrans.assertTypeStack(0, JIT::Type::Cell);
  } else {
    HHIR_EMIT(UnboxR);
  }
}

void IRTranslator::translateUnboxRNop(const NormalizedInstruction& i) {
  TRACE(1, "HHIR: translateUnboxR: output inferred to be Cell\n");
  assert(i.noOp);
  m_hhbcTrans.assertTypeStack(0, JIT::Type::Cell);
}

void
IRTranslator::translateBoxR(const NormalizedInstruction& i) {
  if (i.noOp) {
    // statically proved to be unboxed -- just pass that info to the IR
    TRACE(1, "HHIR: translateBoxR: output inferred to be Box\n");
    m_hhbcTrans.assertTypeStack(0, JIT::Type::BoxedCell);
  } else {
    HHIR_UNIMPLEMENTED(BoxR);
  }
}

void IRTranslator::translateBoxRNop(const NormalizedInstruction& i) {
  assert(i.noOp);
  m_hhbcTrans.assertTypeStack(0, JIT::Type::BoxedCell);
}

void
IRTranslator::translateNull(const NormalizedInstruction& i) {
  HHIR_EMIT(Null);
}

void
IRTranslator::translateNullUninit(const NormalizedInstruction& i) {
  HHIR_EMIT(NullUninit);
}

void
IRTranslator::translateTrue(const NormalizedInstruction& i) {
  HHIR_EMIT(True);
}

void
IRTranslator::translateFalse(const NormalizedInstruction& i) {
  HHIR_EMIT(False);
}

void
IRTranslator::translateInt(const NormalizedInstruction& i) {
  HHIR_EMIT(Int, i.imm[0].u_I64A);
}

void
IRTranslator::translateDouble(const NormalizedInstruction& i) {
  HHIR_EMIT(Double, i.imm[0].u_DA);
}

void
IRTranslator::translateString(const NormalizedInstruction& i) {
  HHIR_EMIT(String, (i.imm[0].u_SA));
}

void
IRTranslator::translateArray(const NormalizedInstruction& i) {
  HHIR_EMIT(Array, i.imm[0].u_AA);
}

void
IRTranslator::translateNewArray(const NormalizedInstruction& i) {
  HHIR_EMIT(NewArrayReserve, 0);
}

void
IRTranslator::translateNewArrayReserve(const NormalizedInstruction& i) {
  HHIR_EMIT(NewArrayReserve, i.imm[0].u_IVA);
}

void
IRTranslator::translateNop(const NormalizedInstruction& i) {
  HHIR_EMIT(Nop);
}

void
IRTranslator::translateAddElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(AddElemC);
}

void
IRTranslator::translateFloor(const NormalizedInstruction& i) {
  HHIR_EMIT(Floor);
}

void
IRTranslator::translateCeil(const NormalizedInstruction& i) {
  HHIR_EMIT(Ceil);
}

void IRTranslator::translateAssertTL(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertTL, i.imm[0].u_LA, static_cast<AssertTOp>(i.imm[1].u_OA));
}

void IRTranslator::translateAssertTStk(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertTStk, i.imm[0].u_IVA, static_cast<AssertTOp>(i.imm[1].u_OA));
}

void IRTranslator::translateAssertObjL(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertObjL, i.imm[0].u_LA, i.imm[1].u_IVA, i.imm[2].u_SA);
}

void IRTranslator::translateAssertObjStk(const NormalizedInstruction& i) {
  HHIR_EMIT(AssertObjStk, i.imm[0].u_IVA, i.imm[1].u_IVA, i.imm[2].u_SA);
}

void IRTranslator::translatePredictTL(const NormalizedInstruction& i) {
  HHIR_EMIT(PredictTL, i.imm[0].u_LA, static_cast<AssertTOp>(i.imm[1].u_OA));
}

void IRTranslator::translatePredictTStk(const NormalizedInstruction& i) {
  HHIR_EMIT(PredictTStk, i.imm[0].u_IVA, static_cast<AssertTOp>(i.imm[1].u_OA));
}

void IRTranslator::translateBreakTraceHint(const NormalizedInstruction&) {
}

void
IRTranslator::translateAddNewElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(AddNewElemC);
}

void
IRTranslator::translateCns(const NormalizedInstruction& i) {
  HHIR_EMIT(Cns, i.imm[0].u_SA);
}

void
IRTranslator::translateCnsE(const NormalizedInstruction& i) {
  HHIR_EMIT(CnsE, i.imm[0].u_SA);
}

void
IRTranslator::translateCnsU(const NormalizedInstruction& i) {
  HHIR_EMIT(CnsU, i.imm[0].u_SA, i.imm[1].u_SA);
}

void
IRTranslator::translateDefCns(const NormalizedInstruction& i) {
  HHIR_EMIT(DefCns, (i.imm[0].u_SA));
}

void
IRTranslator::translateClsCnsD(const NormalizedInstruction& i) {
  HHIR_EMIT(ClsCnsD, (i.imm[0].u_SA), (i.imm[1].u_SA), i.outPred);
}

void
IRTranslator::translateConcat(const NormalizedInstruction& i) {
  HHIR_EMIT(Concat);
}

void
IRTranslator::translateAdd(const NormalizedInstruction& i) {
  auto leftType = m_hhbcTrans.topType(1);
  auto rightType = m_hhbcTrans.topType(0);
  if (leftType.isArray() && rightType.isArray()) {
    HHIR_EMIT(ArrayAdd);
  } else {
    HHIR_EMIT(Add);
  }
}

void
IRTranslator::translateSqrt(const NormalizedInstruction& i) {
  HHIR_EMIT(Sqrt);
}

void
IRTranslator::translateAbs(const NormalizedInstruction& i) {
  HHIR_EMIT(Abs);
}

void
IRTranslator::translateXor(const NormalizedInstruction& i) {
  HHIR_EMIT(Xor);
}

void
IRTranslator::translateNot(const NormalizedInstruction& i) {
  HHIR_EMIT(Not);
}

void
IRTranslator::translateBitNot(const NormalizedInstruction& i) {
  HHIR_EMIT(BitNot);
}

void
IRTranslator::translateShl(const NormalizedInstruction& i) {
  HHIR_EMIT(Shl);
}

void
IRTranslator::translateShr(const NormalizedInstruction& i) {
  HHIR_EMIT(Shr);
}

void
IRTranslator::translateCastInt(const NormalizedInstruction& i) {
  HHIR_EMIT(CastInt);
}

void
IRTranslator::translateCastArray(const NormalizedInstruction& i) {
  HHIR_EMIT(CastArray);
}

void
IRTranslator::translateCastObject(const NormalizedInstruction& i) {
  HHIR_EMIT(CastObject);
}

void
IRTranslator::translateCastDouble(const NormalizedInstruction& i) {
  HHIR_EMIT(CastDouble);
}

void
IRTranslator::translateCastString(const NormalizedInstruction& i) {
  HHIR_EMIT(CastString);
}

void
IRTranslator::translatePrint(const NormalizedInstruction& i) {
  HHIR_EMIT(Print);
}

void IRTranslator::translateJmp(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, false);
}

void IRTranslator::translateJmpNS(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, true);
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

void
IRTranslator::translateNativeImpl(const NormalizedInstruction& ni) {
  HHIR_EMIT(NativeImpl);
}

void IRTranslator::translateAGetC(const NormalizedInstruction& ni) {
  HHIR_EMIT(AGetC);
}

void IRTranslator::translateAGetL(const NormalizedInstruction& i) {
  HHIR_EMIT(AGetL, i.imm[0].u_LA);
}

void IRTranslator::translateSelf(const NormalizedInstruction& i) {
  HHIR_EMIT(Self);
}

void IRTranslator::translateParent(const NormalizedInstruction& i) {
  HHIR_EMIT(Parent);
}

void IRTranslator::translateDup(const NormalizedInstruction& ni) {
  HHIR_EMIT(Dup);
}

void IRTranslator::translateCreateCont(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCont);
}

void IRTranslator::translateContEnter(const NormalizedInstruction& i) {
  auto after = i.nextSk().offset();

  const Func* srcFunc = m_hhbcTrans.curFunc();
  int32_t callOffsetInUnit = after - srcFunc->base();

  HHIR_EMIT(ContEnter, callOffsetInUnit);
}

void IRTranslator::translateUnpackCont(const NormalizedInstruction& i) {
  HHIR_EMIT(UnpackCont);
}

void IRTranslator::translateContSuspend(const NormalizedInstruction& i) {
  HHIR_EMIT(ContSuspend, i.imm[0].u_IVA);
}

void IRTranslator::translateContSuspendK(const NormalizedInstruction& i) {
  HHIR_EMIT(ContSuspendK, i.imm[0].u_IVA);
}

void IRTranslator::translateContRetC(const NormalizedInstruction& i) {
  HHIR_EMIT(ContRetC);
}

void IRTranslator::translateContCheck(const NormalizedInstruction& i) {
  HHIR_EMIT(ContCheck, i.imm[0].u_IVA);
}

void IRTranslator::translateContRaise(const NormalizedInstruction& i) {
  HHIR_EMIT(ContRaise);
}

void IRTranslator::translateContValid(const NormalizedInstruction& i) {
  HHIR_EMIT(ContValid);
}

void IRTranslator::translateContKey(const NormalizedInstruction& i) {
  HHIR_EMIT(ContKey);
}

void IRTranslator::translateContCurrent(const NormalizedInstruction& i) {
  HHIR_EMIT(ContCurrent);
}

void IRTranslator::translateContStopped(const NormalizedInstruction& i) {
  HHIR_EMIT(ContStopped);
}

void IRTranslator::translateAsyncAwait(const NormalizedInstruction&) {
  HHIR_EMIT(AsyncAwait);
}

void IRTranslator::translateAsyncESuspend(const NormalizedInstruction& i) {
  HHIR_EMIT(AsyncESuspend, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void IRTranslator::translateAsyncWrapResult(const NormalizedInstruction& i) {
  HHIR_EMIT(AsyncWrapResult);
}

void IRTranslator::translateAsyncWrapException(const NormalizedInstruction& i) {
  HHIR_EMIT(AsyncWrapException);
}

void IRTranslator::translateStrlen(const NormalizedInstruction& i) {
  HHIR_EMIT(Strlen);
}

void IRTranslator::translateIncStat(const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void IRTranslator::translateIdx(const NormalizedInstruction& i) {
  HHIR_EMIT(Idx);
}

void IRTranslator::translateArrayIdx(const NormalizedInstruction& i) {
  HHIR_EMIT(ArrayIdx);
}

void IRTranslator::translateClassExists(const NormalizedInstruction& i) {
  HHIR_EMIT(ClassExists);
}

void IRTranslator::translateInterfaceExists(const NormalizedInstruction& i) {
  HHIR_EMIT(InterfaceExists);
}

void IRTranslator::translateTraitExists(const NormalizedInstruction& i) {
  HHIR_EMIT(TraitExists);
}

void IRTranslator::translateVGetS(const NormalizedInstruction& i) {
  HHIR_EMIT(VGetS);
}

void
IRTranslator::translateVGetG(const NormalizedInstruction& i) {
  HHIR_EMIT(VGetG);
}

void IRTranslator::translateBindS(const NormalizedInstruction& i) {
  HHIR_EMIT(BindS);
}

void IRTranslator::translateEmptyS(const NormalizedInstruction& i) {
  HHIR_EMIT(EmptyS);
}

void IRTranslator::translateEmptyG(const NormalizedInstruction& i) {
  HHIR_EMIT(EmptyG);
}

void
IRTranslator::translateIssetS(const NormalizedInstruction& i) {
  HHIR_EMIT(IssetS);
}

void
IRTranslator::translateIssetG(const NormalizedInstruction& i) {
  HHIR_EMIT(IssetG);
}

void IRTranslator::translateCGetS(const NormalizedInstruction& i) {
  HHIR_EMIT(CGetS);
}

void IRTranslator::translateSetS(const NormalizedInstruction& i) {
  HHIR_EMIT(SetS);
}

void
IRTranslator::translateCGetG(const NormalizedInstruction& i) {
  HHIR_EMIT(CGetG);
}

void IRTranslator::translateSetG(const NormalizedInstruction& i) {
  HHIR_EMIT(SetG);
}

void IRTranslator::translateBindG(const NormalizedInstruction& i) {
  HHIR_EMIT(BindG);
}

void
IRTranslator::translateLateBoundCls(const NormalizedInstruction&i) {
  HHIR_EMIT(LateBoundCls);
}

void IRTranslator::translateFPassL(const NormalizedInstruction& ni) {
  auto locId = ni.imm[1].u_LA;
  if (ni.preppedByRef) {
    HHIR_EMIT(VGetL, locId);
  } else {
    HHIR_EMIT(CGetL, locId);
  }
}

void IRTranslator::translateFPassS(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetS(ni);
  } else {
    translateCGetS(ni);
  }
}

void IRTranslator::translateFPassG(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetG(ni);
  } else {
    translateCGetG(ni);
  }
}

void
IRTranslator::translateIssetL(const NormalizedInstruction& ni) {
  HHIR_EMIT(IssetL, ni.imm[0].u_LA);
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
IRTranslator::translateAKExists(const NormalizedInstruction& ni) {
  HHIR_EMIT(AKExists);
}

void
IRTranslator::translateSetOpL(const NormalizedInstruction& i) {
  auto const opc = [&] {
    switch (static_cast<SetOpOp>(i.imm[1].u_OA)) {
    case SetOpOp::PlusEqual:   return Add;
    case SetOpOp::MinusEqual:  return Sub;
    case SetOpOp::MulEqual:    return Mul;
    case SetOpOp::DivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpOp::ConcatEqual: return ConcatCellCell;
    case SetOpOp::ModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpOp::AndEqual:    return BitAnd;
    case SetOpOp::OrEqual:     return BitOr;
    case SetOpOp::XorEqual:    return BitXor;
    case SetOpOp::SlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpOp::SrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    }
    not_reached();
  }();
  HHIR_EMIT(SetOpL, opc, i.imm[0].u_LA);
}

void
IRTranslator::translateIncDecL(const NormalizedInstruction& i) {
  const IncDecOp oplet = static_cast<IncDecOp>(i.imm[1].u_OA);
  bool post = (oplet == IncDecOp::PostInc || oplet == IncDecOp::PostDec);
  bool pre  = !post;
  bool inc  = (oplet == IncDecOp::PostInc || oplet == IncDecOp::PreInc);

  HHIR_EMIT(IncDecL, pre, inc, i.imm[0].u_LA);
}

void
IRTranslator::translateUnsetL(const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetL, i.imm[0].u_LA);
}

void IRTranslator::translateDefCls(const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void IRTranslator::translateNopDefCls(const NormalizedInstruction&) {}

void IRTranslator::translateDefFunc(const NormalizedInstruction& i) {
  int fid = i.imm[0].u_IVA;
  HHIR_EMIT(DefFunc, fid);
}

void
IRTranslator::translateFPushFunc(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFunc, (i.imm[0].u_IVA));
}

void
IRTranslator::translateFPushClsMethod(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethod, i.imm[0].u_IVA);
}

void
IRTranslator::translateFPushClsMethodD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethodD,
            i.imm[0].u_IVA, // num params
            i.imm[1].u_SA,  // method name
            i.imm[2].u_SA); // class name
}

void
IRTranslator::translateFPushClsMethodF(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethodF, i.imm[0].u_IVA /* # of arguments */);
}

void
IRTranslator::translateFPushObjMethodD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushObjMethodD,
            i.imm[0].u_IVA, // numParams
            i.imm[1].u_SA); // methodName
}

void IRTranslator::translateFPushCtor(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCtor, (i.imm[0].u_IVA));
}

void IRTranslator::translateFPushCtorD(const NormalizedInstruction& i) {

  HHIR_EMIT(FPushCtorD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void IRTranslator::translateCreateCl(const NormalizedInstruction& i) {
  HHIR_EMIT(CreateCl, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void
IRTranslator::translateThis(const NormalizedInstruction &i) {
  HHIR_EMIT(This);
}

void
IRTranslator::translateBareThis(const NormalizedInstruction &i) {
  HHIR_EMIT(BareThis, (i.imm[0].u_OA));
}

void
IRTranslator::translateCheckThis(const NormalizedInstruction& i) {
  HHIR_EMIT(CheckThis);
}

void
IRTranslator::translateInitThisLoc(const NormalizedInstruction& i) {
  HHIR_EMIT(InitThisLoc, i.imm[0].u_LA);
}

void
IRTranslator::translateFPushFuncD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFuncD, (i.imm[0].u_IVA), (i.imm[1].u_SA));
}

void
IRTranslator::translateFPushFuncU(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFuncU, i.imm[0].u_IVA, i.imm[1].u_SA, i.imm[2].u_SA);
}

void
IRTranslator::translateFPassCOp(const NormalizedInstruction& i) {
  auto const op = i.op();
  if (i.noOp) return;
  if (i.preppedByRef && (op == OpFPassCW || op == OpFPassCE)) {
    // These cases might have to raise a warning or an error
    HHIR_UNIMPLEMENTED(FPassCW_FPassCE_byref);
  } else {
    HHIR_EMIT(FPassCOp);
  }
}

void
IRTranslator::translateFPassV(const NormalizedInstruction& i) {
  if (i.preppedByRef || i.noOp) {
    TRACE(1, "HHIR: translateFPassV: noOp\n");
    return;
  }
  HHIR_EMIT(FPassV);
}

void IRTranslator::translateFPassVNop(const NormalizedInstruction& i) {
  assert(i.noOp);
}

void
IRTranslator::translateFPushCufIter(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCufIter, i.imm[0].u_IVA, i.imm[1].u_IA);
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
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

bool shouldIRInline(const Func* caller, const Func* callee, RegionIter& iter) {
  if (!RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return false;
  }
  if (arch() == Arch::ARM) {
    // TODO(#3331014): hack until more ARM codegen is working.
    return false;
  }

  auto refuse = [&](const char* why) -> bool {
    FTRACE(1, "shouldIRInline: refusing {} <reason: {}> [NI = {}]\n",
           callee->fullName()->data(), why,
           iter.finished() ? "<end>" : iter.sk().showInst());
    return false;
  };
  auto accept = [&](const char* kind) -> bool {
    FTRACE(1, "shouldIRInline: inlining {} <kind: {}>\n",
           callee->fullName()->data(), kind);
    return true;
  };

  if (callee->numIterators() != 0) {
    return refuse("iterators");
  }
  if (callee->isMagic() || Func::isSpecial(callee->name())) {
    return refuse("special or magic function");
  }
  if (callee->attrs() & AttrMayUseVV) {
    return refuse("may use dynamic environment");
  }
  if (callee->numSlotsInFrame() + callee->maxStackCells() >=
      kStackCheckLeafPadding) {
    return refuse("function stack depth too deep");
  }

  ////////////

  assert(!iter.finished() && "shouldIRInline given empty region");
  bool hotCallingCold = !(callee->attrs() & AttrHot) &&
                         (caller->attrs() & AttrHot);
  uint64_t cost = 0;
  int inlineDepth = 0;
  Op op = OpLowInvalid;
  smart::vector<const Func*> funcs;
  const Func* func = callee;
  funcs.push_back(func);

  for (; !iter.finished(); iter.advance()) {
    // If func has changed after an FCall, we've started an inlined call. This
    // will have to change when we support inlining recursive calls.
    if (func != iter.sk().func()) {
      assert(isRet(op) || op == OpFCall);
      if (op == OpFCall) {
        funcs.push_back(iter.sk().func());
        int totalDepth = 0;
        for (auto* f : funcs) {
          totalDepth += f->numSlotsInFrame() + f->maxStackCells();
        }
        if (totalDepth >= kStackCheckLeafPadding) {
          return refuse("stack too deep after nested inlining");
        }
        ++inlineDepth;
      }
    }
    op = iter.sk().op();
    func = iter.sk().func();

    // If we hit a RetC/V while inlining, leave that level and
    // continue. Otherwise, accept the tracelet.
    if (isRet(op)) {
      if (inlineDepth > 0) {
        --inlineDepth;
        funcs.pop_back();
        continue;
      } else {
        assert(inlineDepth == 0);
        return accept("entire function fits in one region");
      }
    }

    if (op == OpFCallArray) return refuse("FCallArray");

    cost += 1;

    // Check for an immediate vector, and if it's present add its size to the
    // cost.
    auto const pc = reinterpret_cast<const Op*>(iter.sk().pc());
    if (hasMVector(op)) {
      cost += getMVector(pc).size();
    } else if (hasImmVector(op)) {
      cost += getImmVector(pc).size();
    }

    if (cost > RuntimeOption::EvalHHIRInliningMaxCost) {
      return refuse("too expensive");
    }

    if (cost > RuntimeOption::EvalHHIRAlwaysInlineMaxCost &&
        hotCallingCold) {
      return refuse("inlining sizeable cold func into hot func");
    }

    if (JIT::opcodeBreaksBB(op)) {
      return refuse("breaks tracelet");
    }
  }

  return refuse("region doesn't end in RetC/RetV");
}

struct TraceletIter : public RegionIter {
  explicit TraceletIter(const Tracelet& tlet)
    : m_current(tlet.m_instrStream.first)
  {}

  bool finished() const { return m_current == nullptr; }

  SrcKey sk() const {
    assert(!finished());
    return m_current->source;
  }

  void advance() {
    assert(!finished());
    m_current = m_current->next;
  }

 private:
  const NormalizedInstruction* m_current;
};

bool shouldIRInline(const Func* caller, const Func* callee,
                    const Tracelet& tlet) {
  TraceletIter iter(tlet);
  return shouldIRInline(caller, callee, iter);
}

void
IRTranslator::translateFCall(const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  const PC after = m_hhbcTrans.curUnit()->at(i.nextSk().offset());
  const Func* srcFunc = m_hhbcTrans.curFunc();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  /*
   * If we have a calleeTrace, we're going to see if we should inline
   * the call.
   */
  if (i.calleeTrace) {
    if (!i.calleeTrace->m_inliningFailed) {
      assert(shouldIRInline(m_hhbcTrans.curFunc(), i.funcd, *i.calleeTrace));

      m_hhbcTrans.beginInlining(numArgs, i.funcd, returnBcOffset);
      static const bool shapeStats = Stats::enabledAny() &&
                                     getenv("HHVM_STATS_INLINESHAPE");
      if (shapeStats) {
        m_hhbcTrans.profileInlineFunctionShape(traceletShape(*i.calleeTrace));
      }

      Unit::MetaHandle metaHand;
      for (auto* ni = i.calleeTrace->m_instrStream.first;
           ni; ni = ni->next) {
        readMetaData(metaHand, *ni, m_hhbcTrans, MetaMode::Legacy);
        translateInstr(*ni);
      }
      return;
    }

    static const auto enabled = Stats::enabledAny() &&
                                getenv("HHVM_STATS_FAILEDINL");
    if (enabled) {
      m_hhbcTrans.profileFunctionEntry("FailedCandidate");
      m_hhbcTrans.profileFailedInlShape(traceletShape(*i.calleeTrace));
    }
  }

  HHIR_EMIT(FCall, numArgs, returnBcOffset, i.funcd,
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void
IRTranslator::translateFCallArray(const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = i.nextSk();
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after,
            JIT::callDestroysLocals(i, m_hhbcTrans.curFunc()));
}

void
IRTranslator::translateNewPackedArray(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  HHIR_EMIT(NewPackedArray, numArgs);
}

void
IRTranslator::translateNewStructArray(const NormalizedInstruction& i) {
  auto numArgs = i.immVec.size();
  auto ids = i.immVec.vec32();
  auto unit = m_hhbcTrans.curUnit();
  StringData* keys[HphpArray::MaxMakeSize];
  for (size_t i = 0; i < numArgs; i++) {
    keys[i] = unit->lookupLitstrId(ids[i]);
  }
  HHIR_EMIT(NewStructArray, numArgs, keys);
}

void
IRTranslator::translateNewCol(const NormalizedInstruction& i) {
  HHIR_EMIT(NewCol, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void IRTranslator::translateClone(const NormalizedInstruction&) {
  HHIR_EMIT(Clone);
}

void
IRTranslator::translateColAddNewElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddNewElemC);
}

void
IRTranslator::translateColAddElemC(const NormalizedInstruction& i) {
  HHIR_EMIT(ColAddElemC);
}

void
IRTranslator::translateStaticLocInit(const NormalizedInstruction& i) {
  HHIR_EMIT(StaticLocInit, i.imm[0].u_IVA, i.imm[1].u_SA);
}

void IRTranslator::translateStaticLoc(const NormalizedInstruction& i) {
  HHIR_EMIT(StaticLoc, i.imm[0].u_IVA, i.imm[1].u_SA);
}

// check class hierarchy and fail if no match
void
IRTranslator::translateVerifyParamType(const NormalizedInstruction& i) {
  int param = i.imm[0].u_IVA;
  HHIR_EMIT(VerifyParamType, param);
}

void
IRTranslator::translateInstanceOfD(const NormalizedInstruction& i) {
  HHIR_EMIT(InstanceOfD, (i.imm[0].u_SA));
}

void
IRTranslator::translateInstanceOf(const NormalizedInstruction& i) {
  HHIR_EMIT(InstanceOf);
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

  if (!i.breaksTracelet && i.nextOffset == targetOffset) {
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
            invertCond);
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
            invertCond);
}

void
IRTranslator::translateIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(IterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
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
            invertCond);
}

void
IRTranslator::translateMIterInit(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInit,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
IRTranslator::translateMIterInitK(const NormalizedInstruction& i) {
  HHIR_EMIT(MIterInitK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
IRTranslator::translateMIterNext(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNext,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA);
}

void
IRTranslator::translateMIterNextK(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterNextK,
            i.imm[0].u_IVA,
            i.offset() + i.imm[1].u_BA,
            i.imm[2].u_IVA,
            i.imm[3].u_IVA);
}

void
IRTranslator::translateWIterInit(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterInit,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
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
            invertCond);
}

void
IRTranslator::translateWIterNext(const NormalizedInstruction& i) {
  bool invertCond = false;
  Offset targetOffset = getBranchTarget(i, invertCond);

  HHIR_EMIT(WIterNext,
            i.imm[0].u_IVA,
            targetOffset,
            i.imm[2].u_IVA,
            invertCond);
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
            invertCond);
}

void
IRTranslator::translateIterFree(const NormalizedInstruction& i) {

  HHIR_EMIT(IterFree, i.imm[0].u_IVA);
}

void
IRTranslator::translateMIterFree(const NormalizedInstruction& i) {

  HHIR_EMIT(MIterFree, i.imm[0].u_IVA);
}

void
IRTranslator::translateIterBreak(const NormalizedInstruction& i) {

  assert(i.breaksTracelet);
  HHIR_EMIT(IterBreak, i.immVec, i.offset() + i.imm[1].u_BA, i.breaksTracelet);
}

void
IRTranslator::translateDecodeCufIter(const NormalizedInstruction& i) {

  HHIR_EMIT(DecodeCufIter, i.imm[0].u_IVA, i.offset() + i.imm[1].u_BA);
}

void
IRTranslator::translateCIterFree(const NormalizedInstruction& i) {

  HHIR_EMIT(CIterFree, i.imm[0].u_IVA);
}

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
#define CASE(iNm)                               \
    case Op::iNm:                               \
      translate ## iNm(i);                      \
      break;
#define TRANSLATE(name, inst) translate ## name(inst); break;
    INSTRS
      PSEUDOINSTR_DISPATCH(TRANSLATE)
#undef TRANSLATE
#undef CASE
    default:
      not_reached();
  }
}

static bool isPop(Op opc) {
  return opc == OpPopC || opc == OpPopR;
}

void
IRTranslator::passPredictedAndInferredTypes(const NormalizedInstruction& i) {
  if (!i.outStack || i.breaksTracelet) return;
  auto const jitType = Type(i.outStack->rtt);

  m_hhbcTrans.setBcOff(i.next->offset(), false);
  if (RuntimeOption::EvalHHIRRelaxGuards) {
    if (i.outputPredicted) {
      if (i.outputPredictionStatic && jitType.notCounted()) {
        // If the prediction is from static analysis it really means jitType |
        // InitNull. When jitType is an uncounted type, we know that the value
        // will always be an uncounted type, so we assert that fact before
        // doing the real check. This allows us to relax the CheckType away
        // while still eliminating some refcounting operations.
        m_hhbcTrans.assertTypeStack(0, Type::Uncounted);
      }
      m_hhbcTrans.checkTypeTopOfStack(jitType, i.next->offset());
    }
    return;
  }

  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);

  if (u == NormalizedInstruction::OutputUse::Inferred) {
    TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
          jitType.toString().c_str());
    m_hhbcTrans.assertTypeStack(0, jitType);

  } else if (u == NormalizedInstruction::OutputUse::Used && i.outputPredicted) {
    // If the value was predicted statically by the front-end, it
    // means that it's either the predicted type or null.  In this
    // case, if the predicted value is not ref-counted and it's simply
    // going to be popped, then pass the information as an assertion
    // that the type is not ref-counted.  This avoid both generating a
    // type check and dec-refing the value.
    if (i.outputPredictionStatic && isPop(i.next->op()) &&
        !jitType.isCounted()) {
      TRACE(1, "irPassPredictedAndInferredTypes: output inferred as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans.assertTypeStack(0, JIT::Type::Uncounted);
    } else {
      TRACE(1, "irPassPredictedAndInferredTypes: output predicted as %s\n",
            jitType.toString().c_str());
      m_hhbcTrans.checkTypeTopOfStack(jitType, i.next->offset());
    }
  }
}

void IRTranslator::interpretInstr(const NormalizedInstruction& i) {
  FTRACE(5, "HHIR: BC Instr {}\n",  i.toString());
  m_hhbcTrans.emitInterpOne(i);
}

static Type flavorToType(FlavorDesc f) {
  switch (f) {
    case NOV: not_reached();

    case CV: return Type::Cell;  // TODO(#3029148) this could be Cell - Uninit
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
              ni.breaksTracelet && !m_hhbcTrans.isInlining());
  FTRACE(1, "\n{:-^60}\n", folly::format("translating {} with stack:\n{}",
                                         ni.toString(), ht.showStack()));
  // When profiling, we disable type predictions to avoid side exits
  assert(JIT::tx64->mode() != TransProfile || !ni.outputPredicted);

  if (ni.guardedThis) {
    // Task #2067635: This should really generate an AssertThis
    ht.setThisAvailable();
  }

  if (moduleEnabled(HPHP::Trace::stats, 2)) {
    ht.emitIncStat(Stats::opcodeToIRPreStatCounter(ni.op()), 1);
  }
  if (RuntimeOption::EnableInstructionCounts ||
      moduleEnabled(HPHP::Trace::stats, 3)) {
    // If the instruction takes a slow exit, the exit trace will
    // decrement the post counter for that opcode.
    ht.emitIncStat(Stats::opcodeToIRPostStatCounter(ni.op()),
                            1, true);
  }

  auto pc = reinterpret_cast<const Op*>(ni.pc());
  for (auto i = 0, num = instrNumPops(pc); i < num; ++i) {
    auto const type = flavorToType(instrInputFlavor(pc, i));
    if (type != Type::Gen) m_hhbcTrans.assertTypeStack(i, type);
  }

  if (instrMustInterp(ni) || ni.interp) {
    interpretInstr(ni);
  } else {
    translateInstrWork(ni);
  }

  passPredictedAndInferredTypes(ni);
}

}}
