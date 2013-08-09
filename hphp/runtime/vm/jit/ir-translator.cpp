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
#include "hphp/util/stack_trace.h"
#include "hphp/util/util.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/base/stats.h"

#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-translator.h"
#include "hphp/runtime/vm/jit/linear-scan.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/opt.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/tracelet.h"

// Include last to localize effects to this file
#include "hphp/util/assert_throw.h"

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

namespace {
bool isInferredType(const NormalizedInstruction& i) {
  return (i.getOutputUsage(i.outStack) ==
          NormalizedInstruction::OutputUse::Inferred);
}

JIT::Type getInferredOrPredictedType(const NormalizedInstruction& i) {
  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);
  if (u == NormalizedInstruction::OutputUse::Inferred ||
     (u == NormalizedInstruction::OutputUse::Used && i.outputPredicted)) {
    return JIT::Type::fromRuntimeType(i.outStack->rtt);
  }
  return JIT::Type::None;
}
}

IRTranslator::IRTranslator(Offset bcOff, Offset spOff, const Func* curFunc)
  : m_hhbcTrans(bcOff, spOff, curFunc)
{
}

void IRTranslator::checkType(const Transl::Location& l,
                             const Transl::RuntimeType& rtt) {
  // We can get invalid inputs as a side effect of reading invalid
  // items out of BBs we truncate; they don't need guards.
  if (rtt.isVagueValue() || l.isThis()) return;

  using Transl::Location;
  switch (l.space) {
    case Location::Stack: {
      uint32_t stackOffset = locPhysicalOffset(l);
      JIT::Type type = JIT::Type::fromRuntimeType(rtt);
      if (type.subtypeOf(Type::Cls)) {
        m_hhbcTrans.assertTypeStack(stackOffset, type);
      } else {
        m_hhbcTrans.guardTypeStack(stackOffset, type);
      }
      break;
    }
    case Location::Local:
      m_hhbcTrans.guardTypeLocal(l.offset, Type::fromRuntimeType(rtt));
      break;

    case Location::Iter:
    case Location::Invalid:
    case Location::Litstr:
    case Location::Litint:
    case Location::This:
      not_reached();
  }
}

void IRTranslator::assertType(const Transl::Location& l,
                              const Transl::RuntimeType& rtt) {
  if (rtt.isVagueValue()) return;

  using Transl::Location;
  switch (l.space) {
    case Location::Stack: {
      // tx64LocPhysicalOffset returns positive offsets for stack values,
      // relative to rVmSp
      uint32_t stackOffset = locPhysicalOffset(l);
      m_hhbcTrans.assertTypeStack(stackOffset, Type::fromRuntimeType(rtt));
      break;
    }
    case Location::Local:  // Stack frame's registers; offset == local register
      m_hhbcTrans.assertTypeLocal(l.offset, Type::fromRuntimeType(rtt));
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
  assert(i.inputs.size() == 2);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);

  DataType leftType = i.inputs[0]->outerType();
  DataType rightType = i.inputs[1]->outerType();
  bool ok = TypeConstraint::equivDataTypes(leftType, rightType) &&
    (i.inputs[0]->isNull() ||
     leftType == KindOfBoolean ||
     i.inputs[0]->isInt());

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
    HHIR_EMIT(EmptyL, i.inputs[0]->location.offset);
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
  DEBUG_ONLY auto const op = i.op();
  assert(op == OpFPassL || op == OpCGetL);
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(inputs[0]->isLocal());

  HHIR_EMIT(CGetL, inputs[0]->location.offset);
}

void
IRTranslator::translateCGetL2(const NormalizedInstruction& ni) {
  const int locIdx   = 1;

  HHIR_EMIT(CGetL2, ni.inputs[locIdx]->location.offset);
}

void
IRTranslator::translateVGetL(const NormalizedInstruction& i) {
  HHIR_EMIT(VGetL, i.inputs[0]->location.offset);
}

void
IRTranslator::translateAssignToLocalOp(const NormalizedInstruction& ni) {
  DEBUG_ONLY const int rhsIdx  = 0;
  const int locIdx  = 1;
  auto const op = ni.op();
  assert(op == OpSetL || op == OpBindL);
  assert(ni.inputs.size() == 2);
  assert((op == OpBindL) ==
         (ni.inputs[rhsIdx]->outerType() == KindOfRef));

  assert(ni.inputs[rhsIdx]->isStack());

  if (op == OpSetL) {
    assert(ni.inputs[locIdx]->isLocal());
    HHIR_EMIT(SetL, ni.inputs[locIdx]->location.offset);
  } else {
    assert(op == OpBindL);
    HHIR_EMIT(BindL, ni.inputs[locIdx]->location.offset);
  }
}

void
IRTranslator::translatePopC(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);

  if (i.inputs[0]->rtt.isVagueValue()) {
    HHIR_EMIT(PopR);
  } else {
    HHIR_EMIT(PopC);
  }
}

void
IRTranslator::translatePopV(const NormalizedInstruction& i) {
  assert(i.inputs[0]->rtt.isVagueValue() || i.inputs[0]->isRef());
  HHIR_EMIT(PopV);
}

void
IRTranslator::translatePopR(const NormalizedInstruction& i) {
  translatePopC(i);
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

void
IRTranslator::translateNull(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);

  HHIR_EMIT(Null);
}

void
IRTranslator::translateNullUninit(const NormalizedInstruction& i) {
  HHIR_EMIT(NullUninit);
}

void
IRTranslator::translateTrue(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);

  HHIR_EMIT(True);
}

void
IRTranslator::translateFalse(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);

  HHIR_EMIT(False);
}

void
IRTranslator::translateInt(const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);

  HHIR_EMIT(Int, i.imm[0].u_I64A);
}

void
IRTranslator::translateDouble(const NormalizedInstruction& i) {
  HHIR_EMIT(Double, i.imm[0].u_DA);
}

void
IRTranslator::translateString(const NormalizedInstruction& i) {
  assert(i.inputs.size()  == 0);

  HHIR_EMIT(String, (i.imm[0].u_SA));
}

void
IRTranslator::translateArray(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 0);

  HHIR_EMIT(Array, i.imm[0].u_AA);
}

void
IRTranslator::translateNewArray(const NormalizedInstruction& i) {
  HHIR_EMIT(NewArray, i.imm[0].u_IVA);
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

void
IRTranslator::translateAddNewElemC(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 2);
  assert(i.inputs[0]->outerType() != KindOfRef);
  assert(i.inputs[1]->outerType() != KindOfRef);
  assert(i.inputs[0]->isStack());
  assert(i.inputs[1]->isStack());

  HHIR_EMIT(AddNewElemC);
}

void
IRTranslator::translateCns(const NormalizedInstruction& i) {
  HHIR_EMIT(Cns, i.imm[0].u_SA);
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
  assert(i.inputs.size() == 2);

  if (i.inputs[0]->valueType() == KindOfArray &&
      i.inputs[1]->valueType() == KindOfArray) {
    HHIR_EMIT(ArrayAdd);
    return;
  }
  HHIR_EMIT(Add);
}

void
IRTranslator::translateSqrt(const NormalizedInstruction& i) {
  HHIR_EMIT(Sqrt);
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
  assert(i.inputs.size() == 1);

  HHIR_EMIT(CastInt);
  /* nop */
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

void
IRTranslator::translateJmp(const NormalizedInstruction& i) {
  HHIR_EMIT(Jmp, i.offset() + i.imm[0].u_BA, i.breaksTracelet, i.noSurprise);
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

const int kEmitClsLocalIdx = 0;

void IRTranslator::translateAGetC(const NormalizedInstruction& ni) {
  const StringData* clsName =
    ni.inputs[kEmitClsLocalIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(AGetC, clsName);
}

void IRTranslator::translateAGetL(const NormalizedInstruction& i) {
  assert(i.inputs[kEmitClsLocalIdx]->isLocal());
  const DynLocation* dynLoc = i.inputs[kEmitClsLocalIdx];
  const StringData* clsName = dynLoc->rtt.valueStringOrNull();
  HHIR_EMIT(AGetL, dynLoc->location.offset, clsName);
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
  HHIR_EMIT(CreateCont, i.imm[0].u_SA);
}

void IRTranslator::translateContEnter(const NormalizedInstruction& i) {
  auto after = i.nextSk().offset();

  // ContEnter can't exist in an inlined function right now.  (If it
  // ever can, this curFunc() needs to change.)
  assert(!m_hhbcTrans.isInlining());
  const Func* srcFunc = i.func();
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

void IRTranslator::translateStrlen(const NormalizedInstruction& i) {
  HHIR_EMIT(Strlen);
}

void IRTranslator::translateIncStat(const NormalizedInstruction& i) {
  HHIR_EMIT(IncStat, i.imm[0].u_IVA, i.imm[1].u_IVA);
}

void IRTranslator::translateArrayIdx(const NormalizedInstruction& i) {
  HHIR_EMIT(ArrayIdx);
}

void IRTranslator::translateClassExists(const NormalizedInstruction& i) {
  const StringData* clsName = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(ClassExists, clsName);
}

void IRTranslator::translateInterfaceExists(const NormalizedInstruction& i) {
  const StringData* ifaceName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(InterfaceExists, ifaceName);
}

void IRTranslator::translateTraitExists(const NormalizedInstruction& i) {
  const StringData* traitName = i.inputs[1]->rtt.valueStringOrNull();

  HHIR_EMIT(TraitExists, traitName);
}

void IRTranslator::translateVGetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetS, propName);
}

void
IRTranslator::translateVGetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(VGetG, name);
}

void IRTranslator::translateBindS(const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(BindS, propName);
}

void IRTranslator::translateEmptyS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyS, propName);
}

void IRTranslator::translateEmptyG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(EmptyG, gblName);
}

void
IRTranslator::translateIssetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetS, propName);
}

void
IRTranslator::translateIssetG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(IssetG, gblName);
}

void
IRTranslator::translateUnsetG(const NormalizedInstruction& i) {
  const StringData* gblName = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(UnsetG, gblName);
}

void
IRTranslator::translateUnsetN(const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetN);
}

void IRTranslator::translateCGetS(const NormalizedInstruction& i) {
  const int kPropNameIdx = 1;
  const StringData* propName = i.inputs[kPropNameIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetS, propName,
            getInferredOrPredictedType(i), isInferredType(i));
}

void IRTranslator::translateSetS(const NormalizedInstruction& i) {
  const int kPropIdx = 2;
  const StringData* propName = i.inputs[kPropIdx]->rtt.valueStringOrNull();
  HHIR_EMIT(SetS, propName);
}

void
IRTranslator::translateCGetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(CGetG, name, getInferredOrPredictedType(i), isInferredType(i));
}

void IRTranslator::translateSetG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(SetG, name);
}

void IRTranslator::translateBindG(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[1]->rtt.valueStringOrNull();
  HHIR_EMIT(BindG, name);
}

void
IRTranslator::translateLateBoundCls(const NormalizedInstruction&i) {
  HHIR_EMIT(LateBoundCls);
}

void IRTranslator::translateFPassL(const NormalizedInstruction& ni) {
  if (ni.preppedByRef) {
    translateVGetL(ni);
  } else {
    translateCGetL(ni);
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
IRTranslator::translateCheckTypeOp(const NormalizedInstruction& ni) {
  assert(ni.inputs.size() == 1);

  auto const op = ni.op();
  const int    off = ni.inputs[0]->location.offset;
  switch (op) {
    case OpIssetL:    HHIR_EMIT(IssetL, off);
    case OpIsNullL:   HHIR_EMIT(IsNullL, off);
    case OpIsNullC:   HHIR_EMIT(IsNullC);
    case OpIsStringL: HHIR_EMIT(IsStringL, off);
    case OpIsStringC: HHIR_EMIT(IsStringC);
    case OpIsArrayL:  HHIR_EMIT(IsArrayL, off);
    case OpIsArrayC:  HHIR_EMIT(IsArrayC);
    case OpIsIntL:    HHIR_EMIT(IsIntL, off);
    case OpIsIntC:    HHIR_EMIT(IsIntC);
    case OpIsBoolL:   HHIR_EMIT(IsBoolL, off);
    case OpIsBoolC:   HHIR_EMIT(IsBoolC);
    case OpIsDoubleL: HHIR_EMIT(IsDoubleL, off);
    case OpIsDoubleC: HHIR_EMIT(IsDoubleC);
    case OpIsObjectL: HHIR_EMIT(IsObjectL, off);
    case OpIsObjectC: HHIR_EMIT(IsObjectC);
    // Note: for IsObject*, we need to emit some kind of
    // call to ObjectData::isResource or something.
    default:          not_reached();
  }
}

void
IRTranslator::translateAKExists(const NormalizedInstruction& ni) {
  HHIR_EMIT(AKExists);
}

void
IRTranslator::translateSetOpL(const NormalizedInstruction& i) {
  Opcode opc;
  switch (i.imm[1].u_OA) {
    case SetOpPlusEqual:   opc = Add;    break;
    case SetOpMinusEqual:  opc = Sub;    break;
    case SetOpMulEqual:    opc = Mul;    break;
    case SetOpDivEqual:    HHIR_UNIMPLEMENTED(SetOpL_Div);
    case SetOpConcatEqual: opc = Concat; break;
    case SetOpModEqual:    HHIR_UNIMPLEMENTED(SetOpL_Mod);
    case SetOpAndEqual:    opc = BitAnd; break;
    case SetOpOrEqual:     opc = BitOr;  break;
    case SetOpXorEqual:    opc = BitXor; break;
    case SetOpSlEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shl);
    case SetOpSrEqual:     HHIR_UNIMPLEMENTED(SetOpL_Shr);
    default: not_reached();
  }
  const int localIdx = 1;
  HHIR_EMIT(SetOpL, opc, i.inputs[localIdx]->location.offset);
}

void
IRTranslator::translateIncDecL(const NormalizedInstruction& i) {
  const vector<DynLocation*>& inputs = i.inputs;
  assert(inputs.size() == 1);
  assert(inputs[0]->isLocal());
  const IncDecOp oplet = IncDecOp(i.imm[1].u_OA);
  assert(oplet == PreInc || oplet == PostInc || oplet == PreDec ||
         oplet == PostDec);
  bool post = (oplet == PostInc || oplet == PostDec);
  bool pre  = !post;
  bool inc  = (oplet == PostInc || oplet == PreInc);

  HHIR_EMIT(IncDecL, pre, inc, inputs[0]->location.offset);
}

void
IRTranslator::translateUnsetL(const NormalizedInstruction& i) {
  HHIR_EMIT(UnsetL, i.inputs[0]->location.offset);
}

void
IRTranslator::translateReqDoc(const NormalizedInstruction& i) {
  const StringData* name = i.inputs[0]->rtt.valueStringOrNull();
  HHIR_EMIT(ReqDoc, name);
}

void IRTranslator::translateDefCls(const NormalizedInstruction& i) {
  int cid = i.imm[0].u_IVA;
  HHIR_EMIT(DefCls, cid, i.source.offset());
}

void IRTranslator::translateDefFunc(const NormalizedInstruction& i) {
  int fid = i.imm[0].u_IVA;
  HHIR_EMIT(DefFunc, fid);
}

void
IRTranslator::translateFPushFunc(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushFunc, (i.imm[0].u_IVA));
}

void
IRTranslator::translateFPushClsMethodD(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushClsMethodD,
             (i.imm[0].u_IVA),
             (i.imm[1].u_SA),
             (i.imm[2].u_SA));
}

void
IRTranslator::translateFPushClsMethodF(const NormalizedInstruction& i) {
  // For now, only support cases where both the class and the method are known.
  const int classIdx  = 0;
  const int methodIdx = 1;
  DynLocation* classLoc  = i.inputs[classIdx];
  DynLocation* methodLoc = i.inputs[methodIdx];

  HHIR_UNIMPLEMENTED_WHEN(!(methodLoc->isString() &&
                            methodLoc->rtt.valueString() != nullptr &&
                            classLoc->valueType() == KindOfClass &&
                            classLoc->rtt.valueClass() != nullptr),
                          FPushClsMethodF_unknown);

  auto cls = classLoc->rtt.valueClass();
  HHIR_EMIT(FPushClsMethodF,
            i.imm[0].u_IVA, // # of arguments
            cls,
            methodLoc->rtt.valueString());
}

void
IRTranslator::translateFPushObjMethodD(const NormalizedInstruction& i) {
  assert(i.inputs.size() == 1);
  HHIR_UNIMPLEMENTED_WHEN((i.inputs[0]->valueType() != KindOfObject),
                          FPushObjMethod_nonObj);
  assert(i.inputs[0]->valueType() == KindOfObject);
  const Class* baseClass = i.inputs[0]->rtt.valueClass();

  HHIR_EMIT(FPushObjMethodD,
            i.imm[0].u_IVA,
            i.imm[1].u_IVA,
            baseClass);
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

// static void fatalNullThis() { raise_error(Strings::FATAL_NULL_THIS); }

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
  HHIR_EMIT(InitThisLoc, i.imm[0].u_HA);
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

void
IRTranslator::translateFPushCufIter(const NormalizedInstruction& i) {
  HHIR_EMIT(FPushCufIter, i.imm[0].u_IVA, i.imm[1].u_IA);
}

static const Func*
findCuf(const NormalizedInstruction& ni,
                    Class*& cls, StringData*& invName, bool& forward) {
  forward = (ni.op() == OpFPushCufF);
  cls = nullptr;
  invName = nullptr;

  DynLocation* callable = ni.inputs[ni.op() == OpFPushCufSafe ? 1 : 0];

  const StringData* str =
    callable->isString() ? callable->rtt.valueString() : nullptr;
  const ArrayData* arr =
    callable->isArray() ? callable->rtt.valueArray() : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = HPHP::Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = StringData::GetStaticString(name.substr(0, pos).get());
    sname = StringData::GetStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    CVarRef e0 = arr->get(int64_t(0), false);
    CVarRef e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  Class* ctx = ni.func()->cls();

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = Unit::lookupUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall, true);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

void
IRTranslator::translateFPushCufOp(const NormalizedInstruction& i) {
  Class* cls = nullptr;
  StringData* invName = nullptr;
  bool forward = false;
  const Func* func = findCuf(i, cls, invName, forward);
  HHIR_EMIT(FPushCufOp, i.op(), cls, invName, func, i.imm[0].u_IVA);
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

  HHIR_EMIT(FCallBuiltin, numArgs, numNonDefault, funcId);
}

bool shouldIRInline(const Func* curFunc,
                    const Func* func,
                    const Tracelet& callee) {
  if (!RuntimeOption::EvalHHIREnableGenTimeInlining) {
    return false;
  }

  auto refuse = [&](const char* why) -> bool {
    FTRACE(1, "shouldIRInline: refusing {} <reason: {}>\n",
              func->fullName()->data(), why);
    return false;
  };
  auto accept = [&](const char* kind) -> bool {
    FTRACE(1, "shouldIRInline: inlining {} <kind: {}>\n",
              func->fullName()->data(), kind);
    return true;
  };

  if (func->numLocals() != func->numParams()) {
    return refuse("more locals than params (unsupported)");
  }
  if (func->numIterators() != 0) {
    return refuse("iterators");
  }
  if (func->maxStackCells() >= kStackCheckLeafPadding) {
    FTRACE(1, "{} >= {}\n", func->maxStackCells(), kStackCheckLeafPadding);
    return refuse("too many stack cells");
  }

  /////////////

  // Little pattern recognition helpers:
  const NormalizedInstruction* cursor;
  Op current;
  auto resetCursor = [&] {
    cursor = callee.m_instrStream.first;
    current = cursor->op();
  };
  auto next = [&]() -> Op {
    auto op = cursor->op();
    cursor = cursor->next;
    current = cursor->op();
    return op;
  };
  auto nextIf = [&](Op op) -> bool {
    if (current != op) return false;
    next();
    return true;
  };
  auto atRet = [&] { return current == OpRetC || current == OpRetV; };

  // Simple operations that just put a Cell on the stack.  There must
  // either be no inputs, or a single local as an input.  For now
  // avoid CreateCont because it depends on the frame.
  auto simpleCell = [&]() -> bool {
    if (current == OpCreateCont) return false;
    if (cursor->outStack && cursor->inputs.empty()) {
      next();
      return true;
    }
    if (current == OpCGetL || current == OpVGetL) {
      next();
      return true;
    }
    return false;
  };

  // Simple two-cell comparison operators.
  auto simpleCmp = [&]() -> bool {
    switch (current) {
    case Op::Add: case Op::Sub: case Op::Mul: case Op::Div: case Op::Mod:
    case Op::Xor: case Op::Not: case Op::Same: case Op::NSame: case Op::Eq:
    case Op::Neq: case Op::Lt: case Op::Lte: case Op::Gt: case Op::Gte:
    case Op::BitAnd: case Op::BitOr: case Op::BitXor: case Op::BitNot:
    case Op::Shl: case Op::Shr:
      next();
      return true;
    default:
      return false;
    }
  };

  // In the various patterns below, when we're down to a cell on the
  // stack, this is used to allow simple constant-foldable
  // manipulations of it before return.
  auto cellManipRet = [&]() -> bool {
    if (nextIf(Op::Not)) return atRet();
    if (simpleCell() && simpleCmp()) return atRet();
    return atRet();
  };

  // Constants that can be printed without an InterpOne.
  auto simplePrintConstant = [&]() -> bool {
    switch (current) {
    case OpFalse: case OpInt: case OpString: case OpTrue: case OpNull:
      next();
      return true;
    default:
      return false;
    }
  };

  resetCursor();

  ////////////

  // Simple property accessors.
  resetCursor();
  if (current == OpCheckThis) next();
  if (cursor->op() == OpCGetM &&
      cursor->immVec.locationCode() == LH &&
      cursor->immVecM.size() == 1 &&
      cursor->immVecM.front() == MPT &&
      !mInstrHasUnknownOffsets(*cursor, func->cls())) {
    next();
    // Can't currently support cellManipRet because it's usually going
    // to be CGetM-prediction, which will use the frame.
    if (atRet()) {
      return accept("simple property accessor");
    }
  }

  /*
   * Functions that set an object property to a simple cell value.
   * E.g. something that does $this->foo = null;
   */
  resetCursor();
  if (current == OpCheckThis) next();
  if (simpleCell()) {
    if (cursor->op() == OpSetM &&
        cursor->immVec.locationCode() == LH &&
        cursor->immVecM.size() == 1 &&
        cursor->immVecM.front() == MPT &&
        !mInstrHasUnknownOffsets(*cursor, func->cls())) {
      next();
      if (nextIf(OpPopC) && simpleCell() && atRet()) {
        return accept("simpleCell prop setter");
      }
    }
  }

  /*
   * Continuation allocation functions.
   */
  resetCursor();
  if (current == OpCreateCont) {
    if (func->numParams()) {
      FTRACE(1, "CreateCont with {} args\n", func->numParams());
    }
    next();
    if (atRet()) {
      return accept("continuation creator");
    }
  }

  /*
   * Anything that just puts a value on the stack with no inputs, and
   * then returns it, after possibly doing some comparison with
   * another such thing.
   *
   * E.g. String; String; Same; RetC, or Null; RetC.
   */
  resetCursor();
  if (simpleCell() && cellManipRet()) {
    return accept("simple returner");
  }

  // BareThis; InstanceOfD; RetC
  resetCursor();
  if (nextIf(OpBareThis) && nextIf(OpInstanceOfD) && atRet()) {
    return accept("$this instanceof D");
  }

  // E.g. String; Print; PopC; Null; RetC
  // Useful primarily for debugging.
  resetCursor();
  if (simplePrintConstant() && nextIf(OpPrint) && nextIf(OpPopC) &&
      simpleCell() && cellManipRet()) {
    return accept("constant printer");
  }

  return refuse("unknown kind of function");
}

void
IRTranslator::translateFCall(const NormalizedInstruction& i) {
  auto const numArgs = i.imm[0].u_IVA;

  always_assert(!m_hhbcTrans.isInlining() && "curUnit and curFunc calls");
  const PC after = i.m_unit->at(i.nextSk().offset());
  const Func* srcFunc = i.func();
  Offset returnBcOffset =
    srcFunc->unit()->offsetOf(after - srcFunc->base());

  /*
   * If we have a calleeTrace, we're going to see if we should inline
   * the call.
   */
  if (i.calleeTrace) {
    if (!i.calleeTrace->m_inliningFailed && !m_hhbcTrans.isInlining()) {
      assert(shouldIRInline(i.func(), i.funcd, *i.calleeTrace));

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

  HHIR_EMIT(FCall, numArgs, returnBcOffset, i.funcd);
}

void
IRTranslator::translateFCallArray(const NormalizedInstruction& i) {
  const Offset pcOffset = i.offset();
  SrcKey next = i.nextSk();
  const Offset after = next.offset();

  HHIR_EMIT(FCallArray, pcOffset, after);
}

void
IRTranslator::translateNewTuple(const NormalizedInstruction& i) {
  int numArgs = i.imm[0].u_IVA;
  HHIR_EMIT(NewTuple, numArgs);
}

void
IRTranslator::translateNewCol(const NormalizedInstruction& i) {
  HHIR_EMIT(NewCol, i.imm[0].u_IVA, i.imm[1].u_IVA);
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
  HHIR_EMIT(IterBreak, i.immVec, i.offset() + i.imm[1].u_BA, i.breaksTracelet,
            i.noSurprise);
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

  NormalizedInstruction::OutputUse u = i.getOutputUsage(i.outStack);
  JIT::Type jitType = JIT::Type::fromRuntimeType(i.outStack->rtt);

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

void IRTranslator::translateInstr(const NormalizedInstruction& i) {
  m_hhbcTrans.setBcOff(i.source.offset(),
                       i.breaksTracelet && !m_hhbcTrans.isInlining());
  FTRACE(1, "\n{:-^60}\n", folly::format("translating {} with stack:\n{}",
                                         i.toString(),
                                         m_hhbcTrans.showStack()));
  // When profiling, we disable type predictions to avoid side exits
  assert(Transl::tx64->mode() != TransProfile || !i.outputPredicted);

  if (i.guardedThis) {
    // Task #2067635: This should really generate an AssertThis
    m_hhbcTrans.setThisAvailable();
  }

  if (moduleEnabled(HPHP::Trace::stats, 2)) {
    m_hhbcTrans.emitIncStat(Stats::opcodeToIRPreStatCounter(i.op()), 1);
  }
  if (RuntimeOption::EnableInstructionCounts ||
      moduleEnabled(HPHP::Trace::stats, 3)) {
    // If the instruction takes a slow exit, the exit trace will
    // decrement the post counter for that opcode.
    m_hhbcTrans.emitIncStat(Stats::opcodeToIRPostStatCounter(i.op()),
                            1, true);
  }

  if (instrMustInterp(i) || i.interp) {
    interpretInstr(i);
  } else {
    translateInstrWork(i);
  }

  if (Transl::callDestroysLocals(i, m_hhbcTrans.curFunc())) {
    m_hhbcTrans.emitSmashLocals();
  }

  passPredictedAndInferredTypes(i);
}

}}
